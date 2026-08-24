# K044AVT Reprog - Reprogramador de Scancodes

Aplicativo com interface gráfica (Java Swing) para reprogramação dos scancodes do teclado programável **TEC44AVT** (44 teclas).

## 📋 O que é?

O K044AVT Reprog é uma ferramenta visual para:
- **Ler** a tabela de scancodes armazenada na EEPROM do teclado
- **Editar** o mapeamento de teclas (MAKE e BREAK)
- **Programar** novas configurações no scancodes das teclas
- **Comparar** configuração atual vs. configuração guardada

## 🚀 Instalação Rápida

### Pré-requisitos

1. **libK044AVT instalada** (biblioteca nativa)
   ```bash
   sudo /caminho/para/libk044avt/install.sh
   ```

2. **Java instalado**
   ```bash
   sudo apt-get install -y default-jdk
   ```

3. **Teclado TEC44AVT conectado** na porta PS/2 roxa

### Instalação do Reprog

```bash
cd reprog
bash install.sh
```

O script instalará:
- Aplicativo JAR em `app/`
- Atalho no menu de aplicações
- Launcher em `/usr/local/bin/k044-reprog`

## 🎯 Como Usar

### Iniciar o Aplicativo

**Opção 1 - Pelo Terminal:**
```bash
k044-reprog
```

**Opção 2 - Pelo Menu de Aplicações:**
- Procure por "K044AVT Reprog Scancodes"

**Opção 3 - Diretamente:**
```bash
cd reprog/bin
./k044-reprog
```

### Interface Principal

A interface possui três abas principais:

#### 📖 **LER SCAN CODE**
- Lê a tabela atual de scancodes da EEPROM
- Carrega em memória para edição
- Exibe MAKE e BREAK codes para cada tecla

#### ✏️ **EDITAR TABELA**
- Edita o mapeamento de scancodes
- Suporta operações em massa:
  - **SUBSTITUIR**: troca um scancode por outro
  - **LIMPAR TABELA**: limpa todas as entradas
  - **SALVAR CONFIGURAÇÃO**: exporta em arquivo `.cnf`

#### 💾 **PROGRAMAR SCAN CODE**
- Grava a tabela editada na EEPROM do teclado
- Recalcula CRC automaticamente
- Requer confirmação antes de gravar

#### 🔍 **COMPARAR SCAN CODE**
- Compara configuração em memória vs. EEPROM
- Destaca diferenças em vermelho
- Útil para verificar se programação foi bem-sucedida

## ⚡ Recursos

- ✅ Interface gráfica intuitiva (Java Swing)
- ✅ Suporte a MAKE e BREAK codes
- ✅ Validação de CRC automática
- ✅ Exportação/Importação de configurações (`.cnf`)
- ✅ Requer privilégios de root (acesso ao PS/2)
- ✅ Feedback em tempo real

## ⚠️ Requisitos

| Item            | Detalhes                          |
|-----------------|-----------------------------------|
| **OS**          | Linux (Ubuntu/Debian recomendado) |
| **Java**        | JDK 11+                           |
| **libK044AVT**  | Versão compilada e instalada      |
| **Hardware**    | TEC44AVT conectado na porta PS/2  |
| **Privilégios** | Root (via `pkexec`)               |
| **Espaço**      | ~3 MB (JAR)                       | 

## 📚 Exemplos de Scancodes

Alguns scancodes comuns do teclado:

| Tecla | MAKE | BREAK |
|-------|------|-------|
| A     | 1C   | F0 1C |
| ENTER | 5A   | F0 5A |
| ESC   | 76   | F0 76 |
| SPACE | 29   | F0 29 |

> Consulte o manual completo para lista completa

## 🐛 Troubleshooting

### "Nenhum dispositivo conectado"
- Verifique se o TEC44AVT está conectado na porta PS/2 **roxa**
- Tente desconectar e reconectar
- Reinicie o aplicativo

### "Operation not permitted"
- Execute com privilégios de root
- O launcher usa `pkexec` automaticamente

### "libK044AVT.so not found"
- Instale a biblioteca: `sudo /caminho/para/libk044avt/install.sh`
- Verifique: `ls -l /usr/local/lib/libK044AVT.so`

### "Java not found"
```bash
sudo apt-get install -y default-jdk
```

## 📝 Arquivo de Configuração

Você pode salvar/carregar configurações em arquivos `.cnf`:

```bash
# Exportar configuração atual
# Arquivo salvo em: exemplos/reprog/Modelos/44T/

# Importar configuração anterior
# Abra o arquivo via interface do Reprog
```

## 🔗 Dependências

```
k044-reprog-demo-1.0.0.jar
├── JNA (Java Native Access)
├── Swing (AWT/Swing)
└── libK044AVT.so (biblioteca nativa)
```

## 📋 Estrutura da Pasta

```
reprog/
├── README.md                           (Este arquivo)
├── install.sh                          (Script de instalação)
├── app/
│   └── k044-reprog-demo-1.0.0.jar    (Aplicativo compilado)
├── bin/
│   └── k044-reprog                    (Launcher/script)
└── config/
    └── k044-reprog-demo.desktop       (Atalho do menu)
```

## 🔐 Segurança & Permissões

- O aplicativo requer `pkexec` para acessar o hardware
- Será solicitada autenticação ao executar
- Somente a EEPROM do teclado é modificada (não afeta o SO)

## 📖 Mais Informações

Para documentação completa:
- Consulte o manual do TEC44AVT
- Veja exemplos em `exemplos/reprog/` no repositório principal
- Documentação de API: `libk044avt/README.md`

## 📄 Licença

[Ver LICENSE no repositório]

---

**Versão**: 1.0.0  
**Data**: 2026-08-24  
**Plataforma**: Linux (x86_64)
