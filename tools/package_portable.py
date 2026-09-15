#!/usr/bin/env python3
"""
Creates a fully self-contained portable distribution of matalasMap for Windows
and packages it into a ZIP file.
"""
import os
import sys
import shutil
import subprocess
import zipfile

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    dist_dir = os.path.join(root, "dist", "matalasMap-windows-portable")
    zip_path = os.path.join(root, "dist", "matalasMap-windows-portable.zip")
    
    os.makedirs(os.path.join(root, "dist"), exist_ok=True)
    if os.path.exists(dist_dir):
        shutil.rmtree(dist_dir)
    os.makedirs(dist_dir, exist_ok=True)
    
    print("1. Copying executables and core libraries...")
    exe_src = os.path.join(root, "build_release", "matalasMap.exe")
    dll_src = os.path.join(root, "target", "release", "matalas_core.dll")
    
    shutil.copy2(exe_src, dist_dir)
    if os.path.exists(dll_src):
        shutil.copy2(dll_src, dist_dir)
        
    print("2. Running windeployqt to deploy all Qt dependencies...")
    windeployqt = r"C:\Qt\6.8.2\msvc2022_64\bin\windeployqt.exe"
    cmd = [
        windeployqt,
        "--qmldir", os.path.join(root, "qml"),
        "--release",
        "--compiler-runtime",
        os.path.join(dist_dir, "matalasMap.exe")
    ]
    subprocess.run(cmd, check=True)
    
    print("3. Copying assets, content, and QML...")
    shutil.copytree(os.path.join(root, "assets"), os.path.join(dist_dir, "assets"), dirs_exist_ok=True)
    shutil.copytree(os.path.join(root, "content"), os.path.join(dist_dir, "content"), dirs_exist_ok=True)
    shutil.copytree(os.path.join(root, "qml"), os.path.join(dist_dir, "qml"), dirs_exist_ok=True)
    
    # 4. Create launcher batch and README
    bat_content = """@echo off
start "" "%~dp0matalasMap.exe"
"""
    with open(os.path.join(dist_dir, "INICIAR_MATALASMAP.bat"), "w", encoding="utf-8") as f:
        f.write(bat_content)
        
    readme_content = """===================================================
matalasMap - Editor Creativo de Mapas Historicos
Version Portable para Windows (Sin Instalacion)
===================================================

Como jugar / usar:
1. Haz doble clic en 'matalasMap.exe' o en 'INICIAR_MATALASMAP.bat'.
2. El juego abrira directamente el mapa creativo en 8K con:
   - Modo Terreno (dibujar continentes e islas).
   - Modo Politico (crear imperios, pintar fronteras y banderas).
   - Vista 2D y Globo 3D interactivo.
   - Biblioteca de mas de 1.100 naciones historicas (1650-2026).
   - Todo funciona de forma local y offline.

¡Que te diviertas inventando mundos!
"""
    with open(os.path.join(dist_dir, "LEEME.txt"), "w", encoding="utf-8") as f:
        f.write(readme_content)

    print("4. Creating portable ZIP archive (dist/matalasMap-windows-portable.zip)...")
    if os.path.exists(zip_path):
        os.remove(zip_path)
        
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zf:
        for root_folder, dirs, files in os.walk(dist_dir):
            for file in files:
                full_p = os.path.join(root_folder, file)
                rel_p = os.path.relpath(full_p, os.path.dirname(dist_dir))
                zf.write(full_p, rel_p)
                
    print(f"SUCCESS: Windows portable app ready at:\n  Folder: {dist_dir}\n  ZIP: {zip_path}")
    return 0

if __name__ == '__main__':
    sys.exit(main())
