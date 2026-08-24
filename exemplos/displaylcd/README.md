# Exemplos - Módulo Display LCD 2×40

Exemplos práticos de uso do display LCD do TEC44AVT em C e Java.

## 📋 Funcionalidades Demonstradas

- ✅ Inicialização do display
- ✅ Limpeza e posicionamento de cursor
- ✅ Escrita de texto
- ✅ Caracteres especiais
- ✅ Efeitos de cursor (blink, visível)
- ✅ Scroll e rolagem de texto
- ✅ Integração com teclado (key echo)
- ✅ Controle de backlight

## 🔹 Exemplos C

### 📁 Estrutura

```
cpp/
├── Makefile                         Arquivo de compilação
├── test_display_basico.c            Exemplo básico
├── test_display_cursor.c            Controle de cursor
├── test_display_blink.c             Cursor piscante
├── test_display_scroll.c            Scroll horizontal
├── test_display_bell.c              Efeito de campainha
├── test_display_keyecho_nav.c       Integração com teclado
├── test_display_printf.c            Printf no display
├── test_display_versao.c            Mostrar versão
├── test_display_visual.c            Testes visuais
├── test_display_repeat.c            Teste de repetição
├── test_display_str_long.c          Strings longas
└── test_display_cursor_blink.c      Cursor com blink
```

### 🚀 Compilação e Execução

```bash
cd cpp

# Compilar todos os testes
make

# Compilar teste específico
make test_display_basico

# Executar teste
sudo ./test_display_basico

# Limpar binários
make clean
```

### 📖 Descrição dos Testes

#### `test_display_basico` - Básico
- Inicializa o display
- Escreve mensagem simples
- Demonstra operações básicas

#### `test_display_cursor` - Controle de Cursor
- Posiciona cursor em diferentes posições
- Demonstra `k044_lcd_set_cursor()`
- Escreve em linhas 0 e 1

#### `test_display_scroll` - Scroll
- Demonstra scroll horizontal
- Efeito de movimento de texto

#### `test_display_keyecho_nav` - Com Teclado
- Integra leitura de teclado
- Eco de caracteres no display
- Navegação com setas
- Mais complexo, mostra integração completa

#### `test_display_printf` - Printf
- Escreve usando formatação similar a `printf()`
- Teste de vários formatos

#### `test_display_bell` - Efeitos
- Demonstra efeitos visuais
- Teste de backlight (se disponível)

### 🛠️ Variáveis de Compilação

```bash
# Com debug
make DEBUG=1

# Com otimizações
make OPTIMIZE=1

# Listar todos os testes disponíveis
make list
```

## 🔸 Exemplos Java

### 📁 Estrutura

```
java/
├── pom.xml                          Configuração Maven
├── README.md                        Documentação Java
├── src/
│   ├── main/java/
│   │   └── com/avanttec/displaylcd/
│   │       └── DisplayLib.java      (Binding JNA)
│   └── test/java/                   (Testes se houver)
└── target/
    └── k044-displaylcd-demo-1.0.0.jar
```

### 🚀 Compilação e Execução

```bash
cd java

# Compilar e empacotar
mvn clean package

# Executar aplicação
sudo java -Djna.library.path=../../../libk044avt/lib \
         -jar target/k044-displaylcd-demo-1.0.0.jar
```

### 📖 Características

- 🎨 Interface para controle de display
- ⌨️ Integração com teclado
- 🔤 Suporte a caracteres especiais
- ⚙️ Controle de cursor e efeitos
- 📊 Visualização de estado

### 🛠️ Opções Maven

```bash
# Compilar sem rodar testes
mvn clean package -DskipTests

# Executar testes
mvn test

# Limpar apenas
mvn clean
```

## 📊 Comparação: C vs Java

| Aspecto             | C                    | Java             |
|---------------------|----------------------|------------------|
| **Tipo**            | CLI Testes           | GUI/Aplicação    |
| **Performance**     | Nativa (mais rápido) | JVM (mais lento) |
| **Uso**             | Prototipagem/Debug   | Produção         |
| **Desenvolvimento** | Mais baixo nível     | Mais alto nível  |
| **Recursos**        | Leve                 | Pesado           |

## 🔧 Pré-requisitos

### C

```bash
# Compilador
sudo apt-get install -y build-essential

# Verificar instalação
gcc --version
make --version
```

### Java

```bash
# JDK + Maven
sudo apt-get install -y default-jdk maven

# Verificar instalação
java -version
mvn --version
```

### Biblioteca

```bash
# Verificar libK044AVT
ls -l /usr/local/lib/libK044AVT.so
ls -l /usr/local/include/display_driver.h
```

## 📚 API do Display LCD

### Funções Principais

```c
// Inicializar (feito automaticamente por k044_open())
int k044_lcd_clear(void);          // Limpar display

int k044_lcd_set_cursor(uint8_t row, uint8_t col);
// Posicionar cursor (linha 0-1, coluna 0-39)

int k044_lcd_write(const char *text);
// Escrever string

int k044_lcd_write_char(char c);
// Escrever caractere único
```

### Exemplo Básico (C)

```c
#include "display_driver.h"

int main() {
    k044_open();
    
    // Limpar e posicionar
    k044_lcd_clear();
    k044_lcd_set_cursor(0, 0);
    k044_lcd_write("Olá!");
    
    k044_lcd_set_cursor(1, 0);
    k044_lcd_write("TEC44AVT");
    
    // ... usar o display ...
    
    k044_close();
    return 0;
}
```

## 🎯 Como Começar

### Iniciante? Comece com:
1. **C**: `test_display_basico` - Simples
2. **Java**: Compilar e rodar o JAR

### Avançado? Explore:
1. **C**: `test_display_keyecho_nav.c` - Integração completa
2. **Java**: Modificar o binding JNA

## ⚠️ Notas Importantes

- 🔴 **Requer sudo** - Acesso ao hardware PS/2
- 🔴 **Exclusivo** - Apenas uma instância por vez
- 🔴 **Hardware** - TEC44AVT conectado obrigatório
- 💡 **Display HD44780** - 2 linhas × 40 caracteres

## 🐛 Troubleshooting

### Erro: "Cannot find libK044AVT"
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Display não aparece nada
- Verificar contraste (se houver potenciômetro)
- Testar com `test_display_basico`
- Verificar conexão do teclado

### Erro: "Port already in use"
- Apenas uma instância por vez
- Feche a anterior antes de abrir nova

### Caracteres estranhos
- Verificar encoding UTF-8
- Alguns caracteres especiais requerem códigos customizados

## 🤝 Modificando os Exemplos

### C

```bash
# Criar novo teste baseado em existente
cp test_display_basico.c meu_teste.c

# Editar e compilar
gcc -o meu_teste meu_teste.c -lK044AVT
sudo ./meu_teste
```

### Java

```bash
# Modificar DisplayLib.java
# Recompilar
mvn package

# Executar
sudo java -Djna.library.path=/usr/local/lib -jar target/seu-app.jar
```

## 📖 Documentação Completa

Para mais informações:
- Ver `display_driver.h` para API completa
- Consulte manual do display para controles avançados
- Exemplos em código-fonte têm comentários detalhados

---

**Última atualização**: 2026-08-24  
**Versão**: 1.0  
**Plataforma**: Linux (x86_64)  
**Display**: 2×40 caracteres
