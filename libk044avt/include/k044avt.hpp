/**
 * k044avt.hpp — Wrapper C++ RAII para a biblioteca libK044AVT.so
 *
 * Fornece a classe avanttec::Display com:
 *   - Gerenciamento automático de recursos (RAII)
 *   - Interface fluente (method chaining)
 *   - Tratamento de erros via exceções std::system_error
 *   - Wrappers tipo-seguros sobre a API C
 *
 * Uso:
 *   #include "k044avt.hpp"
 *
 *   avanttec::Display disp;
 *   disp.clear()
 *       .setCursor(0, 0)
 *       .write("Pronto!")
 *       .setCursor(1, 0)
 *       .write("Aguardando...");
 *
 * Compilar com: g++ -std=c++17 ... -lK044AVT
 */

#ifndef K044AVT_HPP
#define K044AVT_HPP

#include "display_driver.h"

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <stdexcept>
#include <system_error>
#include <chrono>
#include <cstdio>
#include <utility>

namespace avanttec {

/* =========================================================================
 * Categoria de erro personalizada
 * ========================================================================= */

class K044Category : public std::error_category {
public:
    const char *name() const noexcept override { return "k044avt"; }

    std::string message(int ev) const override {
        switch (ev) {
            case K044_OK:           return "Sucesso";
            case K044_ERR_PERM:     return "Permissão negada (requer root/setcap)";
            case K044_ERR_TIMEOUT:  return "Timeout — dispositivo não respondeu";
            case K044_ERR_NODEV:    return "Dispositivo não encontrado";
            case K044_ERR_RANGE:    return "Parâmetro fora do intervalo";
            case K044_ERR_INVAL:    return "Parâmetro inválido";
            case K044_ERR_BUSY:     return "Recurso ocupado";
            case K044_ERR_RESEND:   return "Dispositivo solicitou retransmissão";
            default:                return "Erro desconhecido";
        }
    }
};

inline const K044Category &k044_category() {
    static K044Category cat;
    return cat;
}

inline std::error_code make_error_code(int rc) {
    return std::error_code(rc, k044_category());
}

/* =========================================================================
 * Exceção da biblioteca
 * ========================================================================= */

class K044Error : public std::system_error {
public:
    explicit K044Error(int rc, const std::string &ctx = "")
        : std::system_error(make_error_code(rc),
                            ctx.empty() ? "" : ctx) {}
};

/* =========================================================================
 * Classe Display — interface principal
 * ========================================================================= */

class Display {
public:
    /**
     * Constrói e abre o dispositivo com configuração padrão.
     * @throws K044Error em caso de falha (sem permissão, dispositivo ausente)
     */
    Display() {
        check(k044_open(), "Display::Display()");
        open_ = true;
    }

    /**
     * Constrói e abre o dispositivo com configuração personalizada.
     * @throws K044Error em caso de falha
     */
    explicit Display(const k044_config_t &cfg) {
        check(k044_open_ex(&cfg), "Display::Display(cfg)");
        open_ = true;
    }

    /** Fecha o dispositivo automaticamente ao sair do escopo */
    ~Display() noexcept {
        if (open_) {
            k044_stop_event_loop();
            k044_close();
        }
    }

    /* Não copiável */
    Display(const Display &)            = delete;
    Display &operator=(const Display &) = delete;

    /* Movível */
    Display(Display &&other) noexcept : open_(other.open_) {
        other.open_ = false;
    }

    /* =====================================================================
     * Escrita — interface fluente
     * ===================================================================== */

    Display &clear() {
        check(k044_clear(), "clear");
        return *this;
    }

    Display &home() {
        check(k044_home(), "home");
        return *this;
    }

    Display &write(std::string_view s) {
        check(k044_write_buf(reinterpret_cast<const uint8_t *>(s.data()),
                              s.size()),
              "write");
        return *this;
    }

    Display &writeChar(uint8_t c) {
        check(k044_write_char(c), "writeChar");
        return *this;
    }

    Display &writeLine(int row, std::string_view s) {
        check(k044_write_line(static_cast<uint8_t>(row),
                               std::string(s).c_str()),
              "writeLine");
        return *this;
    }

    Display &writeDisplay(std::string_view l1, std::string_view l2) {
        check(k044_write_display(std::string(l1).c_str(),
                                  std::string(l2).c_str()),
              "writeDisplay");
        return *this;
    }

    template<typename... Args>
    Display &writePos(int row, int col, const char *fmt, Args&&... args) {
        check(k044_write_pos(static_cast<uint8_t>(row),
                              static_cast<uint8_t>(col),
                              fmt, std::forward<Args>(args)...),
              "writePos");
        return *this;
    }

    Display &scrollLine(int row, std::string_view text,
                        std::chrono::milliseconds delay) {
        check(k044_scroll_line(static_cast<uint8_t>(row),
                                std::string(text).c_str(),
                                static_cast<unsigned int>(delay.count())),
              "scrollLine");
        return *this;
    }

    /* =====================================================================
     * Controle de cursor — interface fluente
     * ===================================================================== */

    Display &setCursor(int row, int col) {
        check(k044_set_cursor(static_cast<uint8_t>(row),
                               static_cast<uint8_t>(col)),
              "setCursor");
        return *this;
    }

    Display &cursorOn() {
        check(k044_cursor_on(), "cursorOn");
        return *this;
    }

    Display &cursorOff() {
        check(k044_cursor_off(), "cursorOff");
        return *this;
    }

    Display &carriageReturn() {
        check(k044_carriage_return(), "carriageReturn");
        return *this;
    }

    Display &lineFeed() {
        check(k044_line_feed(), "lineFeed");
        return *this;
    }

    Display &cursorUp() {
        check(k044_cursor_up(), "cursorUp");
        return *this;
    }

    Display &backspace() {
        check(k044_backspace(), "backspace");
        return *this;
    }

    Display &tab() {
        check(k044_tab(), "tab");
        return *this;
    }

    Display &cursorInc() {
        check(k044_cursor_inc(), "cursorInc");
        return *this;
    }

    Display &cursorDec() {
        check(k044_cursor_dec(), "cursorDec");
        return *this;
    }

    Display &eraseEOL() {
        check(k044_erase_eol(), "eraseEOL");
        return *this;
    }

    Display &bell() {
        check(k044_bell(), "bell");
        return *this;
    }

    /* =====================================================================
     * PIN pad
     * ===================================================================== */

    Display &pinEnable() {
        check(k044_pin_enable(), "pinEnable");
        return *this;
    }

    Display &pinDisable() {
        check(k044_pin_disable(), "pinDisable");
        return *this;
    }

    /* =====================================================================
     * Teclado auxiliar
     * ===================================================================== */

    Display &auxEnable() {
        check(k044_aux_enable(), "auxEnable");
        return *this;
    }

    Display &auxDisable() {
        check(k044_aux_disable(), "auxDisable");
        return *this;
    }

    /* =====================================================================
     * Comandos PS/2 padrão
     * ===================================================================== */

    Display &reset() {
        check(k044_reset(), "reset");
        return *this;
    }

    Display &setLEDs(uint8_t mask) {
        check(k044_set_leds(mask), "setLEDs");
        return *this;
    }

    Display &setTypematic(uint8_t rate) {
        check(k044_set_typematic(rate), "setTypematic");
        return *this;
    }

    /** Lê ID do dispositivo. Retorna par {id[0], id[1]} (esperado: {0xAB, 0x83}) */
    std::pair<uint8_t, uint8_t> readID() {
        uint8_t id[2] = {0, 0};
        check(k044_read_id(id), "readID");
        return {id[0], id[1]};
    }

    /** Retorna versão do firmware como string */
    std::string firmwareVersion() {
        char buf[32] = {0};
        k044_firmware_version(buf, sizeof(buf));
        return std::string(buf);
    }

    /** Retorna versão da biblioteca */
    static std::string version() { return K044AVT_VERSION_STRING; }

    /* =====================================================================
     * EEPROM
     * ===================================================================== */

    /** Lê 100 bytes da EEPROM */
    std::vector<uint8_t> readEEPROM() {
        std::vector<uint8_t> buf(100);
        check(k044_eeprom_read(buf.data(), buf.size()), "readEEPROM");
        return buf;
    }

    /** Grava dados na EEPROM (max 500 bytes) */
    Display &writeEEPROM(const std::vector<uint8_t> &data) {
        check(k044_eeprom_write(data.data(), data.size()), "writeEEPROM");
        return *this;
    }

    /* =====================================================================
     * Sistema de eventos (melhoria 4)
     * ===================================================================== */

    /** Registra callback para eventos assíncronos */
    Display &onEvent(std::function<void(const k044_event_t &)> handler) {
        event_handler_ = std::move(handler);
        k044_set_event_callback(
            [](const k044_event_t *evt, void *ud) {
                auto *self = static_cast<Display *>(ud);
                if (self->event_handler_) self->event_handler_(*evt);
            },
            this);
        return *this;
    }

    Display &startEventLoop() {
        check(k044_start_event_loop(), "startEventLoop");
        return *this;
    }

    Display &stopEventLoop() {
        k044_stop_event_loop();
        return *this;
    }

    /**
     * Leitura síncrona de evento com timeout.
     * @throws K044Error se timeout expirar.
     */
    k044_event_t readEvent(std::chrono::milliseconds timeout
                           = std::chrono::milliseconds(1000)) {
        k044_event_t evt{};
        check(k044_read_event(&evt, static_cast<int>(timeout.count())),
              "readEvent");
        return evt;
    }

    /* =====================================================================
     * KeyEcho — exibição de teclas no display (melhoria nova)
     * ===================================================================== */

    /**
     * Inicia rotina de exibição de teclas.
     * @param cfg      Configuração (use k044_keyecho_cfg_init para defaults)
     * @param callback Função chamada a cada evento de tecla
     */
    Display &startKeyEcho(
        const k044_keyecho_cfg_t &cfg,
        std::function<void(k044_keyecho_event_t,
                           std::string_view,
                           char)> callback = nullptr)
    {
        keyecho_handler_ = std::move(callback);
        k044_keyecho_cb_t raw_cb = nullptr;
        if (keyecho_handler_) {
            raw_cb = [](k044_keyecho_event_t evt, const char *buf,
                        uint8_t /*len*/, char last, void *ud) {
                auto *self = static_cast<Display *>(ud);
                if (self->keyecho_handler_)
                    self->keyecho_handler_(evt,
                                           std::string_view(buf),
                                           last);
            };
        }
        check(k044_keyecho_start(&cfg, raw_cb, this), "startKeyEcho");
        return *this;
    }

    Display &stopKeyEcho() {
        check(k044_keyecho_stop(), "stopKeyEcho");
        keyecho_handler_ = nullptr;
        return *this;
    }

    /** Retorna conteúdo atual do buffer keyecho */
    std::string getKeyEchoBuffer() {
        char buf[16] = {0};
        uint8_t len = 0;
        check(k044_keyecho_get_buffer(buf, &len), "getKeyEchoBuffer");
        return std::string(buf, len);
    }

    /** Retorna posição atual do cursor no buffer keyecho (0-based) */
    uint8_t getKeyEchoCursor() {
        uint8_t pos = 0;
        check(k044_keyecho_get_cursor(&pos), "getKeyEchoCursor");
        return pos;
    }

    Display &clearKeyEcho() {
        check(k044_keyecho_clear(), "clearKeyEcho");
        return *this;
    }

    /* =====================================================================
     * Mutex explícito para transações compostas
     * ===================================================================== */

    Display &lock() {
        k044_lock();
        return *this;
    }

    void unlock() {
        k044_unlock();
    }

    /** Retorna true se o dispositivo PS/2 foi marcado como inoperante */
    bool isDeviceDead() const {
        return k044_is_device_dead() != 0;
    }

private:
    bool open_ = false;
    std::function<void(const k044_event_t &)>              event_handler_;
    std::function<void(k044_keyecho_event_t, std::string_view, char)>
                                                           keyecho_handler_;

    /** Lança K044Error se rc != K044_OK */
    static void check(int rc, const char *ctx) {
        if (rc != K044_OK) throw K044Error(rc, ctx);
    }
};

} /* namespace avanttec */

#endif /* K044AVT_HPP */
