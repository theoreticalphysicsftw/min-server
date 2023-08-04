// Copyright 2022 Mihail Mladenov
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

enum class Err : U64
{
    Ok = 0,
    Fail,
    NotFound,
    Corruption,
    NotSupported,
    InvalidArgument,
    IOError
};



#ifdef VI_DEBUG 
    #define PRINT_ERROR(e) \
        printf("Error %s at file:%s, line: %u", #e, __FILE__, __LINE__)
#else
    #define PRINT_ERROR(e) \
        printf("Error %s", #e)
#endif


#define M_SUCCEEDED(e) \
    if (e != Err::Ok) \
    {\
        PRINT_ERROR(e); \
        printf("Error Message: %s\n", GetErrorMessage(e)); \
        exit(EXIT_FAILURE); \
    }


#define M_VERIFY(expression) \
    if (!(expression)) \
    {\
        printf("Verify failed: %s\n", #expression); \
        exit(EXIT_FAILURE); \
    }

#ifdef VI_DEBUG
    #define M_ASSERT(expression) M_VERIFY(expression)
#else
    #define M_ASSERT(expression)
#endif
