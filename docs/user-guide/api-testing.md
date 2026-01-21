# API Testing

Send HTTP requests and view responses directly from DevTools.

## Overview

The API Testing tool allows developers to quickly test REST APIs, debug endpoints, and inspect responses without needing separate tools like Postman or curl.

## Accessing the Tool

1. Launch DevTools
2. Click **API** in the side menu

## Interface Overview

The API Testing interface consists of:

1. **URL Bar**: Enter the endpoint URL
2. **Method Selector**: Choose HTTP method
3. **Headers Section**: Add request headers
4. **Body Section**: Add request body (for POST, PUT, PATCH)
5. **Response Area**: View response data

## Supported HTTP Methods

| Method | Description | Has Body |
|--------|-------------|----------|
| GET | Retrieve data | No |
| POST | Create new resource | Yes |
| PUT | Update/replace resource | Yes |
| PATCH | Partial update | Yes |
| DELETE | Remove resource | Optional |
| HEAD | Get headers only | No |
| OPTIONS | Get supported methods | No |

## Making Requests

### Basic GET Request

1. Enter the URL: `https://api.example.com/users`
2. Select **GET** method
3. Click **Send**
4. View the response

### POST Request with JSON Body

1. Enter the URL: `https://api.example.com/users`
2. Select **POST** method
3. Add header:
   - Key: `Content-Type`
   - Value: `application/json`
4. Add body:
   ```json
   {
     "name": "John Doe",
     "email": "john@example.com"
   }
   ```
5. Click **Send**

### Request with Authentication

#### Bearer Token

Add header:
- Key: `Authorization`
- Value: `Bearer your-token-here`

#### Basic Auth

Add header:
- Key: `Authorization`
- Value: `Basic base64-encoded-credentials`

#### API Key

Add header (varies by API):
- Key: `X-API-Key`
- Value: `your-api-key`

## Headers

### Adding Headers

1. Click **Add Header**
2. Enter key and value
3. Repeat for additional headers

### Common Headers

| Header | Purpose | Example |
|--------|---------|---------|
| Content-Type | Request body format | `application/json` |
| Accept | Expected response format | `application/json` |
| Authorization | Authentication | `Bearer token123` |
| User-Agent | Client identification | `DevTools/0.2.0` |

### Removing Headers

Click the **X** button next to the header to remove it.

## Request Body

### JSON Body

```json
{
  "key": "value",
  "nested": {
    "item": "data"
  }
}
```

### Form Data

When using form data, set:
- Content-Type: `application/x-www-form-urlencoded`
- Body: `key1=value1&key2=value2`

## Reading Responses

### Response Information

| Field | Description |
|-------|-------------|
| Status Code | HTTP status (200, 404, 500, etc.) |
| Status Text | Description (OK, Not Found, etc.) |
| Response Time | Time to receive response |
| Response Headers | Headers returned by server |
| Response Body | Response data |

### Status Codes

| Code Range | Meaning |
|------------|---------|
| 2xx | Success |
| 3xx | Redirect |
| 4xx | Client error |
| 5xx | Server error |

### Common Status Codes

| Code | Meaning |
|------|---------|
| 200 | OK - Success |
| 201 | Created - Resource created |
| 400 | Bad Request - Invalid request |
| 401 | Unauthorized - Auth required |
| 403 | Forbidden - Access denied |
| 404 | Not Found - Resource doesn't exist |
| 500 | Internal Server Error |

## Response Formatting

- JSON responses are automatically formatted
- Syntax highlighting for readability
- Collapsible sections for nested data

## Examples

### Test Public API

GitHub API example:

1. URL: `https://api.github.com/users/octocat`
2. Method: **GET**
3. Click **Send**

Expected response:
```json
{
  "login": "octocat",
  "id": 583231,
  "type": "User",
  ...
}
```

### Create Resource

POST request example:

1. URL: `https://jsonplaceholder.typicode.com/posts`
2. Method: **POST**
3. Header: `Content-Type: application/json`
4. Body:
   ```json
   {
     "title": "Test Post",
     "body": "This is a test",
     "userId": 1
   }
   ```
5. Click **Send**

## Tips and Best Practices

### Testing Workflow

1. Start with simple GET requests
2. Verify endpoints exist and respond
3. Add authentication as needed
4. Test with minimal data first

### Debugging

1. Check response status codes
2. Inspect response headers
3. Look for error messages in body
4. Verify URL is correct

### Security

- Never share API keys or tokens
- Use test/development credentials
- Be cautious with production APIs

## Limitations

- No request history (requests not saved)
- No environment variables
- No automated testing
- File uploads not supported

For advanced API testing needs, consider dedicated tools like Postman.

## Troubleshooting

### Connection Failed

1. **Check URL**: Verify URL is correct
2. **Check network**: Ensure internet connection
3. **Check firewall**: May be blocking requests

### Authentication Errors (401)

1. **Check credentials**: Verify token/key is correct
2. **Check header format**: Authorization header must be exact
3. **Check expiration**: Tokens may have expired

### Request Timeout

1. **Check server**: Server may be slow or down
2. **Reduce payload**: Large requests may timeout
3. **Try again**: Temporary network issues

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
