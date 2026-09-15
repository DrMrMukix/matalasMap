#!/usr/bin/env python3
"""
MatalasMap Automated Historical & Modern Nations Importer (1650-2026)
Queries Wikidata and Wikimedia Commons to automatically extract:
- Sovereign states and historical countries/empires/republics from 1650 to 2026
- Spanish name (with English fallback)
- Official flag SVG downloaded and stored locally in assets/flags/
- Validity dates (min_year, max_year)
- Palette color, source provenance, license, and Wikidata ID
Saves all entries directly into content/nations/nations.json.
"""

import os
import sys
import json
import re
import hashlib
import urllib.request
import urllib.parse
from concurrent.futures import ThreadPoolExecutor, as_completed

USER_AGENT = "MatalasMap/1.0 (creative educational map editor; contact: marti@matalas.local)"

SPARQL_QUERY = """
SELECT DISTINCT ?item ?itemLabel ?itemLabelEn ?flag ?inception ?dissolved ?endTime WHERE {
  { ?item wdt:P31 wd:Q3024240 . }
  UNION
  { ?item wdt:P31 wd:Q6256 . }
  UNION
  { ?item wdt:P31 wd:Q3624078 . }
  
  ?item wdt:P41 ?flag .
  OPTIONAL { ?item wdt:P571 ?inception . }
  OPTIONAL { ?item wdt:P576 ?dissolved . }
  OPTIONAL { ?item wdt:P582 ?endTime . }
  OPTIONAL { ?item rdfs:label ?itemLabel . FILTER(LANG(?itemLabel) = 'es') }
  OPTIONAL { ?item rdfs:label ?itemLabelEn . FILTER(LANG(?itemLabelEn) = 'en') }
}
"""

def parse_year(dt_str):
    if not dt_str:
        return None
    m = re.match(r'^(-?\d+)', dt_str)
    return int(m.group(1)) if m else None

def slugify(text):
    text = text.lower()
    text = re.sub(r'[áàäâ]', 'a', text)
    text = re.sub(r'[éèëê]', 'e', text)
    text = re.sub(r'[íìïî]', 'i', text)
    text = re.sub(r'[óòöô]', 'o', text)
    text = re.sub(r'[úùüû]', 'u', text)
    text = re.sub(r'[ñ]', 'n', text)
    text = re.sub(r'[^a-z0-9]+', '_', text).strip('_')
    return text[:30]

def deterministic_color(key_str):
    h = hashlib.sha256(key_str.encode('utf-8')).hexdigest()
    r = int(h[0:2], 16)
    g = int(h[2:4], 16)
    b = int(h[4:6], 16)
    # Ensure colors aren't too dark or pure white for good visibility on map
    r = max(40, min(220, r))
    g = max(40, min(220, g))
    b = max(40, min(220, b))
    return [r, g, b, 255]

def download_flag(url, target_path):
    if os.path.exists(target_path) and os.path.getsize(target_path) > 50:
        return True, "cached"
    try:
        req = urllib.request.Request(url, headers={'User-Agent': USER_AGENT})
        with urllib.request.urlopen(req, timeout=15) as resp:
            content = resp.read()
            if len(content) < 50:
                return False, "too small"
            with open(target_path, 'wb') as f:
                f.write(content)
            return True, "downloaded"
    except Exception as e:
        return False, str(e)

def main():
    print("=== MatalasMap Automated Historical Nations Fetcher (1650-2026) ===")
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    root_dir = os.path.abspath(os.path.join(script_dir, ".."))
    flags_dir = os.path.join(root_dir, "assets", "flags")
    nations_dir = os.path.join(root_dir, "content", "nations")
    os.makedirs(flags_dir, exist_ok=True)
    os.makedirs(nations_dir, exist_ok=True)
    
    nations_json_path = os.path.join(nations_dir, "nations.json")

    print("1. Querying Wikidata SPARQL endpoint...")
    url = 'https://query.wikidata.org/sparql?' + urllib.parse.urlencode({'query': SPARQL_QUERY, 'format': 'json'})
    req = urllib.request.Request(url, headers={'User-Agent': USER_AGENT, 'Accept': 'application/json'})
    
    try:
        with urllib.request.urlopen(req, timeout=60) as resp:
            data = json.loads(resp.read().decode('utf-8'))
    except Exception as e:
        print(f"Error querying Wikidata SPARQL: {e}", file=sys.stderr)
        return 1

    bindings = data.get('results', {}).get('bindings', [])
    print(f"Received {len(bindings)} total raw entries from Wikidata.")

    # Process and filter nations
    filtered = []
    seen_qids = set()

    for b in bindings:
        qid = b['item']['value'].split('/')[-1]
        if qid in seen_qids:
            continue

        name_es = b.get('itemLabel', {}).get('value')
        name_en = b.get('itemLabelEn', {}).get('value')
        name = name_es or name_en

        if not name or (name.startswith('Q') and name[1:].isdigit()):
            continue

        flag_url = b.get('flag', {}).get('value')
        if not flag_url:
            continue

        inc = parse_year(b.get('inception', {}).get('value'))
        dis = parse_year(b.get('dissolved', {}).get('value')) or parse_year(b.get('endTime', {}).get('value'))

        # Date validation for 1650-2026:
        # If dissolved before 1650, skip
        if dis is not None and dis < 1650:
            continue
        # If born after 2026, skip
        if inc is not None and inc > 2026:
            continue
        # If ancient entity (< 1400) without dissolution date, skip
        if inc is not None and inc < 1400 and dis is None:
            continue

        min_year = inc if (inc is not None and inc >= 1650) else 1650
        max_year = dis if (dis is not None and dis <= 2026) else 2026

        if min_year > max_year:
            # Swap if inverted
            min_year, max_year = max_year, min_year

        seen_qids.add(qid)
        filtered.append({
            "qid": qid,
            "name": name,
            "min_year": min_year,
            "max_year": max_year,
            "flag_url": flag_url,
        })

    print(f"2. Filtered to {len(filtered)} valid nations/states existing in 1650-2026.")

    # Deduplicate flags and plan downloads
    # Build clean IDs
    used_ids = set()
    nations_list = []
    download_tasks = []

    for item in filtered:
        base_slug = slugify(item['name'])
        slug_id = f"{base_slug}_{item['min_year']}"
        if slug_id in used_ids:
            slug_id = f"{base_slug}_{item['qid'].lower()}"
        used_ids.add(slug_id)

        # Flag file naming
        raw_flag_name = item['flag_url'].split('Special:FilePath/')[-1]
        
        # Keep extension
        ext = ".svg"
        if raw_flag_name.lower().endswith('.png'):
            ext = ".png"
            
        local_flag_filename = f"{slug_id}{ext}"
        local_flag_rel = f"assets/flags/{local_flag_filename}"
        local_flag_abs = os.path.join(flags_dir, local_flag_filename)

        # Direct download link: simply convert to https, preserving existing encoding
        download_url = item['flag_url'].replace('http://', 'https://')

        download_tasks.append((download_url, local_flag_abs, item['name']))

        color = deterministic_color(item['name'] + item['qid'])

        entry = {
            "id": slug_id,
            "name": item['name'],
            "min_year": item['min_year'],
            "max_year": item['max_year'],
            "flag": local_flag_rel.replace('\\', '/'),
            "emblem": None,
            "color": color,
            "source": "Wikidata / Wikimedia Commons",
            "wikidata_id": item['qid'],
            "license": "Public domain / CC",
            "author": "Wikimedia Commons contributors",
            "source_url": item['flag_url']
        }
        nations_list.append(entry)

    # Sort nations alphabetically by name, or by min_year
    nations_list.sort(key=lambda x: (x['min_year'], x['name']))

    print(f"3. Downloading flags to assets/flags/ using 8 worker threads...")
    successful_downloads = 0
    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {executor.submit(download_flag, url, path): (name, path) for url, path, name in download_tasks}
        for future in as_completed(futures):
            name, path = futures[future]
            try:
                ok, status = future.result()
                if ok:
                    successful_downloads += 1
            except Exception as e:
                pass

    print(f"Flags ready: {successful_downloads}/{len(download_tasks)} successfully available locally.")

    # 4. Save nations.json
    print(f"4. Saving {len(nations_list)} nations to {nations_json_path}...")
    with open(nations_json_path, 'w', encoding='utf-8') as f:
        json.dump(nations_list, f, ensure_ascii=False, indent=2)

    print("SUCCESS: Automated historical & modern nation library complete!")
    return 0

if __name__ == '__main__':
    sys.exit(main())
