#!/bin/bash
#
# Script para preparar o ambiente de CROSS-COMPILAÇÃO para Windows usando MinGW no Linux.

set -e

QT_VERSION='6.7.2'
QT_HOST='windows'
QT_ARCH='mingw_64'
# Usar caminho absoluto para o diretório de instalação
QT_INSTALL_DIR="$(pwd)/.cache/qt-mingw"

# Função para verificar se um comando existe
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# 1. Verificar dependências do sistema
echo "--- Verificando dependências (mingw-w64, p7zip-full) ---"
if ! command_exists x86_64-w64-mingw32-g++; then
    echo "ERRO: Toolchain MinGW não encontrado."
    echo "Por favor, instale o pacote 'mingw-w64'. Em sistemas baseados em Debian/Ubuntu, use:"
    echo "sudo apt-get update && sudo apt-get install mingw-w64"
    exit 1
fi
if ! command_exists 7z; then
    echo "ERRO: 7-Zip não encontrado."
    echo "Por favor, instale o pacote 'p7zip-full'. Em sistemas baseados em Debian/Ubuntu, use:"
    echo "sudo apt-get update && sudo apt-get install p7zip-full"
    exit 1
fi
echo "Dependências encontradas."

# 2. Instalar aqtinstall
echo "\n--- Instalando/Atualizando aqtinstall ---"
pip install -U aqtinstall

# 3. Instalar Qt para MinGW
QT_PATH="$QT_INSTALL_DIR/$QT_VERSION/$QT_ARCH"
if [ -d "$QT_PATH" ]; then
    echo "\n--- Qt para MinGW já parece estar instalado em $QT_PATH ---"
else
    echo "\n--- Baixando e instalando Qt $QT_VERSION para MinGW ---"
    # Usar -O com caminho absoluto
    python -m aqt install-qt "$QT_HOST" desktop "$QT_VERSION" "$QT_ARCH" -O "$QT_INSTALL_DIR"
fi

# 4. Gerar arquivo de toolchain do CMake
TOOLCHAIN_FILE="toolchain-mingw.cmake"
QT6_DIR="${QT_PATH}/lib/cmake/Qt6"
echo "\n--- Gerando arquivo de toolchain: $TOOLCHAIN_FILE ---"

cat > "$TOOLCHAIN_FILE" << EOF
# CMake toolchain file for cross-compiling for Windows with MinGW
set(CMAKE_SYSTEM_NAME Windows)

# Compilers
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
EOF

echo "Arquivo de toolchain gerado com sucesso."

# 5. Conclusão
echo "\n--- Ambiente de cross-compilação pronto! ---"
echo "Para compilar o projeto, execute os seguintes comandos:"

cat << EOF

# 1. Remova o cache antigo do CMake para forçar a reconfiguração
rm -rf build-windows

# 2. Configure o CMake (execute apenas uma vez)
cmake -S . -B build-windows -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DQt6_DIR=${QT6_DIR} \
    -DCMAKE_BUILD_TYPE=Release

# 3. Compile o executável
cmake --build build-windows

# 4. Crie o pacote de distribuição para Windows
cmake --build build-windows --target dist-windows

EOF
