#pragma once
#include <windows.h>
#include <iostream>
#include <sstream>
#include "stringapiset.h"

// 多字节字符串转宽字符字符串
inline std::wstring _A2W_(const char *pszText)
{
    if (pszText == NULL || strlen(pszText) == 0)
    {
        return std::wstring();
    }
    int iSizeInChars = MultiByteToWideChar(CP_ACP, 0, pszText, -1, NULL, 0);
    wchar_t *pWideChar = new (std::nothrow) wchar_t[iSizeInChars];
    if (pWideChar == NULL)
    {
        return std::wstring();
    }
    wmemset(pWideChar, 0, iSizeInChars);
    MultiByteToWideChar(CP_ACP, 0, pszText, -1, pWideChar, iSizeInChars);
    std::wstring strResult = std::wstring(pWideChar);
    delete[] pWideChar;
    pWideChar = NULL;
    return strResult;
}
// 宽字符字符串转UTF-8字符串
inline std::string _W2U8_(const wchar_t *pwszText)
{
    if (pwszText == NULL || wcslen(pwszText) == 0)
    {
        return std::string();
    }
    int iSizeInBytes = WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, NULL, 0, NULL, NULL);
    char *pUTF8 = new (std::nothrow) char[iSizeInBytes];
    if (pUTF8 == NULL)
    {
        return std::string();
    }
    memset(pUTF8, 0, iSizeInBytes);
    WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, pUTF8, iSizeInBytes, NULL, NULL);
    std::string strResult = std::string(pUTF8);
    delete[] pUTF8;
    pUTF8 = NULL;
    return strResult;
}
// 多字节字符串转UTF-8字符串
inline std::string _A2U8_(const char *pszText)
{
    return _W2U8_(_A2W_(pszText).c_str());
}
// 定义多字节转换至 UTF-8 宏_A2U8
#define _A2U8(lpszText) (const_cast<char *>(_A2U8_(static_cast<const char *>(lpszText)).c_str()))

// 宽字符字符串转多字节字符串
inline std::string _W2A_(const wchar_t *pwszText)
{
    if (pwszText == NULL || wcslen(pwszText) == 0)
    {
        return std::string();
    }
    int iSizeInBytes = WideCharToMultiByte(CP_ACP, 0, pwszText, -1, NULL, 0, NULL, NULL);
    char *pMultiByte = new (std::nothrow) char[iSizeInBytes];
    if (pMultiByte == NULL)
    {
        return std::string();
    }
    memset(pMultiByte, 0, iSizeInBytes);
    WideCharToMultiByte(CP_ACP, 0, pwszText, -1, pMultiByte, iSizeInBytes, NULL, NULL);
    std::string strResult = std::string(pMultiByte);
    delete[] pMultiByte;
    pMultiByte = NULL;
    return strResult;
}
// UTF-8字符串转宽字符字符串
inline std::wstring _U82W_(const char *pszText)
{
    if (pszText == NULL || strlen(pszText) == 0)
    {
        return std::wstring();
    }
    int iSizeInChars = MultiByteToWideChar(CP_UTF8, 0, pszText, -1, NULL, 0);
    wchar_t *pWideChar = new (std::nothrow) wchar_t[iSizeInChars];
    if (pWideChar == NULL)
    {
        return std::wstring();
    }
    wmemset(pWideChar, 0, iSizeInChars);
    MultiByteToWideChar(CP_UTF8, 0, pszText, -1, pWideChar, iSizeInChars);
    std::wstring strResult = std::wstring(pWideChar);
    delete[] pWideChar;
    pWideChar = NULL;
    return strResult;
}
// UTF-8字符串转多字节字符串
inline std::string _U82A_(const char *pszText)
{
    return _W2A_(_U82W_(pszText).c_str());
}
// 定义 UTF-8 转换至多字节宏_U82A
#define _U82A(lpu8Text) (const_cast<char *>(_U82A_(static_cast<const char *>(lpu8Text)).c_str()))