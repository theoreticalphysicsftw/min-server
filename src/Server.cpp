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


#include "Server.hpp"
#include "Utils.hpp"
#include <filesystem>

ConnectionState::ConnectionState()
{
}


ConnectionState::ConnectionState(MgConnection* c, MgHttpMessage* httpMsg, I ev) :
    c(c), httpMsg(httpMsg), ev(ev), sessionStore(nullptr)
{
}



void ConnectionState::AddHeader(const C* name, const C* value)
{
    responseHeaders += Str(name) + ": " + value + "\r\n";
}


void ConnectionState::AddToBody(const C* contents)
{
    responseBody += contents;
}


void ConnectionState::Reply(U32 code)
{
    mg_http_reply(c, I(code), responseHeaders.c_str(), responseBody.c_str());
}


void ConnectionState::SetResponseToJSON()
{
    AddHeader("Content-Type", "application/json");
}


auto
ConnectionState::GetRemoteIPv4Address() const -> U32
{
    return c->rem.ip;
}


auto
ConnectionState::GetRemotePort() const -> U16
{
    return c->rem.port;
}

auto 
ConnectionState::GetRequestBody() const -> StrView
{
    return StrView(httpMsg->body.ptr, httpMsg->body.len);
}


auto
ConnectionState::IsSecure() const -> B
{
    return c->is_tls;
}


auto
ConnectionState::GetCookieValue(CStr valueName) -> Str
{
    auto cookie = mg_http_get_header(httpMsg, "Cookie");
    Str value;

    if (cookie != nullptr)
    {
        auto tempValue = mg_http_get_header_var(*cookie, mg_str(valueName));
        value = Str(tempValue.ptr, tempValue.ptr + tempValue.len);
    }

    return value;
}

auto
ConnectionState::SetCookieValue(
                                 CStr valueName,
                                 CStr value,
                                 CStr path,
                                 B secure,
                                 B sameSite,
                                 B httpOnly
                               ) -> void
{
    auto cookieHeaderValue =
        Str(valueName) + "=" + value;

    cookieHeaderValue += "; Path=/";

    if (secure)
    {
        cookieHeaderValue += "; Secure";
    }
    if (sameSite)
    {
        cookieHeaderValue += "; SameSite=Strict";
    }
    if (httpOnly)
    {
        cookieHeaderValue += "; HttpOnly";
    }

    AddHeader("Set-Cookie", cookieHeaderValue.c_str());
}


UMap<Str, Func<void(ConnectionState*)>> Server::endpoints;
Str Server::certPath;
Str Server::privKeyPath;
Vec<Str> Server::servedDirs;

Str Server::httpAddress;
Str Server::httpsAddress;
mg_mgr Server::mgr;


void Server::Init(CStr addr, CStr certPath, CStr privKeyPath)
{
    httpAddress = Str("http://") + addr + ":80";
    httpsAddress = Str("https://") + addr + ":443";
    mg_log_set(MG_LL_DEBUG);
    mg_mgr_init(&mgr);

    certPath = certPath;
    privKeyPath = privKeyPath;
}


void Server::ServeFile(ConnectionState* cs, const C* pathOverride)
{
    MgHttpServeOpts opts = 
    {
        .extra_headers = cs->responseHeaders.c_str()
    };
    Str path = Str(DOCUMENT_ROOT) + (pathOverride? pathOverride : FirstWord(cs->httpMsg->uri.ptr));
    mg_http_serve_file(cs->c, cs->httpMsg, path.c_str(), &opts);
}


void Server::HttpListener(MgConnection* c, int ev, void* evData, void* fnData)
{
    if(ev == MG_EV_ACCEPT && fnData != nullptr)
    {
        mg_tls_opts opts =
            {
                .cert = certPath.c_str(),
                .certkey = privKeyPath.c_str()
            };
        mg_tls_init(c, &opts);
    }
    else if (ev == MG_EV_HTTP_MSG)
    {
        MgHttpMessage* hm = (MgHttpMessage*)evData;
        ConnectionState cs(c, hm, ev);
        
        if (
             mg_http_match_uri(hm, "/") ||
             mg_http_match_uri(hm, "/index.html") ||
             mg_http_match_uri(hm, "/main_page.html")
           )
        {
            ServeFile(&cs, "/main_page.html");
        }
        else
        {
            B endpointMatched = false;
            for (auto& endpoint : endpoints)
            {
                if (mg_http_match_uri(hm, endpoint.first.c_str()))
                {
                    endpoint.second(&cs);
                    endpointMatched = true;
                }
            }

            for (auto& dir : servedDirs)
            {
                if (mg_http_match_uri(hm, (dir + "/*").c_str()))
                {
                    ServeFile(&cs);
                    endpointMatched = true;
                }
            }

            if (!endpointMatched)
            {
                ServeFile(&cs, "/not_found.html");
            }
        }
    }
    (void)fnData;
}


B Server::TLSIsPossible()
{
    return FileExists(certPath) && FileExists(privKeyPath);
}

auto
Server::AddServeDir(CStr dir) -> void
{
    servedDirs.emplace_back(dir);
}


auto
Server::AddHandler(CStr endpointRegex, ConnectionHandler handler) -> void
{
    endpoints.emplace(Str(endpointRegex), handler);
}


void Server::Run()
{
    if (TLSIsPossible())
    {
        mg_http_listen(&mgr, httpsAddress.c_str(), Server::HttpListener, (void*) certPath.data());
    }
    else
    {
        mg_http_listen(&mgr, httpAddress.c_str(), Server::HttpListener, nullptr);
    }

    while (true)
    {
        mg_mgr_poll(&mgr, 16);
    }
}

void Server::Clean()
{
    mg_mgr_free(&mgr);
}
