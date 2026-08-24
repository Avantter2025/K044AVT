/**
 * finger2_v2.cpp — "Revisão 2" de finger2.cpp: mesmo menu/comportamento,
 * mas cadastrar/identificar/buscar usam as novas funções de alto nível
 * k044_fp_enroll()/k044_fp_search_retry() (driver_display/display_driver.h)
 * em vez de reimplementar o fluxo multi-passo (esperar dedo, capturar,
 * retirar dedo, capturar de novo, mesclar, gravar) aqui no exemplo.
 *
 * finger2.cpp (o original) permanece intocado — este arquivo existe em
 * paralelo, para não arriscar regressão no exemplo já validado, e para
 * demonstrar/testar a nova API de alto nível isoladamente. Compare
 * op_enroll()/op_identify()/op_search() aqui com os equivalentes em
 * finger2.cpp: a lógica de espera/retentativa saiu do exemplo e foi
 * embutida na biblioteca (agora reaproveitável por qualquer binding —
 * C, C++, Java, Python — sem duplicar o state machine em cada um).
 *
 * Todo o resto (setup de teclado/mouse/aux, diagnóstico, demais
 * operações) é idêntico a finger2.cpp — ver os comentários lá para o
 * histórico completo de por que cada coisa é como é.
 *
 * Compilar: make
 * Executar: sudo ./finger2_v2
 */

#include "../../../driver_display/display_driver.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

/* ------------------------------------------------------------------------- */
/* Utilitários de entrada e diagnóstico (idênticos a finger2.cpp)            */
/* ------------------------------------------------------------------------- */

static int read_line(char *buf, size_t len)
{
    if (!fgets(buf, (int)len, stdin)) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

static int read_menu_choice(char *buf, size_t len)
{
    for (;;) {
        if (read_line(buf, len) != 0) return -1;
        if (buf[0] != '\0') return 0;
    }
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
/* Callback de progresso — agora precisa entender os novos K044_FP_STEP_*   */
/* relatados por k044_fp_enroll()/k044_fp_search_retry(), além dos steps    */
/* numéricos crus do AutoEnroll (não usados por este exemplo, mas o         */
/* callback é o mesmo ponto de registro para os dois).                      */
/* ------------------------------------------------------------------------- */

static void fp_progress(int status, int step, void *userdata)
{
    (void)userdata;
    if (status != 0) {
        printf("    [fp] %s (0x%02X)\n", k044_fp_confirmation_str(status), status);
        fflush(stdout);
        return;
    }
    switch (step) {
        case K044_FP_STEP_WAIT_FINGER:   printf("    [fp] coloque o dedo no sensor...\n"); break;
        case K044_FP_STEP_CAPTURED:      printf("    [fp] leitura capturada.\n"); break;
        case K044_FP_STEP_REMOVE_FINGER: printf("    [fp] retire o dedo...\n"); break;
        case K044_FP_STEP_MERGING:       printf("    [fp] processando leituras...\n"); break;
        case K044_FP_STEP_STORING:       printf("    [fp] gravando no banco...\n"); break;
        default:                         printf("    [fp] etapa %d\n", step); break;
    }
    fflush(stdout);
}

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

/* Cadastro em 3 leituras — todo o fluxo (esperar/capturar/retirar/mesclar/
 * gravar) agora mora em k044_fp_enroll() (driver_display/display_driver.c);
 * aqui só chamamos e reportamos o resultado. Compare com op_enroll() em
 * finger2.cpp, que reimplementa o mesmo fluxo linha a linha. */
static void op_enroll(void)
{
    int id = prompt_int("ID para cadastrar (1-999)", 1);
    printf("  Cadastrando ID %d (3 leituras)...\n", id);

    int r = fp_exec("k044_fp_enroll", k044_fp_enroll((uint16_t)id, 10000));
    if (r == K044_OK)
        printf("  Cadastro concluído: ID %d armazenado.\n", id);
    else
        printf("  Cadastro falhou (%d).\n", r);
}

/* Identificação = busca de tentativa única (max_attempts=1), mesma
 * simplificação já usada em finger2.cpp (o Search base do AS608 não
 * aceita threshold de score — o módulo usa seu próprio nível interno). */
static void op_identify(void)
{
    printf("  Coloque o dedo no sensor para identificar...\n");
    uint16_t found_id = 0xFFFF;
    int r = fp_exec("k044_fp_search_retry",
                    k044_fp_search_retry(1, 0, 1000, 10000, 1, &found_id));
    if (r == K044_OK && found_id != 0xFFFF)
        printf("  Identificado! ID = %u\n", found_id);
    else
        printf("  Digital não reconhecida (%d).\n", r);
}

/* Busca com até 3 tentativas de captura+busca — fluxo completo agora em
 * k044_fp_search_retry(). Compare com op_search() em finger2.cpp. */
static void op_search(void)
{
    printf("  Coloque o dedo no sensor para buscar no banco (até 3 tentativas)...\n");
    uint16_t found_id = 0xFFFF;
    int r = fp_exec("k044_fp_search_retry",
                    k044_fp_search_retry(1, 0, 1000, 10000, 3, &found_id));
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

static void op_led(void)
{
    printf("\n  --- LED do sensor ---\n");
    printf("  1) Aceso\n");
    printf("  2) Apagado\n");
    printf("  3) Piscando (rápido)\n");
    printf("  4) Piscando lento (respirando)\n");
    int choice = prompt_int("Opção", 1);

    uint8_t func;
    const char *label;
    switch (choice) {
        case 1: func = K044_FP_LED_ALWAYS_ON;  label = "aceso";           break;
        case 2: func = K044_FP_LED_ALWAYS_OFF; label = "apagado";         break;
        case 3: func = K044_FP_LED_FLASHING;   label = "piscando (rápido)"; break;
        case 4: func = K044_FP_LED_BREATHING;  label = "piscando lento (respirando)"; break;
        default:
            printf("  Opção inválida.\n");
            return;
    }

    int r = fp_exec("k044_fp_led_config",
                    k044_fp_led_config(func, K044_FP_LED_COLOR_BLUE,
                                        K044_FP_LED_COLOR_BLUE, 0));
    if (r == K044_OK)
        printf("  LED %s.\n", label);
    else
        printf("  Falha ao configurar LED (%d).\n", r);
}

/* ------------------------------------------------------------------------- */

static void menu(void)
{
    printf("\n===== Módulo de Impressão Digital (K044AVT) — v2 (API de alto nível) =====\n");
    printf("  1) Informações do módulo\n");
    printf("  2) Contar templates cadastrados\n");
    printf("  3) Cadastrar digital (3 leituras)\n");
    printf("  4) Identificar digital (getImage+search)\n");
    printf("  5) Buscar digital no banco (getImage+search)\n");
    printf("  6) Remover um template\n");
    printf("  7) Apagar todo o banco\n");
    printf("  8) Sleep\n");
    printf("  9) Diagnóstico de estado\n");
    printf(" 10) Controlar LED do sensor\n");
    printf("  0) Sair\n");
    printf("Opção: ");
    fflush(stdout);
}

int main(void)
{
    printf("finger2_v2 — exemplo do módulo de digital via libK044AVT %s (API de alto nível, com teclado ativo)\n",
           k044_version());
    fflush(stdout);

    int r = k044_open();
    if (r != K044_OK) {
        fprintf(stderr, "Falha ao abrir o dispositivo (%d). Execute com sudo.\n", r);
        return 1;
    }

    if (k044_uinput_enable() != K044_OK)
        fprintf(stderr, "Aviso: uinput indisponivel (teclado nao sera repassado ao sistema).\n");
    if (k044_mouse_enable() != K044_OK)
        fprintf(stderr, "Aviso: mouse PS/2 indisponivel.\n");
    k044_start_event_loop();
    if (k044_aux_enable() != K044_OK)
        fprintf(stderr, "Aviso: teclado auxiliar PS/2 indisponivel.\n");

    if (k044_fp_init() != K044_OK) {
        fprintf(stderr, "Módulo de digital não respondeu (k044_fp_init).\n");
        k044_aux_disable();
        k044_mouse_disable();
        k044_uinput_disable();
        k044_close();
        return 1;
    }

    k044_fp_set_callback(fp_progress, NULL);
    //k044_set_log_level(K044_LOG_DEBUG);
    k044_set_log_level(K044_LOG_TRACE);

    printf("--- Diagnóstico inicial ---\n");
    diag_status();
    printf("---------------------------\n");

    for (;;) {
        menu();
        char line[16];
        if (read_menu_choice(line, sizeof(line)) != 0) break;

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
            case 10: op_led();     break;
            default: printf("  Opção inválida.\n"); break;
        }
    }

    k044_fp_set_callback(NULL, NULL);
    k044_aux_disable();
    k044_mouse_disable();
    k044_uinput_disable();
    k044_close();
    printf("Encerrado.\n");
    return 0;
}
