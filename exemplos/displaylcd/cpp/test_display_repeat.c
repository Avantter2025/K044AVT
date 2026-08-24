    #include <stdio.h>
    #include <unistd.h>
    #include <poll.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_write_display("Teste repeticao  ", "Iteracao          ");
        for (int i = 0; i < 100; i++) {
            k044_write_pos(1, 9, "%3d", i);
            poll(NULL, 0, 80);  /* 80ms */
        }
    
        k044_write_display("Resiliencia OK!", "100 iteracoes    ");
        sleep(2);
        k044_clear();
        k044_close();
        printf("Teste resiliencia concluido.\n");
        return 0;
    }
