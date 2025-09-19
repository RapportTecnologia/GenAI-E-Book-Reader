#!/bin/bash
set -e

# --- Configuração ---
QT_VERSION="6.9.2"
QT_ARCH="mingw_64"
QT_DIR_NAME="${QT_VERSION}/${QT_ARCH}"

# Diretório onde o Qt para Windows será instalado
QT_INSTALL_DIR="${PWD}/qt-win"

# Diretório de build
BUILD_DIR="build-windows"

# Diretório do ambiente virtual Python
VENV_DIR=".venv-windows-build"

# Nome do arquivo de toolchain do CMake
TOOLCHAIN_FILE="mingw-toolchain.cmake"

# --- Passo 1: Instalar dependências ---
echo "[+] Verificando dependências..."

# Verifica se o mingw-w64 está instalado (procurando pelo compilador C++)
# Verifica se o Wine está instalado
if ! command -v wine &> /dev/null; then
    echo "Wine não encontrado. Tentando instalar..."
    if [ -x "$(command -v apt-get)" ]; then
        sudo apt-get update && sudo apt-get install -y wine binfmt-support
    elif [ -x "$(command -v dnf)" ]; then
        sudo dnf install -y wine
    elif [ -x "$(command -v pacman)" ]; then
        sudo pacman -S --noconfirm wine
    else
        echo "Não foi possível determinar o gerenciador de pacotes. Por favor, instale o 'wine' manualmente."
        exit 1
    fi
fi

# Verifica se o mingw-w64 está instalado (procurando pelo compilador C++)
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "Compilador MinGW-w64 não encontrado. Tentando instalar..."
    # Tenta usar o gerenciador de pacotes apropriado
    if [ -x "$(command -v apt-get)" ]; then
        sudo apt-get update && sudo apt-get install -y mingw-w64
    elif [ -x "$(command -v dnf)" ]; then
        sudo dnf install -y mingw64-gcc-c++
    elif [ -x "$(command -v pacman)" ]; then
        sudo pacman -S --noconfirm mingw-w64-gcc
    else
        echo "Não foi possível determinar o gerenciador de pacotes. Por favor, instale o 'mingw-w64' manualmente."
        exit 1
    fi
fi

# --- Passo 2: Configurar ambiente virtual Python e instalar aqtinstall ---
if [ ! -d "${VENV_DIR}" ]; then
    echo "[+] Criando ambiente virtual Python em ${VENV_DIR}..."
    python3 -m venv ${VENV_DIR}
fi

echo "[+] Instalando/atualizando aqtinstall no ambiente virtual..."
${VENV_DIR}/bin/python -m pip install -U aqtinstall

echo "[+] Dependências satisfeitas."

# --- Passo 3: Instalar o Qt para Windows ---
if [ ! -d "${QT_INSTALL_DIR}/${QT_DIR_NAME}" ]; then
    echo "[+] Baixando Qt ${QT_VERSION} para ${QT_ARCH}..."
        # Usamos 'win64_mingw' como argumento para aqt, mas o diretório criado é 'mingw_64'
    ${VENV_DIR}/bin/python -m aqt install-qt windows desktop ${QT_VERSION} win64_mingw -O ${QT_INSTALL_DIR} --modules qtpdf qt5compat
else
    echo "[+] Qt para Windows já está instalado em ${QT_INSTALL_DIR}"
fi

echo "[+] Garantindo permissões de execução para as ferramentas do Qt..."
chmod +x ${QT_INSTALL_DIR}/${QT_DIR_NAME}/bin/* || true

# --- Passo 4: Criar o arquivo de toolchain do CMake ---
echo "[+] Gerando o arquivo de toolchain do CMake: ${TOOLCHAIN_FILE}"

cat > ${TOOLCHAIN_FILE} << EOL
# CMake toolchain file for cross-compiling for Windows from Linux with MinGW-w64
set(CMAKE_SYSTEM_NAME Windows)

# Compiladores
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Caminho para encontrar os cabeçalhos e bibliotecas do MinGW
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Ajusta o modo de busca do find_package para procurar apenas no CMAKE_FIND_ROOT_PATH
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

EOL

echo "[+] Arquivo de toolchain criado."

# --- Passo 5: Configurar e Compilar com CMake ---
echo "[+] Configurando o projeto com CMake..."

# O CMAKE_PREFIX_PATH aponta para o diretório de configuração do Qt
cmake -S . -B ${BUILD_DIR} \
      -G "Ninja" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
      -DCMAKE_PREFIX_PATH="${QT_INSTALL_DIR}/${QT_DIR_NAME}"

echo "[+] Compilando o projeto..."
cmake --build ${BUILD_DIR} -j$(nproc)

echo "[+] Compilação concluída!"

# --- Passo 6: Empacotar para distribuição ---
echo "[+] Empacotando as dependências do Windows (windeployqt)..."

DIST_DIR="dist-windows"
mkdir -p ${DIST_DIR}

# Copia o executável
cp "${BUILD_DIR}/bin/genai_reader.exe" "${DIST_DIR}/"

# Executa o windeployqt para copiar todas as DLLs necessárias
# Precisamos adicionar o bin do Qt ao PATH para que o windeployqt encontre tudo
export PATH="${QT_INSTALL_DIR}/${QT_DIR_NAME}/bin:$PATH"
windeployqt --release --compiler-runtime "${DIST_DIR}/genai_reader.exe"

echo "[+] Sucesso!"
echo "O executável para Windows e todas as suas dependências estão em: ${PWD}/${DIST_DIR}"
echo "Você pode copiar a pasta '${DIST_DIR}' inteira para uma máquina Windows e executar 'genai_reader.exe'"
