#define _POSIX_C_SOURCE 199309L
#include "display_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

static volatile int g_running = 1;

static void sigint_handler(int sig) {
    (void)sig;
    g_running = 0;
}

static void on_event(const k044_event_t *evt, void *userdata) {
    FILE *fp = (FILE *)userdata;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    switch (evt->type) {
    case K044_EVT_KEY_MAKE:
        fprintf(fp, "%lld.%06ld MAKE  0x%04X\n",
                (long long)ts.tv_sec, ts.tv_nsec / 1000,
                evt->scancode);
        fflush(fp);
        break;
    case K044_EVT_KEY_BREAK:
        fprintf(fp, "%lld.%06ld BREAK 0x%04X\n",
                (long long)ts.tv_sec, ts.tv_nsec / 1000,
                evt->scancode);
        fflush(fp);
        break;
    default:
        fprintf(fp, "%lld.%06ld EVENT type=%d raw=0x%02X\n",
                (long long)ts.tv_sec, ts.tv_nsec / 1000,
                evt->type, evt->raw_byte);
        fflush(fp);
        break;
    }
}

int main(int argc, char *argv[]) {
    const char *logfile = "captura_teclas.log";
    if (argc > 1) logfile = argv[1];

    signal(SIGINT, sigint_handler);

    FILE *fp = fopen(logfile, "w");
    if (!fp) {
        fprintf(stderr, "Erro ao abrir %s\n", logfile);
        return 1;
    }

    setbuf(fp, NULL);

    fprintf(fp, "# Captura de teclas TEC44AVT\n");
    fprintf(fp, "# Inicio: %s", ctime(&(time_t){time(NULL)}));
    fprintf(fp, "# Formato: timestamp MAKE/BREAK scancode\n\n");

    if (k044_open() != K044_OK) {
        fprintf(stderr, "Falha k044_open (requer sudo)\n");
        fclose(fp);
        return 1;
    }

    k044_set_event_callback(on_event, fp);
    k044_start_event_loop();

    printf("Capturando teclas para: %s\n", logfile);
    printf("Pressione Ctrl+C para encerrar.\n");

    while (g_running) {
        struct timespec ts = { 0, 100000000 };
        nanosleep(&ts, NULL);
    }

    k044_stop_event_loop();
    k044_close();

    fprintf(fp, "\n# Fim: %s", ctime(&(time_t){time(NULL)}));
    fclose(fp);

    printf("Captura salva em: %s\n", logfile);
    return 0;
}
