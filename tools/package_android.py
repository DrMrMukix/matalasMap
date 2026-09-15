#!/usr/bin/env python3
"""
Sets up the Android package structure for matalasMap:
- Places compiled aarch64-linux-android/release/libmatalas_core.so into jniLibs/arm64-v8a/
- Copies 8K terrain PNG, SVG flags, and nations.json into Android assets/
- Creates AndroidManifest.xml, build.gradle, and tablet-optimized launcher metadata.
"""
import os
import sys
import shutil

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    android_dir = os.path.join(root, "dist", "android")
    jni_arm64 = os.path.join(android_dir, "app", "src", "main", "jniLibs", "arm64-v8a")
    assets_dir = os.path.join(android_dir, "app", "src", "main", "assets")
    res_dir = os.path.join(android_dir, "app", "src", "main", "res", "values")
    
    os.makedirs(jni_arm64, exist_ok=True)
    os.makedirs(assets_dir, exist_ok=True)
    os.makedirs(res_dir, exist_ok=True)
    
    # 1. Copy compiled Rust core shared library
    so_src = os.path.join(root, "target", "aarch64-linux-android", "release", "libmatalas_core.so")
    if os.path.exists(so_src):
        shutil.copy2(so_src, jni_arm64)
        print(f"Copied {so_src} -> {jni_arm64}")
    else:
        print(f"Warning: {so_src} not found!")

    # 2. Copy game assets and content into Android assets
    shutil.copytree(os.path.join(root, "assets"), os.path.join(assets_dir, "assets"), dirs_exist_ok=True)
    shutil.copytree(os.path.join(root, "content"), os.path.join(assets_dir, "content"), dirs_exist_ok=True)
    print("Copied assets and content into Android assets.")

    # 3. Generate crisp launcher icons
    from PIL import Image, ImageDraw
    for density, size in [("", 192), ("-mdpi", 48), ("-hdpi", 72), ("-xhdpi", 96), ("-xxhdpi", 144), ("-xxxhdpi", 192)]:
        m_dir = os.path.join(android_dir, "app", "src", "main", "res", f"mipmap{density}")
        os.makedirs(m_dir, exist_ok=True)
        img = Image.new("RGBA", (size, size), (15, 23, 42, 255))
        draw = ImageDraw.Draw(img)
        # Outer ring
        draw.ellipse([size * 0.08, size * 0.08, size * 0.92, size * 0.92], outline=(56, 189, 248, 255), width=max(2, size // 24))
        # Inner sphere
        draw.ellipse([size * 0.15, size * 0.15, size * 0.85, size * 0.85], fill=(37, 99, 235, 255))
        # Continental shape
        draw.polygon([(size * 0.3, size * 0.3), (size * 0.6, size * 0.25), (size * 0.7, size * 0.55), (size * 0.45, size * 0.7), (size * 0.25, size * 0.5)], fill=(16, 185, 129, 255))
        img.save(os.path.join(m_dir, "ic_launcher.png"), "PNG")
    print("Generated launcher mipmap icons.")

    # 4. Create MatalasActivity.java for Sticky Immersive Fullscreen Mode
    java_dir = os.path.join(android_dir, "app", "src", "main", "java", "org", "matalas", "map")
    os.makedirs(java_dir, exist_ok=True)
    activity_path = os.path.join(java_dir, "MatalasActivity.java")
    activity_content = """package org.matalas.map;

import android.os.Bundle;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import org.qtproject.qt.android.bindings.QtActivity;

public class MatalasActivity extends QtActivity {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        hideSystemUI();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    private void hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Window window = getWindow();
            if (window != null) {
                window.setDecorFitsSystemWindows(false);
                WindowInsetsController controller = window.getInsetsController();
                if (controller != null) {
                    controller.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
                    controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                }
            }
        } else {
            View decorView = getWindow().getDecorView();
            if (decorView != null) {
                int flags = View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                          | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                          | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                          | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                          | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                          | View.SYSTEM_UI_FLAG_FULLSCREEN;
                decorView.setSystemUiVisibility(flags);
            }
        }
    }
}
"""
    with open(activity_path, 'w', encoding='utf-8') as f:
        f.write(activity_content)
    print("Generated MatalasActivity.java for immersive sticky fullscreen.")

    # 5. Create AndroidManifest.xml optimized for tablets with fullscreen theme
    manifest_path = os.path.join(android_dir, "app", "src", "main", "AndroidManifest.xml")
    manifest_content = """<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android">

    <supports-screens
        android:smallScreens="false"
        android:normalScreens="true"
        android:largeScreens="true"
        android:xlargeScreens="true"
        android:anyDensity="true" />

    <application
        android:label="@string/app_name"
        android:allowBackup="true"
        android:hardwareAccelerated="true"
        android:largeHeap="true"
        android:icon="@mipmap/ic_launcher">

        <activity
            android:name="org.matalas.map.MatalasActivity"
            android:label="@string/app_name"
            android:theme="@android:style/Theme.NoTitleBar.Fullscreen"
            android:configChanges="orientation|uiMode|screenLayout|screenSize|smallestScreenSize|layoutDirection|locale|fontScale|keyboard|keyboardHidden|navigation|mcc|mnc|density"
            android:screenOrientation="sensorLandscape"
            android:launchMode="singleTop"
            android:exported="true">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
            <meta-data android:name="android.app.lib_name" android:value="matalasMap" />
        </activity>
    </application>
</manifest>
"""
    with open(manifest_path, 'w', encoding='utf-8') as f:
        f.write(manifest_content)

    # 5. Strings resource
    strings_path = os.path.join(res_dir, "strings.xml")
    strings_content = """<?xml version="1.0" encoding="utf-8"?>
<resources>
    <string name="app_name">matalasMap</string>
</resources>
"""
    with open(strings_path, 'w', encoding='utf-8') as f:
        f.write(strings_content)

    print(f"Android deployment files ready at {android_dir}")
    return 0

if __name__ == '__main__':
    sys.exit(main())
