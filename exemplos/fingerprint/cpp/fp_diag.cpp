/**
 * fp_diag.cpp — Diagnóstico do módulo fingerprint via libK044AVT.so
 *
 * Testa a comunicação PS/2 → AT89S52 → UART → Módulo FP em etapas,
 * expondo exatamente onde a corrente quebra.
 */

#include "../../../driver_display/display_driver.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

/* ----------------------------------------------------------------------- */
/* Etapas do diagnóstico                                                    */
/* ----------------------------------------------------------------------- */

static int etapa_barramento(void)
{
    printf("\n=== Etapa 1: Barramento PS/2 ===\n");

    int dead = k044_is_device_dead();
    printf("  device_dead = %s\n", dead ? "SIM (!!!)" : "não");

    uint8_t id[2] = {0, 0};
    int r = k044_read_id(id);
    if (r == K044_OK && id[0] == 0xAB && id[1] == 0x83) {
        printf("  k044_read_id: 0x%02X 0x%02X  => OK\n", id[0], id[1]);
        return 1;
    }

    printf("  k044_read_id: falhou (r=%d, id=0x%02X 0x%02X)\n", r, id[0], id[1]);
    printf("  => DISPOSITIVO NÃO RESPONDE no barramento PS/2\n");
    return 0;
}

/* Versao do firmware do AT89S52 (comando 0xAF, independente do 0xAD/0xAE).
 * Serve para distinguir "0xAD rejeitado porque este firmware especifico
 * nao implementa o passthrough fingerprint" de um problema mais generico
 * de barramento — 0xAF respondendo OK enquanto 0xAD falha 100% das vezes
 * (nao intermitente) aponta fortemente para a primeira hipotese. */
static void mostra_versao_firmware(void)
{
    uint8_t major = 0, minor = 0, patch = 0;
    int r = k044_read_fw_ver(&major, &minor, &patch);
    if (r == K044_OK) {
        printf("  Versao do firmware (0xAF): %u.%u.%u\n", major, minor, patch);
    } else {
        printf("  Versao do firmware (0xAF): falhou (r=%d)\n", r);
    }
}

static int etapa_comando_ad(void)
{
    printf("\n=== Etapa 2: Comando 0xAD isolado ===\n");
    printf("  (Testa se o firmware aceita o passthrough PS/2->UART)\n");

    /* O k044_fp_init() é um no-op (só zera callbacks).
     * O primeiro comando real que toca no hardware é o 0xAD.
     * Vamos testá-lo diretamente com um comando fingerprint real. */

    int dead = k044_is_device_dead();
    printf("  device_dead = %s\n", dead ? "SIM (!!!)" : "não");

    /* Tenta o comando mais leve: valid template count (0x1D) */
    uint16_t count = 0xFFFF;
    int r = k044_fp_valid_template_count(&count);
    if (r == K044_OK) {
        printf("  k044_fp_valid_template_count: OK  (templates=%u)\n", count);
        return 1;
    }

    printf("  k044_fp_valid_template_count: falhou (r=%d)\n", r);

    /* Se o comando 0xAD falhou, tenta um reset parcial: 
     * manda k044_fp_sleep() seguido de novo get_image pra ver
     * se o módulo acorda */
    printf("  Tentando recuperar: k044_fp_sleep + k044_fp_get_image...\n");
    k044_fp_sleep();
    r = k044_fp_get_image();
    printf("  k044_fp_get_image: ret=%d  (%s)\n",
           r, (r == K044_OK) ? "OK" : k044_fp_confirmation_str(k044_fp_last_confirmation_code()));

    return r == K044_OK ? 1 : 0;
}

static int etapa_nfpage(void)
{
    printf("\n=== Etapa 3: Leitura NFPage (k044_fp_read_nfpage) ===\n");
    printf("  (Testa comando 0xAD com resposta multi-byte)\n");

    k044_fp_nfpage_t page;
    memset(&page, 0, sizeof(page));
    int r = k044_fp_read_nfpage(&page);
    if (r != K044_OK) {
        printf("  k044_fp_read_nfpage: falhou (r=%d)\n", r);
        if (r == K044_ERR_FP) {
            int cc = k044_fp_last_confirmation_code();
            printf("  Confirmation code do módulo: 0x%02X (%s)\n",
                   cc, k044_fp_confirmation_str(cc));
        }
        return 0;
    }

    printf("  Número de série       : %s\n", page.product_sn);
    printf("  Versão de software    : %s\n", page.software_version);
    printf("  Fabricante            : %s\n", page.manufacturer);
    printf("  Sensor                : %s\n", page.sensor_name);
    printf("  Registros             : %u\n", page.registros);
    printf("  Capacidade do banco   : %u\n", page.database_size);
    printf("  Nível de segurança    : %u\n", page.secur_level);

    int ok = (page.product_sn[0] != '\0' && page.software_version[0] != '\0');
    printf("  => NFPage %s\n", ok ? "válida (OK)" : "inválida (dados vazios)");
    return ok ? 1 : 0;
}

/* ----------------------------------------------------------------------- */
/* Main                                                                     */
/* ----------------------------------------------------------------------- */

int main(void)
{
    printf("=============================================\n");
    printf("  Diagnóstico do Módulo Fingerprint (K044AVT)\n");
    printf("  libK044AVT %s\n", k044_version());
    printf("=============================================\n");

    /* Ativa log DEBUG da biblioteca */
    k044_set_log_level(K044_LOG_DEBUG);
    printf("\n[Log da biblioteca em DEBUG ativado — mensagens com prefixo K044AVT]\n");

    /* ---- Open ---- */
    printf("\n>>> k044_open()...\n");
    int r = k044_open();
    if (r != K044_OK) {
        fprintf(stderr, "FALHA: k044_open() = %d (execute com sudo)\n", r);
        return 1;
    }
    printf("k044_open: OK\n");

    /* NOTA: k044_fp_init() é um no-op (só zera callbacks).
     * O primeiro comando real que toca no hardware é via 0xAD,
     * testado na Etapa 2. */

    /* ---- Etapas ---- */
    int n_tests = 0, n_pass = 0;

    n_tests++; n_pass += etapa_barramento();
    mostra_versao_firmware();
    n_tests++; n_pass += etapa_comando_ad();
    if (n_pass >= 1) {
        n_tests++; n_pass += etapa_nfpage();
    } else {
        printf("\n=== Etapa 3: NFPage — PULADA (etapa 2 falhou) ===\n");
    }

    /* ---- Resumo ---- */
    printf("\n=============================================\n");
    printf("  Etapas: %d passaram de %d\n", n_pass, n_tests);
    if (n_pass == n_tests) {
        printf("  DIAGNÓSTICO: Todos os testes PASSARAM.\n");
        printf("  O módulo fingerprint está operacional.\n");
    } else {
        int n_fail = n_tests - n_pass;
        printf("  DIAGNÓSTICO: %d etapa(s) com falha.\n", n_fail);
        printf("  Legenda:\n");
        printf("    Etapa 1 (barramento PS/2) — falha se k044_read_id falha\n");
        printf("    Etapa 2 (comando 0xAD)    — falha se firmware rejeita passthrough\n");
        printf("    Etapa 3 (NFPage)          — falha se módulo FP não responde dados\n");
        printf("\n");
        printf("  Se 0xAD falhou com RESEND em TODAS as tentativas (não\n");
        printf("  intermitente) mas a Etapa 1 (0xF2) e a leitura de versão de\n");
        printf("  firmware (0xAF) acima funcionaram normalmente, o problema\n");
        printf("  não é o barramento PS/2 em si — mais retentativas ou\n");
        printf("  recover=1 não ajudam (0xAD já é retentado ~23x com\n");
        printf("  do_recover=0 propositalmente, para não disparar um RESET\n");
        printf("  em loop). As causas mais prováveis são:\n");
        printf("    1) O firmware atualmente gravado no AT89S52 é anterior à\n");
        printf("       adição dos handlers RFP_CMD/RFP_POLL (0xAD/0xAE) —\n");
        printf("       confira a versão acima contra firmware/TEC44fst.ASM e,\n");
        printf("       se for o caso, regrave com firmware/TEC44FST.HEX\n");
        printf("       (veja firmware/compile.sh).\n");
        printf("    2) O módulo AS608 não está fisicamente conectado/\n");
        printf("       alimentado (confira VCC/TX/RX/GND — ver\n");
        printf("       fingerprint_para_usuario.txt, seção 2).\n");
    }
    printf("=============================================\n");

    k044_close();
    printf("Encerrado.\n");
    return n_pass == n_tests ? 0 : 1;
}
