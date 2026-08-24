    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_clear();
    
        /* Escrever moldura */
        k044_write_display("0         1         2         3", "0123456789012345678901234567890123456789");
        sleep(1);
    
        /* Posicionar e escrever marcadores */
        k044_set_cursor(0, 0);  k044_write_char('[');
        k044_set_cursor(0, 39); k044_write_char(']');
        k044_set_cursor(1, 0);  k044_write_char('<');
        k044_set_cursor(1, 39); k044_write_char('>');
        printf("Moldura com cantos marcados\n");
        sleep(2);
    
        /* Escrever no centro */
        k044_set_cursor(0, 18); k044_write_string("CENTER");
        k044_set_cursor(1, 15); k044_write_string("LINHA 1 CENTRO");
        printf("Texto centralizado\n");
        sleep(2);
    
        /* Testar ranges */
        int r1 = k044_set_cursor(2, 0);   /* row > 1 */
        int r2 = k044_set_cursor(0, 40);  /* col > 39 */
        printf("set_cursor(2,0)   = %d (esperado %d)\n", r1, K044_ERR_RANGE);
        printf("set_cursor(0,40)  = %d (esperado %d)\n", r2, K044_ERR_RANGE);
    
        k044_clear();
        k044_close();
        printf("Teste cursor concluido.\n");
        return 0;
    }
