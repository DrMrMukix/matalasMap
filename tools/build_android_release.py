#!/usr/bin/env python3
"""
Optimized Android Release Build for matalasMap
- Rust core: LTO fat + opt-level=3 + panic=abort + strip
- C++/Qt:    O3 + LTO + NDEBUG + dead-code stripping
- APK:       zipaligned + apksigned
- Output:    dist/matalasMap-android-arm64-release.apk
"""
import os, sys, subprocess, glob, shutil, time

ROOT    = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SDK     = r"C:\Users\marti\AppData\Local\Android\Sdk"
NDK     = r"C:\Users\marti\AppData\Local\Android\Sdk\ndk\26.1.10909125"
JDK     = r"C:\Program Files\Java\jdk-21.0.11"
ADB     = os.path.join(SDK, "platform-tools", "adb.exe")
APKSIGN = os.path.join(SDK, "build-tools", "35.0.0", "apksigner.bat")
ZIPALIGN= os.path.join(SDK, "build-tools", "35.0.0", "zipalign.exe")
KEYSTORE= os.path.join(ROOT, "debug.keystore")
DIST    = os.path.join(ROOT, "dist")
FINAL_APK = os.path.join(DIST, "matalasMap-android-arm64-release.apk")

def run(cmd, **kw):
    print(f"\n>>> {' '.join(cmd) if isinstance(cmd, list) else cmd}")
    ret = subprocess.run(cmd, **kw)
    if ret.returncode != 0:
        print(f"FAILED (exit {ret.returncode})")
        sys.exit(ret.returncode)
    return ret

def main():
    os.makedirs(DIST, exist_ok=True)

    env = os.environ.copy()
    env["JAVA_HOME"] = JDK
    env["ANDROID_SDK_ROOT"] = SDK
    env["ANDROID_NDK_ROOT"] = NDK
    env["PATH"] = os.path.join(JDK, "bin") + ";" + os.path.join(SDK, "platform-tools") + ";" + env.get("PATH","")

    # 1. Rust core with full LTO
    print("\n=== 1/5  Building Rust core (LTO + opt-level=3 + panic=abort) ===")
    run(["cargo", "build", "--release", "--target", "aarch64-linux-android",
         "--manifest-path", os.path.join(ROOT, "Cargo.toml")],
        env=env, cwd=ROOT)

    so_src = os.path.join(ROOT, "target", "aarch64-linux-android", "release", "libmatalas_core.so")
    if not os.path.exists(so_src):
        print(f"ERROR: {so_src} not found"); sys.exit(1)
    print(f"   .so size: {os.path.getsize(so_src)//1024} KB")

    # 2. Package assets
    print("\n=== 2/5  Packaging Android assets & JNI libs ===")
    run([sys.executable, os.path.join(ROOT, "tools", "package_android.py")], cwd=ROOT)

    # 3. CMake configure + build
    print("\n=== 3/5  CMake configure + compile (Release / O3 / LTO) ===")
    qt_cmake = r"C:\Qt\6.8.2\android_arm64_v8a\bin\qt-cmake.bat"
    build_dir = os.path.join(ROOT, "build_android")
    run([qt_cmake, "-B", build_dir, "-G", "Ninja",
         "-DCMAKE_BUILD_TYPE=Release",
         f"-DANDROID_SDK_ROOT={SDK.replace(chr(92),'/')}",
         f"-DANDROID_NDK_ROOT={NDK.replace(chr(92),'/')}",
         "-DANDROID_ABI=arm64-v8a", "-DANDROID_PLATFORM=android-34",
         "-DQT_HOST_PATH=C:/Qt/6.8.2/msvc2022_64",
         "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON"],
        env=env, cwd=ROOT)
    run(["cmake", "--build", build_dir, "--target", "apk", "--", "-j0"],
        env=env, cwd=ROOT)

    # 4. Sign + zipalign
    print("\n=== 4/5  Signing + zipaligning APK ===")
    candidates = glob.glob(os.path.join(build_dir, "**", "*.apk"), recursive=True)
    if not candidates:
        print("ERROR: No APK found"); sys.exit(1)
    unsigned = next((p for p in candidates if "unsigned" in p), candidates[0])
    print(f"   Unsigned APK: {unsigned}  ({os.path.getsize(unsigned)//1024} KB)")

    aligned = os.path.join(DIST, "matalasMap-aligned.apk")
    if os.path.exists(ZIPALIGN):
        run([ZIPALIGN, "-v", "-p", "4", unsigned, aligned])
    else:
        shutil.copy2(unsigned, aligned)

    if not os.path.exists(KEYSTORE):
        run([os.path.join(JDK,"bin","keytool.exe"),
             "-genkey","-v","-keystore",KEYSTORE,
             "-storepass","android","-alias","androiddebugkey",
             "-keypass","android","-keyalg","RSA","-keysize","2048",
             "-validity","10000","-dname","CN=Android Debug,O=Android,C=US"])

    if os.path.exists(FINAL_APK):
        os.remove(FINAL_APK)

    run([APKSIGN, "sign", "--ks", KEYSTORE, "--ks-pass", "pass:android",
         "--key-pass", "pass:android", "--ks-key-alias", "androiddebugkey",
         "--out", FINAL_APK, aligned])
    run([APKSIGN, "verify", "--verbose", FINAL_APK])

    size_mb = os.path.getsize(FINAL_APK) / (1024*1024)
    if os.path.exists(aligned): os.remove(aligned)

    print(f"\n  Final APK: {FINAL_APK}")
    print(f"  Size: {size_mb:.1f} MB")

    # 5. Deploy
    print("\n=== 5/5  Deploying to tablet via ADB ===")
    devices = subprocess.run([ADB, "devices"], capture_output=True, text=True).stdout
    print(devices)
    if "\tdevice" in devices:
        run([ADB, "install", "-r", "-d", FINAL_APK])
        subprocess.run([ADB, "shell", "am", "start", "-n",
            "org.qtproject.example.matalasMap/org.matalas.map.MatalasActivity"])
        time.sleep(4)
        subprocess.run([ADB, "shell", "screencap", "-p", "/sdcard/release_screen.png"])
        subprocess.run([ADB, "pull", "/sdcard/release_screen.png",
                        os.path.join(ROOT, "tablet_screen_release.png")])
        print("Screenshot guardada: tablet_screen_release.png")
    else:
        print("No hay dispositivo conectado. APK lista en la ruta indicada.")

    print(f"\n{'='*60}")
    print(f"  APK listo en:  {FINAL_APK}")
    print(f"{'='*60}\n")
    return 0

if __name__ == "__main__":
    sys.exit(main())
