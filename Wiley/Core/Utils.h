#pragma once
#include <string>
#include <assert.h>
#include <Windows.h>
#include <comdef.h>

static std::string HResultToString(HRESULT hr) {
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
#ifdef UNICODE
    int size = WideCharToMultiByte(CP_UTF8, 0, errMsg, -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, errMsg, -1, &result[0], size, nullptr, nullptr);
    return result;
#else
    return std::string(errMsg);
#endif
}


#define WILEY_NODISCARD [[nodiscard]]
#define WILEY_NORETURN [[noreturn]]
#define WILEY_NOEXCEPT  noexcept
#define WILEY_DEBUGBREAK __debugbreak()

#define WILEY_FINAL final


