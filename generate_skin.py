import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

SKIN_DIR = "skins/default/images"

def create_gradient_bg(width, height, color1, color2, filename):
    img = Image.new("RGBA", (width, height))
    draw = ImageDraw.Draw(img)
    for y in range(height):
        r = int(color1[0] + (color2[0] - color1[0]) * y / height)
        g = int(color1[1] + (color2[1] - color1[1]) * y / height)
        b = int(color1[2] + (color2[2] - color1[2]) * y / height)
        draw.line([(0, y), (width, y)], fill=(r, g, b, 255))
    img.save(os.path.join(SKIN_DIR, filename))

def create_glass_panel(width, height, filename, radius=8, alpha=100, border_alpha=80):
    img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Fill background
    draw.rounded_rectangle([(0, 0), (width - 1, height - 1)], radius=radius, fill=(30, 30, 40, alpha))
    
    # Border highlight (top/left bright, bottom/right dim)
    draw.rounded_rectangle([(0, 0), (width - 1, height - 1)], radius=radius, outline=(255, 255, 255, border_alpha), width=1)
    
    img.save(os.path.join(SKIN_DIR, filename))

def create_toolbar(width, height, filename):
    img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle([(0, 0), (width, height)], fill=(15, 15, 25, 200))
    draw.line([(0, height - 1), (width, height - 1)], fill=(255, 255, 255, 50))
    img.save(os.path.join(SKIN_DIR, filename))

def create_progress_bar(width, height, filename, color):
    img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([(0, 0), (width - 1, height - 1)], radius=height//2, fill=color)
    img.save(os.path.join(SKIN_DIR, filename))

def create_button(width, height, filename, text, selected=False):
    img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    if selected:
        fill_color = (80, 120, 255, 150)
        border_color = (255, 255, 255, 150)
    else:
        fill_color = (30, 30, 40, 100)
        border_color = (255, 255, 255, 50)
    
    draw.rounded_rectangle([(0, 0), (width - 1, height - 1)], radius=6, fill=fill_color, outline=border_color, width=1)
    img.save(os.path.join(SKIN_DIR, filename))

def create_icon(size, filename, icon_type):
    img = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    # Simple placeholder icons for now, drawn with lines
    cx, cy = size[0]//2, size[1]//2
    if icon_type == "folder":
        draw.polygon([(cx-10, cy-6), (cx-4, cy-6), (cx-2, cy-4), (cx+10, cy-4), (cx+10, cy+6), (cx-10, cy+6)], fill=(120, 180, 255, 220), outline=(255,255,255,255))
    elif icon_type == "music":
        draw.ellipse([(cx-6, cy+2), (cx-2, cy+6)], fill=(255, 100, 150, 220))
        draw.ellipse([(cx+2, cy+2), (cx+6, cy+6)], fill=(255, 100, 150, 220))
        draw.line([(cx-2, cy+4), (cx-2, cy-6), (cx+6, cy-6), (cx+6, cy+4)], fill=(255, 255, 255, 255), width=2)
    elif icon_type == "star":
        draw.polygon([(cx, cy-8), (cx+2, cy-2), (cx+8, cy-2), (cx+3, cy+2), (cx+5, cy+8), (cx, cy+4), (cx-5, cy+8), (cx-3, cy+2), (cx-8, cy-2), (cx-2, cy-2)], fill=(255, 215, 0, 255))
    elif icon_type == "blankstar":
        draw.polygon([(cx, cy-8), (cx+2, cy-2), (cx+8, cy-2), (cx+3, cy+2), (cx+5, cy+8), (cx, cy+4), (cx-5, cy+8), (cx-3, cy+2), (cx-8, cy-2), (cx-2, cy-2)], fill=(50, 50, 60, 255), outline=(100, 100, 100, 255))
    
    img.save(os.path.join(SKIN_DIR, filename))

def main():
    print("Generating GlassMP3 Skin...")
    os.makedirs(SKIN_DIR, exist_ok=True)
    
    # 1. Main Background
    create_gradient_bg(480, 272, (20, 15, 30), (5, 5, 10), "bkg_default.png")
    
    # 2. Toolbars
    create_toolbar(480, 20, "toptoolbar.png")
    create_toolbar(480, 20, "bottomtoolbar.png")
    
    # 3. Glass Panels
    panels = {
        "fileinfobkg.png": (380, 100),
        "filespecsbkg.png": (230, 80),
        "helpbkg.png": (460, 250),
        "medialibrarybkg.png": (480, 272),
        "medialibraryinfobkg.png": (470, 50),
        "medialibraryquerybkg.png": (300, 200),
        "medialibraryscanbkg.png": (200, 100),
        "menubkg.png": (480, 232),
        "menuplaylistbkg.png": (300, 200),
        "playerstatusbkg.png": (220, 80),
        "playlistinfobkg.png": (470, 50),
        "popupbkg.png": (250, 120),
        "trackinfobkg.png": (470, 50)
    }
    for name, size in panels.items():
        create_glass_panel(size[0], size[1], name, alpha=90)
        
    create_glass_panel(480, 20, "menuhighlight.png", radius=4, alpha=150)
    
    # 4. Progress Bars
    create_progress_bar(470, 10, "progressbkg.png", (40, 40, 50, 200))
    create_progress_bar(470, 10, "progress.png", (100, 180, 255, 255))
    
    # 5. Buttons
    for i in range(1, 6):
        create_button(80, 24, f"button{i}.png", f"Btn{i}", selected=False)
        create_button(80, 24, f"buttonsel{i}.png", f"Btn{i}", selected=True)
        
    # 6. Icons
    create_icon((24, 24), "folder.png", "folder")
    create_icon((24, 24), "music.png", "music")
    create_icon((16, 16), "star.png", "star")
    create_icon((16, 16), "blankstar.png", "blankstar")
    
    # We leave nocoverart.png, volume, battery, etc. blank or generate simple ones.
    create_glass_panel(100, 100, "nocoverart.png", radius=10, alpha=50)

    print("Skin generation complete!")

if __name__ == "__main__":
    main()
