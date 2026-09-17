#define _DEFAULT_SOURCE
#include "display_driver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Estado compartilhado entre main e callback */
static volatile int g_done   = 0;
static char         g_input[16] = {0};

/* =========================================================================
 * Callback keyecho — chamado pela thread interna a cada evento
 * ========================================================================= */
static void on_keyecho(k044_keyecho_event_t event,
                        const char *buffer, uint8_t len,
                        char last_char, void *userdata)
{
    (void)userdata;

    switch (event) {
    case K044_KEYECHO_CHAR:
        /* Char adicionado — buffer atualizado no display automaticamente */
        printf("  Tecla: '%c'  Buffer: \"%.*s\"\n",
               last_char, (int)len, buffer);
        break;

    case K044_KEYECHO_BACKSPACE:
        printf("  [BACKSPACE] Buffer: \"%.*s\"\n", (int)len, buffer);
        break;

    case K044_KEYECHO_DELETE:
        /* Delete (K044_SC_DEL = 0xE071, o Delete do bloco de edicao do
         * teclado auxiliar) remove o caractere SOB o cursor, ao contrario
         * do Backspace acima, que remove o da esquerda. */
        printf("  [DELETE] Buffer: \"%.*s\"\n", (int)len, buffer);
        break;

    case K044_KEYECHO_CLEARED:
        printf("  [LIMPO] Buffer zerado\n");
        break;

    case K044_KEYECHO_CONFIRMED:
        /* Enter pressionado — copiar resultado e sinalizar main */
        strncpy(g_input, buffer, sizeof(g_input) - 1);
        g_done = 1;
        printf("  [CONFIRMADO] Entrada: \"%s\" (%d chars)\n",
               buffer, (int)len);
        break;

    case K044_KEYECHO_FULL:
        printf("  [CHEIO] Máximo de caracteres atingido — tecla ignorada\n");
        break;
    }
}

/* =========================================================================
 * Fase 1: entrada de texto normal (max 10 chars)
 * ========================================================================= */
static void demo_texto_normal(void)
{
    printf("\n--- Fase 1: Entrada de texto normal (máx 10 chars) ---\n");
    printf("Digite até 10 caracteres e pressione Enter para confirmar.\n");
    printf("Backspace remove ultimo char. Esc limpa tudo.\n\n");

    k044_write_display("Digite (max 10):", "                ");

    k044_keyecho_cfg_t cfg;
    k044_keyecho_cfg_init(&cfg);
    cfg.row             = 1;
    cfg.col_start       = 0;
    cfg.max_chars       = 10;
    cfg.clear_scancode  = K044_SC_ESCAPE;   /* Esc limpa */
    cfg.pin_mode        = 0;
    cfg.echo_enabled    = 1;

    g_done = 0;
    k044_keyecho_start(&cfg, on_keyecho, NULL);

    /* Aguarda confirmação */
    while (!g_done) usleep(50000);

    k044_keyecho_stop();
    printf("Resultado capturado: \"%s\"\n", g_input);
}

/* =========================================================================
 * Fase 2: modo PIN (asteriscos no display, char real no buffer)
 * ========================================================================= */
static void demo_pin(void)
{
    printf("\n--- Fase 2: Modo PIN (exibe '*' no display) ---\n");
    printf("Digite um PIN de até 6 dígitos e pressione Enter.\n\n");

    k044_pin_enable();
    k044_write_display("Digite seu PIN:", "                ");

    k044_keyecho_cfg_t cfg;
    k044_keyecho_cfg_init(&cfg);
    cfg.row             = 1;
    cfg.col_start       = 0;
    cfg.max_chars       = 6;
    cfg.clear_scancode  = K044_SC_ESCAPE;   /* Esc limpa */
    cfg.pin_mode        = 1;   /* Exibe '*' no display */
    cfg.echo_enabled    = 1;

    g_done = 0;
    memset(g_input, 0, sizeof(g_input));
    k044_keyecho_start(&cfg, on_keyecho, NULL);

    while (!g_done) usleep(50000);

    k044_keyecho_stop();
    k044_pin_disable();

    /* PIN real disponível no buffer (não exibido no display) */
    printf("PIN capturado (NÃO exibido no display): \"%s\"\n", g_input);
}

/* =========================================================================
 * Fase 3: tecla de limpeza personalizada (Esc = 0x76)
 * ========================================================================= */
static void demo_clear_key(void)
{
    printf("\n--- Fase 3: Tecla de limpeza personalizada (Esc) ---\n");
    printf("Digite texto. Pressione Esc para limpar. Enter para confirmar.\n\n");

    k044_write_display("Esc=limpar      ", "                ");

    k044_keyecho_cfg_t cfg;
    k044_keyecho_cfg_init(&cfg);
    cfg.row              = 1;
    cfg.col_start        = 0;
    cfg.max_chars        = 15;
    cfg.clear_scancode   = K044_SC_ESCAPE;   /* Esc limpa o buffer */
    cfg.confirm_scancode = K044_SC_ENTER;    /* Enter confirma */
    cfg.pin_mode         = 0;

    g_done = 0;
    memset(g_input, 0, sizeof(g_input));
    k044_keyecho_start(&cfg, on_keyecho, NULL);

    while (!g_done) usleep(50000);

    k044_keyecho_stop();
    printf("Resultado: \"%s\"\n", g_input);
}

/* =========================================================================
 * main
 * ========================================================================= */
int main(void)
{
    printf("K044AVT — Exemplo KeyEcho (C)\n");
    printf("Rotina de exibição de teclas no display LCD\n\n");

    if (k044_open() != K044_OK) {
        fprintf(stderr, "Falha ao abrir (requer sudo).\n");
        return 1;
    }

    k044_clear();
    k044_write_display("K044AVT KeyEcho", "Iniciando...");
    sleep(1);

    demo_texto_normal();
    sleep(1);

    demo_pin();
    sleep(1);

    demo_clear_key();
    sleep(1);

    /* Resumo final no display */
    k044_write_display("KeyEcho OK!", "Demos concluidas");
    sleep(2);

    k044_clear();
    k044_close();
    printf("\nFim do exemplo KeyEcho.\n");
    return 0;
}
