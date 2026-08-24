package com.avanttec.fingerprint;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.ShortByReference;

import java.util.HashMap;
import java.util.Map;

/**
 * Interface JNA mapeando as rotinas do módulo de impressão digital exportadas
 * por libK044AVT.so (funções k044_fp_*), mais o ciclo de vida do dispositivo.
 *
 * O firmware do AT89S52 faz o passthrough PS/2 (0xAD/0xAE) &lt;-&gt; UART do módulo;
 * do lado Java basta chamar estas funções — o protocolo do sensor é tratado
 * inteiramente dentro da biblioteca.
 *
 * A propriedade de sistema jna.library.path deve apontar para o diretório com
 * a libK044AVT.so (../../../driver_display a partir desta pasta):
 *   java -Djna.library.path=../../../driver_display -jar &lt;fat-jar&gt;.jar
 */
public interface FingerprintLib extends Library {

    FingerprintLib INSTANCE = Native.load("K044AVT", FingerprintLib.class,
            new HashMap<>(Map.of(Library.OPTION_STRING_ENCODING, "UTF-8")));

    /* Códigos de retorno relevantes */
    int K044_OK          =  0;
    int K044_ERR_FP      = -8;
    int K044_ERR_FP_TIMEOUT = -9;

    /* ---- Ciclo de vida do dispositivo ---- */
    int    k044_open();
    void   k044_close();
    String k044_version();

    /* ---- Log ---- */
    int K044_LOG_DEBUG = 3;
    int K044_LOG_TRACE = 4;
    void k044_set_log_level(int level);

    /* ---- Manter teclado (uinput) e mouse PS/2 ativos no sistema ---- */
    int k044_uinput_enable();
    int k044_uinput_disable();
    int k044_mouse_enable();
    int k044_mouse_disable();
    int k044_aux_enable();
    int k044_aux_disable();
    int k044_start_event_loop();
    int k044_stop_event_loop();

    /* ---- Fingerprint ---- */
    int  k044_fp_init();
    int  k044_fp_get_image();
    int  k044_fp_gen_char(byte bufferId);
    int  k044_fp_match();
    int  k044_fp_search(byte bufferId, short startPage, short pageNum,
                        ShortByReference foundId);
    int  k044_fp_reg_model();
    int  k044_fp_store(byte bufferId, short pageId);
    int  k044_fp_load_char(byte bufferId, short pageId);
    int  k044_fp_delete_char(short pageId, short count);
    int  k044_fp_empty();
    int  k044_fp_valid_template_count(ShortByReference count);
    int  k044_fp_read_nfpage(NFPage page);
    int  k044_fp_sleep();
    int  k044_fp_auto_identify(byte scoreLevel, short idNumber,
                               ShortByReference foundId, ShortByReference score);
    int  k044_fp_auto_enroll(short id, byte entries);

    /* Steps reportados por k044_fp_enroll()/k044_fp_search_retry() via FpCallback */
    int K044_FP_STEP_WAIT_FINGER   = 1;
    int K044_FP_STEP_CAPTURED      = 2;
    int K044_FP_STEP_REMOVE_FINGER = 3;
    int K044_FP_STEP_MERGING       = 4;
    int K044_FP_STEP_STORING       = 5;

    int k044_fp_enroll(short id, int fingerTimeoutMs);
    int k044_fp_search_retry(byte bufferId, short startPage, short pageNum,
                             int fingerTimeoutMs, int maxAttempts, ShortByReference foundId);

    void   k044_fp_set_callback(FpCallback cb, Pointer userdata);
    String k044_fp_confirmation_str(int code);
    int    k044_fp_last_confirmation_code();

    /* ---- LED do sensor (comando 0x3C, PS_ControlBLN) ---- */
    int K044_FP_LED_BREATHING   = 1;
    int K044_FP_LED_FLASHING    = 2;
    int K044_FP_LED_ALWAYS_ON   = 3;
    int K044_FP_LED_ALWAYS_OFF  = 4;
    int K044_FP_LED_GRADUAL_ON  = 5;
    int K044_FP_LED_GRADUAL_OFF = 6;

    int K044_FP_LED_COLOR_BLUE    = 1;
    int K044_FP_LED_COLOR_GREEN   = 2;
    int K044_FP_LED_COLOR_CYAN    = 3;
    int K044_FP_LED_COLOR_RED     = 4;
    int K044_FP_LED_COLOR_MAGENTA = 5;
    int K044_FP_LED_COLOR_YELLOW  = 6;
    int K044_FP_LED_COLOR_WHITE   = 7;

    int k044_fp_led_config(byte func, byte startColor, byte endColor, byte cycleTimes);

    /**
     * Callback de progresso (k044_fp_callback_t): void(int status, int step, void*).
     * status == 0 → progresso normal; != 0 → código de confirmação (erro).
     */
    interface FpCallback extends Callback {
        void invoke(int status, int step, Pointer userdata);
    }
}
