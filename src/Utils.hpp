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
#include "Error.hpp"


#include <iostream>
#include <cstdio>
#include <bit>
#include <type_traits>
#include <chrono>
#include <filesystem>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <Bcrypt.h>
#else
    #include <sys/random.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#endif


inline U64 GetHighResTimeNS()
{
    auto timePoint = std::chrono::high_resolution_clock::now();
    auto duration = timePoint.time_since_epoch();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}


inline Str FirstWord(CStr cStr, C delim = ' ')
{
    Str result;

    while (cStr != nullptr && *cStr != 0 && !(isspace(*cStr) || *cStr == delim))
    {
        result += *cStr;
        cStr++;
    }

    return result;
}


Vec<StrView> SplitToWords(CStr cStr);
Vec<StrView> SplitToWordsRaw(CStr cStr);
Vec<StrView> SplitToWordsRaw(StrView cStr);

inline U64 GetSystemClock()
{
    auto timePoint = std::chrono::high_resolution_clock::now();
    auto duration = timePoint.time_since_epoch();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}


inline B FileExists(const StrView& path)
{
    return std::filesystem::exists(std::filesystem::path(path));
}


inline Str GetStdinLine()
{
    Str result;
    std::getline(std::cin, result);
    return result;
}


inline Str ReadStdin()
{
    static constexpr U32 initSize = 1 << 13;
    Str result;
    result.reserve(initSize);

    auto bytesRead = 0;
    while ((bytesRead = fread(result.data(), 1, initSize, stdin)))
    {
        printf("Read");
        result.resize(result.size() + bytesRead);
    }

    return result;
}


inline void RandomBytes(U8* buffer, U32 size)
{
#ifdef _WIN32
    BCryptGenRandom(nullptr, buffer, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
#else
    getrandom(buffer, size, 0);
#endif
}


Str BytesToHex(const U8* bytes, Size size);
void HexToBytes(const StrView& hex, U8* bytes);
B IsValidHexString(const StrView& str);

void ToUpper(Str& str);

template <typename... Ts>
void Log(Ts... args)
{
    (std::cout<<...<<args)<<std::endl;
    fflush(stdout);
}


template <typename... Ts>
void LogErr(Ts... args)
{
    (std::cerr<<...<<args)<<std::endl;
}


template <typename... Ts>
void LogDebug(Ts... args)
{
#ifdef VI_DEBUG
    Log(args...);
#endif
}

CStr FormatDuration(U64 d);

#define MEASURE_TIME(DESCRIPTION, CODE) \
do \
{ \
    auto __TIMER_START = GetHighResTimeNS(); \
    CODE; \
    auto __TIMER_END = GetHighResTimeNS(); \
    auto __TIMER_DURATION = __TIMER_END - __TIMER_START; \
    Log(DESCRIPTION, ": ", FormatDuration(__TIMER_DURATION)); \
} \
while(0)


template <typename T>
T 
ByteSwap(T n)
{
    if constexpr (sizeof(n) == 16)
    {
#ifdef _WIN32
        return _byteswap_short(n);
#else
        return __builtin_bswap16(n);
#endif
    }
    else if constexpr (sizeof(n) == 32)
    {
#ifdef _WIN32
        return _byteswap_long(n);
#else
        return __builtin_bswap32(n);
#endif
    }
    else if constexpr (sizeof(n) == 64)
    {
#ifdef _WIN32
        return _byteswap_uint64(n);
#else
        return __builtin_bswap64(n);
#endif
    }
    else
    {
        T result;
        U8* np = (U8*)&n;
        for (U32 i = 0; i < sizeof(np); ++i)
        {
            ((U8*)&result)[i] = np[sizeof(np) - 1 - i];
        }
        return result;
    }
}


template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value>::type
Serialize(Str& outBuff, T value)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        value = ByteSwap(value);
    }

    for(U32 i = 0; i < sizeof(T); ++i)
    {
        outBuff.push_back(((CStr)&value)[i]);
    }
}


inline void
Serialize(Str& outBuff, const Str& s)
{
    Serialize(outBuff, (U32)s.size());
    outBuff += s;
}


template <typename T, Size N>
inline void
Serialize(Str& outBuff, const Arr<T, N>& v)
{
    for (U32 i = 0; i < v.size(); ++i)
    {
        Serialize(outBuff, v[i]);
    }
}


template <Size N>
inline void
Serialize(Str& outBuff, const Arr<char, N>& v)
{
    for (U32 i = 0; i < v.size(); ++i)
    {
        outBuff += v[i];
    }
}


template <U32 I = 0, typename... Ts>
constexpr void 
Serialize(Str& outBuff, const Tuple<Ts...>& tuple)
{
    if constexpr (I == sizeof...(Ts))
    {
        return;
    }
    else
    {
        Serialize(outBuff, std::get<I>(tuple));
        Serialize<I + 1>(outBuff, tuple);
    }
}


template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, StrView>::type
Deserialize(const StrView& inBuff, T& value)
{
    value = T(0);

    for(U32 i = 0; i < sizeof(T); ++i)
    {
        value |= inBuff[i] << (8 * i);
    }

    if constexpr (std::endian::native == std::endian::big)
    {
        value = ByteSwap(value);
    }

    StrView result = inBuff;
    result.remove_prefix(sizeof(T));
    return result;
}


inline StrView
Deserialize(const StrView& inBuff, Str& s)
{
    U32 size;
    Deserialize(inBuff, size);
    
    StrView result = inBuff;
    result.remove_prefix(sizeof(size));
    s = result.substr(0, size);
    result.remove_prefix(size);

    return result;
}


template <typename T, Size N>
inline StrView
Deserialize(const StrView& inBuff, Arr<T, N>& v)
{
    StrView result = inBuff;

    for (U32 i = 0; i < v.size(); ++i)
    {
        result = Deserialize(result, v[i]);
    }
    
    return result;
}



template <Size N>
inline StrView
Deserialize(const StrView& inBuff, Arr<char, N>& v)
{
    for (U32 i = 0; i < v.size(); ++i)
    {
        v[i] = inBuff[i];
    }

    StrView result = inBuff;
    result.remove_prefix(v.size());
    return result;
}


template <U32 I = 0, typename... Ts>
constexpr StrView
Deserialize(const StrView& inBuff, Tuple<Ts...>& tuple)
{
    if constexpr (I == sizeof...(Ts))
    {
        return inBuff;
    }
    else
    {
        auto result = Deserialize(inBuff, std::get<I>(tuple));
        return Deserialize<I + 1>(result, tuple);
    }
}


template <typename T>
class Serializable
{
  public:
    void Serialize(Str& outBuff) const
    {
        ::Serialize(outBuff, static_cast<const T*>(this)->data);
    }

    void Deserialize(const Str& inBuff)
    {
        ::Deserialize(inBuff, static_cast<T*>(this)->data);
    }
};

#ifndef _WIN32
template <Size N>
Err ExecCommandWithStdinSync(const Arr<CStr, N>& argP, const Str& stdinContents)
{
    int pipefd[2];

    setuid(1000);

    if (pipe(pipefd))
    {
        return Err::Fail;
    }

    auto pid = fork();
    if (!pid)
    {
        // Close writting end to child.
        close(pipefd[1]);
        if (dup2(pipefd[0], 0) == -1)
        {
            exit(1);
        }
        //auto devNullFd = open("/dev/null", O_WRONLY);
        //dup2(devNullFd, STDOUT_FILENO);
        //dup2(devNullFd, STDERR_FILENO);
        execvp(argP[0], (C* const*)argP.data());
    }
    else
    {
        close(pipefd[0]);
        auto written = write(pipefd[1], stdinContents.c_str(), stdinContents.size());

        B writeFailed = false;
        if(written != stdinContents.size())
        {
            writeFailed = true;
        }
        close(pipefd[1]);

        I32 status;
        waitpid(pid, &status, 0);
        return (status || writeFailed)? Err::Fail : Err::Ok;
    }

    return Err::Ok;
}
#endif

#define M_INIT_GET_MEMBER \
    template <Members M> \
    typename std::tuple_element<(Size)M, decltype(data)>::type& \
    GetMember() \
    { \
        return std::get<(Size)M>(data); \
    } \
    template <Members M> \
    const typename std::tuple_element<(Size)M, decltype(data)>::type& \
    GetMember() const \
    { \
        return std::get<(Size)M>(data); \
    } 



template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, Str>::type
ToJSON(const Str& key, T value)
{
    Str result;
    result += "\"" + key + "\":" + std::to_string(value);
    return result;
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, Str>::type
ToJSON(T value)
{
    Str result = std::to_string(value);
    return result;
}


inline Str
ToJSON(const Str& key, CStr value)
{
    Str result;
    result += "\"" + key + "\":" + "\"" + value + "\"";
    return result;
}


inline Str
ToJSON(CStr value)
{
    Str result;
    result += Str("\"") + value + "\"";
    return result;
}


inline Str
ToJSON(const Str& key, const Str& value)
{
    return ToJSON(key, value.c_str());
}


inline Str
ToJSON(const Str& value)
{
    return ToJSON(value.c_str());
}


template <typename T, Size N>
inline Str
ToJSON(const Str& key, const Arr<T, N>& array)
{
    Str result = "\"" + key + "\":" + "[";

    for (U32 i = 0; i < N; ++i)
    {
        result += ToJSON(array[i]);

        if (i != N - 1)
        {
            result += ",";
        }
    }
    result += "]";

    return result;
}


template <typename T>
inline Str
ToJSON(const Str& key, const Vec<T>& array)
{
    Str result = "\"" + key + "\":" + "[";

    for (U32 i = 0; i < array.size(); ++i)
    {
        result += ToJSON(array[i]);

        if (i != array.size() - 1)
        {
            result += ",";
        }
    }
    result += "]";

    return result;
}



template <typename T>
struct JSONValue
{
    Str key;
    T value;

    JSONValue(const Str& key, const T& value) :
        key(key), value(value)
    {

    }

    Str ToJSON() const
    {
        return ::ToJSON(key, value);
    }
};


template <typename T, Size N>
inline Str
ToJSON(const Str& key, const Arr<JSONValue<T>, N>& array)
{
    Str result = "\"" + key + "\":" + "{";

    for (U32 i = 0; i < N; ++i)
    {
        result += array[i].ToJSON();

        if (i != N - 1)
        {
            result += ",";
        }
    }
    result += "}";

    return result;
}



template <typename T, Size N>
inline Str
ToJSON(const Arr<JSONValue<T>, N>& array)
{
    Str result = "{";

    for (U32 i = 0; i < N; ++i)
    {
        result += array[i].ToJSON();

        if (i != N - 1)
        {
            result += ",";
        }
    }
    result += "}";

    return result;
}





template <typename T>
inline Str
ToJSON(const Vec<JSONValue<T>>& array)
{
    Str result = "{";

    for (U32 i = 0; i < array.size(); ++i)
    {
        result += array[i].ToJSON();

        if (i != array.size() - 1)
        {
            result += ",";
        }
    }
    result += "}";

    return result;
}



template <typename T>
inline Str
ToJSON(const Str& key, const Vec<JSONValue<T>>& array)
{
    Str result = "\"" + key + "\":" + "{";

    for (U32 i = 0; i < array.size(); ++i)
    {
        result += array[i].ToJSON();

        if (i != array.size() - 1)
        {
            result += ",";
        }
    }
    result += "}";

    return result;
}