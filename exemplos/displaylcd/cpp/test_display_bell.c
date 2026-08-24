    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_clear();
    
        /* Tab / cursor_inc */
        k044_write_string("A");
        k044_cursor_inc();
        k044_write_string("B");
        k044_cursor_inc();
        k044_write_string("C");
        printf("cursor_inc: A..B..C\n");
        sleep(2);
    
        /* cursor_dec + write */
        k044_set_cursor(1, 15);
        k044_write_string("XY");
        k044_cursor_dec();
        k044_write_char('Z');
        printf("cursor_dec: XYZ\n");
        sleep(2);
    
        /* Bell */
        k044_bell();
        printf("Bell — deve emitir bip\n");
        sleep(1);
    
        k044_clear();
        k044_close();
        printf("Teste cursor_inc/dec e bell concluido.\n");
        return 0;
    }
