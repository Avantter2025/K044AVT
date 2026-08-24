# Exemplos de Uso - K044AVT

Pasta contendo exemplos práticos de programas que utilizam a biblioteca libK044AVT em diferentes linguagens. Inclui exemplos para o **leitor de impressão digital** e **display LCD**.

## 📁 Estrutura

```
exemplos/
├── fingerprint/          Exemplos do módulo de impressão digital (Sensor AS608)
│   ├── cpp/              Exemplos em C++ (finger, finger2, finger2_v2)
│   ├── java/             Exemplos em Java (GUI Swing - FingerDemo)
│   └── README.md         Documentação
│
└── displaylc/            Exemplos do módulo Display LCD (HD44780 2×40)
    ├── cpp/              Exemplos em C (testes de display)
    ├── java/             Exemplos em Java (GUI - LcdDemo)
    └── README.md         Documentação
```

## 🚀 Como Usar

### Pré-requisitos

1. **libK044AVT instalada**
   ```bash
   sudo /caminho/para/libk044avt/install.sh
   ```

2. **Compiladores/Interpretadores**
   - **C/C++**: `g++`/`gcc` (GCC)
   - **Java**: JDK 11+
   - **Build Tools**: `make` e `maven`
   ```bash
   sudo apt-get install -y build-essential default-jdk maven
   ```

3. **Teclado TEC44FST conectado** na porta PS/2 roxa

## 📚 Exemplos Disponíveis

### 🔴 Fingerprint (Leitor de Impressão Digital - AS608)

#### C++ - Menu CLI Interativo

```bash
cd exemplos/fingerprint/cpp
make                      # Compila todos
sudo ./finger             # Menu básico
sudo ./finger2            # Com event loop
sudo ./finger2_v2         # API alto nível
```

**Programas disponíveis:**
- `finger` - Menu CLI simples (Enroll, Search, Delete, etc.)
- `finger2` - Com event loop + teclado auxiliar
- `finger2_v2` - API de alto nível simplificada
- `fp_diag` - Ferramenta de diagnóstico

#### Java - Interface Gráfica

```bash
cd exemplos/fingerprint/java
mvn clean package
sudo java -Djna.library.path=../../../libk044avt/lib \
         -jar target/k044-fingerprint-demo-1.0.0.jar
```

**Recursos:**
- Interface Swing com painel animado de scan
- Operações: Enroll, Search, Identify, Delete
- Callbacks de progresso em tempo real

---

### 🟢 Display LCD (HD44780 2×40)

#### C - Vários Testes

```bash
cd exemplos/displaylc/cpp
make                           # Compila todos
sudo ./lcd                     # Aplicação principal
```

**Testes disponíveis:**
- `test_display_basico` - Operações básicas
- `test_display_cursor` - Controle de cursor
- `test_display_scroll` - Scroll horizontal
- `test_display_keyecho_nav` - Integração com teclado
- `test_display_bell` - Efeitos visuais
- E mais...

#### Java - Aplicação Demo

```bash
cd exemplos/displaylc/java
mvn clean package
sudo java -Djna.library.path=../../../libk044avt/lib \
         -jar target/k044-displaylcd-demo-1.0.0.jar
```

**Recursos:**
- Interface gráfica para controle do display
- Visualização em tempo real
- Teste de todas as funções LCD

## 🔧 Compilação Rápida

### C/C++

```bash
# Fingerprint
cd exemplos/fingerprint/cpp && make

# Display LCD
cd exemplos/displaylc/cpp && make

# Limpar tudo
make clean
```

### Java

```bash
# Fingerprint
cd exemplos/fingerprint/java && mvn clean package

# Display LCD
cd exemplos/displaylc/java && mvn clean package

# Compilar sem testes
mvn clean package -DskipTests
```

## 📖 Documentação Detalhada

Consulte o README específico de cada módulo:

**Fingerprint:**
- [Documentação Fingerprint](fingerprint/README.md) - Comparação C++ vs Java, pré-requisitos, troubleshooting

**Display LCD:**
- [Documentação Display LCD](displaylc/README.md) - API, testes disponíveis, exemplos de uso

## 📊 Comparação: Fingerprint vs Display LCD

| Aspecto | Fingerprint | Display LCD |
|---------|-------------|-------------|
| **Módulo** | Leitor biométrico AS608 | Display HD44780 2×40 |
| **Exemplo C/C++** | Menu CLI + Event loop | Vários testes específicos |
| **Exemplo Java** | GUI com painel animado | GUI com controles |
| **Operações** | Enroll, Search, Delete | Escrever, posicionar, scroll |
| **Complexidade** | Média | Baixa |

## ⚠️ Notas Importantes

- ⚡ **Requer root** - Todos os exemplos precisam de `sudo` para acessar o hardware
- 🔒 **Exclusividade** - Apenas uma instância pode acessar o dispositivo por vez
- 📌 **Hardware específico** - Requer TEC44FST conectado na porta PS/2 roxa
- 💡 **Uma instância por vez** - Feche a anterior antes de abrir nova

## 🐛 Troubleshooting

### "libK044AVT.so not found"
```bash
# Verificar instalação
ls -l /usr/local/lib/libK044AVT.so

# Se não encontrado, reinstalar
sudo /caminho/para/libk044avt/install.sh

# Ou definir variável de ambiente
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
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
sudo apt-get install -y maven
mvn --version  # Verificar
```

### Erro ao compilar C/C++
```bash
# Verificar headers
ls -l /usr/local/include/display_driver.h

# Reinstalar biblioteca
sudo /caminho/para/libk044avt/install.sh
sudo ldconfig
```

## 🎯 Escolhendo seu Primeiro Exemplo

**Começar com C/C++:**
1. Fingerprint: `exemplos/fingerprint/cpp/` → `make` → `sudo ./finger`
2. Display LCD: `exemplos/displaylc/cpp/` → `make` → `sudo ./lcd`

**Começar com Java:**
1. Fingerprint: `exemplos/fingerprint/java/` → `mvn package` → `sudo java -jar target/...jar`
2. Display LCD: `exemplos/displaylc/java/` → `mvn package` → `sudo java -jar target/...jar`

## 🤝 Contribuições

Para adicionar novos exemplos:
1. Crie uma pasta com seu exemplo
2. Inclua arquivo README.md com instruções
3. Adicione arquivos de compilação (Makefile, pom.xml, etc.)
4. Faça um commit com a descrição das mudanças

---

**Última atualização**: 2026-08-24
