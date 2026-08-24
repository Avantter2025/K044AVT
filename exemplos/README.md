# Exemplos de Uso - K044AVT

Pasta contendo exemplos de programas que utilizam a biblioteca libK044AVT em diferentes linguagens.

## 📁 Estrutura

```
exemplos/
└── fingerprint/          Exemplos do módulo de impressão digital
    ├── cpp/              Exemplos em C++
    ├── java/             Exemplos em Java (GUI Swing)
    └── README.md         Documentação
```

## 🚀 Como Usar

### Pré-requisitos

1. **libK044AVT instalada**
   ```bash
   sudo /caminho/para/libk044avt/install.sh
   ```

2. **Compiladores/Interpretadores**
   - **C++**: `g++` (GCC)
   - **Java**: JDK 11+
   ```bash
   sudo apt-get install -y build-essential default-jdk
   ```

3. **Teclado TEC44FST conectado** na porta PS/2

## 📚 Exemplos Disponíveis

### Fingerprint (Leitor de Impressão Digital)

- **C++**: Menu interativo de linha de comando
  ```bash
  cd exemplos/fingerprint/cpp
  make
  sudo ./finger
  ```

- **Java**: Interface gráfica (Swing)
  ```bash
  cd exemplos/fingerprint/java
  mvn package
  sudo java -Djna.library.path=../../../libk044avt/lib -jar target/k044-fingerprint-demo-1.0.0.jar
  ```

## 🔧 Compilação

### C++

```bash
cd exemplos/fingerprint/cpp
make              # Compila todos os exemplos
make clean        # Remove arquivos compilados
```

### Java

```bash
cd exemplos/fingerprint/java
mvn clean         # Limpa arquivos anteriores
mvn package       # Compila e empacota JAR
mvn test          # Roda testes (se houver)
```

## 📖 Documentação

Consulte o README específico de cada exemplo:
- [Exemplos C++](fingerprint/cpp/README.md) (se disponível)
- [Exemplos Java](fingerprint/java/README.md)

## ⚠️ Notas Importantes

- ⚡ **Requer root** - Todos os exemplos precisam de `sudo` para acessar o hardware
- 🔒 **Exclusividade** - Apenas uma instância pode acessar o dispositivo por vez
- 📌 **Hardware específico** - Requer TEC44FST conectado

## 🐛 Troubleshooting

### "libK044AVT.so not found"
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### "Teclado não conectado"
- Verifique conexão na porta PS/2 roxa
- Teste com: `sudo lsusb` (se USB-PS/2) ou `dmesg`

### "Maven not found"
```bash
sudo apt-get install -y maven
```

## 🤝 Contribuições

Para adicionar novos exemplos:
1. Crie uma pasta com seu exemplo
2. Inclua arquivo README.md com instruções
3. Adicione arquivos de compilação (Makefile, pom.xml, etc.)
4. Faça um commit com a descrição das mudanças

---

**Última atualização**: 2026-08-24
