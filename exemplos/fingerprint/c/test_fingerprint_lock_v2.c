#define _POSIX_C_SOURCE 199309L

#include "display_driver.h"

#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

/*
 * test_fingerprint_lock_v2 — mesma ideia do test_fingerprint_lock (teclado
 * "bloqueado" até uma digital cadastrada ser reconhecida), mas usando
 * k044_keyecho_start() para mostrar as teclas digitadas no display depois
 * de liberado, em vez de k044_read_event()+k044_write_pos() manual.
 *
 * Motivo da mudança: k044_write_pos() é a rotina genérica de escrita —
 * cada chamada é uma transação PS/2 própria, com retentativas e (em caso
 * de colisão) o procedimento completo de recovery, pensada para uso
 * esporádico, não para ser chamada a cada tecla. Já k044_keyecho_start()
 * roda numa thread dedicada, acordada diretamente pelo event loop a cada
 * tecla, e usa uma via interna otimizada (sem o recovery completo, com
 * redesenho em lote) — é o mesmo mecanismo que faz test_display_keyecho_nav
 * (em exemplos/displaylcd/c/) responder bem com teclado e display ativos
 * ao mesmo tempo.
 *
 * Use um dedo já cadastrado (ver test_fingerprint, opção 3 — Cadastrar
 * digital) antes de rodar este exemplo.
 */

static volatile int g_running = 1;

static void sigint_handler(int sig)
{
    (void)sig;
    g_running = 0;
}

/* Callback do keyecho: roda na thread dedicada do keyecho, já DEPOIS da
 * escrita no display ter sido feita internamente pela biblioteca — aqui
 * só reagimos ao resultado (printf/estado local), sem nenhuma chamada de
 * volta ao barramento PS/2. */
static void on_keyecho(k044_keyecho_event_t event, const char *buffer,
                        uint8_t len, char last_char, void *userdata)
{
    (void)userdata;
    switch (event) {
    case K044_KEYECHO_CHAR:
        printf("  [tecla] '%c'  buffer=\"%.*s\"\n", last_char, (int)len, buffer);
        break;
    case K044_KEYECHO_BACKSPACE:
        printf("  [backspace]  buffer=\"%.*s\"\n", (int)len, buffer);
        break;
    case K044_KEYECHO_CLEARED:
        printf("  [limpo] (ESC)\n");
        break;
    case K044_KEYECHO_CONFIRMED:
        printf("  [confirmado] \"%.*s\"\n", (int)len, buffer);
        break;
    case K044_KEYECHO_FULL:
        printf("  [cheio] limite de caracteres atingido\n");
        break;
    default:
        break;
    }
}

int main(void)
{
    printf("test_fingerprint_lock_v2 — teclado bloqueado ate identificacao por digital,\n");
    printf("com eco de tecla via k044_keyecho_start() (C) %s\n", k044_version());
    printf("Ctrl+C a qualquer momento para cancelar.\n\n");

    signal(SIGINT, sigint_handler);

    int r = k044_open();
    if (r != K044_OK) {
        fprintf(stderr, "Falha ao abrir o dispositivo (%d). Execute com sudo.\n", r);
        return 1;
    }

    /* Propositalmente NÃO habilita uinput/mouse/aux/event_loop ainda —
     * o teclado fica capturado pela biblioteca, mas invisível ao resto
     * do sistema, até a digital ser reconhecida. */
    k044_clear();
    k044_write_display("Teclado bloqueado", "Aproxime o dedo");

    if (k044_fp_init() != K044_OK) {
        fprintf(stderr, "Módulo de digital não respondeu (k044_fp_init).\n");
        k044_write_display("Erro: sensor de", "digital ausente");
        k044_close();
        return 1;
    }

    printf("Teclado BLOQUEADO — nenhuma tecla será repassada ao sistema.\n");
    printf("Aguardando uma digital cadastrada no sensor...\n\n");

    int unlocked = 0;
    int attempt = 0;
    uint16_t found_id = 0xFFFF;

    while (g_running && !unlocked) {
        attempt++;
        printf("  [tentativa %d] aproxime o dedo no sensor (5s)...\n", attempt);

        found_id = 0xFFFF;
        r = k044_fp_search_retry(1, 0, 1000, 5000, 1, &found_id);

        if (r == K044_OK && found_id != 0xFFFF) {
            unlocked = 1;
            break;
        }

        if (r == K044_ERR_FP_TIMEOUT)
            printf("  Nenhum dedo detectado, tentando de novo...\n");
        else
            printf("  Digital não reconhecida (%d), tentando de novo...\n", r);

        if (attempt % 3 == 0)
            printf("  Dica: use test_fingerprint (opção 3) para cadastrar uma digital primeiro.\n");
    }

    if (!unlocked) {
        printf("\nCancelado antes de reconhecer uma digital.\n");
        k044_write_display("Cancelado", "");
        k044_close();
        return 0;
    }

    printf("\nDigital reconhecida! ID=%u — liberando o teclado...\n", found_id);

    if (k044_uinput_enable() != K044_OK)
        fprintf(stderr, "Aviso: uinput indisponivel (teclado nao sera repassado ao sistema).\n");
    if (k044_mouse_enable() != K044_OK)
        fprintf(stderr, "Aviso: mouse PS/2 indisponivel.\n");
    k044_start_event_loop();
    if (k044_aux_enable() != K044_OK)
        fprintf(stderr, "Aviso: teclado auxiliar PS/2 indisponivel.\n");

    /* Eco de tecla rápido e correto: motor interno dedicado, não a nossa
     * própria chamada de escrita a cada tecla. */
    k044_keyecho_cfg_t cfg;
    k044_keyecho_cfg_init(&cfg);
    cfg.row            = 1;
    cfg.col_start      = 0;
    cfg.max_chars      = 40;
    cfg.echo_enabled   = 1;
    cfg.clear_scancode = 0x76; /* ESC limpa o campo */

    k044_write_display("Teclado liberado", "");
    if (k044_keyecho_start(&cfg, on_keyecho, NULL) != K044_OK)
        fprintf(stderr, "Aviso: keyecho nao pode ser iniciado.\n");

    printf("Teclado LIBERADO — digite normalmente (linha 2 do display\n");
    printf("mostra o eco em tempo real). ESC limpa, Enter confirma.\n");
    printf("Ctrl+C para sair.\n\n");

    /* Nada de k044_read_event()/escrita manual aqui: o eco já é feito
     * pela thread interna do keyecho, e o repasse ao uinput pela thread
     * do event loop — ambos independentes deste loop, que só espera o
     * Ctrl+C. */
    while (g_running) {
        struct timespec ts = {0, 200000000L}; /* 200 ms */
        nanosleep(&ts, NULL);
    }

    printf("\nCtrl+C recebido. Encerrando...\n");
    k044_keyecho_stop();
    k044_aux_disable();
    k044_mouse_disable();
    k044_uinput_disable();
    k044_close();
    printf("Encerrado.\n");
    return 0;
}
