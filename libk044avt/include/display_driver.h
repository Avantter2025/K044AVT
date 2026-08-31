/**
 * display_driver.h — API pública da biblioteca libK044AVT.so
 *
 * Biblioteca de espaço de usuário para comunicação com o dispositivo
 * Avanttec TEC44AVT via porta PS/2 de hardware.
 *
 * Controla: LCD HD44780 40 colunas × 2 linhas
 *           teclado 44 teclas e outras funcionalidades
 *
 * Protocolo: PS/2 hardware (portas 0x60 / 0x64) com comandos proprietários
 *            0xA0–0xA8 para controle do display e outras funcionalidades.
 *
 * REQUISITO: execução com CAP_SYS_RAWIO (root ou setcap).
 *
 * Versão: 1.0.3
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Versão da biblioteca
 * ========================================================================= */

#define K044AVT_VERSION_MAJOR  1
#define K044AVT_VERSION_MINOR  0
#define K044AVT_VERSION_PATCH  3
#define K044AVT_VERSION_STRING "1.0.3"

/* =========================================================================
 * Códigos de retorno
 * ========================================================================= */

#define K044_OK           0   /**< Sucesso                                   */
#define K044_ERR_PERM    -1   /**< Permissão negada (requer root/setcap)     */
#define K044_ERR_TIMEOUT -2   /**< Dispositivo não respondeu no prazo        */
#define K044_ERR_NODEV   -3   /**< Dispositivo não encontrado / ID inválido  */
#define K044_ERR_RANGE   -4   /**< Parâmetro fora do intervalo válido        */
#define K044_ERR_INVAL   -5   /**< Parâmetro inválido (NULL, etc.)           */
#define K044_ERR_BUSY    -6   /**< Recurso ocupado (event loop já ativo)     */
#define K044_ERR_RESEND  -7   /**< Dispositivo solicitou retransmissão       */

/* =========================================================================
 * Códigos de controle LCD
 * Enviados via k044_write_char() ou k044_write_buf()
 * ========================================================================= */

#define K044_BEL         0x07  /**< Bell (sem efeito neste hardware)           */
#define K044_BS          0x08  /**< Backspace — cursor uma posição à esquerda  */
#define K044_TAB         0x09  /**< Tab — avança cursor (alias de K044_CURSOR_INC) */
#define K044_CURSOR_INC  0x09  /**< Avança cursor uma posição (sem apagar)    */
#define K044_CURSOR_DEC  0x12  /**< Recua cursor uma posição (sem apagar)      */
#define K044_LF          0x0A  /**< Line Feed — move para linha 2              */
#define K044_VT          0x0B  /**< Cursor Up — move para linha 1              */
#define K044_FF          0x0C  /**< Form Feed — limpa display inteiro          */
#define K044_CR          0x0D  /**< Carriage Return — cursor para coluna 0     */
#define K044_CURSON      0x11  /**< Cursor On — ativa exibição do cursor       */
#define K044_CURSOFF     0x14  /**< Cursor Off — oculta cursor                 */
#define K044_ERASE_EOL   0x18  /**< Erase to EOL — apaga até fim da linha      */
#define K044_HOME        0x1E  /**< Home — cursor para posição (0,0)           */
#define K044_SETPOS      0x1F  /**< Set Position — sequência de 3 bytes:       */
                               /**< 0x1F + row_enc + col_enc                   */
                               /**< Use k044_set_cursor() para codificação     */

/* =========================================================================
 * Bitmasks de LEDs (comando 0xED)
 * ========================================================================= */

#define K044_LED_SCROLLLOCK  0x01
#define K044_LED_NUMLOCK     0x02
#define K044_LED_CAPSLOCK    0x04

/* =========================================================================
 * Scancodes PS/2 Set 2 — referência para keyecho e eventos
 * ========================================================================= */

#define K044_SC_ENTER      0x5A    /**< Enter                                */
#define K044_SC_BACKSPACE  0x66    /**< Backspace                             */
#define K044_SC_ESCAPE     0x76    /**< Escape                               */
#define K044_SC_BREAK_PFX  0xF0    /**< Prefixo de break code                */
#define K044_SC_SPACE      0x29    /**< Espaço (barra de espaço)             */
#define K044_SC_EXT_PFX    0xE0    /**< Prefixo de scancode estendido        */
#define K044_SC_UP         0xE075  /**< Seta Up (scancode estendido)         */
#define K044_SC_DOWN       0xE072  /**< Seta Down (scancode estendido)       */
#define K044_SC_LEFT       0xE06B  /**< Seta Left (scancode estendido, AUX)  */
#define K044_SC_RIGHT      0xE074  /**< Seta Right (scancode estendido, AUX) */
#define K044_SC_DEL        0xE071  /**< Delete (scancode estendido, AUX)     */

/* =========================================================================
 * Configuração de runtime
 * ========================================================================= */

typedef struct {
    uint16_t port_data;         /**< Porta de dados PS/2    (padrão: 0x60)  */
    uint16_t port_status;       /**< Porta de status PS/2   (padrão: 0x64)  */
    uint32_t write_timeout_ms;  /**< Timeout escrita em ms  (padrão: 100)   */
    uint32_t read_timeout_ms;   /**< Timeout leitura em ms  (padrão: 200)   */
    uint32_t reset_wait_ms;     /**< Espera após reset em ms(padrão: 1000)  */
    int      max_retries;       /**< Máx. tentativas RESEND (padrão: 3)     */
    int      log_level;         /**< Nível de log           (padrão: NONE)  */
} k044_config_t;

/**
 * Preenche cfg com os valores padrão.
 * Chame antes de personalizar campos específicos.
 */
void k044_config_init(k044_config_t *cfg);

/* =========================================================================
 * Níveis de log
 * ========================================================================= */

#define K044_LOG_NONE   0
#define K044_LOG_ERROR  1
#define K044_LOG_WARN   2
#define K044_LOG_DEBUG  3
#define K044_LOG_TRACE  4

/**
 * Define o nível de verbosidade dos logs internos.
 * Padrão: K044_LOG_NONE (silencioso).
 */
void k044_set_log_level(int level);

/**
 * @param cb  Função: void cb(int level, const char *file, int line,
 *                            const char *msg)
 */
void k044_set_log_callback(void (*cb)(int level, const char *file,
                                      int line, const char *msg));

/* =========================================================================
 * I/O hooks para mock/teste
 * ========================================================================= */

typedef uint8_t (*k044_inb_fn_t)(uint16_t port);
typedef void    (*k044_outb_fn_t)(uint8_t val, uint16_t port);

/**
 * Injeta funções alternativas de leitura/escrita de porta.
 * @param inb_fn   Função de leitura de porta (NULL = usa inb real)
 * @param outb_fn  Função de escrita de porta  (NULL = usa outb real)
 */
void k044_set_io_hooks(k044_inb_fn_t inb_fn, k044_outb_fn_t outb_fn);

/* =========================================================================
 * Ciclo de vida da biblioteca
 * ========================================================================= */

/**
 * Inicializa acesso ao dispositivo com configuração padrão.
 * @return K044_OK em sucesso, código de erro negativo em falha.
 */
int k044_open(void);

/**
 * Inicializa acesso com configuração personalizada.
 * @param cfg  Ponteiro para k044_config_t preenchida. NULL usa defaults.
 * @return K044_OK em sucesso, código de erro negativo em falha.
 */
int k044_open_ex(const k044_config_t *cfg);

/**
 * Libera acesso ao dispositivo. Para event loop se ativo.
 */
void k044_close(void);

/**
 * Adquire mutex do barramento PS/2 para transações compostas.
 * Use quando precisar garantir atomicidade de múltiplas chamadas.
 *
 * @return K044_OK em sucesso.
 */
int k044_lock(void);

/**
 * Libera mutex do barramento PS/2.
 */
void k044_unlock(void);

/* =========================================================================
 * Informações do dispositivo e biblioteca
 * ========================================================================= */

/**
 * Retorna string de versão da biblioteca ("1.0.0").
 */
const char *k044_version(void);

/**
 * Lê identificador do dispositivo via comando 0xF2.
 * Dispositivo responde: 0xFA + 0xAB + 0x83.
 *
 * @param id  Buffer de 2 bytes. id[0]=0xAB, id[1]=0x83 em sucesso.
 * @return K044_OK em sucesso.
 */
int k044_read_id(uint8_t id[2]);

/**
 * Lê versão do firmware via comando 0xF1.
 * Retorna string tipo "K044P001A" decodificada dos scancodes.
 *
 * @param buf  Buffer de destino.
 * @param len  Tamanho do buffer (mínimo 16 bytes recomendado).
 * @return K044_OK em sucesso.
 */
int k044_firmware_version(char *buf, size_t len);

/**
 * Lê versão numérica do firmware via comando 0xAF.
 * Firmware compatível retorna major.minor.patch (ex.: 1.0.0).
 *
 * @param major  Ponteiro para byte major (pode ser NULL).
 * @param minor  Ponteiro para byte minor (pode ser NULL).
 * @param patch  Ponteiro para byte patch (pode ser NULL).
 * @return K044_OK em sucesso, K044_ERR_NODEV se firmware não suporta 0xAF.
 */
int k044_read_fw_ver(uint8_t *major, uint8_t *minor, uint8_t *patch);

/**
 * @param buf  Buffer de destino (mínimo 12 bytes).
 * @param len  Tamanho do buffer.
 * @return K044_OK em sucesso.
 */
int k044_fw_ver_str(char *buf, size_t len);

/**
 * @param buf  Buffer de destino (mínimo 8 bytes).
 * @param len  Tamanho do buffer.
 * @return K044_OK em sucesso, K044_ERR_NODEV se firmware não suporta 0xB6.
 */
int k044_read_serial(char *buf, size_t len);

/* =========================================================================
 * Escrita no display — funções básicas
 * ========================================================================= */

/**
 * @param addr  Endereco CGRAM (0-63) ou numero do char (0-7, auto×8).
 * @param data  8 bytes do padrao do caractere (bits 0-4 usados).
 * @return K044_OK em sucesso.
 */
int k044_write_cgram(uint8_t addr, const uint8_t data[8]);

#define K044_CGRAM_PRESET_ARROW_UP       1
#define K044_CGRAM_PRESET_C_CEDILLA      2
#define K044_CGRAM_PRESET_ARROW_DOWN     3
#define K044_CGRAM_PRESET_ARROW_LEFT     4
#define K044_CGRAM_PRESET_ARROW_RIGHT    5
#define K044_CGRAM_PRESET_A_TILDE        6
#define K044_CGRAM_PRESET_CHECK          7
#define K044_CGRAM_PRESET_BATTERY_EMPTY  8
#define K044_CGRAM_PRESET_BATTERY_FULL   9

/**
 * @param pattern  Um dos K044_CGRAM_PRESET_*.
 * @param slot     Slot da CGRAM a usar (0-7).
 * @return K044_OK em sucesso.
 */
int k044_write_cgram_preset(uint8_t pattern, uint8_t slot);

/**
 * @param out  Buffer de 64 bytes de destino.
 * @return K044_OK em sucesso.
 */
int k044_read_cgram(uint8_t out[64]);

/**
 * @param c  Byte a enviar (via comando 0xA0 INSTDPY).
 * @return K044_OK em sucesso.
 */
int k044_write_char(uint8_t c);

/**
 * @param c  Byte a escrever (0-255, sem restrição de faixa).
 * @return K044_OK em sucesso.
 */
int k044_write_char_raw(uint8_t c);

/**
 * @param s  String a escrever (não NULL).
 * @return K044_OK em sucesso.
 */
int k044_write_string(const char *s);

/**
 * @param buf  Ponteiro para os bytes.
 * @param len  Número de bytes.
 * @return K044_OK em sucesso.
 */
int k044_write_buf(const uint8_t *buf, size_t len);

/* =========================================================================
 * Escrita no display — funções de alto nível
 * ========================================================================= */

/**
 * @param row   Linha: 0 ou 1.
 * @param text  Texto (truncado em 39 chars se maior).
 * @return K044_OK em sucesso.
 */
int k044_write_line(uint8_t row, const char *text);

/**
 * @param direction 0 = desloca para a esquerda, diferente de 0 = direita.
 * @return K044_OK em sucesso.
 */
int k044_display_shift(int direction);

/**
 * @param line1  Texto para linha 0 (NULL = linha vazia).
 * @param line2  Texto para linha 1 (NULL = linha vazia).
 * @return K044_OK em sucesso.
 */
int k044_write_display(const char *line1, const char *line2);

/**
 * @param row  Linha: 0 ou 1.
 * @param col  Coluna: 0–39.
 * @param fmt  Format string (printf-style).
 * @param ...  Argumentos variádicos.
 * @return K044_OK em sucesso.
 */
int k044_write_pos(uint8_t row, uint8_t col, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

/**
 * @param row       Linha: 0 ou 1.
 * @param text      Texto a rolar.
 * @param delay_ms  Intervalo entre passos em milissegundos.
 * @return K044_OK em sucesso.
 */
int k044_scroll_line(uint8_t row, const char *text, unsigned int delay_ms);

/**
 * @param row       Linha: 0 ou 1.
 * @param col_start Coluna inicial da janela de scroll.
 * @param width     Largura da janela (max 40 - col_start).
 * @param text      Texto a rolar.
 * @param delay_ms  Intervalo entre passos em milissegundos.
 * @param repeat    Número de repetições (0 = infinito, 1+ = N passes).
 * @return K044_OK em sucesso.
 */
int k044_scroll_start(uint8_t row, uint8_t col_start, uint8_t width,
                      const char *text, unsigned int delay_ms,
                      int repeat);

/**
 * Para o scroll contínuo iniciado por k044_scroll_start().
 * Aguarda a thread encerrar e restaura o estado.
 *
 * @return K044_OK em sucesso.
 */
int k044_scroll_stop(void);

/* =========================================================================
 * Controle de cursor
 * ========================================================================= */

/** Limpa o display inteiro (envia K044_FF = 0x0C). */
int k044_clear(void);

/**
 * @return K044_OK quando o dispositivo responder, K044_ERR_TIMEOUT
 *         se o timeout for atingido.
 */
int k044_wait_busy(void);

/** Move cursor para posição inicial (0,0) (envia K044_HOME = 0x1E). */
int k044_home(void);

/**
 * @param row  Linha: 0 (linha 1) ou 1 (linha 2).
 * @param col  Coluna: 0–39.
 * @return K044_OK em sucesso, K044_ERR_RANGE se row > 1 ou col > 39.
 */
int k044_set_cursor(uint8_t row, uint8_t col);

/** Ativa exibição do cursor (envia K044_CURSON = 0x11). */
int k044_cursor_on(void);

/** Oculta cursor (envia K044_CURSOFF = 0x14). */
int k044_cursor_off(void);

/** Carriage return: cursor para coluna 0 da linha atual (0x0D). */
int k044_carriage_return(void);

/** Line feed: cursor para linha 2 (0x0A). */
int k044_line_feed(void);

/** Cursor up: cursor para linha 1 (0x0B). */
int k044_cursor_up(void);

/** Backspace: cursor uma posição à esquerda (0x08). */
int k044_backspace(void);

/** Tab: avança cursor uma posição à direita (0x09). */
int k044_tab(void);

/** Avança cursor uma posição à direita sem apagar (0x09, mesmo que k044_tab). */
int k044_cursor_inc(void);

/** Recua cursor uma posição à esquerda sem apagar (0x12). */
int k044_cursor_dec(void);

/** Erase to EOL: apaga da posição atual até fim da linha (0x18). */
int k044_erase_eol(void);

/** Bell: sinal sonoro (0x07 — sem efeito neste hardware). */
int k044_bell(void);

/* =========================================================================
 * Controle do PIN pad (IMPLEMENTAÇÃO FUTURA)
 * ========================================================================= */

/**
 * @return K044_OK em sucesso.
 */
int k044_pin_enable(void);

/**
 * @return K044_OK em sucesso.
 */
int k044_pin_disable(void);

/* =========================================================================
 * Controle do teclado auxiliar
 * ========================================================================= */

/**
 * Habilita o teclado auxiliar.
 * Envia comando 0xA3 (AUX ENABLE).
 * O firmware repassa 0xF4 (ENABLE) ao teclado auxiliar.
 *
 * @return K044_OK em sucesso.
 */
int k044_aux_enable(void);

/**
 * Desabilita o teclado auxiliar.
 * Envia comando 0xAA (AUX DISABLE).
 * O firmware repassa 0xF5 (DISABLE) ao teclado auxiliar.
 *
 * @return K044_OK em sucesso.
 */
int k044_aux_disable(void);

/* =========================================================================
 * EEPROM (IMPLEMENTAÇÃO FUTURA)
 * ========================================================================= */

/**
 * @param buf  Buffer de destino (mínimo 100 bytes).
 * @param len  Tamanho do buffer (deve ser >= 100).
 * @return K044_OK em sucesso, K044_ERR_RANGE se len < 100.
 */
int k044_eeprom_read(uint8_t *buf, size_t len);

/**
 * @param addr  Endereco inicial na EEPROM (0-511).
 * @param buf   Buffer de destino.
 * @param len   Numero de bytes a ler (1-256).
 * @return K044_OK em sucesso.
 */
int k044_eeprom_read_from(uint16_t addr, uint8_t *buf, size_t len);

/**
 * @param table_id  TABLE_MAKE (0) ou TABLE_BREAK (1).
 * @param buf       Buffer com os bytes a gravar.
 * @param len       Tamanho do buffer (deve ser >= TABLEM_SIZE ou TABLEB_SIZE).
 * @return K044_OK em sucesso.
 */
#define TABLE_MAKE  0
#define TABLE_BREAK 1
#define TABLEM_SIZE 176
#define TABLEB_SIZE 264
int k044_write_table(uint8_t table_id, const uint8_t *buf, size_t len);

/**
 * @param buf  Dados a gravar (formato acima).
 * @param len  Numero de bytes (max 500).
 * @return K044_OK em sucesso.
 */
int k044_eeprom_write(const uint8_t *buf, size_t len);

/* =========================================================================
 * Comandos PS/2 padrão
 * ========================================================================= */

/**
 * Reinicia o firmware do dispositivo (comando 0xFF).
 * O dispositivo envia 0xFA (ACK) e 0xAA (self-test OK) após reiniciar.
 * A biblioteca aguarda reset_wait_ms antes de retornar.
 *
 * @return K044_OK em sucesso.
 */
int k044_reset(void);

/**
 * Controla LEDs do teclado (comando 0xED).
 * O dispositivo encaminha o comando para o teclado auxiliar.
 *
 * @param leds  Bitmask: K044_LED_SCROLLLOCK | K044_LED_NUMLOCK | K044_LED_CAPSLOCK
 * @return K044_OK em sucesso.
 */
int k044_set_leds(uint8_t leds);

/**
 * Define taxa de repetição de teclas (comando 0xF3).
 * Encoding PS/2: bits 6:5 = delay (0=250ms..3=1000ms),
 *                bits 4:0 = taxa (0=30cps..0x1F=2cps).
 * Padrão do firmware: 0x2B (delay 500ms, ~10.9 cps).
 *
 * @param rate  Byte de taxa conforme spec PS/2.
 * @return K044_OK em sucesso.
 */
int k044_set_typematic(uint8_t rate);

/* =========================================================================
 * Sistema de eventos — leitura assíncrona (melhoria 4)
 * ========================================================================= */

/** Tipo de evento recebido do dispositivo */
typedef enum {
    K044_EVT_KEY_MAKE   = 0,  /**< Tecla pressionada (44 teclas principais)  */
    K044_EVT_KEY_BREAK  = 1,  /**< Tecla liberada (após prefixo 0xF0)        */
    K044_EVT_PIN_KEY    = 2,  /**< Tecla do PIN pad pressionada              */
    K044_EVT_CARD_START = 3,  /**< Início de leitura de cartão               */
    K044_EVT_CARD_DATA  = 4,  /**< Dado de cartão (nibble decodificado)      */
    K044_EVT_CARD_END   = 5,  /**< Fim de leitura de cartão (sucesso)        */
    K044_EVT_CARD_ERROR = 6,  /**< Erro de leitura de cartão                 */
    K044_EVT_RAW        = 7,  /**< Byte bruto não classificado               */
} k044_event_type_t;

/** Estrutura de evento recebido do dispositivo */
typedef struct {
    k044_event_type_t type;         /**< Tipo do evento                      */
    uint16_t          scancode;     /**< Scancode PS/2 (uint16, estendido
                                         ex: 0xE075 para Seta Up)           */
    uint8_t           raw_byte;     /**< Byte bruto recebido da porta 0x60   */
    uint64_t          timestamp_ns; /**< Timestamp CLOCK_MONOTONIC (ns)      */
} k044_event_t;

/** Tipo do callback de evento */
typedef void (*k044_event_cb_t)(const k044_event_t *evt, void *userdata);

/**
 * Registra função de callback chamada a cada evento recebido.
 * O callback é invocado na thread interna do event loop —
 * deve ser thread-safe e retornar rapidamente.
 *
 * @param cb        Função callback (NULL desregistra).
 * @param userdata  Ponteiro opaco passado ao callback.
 */
void k044_set_event_callback(k044_event_cb_t cb, void *userdata);

/**
 * Inicia thread interna de leitura de eventos.
 * A thread faz poll do OBF (bit 0 de 0x64) a cada 1 ms.
 *
 * @return K044_OK em sucesso, K044_ERR_BUSY se já ativo.
 */
int k044_start_event_loop(void);

/**
 * Para a thread interna de eventos (join com timeout de 2s).
 *
 * @return K044_OK em sucesso.
 */
int k044_stop_event_loop(void);

/**
 * Leitura síncrona bloqueante de um evento.
 * Inicia event loop interno se não estiver ativo.
 *
 * @param evt         Buffer para receber o evento.
 * @param timeout_ms  Tempo máximo de espera (0 = não bloqueante).
 * @return K044_OK se evento recebido, K044_ERR_TIMEOUT se expirou.
 */
int k044_read_event(k044_event_t *evt, int timeout_ms);

/* =========================================================================
 * KeyEcho — exibição de teclas pressionadas no display
 * ========================================================================= */

/** Tipo de notificação do callback keyecho */
typedef enum {
    K044_KEYECHO_CHAR,       /**< Tecla adicionada ao buffer e exibida      */
    K044_KEYECHO_CLEARED,    /**< Tecla de limpeza (clear_scancode)         */
    K044_KEYECHO_BACKSPACE,  /**< Backspace removeu caractere à esquerda    */
    K044_KEYECHO_DELETE,     /**< Delete removeu caractere na posição atual */
    K044_KEYECHO_CONFIRMED,  /**< Tecla de confirmação pressionada          */
    K044_KEYECHO_FULL,       /**< Buffer cheio — tecla ignorada             */
} k044_keyecho_event_t;

/**
 * Callback de notificação da rotina keyecho.
 *
 * @param event      Tipo do evento.
 * @param buffer     Conteúdo atual do buffer (sempre fornecido).
 * @param len        Número de chars no buffer.
 * @param last_char  Char adicionado (apenas em K044_KEYECHO_CHAR).
 * @param userdata   Ponteiro opaco do caller.
 */
typedef void (*k044_keyecho_cb_t)(k044_keyecho_event_t  event,
                                   const char           *buffer,
                                   uint8_t               len,
                                   char                  last_char,
                                   void                 *userdata);

/** Configuração da rotina de exibição de teclas */
typedef struct {
    uint8_t  row;              /**< Linha do display: 0 ou 1       (padrão: 0)    */
    uint8_t  col_start;        /**< Coluna inicial: 0–14           (padrão: 0)    */
    uint8_t  max_chars;        /**< Máx. chars: 1–40               (padrão: 32)   */
    uint16_t clear_scancode;   /**< Scancode para limpar buffer inteiro (0=desligado) */
    uint16_t confirm_scancode; /**< Scancode para confirmar entrada (padrão: 0x5A)      */
    int      pin_mode;         /**< 1 = exibir '*' em vez do char   (padrão: 0)   */
    int      echo_enabled;     /**< 1 = exibir char no display      (padrão: 1)   */
} k044_keyecho_cfg_t;

/**
 * Preenche cfg com valores padrão.
 *   row=0, col_start=0, max_chars=32,
 *   clear_scancode=0 (desabilitado), confirm_scancode=0x5A (Enter),
 *   pin_mode=0, echo_enabled=1
 */
void k044_keyecho_cfg_init(k044_keyecho_cfg_t *cfg);

/**
 * Inicia rotina de exibição de teclas pressionadas no display.
 * Inicia event loop automaticamente se não estiver ativo.
 * Apenas teclas do teclado principal de 44 teclas são monitoradas.
 *
 * Comportamento quando buffer cheio: bloqueia novas teclas (ignora),
 * notifica callback com K044_KEYECHO_FULL.
 *
 * @param cfg       Configuração (NULL usa defaults).
 * @param cb        Callback de notificação (NULL = sem notificação).
 * @param userdata  Ponteiro opaco passado ao callback.
 * @return K044_OK em sucesso, K044_ERR_RANGE se max_chars > 40 ou
 *         col_start + max_chars > 16.
 */
int k044_keyecho_start(const k044_keyecho_cfg_t *cfg,
                       k044_keyecho_cb_t          cb,
                       void                      *userdata);

/**
 * Para a rotina keyecho e apaga os caracteres da área usada no display.
 *
 * @return K044_OK em sucesso.
 */
int k044_keyecho_stop(void);

/**
 * Copia o conteúdo atual do buffer keyecho para buf.
 *
 * @param buf  Buffer de destino (mínimo 16 bytes).
 * @param len  Ponteiro onde o número de chars será armazenado.
 * @return K044_OK em sucesso, K044_ERR_INVAL se buf ou len for NULL.
 */
int k044_keyecho_get_buffer(char *buf, uint8_t *len);

/**
 * Retorna a posição atual do cursor dentro do buffer keyecho (0-based).
 * A posição no display é: col_start + cursor_pos.
 *
 * @param pos  Ponteiro onde a posição do cursor será armazenada.
 * @return K044_OK em sucesso, K044_ERR_INVAL se pos for NULL.
 */
int k044_keyecho_get_cursor(uint8_t *pos);

/**
 * Limpa o buffer keyecho programaticamente e apaga do display.
 * Equivalente a pressionar a tecla clear_scancode configurada.
 *
 * @return K044_OK em sucesso.
 */
int k044_keyecho_clear(void);

/* =========================================================================
 * Injeção de teclas no sistema Linux via uinput
 *
 * Habilita: cria um dispositivo de entrada virtual (/dev/uinput) e injeta
 * todos os scancodes recebidos (teclado interno + teclado PS/2 auxiliar)
 * no subsistema de input do Linux como keycodes padrão.
 *
 * Requer: acesso de escrita a /dev/uinput (geralmente requer root).
 *         libevdev instalada.
 *
 * ATENÇÃO: remover os módulos atkbd/psmouse (modprobe -r) continua
 * sendo necessário para acesso às portas PS/2.
 *
 * Uso típico:
 *   k044_open();
 *   k044_uinput_enable();   // opcional — ativa injeção no sistema
 *   k044_start_event_loop();
 *   ...
 *   k044_uinput_disable();  // opcional — desativa injeção
 *   k044_close();
 * =========================================================================
 */

/**
 * Ativa a injeção de teclas no sistema Linux via uinput.
 *
 * Cria um dispositivo de entrada virtual com mapeamento de scancodes.
 * Uma vez ativo, todo K044_EVT_KEY_MAKE e K044_EVT_KEY_BREAK
 * recebido pelo event loop é traduzido para o keycode Linux equivalente
 * e injetado no sistema.
 *
 * Pode ser chamado antes ou depois de k044_start_event_loop().
 * Chamadas repetidas retornam K044_ERR_BUSY se já ativo.
 *
 * @return K044_OK em sucesso, K044_ERR_NODEV se /dev/uinput inacessível,
 *         K044_ERR_BUSY se já ativo.
 */
int k044_uinput_enable(void);

/**
 * Desativa a injeção uinput e destroi o dispositivo virtual.
 *
 * @return K044_OK em sucesso.
 */
int k044_uinput_disable(void);

/**
 * Retorna 1 se a injeção uinput está ativa, 0 caso contrário.
 */
int k044_uinput_is_active(void);

/**
 * Ativa o forwarding de mouse PS/2 via uinput.
 *
 * A ativacao do forwarding NAO interfere com k044_uinput_enable() —
 * sao dispositivos virtuais independentes (teclado + mouse).
 *
 * Requer acesso de escrita a /dev/uinput (executar como root).
 *
 * @return K044_OK em sucesso, K044_ERR_NODEV se /dev/uinput inacessivel,
 *         K044_ERR_BUSY se ja ativo.
 */
int k044_mouse_enable(void);

/**
 * Desativa o forwarding de mouse e destroi o dispositivo virtual.
 *
 * @return K044_OK em sucesso.
 */
int k044_mouse_disable(void);

/**
 * Retorna 1 se o forwarding de mouse esta ativo, 0 caso contrario.
 */
int k044_mouse_is_active(void);

/**
 * Configura o auto-enable do mouse forwarding na abertura do dispositivo.
 *
 * Se chamada com enable=1 ANTES de k044_open(), a biblioteca ativa
 * automaticamente o mouse forwarding (k044_mouse_enable) e inicia o
 * event loop (k044_start_event_loop) ao final de k044_open().
 *
 * Isto elimina a necessidade de chamadas explícitas a k044_mouse_enable()
 * e k044_start_event_loop() após o open, útil para aplicações que sempre
 * usam o mouse PS/2.
 *
 * @param enable  1 = auto-ativa mouse+event_loop em k044_open(), 0 = manual.
 */
void k044_set_mouse_auto(int enable);

/**
 * Verifica se o dispositivo PS/2 está operacional.
 *
 * Retorna 0 se o dispositivo estiver operacional (comunicação OK),
 * ou 1 se o dispositivo foi marcado como inoperante após uma falha
 * de protocolo irrecuperável (kbc_recover falhou).
 *
 * Quando inoperante, todas as funções de E/S retornam K044_ERR_NODEV
 * imediatamente, sem tentar acessar o barramento PS/2.
 * O keyecho é desativado automaticamente.
 *
 * Uso típico no APP:
 *   if (k044_is_device_dead()) {
 *       printf("ERRO: dispositivo PS/2 inoperante!\n");
 *       k044_close();
 *       // ... reconectar ou abortar
 *   }
 */
int k044_is_device_dead(void);

/* =========================================================================
 * Módulo de impressão digital (fingerprint)
 *
 * Comunicação com módulo fingerprint.
 * O firmware faz passthrough: recebe bytes do host via PS/2 (comando 0xAD),
 * retransmite ao FP module via UART, e retorna a resposta ao host.
 *
 * ========================================================================= */

/** Código de erro do módulo fingerprint */
#define K044_ERR_FP          -8
#define K044_ERR_FP_TIMEOUT  -9

/** Estrutura de pacote do protocolo fingerprint */
typedef struct {
    uint8_t  header[6];    /**< EF 01 FF FF FF FF */
    uint8_t  pid;          /**< 0x01=cmd, 0x07=resp */
    uint16_t payload_len;  /**< PLEN (big-endian) */
    uint8_t  data[256];    /**< Dados do payload */
    uint16_t data_len;     /**< Comprimento real em data[] */
    uint16_t checksum;     /**< Checksum (2 bytes, soma 16-bit) */
} k044_fp_packet_t;

/** Estrutura NFPage (página de informações do módulo) */
typedef struct {
    uint16_t registros;
    uint16_t temp_size;
    uint16_t database_size;
    uint16_t score_level;
    uint8_t  device_address[4];
    uint16_t cfg_pkt_size;
    uint16_t cfg_baud_rate;
    uint16_t anti_fake;
    uint16_t fp_sensor;
    uint16_t secur_level;
    uint16_t enroll_logic;
    uint16_t image_format;
    uint16_t delay_time;
    char     product_sn[17];        /**< Número de série (string+null) */
    char     software_version[17];
    char     manufacturer[17];
    char     sensor_name[17];
    uint8_t  password[4];
    uint8_t  jtag_flag[4];
} k044_fp_nfpage_t;

/**
 * Inicializa comunicação com o módulo fingerprint.
 * @return K044_OK em sucesso.
 */
int k044_fp_init(void);

/**
 * Captura imagem da impressão digital (comando 0x01).
 * O usuário deve estar com o dedo sobre o sensor.
 *
 * @return K044_OK em sucesso, K044_ERR_FP se sensor vazio ou erro.
 */
int k044_fp_get_image(void);

/**
 * Gera arquivo de características (template) a partir do ImageBuffer
 * (comando 0x02).
 *
 * @param buffer_id  1 = CharBuffer1, 2 = CharBuffer2
 * @return K044_OK em sucesso.
 */
int k044_fp_gen_char(uint8_t buffer_id);

/**
 * Compara dois templates nos CharBuffers (comando 0x03).
 * Requer getImage + genChar para dois dedos.
 *
 * @return K044_OK se coincidem, K044_ERR_FP se não coincidem.
 */
int k044_fp_match(void);

/**
 * Busca template no banco de dados (comando 0x04).
 * O template deve estar no CharBuffer especificado.
 *
 * @param buffer_id    1 ou 2
 * @param start_page   página inicial (0-based)
 * @param page_num     número de páginas para buscar
 * @param found_id     retorna o ID encontrado (ou 0xFFFF se não encontrado)
 * @return K044_OK em sucesso, K044_ERR_FP se não encontrado.
 */
int k044_fp_search(uint8_t buffer_id, uint16_t start_page,
                   uint16_t page_num, uint16_t *found_id);

/**
 * Mescla templates dos CharBuffers em um modelo (comando 0x05).
 * Usado internamente pelo AutoEnroll.
 *
 * @return K044_OK em sucesso.
 */
int k044_fp_reg_model(void);

/**
 * Armazena modelo no banco de dados (comando 0x06).
 *
 * @param buffer_id   CharBuffer de origem (1 ou 2)
 * @param page_id     ID da página (0-999)
 * @return K044_OK em sucesso.
 */
int k044_fp_store(uint8_t buffer_id, uint16_t page_id);

/**
 * Carrega modelo do banco de dados para CharBuffer (comando 0x07).
 *
 * @param buffer_id   CharBuffer de destino (1 ou 2)
 * @param page_id     ID da página
 * @return K044_OK em sucesso.
 */
int k044_fp_load_char(uint8_t buffer_id, uint16_t page_id);

/**
 * Remove template(s) do banco de dados (comando 0x0C).
 *
 * @param page_id     ID inicial
 * @param count       número de templates a remover
 * @return K044_OK em sucesso.
 */
int k044_fp_delete_char(uint16_t page_id, uint16_t count);

/**
 * Limpa todo o banco de dados (comando 0x0D).
 *
 * @return K044_OK em sucesso.
 */
int k044_fp_empty(void);

/**
 * Retorna o número de templates válidos no banco (comando 0x1D).
 *
 * @param count  ponteiro para receber o contador
 * @return K044_OK em sucesso.
 */
int k044_fp_valid_template_count(uint16_t *count);

/**
 * Lê a página de informações do módulo (comando 0x16).
 * Retorna dados como número de série, versão de software, etc.
 *
 * @param page  ponteiro para estrutura NFPage (pode ser NULL)
 * @return K044_OK em sucesso.
 */
int k044_fp_read_nfpage(k044_fp_nfpage_t *page);

/**
 * Coloca o módulo em modo sleep (comando 0x33).
 *
 * @return K044_OK em sucesso.
 */
int k044_fp_sleep(void);

/** Modos de k044_fp_led_config() (parâmetro func, comando PS_ControlBLN) */
#define K044_FP_LED_BREATHING    1  /**< Respiração (fade in/out contínuo) */
#define K044_FP_LED_FLASHING     2  /**< Piscando                          */
#define K044_FP_LED_ALWAYS_ON    3  /**< Sempre aceso                      */
#define K044_FP_LED_ALWAYS_OFF   4  /**< Sempre apagado                    */
#define K044_FP_LED_GRADUAL_ON   5  /**< Acende gradualmente                */
#define K044_FP_LED_GRADUAL_OFF  6  /**< Apaga gradualmente                 */

/** Cores de k044_fp_led_config() (start_color/end_color) — bitmask de
 *  3 bits: bit0=azul, bit1=verde, bit2=vermelho. Combine bits para cores
 *  compostas (ex.: azul|verde = ciano). */
#define K044_FP_LED_COLOR_BLUE     1  /**< 001 */
#define K044_FP_LED_COLOR_GREEN    2  /**< 010 */
#define K044_FP_LED_COLOR_CYAN     3  /**< 011 (azul+verde)     */
#define K044_FP_LED_COLOR_RED      4  /**< 100 */
#define K044_FP_LED_COLOR_MAGENTA  5  /**< 101 (azul+vermelho)  */
#define K044_FP_LED_COLOR_YELLOW   6  /**< 110 (verde+vermelho) */
#define K044_FP_LED_COLOR_WHITE    7  /**< 111 (todas)          */

/**
 * Controla o LED do sensor de digital (comando 0x3C, PS_ControlBLN).
 * Nem todo clone AS608 implementa este comando estendido — se não for
 * suportado, retorna K044_ERR_FP (ver k044_fp_last_confirmation_code()).
 *
 * @param func         modo — use uma das constantes K044_FP_LED_* acima
 *                     (BREATHING/FLASHING/ALWAYS_ON/ALWAYS_OFF/
 *                     GRADUAL_ON/GRADUAL_OFF).
 * @param start_color  cor inicial — use K044_FP_LED_COLOR_* acima.
 * @param end_color    cor final (igual a start_color para cor sólida;
 *                     diferente para transição entre cores).
 * @param cycle_times  número de ciclos (0 = infinito, até o próximo
 *                     comando de LED).
 * @return K044_OK em sucesso, K044_ERR_FP se o sensor rejeitar o comando.
 */
int k044_fp_led_config(uint8_t func, uint8_t start_color, uint8_t end_color,
                       uint8_t cycle_times);

/**
 * Identificação automática (comando 0x32).
 * Captura imagem, gera template e busca no banco em um comando.
 *
 * @param score_level  nível de segurança (1-9, recomendado: 5)
 * @param id_number    ID específico para verificar (-1 = busca em todo banco)
 * @param found_id     retorna o ID encontrado
 * @param score        retorna o score da correspondência
 * @return K044_OK se identificado, K044_ERR_FP se não encontrado.
 */
int k044_fp_auto_identify(uint8_t score_level, int16_t id_number,
                          uint16_t *found_id, uint16_t *score);

/**
 * Cadastro automático (comando 0x31).
 * Guia o usuário através do processo de cadastro multi-step.
 *
 * @param id        ID para armazenar (1-999)
 * @param entries   número de amostras (1-3, recomendado: 3)
 * @return K044_OK em sucesso.
 */
int k044_fp_auto_enroll(uint16_t id, uint8_t entries);

/**
 * Callback para eventos do módulo fingerprint.
 *
 * @param status    0x00=sucesso, outro=confirmation code
 * @param step      passo atual (AutoEnroll: 0=picture, 1=feature, 2=entry, etc.)
 * @param userdata  ponteiro opaco do caller
 */
typedef void (*k044_fp_callback_t)(int status, int step, void *userdata);

/**
 * Registra callback para notificações do fingerprint.
 * Usado pelo AutoEnroll para notificar o usuário a colocar/remover o dedo.
 *
 * @param cb        callback (NULL desregistra)
 * @param userdata  ponteiro opaco
 */
void k044_fp_set_callback(k044_fp_callback_t cb, void *userdata);

/**
 * Steps reportados via k044_fp_callback_t por k044_fp_enroll()/
 * k044_fp_search_retry() durante as operacoes de leitura do sensor.
 * status no callback: 0 = notificacao de progresso normal, outro valor =
 * confirmation code de uma tentativa de captura malsucedida.
 */
#define K044_FP_STEP_WAIT_FINGER   1  /* aguardando o dedo */
#define K044_FP_STEP_CAPTURED      2  /* leitura capturada com sucesso */
#define K044_FP_STEP_REMOVE_FINGER 3  /* aguardando remocao do dedo */
#define K044_FP_STEP_MERGING       4  /* RegModel (mesclando leituras) em andamento */
#define K044_FP_STEP_STORING       5  /* Store (gravando no banco) em andamento */

/**
 * Cadastro manual em 3 leituras (GetImage+GenChar+RegModel+Store).
 * Fluxo de captura com progresso reportado via callback:
 * 1a leitura -> retira o dedo -> 2a leitura -> RegModel -> retira o
 * dedo -> 3a leitura -> RegModel -> Store. Reporta progresso via
 * k044_fp_set_callback() (K044_FP_STEP_*).
 *
 * @param id                 ID para armazenar (1-999)
 * @param finger_timeout_ms  timeout esperando o dedo em cada leitura
 *                           (ex.: 10000)
 * @return K044_OK em sucesso, K044_ERR_FP/K044_ERR_FP_TIMEOUT em falha -
 *         use k044_fp_last_confirmation_code() para detalhar.
 */
int k044_fp_enroll(uint16_t id, int finger_timeout_ms);

/**
 * Busca no banco com ate' max_attempts tentativas de captura+busca -
 * uma busca malsucedida pode ser so' posicionamento ruim do dedo nessa
 * tentativa especifica, nao ausencia real no banco; cada tentativa faz
 * uma NOVA captura (rebuscar o mesmo template geraria o mesmo
 * resultado). Mesmo fluxo ja validado em op_search()/doSearch().
 * Reporta progresso via k044_fp_set_callback() (K044_FP_STEP_*).
 *
 * @param buffer_id          buffer de char a usar (tipicamente 1)
 * @param start_page         pagina inicial do banco (0-based)
 * @param page_num           numero de paginas para buscar
 * @param finger_timeout_ms  timeout esperando o dedo em cada tentativa
 * @param max_attempts       numero de tentativas de captura+busca (ex.: 3)
 * @param found_id           [out] ID encontrado, ou 0xFFFF se nao encontrado
 * @return K044_OK se encontrou, K044_ERR_FP/K044_ERR_FP_TIMEOUT senao.
 */
int k044_fp_search_retry(uint8_t buffer_id, uint16_t start_page,
                         uint16_t page_num, int finger_timeout_ms,
                         int max_attempts, uint16_t *found_id);

/**
 * Retorna string descritiva para um código de confirmação do módulo.
 *
 * @param code  código de confirmação (0-53)
 * @return ponteiro para string estática.
 */
const char *k044_fp_confirmation_str(int code);

/**
 * Retorna o último confirmation code (AS608) computado por qualquer
 * k044_fp_*. Como as funções de alto nível só devolvem K044_OK/K044_ERR_FP,
 * esta função permite recuperar o motivo real de uma falha (para logging,
 * diagnóstico ou exibição via k044_fp_confirmation_str()).
 *
 * @return código de confirmação (0x00-0x35), ou 0xFF se a última resposta
 *         não pôde ser interpretada como um frame válido.
 */
int k044_fp_last_confirmation_code(void);

/**
 * Diagnóstico de baixo nível: envia bytes brutos via passthrough 
 * @param data       bytes a enviar (máx. 255)
 * @param data_len   quantidade de bytes em data (máx. 255)
 * @param resp       buffer para a resposta crua
 * @param resp_len   preenchido com a quantidade de bytes realmente lidos
 * @param max_resp   tamanho de resp
 * @return K044_OK se o 0xAD foi aceito (ACK) e a transação completou
 *         (não significa que os bytes recebidos batem com os enviados —
 *         cabe ao chamador comparar); código de erro se o 0xAD foi
 *         rejeitado (RESEND/timeout) ou os parâmetros são inválidos.
 */
int k044_fp_raw_test(const uint8_t *data, uint16_t data_len,
                     uint8_t *resp, uint16_t *resp_len, uint16_t max_resp);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_DRIVER_H */
