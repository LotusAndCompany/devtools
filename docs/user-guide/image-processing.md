# Image Processing

DevTools provides several image processing tools including resize, rotation, division (splitting), and transparency features.

## Overview

The Image Processing module allows you to perform common image manipulations without needing external software or web services.

## Accessing the Tools

1. Launch DevTools
2. Click **Image** in the side menu
3. Select the desired tool tab: Resize, Rotate, Split, or Transparency

## Supported Formats

### Input Formats
- PNG
- JPEG / JPG
- BMP
- GIF
- TIFF

### Output Formats
- PNG (recommended for transparency)
- JPEG (for photos without transparency)

## Resize

Scale images to specific dimensions.

### How to Use

1. Select the **Resize** tab
2. Click **Open Image** to load an image
3. Set the target dimensions:
   - **Width**: Target width in pixels
   - **Height**: Target height in pixels
4. Optionally check **Keep Aspect Ratio**
5. Click **Resize**
6. Save the result

### Options

| Option | Description |
|--------|-------------|
| Keep Aspect Ratio | Maintains proportions; only one dimension needs to be set |
| Width | Target width in pixels |
| Height | Target height in pixels |

### Tips

- Use **Keep Aspect Ratio** to prevent image distortion
- Enter only width or height when maintaining ratio; the other will calculate automatically

## Rotate

Rotate images by any angle.

### How to Use

1. Select the **Rotate** tab
2. Click **Open Image** to load an image
3. Enter the rotation angle in degrees
4. Select rotation direction (clockwise/counter-clockwise)
5. Click **Rotate**
6. Save the result

### Options

| Option | Description |
|--------|-------------|
| Angle | Rotation angle in degrees (0-360) |
| Direction | Clockwise or counter-clockwise |

### Quick Rotations

Common rotation buttons may be available:
- **90° CW**: Rotate 90 degrees clockwise
- **90° CCW**: Rotate 90 degrees counter-clockwise
- **180°**: Flip upside down

### Tips

- Rotating by angles other than 90/180/270 may result in empty corners
- The canvas may expand to fit the rotated image

## Split (Division)

Split an image into multiple equal parts.

### How to Use

1. Select the **Split** tab
2. Click **Open Image** to load an image
3. Set the grid dimensions:
   - **Columns**: Number of horizontal divisions
   - **Rows**: Number of vertical divisions
4. Click **Split**
5. Save the resulting parts

### Options

| Option | Description |
|--------|-------------|
| Columns | Number of horizontal parts (1-10) |
| Rows | Number of vertical parts (1-10) |

### Output

- Multiple images are generated
- Files are numbered sequentially (e.g., image_1.png, image_2.png, ...)
- Order: left-to-right, top-to-bottom

### Use Cases

- Creating sprite sheets
- Breaking large images into tiles
- Social media grid posts (3x3 for Instagram)

## Transparency

Make specific colors transparent in an image.

### How to Use

1. Select the **Transparency** tab
2. Click **Open Image** to load an image
3. Select the color to make transparent:
   - Use the **color picker** to select from the image
   - Or enter a specific color value
4. Adjust **tolerance** for similar colors
5. Click **Apply**
6. Save as PNG (required for transparency)

### Options

| Option | Description |
|--------|-------------|
| Target Color | The color to make transparent |
| Tolerance | How similar colors must be (0 = exact match, higher = more colors) |

### Tips

- Always save as PNG to preserve transparency
- JPEG does not support transparency
- Lower tolerance for precise color matching
- Higher tolerance for gradient edges

### Common Use Cases

- Remove white backgrounds
- Remove solid color backgrounds
- Create transparent logos

## Batch Processing

Currently, DevTools processes one image at a time. For batch processing:

1. Process images one by one
2. Use consistent settings across images

## Image Quality

### Resolution
- DevTools preserves original resolution where possible
- Upscaling may reduce quality

### Compression
- PNG: Lossless compression
- JPEG: Lossy compression (smaller files, some quality loss)

## Troubleshooting

### Image Not Loading

1. **Check format**: Ensure file is a supported format
2. **Check file**: File may be corrupted
3. **Check permissions**: File may be read-protected

### Quality Loss After Processing

1. **Use PNG**: PNG is lossless; JPEG loses quality
2. **Avoid multiple saves**: Each JPEG save reduces quality
3. **Work from originals**: Keep original files

### Transparency Not Working

1. **Save as PNG**: JPEG doesn't support transparency
2. **Check tolerance**: May be too low to capture color variations
3. **Pick exact color**: Use color picker on the image

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
