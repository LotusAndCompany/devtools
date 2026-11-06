# Image Processing

Use Image Editor to resize, rotate, flip, make a color transparent, or split an
image.

## Open and save an image

1. Select **Image Editor** from the sidebar.
2. Click **Open image** or drop an image into the application window.
3. Review the image in the preview.
4. Click **Save image** to choose a destination for the current result.
5. Click **Reset** to restore the original image.

The editor works on one image at a time.

## Resize

The Resize card supports target dimensions and independent horizontal and
vertical scale percentages.

1. Enter the target **Width (px)** and **Height (px)**, or set the scale
   percentages.
2. Select **Keep aspect ratio** to preserve the image proportions.
3. Select **Smooth scaling** if you want smoother resampling.
4. Click **Apply size** or **Apply scale**.

## Rotate and flip

The Rotate and flip card provides **Rotate left**, **Rotate right**,
**Flip horizontal**, and **Flip vertical**. The rotation controls turn the
image in 90-degree steps.

## Make a color transparent

1. Set **Color mode** to RGB, HSL, or HSV.
2. Set **Tolerance (0–1)** and **Transparency (0–1)**.
3. Select **Only connected pixels** to limit the change to the connected area.
4. Click the pixel to change in the image preview.

The change is applied when you click the preview.

## Split an image

1. Select **Number of parts** or **Cell size** in **Split by**.
2. For Number of parts, set **Columns** and **Rows**. For Cell size, set
   **Cell width** and **Cell height**.
3. Select **Ignore remainder pixels** if the split should omit pixels that do
   not fill a complete part.
4. Click **Choose folder and split**, then choose an output folder.

## Supported formats

The **Open image** dialog filters PNG, JPEG, BMP, GIF, and WebP files. The
application also accepts image formats supported by the installed Qt image
plugins. The save dialog uses the format indicated by the filename extension;
PNG preserves transparency, while JPEG does not.

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
