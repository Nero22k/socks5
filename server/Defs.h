#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
#include <commctrl.h>
#include <time.h>

#pragma comment(lib, "Ws2_32.lib")

#define USERNAME "Aquamen"
#define PASSWORD "Password123"

int SOCKS5port = 1080;

typedef struct ServerThreadParams 
{
    HWND hwnd;
    int serverPort;
} ServerThreadParams;

typedef struct tagCustomData {
    HWND hListView;
} CustomData;

INT_PTR CALLBACK PortDialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int promptForPortNumber(HWND hwnd);
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI serverThreadFunction(LPVOID lpParam);
DWORD WINAPI clientThreadFunction(LPVOID lpParam);
void startServerThread(HWND hwnd);
void createMenu(HWND hwnd);
void toggleStatus(HWND hListView, LPARAM lParam);
void insertClientData(HWND hListView, WPARAM wParam, LPARAM lParam);
HWND createListView(HWND hwnd);