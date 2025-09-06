# Microsoft Store Assets

This directory contains the icon assets required for MSIX packaging and Microsoft Store submission.

## Required Assets

The following icon files are required for the Microsoft Store:

### Application Icons
- `Square44x44Logo.png` - 44x44px - App list icon
- `Square71x71Logo.png` - 71x71px - Small tile
- `Square150x150Logo.png` - 150x150px - Medium tile (default)
- `Square310x310Logo.png` - 310x310px - Large tile
- `Wide310x150Logo.png` - 310x150px - Wide tile
- `StoreLogo.png` - 50x50px - Store listing

### Additional Sizes (Optional but recommended)
- `Square44x44Logo.targetsize-16.png` - 16x16px
- `Square44x44Logo.targetsize-24.png` - 24x24px  
- `Square44x44Logo.targetsize-32.png` - 32x32px
- `Square44x44Logo.targetsize-48.png` - 48x48px
- `Square44x44Logo.targetsize-256.png` - 256x256px

### Splash Screen
- `SplashScreen.png` - 620x300px - App launch screen

## Generation

To generate all required assets from the SVG icon, run:

```powershell
.\generate_store_assets.ps1
```

This script requires either ImageMagick or Inkscape to be installed.

## Manual Creation

If you prefer to create the icons manually:

1. Use `base_icon.png` as a starting point
2. Create each required size using your preferred image editor
3. Ensure all backgrounds are transparent
4. Save as PNG format with the exact filenames listed above

## Guidelines

- All icons should have transparent backgrounds
- Maintain consistent visual style across all sizes
- Follow Microsoft Store icon guidelines
- Ensure icons are crisp and clear at all sizes