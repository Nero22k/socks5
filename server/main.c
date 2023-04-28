#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Defs.h"
#include "Utils.h"
#include "resource.h"

BOOL isRunning = FALSE;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) 
{
    static const WCHAR lpctszTitle[] = L"SOCKS5 Reverse Proxy Server", lpctszClass[] = L"ServerWindowClass";
    WNDCLASSEX wc;
    HWND hwnd;
    MSG msg;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WindowProcedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_SHIELD);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"ServerWindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_SHIELD);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error!", MB_ICONERROR | MB_OK);
        return 0;
    }

    hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        lpctszClass,
        lpctszTitle,
        (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error!", MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SOCKS5port = DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_PORTDIALOG), hwnd, PortDialogProcedure, 0);
    if (SOCKS5port <= 0)
    {
        //MessageBoxPrintfW(NULL, L"Error", MB_ICONEXCLAMATION | MB_OK, L"Error (%ld)", GetLastError());
        MessageBoxW(NULL, L"Invalid port number entered. Exiting...", L"Error!", MB_ICONERROR | MB_OK);
        return 0;
    }

    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!TranslateAcceleratorW(hwnd, NULL, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return msg.wParam;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CustomData* pCustomData = (CustomData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE:
        pCustomData = (CustomData*)malloc(sizeof(CustomData));
        pCustomData->hListView = NULL;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCustomData);
        createMenu(hwnd);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) 
        { // Start server button clicked
            if (!isRunning)
            {
                isRunning = TRUE;
                startServerThread(hwnd);
            }
            else
            {
                MessageBoxPrintfW(NULL, L"Info!", MB_ICONINFORMATION | MB_OK, L"Server is already running on port (%d)", SOCKS5port);
            }
        }
        else if (LOWORD(wParam) == 2)
        {
            if (isRunning)
            {
                isRunning = FALSE;
                MessageBoxW(NULL, L"Server stopped!", L"Info!", MB_ICONINFORMATION | MB_OK);
            }
            else
            {
                MessageBoxW(NULL, L"Server not started!", L"Error", MB_ICONERROR | MB_OK);
            }
        }
        break;
    case WM_NOTIFY:
        NMHDR* pnmh = (NMHDR*)lParam;
        if (pnmh->idFrom == 2)
        {
            switch (pnmh->code)
            {
            case NM_CLICK:
                toggleStatus(pCustomData->hListView, lParam);
                break;
            }
        }
        break;
    case WM_APP + 1:
        pCustomData->hListView = createListView(hwnd);
        insertClientData(pCustomData->hListView, wParam, lParam);
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        free(pCustomData);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void toggleStatus(HWND hListView, LPARAM lParam) 
{
    LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)lParam;
    if (pnmia->iSubItem == 3)
    {
        WCHAR szStatus[32];
        ListView_GetItemText(hListView, pnmia->iItem, 3, szStatus, _countof(szStatus));
        if (wcscmp(szStatus, L"Active") == 0)
        {
            wcscpy_s(szStatus, 32, L"Disabled");
        }
        else
        {
            wcscpy_s(szStatus, 32, L"Active");
        }
        ListView_SetItemText(hListView, pnmia->iItem, 3, szStatus);
    }
}

HWND createListView(HWND hwnd)
{
    HWND hListView = GetDlgItem(hwnd, 2);
    if (!IsWindow(hListView))
    {
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        int windowWidth = 500;
        int listViewWidth = 500;
        int listViewHeight = 250;
        int listViewX = 0;
        int listViewY = 0;

        hListView = CreateWindowExW(0, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT, listViewX, listViewY, listViewWidth, listViewHeight, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), NULL);

        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);

        LVCOLUMN lvColumn;
        lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

        lvColumn.cx = 80;
        lvColumn.pszText = L"Sock";
        ListView_InsertColumn(hListView, 0, &lvColumn);

        lvColumn.cx = 120;
        lvColumn.pszText = L"HOST/IP";
        ListView_InsertColumn(hListView, 1, &lvColumn);

        lvColumn.cx = 120;
        lvColumn.pszText = L"Computer Name";
        ListView_InsertColumn(hListView, 2, &lvColumn);

        lvColumn.cx = 80;
        lvColumn.pszText = L"Status";
        ListView_InsertColumn(hListView, 3, &lvColumn);
    }

    return hListView;
}

void insertClientData(HWND hListView, WPARAM wParam, LPARAM lParam) 
{
    LVITEM lvi;
    WCHAR szSockID[16];

    // Convert the random socket ID to a wide string
    swprintf(szSockID, 16, L"%d", wParam);

    // Find the correct index to insert the new item
    int itemCount = ListView_GetItemCount(hListView);
    int insertIndex = 0;
    for (; insertIndex < itemCount; ++insertIndex)
    {
        LVITEM currentItem;
        WCHAR currentItemText[16];
        ZeroMemory(&currentItem, sizeof(currentItem));
        currentItem.mask = LVIF_TEXT;
        currentItem.iItem = insertIndex;
        currentItem.iSubItem = 0;
        currentItem.pszText = currentItemText;
        currentItem.cchTextMax = _countof(currentItemText);
        ListView_GetItem(hListView, &currentItem);

        int currentItemSock = _wtoi(currentItemText);
        int newSock = _wtoi(szSockID);
        if (newSock < currentItemSock)
        {
            break;
        }
    }

    lvi.mask = LVIF_TEXT;
    lvi.iItem = insertIndex; // Use the insertIndex to insert the new item in the correct position
    lvi.iSubItem = 0;
    lvi.pszText = szSockID; // Sock id
    ListView_InsertItem(hListView, &lvi);

    lvi.iSubItem = 1;
    lvi.pszText = (LPWSTR)lParam; // IP and Port
    ListView_SetItem(hListView, &lvi);
    free((WCHAR*)lParam);

    lvi.iSubItem = 2;
    WCHAR ComputerName[255];
    DWORD USize = 255;
    GetComputerNameW(ComputerName, &USize);
    lvi.pszText = ComputerName; // Computer Name
    ListView_SetItem(hListView, &lvi);

    lvi.iSubItem = 3;
    lvi.pszText = L"Active"; // Status
    ListView_SetItem(hListView, &lvi);
}

void createMenu(HWND hwnd) 
{
    HMENU hMenu, hSubMenu;

    hMenu = CreateMenu();
    hSubMenu = CreatePopupMenu();
    AppendMenuW(hSubMenu, MF_STRING, 1, L"Start server");
    AppendMenuW(hSubMenu, MF_STRING, 2, L"Stop server");
    AppendMenuW(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"Options");
    SetMenu(hwnd, hMenu);
}

void startServerThread(HWND hwnd) 
{
    HANDLE hThread = CreateThread(NULL, 0, serverThreadFunction, (LPVOID)hwnd, 0, NULL);
    if (hThread == NULL) 
    {
        MessageBoxPrintfW(NULL, L"Error", MB_ICONEXCLAMATION | MB_OK, L"Server thread creation failed with error code (%ld)", GetLastError());
    }
    else {
        CloseHandle(hThread);
    }
}

DWORD WINAPI clientThreadFunction(LPVOID lpParam)
{
    SOCKET clientSocket = (SOCKET)lpParam;
    unsigned char buf[1024];
    int len;

    // Receive the client's greeting message
    len = recv(clientSocket, buf, 1024, 0);
    if (len <= 0) {
        closesocket(clientSocket);
        return 1;
    }

    // Check the SOCKS version (should be 5)
    if (buf[0] != 0x05) {
        closesocket(clientSocket);
        return 1;
    }

    // Get the number of authentication methods the client supports
    int numAuthMethods = buf[1];
    BOOL noAuthRequired = FALSE;

    // Check if the "No authentication required" method (0x00) is in the list of supported methods
    for (int i = 0; i < numAuthMethods; i++) {
        if (buf[2 + i] == 0x00) {
            noAuthRequired = TRUE;
            break;
        }
    }

    // If the "No authentication required" method is not supported, close the connection
    if (!noAuthRequired) {
        closesocket(clientSocket);
        return 1;
    }

    // Send a response indicating the supported authentication method (no authentication required)
    unsigned char response[2] = { 0x05, 0x00 };
    send(clientSocket, response, 2, 0);

    // Receive the client's connection request
    len = recv(clientSocket, buf, 1024, 0);
    if (len <= 0) {
        closesocket(clientSocket);
        return 1;
    }

    // Check the address type
    int addrType = buf[3];
    char targetHost[256];
    int targetPort;

    if (addrType == 0x01) { // IPv4 address
        struct in_addr ipv4Addr;
        memcpy(&ipv4Addr, buf + 4, 4);
        inet_ntop(AF_INET, &ipv4Addr, targetHost, sizeof(targetHost));
        targetPort = ntohs(*(unsigned short*)(buf + 8));
    }
    else if (addrType == 0x03) { // Domain name
        int domainNameLength = buf[4];
        memcpy(targetHost, buf + 5, domainNameLength);
        targetHost[domainNameLength] = '\0';
        targetPort = ntohs(*(unsigned short*)(buf + 5 + domainNameLength));
    }
    else {
        // Unsupported address type
        closesocket(clientSocket);
        return 1;
    }

    // Connect to the target server
    SOCKET targetSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (targetSocket == INVALID_SOCKET) {
        closesocket(clientSocket);
        return 1;
    }

    struct sockaddr_in targetAddr;
    ZeroMemory(&targetAddr, sizeof(targetAddr));
    targetAddr.sin_family = AF_INET;
    //targetAddr.sin_addr.s_addr = inet_addr(targetHost);
    struct addrinfo hints, * res;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(targetHost, NULL, &hints, &res) != 0) {
        closesocket(clientSocket);
        return 1;
    }

    targetAddr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);

    targetAddr.sin_port = htons(targetPort);

    if (connect(targetSocket, (SOCKADDR*)&targetAddr, sizeof(targetAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        closesocket(targetSocket);
        return 1;
    }

    // Send a successful connection response to the client
    unsigned char successResponse[10] = { 0x05, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    send(clientSocket, successResponse, 10, 0);

    // Forward data between the client and target server
    fd_set readfds;
    int maxFd;

    while (1) {
        // Clear the readfds set and add the client and target sockets
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        FD_SET(targetSocket, &readfds);

        maxFd = (clientSocket > targetSocket) ? clientSocket : targetSocket;

        // Wait for any of the sockets to have data to read
        int activity = select(maxFd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            // An error occurred
            break;
        }

        if (FD_ISSET(clientSocket, &readfds)) {
            // Data is available to read from the client socket
            len = recv(clientSocket, buf, sizeof(buf), 0);

            if (len <= 0) {
                // An error occurred or the client closed the connection
                break;
            }

            // Send the data to the target server
            send(targetSocket, buf, len, 0);
        }

        if (FD_ISSET(targetSocket, &readfds)) {
            // Data is available to read from the target socket
            len = recv(targetSocket, buf, sizeof(buf), 0);

            if (len <= 0) {
                // An error occurred or the target server closed the connection
                break;
            }

            // Send the data to the client
            send(clientSocket, buf, len, 0);
        }
    }

    // Close the client and target sockets
    closesocket(clientSocket);
    closesocket(targetSocket);
    return 0;
}

DWORD WINAPI serverThreadFunction(LPVOID lpParam)
{
    MessageBoxW(NULL, L"Server thread started", L"Info!", MB_ICONINFORMATION | MB_OK);

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        MessageBoxPrintfW(NULL, L"Error", MB_ICONERROR | MB_OK, L"WSAStartup failed with error code (%ld)", GetLastError());
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        MessageBoxPrintfW(NULL, L"Error", MB_ICONERROR | MB_OK, L"socket() failed with error code (%ld)", GetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SOCKS5port); // Bind Port

    iResult = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        MessageBoxPrintfW(NULL, L"Error", MB_ICONERROR | MB_OK, L"bind() failed with error code (%ld)", GetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        MessageBoxPrintfW(NULL, L"Error", MB_ICONERROR | MB_OK, L"listen() failed with error code (%ld)", GetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    SOCKET clientSocket;
    while (isRunning)
    {
        clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            MessageBoxPrintfW(NULL, L"Error", MB_ICONERROR | MB_OK, L"accept() failed with error code (%ld)", GetLastError());
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        if (isRunning)
        {
            // Create a separate thread for the connected client
            HANDLE hClientThread = CreateThread(NULL, 0, clientThreadFunction, (LPVOID)clientSocket, 0, NULL);
            if (hClientThread == NULL)
            {
                closesocket(clientSocket);
                MessageBoxPrintfW(NULL, L"Error", MB_ICONEXCLAMATION | MB_OK, L"Client thread creation failed with error code (%ld)", GetLastError());
            }
            else
            {
                CloseHandle(hClientThread);
            }

            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);

            int clientPort = ntohs(clientAddr.sin_port);

            HWND hwnd = (HWND)lpParam;

            char combinedInfo[INET_ADDRSTRLEN + 10]; // 10 extra characters for the port and separator
            sprintf_s(combinedInfo, INET_ADDRSTRLEN + 10, "%s:%d", clientIP, clientPort);

            // Convert the combined string to a wide string
            WCHAR* wCombinedInfo = (WCHAR*)malloc((INET_ADDRSTRLEN + 10) * sizeof(WCHAR));
            swprintf(wCombinedInfo, INET_ADDRSTRLEN + 10, L"%hs", combinedInfo);
            // Send a message to the main thread to create the ListView and insert data
            PostMessageW(hwnd, WM_APP + 1, (WPARAM)clientSocket, (LPARAM)wCombinedInfo);
        }
    }

    // Close client and server sockets
    closesocket(listenSocket);
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

INT_PTR CALLBACK PortDialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        // Center the dialog box within the parent window
        HWND hParent = GetParent(hwnd);
        RECT rcDlg, rcParent;
        GetWindowRect(hwnd, &rcDlg);
        GetWindowRect(hParent, &rcParent);
        int x = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
        int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;
        SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        SetDlgItemInt(hwnd, IDC_PORTEDIT, 1080, FALSE);
    }
    return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            // Retrieve the entered port number
            WCHAR szPort[6];
            GetDlgItemTextW(hwnd, IDC_PORTEDIT, szPort, _countof(szPort));
            int portNumber = _wtoi(szPort);

            if (portNumber > 0 && portNumber <= 65535)
            {
                // Valid port number, so end the dialog and return the port number
                EndDialog(hwnd, portNumber);
                return (INT_PTR)TRUE;
            }
            else
            {
                MessageBoxW(hwnd, L"Please enter a valid port number (1-65535).", L"Invalid Port", MB_ICONERROR | MB_OK);
                return (INT_PTR)FALSE;
            }
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hwnd, 0);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}