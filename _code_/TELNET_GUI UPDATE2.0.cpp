#pragma comment(lib, "ws2_32.lib")
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <cstdio>

#define ID_CONNECT     101
#define ID_START       102
#define ID_TASKKILL    103
#define ID_MSG         104
#define ID_SHUTDOWN    105
#define ID_SEND        106
#define ID_SEND_START  107
#define ID_SEND_TASKKILL 108
#define ID_SEND_MSG    109

HWND hIPInput, hPortInput, hCommandInput, hSendBtn;
HWND hStartInput = NULL, hTaskkillInput = NULL, hMsgInput = NULL;
SOCKET telnetSocket = INVALID_SOCKET;

LPCWSTR className = L"TelnetGUI";

void ExecuteCommand(const std::wstring& cmd) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcessW(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

void ConnectToServer(HWND hwnd) {
    wchar_t ip[100], portStr[10];
    GetWindowTextW(hIPInput, ip, 100);
    GetWindowTextW(hPortInput, portStr, 10);
    int port = _wtoi(portStr);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    telnetSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(telnetSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
}

void SendCommand(const std::wstring& cmd) {
    if (telnetSocket != INVALID_SOCKET) {
        std::string narrow(cmd.begin(), cmd.end());
        send(telnetSocket, narrow.c_str(), narrow.length(), 0);
        send(telnetSocket, "\r\n", 2, 0);
    }
}

// 監控錯誤訊息窗口並自動關閉
void CloseErrorMessageWindow() {
    HWND hwndError = FindWindowW(L"#32770", NULL);  // 常見的錯誤訊息窗口類型
    if (hwndError != NULL) {
        SendMessageW(hwndError, WM_CLOSE, 0, 0);  // 關閉錯誤窗口
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CreateWindowW(L"STATIC", L"IP:", WS_CHILD | WS_VISIBLE, 20, 20, 30, 20, hwnd, NULL, NULL, NULL);
        hIPInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 60, 20, 120, 20, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"STATIC", L"Port:", WS_CHILD | WS_VISIBLE, 200, 20, 40, 20, hwnd, NULL, NULL, NULL);
        hPortInput = CreateWindowW(L"EDIT", L"2550", WS_CHILD | WS_VISIBLE | WS_BORDER, 250, 20, 60, 20, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Connect", WS_CHILD | WS_VISIBLE, 330, 20, 80, 25, hwnd, (HMENU)ID_CONNECT, NULL, NULL);

        // Basic buttons
        CreateWindowW(L"BUTTON", L"Start", WS_CHILD | WS_VISIBLE, 20, 60, 80, 30, hwnd, (HMENU)ID_START, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Taskkill", WS_CHILD | WS_VISIBLE, 20, 100, 80, 30, hwnd, (HMENU)ID_TASKKILL, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Msg", WS_CHILD | WS_VISIBLE, 20, 140, 80, 30, hwnd, (HMENU)ID_MSG, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Shutdown", WS_CHILD | WS_VISIBLE, 20, 190, 80, 30, hwnd, (HMENU)ID_SHUTDOWN, NULL, NULL);

        // Command Input area
        hCommandInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 150, 250, 300, 50, hwnd, NULL, NULL, NULL);
        hSendBtn = CreateWindowW(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE, 460, 250, 60, 25, hwnd, (HMENU)ID_SEND, NULL, NULL);

        // Start command input and send button
        hStartInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 60, 200, 25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE, 330, 60, 60, 25, hwnd, (HMENU)ID_SEND_START, NULL, NULL);

        // Taskkill command input and send button
        hTaskkillInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 100, 200, 25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE, 330, 100, 60, 25, hwnd, (HMENU)ID_SEND_TASKKILL, NULL, NULL);

        // Msg command input and send button
        hMsgInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 120, 140, 200, 25, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE, 330, 140, 60, 25, hwnd, (HMENU)ID_SEND_MSG, NULL, NULL);

        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_CONNECT:
            ConnectToServer(hwnd);
            break;
        case ID_SEND: {
            wchar_t buf[512];
            GetWindowTextW(hCommandInput, buf, 512);
            SendCommand(buf);
            break;
        }
        case ID_SEND_START: {
            wchar_t buf[256];
            GetWindowTextW(hStartInput, buf, 256);
            SendCommand(L"start " + std::wstring(buf));
            break;
        }
        case ID_SEND_TASKKILL: {
            wchar_t buf[256];
            GetWindowTextW(hTaskkillInput, buf, 256);
            SendCommand(L"taskkill /IM " + std::wstring(buf) + L" /F");
            break;
        }
        case ID_SEND_MSG: {
            wchar_t buf[256];
            GetWindowTextW(hMsgInput, buf, 256);
            SendCommand(L"msg * " + std::wstring(buf));
            break;
        }
        case ID_SHUTDOWN: {
            int res = MessageBoxW(hwnd, L"Are you sure you want to shutdown?", L"Confirm", MB_YESNO | MB_ICONQUESTION);
            if (res == IDYES) {
                SendCommand(L"shutdown /s /f /t 0");
            }
            break;
        }
        }
        break;
    }

    case WM_DESTROY:
        if (telnetSocket != INVALID_SOCKET) closesocket(telnetSocket);
        WSACleanup();
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, className, L"Telnet GUI", WS_OVERLAPPEDWINDOW,
        100, 100, 600, 400, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        CloseErrorMessageWindow(); // 在每個消息循環中檢查並關閉錯誤訊息
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
