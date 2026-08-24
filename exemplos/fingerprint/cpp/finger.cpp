/**
 * finger.cpp — Exemplo do módulo de impressão digital via biblioteca libK044AVT.so
 *
 * Porta do antigo aplicativo Borland C++ Builder (finger_vcl_reference.cpp, que
 * falava direto com a porta serial do Windows) para Linux usando o driver K044.
 * O firmware do AT89S52 faz o passthrough PS/2 (0xAD/0xAE) <-> UART do módulo,
 * então aqui basta usar a API k044_fp_* já implementada na biblioteca — não é
 * mais preciso montar/parsear os pacotes do protocolo à mão.
 *
 * Protocolo do módulo (referência): ver
 *   "Fingerprint module product user manual_V1.1 (communication protocol).pdf"
 *   em ../reference/, e a implementação em driver_display/display_driver.c.
 *   O código Borland original está em ../reference/finger_vcl_reference.cpp.
 *
 * Compilar: make
 * Executar: sudo ./finger     (k044_open() precisa de acesso às portas I/O)
 */

#include "../../../driver_display/display_driver.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <thread>

/* ------------------------------------------------------------------------- */
/* Utilitários de entrada e diagnóstico                                      */
/* ------------------------------------------------------------------------- */

static int read_line(char *buf, size_t len)
{
    if (!fgets(buf, (int)len, stdin)) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

static int prompt_int(const char *msg, int def)
{
    char line[64];
    printf("%s [%d]: ", msg, def);
    fflush(stdout);
    if (read_line(line, sizeof(line)) != 0) return def;
    if (line[0] == '\0') return def;
    return atoi(line);
}

/* ------------------------------------------------------------------------- */
/* Diagnóstico de estado do dispositivo                                      */
/* ------------------------------------------------------------------------- */

static void diag_status(void)
{
    int dead   = k044_is_device_dead();
    int uinput = k044_uinput_is_active();
    int mouse  = k044_mouse_is_active();

    printf("  [diag] device_dead=%s",     dead  ? "SIM" : "não");
    printf("  uinput=%s",                 uinput ? "sim" : "não");
    printf("  mouse_forward=%s\n",        mouse  ? "sim" : "não");

    uint8_t id[2] = {0, 0};
    int r = k044_read_id(id);
    if (r == K044_OK && id[0] == 0xAB && id[1] == 0x83)
        printf("  [diag] ID do dispositivo: 0x%02X 0x%02X (OK)\n", id[0], id[1]);
    else
        printf("  [diag] k044_read_id falhou: %d  (id=0x%02X 0x%02X)\n", r, id[0], id[1]);

    uint16_t fp_count = 0;
    r = k044_fp_valid_template_count(&fp_count);
    if (r == K044_OK)
        printf("  [diag] módulo fingerprint: OK  (templates=%u)\n", fp_count);
    else
        printf("  [diag] módulo fingerprint: falha (%d)\n", r);
}

/* ------------------------------------------------------------------------- */
/* Callback de progresso (enroll/identify)                                   */
/* ------------------------------------------------------------------------- */

static void fp_progress(int status, int step, void *userdata)
{
    (void)userdata;
    if (status != 0) {
        printf("    [fp] %s (0x%02X)\n", k044_fp_confirmation_str(status), status);
    } else {
        printf("    [fp] etapa %d — siga as instruções do sensor\n", step);
    }
    fflush(stdout);
}

/* ------------------------------------------------------------------------- */
/* Utilitário: executa operação com log de diagnóstico                       */
/* ------------------------------------------------------------------------- */

static int fp_exec(const char *label, int r)
{
    int dead = k044_is_device_dead();
    printf("  [log] %s: ret=%d", label, r);
    if (r == K044_ERR_FP) {
        int cc = k044_fp_last_confirmation_code();
        printf(" (sensor: 0x%02X %s)", cc, k044_fp_confirmation_str(cc));
    }
    if (dead) printf("  !!! DEVICE_DEAD !!!");
    printf("\n");
    return r;
}

/* ------------------------------------------------------------------------- */
/* Espera dedo presente/ausente via polling de GetImage (0x01)               */
/* ------------------------------------------------------------------------- */

/* present=true: espera GetImage retornar sucesso (dedo capturado).
 * present=false: espera GetImage voltar com "Sensor vazio" (0x02), ou seja,
 * dedo removido. Retorna K044_OK se atingiu o estado esperado, ou
 * K044_ERR_FP_TIMEOUT se estourou o prazo (timeout_ms). */
static int wait_for_finger(bool present, int timeout_ms)
{
    auto deadline = std::chrono::steady_clock::now() +
                     std::chrono::milliseconds(timeout_ms);
    while (std::chrono::steady_clock::now() < deadline) {
        int r = k044_fp_get_image();
        if (present && r == K044_OK)
            return K044_OK;
        if (!present && r != K044_OK &&
            k044_fp_last_confirmation_code() == 0x02)
            return K044_OK;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    return K044_ERR_FP_TIMEOUT;
}

#define CAPTURE_MAX_ATTEMPTS 5

/* Espera o dedo e gera o template (GenChar) no buffer indicado, repetindo
 * a leitura automaticamente quando ela falha por qualidade (ex.: "Ponto de
 * recurso muito pequeno", "Qualidade da imagem muito baixa") — erro comum
 * de posicionamento do dedo, nao de hardware. Desiste apos
 * CAPTURE_MAX_ATTEMPTS tentativas. Retorna K044_OK com o template pronto,
 * ou o ultimo erro (timeout esperando o dedo, ou falha de leitura
 * persistente). */
static int capture_gen_char(uint8_t buffer_id)
{
    for (int attempt = 1; attempt <= CAPTURE_MAX_ATTEMPTS; attempt++) {
        int r = wait_for_finger(true, 10000);
        if (r != K044_OK) {
            printf("  Timeout esperando o dedo (10s).\n");
            return r;
        }
        r = fp_exec("k044_fp_gen_char", k044_fp_gen_char(buffer_id));
        if (r == K044_OK) return K044_OK;

        printf("  Leitura ruim (tentativa %d/%d): %s.\n", attempt, CAPTURE_MAX_ATTEMPTS,
               k044_fp_confirmation_str(k044_fp_last_confirmation_code()));
        if (attempt < CAPTURE_MAX_ATTEMPTS) {
            printf("  Retire e coloque o dedo novamente...\n");
            wait_for_finger(false, 3000); /* best-effort; segue mesmo em timeout */
        }
    }
    printf("  Falha na leitura apos %d tentativas.\n", CAPTURE_MAX_ATTEMPTS);
    return K044_ERR_FP;
}

/* ------------------------------------------------------------------------- */
/* Operações                                                                 */
/* ------------------------------------------------------------------------- */

static void op_info(void)
{
    k044_fp_nfpage_t page;
    memset(&page, 0, sizeof(page));
    int r = fp_exec("k044_fp_read_nfpage", k044_fp_read_nfpage(&page));
    if (r != K044_OK) {
        printf("  Falha ao ler informações do módulo (%d)\n", r);
        return;
    }
    printf("  Registros cadastrados : %u\n", page.registros);
    printf("  Capacidade do banco   : %u\n", page.database_size);
    printf("  Nível de segurança    : %u\n", page.secur_level);
    printf("  Número de série       : %s\n", page.product_sn);
    printf("  Versão de software    : %s\n", page.software_version);
    printf("  Fabricante            : %s\n", page.manufacturer);
    printf("  Sensor                : %s\n", page.sensor_name);
}

static void op_count(void)
{
    uint16_t count = 0;
    int r = fp_exec("k044_fp_valid_template_count", k044_fp_valid_template_count(&count));
    if (r != K044_OK) {
        printf("  Falha ao contar templates (%d)\n", r);
        return;
    }
    printf("  Templates válidos no banco: %u\n", count);
}

/* Cadastro manual em 2 leituras (GetImage+GenChar x2, RegModel, Store).
 * Usado no lugar de k044_fp_auto_enroll() (comando estendido 0x31) porque
 * o sensor GSL6152 desta placa devolve pacote malformado (cc=0xFF) para
 * 0x31 — clones AS608 baratos costumam so implementar o conjunto base do
 * protocolo, sem os comandos estendidos AutoEnroll/AutoIdentify. */
static void op_enroll(void)
{
    int id = prompt_int("ID para cadastrar (1-999)", 1);
    printf("  Cadastrando ID %d (modo manual, 2 leituras)\n", id);

    printf("  Coloque o dedo no sensor (1a leitura)...\n");
    int r = capture_gen_char(1);
    if (r != K044_OK) { printf("  Cadastro cancelado (1a leitura).\n"); return; }

    printf("  Retire o dedo...\n");
    wait_for_finger(false, 5000); /* best-effort; segue mesmo em timeout */

    printf("  Coloque o MESMO dedo novamente (2a leitura)...\n");
    r = capture_gen_char(2);
    if (r != K044_OK) { printf("  Cadastro cancelado (2a leitura).\n"); return; }

    r = fp_exec("k044_fp_reg_model", k044_fp_reg_model());
    if (r != K044_OK) { printf("  As duas leituras nao correspondem ao mesmo dedo (%d).\n", r); return; }

    r = fp_exec("k044_fp_store", k044_fp_store(1, (uint16_t)id));
    if (r == K044_OK)
        printf("  Cadastro concluído: ID %d armazenado.\n", id);
    else
        printf("  Falha ao armazenar no banco (%d).\n", r);
}

/* Identificacao manual (GetImage+GenChar+Search) em vez de
 * k044_fp_auto_identify() (comando estendido 0x32) — mesmo motivo do
 * op_enroll(): o sensor GSL6152 desta placa devolve pacote malformado
 * (cc=0xFF) para 0x32, mesmo apos estender o timeout de resposta no
 * firmware (RFP_CMD), enquanto comandos base (GetImage/GenChar/Search)
 * continuam respondendo normalmente — indício de que o modulo nao
 * implementa o comando estendido, nao um problema de timing. O Search
 * base do AS608 nao aceita threshold de score como parametro (o modulo
 * usa seu proprio nivel de seguranca interno), entao nao ha prompt de
 * score aqui. */
static void op_identify(void)
{
    printf("  Coloque o dedo no sensor para identificar...\n");
    int r = capture_gen_char(1);
    if (r != K044_OK) return;
    uint16_t found_id = 0xFFFF;
    r = fp_exec("k044_fp_search", k044_fp_search(1, 0, 1000, &found_id));
    if (r == K044_OK && found_id != 0xFFFF)
        printf("  Identificado! ID = %u\n", found_id);
    else
        printf("  Digital não reconhecida (%d).\n", r);
}

static void op_search(void)
{
    printf("  Coloque o dedo no sensor para buscar no banco...\n");
    int r = capture_gen_char(1);
    if (r != K044_OK) return;
    uint16_t found_id = 0xFFFF;
    r = fp_exec("k044_fp_search", k044_fp_search(1, 0, 1000, &found_id));
    if (r == K044_OK && found_id != 0xFFFF)
        printf("  Encontrado no banco: ID = %u\n", found_id);
    else
        printf("  Nenhuma correspondência no banco (%d).\n", r);
}

static void op_delete(void)
{
    int id = prompt_int("ID do template a remover", 1);
    int r = fp_exec("k044_fp_delete_char", k044_fp_delete_char((uint16_t)id, 1));
    printf(r == K044_OK ? "  Template %d removido.\n" : "  Falha ao remover (%d).\n",
           r == K044_OK ? id : r);
}

static void op_empty(void)
{
    char line[16];
    printf("  Apagar TODO o banco de digitais? (s/N): ");
    fflush(stdout);
    if (read_line(line, sizeof(line)) != 0 || (line[0] != 's' && line[0] != 'S')) {
        printf("  Cancelado.\n");
        return;
    }
    int r = fp_exec("k044_fp_empty", k044_fp_empty());
    printf(r == K044_OK ? "  Banco apagado.\n" : "  Falha ao apagar (%d).\n", r);
}

static void op_sleep(void)
{
    int r = fp_exec("k044_fp_sleep", k044_fp_sleep());
    printf(r == K044_OK ? "  Módulo em modo sleep.\n" : "  Falha ao entrar em sleep (%d).\n", r);
}

/* ------------------------------------------------------------------------- */

static void menu(void)
{
    printf("\n===== Módulo de Impressão Digital (K044AVT) =====\n");
    printf("  1) Informações do módulo\n");
    printf("  2) Contar templates cadastrados\n");
    printf("  3) Cadastrar digital (2 leituras)\n");
    printf("  4) Identificar digital (getImage+search)\n");
    printf("  5) Buscar digital no banco (getImage+search)\n");
    printf("  6) Remover um template\n");
    printf("  7) Apagar todo o banco\n");
    printf("  8) Sleep\n");
    printf("  9) Diagnóstico de estado\n");
    printf("  0) Sair\n");
    printf("Opção: ");
    fflush(stdout);
}

int main(void)
{
    printf("finger — exemplo do módulo de digital via libK044AVT %s\n", k044_version());
    fflush(stdout);

    int r = k044_open();
    if (r != K044_OK) {
        fprintf(stderr, "Falha ao abrir o dispositivo (%d). Execute com sudo.\n", r);
        return 1;
    }

    if (k044_fp_init() != K044_OK) {
        fprintf(stderr, "Módulo de digital não respondeu (k044_fp_init).\n");
        k044_close();
        return 1;
    }

    k044_fp_set_callback(fp_progress, NULL);

    k044_set_log_level(K044_LOG_DEBUG);

    printf("--- Diagnóstico inicial ---\n");
    diag_status();
    printf("---------------------------\n");

    for (;;) {
        menu();
        char line[16];
        if (read_line(line, sizeof(line)) != 0) break;
        int opt = atoi(line);
        if (line[0] == '0') break;

        switch (opt) {
            case 1: op_info();     break;
            case 2: op_count();    break;
            case 3: op_enroll();   break;
            case 4: op_identify(); break;
            case 5: op_search();   break;
            case 6: op_delete();   break;
            case 7: op_empty();    break;
            case 8: op_sleep();    break;
            case 9: diag_status(); break;
            default: printf("  Opção inválida.\n"); break;
        }
    }

    k044_fp_set_callback(NULL, NULL);
    k044_close();
    printf("Encerrado.\n");
    return 0;
}
