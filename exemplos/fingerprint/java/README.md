# Exemplo Java (Swing + JNA) — Módulo de Impressão Digital

GUI de demonstração do módulo de digital do dispositivo Avanttec (TEC44AVT),
falando com o sensor **através do driver** `libK044AVT.so` (JNA) — o firmware do
TEC44AVT faz o passthrough PS/2 `0xAD`/`0xAE` ⇄ UART do módulo, então o lado Java
só usa as rotinas `k044_fp_*`.

Durante cada leitura (cadastro/identificação/busca), o painel exibe uma
**animação de varredura da digital** (renderizada em Java2D, sem asset externo).

## Estrutura

- `FingerprintLib.java` — interface JNA mapeando `k044_open/close/version` e as
  rotinas `k044_fp_*` (init, get_image, gen_char, match, search, store, load,
  delete, empty, valid_template_count, read_nfpage, sleep, auto_enroll,
  auto_identify, set_callback).
- `NFPage.java` — `Structure` JNA de `k044_fp_nfpage_t` (info do módulo).
- `FingerprintScanPanel.java` — painel animado do sensor (varredura + brilho
  pulsante, flash verde/vermelho no resultado).
- `FingerDemo.java` — janela principal com os botões das rotinas; cada operação
  roda numa thread de fundo (`SwingWorker`) enquanto a animação toca.

## Compilar

### Via Maven (terminal)

```bash
mvn clean package
```

Gera um **fat-JAR executável** em `target/k044-fingerprint-demo-1.0.0.jar`
(inclui a JNA e todas as dependências — é autocontido). A primeira execução do
Maven precisa de acesso à internet para baixar os plugins/dependências; depois
disso funciona offline.

### Via NetBeans IDE

1. Abra o projeto no NetBeans (File → Open Project, aponte para esta pasta)
2. Right-click no projeto → **Clean and Build**
3. O JAR aparece em `target/k044-fingerprint-demo-1.0.0.jar`

### Alternativa sem empacotar (compile-only, offline)

```bash
mvn -o compile
```

Compila apenas as classes em `target/classes`, sem criar o JAR. Útil se Maven
está offline. Rode depois direto pelo classpath (ver seção de Execução abaixo).

## Executar

**Importante:** `k044_open()` acessa portas de I/O, então **sempre precisa de `sudo`**.
A propriedade `-Djna.library.path` aponta para o diretório com `libK044AVT.so`.

### Via fat-JAR (recomendado)

```bash
sudo java -Djna.library.path=/home/SEU_USERNAME/driver_display \
     -jar target/k044-fingerprint-demo-1.0.0.jar
```

Ou, a partir da pasta `java/`, use o caminho relativo:

```bash
sudo java -Djna.library.path= driver_display \
     -jar target/k044-fingerprint-demo-1.0.0.jar
```

O fat-JAR é **autocontido** — leva a JNA dentro, não precisa de depedências
externas além de `libK044AVT.so`.

### Via classes compiladas (offline, sem Maven)

```bash
JNA=$(find ~/.m2 -name 'jna-5.14.0.jar' | head -1)
sudo java -Djna.library.path= driver_display \
     -cp "target/classes:$JNA" com.avanttec.fingerprint.FingerDemo
```

### Distribuindo o JAR

Se quiser copiar `target/k044-fingerprint-demo-1.0.0.jar` para outro lugar e
rodar lá, use sempre um **caminho absoluto** para o `jna.library.path`:

```bash
sudo java -Djna.library.path=/caminho/absoluto/para/driver_display \
     -jar k044-fingerprint-demo-1.0.0.jar
```

### Usando NetBeans para rodar

1. Right-click no projeto → Run
2. Ou configure a variável `jna.library.path` nas propriedades do projeto
   (Project Properties → Run → VM Options):
   ```
   -Djna.library.path=/home/SEU_USERNAME/driver_display
   ```

### Interface da aplicação

É uma GUI **desktop** (Swing), então precisa de um ambiente gráfico (X11/Wayland).
Na janela:
1. Clique em **Conectar** (tenta abrir o dispositivo)
2. Se conectado, os botões de operação ficam ativos
3. Use **Cadastrar**, **Identificar**, **Buscar**, **Contar**, **Info**, **Remover**,
   **Apagar** ou **Sleep**
4. Enquanto uma operação roda, o painel de varredura (centro) exibe a animação

## Referência do protocolo

O protocolo bruto do sensor está DEVIDAMENTE documentado.
