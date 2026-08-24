#!/bin/bash

# Script de instalação da aplicação K044AVT Reprog
# Instala o aplicativo Java Swing para reprogramação de scancodes

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="$SCRIPT_DIR/app"
CONFIG_DIR="$SCRIPT_DIR/config"
BIN_DIR="$SCRIPT_DIR/bin"

JAR_FILE="$APP_DIR/k044-reprog-demo-1.0.0.jar"
DESKTOP_FILE="$CONFIG_DIR/k044-reprog-demo.desktop"
LAUNCHER_SCRIPT="$BIN_DIR/k044-reprog"

# Verificar se os arquivos existem
if [ ! -f "$JAR_FILE" ]; then
    echo "❌ Erro: JAR não encontrado em $JAR_FILE"
    exit 1
fi

if [ ! -f "$DESKTOP_FILE" ]; then
    echo "❌ Erro: Desktop file não encontrado em $DESKTOP_FILE"
    exit 1
fi

echo "📦 Instalando K044AVT Reprog (Reprogramador de Scancodes)..."

# Criar script launcher
echo "  → Criando script de inicialização..."
mkdir -p "$BIN_DIR"
cat > "$LAUNCHER_SCRIPT" << 'EOF'
#!/bin/bash
# Launcher para K044AVT Reprog

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="$(dirname "$SCRIPT_DIR")/app"
JAR_FILE="$APP_DIR/k044-reprog-demo-1.0.0.jar"

# Verificar se libK044AVT está instalada
if [ ! -f "/usr/local/lib/libK044AVT.so" ]; then
    zenity --error --text="Erro: libK044AVT.so não está instalada.\n\nInstale com:\n  sudo apt-get install k044avt\n\nOu execute:\n  sudo /path/to/libk044avt/install.sh" 2>/dev/null || \
    echo "Erro: libK044AVT.so não encontrada em /usr/local/lib/"
    exit 1
fi

# Executar a aplicação
pkexec env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY java \
    -Djna.library.path=/usr/local/lib \
    -jar "$JAR_FILE"
EOF

chmod +x "$LAUNCHER_SCRIPT"

# Instalar em /usr/local/bin
echo "  → Instalando execução em /usr/local/bin..."
sudo cp "$LAUNCHER_SCRIPT" /usr/local/bin/k044-reprog
sudo chmod +x /usr/local/bin/k044-reprog

# Instalar atalho no menu de aplicações
echo "  → Instalando atalho no menu de aplicações..."
mkdir -p ~/.local/share/applications
cp "$DESKTOP_FILE" ~/.local/share/applications/
update-desktop-database ~/.local/share/applications/ 2>/dev/null || true

echo ""
echo "✅ Instalação concluída com sucesso!"
echo ""
echo "Você pode executar de três formas:"
echo ""
echo "1️⃣  Pelo terminal:"
echo "   k044-reprog"
echo ""
echo "2️⃣  Pelo menu de aplicações:"
echo "   Procure por 'K044AVT Reprog' no menu"
echo ""
echo "3️⃣  Pelo caminho completo:"
echo "   $LAUNCHER_SCRIPT"
echo ""
echo "⚠️  Certifique-se de que:"
echo "   • libK044AVT.so está instalada: /usr/local/lib/"
echo "   • O teclado TEC44FST está conectado"
echo "   • Java (JDK) está instalado"
echo ""
