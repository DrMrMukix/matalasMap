# matalasMap — Editor Creativo de Mapas Históricos 🗺️

**matalasMap** es un editor de mapas multiplataforma (Windows y Android) diseñado especialmente como un cuaderno digital de exploración geográfica y creativa. Pensado con una experiencia de usuario clara, divertida y moderna para niños apasionados por la geografía, los mapas y la historia.

> **Regla de Oro:** El juego **NO** es un juego de gran estrategia, ni un simulador de guerra, economía o diplomacia. No hay turnos ni ticks. El usuario inventa países, dibuja fronteras, modifica la tierra y los océanos, y guarda sus creaciones a resolución 8K (8192 × 4096).

---

## 🌟 Características Principales

- **Superficie 8K Real (8192 × 4096)**: Proyección cilíndrica equidistante (Plate Carrée) a nivel de píxel.
- **Modo Terreno**:
  - ✏️ Pincel de tierra.
  - 🧽 Borrador (convierte en agua/océano).
  - 🪣 Bote de pintura (relleno de tierra/agua).
- **Modo Político**:
  - 🏛️ Crear países con nombres y colores personalizados.
  - ✏️ Pintar territorio soberano sobre tierra firme.
  - 🧽 Desasignar territorio.
  - 🎯 Selector de país territorial con un solo clic.
- **Sistema de Deshacer / Rehacer**:
  - Historial por deltas y regiones rectangulares sin duplicar 32 MB de memoria por acción.
- **Navegación Fluida**:
  - Zoom de alta precisión con rueda del mouse o gestos.
  - Desplazamiento (Pan) arrastrando con clic derecho o rueda.
  - Botones de ajuste y centrado instantáneo.
- **Guardado y Carga Versionada**:
  - Formato binario `.matalas` con compresión de capas y miniaturas instantáneas (512×256).

---

## 🛠️ Tecnologías

- **Rust (Game Core)**: Estado del mundo (`WorldState`), capas de terreno y política, undo/redo, serialización y puente C-ABI / FFI.
- **C++ (Qt 6 Bridge)**: Adaptador fino (`WorldEditorBridge`), modelo de lista de países (`CountryListModel`) y lienzo interactivo (`MapCanvasItem`).
- **Qt 6.8 + QML**: Interfaz de usuario táctil, botones grandes, paleta moderna e intuitiva para niños.

---

## 🚀 Inicio Rápido en Windows

### Requisitos
- Rust 1.80+ (`cargo`)
- Visual Studio 2022 o 2026 (herramientas C++)
- CMake 3.20+ y Ninja
- Qt 6.8.2 (MSVC 2022 64-bit)

### Compilación y Ejecución
```bash
# 1. Compilar el núcleo de Rust
cargo build --release -p matalas_core

# 2. Preprocesar el SVG a PNG 8K con colores planos (sólo una vez)
cargo run --release -p svg_preprocessor

# 3. Configurar y compilar con CMake
cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/msvc2022_64"
cmake --build build

# 4. Desplegar dependencias de Qt y ejecutar
C:\Qt\6.8.2\msvc2022_64\bin\windeployqt.exe --qmldir qml build/matalasMap.exe
xcopy /E /I /Y assets build\assets
xcopy /E /I /Y qml build\qml
build\matalasMap.exe
```
