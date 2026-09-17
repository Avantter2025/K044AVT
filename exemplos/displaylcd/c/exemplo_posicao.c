#include "display_driver.h"
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("K044AVT — Exemplo Posicionamento (C)\n\n");

    if (k044_open() != K044_OK) {
        fprintf(stderr, "Falha ao abrir dispositivo (requer sudo).\n");
        return 1;
    }

    /* --- Demonstração de set_cursor --- */
    printf("1) Posicionamento por coordenadas (row=0..1, col=0..15)\n");
    k044_clear();

    /* Escreve caracteres em posições específicas */
    for (int col = 0; col < 15; col += 3) {
        k044_set_cursor(0, col);
        k044_write_char('0' + (col / 3));
    }
    for (int col = 0; col < 15; col += 3) {
        k044_set_cursor(1, col);
        k044_write_char('A' + (col / 3));
    }
    sleep(2);

    /* --- Home --- */
    printf("2) Home (cursor para 0,0)\n");
    k044_home();
    k044_write_string(">HOME<");
    sleep(2);

    /* --- Erase to EOL --- */
    printf("3) Erase to end of line\n");
    k044_clear();
    k044_write_display("AAAAAAAAAAAAAAAA", "BBBBBBBBBBBBBBBB");
    sleep(1);
    k044_set_cursor(0, 5);
    k044_erase_eol();  /* Apaga linha 1 a partir da coluna 5 */
    k044_set_cursor(1, 8);
    k044_erase_eol();  /* Apaga linha 2 a partir da coluna 8 */
    sleep(2);

    /* --- Cursor on/off --- */
    printf("4) Cursor visível / oculto\n");
    k044_clear();
    k044_write_line(0, "Cursor ON:");
    k044_set_cursor(1, 0);
    k044_cursor_on();
    sleep(2);
    k044_write_line(0, "Cursor OFF:");
    k044_cursor_off();
    sleep(2);
    k044_cursor_on();

    /* --- Scroll horizontal --- */
    printf("5) Scroll horizontal (texto > 15 chars)\n");
    k044_clear();
    k044_write_line(0, "Scroll abaixo:");
    k044_scroll_line(1, "Texto longo para demonstrar scroll horizontal K044", 200);
    sleep(1);

    /* --- Backspace e Tab --- */
    printf("6) Backspace e Tab\n");
    k044_clear();
    k044_set_cursor(0, 0);
    k044_write_string("ABCDEF");
    sleep(1);
    /* Move cursor para esquerda (sem apagar) */
    k044_cursor_dec();
    k044_write_char(' ');
    k044_cursor_dec();
    k044_cursor_dec();
    k044_write_char(' ');
    k044_write_line(1, "BS demo OK");
    sleep(2);

    /* --- write_line com padding automático --- */
    printf("7) write_line com padding automático\n");
    k044_write_line(0, "Curto");        /* Completa com espaços até 15 */
    k044_write_line(1, "Longo demais para caber aqui"); /* Truncado em 15 */
    sleep(2);

    k044_clear();
    k044_close();
    printf("Fim do exemplo de posicionamento.\n");
    return 0;
}
