/*******************************************************************************
 * @file      LcdDemo.java
 * @brief     GUI Swing de demonstração do display LCD (2x40).
 * @project   Teclado de 44 Teclas PS/2 (LCD 2x40, Biometria e Teclado Auxiliar)
 * @author    Cariyl Kirsten <projetos@avanttectecnologia.com.br>
 * @company   Avanttec Tecnologia Ltda. - www.avanttectecnologia.com.br
 * @date      19/08/2026
 * @version   v1.0.0
 *
 * @details
 * GUI Swing de demonstração do módulo de display LCD (HD44780, 2x40) via
 * libK044AVT.so — mesmo espírito de exemplos/fingerprint/java/FingerDemo.java:
 * cada operação é um botão, executado numa thread de fundo (SwingWorker) pra
 * manter a UI responsiva, e o painel central (LcdPanel) mostra visualmente o
 * que a aplicação escreveu no display 2x40.
 *
 * @note      Executar (a partir desta pasta):
 *              mvn -q package
 *              sudo java -Djna.library.path=.driver_display \
 *                   -jar target/k044-displaylcd-demo-1.0.0.jar
 *            (precisa de sudo porque k044_open() acessa as portas de I/O.)
 * @target    Linux (x86_64 / Industrial PC)
 *
 * @copyright (c) 2026 Avanttec Tecnologia. Todos os direitos reservados.
 ******************************************************************************/
package com.avanttec.displaylcd;

import com.sun.jna.ptr.ByteByReference;

import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
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
import java.awt.Image;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class LcdDemo extends JFrame {

    private final DisplayLib lib = DisplayLib.INSTANCE;
    private final LcdPanel lcd = new LcdPanel();
    private final JTextArea logArea = new JTextArea(10, 60);
    private final List<JButton> opButtons = new ArrayList<>();
    private JButton connectBtn;
    private boolean connected = false;

    /** Lembra qual padrão pré-gravado foi escrito por último em cada slot
     * (0-7) da CGRAM — só existe aqui, do lado Java; o dispositivo não
     * expõe uma forma confiável de reler isso (ver LcdPanel). Usado pelo
     * item "Escrever caractere já gravado" pra saber qual bitmap desenhar. */
    private final int[] slotPattern = new int[8];

    public LcdDemo() {
        super("K044AVT — Avanttec Tecnologia — Display LCD (HD44780 2x40)");
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout(10, 10));

        Image icon = loadImage("/avanttec.png");
        if (icon != null) {
            setIconImage(icon);
            /* JFrame.setIconImage() só cobre o ícone da própria janela
             * (barra de título/alt-tab) — em muitos ambientes Linux
             * (GNOME/KDE) o ícone da barra de tarefas/dock é controlado
             * separadamente pela API Taskbar (Java 9+). Sem isto o app
             * podia aparecer com o ícone genérico do Java na barra de
             * tarefas mesmo com a janela mostrando o ícone certo. */
            if (java.awt.Taskbar.isTaskbarSupported()) {
                java.awt.Taskbar taskbar = java.awt.Taskbar.getTaskbar();
                if (taskbar.isSupported(java.awt.Taskbar.Feature.ICON_IMAGE)) {
                    try {
                        taskbar.setIconImage(icon);
                    } catch (UnsupportedOperationException | SecurityException ignored) {
                        /* plataforma/ambiente sem suporte - ícone da janela já foi setado acima */
                    }
                }
            }
        }

        java.util.Arrays.fill(slotPattern, DisplayLib.K044_CGRAM_PRESET_ARROW_UP);

        JPanel top = new JPanel(new BorderLayout());
        top.setBorder(BorderFactory.createEmptyBorder(12, 12, 0, 12));
        top.add(lcd, BorderLayout.CENTER);
        add(top, BorderLayout.NORTH);

        add(buildSidebar(), BorderLayout.EAST);

        logArea.setEditable(false);
        logArea.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
        logArea.setBackground(new Color(0x0c, 0x0f, 0x14));
        logArea.setForeground(new Color(0xcf, 0xd8, 0xe3));
        JScrollPane sp = new JScrollPane(logArea);
        sp.setPreferredSize(new Dimension(200, 160));
        add(sp, BorderLayout.SOUTH);

        setEnabledOps(false);
        log("Pronto. Clique em \"Conectar\" (requer sudo + dispositivo).");
        pack();
        setLocationRelativeTo(null);

        /* setDefaultCloseOperation(EXIT_ON_CLOSE) chama System.exit() direto,
         * sem passar por toggleConnect() — mesmo motivo/fix de FingerDemo:
         * sem o shutdown hook, k044_close() nunca rodaria ao fechar a
         * janela, deixando o CCB do i8042 alterado (atkbd/psmouse fora). */
        Runtime.getRuntime().addShutdownHook(new Thread(this::disconnectDevice));
    }

    private JPanel buildSidebar() {
        JPanel side = new JPanel(new BorderLayout(0, 10));
        side.setBorder(BorderFactory.createEmptyBorder(12, 12, 12, 12));

        /* Cabeçalho: título + botão Conectar/Desconectar à esquerda, banner
         * da empresa à direita, lado a lado na mesma linha (ver tela_exemplo.jpg). */
        JPanel headerLeft = new JPanel();
        headerLeft.setLayout(new BoxLayout(headerLeft, BoxLayout.Y_AXIS));
        JLabel title = new JLabel("Operações do display");
        title.setFont(title.getFont().deriveFont(Font.BOLD, 14f));
        title.setAlignmentX(Component.LEFT_ALIGNMENT);
        headerLeft.add(title);
        headerLeft.add(Box.createVerticalStrut(8));
        connectBtn = addButton(headerLeft, "Conectar", e -> toggleConnect());
        connectBtn.setEnabled(true);

        JLabel banner = new JLabel(loadScaledIcon("/avanttec.jpg", 260));
        banner.setVerticalAlignment(JLabel.CENTER);
        banner.setBorder(BorderFactory.createEmptyBorder(0, 12, 0, 0));

        JPanel header = new JPanel(new BorderLayout());
        header.add(headerLeft, BorderLayout.WEST);
        header.add(banner, BorderLayout.EAST);
        side.add(header, BorderLayout.NORTH);

        /* 16 operações divididas em 2 colunas de 8, ocupando a 
        *  largura toda abaixo do cabeçalho. */
        JPanel col1 = new JPanel();
        col1.setLayout(new BoxLayout(col1, BoxLayout.Y_AXIS));
        op(col1, "Escrever linha", doWriteLine);
        op(col1, "Escrever nas duas linhas", doWriteDisplay);
        op(col1, "Escrever na posição (li,col)", doPrintf);
        op(col1, "Escrever na posição do cursor", doWriteString);
        op(col1, "Escrever scroll", doScrollStart);
        op(col1, "Parar scroll", doScrollStop);
        op(col1, "Limpar display", doClear);
        op(col1, "Limpar display a partir de (li,col)", doEraseFrom);

        JPanel col2 = new JPanel();
        col2.setLayout(new BoxLayout(col2, BoxLayout.Y_AXIS));
        op(col2, "Cursor ON/OFF", doCursorToggle);
        op(col2, "Posicionar Cursor em Home", doHome);
        op(col2, "Posicionar cursor (li,col)", doSetCursor);
        op(col2, "Deslocar linhas", doShift);
        op(col2, "Gravar caracter customizado (CGRAM)", doCgram);
        op(col2, "Escrever caracter customizado gravado", doShowCgram);
        op(col2, "Bell", doBell);
        op(col2, "Diagnóstico", doDiag);

        JPanel columns = new JPanel(new java.awt.GridLayout(1, 2, 8, 0));
        columns.add(col1);
        columns.add(col2);
        side.add(columns, BorderLayout.CENTER);

        return side;
    }

    private interface Op { int run() throws Exception; }

    private void op(JPanel side, String label, Op task) {
        JButton b = addButton(side, label, e -> execute(label, task));
        opButtons.add(b);
        side.add(Box.createVerticalStrut(6));
    }

    private JButton addButton(JPanel side, String label, java.awt.event.ActionListener l) {
        JButton b = new JButton(label);
        b.setAlignmentX(Component.LEFT_ALIGNMENT);
        b.setMaximumSize(new Dimension(260, 32));
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
        lib.k044_aux_disable();
        lib.k044_mouse_disable();
        lib.k044_uinput_disable();
        lib.k044_close();
        connected = false;
    }

    private void toggleConnect() {
        if (connected) {
            disconnectDevice();
            connectBtn.setText("Conectar");
            setEnabledOps(false);
            log("Dispositivo fechado.");
            return;
        }
        log("libK044AVT " + lib.k044_version());
        int r = lib.k044_open();
        if (r != DisplayLib.K044_OK) {
            log("ERRO: k044_open falhou (" + r + "). Rode com sudo e o device conectado.");
            return;
        }

        /* Mantem teclado de 44 teclas, teclado auxiliar e mouse PS/2 vivos
         * enquanto este programa roda. Nenhuma das tres e fatal se falhar. */
        if (lib.k044_uinput_enable() != DisplayLib.K044_OK)
            log("Aviso: uinput indisponível (teclado não será repassado ao sistema).");
        if (lib.k044_mouse_enable() != DisplayLib.K044_OK)
            log("Aviso: mouse PS/2 indisponível.");
        lib.k044_start_event_loop();
        if (lib.k044_aux_enable() != DisplayLib.K044_OK)
            log("Aviso: teclado auxiliar PS/2 indisponível.");

        lib.k044_set_log_level(DisplayLib.K044_LOG_DEBUG);
        connected = true;
        connectBtn.setText("Desconectar");
        setEnabledOps(true);
        lcd.clear();
        log("Conectado ao display.");
    }

    /* ------------------------------------------------------------------ */
    /* Execução assíncrona                                                 */
    /* ------------------------------------------------------------------ */

    /** Roda a operação numa SwingWorker e reabilita os botões ao terminar. */
    private void execute(String label, Op task) {
        setEnabledOps(false);
        connectBtn.setEnabled(false);
        log("» " + label);

        new SwingWorker<Integer, Void>() {
            @Override protected Integer doInBackground() throws Exception {
                return task.run();
            }
            @Override protected void done() {
                try {
                    int rc = get();
                    log("   ret=" + rc + (rc == DisplayLib.K044_OK ? " (OK)" : " (falhou)"));
                } catch (Exception ex) {
                    log("   exceção: " + ex.getMessage());
                }
                setEnabledOps(true);
                connectBtn.setEnabled(true);
            }
        }.execute();
    }

    /* ------------------------------------------------------------------ */
    /* Operações —  17 operações do menu de exemplos                      */
    /* ------------------------------------------------------------------ */

    private Op doWriteLine = () -> {
        int row = askInt("Linha (0 ou 1)", 0);
        String text = askString("Texto (até 39 chars)", "Olá, K044AVT!");
        if (text == null) return cancelled();
        int r = lib.k044_write_line((byte) row, text);
        if (r == DisplayLib.K044_OK) lcd.setLine(row, text);
        return r;
    };

    private Op doWriteDisplay = () -> {
        String l1 = askString("Texto linha 0", "Linha 1");
        String l2 = askString("Texto linha 1", "Linha 2");
        if (l1 == null || l2 == null) return cancelled();
        int r = lib.k044_write_display(l1, l2);
        if (r == DisplayLib.K044_OK) { lcd.setLine(0, l1); lcd.setLine(1, l2); }
        return r;
    };

    private Op doPrintf = () -> {
        int row = askInt("Linha (0 ou 1)", 0);
        int col = askInt("Coluna (0-39)", 0);
        String text = askString("Texto (até 40 chars)", "teste");
        if (text == null) return cancelled();
        int r = lib.k044_write_pos((byte) row, (byte) col, "%s", text);
        if (r == DisplayLib.K044_OK) {
            for (int i = 0; i < text.length(); i++) lcd.setChar(row, col + i, text.charAt(i));
        }
        return r;
    };

    private Op doWriteString = () -> {
        String text = askString("Texto a escrever no cursor atual", "K044AVT");
        if (text == null) return cancelled();
        int r = lib.k044_write_string(text);
        if (r == DisplayLib.K044_OK) {
            int row = lcd.getCursorRow(), col = lcd.getCursorCol();
            for (int i = 0; i < text.length() && col < 40; i++, col++) lcd.setChar(row, col, text.charAt(i));
            lcd.setCursor(row, col);
        }
        return r;
    };

    private Op doSetCursor = () -> {
        int row = askInt("Linha (0 ou 1)", 0);
        int col = askInt("Coluna (0-39)", 0);
        int r = lib.k044_set_cursor((byte) row, (byte) col);
        if (r == DisplayLib.K044_OK) lcd.setCursor(row, col);
        return r;
    };

    private Op doCursorToggle = () -> {
        String[] options = { "Ligar cursor", "Desligar cursor" };
        int choice = JOptionPane.showOptionDialog(this, "Cursor", "Cursor ON/OFF",
                JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null, options, options[0]);
        if (choice < 0) return cancelled();
        int r = (choice == 1) ? lib.k044_cursor_off() : lib.k044_cursor_on();
        if (r == DisplayLib.K044_OK) lcd.setCursorVisible(choice != 1);
        return r;
    };

    private Op doClear = () -> {
        int r = lib.k044_clear();
        if (r == DisplayLib.K044_OK) lcd.clear();
        return r;
    };

    private Op doHome = () -> {
        int r = lib.k044_home();
        if (r == DisplayLib.K044_OK) lcd.setCursor(0, 0);
        return r;
    };

    private Op doScrollStart = () -> {
        int row = askInt("Linha (0 ou 1)", 0);
        int colStart = askInt("Coluna inicial da janela", 0);
        int width = askInt("Largura da janela", 20);
        String text = askString("Texto a rolar (contínuo)", "Letreiro de exemplo - K044AVT display LCD");
        int delay = askInt("Delay entre passos (ms)", 200);
        int repeat = askInt("Repetições (0 = infinito)", 0);
        if (text == null) return cancelled();
        log("   (scroll contínuo roda em thread própria da biblioteca — o painel"
                + " não acompanha ao vivo; use \"Parar scroll contínuo\" pra encerrar)");
        return lib.k044_scroll_start((byte) row, (byte) colStart, (byte) width, text, delay, repeat);
    };

    private Op doScrollStop = () -> lib.k044_scroll_stop();

    private Op doShift = () -> {
        int direction = askInt("Direção (0=esquerda, 1=direita)", 0);
        int times = askInt("Quantas vezes", 1);
        int r = DisplayLib.K044_OK;
        for (int i = 0; i < times && r == DisplayLib.K044_OK; i++) {
            r = lib.k044_display_shift(direction);
        }
        /* Este display é 2x40 sem margem de DDRAM oculta (ver CLAUDE.md) —
         * um shift nativo do HD44780 desloca as 40 colunas visíveis em si
         * mesmas (rotação), não revela texto "fora da tela" como em
         * displays com DDRAM maior que a janela visível. */
        if (r == DisplayLib.K044_OK) {
            for (int i = 0; i < times; i++) lcd.rotate(direction);
        }
        return r;
    };

    private static final String[] CGRAM_PATTERN_NAMES = {
        "Seta pra cima", "ç (c cedilha)", "Seta pra baixo", "Seta pra esquerda",
        "Seta pra direita", "ã (a til)", "Check/visto", "Bateria vazia", "Bateria cheia",
    };

    /** Diálogo modal com um botão por padrão, empilhados verticalmente (em vez
     * de JOptionPane.showOptionDialog, que alinha horizontalmente e deixava
     * as 9 opções apertadas/cortadas). Retorna -1 se fechado sem escolher. */
    private int askCgramPattern() {
        JDialog dialog = new JDialog(this, "Caractere customizado — escolha o padrão", true);
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
        panel.setBorder(BorderFactory.createEmptyBorder(12, 12, 12, 12));

        int[] result = { -1 };
        for (int i = 0; i < CGRAM_PATTERN_NAMES.length; i++) {
            int pattern = i + 1; // K044_CGRAM_PRESET_* começa em 1
            JButton b = new JButton(CGRAM_PATTERN_NAMES[i]);
            b.setAlignmentX(Component.LEFT_ALIGNMENT);
            b.setMaximumSize(new Dimension(240, 32));
            b.addActionListener(e -> {
                result[0] = pattern;
                dialog.dispose();
            });
            panel.add(b);
            if (i < CGRAM_PATTERN_NAMES.length - 1) panel.add(Box.createVerticalStrut(6));
        }

        dialog.add(panel);
        dialog.pack();
        dialog.setLocationRelativeTo(this);
        dialog.setVisible(true); // bloqueia (modal) até um botão ser clicado ou a janela fechada
        return result[0];
    }

    private Op doCgram = () -> {
        int pattern = askCgramPattern();
        if (pattern < 0) return cancelled();
        int slot = askInt("Slot da CGRAM a usar (0-7)", 0);
        if (slot < 0 || slot > 7) { log("   slot inválido."); return DisplayLib.K044_OK; }

        int r = lib.k044_write_cgram_preset((byte) pattern, (byte) slot);
        if (r != DisplayLib.K044_OK) return r;
        slotPattern[slot] = pattern;

        int row = askInt("Linha onde mostrar o caractere (0 ou 1)", 0);
        int col = askInt("Coluna (0-39)", 0);
        r = lib.k044_set_cursor((byte) row, (byte) col);
        if (r != DisplayLib.K044_OK) return r;
        r = lib.k044_write_char_raw((byte) slot);
        if (r == DisplayLib.K044_OK) lcd.setCustomChar(row, col, pattern);
        return r;
    };

    private Op doShowCgram = () -> {
        int slot = askInt("Slot da CGRAM já gravado (0-7)", 0);
        if (slot < 0 || slot > 7) { log("   slot inválido."); return DisplayLib.K044_OK; }
        int row = askInt("Linha onde mostrar o caractere (0 ou 1)", 0);
        int col = askInt("Coluna (0-39)", 0);
        int r = lib.k044_set_cursor((byte) row, (byte) col);
        if (r != DisplayLib.K044_OK) return r;
        r = lib.k044_write_char_raw((byte) slot);
        if (r == DisplayLib.K044_OK) lcd.setCustomChar(row, col, slotPattern[slot]);
        return r;
    };

    private Op doBell = () -> {
        int r = lib.k044_bell();
        log("   🔔");
        return r;
    };

    private Op doDiag = () -> {
        boolean dead = lib.k044_is_device_dead();
        log("   device_dead=" + (dead ? "sim" : "não"));

        byte[] id = new byte[2];
        int r = lib.k044_read_id(id);
        if (r == DisplayLib.K044_OK)
            log(String.format("   ID do dispositivo: 0x%02X 0x%02X", id[0], id[1]));
        else
            log("   k044_read_id falhou: " + r);

        ByteByReference major = new ByteByReference(), minor = new ByteByReference(),
                         patch = new ByteByReference();
        r = lib.k044_read_fw_ver(major, minor, patch);
        if (r == DisplayLib.K044_OK)
            log("   versão do firmware: " + (major.getValue() & 0xFF) + "."
                    + (minor.getValue() & 0xFF) + "." + (patch.getValue() & 0xFF));
        else
            log("   k044_read_fw_ver falhou: " + r);
        return DisplayLib.K044_OK;
    };

    private Op doEraseFrom = () -> {
        int row = askInt("Linha (0 ou 1)", 0);
        int col = askInt("Coluna (0-39)", 0);
        int r = lib.k044_set_cursor((byte) row, (byte) col);
        if (r != DisplayLib.K044_OK) return r;
        r = lib.k044_erase_eol();
        if (r == DisplayLib.K044_OK) lcd.eraseToEol(row, col);
        return r;
    };

    /* ------------------------------------------------------------------ */
    /* Helpers                                                             */
    /* ------------------------------------------------------------------ */

    /** Carrega uma imagem do classpath */
    private Image loadImage(String resourcePath) {
        try (InputStream in = getClass().getResourceAsStream(resourcePath)) {
            if (in == null) return null;
            return ImageIO.read(in);
        } catch (IOException e) {
            return null;
        }
    }

    /** Como loadImage(), mas devolve um ImageIcon já escalado (mantendo a
     * proporção original) pra caber em targetWidth de largura. */
    private ImageIcon loadScaledIcon(String resourcePath, int targetWidth) {
        Image img = loadImage(resourcePath);
        if (img == null) return new ImageIcon();
        int w = img.getWidth(null), h = img.getHeight(null);
        if (w <= 0 || h <= 0) return new ImageIcon();
        int targetHeight = (int) Math.round(targetWidth * (h / (double) w));
        return new ImageIcon(img.getScaledInstance(targetWidth, targetHeight, Image.SCALE_SMOOTH));
    }

    private int askInt(String msg, int def) {
        String s = JOptionPane.showInputDialog(this, msg, Integer.toString(def));
        if (s == null || s.trim().isEmpty()) return def;
        try { return Integer.parseInt(s.trim()); }
        catch (NumberFormatException e) { return def; }
    }

    /** Retorna null se o usuário cancelar o diálogo (diferente de string vazia). */
    private String askString(String msg, String def) {
        String s = JOptionPane.showInputDialog(this, msg, def);
        return (s == null) ? null : s;
    }

    private int cancelled() { log("   cancelado."); return DisplayLib.K044_OK; }

    private void setEnabledOps(boolean en) {
        for (JButton b : opButtons) b.setEnabled(en);
    }

    private void log(String s) {
        logArea.append(s + "\n");
        logArea.setCaretPosition(logArea.getDocument().getLength());
    }

    /* ------------------------------------------------------------------ */

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new LcdDemo().setVisible(true));
    }
}
