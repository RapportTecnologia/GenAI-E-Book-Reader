# How to Compile on Windows

This tutorial provides a step-by-step guide to setting up a development environment and compiling the GenAI E-Book Reader on Windows.

## 1. Prerequisites

You will need to download and install several tools. It is recommended to use the specific versions listed below to ensure compatibility.

- **Visual Studio 2019**: Provides the C++ compiler (MSVC v142 toolset).
- **CMake**: The build system used by the project.
- **Python**: Used by some build scripts.
- **Ninja**: A fast, alternative build tool that works with CMake.
- **Qt 6.9.2**: The UI framework.

## 2. Installation and Setup

Follow these steps carefully to install and configure each prerequisite.

### Step 2.1: Install Visual Studio 2019

1.  Download the **Visual Studio 2019 Community** installer from the [official Microsoft website](https://visualstudio.microsoft.com/vs/older-downloads/). (You may need to create a free Microsoft account).
2.  Run the installer.
3.  In the "Workloads" tab, select **"Desktop development with C++"**.
4.  On the right-hand "Installation details" panel, ensure **"MSVC v142 - VS 2019 C++ x64/x86 build tools"** and the latest **"Windows 10 SDK"** are selected.
5.  Proceed with the installation.

### Step 2.2: Install CMake

1.  Download the latest CMake installer for Windows from the [official website](https://cmake.org/download/).
2.  Run the installer.
3.  During installation, select the option **"Add CMake to the system PATH for all users"** (or for the current user).

### Step 2.3: Install Python

1.  Download a Python installer (version 3.8 or newer) from the [official Python website](https://www.python.org/downloads/).
2.  Run the installer.
3.  **Important**: On the first screen, check the box that says **"Add Python.exe to PATH"**.

### Step 2.4: Install Ninja

1.  Open a new Command Prompt or PowerShell window.
2.  Run the following command to install Ninja using Python's package manager:
    ```sh
    pip install ninja
    ```

### Step 2.5: Install Qt 6.9.2

We will use the official Qt Online Installer, as it is the most reliable method.

1.  Go to the [Qt download page](https://www.qt.io/download-qt-installer) and download the online installer. You will need to create a free Qt Account.
2.  Run the installer and log in with your Qt Account.
3.  On the "Select Components" screen:
    -   Under the "Qt" section, expand **"Qt 6.7.2"**.
    -   Check the box for **"MSVC 2019 64-bit"**.
    -   (Optional but recommended) Check the box for **"Qt 5 Compatibility Module"** if it is listed.
    -   You can uncheck all other components (like MinGW, MSVC 2022, Android, etc.) to save disk space.
4.  Complete the installation.

## 3. Building the Project

All build commands must be run in the special terminal provided by Visual Studio.

### Step 3.1: Open the Correct Terminal

1.  Open the Windows Start Menu.
2.  Type `Developer Command Prompt for VS 2019` and open it. A new command prompt window will appear, pre-configured with the compiler environment.

### Step 3.2: Run Build Commands

In the Developer Command Prompt you just opened, run the following commands in sequence.

1.  **Navigate to the project directory:**
    ```powershell
    cd c:\Users\consu\workspace\rapport\GenAI-E-Book-Reader\GenAI-E-Book-Reader
    ```
    *(Adjust the path if your project is located elsewhere)*

2.  **Configure the project with CMake:**
    *This command generates the build files for Ninja. It only needs to be run once, or after cleaning the build directory.*
    ```powershell
    cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.7.2/msvc2019_64"
    ```

3.  **Compile the project:**
    *This command uses Ninja to compile the source code and create the executable.*
    ```powershell
    cmake --build build -j
    ```

## 4. Alternative: Building with MinGW (g++)

If you prefer to use the MinGW (g++) compiler instead of Visual Studio's MSVC, the process is slightly different. The main challenge is ensuring the terminal can find the MinGW compiler.

### Step 4.1: Install MinGW Components

When you run the official Qt Online Installer (Step 2.5), make sure you select the **"MinGW 64-bit"** component under the desired Qt version (e.g., Qt 6.9.2).

### Step 4.2: Configure Windows Environment (Permanent Fix)

To allow any terminal to find the MinGW compiler, you should add it to your system's PATH.

1.  Press the **Windows Key**, type `env`, and select **"Edit the system environment variables"**.
2.  Click the **"Environment Variables..."** button.
3.  In the "System variables" section, find `Path` and click **"Edit..."**.
4.  Click **"New"** and add the path to your MinGW compiler's `bin` directory. This path depends on how you installed it.
    - If you installed MinGW separately to `C:\mingw`, the path is `C:\mingw\bin`.
    - If it was installed by the Qt installer, the path will be something like `C:\Qt\Tools\mingw1120_64\bin`.
5.  Click **"New"** again and add the path to the Qt binaries for MinGW:
    `C:\Qt\6.9.2\mingw_64\bin`
6.  Click "OK" to save all changes.
7.  **Important**: You must open a **new** terminal window for these changes to apply.

### Step 4.3: Build the Project with MinGW

Open a **new** PowerShell or Command Prompt window and run the following commands.

1.  **Navigate to the project directory:**
    ```powershell
    cd c:\Users\consu\workspace\rapport\GenAI-E-Book-Reader\GenAI-E-Book-Reader
    ```

2.  **Clean the previous build directory (important):**
    ```powershell
    rmdir /s /q build
    ```

3.  **Configure with CMake:**
    ```powershell
    cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.9.2\mingw_64"
    ```

4.  **Compile the project:**
    ```powershell
    cmake --build build -j
    ```

## 5. Run the Application

If the build completes successfully, the executable will be located at:

`build\bin\genai_reader.exe`

You can run it directly from the command line or by double-clicking it in File Explorer.
