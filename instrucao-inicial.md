# Exemplos de Uso da Biblioteca libK044AVT.so

**Referência para o desenvolvedor** (Linux 64/32-bits) — Teclado 44 Teclas
**TEC44AVT** LCD+Finger+Tecl.Aux, da Avanttec Tecnologia.

🌐 www.avanttectecnologia.com.br

## 🛠️ Pré-requisitos

- Sistema operacional Linux 64 ou 32 bits, nativo (não em máquina virtual
  com passthrough parcial de PS/2).
- A pasta `avanttec` deve estar localizada na pasta pessoal do usuário:
  `/home/usuario/avanttec` (ou seja, `~/avanttec`) — os comandos abaixo
  assumem esse caminho.

## ⚙️ Instalação do Ambiente e das Ferramentas

### 1. Atualizar a lista de repositórios

```bash
sudo apt update
```

### 2. Instalar as ferramentas de compilação (C++ / Java / Maven)

```bash
sudo apt install -y build-essential pkgconf libevdev-dev openjdk-17-jdk maven
```

### 3. Configurar o `JAVA_HOME`

Abra o arquivo de perfil do seu usuário:

```bash
nano ~/.bashrc
```

Adicione as linhas abaixo ao final do arquivo — use a linha correspondente
à sua arquitetura (confirme com `uname -m` no passo seguinte):

```bash
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64   # 64-bits (x86_64)
#export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-i386   # 32-bits (i686/i386)
export PATH=$JAVA_HOME/bin:$PATH
```

Aplique as alterações:

```bash
source ~/.bashrc
```

### 4. Criar a pasta de recursos para o JNA

```bash
mkdir -p src/main/resources/linux-x86
```

### 5. Atualizar e registrar as bibliotecas instaladas

```bash
sudo ldconfig
```

### 6. Validar as ferramentas

```bash
java -version
gcc --version
mvn -version
```

## ⚠️ Antes de executar os exemplos — leia isto

**NÃO execute nenhum aplicativo se o teclado K044AVT não estiver conectado
na porta PS/2.** Isso deixa as portas PS/2 do computador inoperantes.

Para restabelecer o controlador PS/2 (é necessário um **teclado e mouse
USB** disponíveis para digitar os comandos):

```bash
sudo bash -c 'echo -a "i8042" > /sys/bus/platform/drivers/i8042/unbind'
sudo bash -c 'echo -a "i8042" > /sys/bus/platform/drivers/i8042/bind'
```

## 🖥️ Confirmar o Ambiente Operacional

```bash
uname -m
```

- [ ] `x86_64` → 64-bits
- [ ] `i686` ou `i386` → 32-bits

## 📦 Instalar a Biblioteca

A biblioteca deve ser instalada conforme a arquitetura confirmada acima:

```bash
cd ~/avanttec/driver64    # se 64-bits
# cd ~/avanttec/driver32  # se 32-bits

sudo cp lib/libK044AVT.so          /usr/local/lib/
sudo cp include/display_driver.h   /usr/local/include/
sudo cp include/k044avt.hpp        /usr/local/include/
sudo ldconfig
```

Confirme a instalação e a arquitetura da biblioteca:

```bash
file /usr/local/lib/libK044AVT.so
```

- [ ] `ELF 64-bit`
- [ ] `ELF 32-bit`

## 🟢 Código-fonte do Display LCD

### 1. Compilar e executar fonte em C

> **OBS**: utilize teclado e mouse USB para a rotina de scroll, que ocupa
> larga capacidade do canal PS/2.

```bash
cd ~/avanttec/exemplos/displaylcd
sudo make -C ./c
sudo ./c/diagnostico_lcd
```

### 2. Compilar e executar fonte em C++

```bash
cd ~/avanttec/exemplos/displaylcd
sudo make -C ./cpp
sudo ./cpp/lcd
```

### 3. Compilar e executar fonte em Java

```bash
cd ~/avanttec/exemplos/displaylcd/java
mvn -q clean compile package
sudo java -jar target/k044-displaylcd-demo-1.0.0.jar
```

## 🔴 Código-fonte do Fingerprint

### 1. Compilar e executar fonte em C

```bash
cd ~/avanttec/exemplos/fingerprint
sudo make -C ./c
sudo ./c/test_fingerprint
```

### 2. Compilar e executar fonte em C++

```bash
cd ~/avanttec/exemplos/fingerprint
sudo make -C ./cpp
sudo ./cpp/finger2_v2
```

### 3. Compilar e executar fonte em Java

```bash
cd ~/avanttec/exemplos/fingerprint/java
mvn -q clean compile package
sudo java -jar target/k044-fingerprint-demo-1.0.0.jar
```

## 🔧 Aplicativo de Reprogramação do Scancode das Teclas (Reprog)

```bash
cd ~/avanttec/reprog
sudo chrt -f 50 java -jar java/k044-reprog-demo-1.0.0.jar
```

---

📧 **Suporte**: www.avanttectecnologia.com.br — suporte@avanttectecnologia.com.br
