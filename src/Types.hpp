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

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <algorithm>
#include <utility>
#include <array>
#include <functional>
#include <thread>
#include <mutex>


using Str = std::string;
using Str32 = std::u32string;
using Str16 = std::u16string;
using StrView = std::string_view;

using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

using I8 = int8_t;
using I16 = int16_t;
using I32 = int32_t;
using I64 = int64_t;

using F32 = float;
using F64 = double;


using F = float;
using D = double;
using C = char;
using I = int;
using U = unsigned;
using S = short;
using B = bool;
using Void = void;

using Size = size_t;

using CStr = const C*;

template <typename... Ts>
using Tuple = std::tuple<Ts...>;

template<typename T>
using Func = std::function<T>;

template <typename T, typename U>
using Pair = std::pair<T, U>;

template <typename T>
using Vec = std::vector<T>;

template <typename T, U32 N>
using Arr = std::array<T, N>;

template <typename K, typename V, typename H = std::hash<K>>
using UMap = std::unordered_map<K, V, H>;

template <typename K,typename H = std::hash<K>>
using USet = std::unordered_set<K, H>;

using Thread = std::thread;
using Mutex = std::mutex;

template <typename T>
using LockGuard = std::lock_guard<T>;