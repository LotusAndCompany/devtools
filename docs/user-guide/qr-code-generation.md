# QR Code Generation

Generate QR codes for various types of content including text, URLs, emails, phone numbers, SMS messages, and geographic locations.

## Overview

The QR Code Generator creates scannable QR codes that can encode different types of information. When scanned with a smartphone, the QR code performs the appropriate action based on its content type.

## Accessing the Tool

1. Launch DevTools
2. Click **QR Code** in the side menu

## QR Code Types

### Text

Generate QR codes containing plain text.

**How to use:**
1. Select the **Text** tab
2. Enter your text in the input field
3. Click **Generate**

**Use cases:**
- Share plain text messages
- Encode serial numbers or IDs
- Store short notes

### URL

Create QR codes that open websites when scanned.

**How to use:**
1. Select the **URL** tab
2. Enter the full URL (including `https://`)
3. Click **Generate**

**Example:**
```
https://github.com/LotusAndCompany/devtools
```

**Tip:** Always include the protocol (`https://` or `http://`) for best compatibility.

### Email

Generate QR codes that compose emails when scanned.

**How to use:**
1. Select the **Email** tab
2. Enter the recipient email address
3. Optionally add subject and body
4. Click **Generate**

**Generated format:**
```
mailto:email@example.com?subject=Subject&body=Body
```

**Fields:**
| Field | Required | Description |
|-------|----------|-------------|
| To | Yes | Recipient email address |
| Subject | No | Email subject line |
| Body | No | Email body text |

### Phone

Create QR codes that initiate phone calls when scanned.

**How to use:**
1. Select the **Phone** tab
2. Enter the phone number
3. Click **Generate**

**Generated format:**
```
tel:+1234567890
```

**Tips:**
- Include country code for international numbers
- Use digits only (no spaces or dashes)

### SMS

Generate QR codes that compose SMS messages when scanned.

**How to use:**
1. Select the **SMS** tab
2. Enter the phone number
3. Optionally add a message body
4. Click **Generate**

**Generated format:**
```
sms:+1234567890?body=Your message here
```

**Fields:**
| Field | Required | Description |
|-------|----------|-------------|
| Phone Number | Yes | Recipient phone number |
| Message | No | Pre-filled SMS text |

### Geo (Location)

Create QR codes that open map applications with a specific location.

**How to use:**
1. Select the **Geo** tab
2. Enter latitude and longitude
3. Click **Generate**

**Generated format:**
```
geo:35.6762,139.6503
```

**Coordinate format:**
- Latitude: -90 to 90 (negative = South)
- Longitude: -180 to 180 (negative = West)

**Example locations:**
| Location | Coordinates |
|----------|-------------|
| Tokyo Tower | 35.6586, 139.7454 |
| Eiffel Tower | 48.8584, 2.2945 |
| Statue of Liberty | 40.6892, -74.0445 |

## Saving QR Codes

After generating a QR code:

1. **Right-click** on the QR code image
2. Select **Save Image**
3. Choose a location and filename
4. The image is saved as PNG

**Alternative:**
1. Click the **Save** button (if available)
2. Choose save location

## QR Code Size and Quality

- Generated QR codes are high-resolution
- Default size is suitable for most uses
- For printing, use higher resolution settings if available

## Best Practices

### Content Length
- Keep content as short as possible
- Longer content = more complex QR code = harder to scan

### Testing
- Always test generated QR codes with a smartphone
- Test on multiple devices if possible

### Placement
- Ensure adequate contrast when printing
- Dark QR code on light background works best
- Avoid very small sizes (minimum 2cm x 2cm recommended)

## Troubleshooting

### QR Code Won't Scan

1. **Check size**: QR code might be too small
2. **Check contrast**: Ensure dark/light contrast
3. **Check content**: Very long content may create unreadable codes
4. **Clean camera**: Ensure scanner camera is clean

### Wrong Action When Scanned

1. **Verify format**: Check that the correct tab was selected
2. **Check content**: Ensure no typos in URLs, emails, etc.
3. **Regenerate**: Create a new QR code

## Technical Details

DevTools uses the [QR Code Generator](https://www.nayuki.io/page/qr-code-generator-library) library by Project Nayuki, which supports:

- QR Code versions 1-40
- Error correction levels: L, M, Q, H
- All standard encodings

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
