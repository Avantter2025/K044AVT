/*******************************************************************************
 * @file      DisplayLib.java
 * @brief     Interface JNA mapeando as rotinas de display (LCD HD44780 2x40).
 * @project   Teclado de 44 Teclas PS/2 (LCD 2x40, Biometria e Teclado Auxiliar)
 * @author    Cariyl Kirsten <projetos@avanttectecnologia.com.br>
 * @company   Avanttec Tecnologia Ltda. - www.avanttectecnologia.com.br
 * @date      19/08/2026
 * @version   v1.0.0
 *
 * @details
 * Mapeia as rotinas de display (LCD HD44780 2x40) exportadas por
 * libK044AVT.so (driver_display/display_driver.h), mais o pequeno
 * subconjunto de ciclo de vida/uinput/mouse/aux/event-loop necessário para
 * abrir o dispositivo e manter os teclados repassados ao sistema — mesmo
 * padrão de exemplos/fingerprint/java/FingerprintLib.java: um binding JNA
 * próprio do módulo, duplicando localmente essas funções gerais em vez de
 * depender do binding compartilhado exemplos/java/K044AVT.java (que por sua
 * vez não mapeia k044_write_char_raw()/k044_write_cgram_preset()).
 *
 * @note      Dependências: libK044AVT.so, JNA (net.java.dev.jna)
 * @target    Linux (x86_64 / Industrial PC)
 *
 * @copyright (c) 2026 Avanttec Tecnologia. Todos os direitos reservados.
 ******************************************************************************/
package com.avanttec.displaylcd;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.ptr.ByteByReference;

import java.util.HashMap;
import java.util.Map;

public interface DisplayLib extends Library {

    DisplayLib INSTANCE = Native.load("K044AVT", DisplayLib.class,
            new HashMap<>(Map.of(Library.OPTION_STRING_ENCODING, "UTF-8")));

    int K044_OK = 0;

    /* ---- Ciclo de vida do dispositivo ---- */
    int     k044_open();
    void    k044_close();
    String  k044_version();
    boolean k044_is_device_dead();

    /* ---- Log ---- */
    int K044_LOG_NONE  = 0;
    int K044_LOG_ERROR = 1;
    int K044_LOG_WARN  = 2;
    int K044_LOG_DEBUG = 3;
    int K044_LOG_TRACE = 4;
    void k044_set_log_level(int level);

    /* ---- Manter teclado (uinput), mouse PS/2 e teclado auxiliar ativos ---- */
    int k044_uinput_enable();
    int k044_uinput_disable();
    int k044_mouse_enable();
    int k044_mouse_disable();
    int k044_aux_enable();
    int k044_aux_disable();
    int k044_start_event_loop();
    int k044_stop_event_loop();

    /* ---- Diagnóstico ---- */
    int k044_read_id(byte[] id);   // id.length == 2
    int k044_read_fw_ver(ByteByReference major, ByteByReference minor,
                          ByteByReference patch);

    /* ---- Escrita básica ---- */
    int k044_write_line(byte row, String text);
    int k044_write_display(String line1, String line2);
    int k044_write_pos(byte row, byte col, String fmt, Object... args);
    int k044_write_string(String text);
    int k044_write_char_raw(byte c);

    /* ---- Cursor ---- */
    int k044_set_cursor(byte row, byte col);
    int k044_cursor_on();
    int k044_cursor_off();
    int k044_clear();
    int k044_home();
    int k044_erase_eol();

    /* ---- Scroll ---- */
    int k044_scroll_line(byte row, String text, int delayMs);
    int k044_scroll_start(byte row, byte colStart, byte width, String text,
                           int delayMs, int repeat);
    int k044_scroll_stop();

    /* ---- Shift nativo do HD44780 ---- */
    int k044_display_shift(int direction);

    /* ---- CGRAM: padrões pré-gravados em ROM (comando 0xB5) ---- */
    int K044_CGRAM_PRESET_ARROW_UP      = 1;
    int K044_CGRAM_PRESET_C_CEDILLA     = 2;
    int K044_CGRAM_PRESET_ARROW_DOWN    = 3;
    int K044_CGRAM_PRESET_ARROW_LEFT    = 4;
    int K044_CGRAM_PRESET_ARROW_RIGHT   = 5;
    int K044_CGRAM_PRESET_A_TILDE       = 6;
    int K044_CGRAM_PRESET_CHECK         = 7;
    int K044_CGRAM_PRESET_BATTERY_EMPTY = 8;
    int K044_CGRAM_PRESET_BATTERY_FULL  = 9;
    int k044_write_cgram_preset(byte pattern, byte slot);

    int k044_bell();
}
