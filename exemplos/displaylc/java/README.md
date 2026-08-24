# Exemplo Java (Swing + JNA) — Display LCD (2x40)

GUI de demonstração do módulo de display do dispositivo Avanttec (TEC44AVT),
falando com o LCD **através do driver** `libK044AVT.so` (JNA) — o lado Java só
usa as rotinas `k044_*` de escrita/cursor/scroll/CGRAM, sem tocar no protocolo
PS/2 bruto.

O painel central (`LcdPanel`) mostra visualmente um display 2x40, atualizado a
cada operação que a própria GUI executa.

## Estrutura

- `DisplayLib.java` — interface JNA mapeando `k044_open/close/version` e as
  rotinas de display (`k044_write_line`, `k044_write_display`, `k044_printf`,
  `k044_write_string`, `k044_set_cursor`, `k044_cursor_on/off`, `k044_clear`,
  `k044_home`, `k044_erase_eol`, `k044_scroll_line`, `k044_scroll_start/stop`,
  `k044_display_shift`, `k044_write_cgram_preset`, `k044_write_char_raw`,
  `k044_bell`, `k044_read_id`, `k044_read_fw_ver`), mais o pequeno
  subconjunto de ciclo de vida/uinput/mouse/aux/event-loop necessário —
  mesmo padrão de `exemplos/fingerprint/java/FingerprintLib.java`: um binding
  JNA próprio do módulo, sem depender do binding compartilhado
  `exemplos/java`.
- `LcdPanel.java` — painel visual do display 2x40 (Java2D puro, sem asset
  externo): grade de 40×2 células em fonte monoespaçada, cursor piscando,
  caracteres customizados da CGRAM desenhados como matriz de pontos 5x8.
- `LcdDemo.java` — janela principal com os botões das 17 operações (mesmo
  conjunto do menu de `exemplos/displaylcd/cpp/lcd.cpp`); cada operação roda
  numa thread de fundo (`SwingWorker`) pra manter a UI responsiva.

## Compilar

### Via Maven (terminal)

```bash
mvn clean package
```

Gera um **fat-JAR executável** em `target/k044-displaylcd-demo-1.0.0.jar`
(inclui a JNA e todas as dependências — é autocontido). A primeira execução do
Maven precisa de acesso à internet para baixar os plugins/dependências; depois
disso funciona offline.

### Via NetBeans IDE

1. Abra o projeto no NetBeans (File → Open Project, aponte para esta pasta)
2. Right-click no projeto → **Clean and Build**
3. O JAR aparece em `target/k044-displaylcd-demo-1.0.0.jar`

### Alternativa sem empacotar (compile-only, offline)

```bash
mvn -o compile
```

Compila apenas as classes em `target/classes`, sem criar o JAR. Útil se Maven
está offline. Rode depois direto pelo classpath (ver seção de Execução abaixo).

## Executar

**Importante:** `k044_open()` acessa portas de I/O, então **sempre precisa de
`sudo`**. A propriedade `-Djna.library.path` aponta para o diretório com
`libK044AVT.so`.

### Via fat-JAR (recomendado)

```bash
sudo java -Djna.library.path= driver_display \
     -jar target/k044-displaylcd-demo-1.0.0.jar
```

Ou, a partir da pasta `java/`, use o caminho relativo:

```bash
sudo java -Djna.library.path= driver_display \
     -jar target/k044-displaylcd-demo-1.0.0.jar
```

O fat-JAR é **autocontido** — leva a JNA dentro, não precisa de dependências
externas além de `libK044AVT.so`.

### Via classes compiladas (offline, sem Maven)

```bash
JNA=$(find ~/.m2 -name 'jna-5.14.0.jar' | head -1)
sudo java -Djna.library.path= driver_display \
     -cp "target/classes:$JNA" com.avanttec.displaylcd.LcdDemo
```

### Distribuindo o JAR

Se quiser copiar `target/k044-displaylcd-demo-1.0.0.jar` para outro lugar e
rodar lá, use sempre um **caminho absoluto** para o `jna.library.path`:

```bash
sudo java -Djna.library.path=/caminho/absoluto/para/driver_display \
     -jar k044-displaylcd-demo-1.0.0.jar
```

### Usando NetBeans para rodar

1. Right-click no projeto → Run
2. Ou configure a variável `jna.library.path` nas propriedades do projeto
   (Project Properties → Run → VM Options):
   ```
   -Djna.library.path=/home/paulo/Projetos/projetos-ubuntu/avanttec_project/driver_display
   ```

### Interface da aplicação

É uma GUI **desktop** (Swing), então precisa de um ambiente gráfico (X11/Wayland).
Na janela:
1. Clique em **Conectar** (tenta abrir o dispositivo)
2. Se conectado, os botões de operação ficam ativos e o painel do display
   (topo) começa a refletir o que for escrito
3. Use os botões de escrita/cursor/scroll/CGRAM — cada um pede os parâmetros
   necessários por diálogo (linha, coluna, texto, etc.)
4. **Caractere customizado (CGRAM)** grava um dos 9 padrões pré-definidos
   (setas, ç, ã, check, bateria) num slot (0-7) e mostra na posição pedida;
   **Escrever caractere já gravado** reaproveita um slot já gravado numa
   chamada anterior, sem regravar a CGRAM
5. Nota: `k044_aux_enable()` só deve ter efeito com o teclado auxiliar PS/2
   fisicamente conectado — sem isso o firmware pode travar esperando um
   sinal que nunca chega (mesmo aviso de `k044_aux_enable()`/`CLAUDE.md`)

## Referência

A API completa está documentada em `driver_display/display_driver.h`. O
exemplo CLI equivalente em C++ está em `../cpp/lcd.cpp`.
