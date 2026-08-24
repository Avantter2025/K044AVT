#!/bin/bash

# Script de instalação da libK044AVT
# Copia a biblioteca nativa e headers para os diretórios do sistema

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIB_FILE="$SCRIPT_DIR/lib/libK044AVT.so"
HEADER_FILE="$SCRIPT_DIR/include/display_driver.h"

# Verificar se os arquivos existem
if [ ! -f "$LIB_FILE" ]; then
    echo "❌ Erro: libK044AVT.so não encontrado em $LIB_FILE"
    exit 1
fi

if [ ! -f "$HEADER_FILE" ]; then
    echo "❌ Erro: display_driver.h não encontrado em $HEADER_FILE"
    exit 1
fi

# Verificar permissões de root
if [ "$EUID" -ne 0 ]; then
    echo "❌ Este script precisa de permissões de root (sudo)"
    exit 1
fi

echo "📦 Instalando libK044AVT..."

# Instalar biblioteca
echo "  → Copiando libK044AVT.so para /usr/local/lib/"
cp "$LIB_FILE" /usr/local/lib/
chmod 644 /usr/local/lib/libK044AVT.so
chown root:root /usr/local/lib/libK044AVT.so

# Instalar header
echo "  → Copiando display_driver.h para /usr/local/include/"
cp "$HEADER_FILE" /usr/local/include/
chmod 644 /usr/local/include/display_driver.h
chown root:root /usr/local/include/display_driver.h

# Atualizar cache de biblioteca dinâmica
echo "  → Atualizando cache ldconfig..."
ldconfig

echo ""
echo "✅ Instalação concluída com sucesso!"
echo ""
echo "A biblioteca está disponível em:"
echo "  - /usr/local/lib/libK044AVT.so"
echo "  - /usr/local/include/display_driver.h"
echo ""
echo "Para compilar programas que usam libK044AVT, use:"
echo "  gcc -o programa programa.c -lK044AVT"
