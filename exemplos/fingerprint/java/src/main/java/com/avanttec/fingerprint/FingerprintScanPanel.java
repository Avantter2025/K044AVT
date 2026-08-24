package com.avanttec.fingerprint;

import javax.swing.JPanel;
import javax.swing.Timer;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GradientPaint;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.geom.Arc2D;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Rectangle2D;

/**
 * Painel Swing que desenha uma digital estilizada e a anima como um sensor de
 * leitura: uma barra de varredura percorre a digital de baixo para cima, com um
 * brilho pulsante, enquanto uma operação (cadastro/identificação/busca) está em
 * andamento. Ao terminar, pisca verde (sucesso) ou vermelho (falha).
 *
 * Toda a "imagem animada" é renderizada por código (Java2D) — não há asset
 * binário externo, então o exemplo é autocontido.
 */
public class FingerprintScanPanel extends JPanel {

    public enum State { IDLE, SCANNING, SUCCESS, FAIL }

    private State state = State.IDLE;
    private String status = "Aguardando";
    private double phase = 0.0;          // 0..1 posição da varredura
    private double pulse = 0.0;          // fase do brilho pulsante
    private int flashTicks = 0;          // contador para o flash de resultado
    private final Timer timer;

    public FingerprintScanPanel() {
        setPreferredSize(new Dimension(320, 400));
        setBackground(new Color(0x10, 0x14, 0x1c));
        timer = new Timer(1000 / 30, e -> tick());
        timer.start();
    }

    private void tick() {
        pulse += 0.12;
        if (state == State.SCANNING) {
            phase += 0.012;
            if (phase > 1.0) phase -= 1.0;
        }
        if (flashTicks > 0) {
            flashTicks--;
            if (flashTicks == 0 && (state == State.SUCCESS || state == State.FAIL)) {
                state = State.IDLE;
            }
        }
        repaint();
    }

    /* ---- API pública (chamar na EDT) ---- */

    public void startScanning(String msg) {
        state = State.SCANNING;
        status = msg;
        phase = 0.0;
        flashTicks = 0;
        repaint();
    }

    public void finish(boolean ok, String msg) {
        state = ok ? State.SUCCESS : State.FAIL;
        status = msg;
        flashTicks = 45;   // ~1.5s de flash antes de voltar a IDLE
        repaint();
    }

    public void idle(String msg) {
        state = State.IDLE;
        status = msg;
        flashTicks = 0;
        repaint();
    }

    /* ---- Render ---- */

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D) g.create();
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

        int w = getWidth(), h = getHeight();
        int cx = w / 2;
        int cy = h / 2 - 20;
        int rw = Math.min(w, h) / 2 - 30;   // "raio" horizontal da digital
        int rh = (int) (rw * 1.25);

        // Região oval da digital (usada como clip para a varredura)
        Shape print = new Ellipse2D.Double(cx - rw, cy - rh, rw * 2, rh * 2);

        Color base;
        switch (state) {
            case SUCCESS: base = new Color(0x35, 0xd0, 0x6b); break;
            case FAIL:    base = new Color(0xe0, 0x4f, 0x4f); break;
            case SCANNING:base = new Color(0x39, 0xa8, 0xff); break;
            default:      base = new Color(0x4a, 0x5a, 0x70); break;
        }

        // Brilho de fundo pulsante quando ativo
        if (state == State.SCANNING || flashTicks > 0) {
            double a = 0.25 + 0.15 * Math.sin(pulse);
            g2.setPaint(new Color(base.getRed(), base.getGreen(), base.getBlue(),
                    (int) (a * 120)));
            g2.fill(new Ellipse2D.Double(cx - rw - 12, cy - rh - 12,
                    (rw + 12) * 2, (rh + 12) * 2));
        }

        // Cristas da digital: arcos concêntricos + um "laço" central
        g2.setStroke(new BasicStroke(2.4f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
        g2.setColor(new Color(base.getRed(), base.getGreen(), base.getBlue(), 150));
        for (int i = 0; i < 9; i++) {
            double f = 0.30 + i * 0.078;
            double aw = rw * 2 * f;
            double ah = rh * 2 * f;
            double ax = cx - aw / 2;
            double ay = cy - ah / 2 - rh * 0.05;
            int start = 15 + (i % 2) * 12;
            int extent = 150 + (i % 3) * 20;
            g2.draw(new Arc2D.Double(ax, ay, aw, ah, start, extent, Arc2D.OPEN));
            g2.draw(new Arc2D.Double(ax, ay, aw, ah, start + 180, extent, Arc2D.OPEN));
        }
        // Núcleo (laço) da digital
        g2.draw(new Arc2D.Double(cx - rw * 0.18, cy - rh * 0.30,
                rw * 0.36, rh * 0.5, -20, 220, Arc2D.OPEN));

        // Barra de varredura + faixa de brilho, recortada à digital
        if (state == State.SCANNING) {
            Graphics2D gc = (Graphics2D) g2.create();
            gc.clip(print);
            double y = (cy + rh) - phase * (rh * 2);   // sobe de baixo p/ cima
            int bandH = 46;
            GradientPaint gp = new GradientPaint(
                    0, (float) (y - bandH), new Color(0x39, 0xa8, 0xff, 0),
                    0, (float) y, new Color(0x9a, 0xd8, 0xff, 160));
            gc.setPaint(gp);
            gc.fill(new Rectangle2D.Double(cx - rw, y - bandH, rw * 2, bandH));
            gc.setColor(new Color(0xd8, 0xef, 0xff, 230));
            gc.setStroke(new BasicStroke(2.5f));
            gc.draw(new java.awt.geom.Line2D.Double(cx - rw, y, cx + rw, y));
            gc.dispose();
        }

        // Contorno da digital
        g2.setStroke(new BasicStroke(1.5f));
        g2.setColor(new Color(base.getRed(), base.getGreen(), base.getBlue(), 90));
        g2.draw(print);

        // Ícone de resultado (check / x) durante o flash
        if (flashTicks > 0 && (state == State.SUCCESS || state == State.FAIL)) {
            g2.setStroke(new BasicStroke(6f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
            g2.setColor(base);
            int s = 26;
            if (state == State.SUCCESS) {
                g2.drawLine(cx - s, cy, cx - 4, cy + s - 4);
                g2.drawLine(cx - 4, cy + s - 4, cx + s + 6, cy - s + 2);
            } else {
                g2.drawLine(cx - s, cy - s, cx + s, cy + s);
                g2.drawLine(cx - s, cy + s, cx + s, cy - s);
            }
        }

        // Texto de status
        g2.setColor(new Color(0xdf, 0xe7, 0xf0));
        g2.setFont(getFont().deriveFont(15f));
        int tw = g2.getFontMetrics().stringWidth(status);
        g2.drawString(status, cx - tw / 2, cy + rh + 34);

        g2.dispose();
    }
}
