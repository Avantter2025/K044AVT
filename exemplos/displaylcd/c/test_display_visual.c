    #include <stdio.h>
    #include <unistd.h>
    #include "display_driver.h"
    
    int main() {
        if (k044_open() != K044_OK) return 1;
    
        k044_clear();
        k044_write_string("ABCDEFGHIJ");
        sleep(1);
    
        /* home */
        k044_home();
        k044_write_string(">>>");
        printf("home + escreveu >>>\n");
        sleep(2);
    
        /* carriage_return */
        k044_set_cursor(0, 5);
        k044_carriage_return();
        k044_write_string("CR");
        printf("carriage_return na col 5\n");
        sleep(2);
    
        /* line_feed */
        k044_line_feed();
        k044_write_string("LF");
        printf("line_feed + escreveu LF\n");
        sleep(2);
    
        /* cursor_up */
        k044_cursor_up();
        k044_write_string("^");
        printf("cursor_up + escreveu ^\n");
        sleep(2);
    
        /* backspace */
        k044_set_cursor(1, 5);
        k044_write_string("ABCDE");
        k044_cursor_dec();
        k044_cursor_dec();
        k044_write_string("--");
        printf("backspace 2x + escreveu --\n");
        sleep(2);
    
        /* erase_eol */
        k044_set_cursor(1, 3);
        k044_erase_eol();
        printf("erase_eol da col 3 ate o fim\n");
        sleep(2);
    
        k044_clear();
        k044_close();
        printf("Teste controle de tela concluido.\n");
        return 0;
    }
