    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_clear();
        k044_write_pos(0, 0, "Valor: %d", 42);
        sleep(2);
    
        k044_write_pos(1, 0, "Hex: 0x%04X", 0xABCD);
        sleep(3);
    
        k044_clear();
        k044_close();
        return 0;
    }
