    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_clear();
 
        k044_write_string("ABCDEFGHIJKLMNOPQRSTUVWYZ1234567890!@#$%¨&*()_abcdefghijklmnopqrstuvwyz*-+.ABCDEFGHIJKLMNOPQRSTUVWYZ1234567890!@#$%¨&*()_abcdefghijklmnopqrstuvwyz*-+.ABCDEFGHIJKLMNOPQRSTUVWYZ1234567890!@#$%¨&*()_abcdefghijklmnopqrstuvwyz*-+.ABCDEFGHIJKLMNOPQRSTUVWYZ1234567890!@#$%¨&*()_abcdefghijklmnopqrstuvwyz*-+.");
        sleep(3);
    
        k044_clear();
        k044_close();
        return 0;
    }
