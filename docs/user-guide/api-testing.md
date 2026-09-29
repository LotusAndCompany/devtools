# API Testing

Send HTTP requests and view responses directly from DevTools.

## Overview

The API Testing tool allows developers to quickly test REST APIs, debug endpoints, and inspect responses without needing separate tools like Postman or curl.

## Accessing the Tool

1. Launch DevTools
2. Click **HTTP Request** in the side menu

## Interface Overview

The API Testing interface consists of:

1. **Request bar**: Choose the HTTP method, enter the endpoint URL, and send
   the request
2. **Options**: Configure query parameters, basic authentication, or a request
   body in the corresponding tabs
3. **Response area**: View the status, timing, size, and response body

## Supported HTTP Methods

| Method | Description | Has Body |
|--------|-------------|----------|
| GET | Retrieve data | No |
| POST | Create new resource | Yes |
| PUT | Update/replace resource | Yes |
| DELETE | Remove resource | Optional |

## Making Requests

### Basic GET Request

1. Enter the URL: `https://api.example.com/users`
2. Select **GET** method
3. Click **Send**
4. View the response

### POST Request with JSON Body

1. Enter the URL: `https://api.example.com/users`
2. Select **POST** method
3. Open the **Body** tab and enter:
   ```json
   {
     "name": "John Doe",
     "email": "john@example.com"
   }
   ```
4. Click **Send**

### Request with Authentication

The **Authentication** tab supports HTTP Basic authentication. Enter both a
username and password; the tool adds the corresponding `Authorization` header
when sending the request.

## Query Parameters

Open the **Parameters** tab under **Options** and enter keys, values, and
descriptions in the table. Query parameters are appended to the URL as they
are edited.

## Authentication

Open the **Authentication** tab and enter a username and password. When both
fields are set, the request uses HTTP Basic authentication.

## Request Body

### JSON Body

Open the **Body** tab and enter the request body. The current tool sends the
body for `POST` and `PUT` requests with `Content-Type: application/json`.

```json
{
  "key": "value",
  "nested": {
    "item": "data"
  }
}
```

## Reading Responses

### Response Information

| Field | Description |
|-------|-------------|
| Status Code | HTTP status (200, 404, 500, etc.) |
| Status Text | Description (OK, Not Found, etc.) |
| Response Time | Time to receive response |
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

The response body is shown as read-only text. It remains selectable so that
you can copy response data from the response area.

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
3. Body:
   ```json
   {
     "title": "Test Post",
     "body": "This is a test",
     "userId": 1
   }
   ```
4. Click **Send**

## Tips and Best Practices

### Testing Workflow

1. Start with simple GET requests
2. Verify endpoints exist and respond
3. Add authentication as needed
4. Test with minimal data first

### Debugging

1. Check the response status and status text
2. Look for error messages in the response body
3. Verify the URL is correct

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

1. **Check credentials**: Verify the username and password are correct
2. **Check the request**: Confirm the endpoint requires HTTP Basic authentication

### Request Timeout

1. **Check server**: Server may be slow or down
2. **Reduce payload**: Large requests may timeout
3. **Try again**: Temporary network issues

## Related Documentation

- [Overview](overview.md) - Application overview
- [Quick Start](../getting-started/quick-start.md) - Getting started guide
