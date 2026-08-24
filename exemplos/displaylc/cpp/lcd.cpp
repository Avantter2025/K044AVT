/*******************************************************************************
 * @file      lcd.cpp
 * @brief     Exemplo interativo (CLI) do display LCD 2x40 via libK044AVT.so.
 * @project   Teclado de 44 Teclas PS/2 (LCD 2x40, Biometria e Teclado Auxiliar)
 * @author    Cariyl Kirsten <projetos@avanttectecnologia.com.br>
 * @company   Avanttec Tecnologia Ltda. - www.avanttectecnologia.com.br
 * @date      19/08/2026
 * @version   v1.0.0
 *
 * @details
 * Menu interativo cobrindo a API pública de display (driver_display/
 * display_driver.h) — escrita, cursor, scroll e o shift nativo ) 
 * cada opção é uma chamada fina para a função k044_* correspondente, 
 * sem reimplementar nada do protocolo.
 *
 * @note      Compilar: make
 * @note      Executar: sudo ./lcd (k044_open() precisa de acesso às portas I/O)
 * @target    Linux (x86_64 / Industrial PC)
 *
 * @copyright (c) 2026 Avanttec Tecnologia. Todos os direitos reservados.
 ******************************************************************************/

#include "../../../driver_display/display_driver.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <thread>

/* ------------------------------------------------------------------------- */
/* Utilitários de entrada                                                    */
/* ------------------------------------------------------------------------- */

static int read_line(char *buf, size_t len)
{
    if (!fgets(buf, (int)len, stdin)) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

static int prompt_int(const char *msg, int def)
{
    char line[64];
    printf("%s [%d]: ", msg, def);
    fflush(stdout);
    if (read_line(line, sizeof(line)) != 0) return def;
    if (line[0] == '\0') return def;
    return atoi(line);
}

static void prompt_str(const char *msg, const char *def, char *out, size_t out_len)
{
    char line[128];
    printf("%s [%s]: ", msg, def);
    fflush(stdout);
    if (read_line(line, sizeof(line)) != 0 || line[0] == '\0') {
        snprintf(out, out_len, "%s", def);
        return;
    }
    snprintf(out, out_len, "%s", line);
}

/* ------------------------------------------------------------------------- */
/* Diagnóstico de estado                                                     */
/* ------------------------------------------------------------------------- */

static void diag_status(void)
{
    int dead = k044_is_device_dead();
    printf("  [diag] device_dead=%s\n", dead ? "SIM" : "não");

    uint8_t id[2] = {0, 0};
    int r = k044_read_id(id);
    if (r == K044_OK && id[0] == 0xAB && id[1] == 0x83)
        printf("  [diag] ID do dispositivo: 0x%02X 0x%02X (OK)\n", id[0], id[1]);
    else
        printf("  [diag] k044_read_id falhou: %d  (id=0x%02X 0x%02X)\n", r, id[0], id[1]);

    uint8_t major = 0, minor = 0, patch = 0;
    r = k044_read_fw_ver(&major, &minor, &patch);
    if (r == K044_OK)
        printf("  [diag] versão do firmware: %u.%u.%u\n", major, minor, patch);
    else
        printf("  [diag] k044_read_fw_ver falhou: %d\n", r);
}

/* ------------------------------------------------------------------------- */
/* Utilitário: executa operação com log de resultado                        */
/* ------------------------------------------------------------------------- */

static int exec(const char *label, int r)
{
    printf("  [log] %s: ret=%d %s\n", label, r, r == K044_OK ? "(OK)" : "(falhou)");
    return r;
}

/* ------------------------------------------------------------------------- */
/* Operações                                                                 */
/* ------------------------------------------------------------------------- */

static void op_write_line(void)
{
    int row = prompt_int("Linha (0 ou 1)", 0);
    char text[64];
    prompt_str("Texto (até 39 chars)", "Olá, K044AVT!", text, sizeof(text));
    exec("k044_write_line", k044_write_line((uint8_t)row, text));
}

static void op_write_display(void)
{
    char l1[64], l2[64];
    prompt_str("Texto linha 0", "Linha 1", l1, sizeof(l1));
    prompt_str("Texto linha 1", "Linha 2", l2, sizeof(l2));
    exec("k044_write_display", k044_write_display(l1, l2));
}

static void op_printf(void)
{
    int row = prompt_int("Linha (0 ou 1)", 0);
    int col = prompt_int("Coluna (0-39)", 0);
    char text[41];
    prompt_str("Texto (até 40 chars, truncado conforme a coluna)", "teste", text, sizeof(text));
    exec("k044_write_pos", k044_write_pos((uint8_t)row, (uint8_t)col, "%s", text));
}

static void op_write_string(void)
{
    char text[128];
    prompt_str("Texto a escrever no cursor atual", "K044AVT", text, sizeof(text));
    exec("k044_write_string", k044_write_string(text));
}

static void op_set_cursor(void)
{
    int row = prompt_int("Linha (0 ou 1)", 0);
    int col = prompt_int("Coluna (0-39)", 0);
    exec("k044_set_cursor", k044_set_cursor((uint8_t)row, (uint8_t)col));
}

static void op_cursor_toggle(void)
{
    printf("  1) Ligar cursor\n  2) Desligar cursor\n");
    int choice = prompt_int("Opção", 1);
    if (choice == 2) exec("k044_cursor_off", k044_cursor_off());
    else             exec("k044_cursor_on",  k044_cursor_on());
}

static void op_clear(void)
{
    exec("k044_clear", k044_clear());
}

static void op_home(void)
{
    exec("k044_home", k044_home());
}

/* Qualquer tecla pressionada para o scroll contínuo automaticamente. 
 * Roda na thread interna do event e bloqueia só até a thread de scroll 
 * notar e sair, no máximo ~150-200ms, ver  scroll_thread_fn . */
static void key_stops_scroll_cb(const k044_event_t *evt, void *userdata)
{
    (void)userdata;
    if (evt->type != K044_EVT_KEY_MAKE) return;
    k044_scroll_stop();
    //k044_set_log_level(K044_LOG_DEBUG);
    //k044_set_log_level(K044_LOG_TRACE);
}

static void op_scroll_start(void)
{
    int row       = prompt_int("Linha (0 ou 1)", 0);
    int col_start = prompt_int("Coluna inicial da janela", 0);
    int width     = prompt_int("Largura da janela", 20);
    char text[128];
    prompt_str("Texto a rolar (contínuo)", "Letreiro de exemplo - K044AVT display LCD",
               text, sizeof(text));
    int delay_ms = prompt_int("Delay entre passos (ms)", 200);
    int repeat   = prompt_int("Repetições (0 = infinito)", 0);

    /* O scroll continuo transmite PS/2 a cada passo com
     * K044_LOG_DEBUG ativo isso inunda o terminal continuamente, 
     * atrapalhando o uso do menu. Silencia o log enquanto
     * o scroll roda; op_scroll_stop() restaura. */
    k044_set_log_level(K044_LOG_NONE);
    exec("k044_scroll_start",
         k044_scroll_start((uint8_t)row, (uint8_t)col_start, (uint8_t)width,
                            text, (unsigned int)delay_ms, repeat));
}

static void op_scroll_stop(void)
{
    exec("k044_scroll_stop", k044_scroll_stop());
    //k044_set_log_level(K044_LOG_DEBUG);
    //k044_set_log_level(K044_LOG_TRACE);
    printf("  Log de debug restaurado.\n");
}

/* Posiciona o cursor em (row,col) e apaga da' ate' o fim da linha
 * (k044_set_cursor + k044_erase_eol) - "limpar a partir de uma posicao",
 * em vez de k044_clear() (que apaga o display inteiro). */
static void op_erase_from(void)
{
    int row = prompt_int("Linha (0 ou 1)", 0);
    int col = prompt_int("Coluna (0-39)", 0);
    int r = exec("k044_set_cursor", k044_set_cursor((uint8_t)row, (uint8_t)col));
    if (r != K044_OK) return;
    exec("k044_erase_eol", k044_erase_eol());
}

/* Dispara k044_display_shift() N vezes seguidas, com uma pausa entre cada
 * chamada para dar tempo de observar o display fisicamente — util tambem
 * para inspecionar o comportamento real do shift nativo do HD44780 neste
 * hardware. */
static void op_shift(void)
{
    int direction = prompt_int("Direção (0=esquerda, 1=direita)", 0);
    int times     = prompt_int("Quantas vezes", 1);
    for (int i = 1; i <= times; i++) {
        printf("  shift %d/%d...\n", i, times);
        int r = exec("k044_display_shift", k044_display_shift(direction));
        if (r != K044_OK) break;
        if (i < times) std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}

/* Grava um dos padrões pré-definidos (ROM do firmware,
 * k044_write_cgram_preset()) na CGRAM e escreve numa posição do display
 * pra demonstrar. addr 0-7 mapeia direto para os codigos de controle
 * 0x00-0x07 do HD44780.
 *
 * Os 8 bytes do desenho não são mais transmitidos pelo canal PS/2 - só
 * 2 bytes de parâmetro (padrão + slot) */
static void op_cgram(void)
{
    printf("  1) Seta pra cima\n  2) ç (c cedilha)\n  3) Seta pra baixo\n"
           "  4) Seta pra esquerda\n  5) Seta pra direita\n  6) ã (a til)\n"
           "  7) Check/visto\n  8) Bateria vazia\n  9) Bateria cheia\n");
    int choice = prompt_int("Padrão", 1);
    uint8_t pattern;
    switch (choice) {
        case 2: pattern = K044_CGRAM_PRESET_C_CEDILLA;     break;
        case 3: pattern = K044_CGRAM_PRESET_ARROW_DOWN;    break;
        case 4: pattern = K044_CGRAM_PRESET_ARROW_LEFT;    break;
        case 5: pattern = K044_CGRAM_PRESET_ARROW_RIGHT;   break;
        case 6: pattern = K044_CGRAM_PRESET_A_TILDE;       break;
        case 7: pattern = K044_CGRAM_PRESET_CHECK;         break;
        case 8: pattern = K044_CGRAM_PRESET_BATTERY_EMPTY; break;
        case 9: pattern = K044_CGRAM_PRESET_BATTERY_FULL;  break;
        default: pattern = K044_CGRAM_PRESET_ARROW_UP;     break;
    }

    int slot = prompt_int("Slot da CGRAM a usar (0-7)", 0);
    if (slot < 0 || slot > 7) { printf("  Slot inválido.\n"); return; }

    int r = exec("k044_write_cgram_preset",
                 k044_write_cgram_preset(pattern, (uint8_t)slot));
    if (r != K044_OK) return;

    int row = prompt_int("Linha onde mostrar o caractere (0 ou 1)", 0);
    int col = prompt_int("Coluna (0-39)", 0);
    r = exec("k044_set_cursor", k044_set_cursor((uint8_t)row, (uint8_t)col));
    if (r != K044_OK) return;
    printf("  Escrevendo o caractere customizado no display...\n");
    exec("k044_write_char_raw", k044_write_char_raw((uint8_t)slot));
}

/* Escreve numa posição do display um caractere customizado que já foi
 * gravado num slot da CGRAM (via op_cgram()) - sem reler a CGRAM pelo
 * canal PS/2  */
static void op_show_cgram(void)
{
    int slot = prompt_int("Slot da CGRAM já gravado (0-7)", 0);
    if (slot < 0 || slot > 7) { printf("  Slot inválido.\n"); return; }

    int row = prompt_int("Linha onde mostrar o caractere (0 ou 1)", 0);
    int col = prompt_int("Coluna (0-39)", 0);
    int r = exec("k044_set_cursor", k044_set_cursor((uint8_t)row, (uint8_t)col));
    if (r != K044_OK) return;
    printf("  Escrevendo o caractere no display...\n");
    exec("k044_write_char_raw", k044_write_char_raw((uint8_t)slot));
}

static void op_bell(void)
{
    exec("k044_bell", k044_bell());
}

/* ------------------------------------------------------------------------- */

static void menu(void)
{
    printf("\n===== Display LCD (K044AVT) =====\n");
    printf("  1) Escrever linha\n");
    printf("  2) Escrever nas duas linhas\n");
    printf("  3) Escrever na posição (li,col)\n");
    printf("  4) Escrever na posição do cursor\n");
    printf("  5) Escrever scroll\n");
    printf("  6) Parar scroll\n");
    printf("  7) Limpar display\n");
    printf("  8) Limpar display a partir de (li,col)\n");
    printf("  9) Cursor ON/OFF\n");
    printf(" 10) Posicionar Cursor em Home\n");
    printf(" 11) Posicionar cursor (li,col)\n");
    printf(" 12) Deslocar linhas\n");
    printf(" 13) Gravar caracter customizado (CGRAM)\n");
    printf(" 14) Escrever caracter customizado gravado\n");
    printf(" 15) Bell\n");
    printf(" 16) Diagnóstico\n");
    printf("  0) Sair\n");
    printf("Opção: ");
    fflush(stdout);
}

int main(void)
{
    printf("lcd — exemplo do display LCD via libK044AVT %s\n", k044_version());
    fflush(stdout);

    int r = k044_open();
    if (r != K044_OK) {
        fprintf(stderr, "Falha ao abrir o dispositivo (%d). Execute com sudo.\n", r);
        return 1;
    }

    /* k044_open() desliga atkbd/psmouse do kernel (CCB=0x04) para assumir
     * o barramento PS/2 pelo resto do processo */
    if (k044_uinput_enable() != K044_OK)
        fprintf(stderr, "Aviso: uinput indisponivel (teclado nao sera repassado ao sistema).\n");
    if (k044_mouse_enable() != K044_OK)
        fprintf(stderr, "Aviso: mouse PS/2 indisponivel.\n");
    k044_start_event_loop();
    if (k044_aux_enable() != K044_OK)
        fprintf(stderr, "Aviso: teclado auxiliar PS/2 indisponivel.\n");
    k044_set_event_callback(key_stops_scroll_cb, NULL);

    //k044_set_log_level(K044_LOG_DEBUG);

    printf("--- Diagnóstico inicial ---\n");
    diag_status();
    printf("---------------------------\n");

    for (;;) {
        menu();
        char line[16];
        if (read_line(line, sizeof(line)) != 0) break;
        int opt = atoi(line);
        if (line[0] == '0') break;

        switch (opt) {
            case 1:  op_write_line();     break;
            case 2:  op_write_display();  break;
            case 3:  op_printf();         break;
            case 4:  op_write_string();   break;
            case 5:  op_scroll_start();   break;
            case 6:  op_scroll_stop();    break;
            case 7:  op_clear();          break;
            case 8:  op_erase_from();     break;
            case 9:  op_cursor_toggle();  break;
            case 10: op_home();           break;
            case 11: op_set_cursor();     break;
            case 12: op_shift();          break;
            case 13: op_cgram();          break;
            case 14: op_show_cgram();     break;
            case 15: op_bell();           break;
            case 16: diag_status();       break;
            default: printf("  Opção inválida.\n"); break;
        }
    }

    k044_scroll_stop();
    k044_set_event_callback(NULL, NULL);
    k044_aux_disable();
    k044_mouse_disable();
    k044_uinput_disable();
    k044_close();
    printf("Encerrado.\n");
    return 0;
}
