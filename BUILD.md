# Guía de Compilación y Ejecución — matalasMap

## 1. Requisitos Previos

### Windows
- **Rust**: 1.80 o superior (`rustup default stable-x86_64-pc-windows-msvc`).
- **MSVC**: Visual Studio 2022 o 2026 con soporte C++ (C++20).
- **Qt 6**: Versión 6.5+ (verificado con Qt 6.8.2 MSVC 2022 64-bit en `C:\Qt\6.8.2\msvc2022_64`).
- **CMake**: Versión 3.20+.
- **Ninja**: Generador de compilación rápido.

---

## 2. Compilación en Windows

Desde una consola de comandos de desarrollador de Visual Studio (`vcvars64.bat`):

```cmd
:: 1. Compilar el núcleo Rust en modo Release
cargo build --release -p matalas_core

:: 2. Generar el mapa base 8K plano a partir del SVG
cargo run --release -p svg_preprocessor

:: 3. Configurar CMake
cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/msvc2022_64" -DCMAKE_MAKE_PROGRAM="C:/w64devkit/bin/ninja.exe"

:: 4. Compilar el ejecutable C++
cmake --build build

:: 5. Desplegar dependencias de Qt y recursos
C:\Qt\6.8.2\msvc2022_64\bin\windeployqt.exe --qmldir qml build/matalasMap.exe
xcopy /E /I /Y assets build\assets
xcopy /E /I /Y qml build\qml

:: 6. Ejecutar la aplicación
build\matalasMap.exe
```

---

## 3. Pruebas Automatizadas

Para ejecutar las pruebas unitarias del núcleo de Rust:

```cmd
cargo test -p matalas_core
```

---

## 4. Compilación para Android (Referencia Multiplataforma)

La arquitectura compartida de Rust permite compilar el core para las arquitecturas de Android:
- `aarch64-linux-android`
- `armv7-linux-androideabi`
- `x86_64-linux-android`

Con `cargo-ndk`:
```bash
cargo ndk -t arm64-v8a build --release -p matalas_core
```
Y configurar el proyecto de Qt para Android apuntando al Android NDK correspondiente.
