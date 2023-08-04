// Copyright 2022-2023 Mihail Mladenov
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


#include "Utils.hpp"
#include "Types.hpp"
#include "Server.hpp"

#include <filesystem>

int main(int argc, char** argv)
{
    auto certPath = DOCUMENT_ROOT"/min-server.org/fullchain.pem";
    auto privKeyPath = DOCUMENT_ROOT"/min-server.org/privkey.pem";
    Str address = "0.0.0.0";

    Server::Init(address.c_str(), certPath, privKeyPath);

    for (auto i = 1; i < argc; ++i)
    {
        Server::AddServeDir(argv[i]);
    }

    Server::Run();
    Server::Clean();

    return 0;
}
