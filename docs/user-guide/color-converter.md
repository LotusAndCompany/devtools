# Color Converter

Pick a color and instantly view its representation across common color spaces. Adjust channels, copy values, and compare shades side-by-side.

## Overview

The Color Converter is a single-screen tool that lets developers and designers convert a single color between RGB, HEX, HSL, HSV, and CMYK. A visual picker and a live swatch make it easy to fine-tune and copy the exact value needed for CSS, design tools, or print work.

Reference inspiration: [howpon.com/10207](https://howpon.com/10207) (GIMP color exchange walkthrough) — the conversion workflow and dual-color comparison are adapted from that tutorial.

## Accessing the Tool

1. Launch DevTools
2. Click **Color Converter** in the side menu

## Interface Overview

The Color Converter interface consists of:

1. **Color Picker**: Saturation/value square + hue slider + alpha slider
2. **Preview Swatch**: Large block that fills with the current color
3. **Value Fields**: Synchronized inputs for each color space
4. **Channel Sliders**: Adjust individual R/G/B, H/S/L, H/S/V, C/M/Y/K channels
5. **Compare Panel**: "From" and "To" swatches with delta display
6. **History**: Last 10 selected colors as clickable swatches

## Features

### Supported Color Spaces

| Space | Format | Example |
|-------|--------|---------|
| HEX | `#RRGGBB` or `#RRGGBBAA` | `#3B82F6` |
| RGB | `rgb(r, g, b)` / `rgba(r, g, b, a)` | `rgb(59, 130, 246)` |
| HSL | `hsl(h, s%, l%)` / `hsla(...)` | `hsl(217, 91%, 60%)` |
| HSV | `hsv(h, s%, v%)` | `hsv(217, 76%, 96%)` |
| CMYK | `cmyk(c%, m%, y%, k%)` | `cmyk(76%, 47%, 0%, 4%)` |

### Bi-directional Editing

Editing any value field or slider re-computes all other fields. No "source of truth" dropdown — every field is live.

### Color Picker

- Saturation/value square for fine selection
- Hue slider (0–360°)
- Alpha slider (0–100%)
- Click on the preview swatch to open the OS-native color picker

### Copy

Each value field has a **Copy** button that copies the formatted string (including the `#` for HEX or the `rgb(...)` wrapper).

### Compare Mode

Toggle **Compare** to lock the current color as "From" and continue editing to build a "To" color. The panel shows:

- Both swatches side-by-side
- Delta E (CIE76) between them
- Contrast ratio (WCAG 2.1) for accessibility checking

### History

The last 10 colors are shown as small swatches. Click a swatch to restore that color.

### Eyedropper

On supported platforms, an **Eyedropper** button launches the system picker to sample a color from anywhere on screen.

## How to Use

### Convert a HEX to HSL

1. Type `#3B82F6` into the HEX field (or paste)
2. Read the HSL field: `hsl(217, 91%, 60%)`
3. Click **Copy** next to the HSL field

### Adjust a Color by Channel

1. Select a starting color
2. Move the **S** slider to 50%
3. Watch all other fields update live
4. Copy the new HEX value

### Check Contrast

1. Pick a foreground color
2. Toggle **Compare** to lock it as "From"
3. Pick a background color as "To"
4. Read the contrast ratio (e.g. `4.85:1` — AA for normal text)

## Use Cases

- Converting design-tool HEX values to CSS `hsl()` for easier theme tweaking
- Checking WCAG contrast between text and background colors
- Matching print (CMYK) values to screen (RGB) approximations
- Building a quick palette by sampling and comparing nearby shades

## Requirements

### Functional Requirements

- FR-1: Convert between HEX, RGB, HSL, HSV, and CMYK with round-trip accuracy within rendering tolerance
- FR-2: Update every value field when any field, slider, or picker changes (debounced ≤ 50ms)
- FR-3: Validate each field on blur and surface inline errors for unparseable input
- FR-4: Provide one-click copy for every value field
- FR-5: Provide a Compare mode with Delta E (CIE76) and WCAG contrast ratio
- FR-6: Keep a rolling history of the last 10 colors with click-to-restore
- FR-7: Support alpha channel in HEX (`#RRGGBBAA`) and `rgba()` / `hsla()` formats
- FR-8: Persist the current color and history across sessions

### Non-functional Requirements

- NFR-1: Update all fields within one animation frame on slider drag (no perceptible lag)
- NFR-2: Honor the application light/dark theme; the preview swatch remains color-accurate in both
- NFR-3: Display computed values with fixed precision (HEX uppercase, percentages to whole numbers, hue to whole degrees)

## Notes

- CMYK conversion uses the naive formula (no ICC profile). Printed output may differ.
- WCAG contrast is computed on the relative luminance of the non-alpha color values.

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
