#define _POSIX_C_SOURCE 199309L
#include "display_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    printf("Diagnostico LCD — passo a passo\n\n");

    printf("Tentando k044_open_ex com timeout 2000ms...\n");

    k044_config_t cfg;
    k044_config_init(&cfg);
    cfg.read_timeout_ms = 2000;
    cfg.log_level = K044_LOG_NONE;

    int r = k044_open_ex(&cfg);
    if (r != K044_OK) {
        fprintf(stderr, "Erro ao abrir: %d\n", r);
        return 1;
    }
    printf("=== ABERTO ===\n\n");

    /* Passo 1: apenas clear */
    printf("Passo 1: clear...\n");
    k044_clear();
    sleep(3);
    printf("(3s — display em branco?)\n\n");

    /* Passo 2: escreve 'X' no canto (0,0) */
    printf("Passo 2: escrevendo 'X' em (0,0)...\n");
    k044_set_cursor(0, 0);
    k044_write_string("X");
    sleep(3);
    printf("(3s — viu um X no canto superior esquerdo?)\n\n");

    /* Passo 3: escreve na linha 2 */
    printf("Passo 3: escrevendo 'Y' em (1,0)...\n");
    k044_set_cursor(1, 0);
    k044_write_string("Y");
    sleep(3);
    printf("(3s — viu um Y no canto inferior esquerdo?)\n\n");

    /* Passo 4: clear + texto completo */
    printf("Passo 4: clear + 'Linha 1' / 'Linha 2'...\n");
    k044_clear();
    k044_set_cursor(0, 0);
    k044_write_string("Linha 1");
    k044_set_cursor(1, 0);
    k044_write_string("Linha 2");
    sleep(5);
    printf("(5s — viu 'Linha 1' e 'Linha 2'?)\n\n");

    /* Passo 5: testa coluna 39 da linha 1 */
    printf("Passo 5: escrevendo '!' na coluna 39 da linha 1...\n");
    k044_set_cursor(0, 39);
    k044_write_string("!");
    sleep(3);
    printf("(3s — viu '!' no canto direito da linha 1?)\n\n");

    /* Passo 6: testa coluna 39 da linha 2 */
    printf("Passo 6: escrevendo 'Z' na coluna 39 da linha 2...\n");
    k044_set_cursor(1, 39);
    k044_write_string("Z");
    sleep(3);
    printf("(3s — viu 'Z' no canto direito da linha 2?)\n\n");

    printf("Encerrando...\n");
    k044_clear();
    sleep(1);
    k044_close();
    printf("Fim.\n");
    return 0;
}
