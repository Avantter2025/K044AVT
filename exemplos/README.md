# Exemplos de Uso - K044AVT

Pasta contendo exemplos práticos de programas que utilizam a biblioteca libK044AVT em diferentes linguagens. Inclui exemplos para o **leitor de impressão digital** e **display LCD**.

## 📁 Estrutura

```
exemplos/
├── fingerprint/          Exemplos do módulo de impressão digital (Sensor AS608)
│   ├── c/                Exemplos em C (test_fingerprint, test_fingerprint_led) + README.md
│   ├── cpp/              Exemplo em C++ (finger2_v2)
│   └── java/             Exemplos em Java (GUI Swing - FingerDemo)
│
└── displaylcd/           Exemplos do módulo Display LCD (2×40)
    ├── c/                Exemplos em C (diagnostico_lcd e outros testes) + README.md
    ├── cpp/              Exemplo em C++ (lcd)
    └── java/             Exemplos em Java (GUI - LcdDemo)
```

## 🚀 Como Usar

### Pré-requisitos

1. **libK044AVT instalada** — ver [../README.md](../README.md) ou
   [../instrucao-inicial.md](../instrucao-inicial.md) para o passo a passo
   completo (`driver64/`/`driver32/` → `/usr/local/{lib,include}` →
   `ldconfig`).

2. **Compiladores/Interpretadores**
   - **C/C++**: `gcc`/`g++` (GCC)
   - **Java**: JDK 17
   - **Build Tools**: `make` e `maven`
   ```bash
   sudo apt install -y build-essential pkgconf libevdev-dev openjdk-17-jdk maven
   ```

3. **Teclado TEC44AVT conectado** na porta PS/2 roxa

## 📚 Exemplos Disponíveis

### 🔴 Fingerprint (Leitor de Impressão Digital)

#### C - Exemplos mínimos

```bash
cd exemplos/fingerprint
sudo make -C ./c
sudo ./c/test_fingerprint
```
**Programas disponíveis:**
- `test_fingerprint` - menu completo (info, cadastro, busca, LED, diagnóstico)
- `test_fingerprint_led` - teste isolado do LED do sensor (cor + estado)

Detalhes em [fingerprint/c/README.md](fingerprint/c/README.md).

#### C++ - Menu CLI Interativo

```bash
cd exemplos/fingerprint/cpp
make                      # Compila
sudo ./finger2_v2         # API alto nível
```
**Programas disponíveis:**
- `finger2_v2` - API de alto nível simplificada

#### Java - Interface Gráfica

```bash
cd exemplos/fingerprint/java
mvn -q clean package
sudo java -jar target/k044-fingerprint-demo-1.0.0.jar
```

**Recursos:**
- Interface Swing com painel animado de scan
- Operações: Enroll, Search, Identify, Delete
- Callbacks de progresso em tempo real

---

### 🟢 Display LCD (2×40)

#### C - Vários Testes

```bash
cd exemplos/displaylcd
sudo make -C ./c
sudo ./c/diagnostico_lcd       # Diagnóstico do display
```

Lista completa dos testes disponíveis (mais de dez programas — teclado,
cursor, scroll, keyecho, CGRAM, etc.) em
[displaylcd/c/README.md](displaylcd/c/README.md).

OBS: use teclado e mouse USB durante o teste de scroll — ele ocupa banda
grande do canal PS/2 e pode disputar com o teclado K044AVT.

#### C++ - Aplicação principal

```bash
cd exemplos/displaylcd/cpp
make                           # Compila
sudo ./lcd                     # Aplicação principal
```

#### Java - Aplicação Demo

```bash
cd exemplos/displaylcd/java
mvn -q clean package
sudo java -jar target/k044-displaylcd-demo-1.0.0.jar
```

**Recursos:**
- Interface gráfica para controle do display
- Visualização em tempo real
- Teste de todas as funções LCD

## 🔧 Compilação Rápida

### C

```bash
# Fingerprint
cd exemplos/fingerprint && sudo make -C ./c

# Display LCD
cd exemplos/displaylcd && sudo make -C ./c
```

### C++

```bash
# Fingerprint
cd exemplos/fingerprint/cpp && make

# Display LCD
cd exemplos/displaylcd/cpp && make

# Limpar (rodar dentro de cada pasta)
make clean
```

### Java

```bash
# Fingerprint
cd exemplos/fingerprint/java && mvn -q clean package

# Display LCD
cd exemplos/displaylcd/java && mvn -q clean package

# Compilar sem testes
mvn clean package -DskipTests
```

## 📖 Documentação Detalhada

Consulte o README específico de cada variante:

**Fingerprint:**
- [Documentação Fingerprint (C)](fingerprint/c/README.md) - arquivos, compilação, execução

**Display LCD:**
- [Documentação Display LCD (C)](displaylcd/c/README.md) - lista completa de testes, compilação, execução

## 📊 Comparação: Fingerprint vs Display LCD

| Aspecto           | Fingerprint             | Display LCD                  |
|-------------------|-------------------------|------------------------------|
| **Módulo**        | Leitor biométrico AS608 | Display HD44780 2×40         |
| **Exemplo C/C++** | Menu CLI + Event loop   | Vários testes específicos    |
| **Exemplo Java**  | GUI com painel animado  | GUI com controles            |
| **Operações**     | Enroll, Search, Delete  | Escrever, posicionar, scroll |
| **Complexidade**  | Média                   | Baixa                        |

## ⚠️ Notas Importantes

- ⚡ **Requer root** - Todos os exemplos precisam de `sudo` para acessar o hardware
- 🔒 **Exclusividade** - Apenas uma instância pode acessar o dispositivo por vez
- 📌 **Hardware específico** - Requer TEC44AVT conectado na porta PS/2 roxa
- 💡 **Uma instância por vez** - Feche a anterior antes de abrir nova

## 🐛 Troubleshooting

### "libK044AVT.so not found"
```bash
# Verificar instalação
ls -l /usr/local/lib/libK044AVT.so

# Se não encontrado, reinstalar (ajuste driver64/driver32 conforme sua arquitetura)
cd ~/avanttec/driver64
sudo cp lib/libK044AVT.so /usr/local/lib/
sudo ldconfig
```

### "Teclado não conectado"
- Verifique conexão na porta PS/2 **roxa** (não verde)
- Teste com: `dmesg | tail` para ver mensagens de hardware
- Tente desconectar e reconectar

### "Port already in use" ou "Device busy"
- Apenas uma instância pode acessar o dispositivo
- Feche completamente a instância anterior com `Ctrl+C`
- Aguarde alguns segundos antes de abrir nova

### "Maven not found"
```bash
sudo apt install -y maven
mvn --version  # Verificar
```

### Erro ao compilar C/C++
```bash
# Verificar headers
ls -l /usr/local/include/display_driver.h

# Reinstalar biblioteca (ajuste driver64/driver32 conforme sua arquitetura)
cd ~/avanttec/driver64
sudo cp lib/libK044AVT.so /usr/local/lib/
sudo cp include/*.h include/*.hpp /usr/local/include/
sudo ldconfig
```

## 🎯 Escolhendo seu Primeiro Exemplo

**Começar com C:**
1. Fingerprint: `exemplos/fingerprint/` → `sudo make -C ./c` → `sudo ./c/test_fingerprint`
2. Display LCD: `exemplos/displaylcd/` → `sudo make -C ./c` → `sudo ./c/diagnostico_lcd`

**Começar com C++:**
1. Fingerprint: `exemplos/fingerprint/cpp/` → `make` → `sudo ./finger2_v2`
2. Display LCD: `exemplos/displaylcd/cpp/` → `make` → `sudo ./lcd`

**Começar com Java:**
1. Fingerprint: `exemplos/fingerprint/java/` → `mvn package` → `sudo java -jar target/...jar`
2. Display LCD: `exemplos/displaylcd/java/` → `mvn package` → `sudo java -jar target/...jar`

## 🤝 Contribuições

Para adicionar novos exemplos:
1. Crie uma pasta com seu exemplo
2. Inclua arquivo README.md com instruções
3. Adicione arquivos de compilação (Makefile, pom.xml, etc.)
4. Faça um commit com a descrição das mudanças

---

**Última atualização**: 2026-09-12
