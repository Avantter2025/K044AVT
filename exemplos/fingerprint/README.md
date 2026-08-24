# Exemplos - Módulo Fingerprint (Leitor de Impressão Digital)

Exemplos práticos de uso do módulo de impressão digital do TEC44AVT em C++ e Java.

## 📋 Funcionalidades Demonstradas

- ✅ Enroll (Cadastrar impressão digital)
- ✅ Search/Identify (Buscar/Identificar)
- ✅ Delete (Deletar template)
- ✅ Template Count (Contar templates)
- ✅ Callbacks de progresso em tempo real

## 🔹 Exemplos C++

### 📁 Estrutura

```
cpp/
├── Makefile              Arquivo de compilação
├── finger2_v2.cpp        Versão 2.1 (API de alto nível)
└── Binários compilados   (finger2_v2)
```

### 🚀 Compilação e Execução

```bash
cd cpp

# Compilar todos
make

# Executar versão 2.1 (API de alto nível)
sudo ./finger2_v2

### 📖 Descrição dos Programas

#### `finger2_v2` - API de Alto Nível
- Usa funções de alto nível da biblioteca

### 🛠️ Opções de Compilação

```bash
# Remove objetos e binários
make clean

# Recompila tudo do zero
make clean all
```

## 🔸 Exemplos Java

### 📁 Estrutura

```
java/
├── pom.xml               Configuração Maven
├── README.md             Documentação Java
├── src/
│   ├── main/java/        Código-fonte
│   │   └── com/avanttec/fingerprint/
│   │       ├── FingerDemo.java
│   │       ├── FingerDemo2.java        (Versão 2 - API alto nível)
│   │       ├── FingerprintLib.java     (Binding JNA)
│   │       └── FingerprintScanPanel.java
│   └── test/java/        Testes (se houver)
└── target/               Binários compilados
    └── k044-fingerprint-demo-1.0.0.jar
```

### 🚀 Compilação e Execução

```bash
cd java

# Compilar e empacotar
mvn clean package

# Executar FingerDemo2 (API alto nível)
sudo java -Djna.library.path=../../../libk044avt/lib \
         -cp target/k044-fingerprint-demo-1.0.0.jar \
         com.avanttec.fingerprint.FingerDemo2
```

### 📖 Características

- ✨ Interface gráfica (Swing)
- 🎨 Painel animado de scan
- 🔄 Callbacks de progresso em tempo real
- 💾 Integração com fingerprint sensor
- ⌨️ Teclado auxiliar ativo

### 🛠️ Opções Maven

```bash
# Compilar sem rodar testes
mvn clean package -DskipTests

# Executar testes
mvn test

# Limpar apenas
mvn clean
```

## 📊 Comparação: C++ vs Java

| Aspecto             | C++                  | Java             |
|---------------------|----------------------|------------------|
| **Interface**       | CLI (terminal)       | GUI (Swing)      |
| **Performance**     | Nativa (mais rápido) | JVM (mais lento) |
| **Portabilidade**   | Baixa                | Alta             |
| **Desenvolvimento** | Mais código          | Menos código     |
| **Debugging**       | GDB                  | IDE do Java      |
| **Instalação**      | Requer compilação    | JAR pronto       |

## 🔧 Pré-requisitos Detalhados

### C++

```bash
# Compilador e ferramentas
sudo apt-get install -y build-essential

# Verificar instalação
g++ --version
make --version
```

### Java

```bash
# JDK
sudo apt-get install -y default-jdk

# Maven
sudo apt-get install -y maven

# Verificar instalação
java -version
mvn --version
```

### Biblioteca

```bash
# Verificar se libK044AVT está instalada
ls -l /usr/local/lib/libK044AVT.so
ls -l /usr/local/include/display_driver.h
```

## 🎯 Como Começar

### Iniciante? Comece com:
1. **C++**: `finger` - Menu simples
2. **Java**: `FingerDemo` - GUI visual

### Avançado? Explore:
1. **C++**: `finger2_v2.cpp` - Código bem estruturado
2. **Java**: `FingerDemo2.java` - API de alto nível

## ⚠️ Notas Importantes

- 🔴 **Requer sudo** - Acesso direto ao hardware
- 🔴 **Exclusivo** - Apenas uma instância por vez
- 🔴 **Hardware** - TEC44FST conectado obrigatório
- 💡 **Dica** - Use os exemplos como base para seus próprios programas

## 🐛 Troubleshooting

### Erro: "Cannot find libK044AVT"
```bash
# Defina LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### Erro: "jna.library.path"
```bash
# Use o path correto
java -Djna.library.path=/usr/local/lib ...
```

### Erro: "Port already in use"
- Apenas uma instância por vez
- Feche a anterior antes de abrir nova

## 🤝 Modificando os Exemplos

Para criar seu próprio programa baseado nesses exemplos:

### C++
```bash
cp finger.cpp meu_programa.cpp
# Edite e compile
g++ -o meu_programa meu_programa.cpp -lK044AVT
sudo ./meu_programa
```

### Java
```bash
# Copie FingerprintDemo.java
# Modifique conforme necessário
mvn package
sudo java -Djna.library.path=/usr/local/lib -jar target/seu-app.jar
```

---

**Última atualização**: 2026-08-24  
**Versão**: 1.0  
**Plataforma**: Linux (x86_64)
