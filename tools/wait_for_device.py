import os
import subprocess
import time
import sys

adb = r"C:\Users\marti\AppData\Local\Android\Sdk\platform-tools\adb.exe"

print("Iniciando servidor ADB...")
subprocess.run([adb, "start-server"])

print("\n>>> POR FAVOR MIRA TU TABLET:")
print("1. Desbloquea la pantalla.")
print("2. Marca la casilla 'Permitir siempre desde esta computadora' si aparece.")
print("3. Toca 'Permitir' o 'Aceptar'.")
print("Esperando autorizacion (tienes hasta 90 segundos)...\n")

for i in range(30):
    res = subprocess.run([adb, "devices"], capture_output=True, text=True)
    lines = [line.strip() for line in res.stdout.strip().split('\n') if line.strip()]
    print(f"[{i*3}s] Estado: {lines}")
    
    device_ready = False
    for line in lines[1:]: # skip header
        if "\tdevice" in line:
            device_ready = True
            break
            
    if device_ready:
        print("\n¡EXCELENTE! La tablet esta autorizada y lista:")
        subprocess.run([adb, "devices"])
        # Print device model
        m = subprocess.run([adb, "shell", "getprop", "ro.product.model"], capture_output=True, text=True)
        print(f"Modelo detectado: {m.stdout.strip()}")
        sys.exit(0)
        
    time.sleep(3)

print("\nTiempo de espera agotado. El dispositivo sigue sin autorizar.")
sys.exit(1)
