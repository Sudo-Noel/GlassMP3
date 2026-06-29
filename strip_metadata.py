import os
from PIL import Image

files = [
    "skins/LightGlass/images/bkg_default.png",
    "skins/DarkGlass/images/bkg_default.png",
    "ICON0.PNG",
    "PIC1.PNG"
]

for f in files:
    try:
        img = Image.open(f)
        # convert to RGB to drop any weird profiles or alpha unless needed, wait PNG has alpha?
        # we can just putdata to a new image
        data = list(img.getdata())
        clean = Image.new(img.mode, img.size)
        clean.putdata(data)
        clean.save(f, format="PNG")
        print(f"Stripped {f}")
    except Exception as e:
        print(f"Failed {f}: {e}")
