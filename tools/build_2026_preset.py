import json
import os
import gzip
import time
from PIL import Image, ImageDraw

W = 8192
H = 4096

# Palette of distinct, harmonious sovereign country colors
PALETTE = [
    (198, 40, 40, 255),    # Red
    (21, 101, 192, 255),   # Blue
    (46, 125, 50, 255),    # Green
    (245, 124, 0, 255),    # Orange
    (106, 27, 154, 255),   # Purple
    (0, 131, 143, 255),    # Cyan/Teal
    (194, 24, 91, 255),    # Deep Pink
    (251, 192, 45, 255),   # Yellow/Amber
    (78, 52, 46, 255),     # Brown
    (55, 71, 79, 255),     # Slate
    (92, 107, 115, 255),   # Gray-Blue
    (0, 150, 136, 255),    # Teal
    (139, 195, 74, 255),   # Light Green
    (255, 112, 67, 255),   # Deep Orange
    (126, 87, 194, 255),   # Deep Purple
    (66, 165, 245, 255),   # Sky Blue
]

# Spanish names and flag mappings for major sovereign countries
NAME_MAP = {
    "Spain": ("España", "assets/flags/esp_1978.svg"),
    "France": ("Francia", "assets/flags/fra_1958.svg"),
    "Germany": ("Alemania", "assets/flags/deu_1990.svg"),
    "United Kingdom": ("Reino Unido", "assets/flags/gbr_1927.svg"),
    "Italy": ("Italia", "assets/flags/ita_1946.svg"),
    "United States": ("Estados Unidos", "assets/flags/usa_1960.svg"),
    "United States of America": ("Estados Unidos", "assets/flags/usa_1960.svg"),
    "Mexico": ("México", "assets/flags/mex_1824.svg"),
    "Brazil": ("Brasil", "assets/flags/bra_1889.svg"),
    "Argentina": ("Argentina", "assets/flags/arg_1816.svg"),
    "Chile": ("Chile", "assets/flags/chl_1817.svg"),
    "Japan": ("Japón", "assets/flags/jpn_1947.svg"),
    "China": ("China", "assets/flags/chn_1949.svg"),
    "Russia": ("Rusia", "assets/flags/rusia_1991.svg"),
    "Russian Federation": ("Rusia", "assets/flags/rusia_1991.svg"),
    "Canada": ("Canadá", "assets/flags/canada_1867.svg"),
    "Australia": ("Australia", "assets/flags/australia_1901.svg"),
    "India": ("India", "assets/flags/india_1947.svg"),
    "Portugal": ("Portugal", "assets/flags/portugal_1911.svg"),
    "Netherlands": ("Países Bajos", "assets/flags/paises_bajos_1815.svg"),
    "Belgium": ("Bélgica", "assets/flags/belgica_1830.svg"),
    "Switzerland": ("Suiza", "assets/flags/suiza_1848.svg"),
    "Sweden": ("Suecia", "assets/flags/suecia_1906.svg"),
    "Norway": ("Noruega", "assets/flags/noruega_1899.svg"),
    "Denmark": ("Dinamarca", "assets/flags/dinamarca_1854.svg"),
    "Finland": ("Finlandia", "assets/flags/finlandia_1918.svg"),
    "Poland": ("Polonia", "assets/flags/polonia_1919.svg"),
    "Greece": ("Grecia", "assets/flags/grecia_1978.svg"),
    "Turkey": ("Turquía", "assets/flags/turquia_1936.svg"),
    "Egypt": ("Egipto", "assets/flags/egipto_1984.svg"),
    "South Africa": ("Sudáfrica", "assets/flags/sudafrica_1994.svg"),
    "Morocco": ("Marruecos", "assets/flags/marruecos_1915.svg"),
    "Colombia": ("Colombia", "assets/flags/colombia_1861.svg"),
    "Peru": ("Perú", "assets/flags/peru_1825.svg"),
    "Venezuela": ("Venezuela", "assets/flags/venezuela_2006.svg"),
    "Ecuador": ("Ecuador", "assets/flags/ecuador_1860.svg"),
    "Bolivia": ("Bolivia", "assets/flags/bolivia_1851.svg"),
    "Uruguay": ("Uruguay", "assets/flags/uruguay_1830.svg"),
    "Paraguay": ("Paraguay", "assets/flags/paraguay_1842.svg"),
    "Cuba": ("Cuba", "assets/flags/cuba_1902.svg"),
    "South Korea": ("Corea del Sur", "assets/flags/corea_del_sur_1948.svg"),
    "Korea, Republic of": ("Corea del Sur", "assets/flags/corea_del_sur_1948.svg"),
    "Saudi Arabia": ("Arabia Saudita", "assets/flags/arabia_saudita_1973.svg"),
    "Iran": ("Irán", "assets/flags/iran_1980.svg"),
    "Iran, Islamic Republic of": ("Irán", "assets/flags/iran_1980.svg"),
    "Iraq": ("Irak", "assets/flags/irak_2008.svg"),
    "Ukraine": ("Ucrania", "assets/flags/ucrania_1992.svg"),
    "Ireland": ("Irlanda", "assets/flags/irlanda_1922.svg"),
    "Austria": ("Austria", "assets/flags/austria_1945.svg"),
    "Czechia": ("Chequia", "assets/flags/chequia_1993.svg"),
    "Hungary": ("Hungría", "assets/flags/hungria_1957.svg"),
    "Romania": ("Rumania", "assets/flags/rumania_1989.svg"),
    "New Zealand": ("Nueva Zelanda", "assets/flags/nueva_zelanda_1902.svg"),
    "Philippines": ("Filipinas", "assets/flags/filipinas_1998.svg"),
    "Indonesia": ("Indonesia", "assets/flags/indonesia_1945.svg"),
    "Thailand": ("Tailandia", "assets/flags/tailandia_1917.svg"),
    "Vietnam": ("Vietnam", "assets/flags/vietnam_1976.svg"),
    "Nigeria": ("Nigeria", "assets/flags/nigeria_1960.svg"),
    "Kenya": ("Kenia", "assets/flags/kenia_1963.svg"),
}

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    geo_path = os.path.join(root, "assets", "geo", "countries.geojson")
    terrain_path = os.path.join(root, "assets", "maps", "earth_terrain_8192x4096.bin.gz")
    out_preset = os.path.join(root, "assets", "presets", "Tierra_con_Naciones_2026.matalas")
    os.makedirs(os.path.dirname(out_preset), exist_ok=True)

    print("1. Loading Earth Terrain Mask (8192x4096)...")
    with gzip.open(terrain_path, "rb") as f:
        terrain_bytes = bytearray(f.read())
    print(f"Terrain size: {len(terrain_bytes)} bytes")

    print("2. Loading GeoJSON boundaries...")
    with open(geo_path, encoding="utf-8") as f:
        geo = json.load(f)

    print("3. Rasterizing Sovereign Countries into Political Grid...")
    t0 = time.time()
    im = Image.new("I", (W, H), 0)
    draw = ImageDraw.Draw(im)

    def to_px(lon, lat):
        x = int(((lon + 180.0) / 360.0) * W)
        y = int(((90.0 - lat) / 180.0) * H)
        return (max(0, min(W - 1, x)), max(0, min(H - 1, y)))

    country_records = []
    
    # Check all flags in assets/flags/
    existing_flags = set()
    flags_dir = os.path.join(root, "assets", "flags")
    if os.path.exists(flags_dir):
        existing_flags = {os.path.join("assets", "flags", f).replace("\\", "/") for f in os.listdir(flags_dir)}

    for idx, feat in enumerate(geo["features"], start=1):
        props = feat.get("properties", {})
        eng_name = props.get("name", f"Nación {idx}")
        iso3 = props.get("ISO3166-1-Alpha-3", "")
        
        # Determine name, flag, and color
        spa_name, flag_path = NAME_MAP.get(eng_name, (eng_name, ""))
        if not flag_path or flag_path not in existing_flags:
            # Try fuzzy match in existing flags
            clean_name = eng_name.lower().replace(" ", "_").replace("-", "_")
            for f in existing_flags:
                if clean_name in f.lower():
                    flag_path = f
                    break

        col = PALETTE[(idx - 1) % len(PALETTE)]
        
        country_obj = {
            "id": idx,
            "name": spa_name,
            "color": {
                "r": col[0],
                "g": col[1],
                "b": col[2],
                "a": col[3]
            },
            "flag_path": flag_path,
            "pixel_count": 0
        }
        country_records.append(country_obj)

        geom = feat.get("geometry")
        if not geom:
            continue
        gtype = geom.get("type")
        coords = geom.get("coordinates", [])

        if gtype == "Polygon":
            for ring in coords:
                pts = [to_px(pt[0], pt[1]) for pt in ring]
                if len(pts) >= 3:
                    draw.polygon(pts, fill=idx)
        elif gtype == "MultiPolygon":
            for poly in coords:
                for ring in poly:
                    pts = [to_px(pt[0], pt[1]) for pt in ring]
                    if len(pts) >= 3:
                        draw.polygon(pts, fill=idx)

    print(f"Rasterized polygons in {time.time() - t0:.2f}s!")

    print("4. Applying Land Mask to Political Data...")
    raw_pol = im.tobytes() # 32-bit int per pixel (8192*4096*4)
    # Convert to 16-bit uint16 little endian, clipped to terrain == 1
    import struct
    import array
    
    # Read as 32-bit ints
    pol_ints = array.array('I')
    pol_ints.frombytes(raw_pol)
    
    # Political u16 array
    pol_u16 = array.array('H', [0] * (W * H))
    
    counts = {}
    for i in range(W * H):
        if terrain_bytes[i] == 1:
            cid = pol_ints[i]
            if cid > 0:
                pol_u16[i] = cid
                counts[cid] = counts.get(cid, 0) + 1
        else:
            pol_u16[i] = 0

    for c in country_records:
        c["pixel_count"] = counts.get(c["id"], 0)

    # Filter out empty or tiny unplaced territories to keep country list clean
    active_countries = [c for c in country_records if c["pixel_count"] > 10]
    print(f"Active sovereign states with land pixels: {len(active_countries)} countries!")

    print("5. Generating 512x256 Thumbnail...")
    thumb = Image.new("RGBA", (512, 256), (36, 79, 117, 255))
    draw_th = ImageDraw.Draw(thumb)
    step_x = W // 512
    step_y = H // 256
    
    c_color_map = {c["id"]: (c["color"]["r"], c["color"]["g"], c["color"]["b"], 255) for c in active_countries}

    for ty in range(256):
        sy = ty * step_y
        row_offset = sy * W
        for tx in range(512):
            sx = tx * step_x
            idx = row_offset + sx
            if pol_u16[idx] in c_color_map:
                thumb.putpixel((tx, ty), c_color_map[pol_u16[idx]])
            elif terrain_bytes[idx] == 1:
                thumb.putpixel((tx, ty), (78, 154, 81, 255))

    import io
    thumb_buf = io.BytesIO()
    thumb.save(thumb_buf, format="PNG")
    thumb_bytes = list(thumb_buf.getvalue())

    print("6. Compressing and Writing .matalas World File...")
    terrain_compressed = list(gzip.compress(bytes(terrain_bytes), compresslevel=6))
    pol_compressed = list(gzip.compress(pol_u16.tobytes(), compresslevel=6))

    now = int(time.time())
    save_file = {
        "header": {
            "version": 1,
            "name": "Tierra con Naciones (2026)",
            "created_at": now,
            "modified_at": now,
            "width": W,
            "height": H,
            "country_count": len(active_countries),
            "thumbnail_png": thumb_bytes
        },
        "countries": active_countries,
        "terrain_compressed": terrain_compressed,
        "political_compressed": pol_compressed
    }

    with open(out_preset, "w", encoding="utf-8") as f:
        json.dump(save_file, f)

    file_size_mb = os.path.getsize(out_preset) / (1024 * 1024)
    print(f"Successfully generated {out_preset} ({file_size_mb:.2f} MB)!")

if __name__ == "__main__":
    main()
