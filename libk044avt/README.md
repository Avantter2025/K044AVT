# libK044AVT - Biblioteca Nativa do TEC44AVT

Biblioteca compartilhada (`libK044AVT.so`) para comunicação com o teclado TEC44AVT (teclado programável com 44 teclas, leitor de impressão digital, display LCD e outras funcionalidades).

## 📋 Requisitos

- Linux (sistemas baseados em Debian/Ubuntu)
- Permissões de root (para instalação)
- GCC/Clang (para compilar programas que usam a biblioteca)
- `libc6` (dependência de tempo de execução)

## 🚀 Instalação Rápida

```bash
cd libk044avt
sudo bash install.sh
```

O script instalará:
- `libK044AVT.so` → `/usr/local/lib/`
- `display_driver.h` → `/usr/local/include/`

## 📖 Como Usar

### Compilar um programa C que usa libK044AVT

```bash
gcc -o meu_programa meu_programa.c -lK044AVT
```

Com `Djna.library.path` para Java/JNA:

```bash
java -Djna.library.path=/usr/local/lib -jar meu_aplicativo.jar
```

### Exemplo Básico (C)

```c
#include <stdio.h>
#include "display_driver.h"

int main() {
    int status = k044_open();
    if (status == K044_OK) {
        printf("Teclado conectado!\n");
        
        // Usar a biblioteca...
        
        k044_close();
    }
    return 0;
}
```

## 📚 Documentação

A documentação completa está disponível em:
- Headers: `display_driver.h` (comentários no código)
- Manual de instalação: Consulte a documentação completa do projeto

## ⚙️ Funcionalidades da Biblioteca

A biblioteca expõe as seguintes APIs:

- **Inicialização**: `k044_open()`, `k044_close()`
- **Teclado**: leitura de scancodes, evento loop
- **Fingerprint**: enroll, search, delete, contagem de templates
- **Display LCD**: controle de um display 2×40
- **LEDs**: controle de LED de status

## 🔧 Desinstalação

Para remover a biblioteca:

```bash
sudo rm /usr/local/lib/libK044AVT.so
sudo rm /usr/local/include/display_driver.h
sudo ldconfig
```

## ⚠️ Notas Importantes

- A biblioteca requer acesso direto ao hardware PS/2 (necessita `CAP_SYS_RAWIO`)
- Programas que usam libK044AVT devem ser executados com `sudo`
- Apenas uma instância pode abrir o dispositivo por vez

## 📝 Licença

Software proprietário. Uso restrito ao hardware TEC44FST da Avanttec Tecnologia.
Veja o arquivo [LICENSE](../LICENSE) para os termos completos.

## 🐛 Suporte

Para issues, dúvidas ou sugestões:
- GitHub Issues: https://github.com/Avantter2025/K044AVT/issues
- Documentação: Consulte o manual completo do projeto

---

**Versão**: 1.0  
**Data**: 2026-08-24
