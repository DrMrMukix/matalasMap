import subprocess
import time
import os

adb = r"C:\Users\marti\AppData\Local\Android\Sdk\platform-tools\adb.exe"

def tap(x, y):
    subprocess.run([adb, "shell", "input", "tap", str(x), str(y)], check=True)

def screenshot(filename):
    with open(filename, "wb") as f:
        subprocess.run([adb, "exec-out", "screencap", "-p"], stdout=f, check=True)
    print(f"Captured: {filename}")

if __name__ == "__main__":
    # 1. Tap "3D" button (around x=1040, y=50)
    print("Tapping 3D mode...")
    tap(1040, 50)
    time.sleep(2)
    screenshot("tablet_screen_3d.png")

    # 2. Tap "2D" button (around x=930, y=50)
    print("Tapping back to 2D mode...")
    tap(930, 50)
    time.sleep(1)

    # 3. Tap "Ocultar" button (around x=2380, y=50)
    print("Tapping Ocultar (Zen Fullscreen mode)...")
    tap(2380, 50)
    time.sleep(1)
    screenshot("tablet_screen_zen.png")

    # 4. Tap "Mostrar Barras" button (around x=2420, y=50)
    print("Tapping Mostrar Barras...")
    tap(2420, 50)
    time.sleep(1)
    screenshot("tablet_screen_restored.png")
