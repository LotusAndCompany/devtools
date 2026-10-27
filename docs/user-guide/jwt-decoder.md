# JWT Decoder

Decode a JSON Web Token and inspect its header, payload, and signature.

## Overview

The JWT Decoder splits a token into its three parts, Base64URL-decodes the header and payload, and displays them as formatted JSON. The signature is shown as-is.

**This tool does not verify signatures.** It only decodes, exactly like the decoder on jwt.io. A token that decodes successfully is not proof that it is authentic.

Decoding runs entirely on your machine. Nothing is sent over the network, so it is safe to paste tokens that contain sensitive claims.

## Accessing the Tool

1. Launch DevTools
2. Click **JWT Decoder** in the side menu

## Interface Overview

The JWT Decoder interface consists of:

1. **Notice banner**: a permanent reminder that signatures are not verified
2. **Encoded JWT**: the input field for the token
3. **Header pane**: the decoded header as formatted JSON
4. **Payload pane**: the decoded payload as formatted JSON
5. **Signature pane**: the signature as a raw Base64URL string

The three panes sit in a splitter, so you can drag the dividers to give a pane more room.

## Features

### Automatic Decoding

- The result updates as you type, debounced by ~180 ms
- No button to press

### Color-Coded Segments

The input field colors each segment so you can see the token structure at a glance:

| Segment | Color |
|---------|-------|
| Header | Crimson |
| Payload | Purple |
| Signature | Blue |

The pane titles use the same colors. Inside the header and payload panes, JSON keys, strings, and numbers are colored as well.

### Independent Parts

Each part is decoded on its own. If the payload is malformed, the header and signature are still shown.

## How to Use

### Basic Usage

1. Paste a token in the form `header.payload.signature` into **Encoded JWT**
2. Read the decoded header and payload below

### Two-Part Tokens

A token without a signature (`header.payload`) is accepted. The signature pane shows **No data**.

## Error Handling

| Situation | What you see |
|-----------|--------------|
| Fewer than two parts, or more than three | An **Invalid JWT structure** banner below the input; all three panes show **No data** |
| A segment is not valid Base64URL | That pane shows **Decode failed** with **Failed to Base64URL-decode**; the segment turns red in the input |
| A segment decodes but is not JSON | That pane shows **Decode failed** with **Invalid JSON** |

Base64URL padding is optional, as required by the JWT specification.

## Use Cases

- Checking which claims an API returned in an access token
- Confirming the `alg` and `typ` of a token during integration work
- Reading `sub`, `iss`, or custom claims while debugging authentication

## Limitations

This is a decoder only. The following are out of scope:

- Signature verification (HS/RS/ES algorithms)
- Converting `exp` / `iat` / `nbf` to dates, or judging whether a token has expired
- Creating or encoding tokens
- Encrypted tokens (JWE); only JWS tokens with two or three parts are supported
- History, persistence, and copy buttons
