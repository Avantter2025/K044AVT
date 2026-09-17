#include "display_driver.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile int g_running = 1;

static void sigint_handler(int sig)
{
    (void)sig;
    g_running = 0;
}

/* Callback chamado pela thread interna do event loop */
static void on_event(const k044_event_t *evt, void *userdata)
{
    (void)userdata;

    switch (evt->type) {
    case K044_EVT_KEY_MAKE:
        printf("[MAKE ] scancode=0x%04X\n", evt->scancode);
        break;
    case K044_EVT_KEY_BREAK:
        printf("[BREAK] scancode=0x%04X\n", evt->scancode);
        break;
    case K044_EVT_PIN_KEY:
        printf("[PIN  ] scancode=0x%04X\n", evt->scancode);
        break;
    case K044_EVT_CARD_START:
        printf("[CARD ] Início de leitura de cartão\n");
        break;
    case K044_EVT_CARD_DATA:
        printf("[CARD ] Dado: 0x%02X\n", evt->raw_byte);
        break;
    case K044_EVT_CARD_END:
        printf("[CARD ] Leitura concluída com sucesso\n");
        break;
    case K044_EVT_CARD_ERROR:
        printf("[CARD ] Erro de leitura\n");
        break;
    case K044_EVT_RAW:
        printf("[RAW  ] byte=0x%02X\n", evt->raw_byte);
        break;
    }
}

int main(void)
{
    printf("K044AVT — Exemplo Eventos (C)\n");
    printf("Pressione teclas no TEC44. Ctrl+C para sair.\n\n");

    signal(SIGINT, sigint_handler);

    if (k044_open() != K044_OK) {
        fprintf(stderr, "Falha ao abrir (requer sudo).\n");
        return 1;
    }

    k044_clear();
    k044_write_display("Eventos ativos", "Pressione tecla");

    /* Registra callback e inicia thread de eventos */
    k044_set_event_callback(on_event, NULL);
    k044_start_event_loop();

    printf("Event loop iniciado. Aguardando eventos...\n");

    /* Aguarda até Ctrl+C ou pode também usar k044_read_event em loop */
    while (g_running) {
        k044_event_t evt;
        int r = k044_read_event(&evt, 500);  /* timeout 500 ms */
        if (r == K044_OK) {
            /* Evento já processado pelo callback — mostrar também no display */
            if (evt.type == K044_EVT_KEY_MAKE) {
                k044_write_pos(1, 0, "SC=0x%04X   ", evt.scancode);
            }
        }
    }

    printf("\nCtrl+C recebido. Encerrando...\n");
    k044_stop_event_loop();
    k044_clear();
    k044_close();
    return 0;
}
