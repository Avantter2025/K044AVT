/**
 * uart_diag.cpp — Diagnóstico de loopback da UART do módulo fingerprint
 *
 * Isola o subsistema UART do AT89S52 (Timer 2, 9600 baud, pinos TX_FP/
 * RX_FP) do restante do protocolo AS608. Uso pretendido: retire o módulo
 * AS608 e curto-circuite TX_FP com RX_FP diretamente no conector — sem o
 * sensor, tudo que o firmware transmitir por essa UART deve voltar
 * exatamente igual pela mesma UART.
 *
 * Isso testa uma hipótese diferente da falha "0xAD sempre RESEND" já
 * observada: aquele RESEND acontece na recepção do PRÓPRIO byte 0xAD pelo
 * PS/2 (rotina RECEBE, compartilhada por todos os comandos), antes do
 * firmware sequer entrar no handler RFP_CMD — ou seja, é um problema do
 * lado PS/2 (host↔AT89S52), não necessariamente da UART do sensor
 * (AT89S52↔AS608). Este teste verifica dois pontos:
 *
 *   1. O 0xAD continua sendo rejeitado com RESEND mesmo com o sensor
 *      removido e as linhas em loopback? Se sim, o problema independe
 *      totalmente do módulo AS608 (aponta para firmware/timing do próprio
 *      AT89S52 ou ruído elétrico no divisor de tensão do RX_FP).
 *   2. Se o 0xAD for aceito, os bytes que voltam pelo loopback batem
 *      exatamente com o que foi enviado? Divergências apontam para
 *      problema elétrico (divisor de tensão, baud rate, ruído) em vez de
 *      lógico.
 *
 * Usa k044_fp_raw_test() — API de diagnóstico de baixo nível que envia
 * bytes brutos via 0xAD e lê a resposta crua, sem qualquer parsing do
 * protocolo AS608 (ver display_driver.h).
 *
 * IMPORTANTE: FP_RXBUF (buffer circular do firmware) tem apenas 120 bytes
 * — os testes abaixo usam padrões menores que isso de propósito, para não
 * confundir corrupção real com o wraparound esperado de um buffer cheio.
 *
 * Build: make (nesta pasta); Uso: sudo ./uart_diag
 */

#include "../../../driver_display/display_driver.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

struct TestCase {
    const char *nome;
    const uint8_t *dados;
    int len;
};

static const uint8_t PAT_INCREMENTAL[] = {
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
    0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10
};
static const uint8_t PAT_ASCII[]      = { 'A','V','A','N','T','T','E','C' };
static const uint8_t PAT_WALKING_BIT[] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80
};
static const uint8_t PAT_ALL_ONES[]   = {
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

static const TestCase TESTES[] = {
    { "Incremental (16 bytes)",   PAT_INCREMENTAL,  (int)sizeof(PAT_INCREMENTAL)  },
    { "ASCII \"AVANTTEC\"",       PAT_ASCII,        (int)sizeof(PAT_ASCII)        },
    { "Walking bit (8 bytes)",    PAT_WALKING_BIT,  (int)sizeof(PAT_WALKING_BIT)  },
    { "All-ones 0xFF (8 bytes)",  PAT_ALL_ONES,     (int)sizeof(PAT_ALL_ONES)     },
};

static void hexdump(const char *label, const uint8_t *buf, int len)
{
    printf("  %-10s (%d bytes): ", label, len);
    for (int i = 0; i < len; i++) printf("%02X ", buf[i]);
    printf("\n");
}

/* Compara os bytes enviados contra a resposta recebida. Como o firmware
 * sempre acrescenta um terminador 0x00 ao final de toda resposta 0xAD
 * (ver RFP_CMD/RFP_RX_DONE), e nenhum padrao de teste aqui contem 0x00,
 * o ultimo byte recebido (se for exatamente 0x00) e o terminador — nao
 * conta como parte do eco. Qualquer 0x00 ANTES do fim, ou ausencia do
 * terminador, ja e sinal de problema. */
static int compara(const uint8_t *enviado, int len_env,
                    const uint8_t *recebido, int len_rec)
{
    int eco_len = len_rec;
    int tem_terminador = (len_rec > 0 && recebido[len_rec - 1] == 0x00);
    if (tem_terminador) eco_len = len_rec - 1;

    int erros = 0;
    int n = eco_len < len_env ? eco_len : len_env;
    for (int i = 0; i < n; i++) {
        if (recebido[i] != enviado[i]) {
            printf("    diff[%d]: enviado=0x%02X recebido=0x%02X\n",
                   i, enviado[i], recebido[i]);
            erros++;
        }
    }
    if (eco_len != len_env) {
        printf("    tamanho do eco (%d) != tamanho enviado (%d)%s\n",
               eco_len, len_env,
               tem_terminador ? "" : " (terminador 0x00 nao encontrado!)");
        erros++;
    }
    return erros == 0;
}

static int roda_teste(const TestCase &tc)
{
    printf("\n--- Teste: %s ---\n", tc.nome);
    hexdump("Enviado", tc.dados, tc.len);

    uint8_t resp[300];
    uint16_t resp_len = 0;
    int r = k044_fp_raw_test(tc.dados, (uint16_t)tc.len, resp, &resp_len, sizeof(resp));

    if (r != K044_OK) {
        printf("  FALHA: 0xAD rejeitado (r=%d) — nao chegou nem a enviar/ler o loopback.\n", r);
        printf("  => Isso indica que o problema NAO depende do modulo AS608/loopback:\n");
        printf("     o proprio comando 0xAD ja falha na recepcao PS/2, antes do\n");
        printf("     firmware tentar falar com a UART do sensor.\n");
        return 0;
    }

    hexdump("Recebido", resp, resp_len);
    int ok = compara(tc.dados, tc.len, resp, resp_len);
    printf("  => %s\n", ok ? "OK (loopback perfeito)" : "FALHA (loopback corrompido)");
    return ok;
}

int main(void)
{
    printf("=============================================\n");
    printf("  Diagnóstico de loopback UART (K044AVT)\n");
    printf("  libK044AVT %s\n", k044_version());
    printf("=============================================\n");
    printf("\nPRE-REQUISITO: sensor AS608 removido, TX_FP curto-circuitado\n");
    printf("com RX_FP diretamente no conector do módulo.\n");

    k044_set_log_level(K044_LOG_DEBUG);

    printf("\n>>> k044_open()...\n");
    int r = k044_open();
    if (r != K044_OK) {
        fprintf(stderr, "FALHA: k044_open() = %d (execute com sudo)\n", r);
        return 1;
    }
    printf("k044_open: OK\n");

    int n_tests = (int)(sizeof(TESTES) / sizeof(TESTES[0]));
    int n_pass = 0;
    for (int i = 0; i < n_tests; i++) {
        n_pass += roda_teste(TESTES[i]);
    }

    printf("\n=============================================\n");
    printf("  Testes: %d passaram de %d\n", n_pass, n_tests);
    if (n_pass == n_tests) {
        printf("  DIAGNÓSTICO: loopback da UART do fingerprint OK.\n");
        printf("  O 0xAD foi aceito e todos os padrões voltaram intactos —\n");
        printf("  a UART do AT89S52 (TX_FP/RX_FP, Timer 2 @ 9600 baud) está\n");
        printf("  saudável. Se o módulo AS608 real ainda falhar, suspeite\n");
        printf("  do próprio sensor, da alimentação dele, ou do divisor de\n");
        printf("  tensão no RX_FP ao reconectá-lo.\n");
    } else if (n_pass == 0) {
        printf("  DIAGNÓSTICO: 0xAD rejeitado (RESEND) mesmo em loopback.\n");
        printf("  Isso ISOLA o problema do módulo AS608 — a falha acontece\n");
        printf("  antes de qualquer dado tocar a UART do sensor, na recepção\n");
        printf("  PS/2 do próprio byte de comando 0xAD (rotina RECEBE do\n");
        printf("  firmware). Não é wiring do sensor nem o sensor em si.\n");
    } else {
        printf("  DIAGNÓSTICO: loopback parcialmente corrompido.\n");
        printf("  0xAD foi aceito, mas os dados voltaram alterados —\n");
        printf("  investigue o divisor de tensão 1k/2k2 no RX_FP, o baud\n");
        printf("  rate (9600 8N1) e a qualidade da conexão do loopback.\n");
    }
    printf("=============================================\n");

    k044_close();
    printf("Encerrado.\n");
    return n_pass == n_tests ? 0 : 1;
}
