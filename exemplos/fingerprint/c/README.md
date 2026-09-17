# Exemplo em C — Módulo de Impressão Digital

Quatro exemplos mínimos em C puro, usando diretamente a API pública de
`display_driver.h` (`k044_fp_*`) — sem GUI, sem dependências além da
própria `libK044AVT.so`.

## Estrutura

```
fingerprint/c/
├── Makefile
├── test_fingerprint.c          Menu completo do módulo (info, cadastro, busca, LED, diagnóstico)
├── test_fingerprint_led.c      Teste isolado do LED do sensor (cor + estado)
├── test_fingerprint_lock.c     Teclado bloqueado até identificação por digital
└── test_fingerprint_lock_v2.c  Igual ao anterior, com eco de tecla via k044_keyecho_start()
```

## Compilar

```bash
cd exemplos/fingerprint/c
make
```

Gera os quatro binários: `test_fingerprint`, `test_fingerprint_led`,
`test_fingerprint_lock` e `test_fingerprint_lock_v2`. Requer a biblioteca
já instalada em `/usr/local/lib` e os headers em `/usr/local/include` —
ver [../../../README.md](../../../README.md) ou
[../../../instrucao-inicial.md](../../../instrucao-inicial.md) para o
passo de instalação.

## Executar

### `test_fingerprint` — menu completo

```bash
sudo ./test_fingerprint
```

Usa as funções de alto nível `k044_fp_enroll()`/`k044_fp_search_retry()`
(o fluxo de esperar o dedo, capturar, retirar, capturar de novo e mesclar
fica todo dentro da biblioteca). Ativa teclado/mouse/event loop/auxiliar
junto, então o teclado continua funcionando normalmente durante o uso do
menu. Opções disponíveis:

1. Informações do módulo
2. Contar templates cadastrados
3. Cadastrar digital (3 leituras)
4. Identificar digital
5. Buscar digital no banco
6. Remover um template
7. Apagar todo o banco
8. Sleep
9. Diagnóstico de estado
10. Controlar LED do sensor

### `test_fingerprint_led` — só o LED

```bash
sudo ./test_fingerprint_led
```

Menu em dois passos independentes — escolher a **cor** (Azul/Verde/
Vermelho) e escolher o **estado** (Aceso/Apagado/Rápido/Respirando).
Qualquer um dos dois passos aplica no sensor imediatamente, sempre com a
cor e o estado atuais combinados no mesmo comando.

### `test_fingerprint_lock` — teclado bloqueado até identificação

```bash
sudo ./test_fingerprint_lock
```

Cadastre pelo menos uma digital antes (`test_fingerprint`, opção 3). Ao
iniciar, o programa **não** ativa `uinput`/mouse/auxiliar — o teclado
fica capturado pela biblioteca mas não é repassado ao sistema (nenhuma
tecla funciona em outros programas). Ele fica chamando
`k044_fp_search_retry()` em loop até reconhecer uma digital já
cadastrada no banco; só então ativa `uinput`/mouse/event loop/auxiliar,
liberando o teclado (principal e auxiliar) para o resto do sistema.
Ctrl+C cancela a qualquer momento.

Não existe, nesta biblioteca, um comando de baixo nível equivalente ao
`0xF5` (Disable Scanning) do protocolo PS/2 padrão para o teclado
principal — o "bloqueio" demonstrado aqui é feito no nível de injeção
no sistema (uinput), não no barramento PS/2 em si.

**Atenção**: depois de liberado, as teclas pressionadas (principal ou
auxiliar) são digitadas de verdade no terminal onde o programa está
rodando, via `uinput` — é assim que se confirma visualmente que o
teclado foi liberado. Por isso o programa **não** imprime mais os
eventos no terminal depois da liberação (ficaria misturado com o que
você digitar) e **não** faz mais nenhuma escrita no display nem leitura
extra do barramento PS/2 (nem para atualizar o display) — qualquer
transação nossa ali disputaria banda com o escaneamento do teclado e
atrasaria a digitação. A última mensagem do display ("Acesso liberado")
fica estática até o programa encerrar.

### `test_fingerprint_lock_v2` — igual, com eco de tecla no display

```bash
sudo ./test_fingerprint_lock_v2
```

Mesmo fluxo de bloqueio/identificação do `test_fingerprint_lock`. A
diferença é o que acontece depois de liberado: em vez de deixar o
display parado, usa `k044_keyecho_start()` para mostrar o que é digitado
na linha 2 do display em tempo real — ESC limpa o campo, Enter confirma
(callback só imprime no terminal, sem tocar o barramento). Isso funciona
rápido e sem atrasar o teclado porque o keyecho roda numa thread própria,
dedicada, dentro da biblioteca, com uma via de escrita otimizada para
isso — o mesmo mecanismo que faz o `test_display_keyecho_nav.c`
(`exemplos/displaylcd/c/`) responder bem com teclado e display ativos ao
mesmo tempo. Veja a nota abaixo sobre por que **não** escrever no display
manualmente por tecla.

## Notas

- Os quatro programas fazem `k044_open()`. `test_fingerprint` e os dois
  `test_fingerprint_lock*` (depois de liberado) também ativam
  `uinput`/mouse/event loop/teclado auxiliar — necessário porque leem a
  entrada do usuário pelo terminal enquanto o dispositivo está aberto, e
  `k044_open()` toma temporariamente o controle da porta PS/2 primária.
- Sem o sensor de impressão digital conectado, `k044_fp_init()` falha e o
  programa encerra com uma mensagem de erro.
- **Cuidado com escritas no display (ou qualquer outra rotina que use o
  barramento PS/2) dentro de um loop acionado por tecla.** Foi exatamente
  isso que causou lentidão perceptível na digitação numa versão inicial
  do `test_fingerprint_lock`: chamar `k044_write_pos()` a cada evento
  `K044_EVT_KEY_MAKE` (inclusive em rajadas de autorepeat) fazia a
  escrita no display disputar o mesmo barramento usado para escanear o
  teclado, atrasando a resposta. Duas soluções válidas, dependendo do que
  se precisa: `test_fingerprint_lock` simplesmente para de tocar no
  barramento depois de liberado (nem ler evento, nem escrever no
  display); `test_fingerprint_lock_v2` mostra como ter eco de tecla no
  display **e** desempenho correto ao mesmo tempo, usando
  `k044_keyecho_start()` em vez de escrita manual — vale como referência
  para qualquer exemplo futuro que precise reagir a teclas em tempo real.
