#define _POSIX_C_SOURCE 199309L

#include "display_driver.h"

#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

/*
 * test_fingerprint_lock — mantém o teclado "bloqueado" (não repassado ao
 * sistema Linux) enquanto o sensor de digital não reconhece uma digital
 * já cadastrada no banco. Assim que uma digital válida é identificada, o
 * teclado é liberado (uinput/mouse/auxiliar habilitados) e passa a
 * funcionar normalmente.
 *
 * Importante: não existe um comando de baixo nível para desligar o
 * escaneamento do teclado principal (equivalente ao 0xF5 do protocolo
 * PS/2 padrão) exposto por esta biblioteca — o "bloqueio" aqui é feito no
 * nível de injeção no sistema: com k044_uinput_enable() não chamado, as
 * teclas continuam sendo capturadas internamente, mas nunca chegam ao
 * restante do Linux. É o mesmo princípio explicado no README do módulo.
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

int main(void)
{
    printf("test_fingerprint_lock — teclado bloqueado até identificação por digital (C) %s\n",
           k044_version());
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
    k044_write_pos(0, 0, "Acesso liberado ");
    k044_write_pos(1, 0, "ID: %u          ", found_id);

    if (k044_uinput_enable() != K044_OK)
        fprintf(stderr, "Aviso: uinput indisponivel (teclado nao sera repassado ao sistema).\n");
    if (k044_mouse_enable() != K044_OK)
        fprintf(stderr, "Aviso: mouse PS/2 indisponivel.\n");
    k044_start_event_loop();
    if (k044_aux_enable() != K044_OK)
        fprintf(stderr, "Aviso: teclado auxiliar PS/2 indisponivel.\n");

    printf("Teclado LIBERADO — a partir de agora as teclas (principal e\n");
    printf("auxiliar) sao digitadas de verdade neste terminal via uinput.\n");
    printf("O display e o event loop de leitura ficam livres (sem\n");
    printf("nenhuma escrita no barramento PS/2 daqui em diante), para nao\n");
    printf("disputar banda com o escaneamento do teclado. Ctrl+C para sair.\n\n");

    /* Nao chamamos k044_read_event()/escrevemos no display aqui: o
     * repasse ao uinput ja acontece sozinho na thread interna do event
     * loop (iniciada acima) independente disso. Qualquer transacao
     * extra nossa no barramento (leitura ou escrita) so disputaria
     * banda com o escaneamento do teclado e atrasaria a digitacao. */
    while (g_running) {
        struct timespec ts = {0, 200000000L}; /* 200 ms */
        nanosleep(&ts, NULL);
    }

    printf("\nCtrl+C recebido. Encerrando...\n");
    k044_aux_disable();
    k044_mouse_disable();
    k044_uinput_disable();
    k044_close();
    printf("Encerrado.\n");
    return 0;
}
