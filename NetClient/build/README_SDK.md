# NetClient SDK Documentation

Welcome to the `NetClient` C++ SDK. This library provides a simple, Python-like interface for making HTTP requests.

## Folder Structure
- `bin/`: Contains `NetClient.dll` (Windows) or `libNetClient.so` (Linux).
- `lib/`: Contains `NetClient.lib` for linking.
- `include/`: Contains `NetClient.h`.

## Quick Start

```cpp
#include "NetClient.h"

int main() {
    auto resp = NetClient::get("https://api.example.com");
    if (resp.ok()) {
        printf("Response: %s\n", resp.text.c_str());
    }
    return 0;
}
```

## API Reference
- `get(url, params, headers)`
- `post(url, data, headers)`
- `put(url, data, headers)`
- `del(url, headers)`
- `options(url, headers)`

### Response Object
- `status_code`: HTTP status code (int).
- `text`: Response body string.
- `ok()`: Returns true if status is 2xx.
- `is_json()`: Checks if the response is JSON.
- `header(name)`: Retrieves a header value.
