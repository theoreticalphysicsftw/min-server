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


#include <cctype>

#include "Utils.hpp"


const C* FormatDuration(U64 d)
{
    static C buff[256];
    U32 length = 0;
    
    F32 adjusted = d;
    const C* secondsPrefix = "";
    if (d > 1E9)
    {
        adjusted /= 1E9;
    }
    else if (d > 1E6 && d < 1E9)
    {
        secondsPrefix = "m";
        adjusted /= 1E6;
    }
    else if (d > 1E3)
    {
        secondsPrefix = "u";
        adjusted /= 1E3;
    }
    else
    {
        secondsPrefix = "n";
    }

    sprintf(buff, "%.2f%ss", adjusted, secondsPrefix);

    return buff;
}


Str BytesToHex(const U8* bytes, Size size)
{
    Str result;
    result.reserve(size * 2);

    static constexpr C hexMap[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7', 
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    for (Size i = 0; i < size; ++i)
    {
        result.push_back(hexMap[(bytes[i] & 0xF0) >> 4]);
        result.push_back(hexMap[bytes[i] & 0x0F]);
    }
    
    return result;
}


// Assumes valid hex string!
void HexToBytes(const StrView& hex, U8* bytes)
{
    static constexpr U8 asciiToHex[] =
    {
        // Starts from '0'
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

    for (Size i = 0; i < hex.size() / 2; ++i)
    {
        bytes[i] = (asciiToHex[hex[i * 2] - '0'] << 4) +
                   asciiToHex[hex[i * 2 + 1] - '0'];
    }


}


B IsValidHexString(const StrView& str)
{
    for (auto symb : str)
    {
        if (!isxdigit(symb))
        {
            return false;
        }
    }
    
    return true;
}





Vec<StrView> SplitToWords(CStr cStr)
{
    Vec<StrView> result;
    CStr currentWord = nullptr;
    U32 currentLength = 0;
    B parsingSpace = true;
    B parsingQuote = false;
    while (cStr != nullptr && *cStr != 0)
    {
        B isSpace = isspace(*cStr);
        B isQuote = *cStr == '"';
        if (isSpace && !parsingSpace && !parsingQuote)
        {
            result.emplace_back(currentWord, currentLength);
            currentLength = 0;
            parsingSpace = true;
        }
        else if (!isSpace && parsingSpace && !isQuote && !parsingQuote)
        {
            currentWord = cStr;
            currentLength = 1;
            parsingSpace = false;
        }
        else if (isQuote && parsingQuote)
        {
            result.emplace_back(currentWord, currentLength);
            currentLength = 0;
            parsingQuote = false;
            parsingSpace = true;
        }
        else if (isQuote && !parsingSpace)
        {
            result.emplace_back(currentWord, currentLength);
            currentLength = 0;
            currentWord = cStr + 1;
            parsingQuote = true;
        }
        else if (isQuote)
        {
            currentLength = 0;
            currentWord = cStr + 1;
            parsingQuote = true;
            parsingSpace = false;
        }
        else if ((!isSpace && !parsingSpace) || parsingQuote)
        {
            currentLength++;
        }
        cStr++;
    }

    if (currentLength)
    {
        result.emplace_back(currentWord, currentLength);
    }

    return result;
}

auto
SplitToWordsRaw(CStr cStr) -> Vec<StrView>
{
    Vec<StrView> result;
    CStr currentWord = nullptr;
    U32 currentLength = 0;
    B parsingSpace = true;
    while (cStr != nullptr && *cStr)
    {
        B isSpace = isspace(*cStr);

        if (parsingSpace && !isSpace)
        {
            parsingSpace = false;
            currentWord = cStr;
            currentLength = 1;
        }
        else if (!parsingSpace && !isSpace)
        {
            currentLength++;
        }
        else if (!parsingSpace && isSpace)
        {
            parsingSpace = true;
            currentLength = 0;
            result.emplace_back(currentWord, currentLength);
        }
        cStr++;
    }

    return result;
}


auto
SplitToWordsRaw(StrView in) -> Vec<StrView>
{
    Vec<StrView> result;

    while (!in.empty())
    {
        in.remove_prefix(in.find_first_not_of(" \n\t\r"));
        auto wordEnd = std::min(in.find_first_of(" \n\t\r"), in.size());
        auto word = in.substr(0, wordEnd);
        
        if (!word.empty())
        {
            result.push_back(word);
        }
        in.remove_prefix(wordEnd);
    }

    return result;
}

void ToUpper(Str& str)
{
    for (auto& symb : str)
    {
        symb = toupper(symb);
    }
}
