    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
    
        k044_set_log_level(K044_LOG_DEBUG);
      
        if (k044_open() != K044_OK) {
            fprintf(stderr, "Falha ao abrir (requer sudo)\n");
            return 1;
        }
    
        printf("k044_open OK\n");
    
        /* Teste 1: clear */
        k044_clear();
        printf("k044_clear OK\n");
        sleep(4);
    
        /* Teste 2: write_char */
        k044_write_char('A');
        printf("k044_write_char 'A' no canto (0,0)\n");
        sleep(1);
    
        /* Teste 3: write_string */
        k044_clear();
        k044_write_string("Hello World");
        printf("k044_write_string \"Hello World\"\n");
        sleep(2);
    
        /* Teste 4: write_line */
        k044_write_line(0, "Linha 0");
        k044_write_line(1, "Linha 1");
        printf("k044_write_line: duas linhas\n");
        sleep(2);
    
        /* Teste 5: write_display */
        k044_write_display("POS Avanttec    ", "TEC44FST v1.0    ");
        printf("k044_write_display: duas linhas\n");
        sleep(2);
    
        k044_clear();
        k044_close();
        printf("Teste basico concluido.\n");
        return 0;
    }
