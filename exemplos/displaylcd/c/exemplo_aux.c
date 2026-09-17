#define _DEFAULT_SOURCE
#include "display_driver.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

static volatile int g_running = 1;

static void on_sigint(int sig)
{
    (void)sig;
    g_running = 0;
}

/* Tecla SAIR (0x76 — ESC no teclado de 44 teclas) encerra o teste. */
#define SC_SAIR 0x76

static uint64_t g_evt_count = 0;

static void event_callback(const k044_event_t *evt, void *userdata)
{
    (void)userdata;
    g_evt_count++;
    printf("[%03llu] type=%-5s sc=0x%04X raw=0x%02X ts=%llu\n",
           (unsigned long long)g_evt_count,
           evt->type == 0 ? "MAKE" :
           evt->type == 1 ? "BREAK" : "?",
           evt->scancode,
           evt->raw_byte,
           (unsigned long long)evt->timestamp_ns);
    fflush(stdout);

    if (evt->type == K044_EVT_KEY_MAKE && evt->scancode == SC_SAIR) {
        printf("\n[SAIR pressionado — encerrando]\n");
        g_running = 0;
    }
}

int main(void)
{
    signal(SIGINT, on_sigint);

    printf("K044AVT — Exemplo Teclado Auxiliar\n\n");

    if (k044_open() != K044_OK) {
        fprintf(stderr, "Falha ao abrir (requer sudo).\n");
        return 1;
    }

    k044_clear();
    k044_write_display("Auxiliar ativo", "  scs no log   ");

    printf("=== Teclado auxiliar HABILITADO ===\n");
    printf("Digite no teclado auxiliar ou nas 44 teclas — cada MAKE/BREAK\n");
    printf("aparece abaixo com scancode e raw_byte, sem depender do\n");
    printf("servidor HTTP/JS/uinput (isola driver de app).\n\n");
    printf("Teste sugerido: pressione algumas teclas do auxiliar (deve\n");
    printf("logar normalmente), depois pressione CAPS LOCK (sc=0x58) e\n");
    printf("pressione as MESMAS teclas do auxiliar de novo — se pararem\n");
    printf("de aparecer no log apos o Caps Lock, o problema esta aqui\n");
    printf("na biblioteca (nao no server.c/JS/uinput).\n\n");
    printf("Pressione SAIR (Esc, sc=0x76) no teclado ou Ctrl+C no\n");
    printf("terminal para encerrar.\n\n");

    k044_set_event_callback(event_callback, NULL);
    k044_start_event_loop();

    while (g_running) usleep(100000);

    k044_stop_event_loop();

    printf("\n=== Desabilitando teclado auxiliar... ===\n");
    k044_aux_disable();
    k044_write_display("Auxiliar OFF   ", "                ");
    sleep(1);

    printf("=== Reabilitando teclado auxiliar... ===\n");
    k044_aux_enable();
    k044_write_display("Auxiliar ON    ", "                ");
    sleep(1);

    k044_clear();
    k044_close();
    printf("\nFim do exemplo.\n");
    return 0;
}
