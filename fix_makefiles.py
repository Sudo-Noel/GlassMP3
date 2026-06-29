import os
import glob

files_to_fix = [
    "cooleyesBridge/src/Makefile",
    "miniconv/Makefile.eur",
    "miniconv/Makefile.all",
    "miniconv/Makefile.cjk",
    "prx/Makefile"
]

for file in files_to_fix:
    path = os.path.join("/Users/noel/.gemini/antigravity/scratch/modern-psp-player/GlassMP3", file)
    if os.path.exists(path):
        with open(path, "r") as f:
            content = f.read()
        
        content = content.replace("-mno-crt0 -nostartfiles", "-nostartfiles")
        
        with open(path, "w") as f:
            f.write(content)
        print(f"Fixed {file}")
