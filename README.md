# min-server

## Introduction
Minimalistic C++ HTTP(s) server for easy serving of static files. It's useful for testing emscripten WASM frontends. It can also be made to serve dynamic content with custom entry points and bare-bones API.

## Building
    
    cmake -B build && cmake --build build -j `nproc`

## Static Usage
It automatically serves `main_page.html` and `index.html` from the directory where the binary is located. Other serve dirs can be added
as command line arguments. The root path is the executable directory.

    ./min-server /dir1 /dir2 /dir3

## Dynamic Usage
The function `Server::AddHandler(const char* endpointRegex, ConnectionHandler handler);` gives the ability to add custom handler for an entry point.
The `ConnectionHandler` takes `ConnectionState` argument that contains info for the current connection and supports the following API:

    StrView GetRequestBody() const;
    void AddHeader(CStr name, CStr value);
    void AddToBody(CStr contents);
    void SetResponseToJSON();
    void Reply(U32 code = 200);

    U32 GetRemoteIPv4Address() const;
    U16 GetRemotePort() const;
    

    B IsSecure() const;
    
    Str GetCookieValue(CStr valueName);
    void SetCookieValue(
                         CStr valueName,
                         CStr value,
                         CStr path = "/",
                         B secure = true,
                         B sameSite = true,
                         B httpOnly = true
                       );
