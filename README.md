# K044AVT - Teclado Programável

Repositório com a biblioteca nativa `libK044AVT.so` compilada e arquivos de instalação para o teclado programável TEC44AVT com funcionalidades avançadas.

## 📦 Conteúdo

Este repositório contém:

- **`libk044avt/`** - Pasta com a biblioteca nativa compilada e headers
  - `lib/libK044AVT.so` - Biblioteca compartilhada
  - `include/display_driver.h` - Headers da biblioteca
  - `install.sh` - Script de instalação automática
  - `README.md` - Documentação de instalação e uso

## 🚀 Instalação Rápida

```bash
# Clonar repositório
git clone https://github.com/Avantter2025/K044AVT.git
cd K044AVT/libk044avt

# Instalar
sudo bash install.sh
```

## ✨ Funcionalidades

A biblioteca libK044AVT oferece acesso a:

- **Teclado de 44 teclas** - Programável via EEPROM
- **Leitor de Impressão Digital** - Módulo AS608 integrado
- **Display LCD** - 2 linhas × 40 caracteres (HD44780)
- **PIN Pad** - Teclado numérico integrado (USO FUTURO)
- **Leitor de Cartão Magnético** - ANSI/ISO suportado (USO FUTURO)
- **EEPROM** - Armazenamento programável
- **Teclado Auxiliar** - Suporte a PS/2 externo
- **LEDs de Status** - Controle de indicadores (USO FUTURO)

## 📚 Documentação

- [Instruções de Instalação](libk044avt/README.md)
- Documentação de API: Ver `display_driver.h`
- Exemplos de código: Ver repositório principal do projeto

## 🛠️ Requisitos de Sistema

- **OS**: Linux (Ubuntu/Debian recomendado)
- **Arquitetura**: x86_64
- **Dependências**: 
  - `libc6`
  - GCC/Clang (para compilar programas que usam a biblioteca)

## 📖 Como Usar

### Com C/C++

```bash
gcc -o programa programa.c -lK044AVT
sudo ./programa
```

### Com Java (JNA)

```bash
java -Djna.library.path=/usr/local/lib -cp lib.jar com.exemplo.MeuApp
```

### Com Python (ctypes)

```python
import ctypes
lib = ctypes.CDLL('/usr/local/lib/libK044AVT.so')
```

## ⚠️ Notas Importantes

- ⚡ **Requer root** - Acesso direto ao hardware PS/2
- 🔒 **Exclusividade** - Apenas uma instância pode acessar o dispositivo
- 📌 **Hardware específico** - Requer TEC44AVT conectado

## 🤝 Contribuições

Este repositório contém apenas a biblioteca compilada e arquivos de instalação. Para contribuições ao código-fonte, consulte o repositório principal do projeto.

## 📄 Licença

[Ver LICENSE no repositório]

## 🔗 Links

- **Repositório Principal**: [Avanttec Project](https://github.com/Avantter2025/...)
- **Issues**: https://github.com/Avantter2025/K044AVT/issues

---

**Última atualização**: 2026-08-24
