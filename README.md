# K044AVT - Teclado Programável TEC44FST

**Repositório completo** com biblioteca nativa compilada, aplicativos e exemplos práticos para o teclado programável **TEC44FST** com funcionalidades avançadas (biometria, display LCD, 44 teclas programáveis).

## 📦 Conteúdo do Repositório

```
K044AVT/
├── libk044avt/              Biblioteca nativa compilada + instalação
│   ├── lib/libK044AVT.so
│   ├── include/display_driver.h
│   ├── install.sh
│   └── README.md
│
├── reprog/                  Aplicativo Java para reprogramação de scancodes
│   ├── app/k044-reprog-demo-1.0.0.jar
│   ├── bin/k044-reprog      (launcher)
│   ├── config/k044-reprog-demo.desktop
│   ├── install.sh
│   └── README.md
│
└── exemplos/                Exemplos práticos em C/C++ e Java
    ├── fingerprint/         Leitor de impressão digital (AS608)
    │   ├── cpp/            (finger, finger2, finger2_v2)
    │   ├── java/           (GUI Swing)
    │   └── README.md
    │
    └── displaylc/          Display LCD (HD44780 2×40)
        ├── cpp/            (vários testes)
        ├── java/           (GUI)
        └── README.md
```

## 🚀 Instalação Rápida

### 1. Instalar Biblioteca

```bash
git clone https://github.com/Avantter2025/K044AVT.git
cd K044AVT/libk044avt
sudo bash install.sh
```

### 2. (Opcional) Instalar Aplicativo Reprog

```bash
cd ../reprog
bash install.sh
```

### 3. Explorar Exemplos

```bash
cd ../exemplos

# Fingerprint
cd fingerprint/cpp && make && sudo ./finger2_v2

# Ou Display LCD
cd ../displaylc/cpp && make && sudo ./lcd
```

## ✨ Funcionalidades Disponíveis

| Módulo | Descrição | Status |
|--------|-----------|--------|
| **Teclado 44 teclas** | Programável via EEPROM | ✅ Funcional |
| **Leitor Fingerprint** | AS608 - Enroll, Search, Delete | ✅ Funcional |
| **Display LCD** | HD44780 2×40 caracteres | ✅ Funcional |
| **EEPROM** | Armazenamento programável | ✅ Funcional |
| **Teclado Auxiliar** | PS/2 externo | ✅ Funcional |
| **LEDs de Status** | Controle de indicadores | ✅ Funcional |
| **PIN Pad** | Teclado numérico | 🔄 USO FUTURO |
| **Leitor Magnético** | ANSI/ISO | 🔄 USO FUTURO |

## 📚 Documentação Completa

| Componente | Documentação |
|-----------|-----------|
| **Biblioteca** | [libk044avt/README.md](libk044avt/README.md) - Instalação e API |
| **Aplicativo Reprog** | [reprog/README.md](reprog/README.md) - Reprogramação de scancodes |
| **Exemplos** | [exemplos/README.md](exemplos/README.md) - Guia completo |
| **Fingerprint** | [exemplos/fingerprint/README.md](exemplos/fingerprint/README.md) - Leitor biométrico |
| **Display LCD** | [exemplos/displaylc/README.md](exemplos/displaylc/README.md) - Controle de display |

## 🛠️ Pré-requisitos

### Sistema Operacional
- **OS**: Linux (Ubuntu 18.04+ / Debian 10+)
- **Arquitetura**: x86_64
- **Hardware**: TEC44FST conectado na porta PS/2 roxa

### Dependências

```bash
# Build essentials
sudo apt-get install -y build-essential

# Java (para exemplos Java)
sudo apt-get install -y default-jdk maven

# Verificar instalações
gcc --version
java -version
mvn --version
```

## 🚀 Como Começar

### Começante? Siga este caminho:

```bash
# 1. Clonar e instalar biblioteca
git clone https://github.com/Avantter2025/K044AVT.git
cd K044AVT
sudo libk044avt/install.sh

# 2. Rodar primeiro exemplo (Fingerprint)
cd exemplos/fingerprint/java
mvn clean package
sudo java -Djna.library.path=../../../libk044avt/lib -jar target/k044-fingerprint-demo-1.0.0.jar

# 3. Ou experimentar Display LCD
cd ../displaylc/java
mvn clean package
sudo java -Djna.library.path=../../../libk044avt/lib -jar target/k044-displaylc-demo-1.0.0.jar
```

### Desenvolvedor? Escolha sua linguagem:

**C/C++** - Mais performance, mais controle
```bash
cd exemplos/fingerprint/cpp
make
sudo ./finger2_v2
```

**Java** - Mais produtivo, multi-plataforma
```bash
cd exemplos/fingerprint/java
mvn clean package
sudo java -Djna.library.path=/usr/local/lib -jar target/k044-fingerprint-demo-1.0.0.jar
```

## 📖 Casos de Uso

| Caso | Solução | Documentação |
|------|---------|-----------|
| Compilar programa em C/C++ | Usar libK044AVT.so + headers | [libk044avt/README.md](libk044avt/README.md) |
| Reprogramar scancodes do teclado | Usar aplicativo Reprog | [reprog/README.md](reprog/README.md) |
| Aprender sobre Fingerprint | Rodar exemplos | [exemplos/fingerprint/README.md](exemplos/fingerprint/README.md) |
| Testar Display LCD | Rodar exemplos | [exemplos/displaylc/README.md](exemplos/displaylc/README.md) |
| Integrar em seu projeto | Copiar exemplos como base | Ver pasta `exemplos/` |

## 🎯 Roadmap de Uso

```
┌─────────────┐
│  Instalar   │  sudo libk044avt/install.sh
│  Biblioteca │
└──────┬──────┘
       │
       ├─→ [Só compilar programas?]
       │   └─→ Use a biblioteca no seu código
       │
       ├─→ [Reprogramar teclado?]
       │   └─→ bash reprog/install.sh
       │
       └─→ [Aprender API?]
           └─→ cd exemplos/ → Escolher módulo → make/mvn package
```

## ⚠️ Notas Importantes

- ⚡ **Requer root** - Acesso direto ao hardware PS/2
- 🔒 **Exclusividade** - Apenas UMA instância por vez
- 📌 **Hardware específico** - Requer TEC44FST conectado
- 💡 **Porta correta** - Sempre usar porta PS/2 **roxa** (não verde/mouse)

## 🐛 Troubleshooting Rápido

### "libK044AVT.so not found"
```bash
sudo libk044avt/install.sh
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
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

| Pasta | Conteúdo | Use quando... |
|-------|----------|----------|
| `libk044avt/` | Biblioteca + instalação | Instalar ou compilar programas |
| `reprog/` | App reprogramação | Alterar scancodes do teclado |
| `exemplos/fingerprint/` | Exemplos de biometria | Aprender/testar fingerprint |
| `exemplos/displaylc/` | Exemplos de display | Aprender/testar LCD |

## 🤝 Contribuições & Suporte

- **Bug Report**: [Issues no GitHub](https://github.com/Avantter2025/K044AVT/issues)
- **Sugestões**: Abra uma issue com o prefixo `[FEATURE]`
- **Código**: Consulte o repositório principal do projeto

## 📄 Licença

Software proprietário. Uso restrito ao hardware TEC44FST da Avanttec Tecnologia.
Veja o arquivo [LICENSE](LICENSE) para os termos completos.

## 🔗 Links Úteis

- **Repositório Principal**: [Avanttec Project](https://github.com/Avantter2025)
- **Issues e Discussões**: [GitHub Issues](https://github.com/Avantter2025/K044AVT/issues)
- **Hardware**: TEC44FST - Teclado Programável com Biometria

---

**Última atualização**: 2026-08-24  
**Versão**: 1.0  
**Status**: ✅ Pronto para Produção
