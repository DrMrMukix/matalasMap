#!/usr/bin/env python3
"""
Downloads all missing flags for nations in content/nations/nations.json politely
with retry logic, rate limit handling, and SVG fallbacks.
"""
import os
import sys
import json
import time
import urllib.request
import urllib.parse
from concurrent.futures import ThreadPoolExecutor

USER_AGENT = "MatalasMap/1.0 (educational tool; contact: marti@matalas.local)"

def generate_fallback_svg(name, color_rgba, out_path):
    r, g, b = color_rgba[0], color_rgba[1], color_rgba[2]
    initial = (name[0] if name else "?").upper()
    svg = f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 600" width="900" height="600">
  <rect width="900" height="600" fill="rgb({r},{g},{b})"/>
  <rect x="50" y="50" width="800" height="500" fill="none" stroke="#FFFFFF" stroke-width="20" stroke-opacity="0.4"/>
  <circle cx="450" cy="300" r="140" fill="#FFFFFF" fill-opacity="0.25"/>
  <text x="450" y="360" font-size="180" font-weight="bold" font-family="sans-serif" text-anchor="middle" fill="#FFFFFF">{initial}</text>
</svg>"""
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write(svg)

def download_one(entry):
    flag_rel = entry['flag']
    out_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), flag_rel)
    
    if os.path.exists(out_path) and os.path.getsize(out_path) > 100:
        return True, "exists"
        
    url = entry['source_url'].replace('http://', 'https://')
    
    for attempt in range(3):
        try:
            req = urllib.request.Request(url, headers={'User-Agent': USER_AGENT})
            with urllib.request.urlopen(req, timeout=12) as resp:
                data = resp.read()
                if len(data) > 100:
                    with open(out_path, 'wb') as f:
                        f.write(data)
                    return True, "downloaded"
        except urllib.error.HTTPError as e:
            if e.code == 429: # Rate limit
                time.sleep(1.5 * (attempt + 1))
            else:
                break
        except Exception:
            time.sleep(0.5)

    # If download failed or file unavailable on Wikimedia, generate a beautiful fallback SVG
    generate_fallback_svg(entry['name'], entry.get('color', [100, 100, 100]), out_path)
    return True, "fallback"

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    json_path = os.path.join(root, "content", "nations", "nations.json")
    flags_dir = os.path.join(root, "assets", "flags")
    os.makedirs(flags_dir, exist_ok=True)
    
    with open(json_path, 'r', encoding='utf-8') as f:
        nations = json.load(f)
        
    print(f"Checking flags for {len(nations)} nations...")
    
    success = 0
    with ThreadPoolExecutor(max_workers=6) as pool:
        results = pool.map(download_one, nations)
        for r in results:
            if r[0]:
                success += 1

    print(f"All flags ready! {success}/{len(nations)} available locally in assets/flags/.")
    return 0

if __name__ == '__main__':
    sys.exit(main())
