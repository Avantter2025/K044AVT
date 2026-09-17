    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_clear();
    
        /* Scroll na linha 0 (delay 200ms entre frames) */
        k044_scroll_line(0, "SCROLL LINE 0 DEMO", 200);
        printf("Scroll na linha 0 por 5s\n");
        sleep(5);
    
        /* Scroll na linha 1 */
        k044_scroll_line(1, "SCROLL LINE 1 TEST", 200);
        printf("Scroll na linha 1 por 5s\n");
        sleep(5);
    
        k044_scroll_line(0, "", 0);  /* encerra scroll */
        k044_close();
        printf("Teste scroll concluido.\n");
        return 0;
    }
