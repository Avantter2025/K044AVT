/*******************************************************************************
 * @file      LcdPanel.java
 * @brief     Painel visual Swing (Java2D) representando o display 2x40.
 * @project   Teclado de 44 Teclas PS/2 (LCD 2x40, Biometria e Teclado Auxiliar)
 * @author    Cariyl Kirsten <projetos@avanttectecnologia.com.br>
 * @company   Avanttec Tecnologia Ltda. - www.avanttectecnologia.com.br
 * @date      19/08/2026
 * @version   v1.0.0
 *
 * @details
 * Painel visual representando o display físico HD44780 2x40 — Java2D puro,
 * sem assets externos (mesmo princípio de FingerprintScanPanel em
 * exemplos/fingerprint/java/: nenhuma imagem/ícone bundled).
 *
 * A biblioteca não expõe nenhuma função de "ler o texto atual do display"
 * (k044_read_cgram() só lê CGRAM, não DDRAM/texto — e mesmo essa leitura se
 * mostrou pouco confiável neste hardware, ver CLAUDE.md), então este painel
 * mantém um modelo local do que a própria aplicação escreveu por último,
 * atualizado explicitamente por LcdDemo a cada operação bem-sucedida — não
 * é uma leitura ao vivo do hardware, é a representação do que deveria estar
 * lá segundo os comandos já enviados.
 *
 * @target    Linux (x86_64 / Industrial PC)
 *
 * @copyright (c) 2026 Avanttec Tecnologia. Todos os direitos reservados.
 ******************************************************************************/
package com.avanttec.displaylcd;

import javax.swing.JPanel;
import javax.swing.Timer;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.geom.RoundRectangle2D;

public class LcdPanel extends JPanel {

    private static final int ROWS = 2;
    private static final int COLS = 40;

    private static final Color COLOR_BEZEL     = new Color(0x2B, 0x2B, 0x2B);
    private static final Color COLOR_SCREEN_BG = new Color(0x0F, 0x3D, 0x0F);
    private static final Color COLOR_TEXT      = new Color(0x8C, 0xFF, 0x6B);
    private static final Color COLOR_TEXT_DIM  = new Color(0x1E, 0x55, 0x1E);
    private static final Color COLOR_CURSOR    = new Color(0xC8, 0xFF, 0xB0);

    /**
     * Bitmaps 5x8 dos padrões pré-gravados em ROM — índice 1-9, casando com
     * DisplayLib.K044_CGRAM_PRESET_*. Usado só pra desenhar visualmente o
     * caractere customizado no slot certo; não é lido do hardware.
     */
    private static final int[][] PRESET_BITMAPS = {
        null, // índice 0 não usado (presets começam em 1)
        { 0x04, 0x0E, 0x15, 0x04, 0x04, 0x04, 0x04, 0x00 }, // 1 ARROW_UP
        { 0x00, 0x0E, 0x10, 0x10, 0x10, 0x0E, 0x04, 0x0C }, // 2 C_CEDILLA
        { 0x00, 0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04 }, // 3 ARROW_DOWN
        { 0x00, 0x04, 0x0C, 0x1F, 0x0C, 0x04, 0x00, 0x00 }, // 4 ARROW_LEFT
        { 0x00, 0x04, 0x06, 0x1F, 0x06, 0x04, 0x00, 0x00 }, // 5 ARROW_RIGHT
        { 0x0A, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00 }, // 6 A_TILDE
        { 0x00, 0x00, 0x01, 0x02, 0x14, 0x08, 0x00, 0x00 }, // 7 CHECK
        { 0x0E, 0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F }, // 8 BATTERY_EMPTY
        { 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }, // 9 BATTERY_FULL
    };

    /** Sentinela: valores 1-9 no grid significam "desenhar o bitmap do
     * preset N aqui" em vez de um caractere de texto normal  */
    private static boolean isCustomSlotMarker(char c) {
        return c >= 1 && c <= 9;
    }

    private final char[][] grid = new char[ROWS][COLS];
    private int cursorRow = 0;
    private int cursorCol = 0;
    private boolean cursorVisible = false;
    private boolean blinkOn = true;

    private final Timer blinkTimer;

    public LcdPanel() {
        clear();
        setBackground(COLOR_BEZEL);
        setPreferredSize(new Dimension(COLS * 16 + 24, ROWS * 34 + 24));

        blinkTimer = new Timer(500, e -> {
            blinkOn = !blinkOn;
            if (cursorVisible) repaint();
        });
        blinkTimer.start();
    }

    /** Escreve uma linha inteira (0 ou 1), truncando/preenchendo até 40 colunas. */
    public void setLine(int row, String text) {
        if (row < 0 || row >= ROWS) return;
        char[] r = grid[row];
        int n = text == null ? 0 : text.length();
        for (int c = 0; c < COLS; c++) {
            r[c] = (c < n) ? text.charAt(c) : ' ';
        }
        repaint();
    }

    /** Escreve um único caractere numa posição, sem afetar o resto da linha. */
    public void setChar(int row, int col, char c) {
        if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return;
        grid[row][col] = c;
        repaint();
    }

    /** Apaga da coluna indicada até o fim da linha. */
    public void eraseToEol(int row, int col) {
        if (row < 0 || row >= ROWS) return;
        for (int c = Math.max(0, col); c < COLS; c++) grid[row][c] = ' ';
        repaint();
    }

    /** Roda as 40 colunas de ambas as linhas em si mesmas — equivalente
     * visual do shift nativo do HD44780: 
     * direction=0 desloca pra esquerda  direction=1 desloca pra direita. */
    public void rotate(int direction) {
        for (char[] r : grid) {
            if (direction == 1) {
                char last = r[COLS - 1];
                System.arraycopy(r, 0, r, 1, COLS - 1);
                r[0] = last;
            } else {
                char first = r[0];
                System.arraycopy(r, 1, r, 0, COLS - 1);
                r[COLS - 1] = first;
            }
        }
        repaint();
    }

    /** Marca uma posição pra desenhar o bitmap do padrão CGRAM pré-gravado
     * ( 1-9) em vez de um caractere de texto — usado pelos itens 
     * "Caractere customizado"/"Escrever  caractere já gravado". */
    public void setCustomChar(int row, int col, int presetId) {
        if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return;
        if (presetId < 1 || presetId >= PRESET_BITMAPS.length) return;
        grid[row][col] = (char) presetId;
        repaint();
    }

    public int getCursorRow() { return cursorRow; }
    public int getCursorCol() { return cursorCol; }

    /** Limpa as duas linhas (equivalente visual a k044_clear()). */
    public void clear() {
        for (char[] r : grid) java.util.Arrays.fill(r, ' ');
        cursorRow = 0;
        cursorCol = 0;
        repaint();
    }

    public void setCursor(int row, int col) {
        cursorRow = Math.max(0, Math.min(ROWS - 1, row));
        cursorCol = Math.max(0, Math.min(COLS - 1, col));
        repaint();
    }

    public void setCursorVisible(boolean on) {
        cursorVisible = on;
        blinkOn = true;
        repaint();
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D) g.create();
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);

        int w = getWidth();
        int h = getHeight();

        g2.setColor(COLOR_BEZEL);
        g2.fillRect(0, 0, w, h);

        int pad = 10;
        int screenX = pad, screenY = pad;
        int screenW = w - 2 * pad, screenH = h - 2 * pad;
        g2.setColor(COLOR_SCREEN_BG);
        g2.fill(new RoundRectangle2D.Float(screenX, screenY, screenW, screenH, 8, 8));

        float cellW = screenW / (float) COLS;
        float cellH = screenH / (float) ROWS;

        Font font = new Font(Font.MONOSPACED, Font.BOLD, Math.max(8, (int) (cellH * 0.62f)));
        g2.setFont(font);
        FontMetrics fm = g2.getFontMetrics();

        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                float cellX = screenX + col * cellW;
                float cellY = screenY + row * cellH;

                char c = grid[row][col];
                boolean isCursorCell = cursorVisible && blinkOn && row == cursorRow && col == cursorCol;

                if (isCursorCell) {
                    g2.setColor(COLOR_CURSOR);
                    g2.fillRect((int) cellX + 1, (int) (cellY + cellH * 0.78f),
                                (int) cellW - 2, Math.max(2, (int) (cellH * 0.12f)));
                }

                if (isCustomSlotMarker(c)) {
                    drawCustomGlyph(g2, PRESET_BITMAPS[c], cellX, cellY, cellW, cellH,
                                     isCursorCell ? COLOR_CURSOR : COLOR_TEXT);
                } else if (c != ' ' && c != 0) {
                    g2.setColor(isCursorCell ? COLOR_CURSOR : COLOR_TEXT);
                    int tx = (int) (cellX + (cellW - fm.charWidth(c)) / 2f);
                    int ty = (int) (cellY + (cellH + fm.getAscent() - fm.getDescent()) / 2f);
                    g2.drawString(String.valueOf(c), tx, ty);
                } else {
                    // Traço bem sutil marcando a célula vazia (visual autêntico de LCD segmentado)
                    g2.setColor(COLOR_TEXT_DIM);
                    int ty = (int) (cellY + cellH * 0.86f);
                    g2.drawLine((int) (cellX + cellW * 0.15f), ty, (int) (cellX + cellW * 0.85f), ty);
                }
            }
        }

        g2.dispose();
    }

    /** Desenha o bitmap 5x8 de um padrão CGRAM como uma matriz de pontos,
     * do mesmo jeito que o HD44780 real renderiza um caractere customizado
     * (cada byte de bitmap = 1 linha, 5 bits menos significativos usados). */
    private void drawCustomGlyph(Graphics2D g2, int[] bitmap, float cellX, float cellY,
                                  float cellW, float cellH, Color color) {
        int bitmapCols = 5, bitmapRows = 8;
        float dotAreaW = cellW * 0.7f, dotAreaH = cellH * 0.8f;
        float dotX0 = cellX + (cellW - dotAreaW) / 2f;
        float dotY0 = cellY + (cellH - dotAreaH) / 2f;
        float dotW = dotAreaW / bitmapCols, dotH = dotAreaH / bitmapRows;
        float dotSize = Math.min(dotW, dotH) * 0.85f;

        g2.setColor(color);
        for (int row = 0; row < bitmapRows; row++) {
            int bits = bitmap[row];
            for (int col = 0; col < bitmapCols; col++) {
                boolean on = (bits & (1 << (bitmapCols - 1 - col))) != 0;
                if (!on) continue;
                float dx = dotX0 + col * dotW + (dotW - dotSize) / 2f;
                float dy = dotY0 + row * dotH + (dotH - dotSize) / 2f;
                g2.fillOval((int) dx, (int) dy, (int) dotSize, (int) dotSize);
            }
        }
    }
}
