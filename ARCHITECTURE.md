# Arquitectura del Sistema — matalasMap

## 1. Principio Fundamental de Separación

El proyecto se divide de forma estricta en tres capas funcionales:

```text
┌──────────────────────────────────────────────┐
│           QML (Qt Quick Controls)            │
│  UI, Pantallas, Paneles, Botones, Animaciones│
└──────────────────────┬───────────────────────┘
                       │ Q_PROPERTY / Q_INVOKABLE
┌──────────────────────┴───────────────────────┐
│              C++ / Qt 6 Bridge               │
│ ViewModels, Canvas QQuickPaintedItem, Buffers│
└──────────────────────┬───────────────────────┘
                       │ C-ABI / FFI (tipos primitivos & buffers)
┌──────────────────────┴───────────────────────┐
│                  RUST CORE                   │
│ WorldState, TerrainGrid, PoliticalGrid, Save │
└──────────────────────────────────────────────┘
```

---

## 2. Componentes de Rust Core (`rust/matalas_core`)

- **`core::types`**:
  - `WORLD_WIDTH` = 8192, `WORLD_HEIGHT` = 4096 (Total: 33,554,432 píxeles).
  - `Rect`: Rectángulo delimitador para dirty rects y undo/redo.
  - `ColorRgba`: 4 bytes empaquetados.
  - `ToolType`: Brush (0), Eraser (1), Fill (2), Picker (3).
  - `EditorMode`: Terrain (0), Political (1).
- **`terrain::grid`**:
  - `TerrainGrid`: Capa de bytes (`Vec<u8>`). 0 = Agua, 1 = Tierra.
  - Pintura circular optimizada y Flood Fill acotado.
- **`political`**:
  - `Country`: ID único (`u16`), nombre, color, bandera y conteo de píxeles.
  - `PoliticalGrid`: Capa de soberanía (`Vec<u16>`). 0 = Sin reclamar / Internacional, >0 = CountryID.
- **`undo`**:
  - Historial delta. Almacena únicamente el sub-rectángulo modificado por la pincelada previa.
- **`save`**:
  - Formato versionado comprimido con Deflate.
  - Generación de miniaturas (512×256) incorporadas en la cabecera para lectura instantánea sin decodificar el mapa completo.
- **`ffi`**:
  - Funciones `extern "C"` libres de estructuras opacas complejas, con transferencia por punteros de memoria y arrays.

---

## 3. Componentes C++ / Qt (`cpp/`)

- **`matalas_ffi.h`**: Cabecera C pura con las firmas exportadas por Rust.
- **`WorldEditorBridge`**: Objeto principal expuesto como `worldEditor` en el contexto raíz de QML.
- **`CountryListModel`**: Modelo de datos (`QAbstractListModel`) que alimenta las listas de países en QML.
- **`MapCanvasItem`**: Item de Qt Quick que dibuja la imagen 8K en la GPU usando `QPainter` y actualiza únicamente las porciones rectangulares sucias recibidas desde Rust.

---

## 4. UI QML (`qml/`)

Diseñada bajo los principios de accesibilidad infantil:
- Botones de 72x72px con iconos grandes y feedback háptico/visual.
- Paleta de colores vivos y modernos.
- Sin menús ocultos ni configuración técnica abrumadora.
- Flujo directo: entrar, pintar, inventar país y guardar.
