/**
 * fp_raw_probe.cpp — Diagnostico raw de um comando AS608 real (nao loopback)
 *
 * Manda PS_GetImage (0x01) formatado como pacote AS608 real e imprime
 * os bytes crus da resposta, sem o parser de fp_read_framed (que exige
 * >=10 bytes e header valido para nao retornar 0xFF generico).
 *
 * Build: make fp_raw_probe (nesta pasta); Uso: sudo ./fp_raw_probe
 */

#include "../../../driver_display/display_driver.h"

#include <cstdio>
#include <cstdint>
#include <cstring>

int main(void)
{
    printf("=== fp_raw_probe: PS_GetImage cru (nao loopback) ===\n\n");

    int rc = k044_open();
    if (rc != K044_OK) {
        printf("k044_open falhou: %d\n", rc);
        return 1;
    }
    printf("k044_open: OK\n\n");

    /* Pacote AS608 real: EF 01 (header) FF FF FF FF (addr) 01 (PID=cmd)
     * 00 03 (length=3: 1 instr + 2 checksum) <instr>
     * checksum = PID+lenHi+lenLo+instr, 2 bytes */
    static const uint8_t PS_GET_IMAGE[] = {
        0xEF,0x01, 0xFF,0xFF,0xFF,0xFF, 0x01, 0x00,0x03, 0x01, 0x00,0x05
    };
    /* PS_ReadSysPara (0x0F): comando leve, NAO liga o sensor optico/LED -
     * usado para separar "sensor sem energia suficiente pro GetImage"
     * de um problema de comunicacao mais fundamental.
     * checksum = 1+0+3+0x0F = 0x13 */
    static const uint8_t PS_READ_SYS_PARA[] = {
        0xEF,0x01, 0xFF,0xFF,0xFF,0xFF, 0x01, 0x00,0x03, 0x0F, 0x00,0x13
    };
    /* PS_ControlBLN (0x3C): mantem o anel do LED em verde solido durante o
     * diagnostico (func=03H sempre aceso, color=02H verde bit1) - mesmo
     * pacote ja validado em firmware-3/LEDGRN.ASM contra o hardware real.
     * checksum = soma(01,00,07,3C,03,02,02,00) = 004BH */
    static const uint8_t PS_CONTROL_BLN_GREEN[] = {
        0xEF,0x01, 0xFF,0xFF,0xFF,0xFF, 0x01, 0x00,0x07, 0x3C,0x03,0x02,0x02,0x00, 0x00,0x4B
    };

    /* Mantem o anel verde durante todo o diagnostico (nao valida resposta -
     * so' um indicador visual de que o modulo esta vivo/respondendo). */
    {
        uint8_t resp[256];
        uint16_t resp_len = 0;
        k044_fp_raw_test(PS_CONTROL_BLN_GREEN, sizeof(PS_CONTROL_BLN_GREEN),
                          resp, &resp_len, sizeof(resp));
    }

    /* Ordem invertida (GetImage primeiro) para separar "corrupcao segue o
     * primeiro comando da sessao" de "corrupcao segue o comando/tamanho
     * de resposta especifico". */
    struct { const char *nome; const uint8_t *pkt; size_t len; } testes[] = {
        { "PS_GetImage (pesado, liga optica)", PS_GET_IMAGE,     sizeof(PS_GET_IMAGE) },
        { "PS_ReadSysPara (leve, sem optica)", PS_READ_SYS_PARA, sizeof(PS_READ_SYS_PARA) },
    };

    for (auto &t : testes) {
        printf("--- %s ---\n", t.nome);
        uint8_t resp[256];
        uint16_t resp_len = 0;
        memset(resp, 0, sizeof(resp));

        printf("Enviado (%zu bytes): ", t.len);
        for (size_t i = 0; i < t.len; i++) printf("%02X ", t.pkt[i]);
        printf("\n");

        int r = k044_fp_raw_test(t.pkt, t.len, resp, &resp_len, sizeof(resp));
        printf("k044_fp_raw_test: ret=%d  resp_len=%u\n", r, resp_len);
        printf("Recebido (%u bytes): ", resp_len);
        for (uint16_t i = 0; i < resp_len; i++) printf("%02X ", resp[i]);
        printf("\n");

        if (resp_len == 0) {
            printf("-> 0 bytes de volta\n");
        } else if (resp_len < 12) {
            printf("-> truncada (< 12 bytes esperados de um ACK AS608)\n");
        } else if (resp[0] == 0xEF && resp[1] == 0x01) {
            printf("-> header AS608 valido! PID=0x%02X  confirmation code=0x%02X\n", resp[6], resp[9]);
        } else {
            printf("-> bytes recebidos mas sem header AS608 valido (EF 01)\n");
        }
        printf("\n");
    }

    /* PS_ReadINFpage (0x16): comando multi-pacote (ACK so' confirma "dados a
     * seguir"; os dados de verdade vem depois, em pacotes PID=02/08 - ja
     * confirmado com captura direta via CH340G que o sensor responde certo
     * a esse comando). Testado aqui com fp_read_raw (sem validar header/
     * resync, ao contrario de fp_read_framed usado por k044_fp_read_nfpage)
     * pra ver os bytes crus que chegam pela via AT89S52/PS2, ja que
     * k044_fp_read_nfpage vem retornando "0 bytes" (ret=-8) - fp_read_raw
     * mostra ate' bytes de lixo/fora de ordem que fp_read_framed descartaria
     * silenciosamente no resync.
     * checksum = 1+0+3+0x16 = 0x1A */
    {
        static const uint8_t PS_READ_INF_PAGE[] = {
            0xEF,0x01, 0xFF,0xFF,0xFF,0xFF, 0x01, 0x00,0x03, 0x16, 0x00,0x1A
        };
        printf("--- PS_ReadINFpage (0x16, so' o ACK - multi-pacote) ---\n");
        uint8_t resp[256];
        uint16_t resp_len = 0;
        memset(resp, 0, sizeof(resp));

        printf("Enviado (%zu bytes): ", sizeof(PS_READ_INF_PAGE));
        for (size_t i = 0; i < sizeof(PS_READ_INF_PAGE); i++) printf("%02X ", PS_READ_INF_PAGE[i]);
        printf("\n");

        int r = k044_fp_raw_test(PS_READ_INF_PAGE, sizeof(PS_READ_INF_PAGE), resp, &resp_len, sizeof(resp));
        printf("k044_fp_raw_test: ret=%d  resp_len=%u\n", r, resp_len);
        printf("Recebido (%u bytes): ", resp_len);
        for (uint16_t i = 0; i < resp_len; i++) printf("%02X ", resp[i]);
        printf("\n\n");
    }

    k044_close();
    return 0;
}
