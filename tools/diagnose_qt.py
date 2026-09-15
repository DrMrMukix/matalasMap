#!/usr/bin/env python3
import os
import sys
import subprocess
import time

def test_executable(exe_path, cwd):
    print(f"\n==========================================")
    print(f"Testing: {exe_path}")
    print(f"Working Dir: {cwd}")
    print(f"Exists: {os.path.exists(exe_path)}")
    if not os.path.exists(exe_path):
        return
        
    env = os.environ.copy()
    env["QT_DEBUG_PLUGINS"] = "1"
    env["QML_IMPORT_TRACE"] = "1"
    
    # Run process with timeout
    try:
        p = subprocess.Popen(
            [os.path.abspath(exe_path)],
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=env,
            text=True
        )
        
        # Wait up to 3 seconds to see if it crashes or continues running
        time.sleep(2.5)
        poll = p.poll()
        if poll is None:
            print("STATUS: Process is RUNNING successfully!")
            p.terminate()
            try:
                p.wait(timeout=2)
            except Exception:
                p.kill()
        else:
            print(f"STATUS: Process EXITED with code {poll} (0x{poll & 0xFFFFFFFF:X})")
            stdout, stderr = p.communicate()
            print("--- STDOUT ---")
            print(stdout[:2000])
            print("--- STDERR ---")
            print(stderr[:4000])
    except Exception as e:
        print(f"ERROR launching process: {e}")

def check_adb():
    print(f"\n==========================================")
    print("Checking ADB and connected devices...")
    adb_path = r"C:\Users\marti\AppData\Local\Android\Sdk\platform-tools\adb.exe"
    print(f"ADB Exists: {os.path.exists(adb_path)}")
    try:
        res = subprocess.run([adb_path, "devices", "-l"], capture_output=True, text=True, timeout=10)
        print("ADB devices output:")
        print(res.stdout)
        if res.stderr:
            print("ADB stderr:", res.stderr)
    except Exception as e:
        print("ADB error:", e)

if __name__ == '__main__':
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    # Test 1: build/matalasMap.exe (Debug)
    test_executable(os.path.join(root, "build", "matalasMap.exe"), os.path.join(root, "build"))
    
    # Test 2: build_release/matalasMap.exe (Release)
    test_executable(os.path.join(root, "build_release", "matalasMap.exe"), os.path.join(root, "build_release"))
    
    # Test 3: dist/matalasMap-windows-portable/matalasMap.exe (Portable)
    test_executable(os.path.join(root, "dist", "matalasMap-windows-portable", "matalasMap.exe"), os.path.join(root, "dist", "matalasMap-windows-portable"))
    
    # Test 4: Check ADB tablet connection
    check_adb()
