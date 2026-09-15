import json
import time
from PIL import Image, ImageDraw

t0 = time.time()
with open('assets/geo/countries.geojson', encoding='utf-8') as f:
    geo = json.load(f)

W, H = 8192, 4096
im = Image.new('I', (W, H), 0)
draw = ImageDraw.Draw(im)

def to_px(lon, lat):
    x = int(((lon + 180.0) / 360.0) * W)
    y = int(((90.0 - lat) / 180.0) * H)
    return (max(0, min(W - 1, x)), max(0, min(H - 1, y)))

poly_count = 0
for idx, feat in enumerate(geo['features'], start=1):
    geom = feat.get('geometry')
    if not geom:
        continue
    gtype = geom.get('type')
    coords = geom.get('coordinates', [])
    cid = idx
    
    if gtype == 'Polygon':
        for ring in coords:
            pts = [to_px(pt[0], pt[1]) for pt in ring]
            if len(pts) >= 3:
                draw.polygon(pts, fill=cid)
                poly_count += 1
    elif gtype == 'MultiPolygon':
        for poly in coords:
            for ring in poly:
                pts = [to_px(pt[0], pt[1]) for pt in ring]
                if len(pts) >= 3:
                    draw.polygon(pts, fill=cid)
                    poly_count += 1

elapsed = time.time() - t0
print(f"Done in {elapsed:.2f}s! Drew {poly_count} polygons for {len(geo['features'])} countries.")
