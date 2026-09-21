# K044AVT - Teclado Programável TEC44AVT

**Repositório completo** com biblioteca nativa compilada, aplicativos e exemplos práticos para o teclado programável **TEC44AVT** com funcionalidades avançadas (biometria, display LCD, 44 teclas programáveis).

## 📦 Conteúdo do Repositório

```
K044AVT/
├── driver64/                Biblioteca nativa (64-bits) + headers
│   ├── lib/libK044AVT.so
│   └── include/ (display_driver.h, k044avt.hpp)
│
├── driver32/                Biblioteca nativa (32-bits) + headers
│   ├── lib/libK044AVT.so
│   └── include/ (display_driver.h, k044avt.hpp)
│
├── reprog/                  Aplicativo Java para reprogramação de scancodes
│   └── java/k044-reprog-demo-1.0.0.jar
│
├── instrucao-inicial.md    Guia de instalação e execução passo a passo
├── API libK044AVT.pdf      Referência da API (Biblioteca de funções)
│
└── exemplos/                Exemplos práticos em C, C++ e Java
    ├── README.md            Guia completo dos exemplos
    │
    ├── fingerprint/         Leitor de impressão digital
    │   ├── c/               (test_fingerprint, test_fingerprint_led)
    │   ├── cpp/             (finger2_v2 e outros)
    │   └── java/            (GUI Swing)
    │
    └── displaylcd/          Display LCD (HD44780 2×40)
        ├── c/               (diagnostico_lcd e outros testes)
        ├── cpp/             (lcd)
        └── java/            (GUI Swing)
```

## ⚠️ Antes de começar — leia isto

**NÃO execute nenhum aplicativo se o teclado K044AVT não estiver conectado
na porta PS/2.** Isso deixa as portas PS/2 do computador inoperantes até a
recuperação abaixo (é necessário um **teclado e mouse USB** disponíveis
para digitar os comandos):

```bash
sudo bash -c 'echo -a "i8042" > /sys/bus/platform/drivers/i8042/unbind'
sudo bash -c 'echo -a "i8042" > /sys/bus/platform/drivers/i8042/bind'
```

## 🚀 Instalação Rápida

### 1. Instalar as ferramentas de compilação

```bash
sudo apt update
sudo apt install -y build-essential pkgconf libevdev-dev openjdk-17-jdk maven
```

Configure o `JAVA_HOME` no perfil do seu usuário (`nano ~/.bashrc`, acrescente
ao final e depois rode `source ~/.bashrc`) — use a linha correspondente à sua
arquitetura, confirmada no passo 2 abaixo:

```bash
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64   # 64-bits (x86_64)
#export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-i386   # 32-bits (i686/i386)
export PATH=$JAVA_HOME/bin:$PATH
```

Valide as ferramentas: `java -version`, `gcc --version`, `mvn -version`.

### 2. Confirmar a arquitetura e instalar a biblioteca

```bash
uname -m
# x86_64        -> 64-bits
# i686 / i386   -> 32-bits
```

Clone o repositório na sua pasta pessoal (`~/avanttec`) e instale a
biblioteca correspondente à arquitetura confirmada acima:

```bash
git clone https://github.com/Avantter2025/K044AVT.git ~/avanttec
cd ~/avanttec/driver64    # se 64-bits
# cd ~/avanttec/driver32  # se 32-bits

sudo cp lib/libK044AVT.so          /usr/local/lib/
sudo cp include/display_driver.h   /usr/local/include/
sudo cp include/k044avt.hpp        /usr/local/include/
sudo ldconfig
```

Confirme que a arquitetura da biblioteca instalada bate com a do sistema:

```bash
file /usr/local/lib/libK044AVT.so
# deve mostrar "ELF 64-bit" (x86_64) ou "ELF 32-bit" (i386), conforme o passo acima
```

### 3. Explorar Exemplos

```bash
cd ~/avanttec/exemplos

# Fingerprint (C)
cd fingerprint && sudo make -C ./c && sudo ./c/test_fingerprint

# Ou Display LCD (C)
cd ../displaylcd && sudo make -C ./c && sudo ./c/diagnostico_lcd
```

Veja [exemplos/README.md](exemplos/README.md) para as variantes C++ e Java
de cada módulo.

## ✨ Funcionalidades Disponíveis

| Módulo                 | Descrição                      | Status        |
|------------------------|---------------------------|---------------|
| **Teclado 44 teclas**  | Programável via PS2       | ✅ Funcional  |
| **Leitor Fingerprint** | Enroll, Search, Delete    | ✅ Funcional  |
| **Display LCD**        | HD44780 2×40 caracteres   | ✅ Funcional  |
| **Teclado Auxiliar**   | PS/2 externo              | ✅ Funcional  |
| **LEDs de Status**     | Controle de indicadores   | ✅ Funcional  |
| **PIN Pad**            | Teclado numérico          | 🔄 USO FUTURO |
| **Leitor Magnético**   | ANSI/ISO                  | 🔄 USO FUTURO |

## 📚 Documentação Completa

| Componente            | Documentação                                                                         |
|-----------------------|--------------------------------------------------------------------------------------|
| **Instalação/Execução** | [instrucao-inicial.md](instrucao-inicial.md) - Guia passo a passo completo        |
| **API da biblioteca** | [rotinas_biblioteca.txt](rotinas_biblioteca.txt) - Funções, parâmetros e retorno      |
| **Exemplos**          | [exemplos/README.md](exemplos/README.md) - Guia completo (C, C++ e Java)             |
| **Fingerprint (C)**   | [exemplos/fingerprint/c/README.md](exemplos/fingerprint/c/README.md)                 |
| **Display LCD (C)**   | [exemplos/displaylcd/c/README.md](exemplos/displaylcd/c/README.md)                   |

## 🛠️ Pré-requisitos

### Sistema Operacional
- **OS**: Linux (Ubuntu 18.04+ / Debian 10+), 64 ou 32 bits
- **Hardware**: TEC44AVT conectado na porta PS/2 roxa

### Dependências

```bash
sudo apt update
sudo apt install -y build-essential pkgconf libevdev-dev openjdk-17-jdk maven

# Verificar instalações
gcc --version
java -version
mvn -version
```

## 🚀 Como Começar

### Iniciante? Siga este caminho:

```bash
# 1. Clonar e instalar a biblioteca (ver seção "Instalação Rápida" acima
#    para o passo completo, com a arquitetura correta)
git clone https://github.com/Avantter2025/K044AVT.git ~/avanttec
cd ~/avanttec/driver64   # ou driver32
sudo cp lib/libK044AVT.so /usr/local/lib/ && sudo cp include/*.h include/*.hpp /usr/local/include/ && sudo ldconfig

# 2. Rodar primeiro exemplo (Fingerprint)
cd ~/avanttec/exemplos/fingerprint/java
mvn -q clean compile package
sudo java -jar target/k044-fingerprint-demo-1.0.0.jar

# 3. Ou experimentar Display LCD
cd ~/avanttec/exemplos/displaylcd/java
mvn -q clean compile package
sudo java -jar target/k044-displaylcd-demo-1.0.0.jar
```

Com a biblioteca instalada em `/usr/local/lib` (passo 1) e o cache do
linker atualizado (`ldconfig`), não é necessário passar
`-Djna.library.path` — o JNA encontra `libK044AVT.so` pelo caminho padrão
do sistema.

### Desenvolvedor? Escolha sua linguagem:

**C** - Exemplos mínimos, sem dependências além da biblioteca
```bash
cd ~/avanttec/exemplos/fingerprint
sudo make -C ./c
sudo ./c/test_fingerprint
```

**C/C++** - Mais performance, mais controle
```bash
cd ~/avanttec/exemplos/fingerprint
sudo make -C ./cpp
sudo ./cpp/finger2_v2
```

**Java** - Mais produtivo, multi-plataforma
```bash
cd ~/avanttec/exemplos/fingerprint/java
mvn -q clean compile package
sudo java -jar target/k044-fingerprint-demo-1.0.0.jar
```

## 📖 Casos de Uso

| Caso                             | Solução                                              | Documentação                                                         |
|----------------------------------|------------------------------------------------------|----------------------------------------------------------------------|
| Compilar programa em C/C++       | Usar libK044AVT.so + headers (`driver64`/`driver32`) | [instrucao-inicial.md](instrucao-inicial.md)                       |
| Reprogramar scancodes do teclado | Usar aplicativo Reprog                               | Ver seção "Reprog" abaixo                                            |
| Aprender sobre Fingerprint       | Rodar exemplos                                       | [exemplos/fingerprint/c/README.md](exemplos/fingerprint/c/README.md) |
| Testar Display LCD               | Rodar exemplos                                       | [exemplos/displaylcd/c/README.md](exemplos/displaylcd/c/README.md)   |
| Integrar em seu projeto          | Copiar exemplos como base                            | Ver pasta `exemplos/`                                                |

### Reprog — reprogramação de scancodes

```bash
cd ~/avanttec/reprog
sudo chrt -f 50 java -jar java/k044-reprog-demo-1.0.0.jar
```

`chrt -f 50` eleva a prioridade do processo (SCHED_FIFO) durante a gravação
na EEPROM do teclado — a transação PS/2 é sensível a atrasos de
escalonamento do sistema.

## 🎯 Roadmap de Uso

```
┌─────────────┐
│  Instalar   │  cd driver64 (ou driver32) && sudo cp ... /usr/local/lib
│  Biblioteca │  && sudo cp ... /usr/local/include && sudo ldconfig
└──────┬──────┘
       │
       ├─→ [Só compilar programas?]
       │   └─→ Use a biblioteca no seu código (ver instrucao-inicial.md)
       │
       ├─→ [Reprogramar teclado?]
       │   └─→ cd reprog && sudo chrt -f 50 java -jar java/k044-reprog-demo-1.0.0.jar
       │
       └─→ [Aprender API?]
           └─→ cd exemplos/ → Escolher módulo → make/mvn package
```

## ⚠️ Notas Importantes

- ⚡ **Requer root** - Acesso direto ao hardware PS/2
- 🔒 **Exclusividade** - Apenas UMA instância por vez
- 📌 **Hardware específico** - Requer TEC44AVT conectado
- 💡 **Porta correta** - Sempre usar porta PS/2 **roxa** (não verde/mouse)
- 🚫 **Nunca execute sem o teclado conectado** - ver aviso no topo deste README

## 🐛 Troubleshooting Rápido

### "libK044AVT.so not found"
```bash
cd ~/avanttec/driver64   # ou driver32, conforme sua arquitetura
sudo cp lib/libK044AVT.so /usr/local/lib/
sudo ldconfig
```

### "Teclado não responde"
```bash
# Verificar conexão
dmesg | tail

# Reiniciar aplicação
# Desconectar/reconectar teclado
```

### "Port already in use"
- Fechar aplicação anterior (`Ctrl+C`)
- Aguardar 2-3 segundos
- Tentar novamente

## 📊 Estrutura de Pastas Explicada

| Pasta                     | Conteúdo                    | Use quando...                  |
|---------------------------|-----------------------------|--------------------------------|
| `driver64/` / `driver32/` | Biblioteca + headers        | Instalar ou compilar programas |
| `reprog/`                 | App reprogramação (Java)    | Alterar scancodes do teclado   |
| `exemplos/fingerprint/`   | Exemplos de biometria       | Aprender/testar fingerprint    |
| `exemplos/displaylcd/`    | Exemplos de display         | Aprender/testar LCD            |

## 🤝 Contribuições & Suporte

- **Bug Report**: [Issues no GitHub](https://github.com/Avantter2025/K044AVT/issues)
- **Sugestões**: Abra uma issue com o prefixo `[FEATURE]`
- **Código**: Consulte o repositório principal do projeto

## 📄 Licença

Software proprietário. Uso restrito ao hardware TEC44AVT da Avanttec Tecnologia.
Veja o arquivo [LICENSE](LICENSE) para os termos completos.

## 🔗 Links Úteis

- **Repositório Principal**: [Avanttec Project](https://github.com/Avantter2025)
- **Issues e Discussões**: [GitHub Issues](https://github.com/Avantter2025/K044AVT/issues)
- **Hardware**: TEC44AVT - Teclado Programável 44 Teclas com LCD, Biometria e tecl. Auxiliar

---

**Última atualização**: 2026-09-12
**Versão**: 1.0
**Status**: ✅ Pronto para Produção
