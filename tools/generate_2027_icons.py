"""
Omor Ekushe - 2027 Icon Design Generator
Generates ultra-modern Fluent 2.0 / 2027 multi-resolution Windows .ico files
for Omor Ekushe:
  1. Bijoy.ico    - Main Application Icon (Emerald/Crimson/Obsidian with 'অ')
  2. Phonetic.ico - Avro Phonetic Engine Icon (Electric Cyan/Royal Blue with 'ফ')
  3. Unicode.ico  - Unicode Layout Icon (Emerald/Mint Green with 'ইউ')
  4. Classic.ico  - Classic ANSI Layout Icon (Crimson/Ruby Red with 'ক')

Multi-resolution support:
  256x256, 128x128, 64x64, 48x48, 32x32, 24x24, 16x16
"""

import os
import math
from PIL import Image, ImageDraw, ImageFont, ImageFilter

SIZES = [(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (24, 24), (16, 16)]
SUPER_SIZE = 1024

def create_rounded_rectangle_mask(size, radius):
    mask = Image.new('L', size, 0)
    draw = ImageDraw.Draw(mask)
    draw.rounded_rectangle([(0, 0), (size[0]-1, size[1]-1)], radius=radius, fill=255)
    return mask

def create_linear_gradient(size, start_color, end_color, angle=45):
    """Generates a smooth linear gradient at a specified angle."""
    width, height = size
    grad = Image.new('RGBA', size)
    draw = ImageDraw.Draw(grad)
    rad = math.radians(angle)
    cos_a, sin_a = math.cos(rad), math.sin(rad)
    max_dist = math.sqrt(width**2 + height**2)

    for y in range(height):
        for x in range(width):
            dist = (x * cos_a + y * sin_a)
            t = max(0.0, min(1.0, dist / max_dist))
            r = int(start_color[0] + (end_color[0] - start_color[0]) * t)
            g = int(start_color[1] + (end_color[1] - start_color[1]) * t)
            b = int(start_color[2] + (end_color[2] - start_color[2]) * t)
            a = int(start_color[3] + (end_color[3] - start_color[3]) * t) if len(start_color) > 3 else 255
            grad.putpixel((x, y), (r, g, b, a))
    return grad

def create_radial_gradient_disk(size, center, radius, inner_color, outer_color):
    """Draws a smooth radial gradient disk."""
    disk = Image.new('RGBA', size, (0, 0, 0, 0))
    for y in range(size[1]):
        for x in range(size[0]):
            dx = x - center[0]
            dy = y - center[1]
            d = math.hypot(dx, dy)
            if d <= radius:
                t = d / radius
                r = int(inner_color[0] + (outer_color[0] - inner_color[0]) * t)
                g = int(inner_color[1] + (outer_color[1] - inner_color[1]) * t)
                b = int(inner_color[2] + (outer_color[2] - inner_color[2]) * t)
                a = int(inner_color[3] + (outer_color[3] - inner_color[3]) * t)
                disk.putpixel((x, y), (r, g, b, a))
    return disk

def draw_squircle_badge(bg_gradient, radius_pct=0.24, border_color=(255, 255, 255, 70), shadow=True):
    """Creates a modern floating squircle container with drop-shadow and glass edge."""
    W, H = SUPER_SIZE, SUPER_SIZE
    canvas = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    
    # Margin for drop shadow
    margin = 48
    box = [margin, margin, W - margin, H - margin]
    bw = box[2] - box[0]
    bh = box[3] - box[1]
    radius = int(bw * radius_pct)

    # 1. Ambient drop shadow
    if shadow:
        shadow_img = Image.new('RGBA', (W, H), (0, 0, 0, 0))
        sdraw = ImageDraw.Draw(shadow_img)
        shadow_box = [margin, margin + 20, W - margin, H - margin + 20]
        sdraw.rounded_rectangle(shadow_box, radius=radius, fill=(0, 0, 0, 110))
        shadow_img = shadow_img.filter(ImageFilter.GaussianBlur(32))
        canvas.paste(shadow_img, (0, 0), shadow_img)

    # 2. Base Squircle with Gradient
    squircle = Image.new('RGBA', (bw, bh), (0, 0, 0, 0))
    mask = create_rounded_rectangle_mask((bw, bh), radius)
    squircle.paste(bg_gradient.resize((bw, bh)), (0, 0), mask)

    # 3. Glass specular highlight (top-half subtle shine)
    highlight = Image.new('RGBA', (bw, bh), (0, 0, 0, 0))
    hdraw = ImageDraw.Draw(highlight)
    hdraw.rounded_rectangle([0, 0, bw, int(bh * 0.48)], radius=radius, fill=(255, 255, 255, 30))
    highlight = highlight.filter(ImageFilter.GaussianBlur(16))
    squircle.paste(highlight, (0, 0), mask)

    # 4. Ultra-thin 2027 acrylic border rim
    border = Image.new('RGBA', (bw, bh), (0, 0, 0, 0))
    bdraw = ImageDraw.Draw(border)
    bdraw.rounded_rectangle([1, 1, bw - 2, bh - 2], radius=radius, outline=border_color, width=4)
    squircle.paste(border, (0, 0), border)

    canvas.paste(squircle, (margin, margin), squircle)
    return canvas, (margin, margin, bw, bh)

def generate_bijoy_icon():
    """Omor Ekushe Master Application Icon (Bijoy.ico): Obsidian/Emerald with Crimson Sun & 'অ'"""
    # Background: Deep Obsidian to Cosmic Slate
    bg_grad = create_linear_gradient((SUPER_SIZE, SUPER_SIZE), (15, 23, 42), (30, 41, 59), angle=135)
    canvas, (ox, oy, bw, bh) = draw_squircle_badge(bg_grad, radius_pct=0.25, border_color=(16, 185, 129, 90))

    cx = ox + bw // 2
    cy = oy + bh // 2

    # Glowing Crimson-Ruby circular disk (the rising sun of Language Movement & flag)
    sun_radius = int(bw * 0.38)
    sun = Image.new('RGBA', (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    sdraw = ImageDraw.Draw(sun)
    
    # Outer glow
    glow_box = [cx - sun_radius - 20, cy - sun_radius - 20, cx + sun_radius + 20, cy + sun_radius + 20]
    sdraw.ellipse(glow_box, fill=(225, 29, 72, 80))
    sun = sun.filter(ImageFilter.GaussianBlur(24))
    canvas.paste(sun, (0, 0), sun)

    # Crisp Crimson disk with subtle gradient
    sun_disk = create_radial_gradient_disk(
        (SUPER_SIZE, SUPER_SIZE),
        (cx - 20, cy - 30),
        sun_radius,
        (244, 63, 94, 255),  # Rose 500
        (190, 18, 60, 255)   # Rose 700
    )
    sun_mask = Image.new('L', (SUPER_SIZE, SUPER_SIZE), 0)
    ImageDraw.Draw(sun_mask).ellipse([cx - sun_radius, cy - sun_radius, cx + sun_radius, cy + sun_radius], fill=255)
    canvas.paste(sun_disk, (0, 0), sun_mask)

    # Disk subtle inner border
    ImageDraw.Draw(canvas).ellipse(
        [cx - sun_radius, cy - sun_radius, cx + sun_radius, cy + sun_radius],
        outline=(255, 255, 255, 75),
        width=3
    )

    # Emerald crescent/accent arc at the bottom representing freedom & Bangladesh green
    arc_mask = Image.new('RGBA', (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    adraw = ImageDraw.Draw(arc_mask)
    adraw.arc([cx - sun_radius - 6, cy - sun_radius - 6, cx + sun_radius + 6, cy + sun_radius + 6],
              start=30, end=150, fill=(16, 185, 129, 230), width=10)
    canvas.paste(arc_mask, (0, 0), arc_mask)

    # Hero Glyph: 'অ' in NirmalaB.ttf
    font = ImageFont.truetype(r'C:\Windows\Fonts\NirmalaB.ttf', int(sun_radius * 1.40))
    
    # Text shadow
    tdraw = ImageDraw.Draw(canvas)
    tdraw.text((cx + 4, cy + 8), 'অ', font=font, fill=(0, 0, 0, 160), anchor='mm')
    # Text main glyph
    tdraw.text((cx, cy), 'অ', font=font, fill=(255, 255, 255, 255), anchor='mm')

    return canvas

def generate_phonetic_icon():
    """Avro Phonetic Mode Icon (Phonetic.ico): Electric Cyan to Royal Blue with 'ফ'"""
    # Background: Electric Cyan to Deep Cobalt Blue
    bg_grad = create_linear_gradient((SUPER_SIZE, SUPER_SIZE), (6, 182, 212), (30, 64, 175), angle=135)
    canvas, (ox, oy, bw, bh) = draw_squircle_badge(bg_grad, radius_pct=0.25, border_color=(255, 255, 255, 95))

    cx = ox + bw // 2
    cy = oy + bh // 2

    # Modern soundwave / transliteration audio arcs on the sides
    wave_img = Image.new('RGBA', (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    wdraw = ImageDraw.Draw(wave_img)
    
    # Left sound arcs
    wdraw.arc([cx - 360, cy - 180, cx - 180, cy + 180], start=120, end=240, fill=(255, 255, 255, 80), width=12)
    wdraw.arc([cx - 400, cy - 230, cx - 140, cy + 230], start=130, end=230, fill=(255, 255, 255, 45), width=10)
    # Right sound arcs
    wdraw.arc([cx + 180, cy - 180, cx + 360, cy + 180], start=300, end=60, fill=(255, 255, 255, 80), width=12)
    wdraw.arc([cx + 140, cy - 230, cx + 400, cy + 230], start=310, end=50, fill=(255, 255, 255, 45), width=10)
    canvas.paste(wave_img, (0, 0), wave_img)

    # Hero Glyph: 'ফ' (Phonetic) in crisp bold white
    font = ImageFont.truetype(r'C:\Windows\Fonts\NirmalaB.ttf', int(bw * 0.65))
    tdraw = ImageDraw.Draw(canvas)
    # Deep drop shadow
    tdraw.text((cx + 4, cy + 10), 'ফ', font=font, fill=(15, 23, 42, 170), anchor='mm')
    # Brilliant white glyph
    tdraw.text((cx, cy), 'ফ', font=font, fill=(255, 255, 255, 255), anchor='mm')

    return canvas

def generate_unicode_icon():
    """Unicode Layout Mode Icon (Unicode.ico): Emerald to Deep Forest Mint with 'ইউ'"""
    # Background: Mint / Emerald to Deep Forest Teal
    bg_grad = create_linear_gradient((SUPER_SIZE, SUPER_SIZE), (16, 185, 129), (6, 95, 70), angle=135)
    canvas, (ox, oy, bw, bh) = draw_squircle_badge(bg_grad, radius_pct=0.25, border_color=(255, 255, 255, 95))

    cx = ox + bw // 2
    cy = oy + bh // 2

    # Subtle circular Unicode globe grid lines
    globe = Image.new('RGBA', (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    gdraw = ImageDraw.Draw(globe)
    r = int(bw * 0.42)
    gdraw.ellipse([cx - r, cy - r, cx + r, cy + r], outline=(255, 255, 255, 40), width=4)
    gdraw.ellipse([cx - int(r*0.55), cy - r, cx + int(r*0.55), cy + r], outline=(255, 255, 255, 30), width=3)
    gdraw.line([(cx - r, cy), (cx + r, cy)], fill=(255, 255, 255, 35), width=3)
    canvas.paste(globe, (0, 0), globe)

    # Hero Glyph: 'ইউ' in crisp bold white
    font = ImageFont.truetype(r'C:\Windows\Fonts\NirmalaB.ttf', int(bw * 0.54))
    tdraw = ImageDraw.Draw(canvas)
    # Deep drop shadow
    tdraw.text((cx + 4, cy + 10), 'ইউ', font=font, fill=(6, 78, 59, 180), anchor='mm')
    # Brilliant white glyph
    tdraw.text((cx, cy), 'ইউ', font=font, fill=(255, 255, 255, 255), anchor='mm')

    return canvas

def generate_classic_icon():
    """Classic ANSI Mode Icon (Classic.ico): Ruby Crimson to Warm Coral with 'ক'"""
    # Background: Ruby Crimson to Deep Velvet Wine
    bg_grad = create_linear_gradient((SUPER_SIZE, SUPER_SIZE), (244, 63, 94), (159, 18, 57), angle=135)
    canvas, (ox, oy, bw, bh) = draw_squircle_badge(bg_grad, radius_pct=0.25, border_color=(255, 255, 255, 95))

    cx = ox + bw // 2
    cy = oy + bh // 2

    # Modern dynamic corner accent rays
    accents = Image.new('RGBA', (SUPER_SIZE, SUPER_SIZE), (0, 0, 0, 0))
    adraw = ImageDraw.Draw(accents)
    r = int(bw * 0.40)
    adraw.arc([cx - r, cy - r, cx + r, cy + r], start=210, end=330, fill=(255, 255, 255, 55), width=6)
    adraw.arc([cx - r, cy - r, cx + r, cy + r], start=30, end=150, fill=(255, 255, 255, 55), width=6)
    canvas.paste(accents, (0, 0), accents)

    # Hero Glyph: 'ক' in crisp bold white
    font = ImageFont.truetype(r'C:\Windows\Fonts\NirmalaB.ttf', int(bw * 0.65))
    tdraw = ImageDraw.Draw(canvas)
    # Deep drop shadow
    tdraw.text((cx + 4, cy + 10), 'ক', font=font, fill=(136, 19, 55, 180), anchor='mm')
    # Brilliant white glyph
    tdraw.text((cx, cy), 'ক', font=font, fill=(255, 255, 255, 255), anchor='mm')

    return canvas

def save_multisize_ico(master_img, output_path):
    """Generates and packages all standard Windows .ico resolutions using Lanczos anti-aliasing."""
    frames = []
    for w, h in SIZES:
        resized = master_img.resize((w, h), Image.Resampling.LANCZOS)
        frames.append(resized)
    
    # Save as multi-resolution ICO file
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    master_img.save(output_path, format='ICO', sizes=SIZES)
    print(f"[OK] Saved 2027 icon: {output_path} ({len(SIZES)} frames: 256px down to 16px)")

def main():
    icons = {
        'data/Icons/Bijoy.ico': generate_bijoy_icon(),
        'data/Icons/Phonetic.ico': generate_phonetic_icon(),
        'data/Icons/Unicode.ico': generate_unicode_icon(),
        'data/Icons/Classic.ico': generate_classic_icon(),
    }

    for path, img in icons.items():
        save_multisize_ico(img, path)

    # Also update app.ico (main application icon for Win32 resource script)
    app_ico_path = 'src/platform/windows/app.ico'
    save_multisize_ico(icons['data/Icons/Bijoy.ico'], app_ico_path)

if __name__ == '__main__':
    main()

