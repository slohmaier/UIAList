# PowerShell script to generate Microsoft Store assets from SVG
# Requires ImageMagick or Inkscape to be installed

param(
    [string]$SvgPath = "uialist_icon.svg",
    [string]$OutputDir = "Assets"
)

# Ensure output directory exists
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force
}

# Define required asset sizes and names for Microsoft Store
$assets = @(
    @{ Name = "Square44x44Logo.png"; Size = 44 },
    @{ Name = "Square71x71Logo.png"; Size = 71 },    # SmallTile
    @{ Name = "Square150x150Logo.png"; Size = 150 },
    @{ Name = "Square310x310Logo.png"; Size = 310 },  # LargeTile
    @{ Name = "Wide310x150Logo.png"; Width = 310; Height = 150 },
    @{ Name = "StoreLogo.png"; Size = 50 },
    @{ Name = "SplashScreen.png"; Width = 620; Height = 300 },
    
    # Additional sizes for better quality
    @{ Name = "Square44x44Logo.targetsize-16.png"; Size = 16 },
    @{ Name = "Square44x44Logo.targetsize-24.png"; Size = 24 },
    @{ Name = "Square44x44Logo.targetsize-32.png"; Size = 32 },
    @{ Name = "Square44x44Logo.targetsize-48.png"; Size = 48 },
    @{ Name = "Square44x44Logo.targetsize-256.png"; Size = 256 }
)

# Function to check if ImageMagick is available
function Test-ImageMagick {
    try {
        & magick -version 2>$null
        return $true
    } catch {
        return $false
    }
}

# Function to check if Inkscape is available
function Test-Inkscape {
    try {
        & inkscape --version 2>$null
        return $true
    } catch {
        return $false
    }
}

Write-Host "Generating Microsoft Store assets from $SvgPath..." -ForegroundColor Green

if (Test-ImageMagick) {
    Write-Host "Using ImageMagick to generate assets..." -ForegroundColor Yellow
    
    foreach ($asset in $assets) {
        $outputPath = Join-Path $OutputDir $asset.Name
        
        if ($asset.ContainsKey("Size")) {
            # Square image
            $size = $asset.Size
            & magick convert $SvgPath -background transparent -resize "${size}x${size}" $outputPath
        } else {
            # Rectangular image
            $width = $asset.Width
            $height = $asset.Height
            & magick convert $SvgPath -background transparent -resize "${width}x${height}" $outputPath
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Generated: $($asset.Name)" -ForegroundColor Cyan
        } else {
            Write-Host "Failed to generate: $($asset.Name)" -ForegroundColor Red
        }
    }
} elseif (Test-Inkscape) {
    Write-Host "Using Inkscape to generate assets..." -ForegroundColor Yellow
    
    foreach ($asset in $assets) {
        $outputPath = Join-Path $OutputDir $asset.Name
        
        if ($asset.ContainsKey("Size")) {
            # Square image
            $size = $asset.Size
            & inkscape --export-type=png --export-filename=$outputPath --export-width=$size --export-height=$size $SvgPath
        } else {
            # Rectangular image
            $width = $asset.Width
            $height = $asset.Height
            & inkscape --export-type=png --export-filename=$outputPath --export-width=$width --export-height=$height $SvgPath
        }
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Generated: $($asset.Name)" -ForegroundColor Cyan
        } else {
            Write-Host "Failed to generate: $($asset.Name)" -ForegroundColor Red
        }
    }
} else {
    Write-Host "Neither ImageMagick nor Inkscape found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install one of the following:" -ForegroundColor Yellow
    Write-Host "1. ImageMagick: https://imagemagick.org/script/download.php#windows" -ForegroundColor White
    Write-Host "2. Inkscape: https://inkscape.org/release/" -ForegroundColor White
    Write-Host ""
    Write-Host "After installation, run this script again to generate the required assets." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Required asset files:" -ForegroundColor Green
    foreach ($asset in $assets) {
        if ($asset.ContainsKey("Size")) {
            Write-Host "  $($asset.Name) - ${$asset.Size}x$($asset.Size)px" -ForegroundColor White
        } else {
            Write-Host "  $($asset.Name) - $($asset.Width)x$($asset.Height)px" -ForegroundColor White
        }
    }
    exit 1
}

Write-Host ""
Write-Host "Asset generation complete!" -ForegroundColor Green
Write-Host "All files have been saved to the '$OutputDir' directory." -ForegroundColor Cyan