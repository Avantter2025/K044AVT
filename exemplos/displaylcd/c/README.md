# Exemplos em C — Módulo Display LCD (HD44780 2×40)

Conjunto de exemplos mínimos em C puro, cada um focado numa parte da API
de display/cursor/teclado de `display_driver.h` — sem GUI, sem
dependências além da própria `libK044AVT.so`.

## Estrutura

```
displaylcd/c/
├── Makefile
├── diagnostico_lcd.c          Diagnóstico básico: abre, limpa, escreve, posiciona cursor
├── captura_teclas.c           Captura e imprime no terminal as teclas pressionadas
├── exemplo_aux.c              Teclado auxiliar PS/2 (habilitar/desabilitar, eventos)
├── exemplo_eventos.c          Leitura assíncrona de eventos (event loop + callback)
├── exemplo_keyecho.c          Eco de teclas no display, incluindo modo PIN (exibe '*')
├── exemplo_posicao.c          Posicionamento de cursor, scroll de linha, escrita
├── test_display_basico.c      Operações básicas de escrita (char/linha/string)
├── test_display_cursor.c      Controle de cursor (posicionar, mostrar/ocultar)
├── test_display_keyecho_nav.c Keyecho com navegação de cursor (setas, backspace)
├── test_display_repeat.c      Comportamento de tecla mantida pressionada (autorepeat)
├── test_display_scroll.c      Scroll horizontal de texto numa linha
├── test_display_versao.c      Leitura da versão da biblioteca e do firmware
├── test_display_visual.c      Teste visual dos comandos de cursor (home, line feed, etc.)
└── test_display_visual_ctrl.c Igual ao anterior, incluindo backspace
```

## Compilar

```bash
cd exemplos/displaylcd/c
make
```

Gera todos os binários acima. Requer a biblioteca já instalada em
`/usr/local/lib` e os headers em `/usr/local/include` — ver
[../../../README.md](../../../README.md) ou
[../../../instrucao-inicial.md](../../../instrucao-inicial.md) para o
passo de instalação.

## Executar

```bash
sudo ./diagnostico_lcd
```

é o ponto de partida recomendado — confirma que a biblioteca abre o
dispositivo e escreve no display corretamente. Os demais binários (mesmo
nome do `.c` de origem, sem a extensão) testam um aspecto específico cada
um; rode qualquer um com `sudo ./<nome>`.

**Atenção**: use teclado e mouse USB durante `test_display_scroll` (ou
qualquer teste com scroll contínuo) — a rotina ocupa banda grande do canal
PS/2 e pode disputar com o teclado K044AVT nesse intervalo.

## Notas

- Todos os exemplos chamam `k044_open()`, e a maioria fecha com
  `k044_close()` ao final — se um exemplo travar ou for interrompido com
  `Ctrl+C` antes de fechar, rode-o de novo normalmente (não deixa o
  dispositivo em estado inconsistente entre execuções).
- `exemplo_aux.c` só deve ser usado com um teclado PS/2 auxiliar de fato
  conectado — habilitar sem o dispositivo físico presente pode travar
  aguardando resposta.
