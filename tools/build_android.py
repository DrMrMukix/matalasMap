#!/usr/bin/env python3
"""
Automates building and deploying matalasMap for Android ARM64:
1. Prepares Android assets, JNI libs, and manifest.
2. Invokes Qt 6.8.2 Android CMake toolchain.
3. Builds the APK.
4. Installs and launches on the connected tablet via ADB.
"""
import os
import sys
import subprocess
import glob

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    # 1. Prepare Android assets and package
    print("--- 1. Packaging Android Assets and JNI Libs ---")
    pkg_script = os.path.join(root, "tools", "package_android.py")
    subprocess.run([sys.executable, pkg_script], check=True, cwd=root)

    # 2. Setup Environment
    env = os.environ.copy()
    jdk_path = r"C:\Program Files\Java\jdk-21.0.11"
    sdk_path = r"C:\Users\marti\AppData\Local\Android\Sdk"
    ndk_path = r"C:\Users\marti\AppData\Local\Android\Sdk\ndk\26.1.10909125"
    adb_path = os.path.join(sdk_path, "platform-tools", "adb.exe")

    env["JAVA_HOME"] = jdk_path
    env["ANDROID_SDK_ROOT"] = sdk_path
    env["ANDROID_NDK_ROOT"] = ndk_path
    env["PATH"] = os.path.join(jdk_path, "bin") + ";" + os.path.join(sdk_path, "platform-tools") + ";" + env.get("PATH", "")

    # 3. Configure with Qt CMake for Android
    print("--- 2. Configuring CMake with Qt 6.8.2 Android Toolchain ---")
    qt_cmake = r"C:\Qt\6.8.2\android_arm64_v8a\bin\qt-cmake.bat"
    cfg_cmd = [
        qt_cmake,
        "-B", "build_android",
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        f"-DANDROID_SDK_ROOT={sdk_path.replace('\\', '/')}",
        f"-DANDROID_NDK_ROOT={ndk_path.replace('\\', '/')}",
        "-DANDROID_ABI=arm64-v8a",
        "-DANDROID_PLATFORM=android-34",
        "-DQT_HOST_PATH=C:/Qt/6.8.2/msvc2022_64"
    ]
    ret = subprocess.run(cfg_cmd, env=env, cwd=root)
    if ret.returncode != 0:
        print("Error configuring CMake for Android!")
        return ret.returncode

    # 4. Build APK target
    print("--- 3. Compiling Android ARM64 Target & APK ---")
    build_cmd = ["cmake", "--build", "build_android", "--target", "apk"]
    ret = subprocess.run(build_cmd, env=env, cwd=root)
    if ret.returncode != 0:
        print("Error building Android APK target!")
        return ret.returncode

    # 5. Locate APK & Sign
    print("--- 4. Locating Built APK and Signing ---")
    apk_candidates = glob.glob(os.path.join(root, "build_android", "**", "*.apk"), recursive=True)
    if not apk_candidates:
        print("No APK found in build_android!")
        return 1

    apk_path = apk_candidates[0]
    print(f"Found APK: {apk_path}")

    # Copy APK to dist/
    dist_apk = os.path.join(root, "dist", "matalasMap-android-arm64.apk")
    import shutil
    shutil.copy2(apk_path, dist_apk)
    print(f"Copied APK to: {dist_apk}")

    # Ensure debug keystore exists
    keystore_path = os.path.join(root, "debug.keystore")
    keytool_path = os.path.join(jdk_path, "bin", "keytool.exe")
    if not os.path.exists(keystore_path):
        print("Generating debug keystore...")
        subprocess.run([
            keytool_path, "-genkey", "-v",
            "-keystore", keystore_path,
            "-storepass", "android",
            "-alias", "androiddebugkey",
            "-keypass", "android",
            "-keyalg", "RSA",
            "-keysize", "2048",
            "-validity", "10000",
            "-dname", "CN=Android Debug,O=Android,C=US"
        ], check=True)

    # Sign APK with apksigner
    apksigner_path = os.path.join(sdk_path, "build-tools", "35.0.0", "apksigner.bat")
    if not os.path.exists(apksigner_path):
        apksigner_path = os.path.join(sdk_path, "build-tools", "34.0.0", "apksigner.bat")

    print(f"Signing APK with {apksigner_path}...")
    subprocess.run([
        apksigner_path, "sign",
        "--ks", keystore_path,
        "--ks-pass", "pass:android",
        "--key-pass", "pass:android",
        "--ks-key-alias", "androiddebugkey",
        dist_apk
    ], check=True)

    print("Verifying signed APK...")
    subprocess.run([apksigner_path, "verify", dist_apk], check=True)
    print("APK signed and verified successfully!")

    # 6. Deploy to connected ADB device
    print("--- 5. Deploying to Tablet via ADB ---")
    devices_out = subprocess.run([adb_path, "devices"], capture_output=True, text=True).stdout
    print(devices_out)

    if "device\n" in devices_out or "\tdevice" in devices_out:
        print(f"Installing {dist_apk} to tablet...")
        install_res = subprocess.run([adb_path, "install", "-r", dist_apk])
        if install_res.returncode == 0:
            print("Launching matalasMap on tablet...")
            subprocess.run([adb_path, "shell", "am", "start", "-n", "org.qtproject.example.matalasMap/org.matalas.map.MatalasActivity"])
            print("Successfully launched on tablet!")
            
            # Wait 3 seconds and take screenshot to verify
            import time
            time.sleep(3)
            screenshot_path = os.path.join(root, "tablet_screen.png")
            with open(screenshot_path, "wb") as sc_file:
                subprocess.run([adb_path, "exec-out", "screencap", "-p"], stdout=sc_file)
            print(f"Captured tablet screenshot: {screenshot_path}")

if __name__ == '__main__':
    sys.exit(main())
