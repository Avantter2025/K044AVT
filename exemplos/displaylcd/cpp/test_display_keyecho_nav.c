#define _DEFAULT_SOURCE
#include "display_driver.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

static volatile int g_done = 0;
static volatile int g_exit  = 0;
static char         g_last_buf[41] = {0};
static char         g_teste_nome[40] = {0};
static char         g_label_l1[41] = {0};
static char         g_label_l2[41] = {0};
static uint8_t      g_kb_row      = 1;
static uint8_t      g_kb_col_start = 0;

#define SC_F1 0x05
#define SC_F2 0x06

static void on_raw_event(const k044_event_t *evt, void *userdata)
{
    (void)userdata;
    if (evt->type == K044_EVT_KEY_MAKE) {
        if (evt->scancode == SC_F1) {
            g_done = 1;
            g_exit = 1;
        }
        if (evt->scancode == SC_F2) {
            g_done = 1;
        }
    }
}

static void on_keyecho(k044_keyecho_event_t event,
                        const char *buffer, uint8_t len,
                        char last_char, void *userdata)
{
    uint8_t cursor = 0;
    k044_keyecho_get_cursor(&cursor);
    (void)userdata;

    switch (event) {
    case K044_KEYECHO_CHAR:
        printf("  CHAR '%c'  buf=\"%.*s\" cursor=%u\n",
               last_char, (int)len, buffer, cursor);
        break;
    case K044_KEYECHO_BACKSPACE:
        printf("  BACKSPACE  buf=\"%.*s\" cursor=%u\n",
               (int)len, buffer, cursor);
        break;
    case K044_KEYECHO_DELETE:
        printf("  DELETE     buf=\"%.*s\" cursor=%u\n",
               (int)len, buffer, cursor);
        break;
    case K044_KEYECHO_CLEARED:
        printf("  CLEARED buf=\"%.*s\" cursor=%u\n",
               (int)len, buffer, cursor);
        k044_write_display(g_label_l1, g_label_l2);
        k044_set_cursor(g_kb_row, g_kb_col_start);
        break;
    case K044_KEYECHO_CONFIRMED:
        strncpy(g_last_buf, buffer, sizeof(g_last_buf) - 1);
        g_done = 1;
        printf("  CONFIRMADO: \"%s\" cursor=%u\n", buffer, cursor);
        break;
    default:
        break;
    }
}

static void keyecho_iniciar(uint8_t max_chars,
                            const char *label_l1, const char *label_l2)
{
    k044_keyecho_cfg_t cfg;
    k044_keyecho_cfg_init(&cfg);
    cfg.row            = 1;
    cfg.col_start      = 0;
    cfg.max_chars      = max_chars;
    cfg.echo_enabled   = 1;
    cfg.clear_scancode = 0x76; /* ESC */

    g_kb_row       = cfg.row;
    g_kb_col_start = cfg.col_start;
    strncpy(g_label_l1, label_l1 ? label_l1 : "", sizeof(g_label_l1) - 1);
    strncpy(g_label_l2, label_l2 ? label_l2 : "", sizeof(g_label_l2) - 1);

    memset(g_last_buf, 0, sizeof(g_last_buf));
    g_done = 0;
    k044_write_display(label_l1, label_l2);
    k044_keyecho_start(&cfg, on_keyecho, NULL);
}

static int test_navegacao(void)
{
    int ok = 0;

    printf("\n=== %s ===\n", g_teste_nome);
    printf("1. Digite \"TESTE\" (5 letras).\n");
    printf("2. Seta <- <- move cursor sobre o 'S'\n");
    printf("3. BACKSPACE deleta o 'S' -> \"TETE\"\n");
    printf("4. Seta -> -> volta cursor ao fim\n");
    printf("5. Digite \"R\" -> \"TETER\"\n");
    printf("6. Enter confirma.\n\n");

    keyecho_iniciar(40, "Setas+BS teste:", "                                ");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    ok = (strcmp(g_last_buf, "TETER") == 0);
    printf("Resultado: \"%s\" %s\n", g_last_buf,
           ok ? "[PASS]" : "[FAIL] (esperado \"TETER\")");

    if (!ok)
        k044_write_display("FALHA: Navegacao", "veja o console");
    return ok;
}

static int test_insert_meio(void)
{
    int ok = 0;

    printf("\n=== %s ===\n", g_teste_nome);
    printf("1. Digite \"ABDEF\" (5 letras).\n");
    printf("2. Seta <- <- move cursor sobre 'D'\n");
    printf("3. Digite \"C\" -> \"ABCDEF\" (INSERT no meio)\n");
    printf("4. Enter confirma.\n\n");

    keyecho_iniciar(40, "Insert meio:", "                                ");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    ok = (strcmp(g_last_buf, "ABCDEF") == 0);
    printf("Resultado: \"%s\" %s\n", g_last_buf,
           ok ? "[PASS]" : "[FAIL] (esperado \"ABCDEF\")");

    if (!ok)
        k044_write_display("FALHA: Insert meio", "veja o console");
    return ok;
}

static int test_backspace_meio(void)
{
    int ok = 0;

    printf("\n=== %s ===\n", g_teste_nome);
    printf("1. Digite \"ABCXDEF\" (7 letras).\n");
    printf("2. Seta <- <- <- move cursor sobre 'X'\n");
    printf("3. BACKSPACE deleta o 'X' -> \"ABCDEF\"\n");
    printf("4. Enter confirma.\n\n");

    keyecho_iniciar(40, "BS meio:", "                                ");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    ok = (strcmp(g_last_buf, "ABCDEF") == 0);
    printf("Resultado: \"%s\" %s\n", g_last_buf,
           ok ? "[PASS]" : "[FAIL] (esperado \"ABCDEF\")");

    if (!ok)
        k044_write_display("FALHA: BS meio", "veja o console");
    return ok;
}

static int test_multiplos_inserts(void)
{
    int ok = 0;

    printf("\n=== %s ===\n", g_teste_nome);
    printf("1. Digite \"AC\" (2 letras).\n");
    printf("2. Seta <- volta cursor sobre 'C'\n");
    printf("3. Digite \"B\" -> \"ABC\" (1o INSERT)\n");
    printf("4. Digite \"DEF\" -> \"ABCDEF\" (APPEND)\n");
    printf("5. Seta <- <- <- volta cursor sobre 'D'\n");
    printf("6. Digite \"XY\" -> \"ABCXYDEF\" (INSERT multiplo)\n");
    printf("7. Enter confirma.\n\n");

    keyecho_iniciar(40, "Mult insert:", "                                ");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    ok = (strcmp(g_last_buf, "ABCXYDEF") == 0);
    printf("Resultado: \"%s\" %s\n", g_last_buf,
           ok ? "[PASS]" : "[FAIL] (esperado \"ABCXYDEF\")");

    if (!ok)
        k044_write_display("FALHA: Mult insert", "veja o console");
    return ok;
}

static int test_delete_meio(void)
{
    int ok = 0;

    printf("\n=== %s ===\n", g_teste_nome);
    printf("1. Digite \"ABCDEFG\" (7 letras).\n");
    printf("2. Seta <- <- <- posiciona cursor sobre 'D'\n");
    printf("3. DEL (Delete) apaga 'D' -> \"ABCEFG\"\n");
    printf("4. Seta -> -> posiciona cursor sobre 'G'\n");
    printf("5. DEL apaga 'G' -> \"ABCEF\" (cursor no fim)\n");
    printf("6. Enter confirma.\n\n");

    keyecho_iniciar(40, "DEL teste:", "                                ");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    ok = (strcmp(g_last_buf, "ABCEF") == 0);
    printf("Resultado: \"%s\" %s\n", g_last_buf,
           ok ? "[PASS]" : "[FAIL] (esperado \"ABCEF\")");

    if (!ok)
        k044_write_display("FALHA: DEL", "veja o console");
    return ok;
}

static int test_texto_longo(void)
{
    printf("\n=== %s ===\n", g_teste_nome);
    printf("Digite ate 40 caracteres. Use <- -> DEL BS.");
    printf(" Enter confirma. F1=fim.\n\n");

    keyecho_iniciar(40, "40 col teste:", "                                ");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    printf("Resultado (%zu chars): \"%s\"\n", strlen(g_last_buf), g_last_buf);
    return 1;
}

static int test_caps_lock(void)
{
    printf("\n=== %s ===\n", g_teste_nome);
    printf("1. Pressione CAPS LOCK (LED do teclado auxiliar deve acender).\n");
    printf("2. Digite \"teste\" — deve aparecer \"TESTE\" (maiusculo).\n");
    printf("3. Pressione SHIFT + \"A\" — deve aparecer 'a' (minusculo).\n");
    printf("4. Pressione CAPS LOCK novamente (LED deve apagar).\n");
    printf("5. Digite \"abc\" — deve aparecer \"abc\" (minusculo).\n");
    printf("6. Enter confirma.\n\n");

    keyecho_iniciar(40, "CapsLock LED+txt:", "                                ");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    printf("Resultado: \"%s\"\n", g_last_buf);
    return 1;
}

static void test_debounce(void)
{
    printf("\n--- Teste: Debounce temporal (5ms) ---\n");
    printf("Pressione a mesma tecla rapidamente 2 vezes.\n");
    printf("Se < 5ms entre os pressionamentos, o 2o e ignorado.\n");
    printf("Digite letras com pausa entre elas. Enter confirma.\n\n");

    keyecho_iniciar(40, "Debounce:", "Digite e Enter");

    while (!g_done) poll(NULL, 0, 50);
    k044_keyecho_stop();
    k044_cursor_off();

    printf("Resultado: \"%s\"\n", g_last_buf);
}

int main(void)
{
    if (k044_open() != K044_OK) {
        fprintf(stderr, "Falha ao abrir (requer sudo).\n");
        return 1;
    }

    k044_set_log_level(K044_LOG_DEBUG);
    k044_clear();

    {
        uint8_t maj = 0, min = 0, pat = 0;
        if (k044_read_fw_ver(&maj, &min, &pat) == K044_OK) {
            char ver[16];
            k044_fw_ver_str(ver, sizeof(ver));
            printf("Firmware: %s\n\n", ver);
        } else {
            printf("Firmware: NAO DISPONIVEL\n\n");
        }
    }

    k044_set_event_callback(on_raw_event, NULL);
    k044_write_display("Keyecho Nav+BS+DEL", "F1=fim F2=prox");

    int pass = 1;

    strncpy(g_teste_nome, "Teste 1: Navegacao", sizeof(g_teste_nome) - 1);
    if (!test_navegacao())         pass = 0;
    if (g_exit) goto fim;

    strncpy(g_teste_nome, "Teste 2: Insert meio", sizeof(g_teste_nome) - 1);
    if (!test_insert_meio())        pass = 0;
    if (g_exit) goto fim;

    strncpy(g_teste_nome, "Teste 3: BS meio", sizeof(g_teste_nome) - 1);
    if (!test_backspace_meio())     pass = 0;
    if (g_exit) goto fim;

    strncpy(g_teste_nome, "Teste 4: Mult insert", sizeof(g_teste_nome) - 1);
    if (!test_multiplos_inserts())  pass = 0;
    if (g_exit) goto fim;

    strncpy(g_teste_nome, "Teste 5: Delete meio", sizeof(g_teste_nome) - 1);
    if (!test_delete_meio())        pass = 0;
    if (g_exit) goto fim;

    strncpy(g_teste_nome, "Teste 6: CapsLock", sizeof(g_teste_nome) - 1);
    if (!test_caps_lock())          pass = 0;
    if (g_exit) goto fim;

    strncpy(g_teste_nome, "Teste 7: Texto longo", sizeof(g_teste_nome) - 1);
    if (!test_texto_longo())        pass = 0;
    if (g_exit) goto fim;

    test_debounce();
    if (g_exit) goto fim;

    k044_write_display("Varredura col:", "                                ");
    k044_cursor_on();
    for (int col = 0; col <= 15 && !g_exit; col++) {
        k044_set_cursor(1, col);
        k044_write_char('X');
        usleep(200000);
    }
    k044_set_cursor(1, 8);
    k044_write_char('|');
    k044_set_cursor(1, 10);
    k044_cursor_off();

    if (!pass) {
        k044_write_display("FALHA!", "veja o console");
        printf("\nATENCAO: falha detectada.\n");
    }

fim:
    k044_set_event_callback(NULL, NULL);
    k044_write_display(pass ? "Testes OK" : "Testes FALHA", g_exit ? "F1" : "");
    k044_clear();
    k044_close();
    printf("\nKeyecho Nav: %s\n", pass ? "TODOS PASS" : "ALGUEM FALHOU");
    return pass ? 0 : 1;
}