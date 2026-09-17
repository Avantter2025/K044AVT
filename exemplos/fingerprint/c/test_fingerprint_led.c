#include "display_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

static const char *color_name(int color)
{
    switch (color) {
        case K044_FP_LED_COLOR_BLUE:  return "Azul";
        case K044_FP_LED_COLOR_GREEN: return "Verde";
        case K044_FP_LED_COLOR_RED:   return "Vermelho";
        default:                      return "?";
    }
}

static const char *func_label(int func)
{
    switch (func) {
        case K044_FP_LED_ALWAYS_ON:  return "aceso";
        case K044_FP_LED_ALWAYS_OFF: return "apagado";
        case K044_FP_LED_FLASHING:   return "rápido (piscando)";
        case K044_FP_LED_BREATHING:  return "respirando (lento)";
        default:                     return "?";
    }
}


static void aplicar_led(int func, int color)
{
    int r = k044_fp_led_config((uint8_t)func, (uint8_t)color, (uint8_t)color, 0);
    printf("  [log] k044_fp_led_config: ret=%d", r);
    if (r == K044_ERR_FP) {
        int cc = k044_fp_last_confirmation_code();
        printf(" (sensor: 0x%02X %s)", cc, k044_fp_confirmation_str(cc));
    }
    printf("\n");

    if (r == K044_OK)
        printf("  LED %s, cor %s.\n", func_label(func), color_name(color));
    else
        printf("  Falha ao configurar o LED (%d).\n", r);
}

static void menu_cor(int *color, int func)
{
    printf("\n  --- Selecionar cor do LED ---\n");
    printf("  1) Azul\n");
    printf("  2) Verde\n");
    printf("  3) Vermelho\n");
    printf("  Opção: ");
    fflush(stdout);

    char line[16];
    if (read_line(line, sizeof(line)) != 0) return;

    switch (line[0]) {
        case '1': *color = K044_FP_LED_COLOR_BLUE;  break;
        case '2': *color = K044_FP_LED_COLOR_GREEN; break;
        case '3': *color = K044_FP_LED_COLOR_RED;   break;
        default:
            printf("  Opção inválida.\n");
            return;
    }
    printf("  Cor selecionada: %s\n", color_name(*color));
    aplicar_led(func, *color);
}

static void menu_estado(int *func, int color)
{
    printf("\n  --- Selecionar estado do LED (cor atual: %s) ---\n", color_name(color));
    printf("  1) Aceso\n");
    printf("  2) Apagado\n");
    printf("  3) Rápido (piscando)\n");
    printf("  4) Respirando (lento)\n");
    printf("  Opção: ");
    fflush(stdout);

    char line[16];
    if (read_line(line, sizeof(line)) != 0) return;

    switch (line[0]) {
        case '1': *func = K044_FP_LED_ALWAYS_ON;  break;
        case '2': *func = K044_FP_LED_ALWAYS_OFF; break;
        case '3': *func = K044_FP_LED_FLASHING;   break;
        case '4': *func = K044_FP_LED_BREATHING;  break;
        default:
            printf("  Opção inválida.\n");
            return;
    }
    aplicar_led(*func, color);
}

static void menu_principal(int color, int func)
{
    printf("\n===== Teste do LED do sensor de digital (K044AVT) =====\n");
    printf("  Cor atual   : %s\n", color_name(color));
    printf("  Estado atual: %s\n", func_label(func));
    printf("  1) Selecionar cor (Azul / Verde / Vermelho)\n");
    printf("  2) Selecionar estado (Aceso / Apagado / Rápido / Respirando)\n");
    printf("  0) Sair\n");
    printf("Opção: ");
    fflush(stdout);
}

int main(void)
{
    printf("test_fingerprint_led — teste do LED do sensor de digital via libK044AVT %s\n",
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

    /* padrao */
    int color = K044_FP_LED_COLOR_BLUE;
    int func  = K044_FP_LED_ALWAYS_ON;

    for (;;) {
        menu_principal(color, func);
        char line[16];
        if (read_menu_choice(line, sizeof(line)) != 0) break;
        if (line[0] == '0') break;

        switch (line[0]) {
            case '1': menu_cor(&color, func);   break;
            case '2': menu_estado(&func, color); break;
            default:  printf("  Opção inválida.\n"); break;
        }
    }

    /* ---------------------------------------------- */
    /* Mantenha as chamadas como seguem abaixo        */ 
    /* como pratica para o processo terminar limpo.   */
    /* ---------------------------------------------  */

    k044_aux_disable();
    k044_mouse_disable();
    k044_uinput_disable();
    k044_close();
    
    printf("Encerrado.\n");
    return 0;
}
