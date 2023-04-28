#pragma once
#include <stdarg.h>
#include <windows.h>

void MessageBoxPrintfW(HWND hWnd, LPCWSTR lpCaption, UINT uType, LPCWSTR lpFormat, ...);
void MessageBoxPrintfA(HWND hWnd, LPCSTR lpCaption, UINT uType, LPCSTR lpFormat, ...);

void MessageBoxPrintfW(HWND hWnd, LPCWSTR lpCaption, UINT uType, LPCWSTR lpFormat, ...)
{
    // Allocate a buffer for the formatted message
    WCHAR szBuffer[1024];

    // Initialize the va_list and call vswprintf_s to format the message
    va_list argList;
    va_start(argList, lpFormat);
    vswprintf_s(szBuffer, sizeof(szBuffer) / sizeof(WCHAR), lpFormat, argList);
    va_end(argList);

    // Show the MessageBox with the formatted message
    MessageBoxW(hWnd, szBuffer, lpCaption, uType);
}

void MessageBoxPrintfA(HWND hWnd, LPCSTR lpCaption, UINT uType, LPCSTR lpFormat, ...)
{
    // Allocate a buffer for the formatted message
    CHAR szBuffer[1024];
    va_list argList;
    va_start(argList, lpFormat);
    // Call vsnprintf_s to format the message
    vsnprintf_s(szBuffer, sizeof(szBuffer) / sizeof(CHAR), _TRUNCATE, lpFormat, argList);
    va_end(argList);
    // Show the MessageBox with the formatted message
    MessageBoxA(hWnd, szBuffer, lpCaption, uType);
}