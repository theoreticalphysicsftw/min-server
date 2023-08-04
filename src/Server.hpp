// Copyright 2023 Mihail Mladenov
//
// This file is part of min-server.
//
// min-server is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// min-server is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with min-server.  If not, see <http://www.gnu.org/licenses/>.


#pragma once


#include "Types.hpp"

#include "mongoose/mongoose.h"


struct SessionStore;

using MgConnection = mg_connection;
using MgMgr = mg_mgr;
using MgHttpMessage = mg_http_message;
using MgHttpServeOpts = mg_http_serve_opts;


struct ConnectionState
{
    MgConnection* c;
    MgHttpMessage* httpMsg;
    I ev;
    SessionStore* sessionStore;
    Str responseHeaders;
    Str responseBody;

    void AddHeader(CStr name, CStr value);
    void AddToBody(CStr contents);
    void SetResponseToJSON();
    void Reply(U32 code = 200);

    ConnectionState();
    ConnectionState(MgConnection* c, MgHttpMessage* hm, I ev);

    U32 GetRemoteIPv4Address() const;
    U16 GetRemotePort() const;
    StrView GetRequestBody() const;

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
};


class Server
{
public:
    using ConnectionHandler = Func<void(ConnectionState*)>;

private:
    static Str httpAddress;
    static Str httpsAddress;
    static mg_mgr mgr;

    static Str certPath;
    static Str privKeyPath;
    static UMap<Str, ConnectionHandler> endpoints;
    static Vec<Str> servedDirs;

   
    static B TLSIsPossible();
    static void ServeFile(ConnectionState* cs, const C* pathOverride = nullptr);

public:
    static void Init(CStr addr, CStr certPath, CStr privKeyPath);
    static void AddHandler(CStr endpointRegex, ConnectionHandler handler);
    static void AddServeDir(CStr dir);
    static void HttpListener(MgConnection* c, I ev, void* evData, void* fnData);
    static void Run();
    static void Clean();
};
