import os
from PIL import Image

def resize_and_convert(src, dst, size):
    try:
        img = Image.open(src)
        img = img.resize(size, Image.Resampling.LANCZOS)
        img.save(dst, "PNG")
        print(f"Saved {dst} ({size[0]}x{size[1]})")
    except Exception as e:
        print(f"Error processing {src}: {e}")

icon_src = "/Users/noel/.gemini/antigravity/brain/3d7ff54d-97a6-4bcf-a6bd-19a4e97a1767/glassmp3_icon_1782671755664.jpg"
bg_src = "/Users/noel/.gemini/antigravity/brain/3d7ff54d-97a6-4bcf-a6bd-19a4e97a1767/glassmp3_bg_1782671764057.jpg"

icon_dst = "/Users/noel/.gemini/antigravity/scratch/modern-psp-player/GlassMP3/ICON0.PNG"
bg_dst = "/Users/noel/.gemini/antigravity/scratch/modern-psp-player/GlassMP3/PIC1.PNG"

resize_and_convert(icon_src, icon_dst, (144, 80))
resize_and_convert(bg_src, bg_dst, (480, 272))
