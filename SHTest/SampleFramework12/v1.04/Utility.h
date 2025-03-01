//=================================================================================================
//
//  MJP's DX12 Sample Framework
//  https://therealmjp.github.io/
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "PCH.h"

#include "Exceptions.h"
#include "InterfacePointers.h"
#include "SF12_Math.h"
#include "SF12_Assert.h"
#include "Containers.h"

namespace SampleFramework12
{

// Converts an ANSI string to a std::wstring
inline std::wstring AnsiToWString(const char* ansiString)
{
    wchar buffer[512];
    Win32Call(MultiByteToWideChar(CP_ACP, 0, ansiString, -1, buffer, 512));
    return std::wstring(buffer);
}

inline std::string WStringToAnsi(const wchar* wideString)
{
    char buffer[512];
    Win32Call(WideCharToMultiByte(CP_ACP, 0, wideString, -1, buffer, 512, NULL, NULL));
    return std::string(buffer);
}

// Splits up a string using a delimiter
inline void Split(const std::wstring& str, List<std::wstring>& parts, const std::wstring& delimiters = L" ")
{
    // Skip delimiters at beginning
    std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // Find first "non-delimiter"
    std::wstring::size_type pos = str.find_first_of(delimiters, lastPos);

    while(std::wstring::npos != pos || std::wstring::npos != lastPos)
    {
        // Found a token, add it to the vector
        parts.Add(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// Splits up a string using a delimiter
inline List<std::wstring> Split(const std::wstring& str, const std::wstring& delimiters = L" ")
{
    List<std::wstring> parts;
    Split(str, parts, delimiters);
    return parts;
}

// Splits up a string using a delimiter
inline void Split(const std::string& str, List<std::string>& parts, const std::string& delimiters = " ")
{
    // Skip delimiters at beginning
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // Find first "non-delimiter"
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while(std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector
        parts.Add(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// Splits up a string using a delimiter
inline List<std::string> Split(const std::string& str, const std::string& delimiters = " ")
{
    List<std::string> parts;
    Split(str, parts, delimiters);
    return parts;
}

void WriteLog(const wchar* format, ...);
void WriteLog(const char* format, ...);

std::wstring MakeString(const wchar* format, ...);
std::string MakeString(const char* format, ...);

std::wstring SampleFrameworkDir();

// Outputs a string to the debugger output and stdout
inline void DebugPrint(const std::wstring& str)
{
    std::wstring output = str + L"\n";
    OutputDebugStringW(output.c_str());
    std::printf("%ls", output.c_str());
}

// Gets an index from an index buffer
inline uint32 GetIndex(const void* indices, uint32 idx, uint32 indexSize)
{
    if(indexSize == 2)
        return reinterpret_cast<const uint16*>(indices)[idx];
    else
        return reinterpret_cast<const uint32*>(indices)[idx];
}


template<typename T, uint64 N>
uint64 ArraySize(T(&)[N])
{
    return N;
}

#define ArraySize_(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

inline uint32 AlignTo(uint32 num, uint32 alignment)
{
    Assert_(alignment > 0);
    return ((num + alignment - 1) / alignment) * alignment;
}

inline uint64 AlignTo(uint64 num, uint64 alignment)
{
    Assert_(alignment > 0);
    return ((num + alignment - 1) / alignment) * alignment;
}

template<typename To, typename From> To SafeCast(From x)
{
    Assert_(x >= std::numeric_limits<To>::min());
    Assert_(x <= std::numeric_limits<To>::max());
    return To(x);
}

}