    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_clear();
        k044_write_string("Cursor On/Off");
        sleep(1);
    
        k044_set_cursor(1, 0);
        k044_cursor_on();
        printf("Cursor ON — deve piscar\n");
        sleep(3);
    
        k044_cursor_off();
        printf("Cursor OFF — cursor sumiu\n");
        sleep(3);
    
        k044_cursor_on();
        printf("Cursor ON novamente\n");
        sleep(3);
    
        k044_clear();
        k044_close();
        printf("Teste cursor on/off concluido.\n");
        return 0;
    }
