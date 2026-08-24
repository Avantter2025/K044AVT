package com.avanttec.fingerprint;

import com.sun.jna.Pointer;
import com.sun.jna.ptr.ShortByReference;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;
import javax.swing.SwingWorker;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.util.ArrayList;
import java.util.List;

/**
 * GUI Swing de demonstração do módulo de impressão digital via libK044AVT.so.
 *
 * Mostra uma "imagem animada" de leitura da digital (FingerprintScanPanel)
 * enquanto uma operação está em processamento. Cada rotina do módulo é acionada
 * por um botão e executada numa thread de fundo (SwingWorker) para manter a
 * animação fluida.
 *
 * Executar (a partir desta pasta):
 *   mvn -q package
 *   sudo java -Djna.library.path=../../../driver_display \
 *        -jar target/k044-fingerprint-demo-1.0.0.jar
 * (precisa de sudo porque k044_open() acessa as portas de I/O do dispositivo.)
 */
public class FingerDemo extends JFrame {

    private final FingerprintLib fp = FingerprintLib.INSTANCE;
    private final FingerprintScanPanel scan = new FingerprintScanPanel();
    private final JTextArea logArea = new JTextArea(10, 32);
    private final List<JButton> opButtons = new ArrayList<>();
    private JButton connectBtn;
    private boolean connected = false;

    // Mantém referência forte ao callback (evita coleta pelo GC durante o uso).
    private final FingerprintLib.FpCallback progressCb = (status, step, ud) ->
            SwingUtilities.invokeLater(() -> {
                if (status != 0)
                    log("   · " + fp.k044_fp_confirmation_str(status)
                            + String.format(" (0x%02X)", status));
                else
                    log("   · etapa " + step + " — siga o sensor");
            });

    public FingerDemo() {
        super("K044AVT — Módulo de Impressão Digital");
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout(10, 10));

        add(scan, BorderLayout.CENTER);
        add(buildSidebar(), BorderLayout.EAST);

        logArea.setEditable(false);
        logArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
        logArea.setBackground(new Color(0x0c, 0x0f, 0x14));
        logArea.setForeground(new Color(0xcf, 0xd8, 0xe3));
        JScrollPane sp = new JScrollPane(logArea);
        sp.setPreferredSize(new Dimension(200, 150));
        add(sp, BorderLayout.SOUTH);

        setEnabledOps(false);
        log("Pronto. Clique em \"Conectar\" (requer sudo + dispositivo).");
        pack();
        setLocationRelativeTo(null);

        /* setDefaultCloseOperation(EXIT_ON_CLOSE) chama System.exit()
         * direto, sem passar por toggleConnect() — se o usuario fechar a
         * janela (ou o processo receber SIGTERM/Ctrl+C) ainda conectado,
         * k044_close() nunca seria chamado, deixando o CCB do i8042
         * alterado (atkbd/psmouse fora, ver k044_open()) e o dispositivo
         * sem ser resetado. O shutdown hook da JVM roda para qualquer
         * caminho de saida normal, entao cobre todos esses casos. */
        Runtime.getRuntime().addShutdownHook(new Thread(this::disconnectDevice));
    }

    private JPanel buildSidebar() {
        JPanel side = new JPanel();
        side.setLayout(new BoxLayout(side, BoxLayout.Y_AXIS));
        side.setBorder(BorderFactory.createEmptyBorder(12, 12, 12, 12));

        JLabel title = new JLabel("Rotinas do sensor");
        title.setFont(title.getFont().deriveFont(Font.BOLD, 14f));
        title.setAlignmentX(Component.LEFT_ALIGNMENT);
        side.add(title);
        side.add(Box.createVerticalStrut(10));

        connectBtn = addButton(side, "Conectar", e -> toggleConnect());
        connectBtn.setEnabled(true);
        side.add(Box.createVerticalStrut(8));

        op(side, "Cadastrar (3 leituras)", doEnroll);
        op(side, "Identificar (getImage+search)", doIdentify);
        op(side, "Buscar no banco", doSearch);
        op(side, "Contar templates", doCount);
        op(side, "Informações do módulo", doInfo);
        op(side, "Remover template", doDelete);
        op(side, "Apagar todo o banco", doEmpty);
        op(side, "Sleep", doSleep);
        op(side, "LED do sensor", doLed);
        return side;
    }

    private interface Op { int run() throws Exception; }

    /** Cria um botão de operação que anima e roda em background. */
    private void op(JPanel side, String label, OpTask task) {
        JButton b = addButton(side, label, e -> task.execute(label));
        opButtons.add(b);
        side.add(Box.createVerticalStrut(6));
    }

    private JButton addButton(JPanel side, String label, java.awt.event.ActionListener l) {
        JButton b = new JButton(label);
        b.setAlignmentX(Component.LEFT_ALIGNMENT);
        b.setMaximumSize(new Dimension(240, 32));
        b.addActionListener(l);
        side.add(b);
        return b;
    }

    /* ------------------------------------------------------------------ */
    /* Conexão                                                             */
    /* ------------------------------------------------------------------ */

    /** Fecha o dispositivo se estiver conectado. Chamada tanto pelo botão
     *  "Desconectar" quanto pelo shutdown hook da JVM (ver construtor) —
     *  idempotente e segura de chamar de outra thread (shutdown hook). */
    private synchronized void disconnectDevice() {
        if (!connected) return;
        fp.k044_fp_set_callback(null, Pointer.NULL);
        fp.k044_aux_disable();
        fp.k044_mouse_disable();
        fp.k044_uinput_disable();
        fp.k044_close();
        connected = false;
    }

    private void toggleConnect() {
        if (connected) {
            disconnectDevice();
            connectBtn.setText("Conectar");
            setEnabledOps(false);
            scan.idle("Desconectado");
            log("Dispositivo fechado.");
            return;
        }
        log("libK044AVT " + fp.k044_version());
        int r = fp.k044_open();
        if (r != FingerprintLib.K044_OK) {
            log("ERRO: k044_open falhou (" + r + "). Rode com sudo e o device conectado.");
            scan.finish(false, "Falha ao conectar");
            return;
        }

        /* Mantem teclado TEC44FST (44 teclas), teclado auxiliar (110
         * teclas, porta PS/2 propria do AT89S52) e mouse PS/2 vivos no
         * sistema enquanto este programa roda — sem isto, k044_open() ja
         * desliga atkbd/psmouse do kernel (CCB=0x04) e nada mais fica
         * escutando o barramento em favor deles (mesmo padrao de
         * exemplos/fingerprint/cpp/finger2.cpp). Nenhuma das tres e
         * fatal se falhar (ex. /dev/uinput sem permissao, ou nenhum
         * dispositivo fisico no canal correspondente). */
        if (fp.k044_uinput_enable() != FingerprintLib.K044_OK)
            log("Aviso: uinput indisponível (teclado não será repassado ao sistema).");
        if (fp.k044_mouse_enable() != FingerprintLib.K044_OK)
            log("Aviso: mouse PS/2 indisponível.");
        fp.k044_start_event_loop();
        if (fp.k044_aux_enable() != FingerprintLib.K044_OK)
            log("Aviso: teclado auxiliar PS/2 indisponível.");

        if (fp.k044_fp_init() != FingerprintLib.K044_OK) {
            log("ERRO: módulo de digital não respondeu (k044_fp_init).");
            fp.k044_aux_disable();
            fp.k044_mouse_disable();
            fp.k044_uinput_disable();
            fp.k044_close();
            scan.finish(false, "Sem módulo");
            return;
        }
        fp.k044_fp_set_callback(progressCb, Pointer.NULL);
        fp.k044_set_log_level(FingerprintLib.K044_LOG_DEBUG);
        connected = true;
        connectBtn.setText("Desconectar");
        setEnabledOps(true);
        scan.idle("Conectado");
        log("Conectado ao módulo de digital.");
    }

    /* ------------------------------------------------------------------ */
    /* Execução assíncrona com animação                                    */
    /* ------------------------------------------------------------------ */

    /** Encapsula uma operação: anima, roda em background, reporta resultado. */
    private final class OpTask {
        final Op body;
        final boolean scanning;   // true = operação que lê o dedo (anima varredura)
        OpTask(Op body, boolean scanning) { this.body = body; this.scanning = scanning; }

        void execute(String label) {
            setEnabledOps(false);
            connectBtn.setEnabled(false);
            if (scanning) scan.startScanning("Lendo digital...");
            log("» " + label);

            new SwingWorker<Integer, Void>() {
                @Override protected Integer doInBackground() throws Exception {
                    return body.run();
                }
                @Override protected void done() {
                    int rc;
                    try { rc = get(); }
                    catch (Exception ex) { rc = -100; log("   exceção: " + ex.getMessage()); }
                    boolean ok = rc == FingerprintLib.K044_OK;
                    if (scanning) scan.finish(ok, ok ? "OK" : "Falhou");
                    setEnabledOps(true);
                    connectBtn.setEnabled(true);
                }
            }.execute();
        }
    }

    private OpTask scanOp(Op body)  { return new OpTask(body, true); }
    private OpTask quickOp(Op body) { return new OpTask(body, false); }

    /* ------------------------------------------------------------------ */
    /* Captura robusta (equivalente Java de wait_for_finger()/             */
    /* capture_gen_char() em exemplos/fingerprint/cpp/finger2.cpp)         */
    /* ------------------------------------------------------------------ */

    private static final int CAPTURE_MAX_ATTEMPTS = 5;

    /** present=true: espera k044_fp_get_image() ter sucesso (dedo capturado).
     *  present=false: espera "Sensor vazio" (0x02), ou seja, dedo removido.
     *  Roda dentro de doInBackground() (SwingWorker) — bloquear aqui não
     *  trava a UI. Retorna true se atingiu o estado esperado dentro do prazo. */
    private boolean waitForFinger(boolean present, long timeoutMs) throws InterruptedException {
        long deadline = System.currentTimeMillis() + timeoutMs;
        while (System.currentTimeMillis() < deadline) {
            int r = fp.k044_fp_get_image();
            if (present && r == FingerprintLib.K044_OK) return true;
            if (!present && r != FingerprintLib.K044_OK
                    && fp.k044_fp_last_confirmation_code() == 0x02) return true;
            Thread.sleep(150);
        }
        return false;
    }

    /** Espera o dedo e gera o template no buffer indicado, repetindo a
     *  leitura automaticamente em falha de qualidade (posicionamento ruim,
     *  não hardware) — até CAPTURE_MAX_ATTEMPTS tentativas. */
    private int captureGenChar(byte bufferId) throws InterruptedException {
        for (int attempt = 1; attempt <= CAPTURE_MAX_ATTEMPTS; attempt++) {
            if (!waitForFinger(true, 10000)) {
                log("   ✗ tempo esgotado esperando o dedo.");
                return FingerprintLib.K044_ERR_FP_TIMEOUT;
            }
            int r = fp.k044_fp_gen_char(bufferId);
            if (r == FingerprintLib.K044_OK) return r;

            log("   leitura ruim (tentativa " + attempt + "/" + CAPTURE_MAX_ATTEMPTS + "): "
                    + fp.k044_fp_confirmation_str(fp.k044_fp_last_confirmation_code()));
            if (attempt < CAPTURE_MAX_ATTEMPTS) {
                log("   retire e coloque o dedo novamente...");
                waitForFinger(false, 3000); /* best-effort */
            }
        }
        log("   ✗ falha na leitura após " + CAPTURE_MAX_ATTEMPTS + " tentativas.");
        return FingerprintLib.K044_ERR_FP;
    }

    /* ------------------------------------------------------------------ */
    /* Operações                                                           */
    /* ------------------------------------------------------------------ */

    /* Cadastro manual em 3 leituras (GetImage+GenChar x3, RegModel após
     * cada leitura extra, Store) — em vez de k044_fp_auto_enroll() (0x31),
     * que o sensor GSL6152 desta placa rejeita com pacote malformado
     * (cc=0xFF). RegModel base só mescla 2 buffers por vez, então a 3ª
     * leitura é combinada iterativamente (mesmo padrão de op_enroll() em
     * finger2.cpp). */
    private OpTask doEnroll = scanOp(() -> {
        int id = askInt("ID para cadastrar (1-999)", 1);
        if (id < 0) return cancelled();
        log("   cadastrando ID " + id + " (3 leituras)...");

        log("   coloque o dedo (1a leitura)...");
        int r = captureGenChar((byte) 1);
        if (r != FingerprintLib.K044_OK) return r;

        log("   retire o dedo...");
        waitForFinger(false, 5000);
        log("   coloque o MESMO dedo novamente (2a leitura)...");
        r = captureGenChar((byte) 2);
        if (r != FingerprintLib.K044_OK) return r;

        r = fp.k044_fp_reg_model();
        if (r != FingerprintLib.K044_OK) {
            log("   ✗ as leituras não correspondem ao mesmo dedo (" + r + ")");
            return r;
        }

        log("   retire o dedo...");
        waitForFinger(false, 5000);
        log("   coloque o MESMO dedo mais uma vez (3a leitura)...");
        r = captureGenChar((byte) 2);
        if (r != FingerprintLib.K044_OK) return r;

        r = fp.k044_fp_reg_model();
        if (r != FingerprintLib.K044_OK) {
            log("   ✗ as leituras não correspondem ao mesmo dedo (" + r + ")");
            return r;
        }

        r = fp.k044_fp_store((byte) 1, (short) id);
        log(r == FingerprintLib.K044_OK ? "   ✓ ID " + id + " cadastrado."
                                         : "   ✗ falha ao armazenar (" + r + ")");
        return r;
    });

    /* Identificação manual (GetImage+GenChar+Search) em vez de
     * k044_fp_auto_identify() (0x32), também rejeitado por este sensor.
     * O Search base do AS608 não aceita threshold de score, então esta
     * operação fica equivalente a "Buscar no banco" (mesma simplificação
     * já feita em finger2.cpp). */
    private OpTask doIdentify = scanOp(() -> {
        log("   coloque o dedo para identificar...");
        int r = captureGenChar((byte) 1);
        if (r != FingerprintLib.K044_OK) return r;
        ShortByReference id = new ShortByReference();
        r = fp.k044_fp_search((byte) 1, (short) 0, (short) 1000, id);
        if (r == 0 && (id.getValue() & 0xFFFF) != 0xFFFF)
            log("   ✓ identificado: ID " + (id.getValue() & 0xFFFF));
        else
            log("   ✗ digital não reconhecida (" + r + ")");
        return r;
    });

    private static final int SEARCH_MAX_ATTEMPTS = 3;

    /* Repete captura+busca ate' 3 tentativas se nao encontrar — uma busca
     * mal-sucedida pode ser so' posicionamento ruim do dedo nesta tentativa
     * especifica, nao necessariamente ausencia real no banco. Cada tentativa
     * faz uma NOVA captura (mesmo padrao de op_search() em finger2.cpp). */
    private OpTask doSearch = scanOp(() -> {
        ShortByReference id = new ShortByReference();
        int r = FingerprintLib.K044_ERR_FP;

        for (int attempt = 1; attempt <= SEARCH_MAX_ATTEMPTS; attempt++) {
            log("   coloque o dedo para buscar no banco (tentativa "
                    + attempt + "/" + SEARCH_MAX_ATTEMPTS + ")...");
            r = captureGenChar((byte) 1);
            if (r != FingerprintLib.K044_OK) return r;

            id.setValue((short) 0xFFFF);
            r = fp.k044_fp_search((byte) 1, (short) 0, (short) 1000, id);
            if (r == 0 && (id.getValue() & 0xFFFF) != 0xFFFF) {
                log("   ✓ encontrado: ID " + (id.getValue() & 0xFFFF));
                return r;
            }

            if (attempt < SEARCH_MAX_ATTEMPTS) {
                log("   não encontrado nesta tentativa. Retire o dedo...");
                waitForFinger(false, 3000);
            }
        }
        log("   ✗ sem correspondência após " + SEARCH_MAX_ATTEMPTS + " tentativas (" + r + ")");
        return r;
    });

    /* Controla o LED do sensor (comando 0x3C, PS_ControlBLN) — cor azul
     * fixa, mesmo mapeamento de op_led() em finger2.cpp. Não lê o dedo,
     * então usa quickOp (sem animação de varredura). */
    private OpTask doLed = quickOp(() -> {
        String[] options = { "Aceso", "Apagado", "Piscando (rápido)", "Piscando lento (respirando)" };
        int choice = JOptionPane.showOptionDialog(this, "Modo do LED", "LED do sensor",
                JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null, options, options[0]);
        if (choice < 0) { log("   cancelado."); return FingerprintLib.K044_OK; }

        byte func;
        String label;
        switch (choice) {
            case 0: func = (byte) FingerprintLib.K044_FP_LED_ALWAYS_ON;  label = "aceso";    break;
            case 1: func = (byte) FingerprintLib.K044_FP_LED_ALWAYS_OFF; label = "apagado";  break;
            case 2: func = (byte) FingerprintLib.K044_FP_LED_FLASHING;   label = "piscando (rápido)"; break;
            default: func = (byte) FingerprintLib.K044_FP_LED_BREATHING; label = "piscando lento (respirando)"; break;
        }

        int r = fp.k044_fp_led_config(func, (byte) FingerprintLib.K044_FP_LED_COLOR_BLUE,
                                       (byte) FingerprintLib.K044_FP_LED_COLOR_BLUE, (byte) 0);
        log(r == FingerprintLib.K044_OK ? "   ✓ LED " + label + "."
                                         : "   ✗ falha ao configurar LED (" + r + ")");
        return r;
    });

    private OpTask doCount = quickOp(() -> {
        ShortByReference c = new ShortByReference();
        int r = fp.k044_fp_valid_template_count(c);
        log(r == 0 ? "   templates cadastrados: " + (c.getValue() & 0xFFFF)
                   : "   ✗ falha ao contar (" + r + ")");
        return r;
    });

    private OpTask doInfo = quickOp(() -> {
        NFPage p = new NFPage();
        int r = fp.k044_fp_read_nfpage(p);
        if (r != 0) { log("   ✗ falha ao ler info (" + r + ")"); return r; }
        /* JNA nao sincroniza automaticamente os campos Java a partir da
         * memoria nativa apos a chamada, a menos que a Structure seja
         * declarada como Structure.ByReference — sem isto os campos
         * ficavam nos valores padrao (0/vazio) mesmo com a chamada
         * nativa tendo escrito dados reais (confirmado: k044_fp_read_nfpage
         * retornava K044_OK, mas registros/SN/etc apareciam zerados). */
        p.read();
        log("   registros=" + (p.registros & 0xFFFF)
                + " capacidade=" + (p.database_size & 0xFFFF)
                + " seg=" + (p.secur_level & 0xFFFF));
        log("   SN=" + NFPage.cstr(p.product_sn)
                + " sw=" + NFPage.cstr(p.software_version));
        return r;
    });

    private OpTask doDelete = quickOp(() -> {
        int id = askInt("ID do template a remover", 1);
        if (id < 0) return cancelled();
        int r = fp.k044_fp_delete_char((short) id, (short) 1);
        log(r == 0 ? "   ✓ template " + id + " removido." : "   ✗ falha (" + r + ")");
        return r;
    });

    private OpTask doEmpty = quickOp(() -> {
        int opt = JOptionPane.showConfirmDialog(this,
                "Apagar TODO o banco de digitais?", "Confirmar",
                JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE);
        if (opt != JOptionPane.YES_OPTION) { log("   cancelado."); return 0; }
        int r = fp.k044_fp_empty();
        log(r == 0 ? "   ✓ banco apagado." : "   ✗ falha (" + r + ")");
        return r;
    });

    private OpTask doSleep = quickOp(() -> {
        int r = fp.k044_fp_sleep();
        log(r == 0 ? "   módulo em sleep." : "   ✗ falha (" + r + ")");
        return r;
    });

    /* ------------------------------------------------------------------ */
    /* Helpers                                                             */
    /* ------------------------------------------------------------------ */

    /** Diálogo de inteiro; retorna -1 se cancelado/ inválido. */
    private int askInt(String msg, int def) {
        String s = JOptionPane.showInputDialog(this, msg, Integer.toString(def));
        if (s == null) return -1;
        try { return Integer.parseInt(s.trim()); }
        catch (NumberFormatException e) { return -1; }
    }

    /** Retorno "cancelado" que não conta como falha visual da leitura. */
    private int cancelled() { log("   cancelado."); return FingerprintLib.K044_OK; }

    private void setEnabledOps(boolean en) {
        for (JButton b : opButtons) b.setEnabled(en);
    }

    private void log(String s) {
        logArea.append(s + "\n");
        logArea.setCaretPosition(logArea.getDocument().getLength());
    }

    /* ------------------------------------------------------------------ */

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new FingerDemo().setVisible(true));
    }
}
