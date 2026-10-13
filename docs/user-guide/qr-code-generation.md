# QR Code Generation

Generate QR codes for various types of content including text, URLs, emails, phone numbers, SMS messages, and geographic locations.

## Overview

The QR Code Generator creates scannable QR codes that can encode different types of information. When scanned with a smartphone, the QR code performs the appropriate action based on its content type.

## Accessing the Tool

1. Launch DevTools
2. Click **QR Code Generation** in the side menu

## QR Code Types

### Text

Generate QR codes containing plain text.

**How to use:**
1. Select **Text** from the **QR Code Type** selector
2. Enter your text in the input field
3. Click **Generate**

**Use cases:**
- Share plain text messages
- Encode serial numbers or IDs
- Store short notes

### URL

Create QR codes that open websites when scanned.

**How to use:**
1. Select **URL** from the **QR Code Type** selector
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
1. Select **Email** from the **QR Code Type** selector
2. Enter the recipient email address
3. Click **Generate**

**Generated format:**
```
mailto:email@example.com
```

**Fields:**
| Field | Required | Description |
|-------|----------|-------------|
| Email | Yes | Recipient email address |

### Phone

Create QR codes that initiate phone calls when scanned.

**How to use:**
1. Select **Phone** from the **QR Code Type** selector
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
1. Select **SMS** from the **QR Code Type** selector
2. Enter the phone number
3. Enter the message body
4. Click **Generate**

**Generated format:**
```
SMSTO:+1234567890:Your message here
```

**Fields:**
| Field | Required | Description |
|-------|----------|-------------|
| Phone Number | Yes | Recipient phone number |
| Message | Yes | Pre-filled SMS text |

### Wi-Fi

Create a QR code containing a Wi-Fi network name, password, and security
type. Select **WiFi**, enter the SSID and password, choose **WPA**, **WEP**,
or **nopass**, and click **Generate**.

### Contact

Create a vCard QR code. Select **Contact**, enter a name, and enter at least a
phone number or an email address before clicking **Generate**.

### Calendar

Create a calendar event QR code. Select **Calendar**, enter an event title and
start and end times, then click **Generate**. The end time must be later than
the start time.

### Geo (Location)

Create QR codes that open map applications with a specific location.

**How to use:**
1. Select **Geo Location** from the **QR Code Type** selector
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

1. Click **Save Image** below the QR code
2. Choose a location and filename
3. The image is saved as PNG

Use **Copy Image** to copy the generated QR code to the clipboard.

## QR Code Size and Quality

- Generated QR codes use a fixed scale suitable for normal use
- The tool does not provide a separate size or quality control

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
