# Gestión de Contenido y Assets — matalasMap

## 1. Separación entre Motor y Contenido

El motor (`matalas_core`) implementa únicamente las reglas cartográficas, estructuras de memoria y algoritmos de edición. Todos los datos geográficos e históricos provienen de archivos externos preprocesados o guardados por el usuario:

```text
assets/
├── maps/
│   ├── earth_terrain_8192x4096.png  <- Mapa plano base 8K generado del SVG
│   └── earth_preview_512x256.png    <- Miniatura rápida para el selector
├── flags/                           <- Banderas en formato SVG / PNG
├── emblems/                         <- Escudos y emblemas
├── presets/                         <- Plantillas predefinidas (.matalas)
└── icons/                           <- Iconos de la interfaz
```

---

## 2. Mapa Base 8K Preprocesado

Siguiendo las especificaciones del proyecto:
- El SVG original (`Equirectangular_projection_world_map_without_borders.svg`) se preprocesa una única vez fuera de la aplicación principal mediante la herramienta `svg_preprocessor`.
- La imagen resultante es un PNG plano de **8192 × 4096 píxeles**:
  - **Océano / Agua**: Color plano `#244F75` (R: 36, G: 79, B: 117)
  - **Tierra**: Color plano `#4E9A51` (R: 78, G: 154, B: 81)
- La aplicación carga directamente este asset optimizado al instante sin overhead de rasterización de curvas vectoriales en tiempo de ejecución.

---

## 3. Formato de Guardados (`.matalas`)

Cada mundo guardado contiene:
1. **Encabezado JSON/Binario**:
   - Versión del formato.
   - Nombre del mundo.
   - Timestamp de creación y última modificación.
   - Dimensiones (8192×4096).
   - Miniatura PNG integrada (512×256) para visualización inmediata en "Mis Mundos" sin decodificar capas pesadas.
2. **Lista de Países**:
   - Identificadores, nombres y colores RGBA.
3. **Capa de Terreno Comprimida (Deflate)**:
   - 33.5 millones de píxeles comprimidos a ~1 MB.
4. **Capa Política Comprimida (Deflate)**:
   - Índices de soberanía territorial por píxel.
