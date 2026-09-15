import os
import subprocess
import sys

vcvars = r"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

cmd = f'call "{vcvars}" && cmake -B build_release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/msvc2022_64" && cmake --build build_release --config Release'
print(f"Building Release executable with MSVC: {cmd}")
ret = subprocess.run(cmd, shell=True, cwd=root)
sys.exit(ret.returncode)
