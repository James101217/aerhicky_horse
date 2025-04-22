#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include <shlwapi.h>
#include <ws2tcpip.h>  // 必須包含這個頭檔
#include <iostream>
#include <fstream>
#include <string>

#define PORT 8080

HBITMAP LoadBMP(const char* filename) {
    return (HBITMAP)LoadImageA(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

void DrawBMP(HBITMAP hBitmap) {
    HWND hwnd = GetConsoleWindow();
    HDC hdc = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(hdc);

    SelectObject(memDC, hBitmap);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    StretchBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

    DeleteDC(memDC);
    ReleaseDC(hwnd, hdc);
}

std::string GetExecutableDir() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    PathRemoveFileSpecA(path);
    return std::string(path);
}

// 重置 iplog.txt，清空它
void ResetIPLog(const std::string& ipLogPath) {
    std::ofstream ipOut(ipLogPath, std::ios::trunc);
    if (ipOut.is_open()) {
        ipOut.close();
    }
}

int main() {
    std::string basePath = GetExecutableDir();
    std::string bmpPath = basePath + "\\screen.bmp";
    std::string ipLogPath = basePath + "\\iplog.txt";  // 修改為 iplog.txt

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (SOCKADDR*)&addr, sizeof(addr));
    listen(server, 1);

    std::cout << "等待圖片接收..." << std::endl;

    SOCKET client;
    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);

    bool isMessageShown = false;  // 確保只顯示一次訊息

    // 開啟時重置 iplog.txt
    ResetIPLog(ipLogPath);

    while (true) {
        client = accept(server, (SOCKADDR*)&clientAddr, &clientSize);
        if (client == INVALID_SOCKET) continue;

        // ? 取得 client IP 並顯示
        char ipStr[INET_ADDRSTRLEN];
        strcpy(ipStr, inet_ntoa(clientAddr.sin_addr));

        // 只顯示一次 IP 提示視窗
        if (!isMessageShown) {
            MessageBoxA(NULL, ipStr, "新連線來自", MB_OK);
            isMessageShown = true;  // 設置為已顯示，之後不再顯示
        }

        // 將新的 IP 寫入 iplog.txt
        std::ofstream ipOut(ipLogPath, std::ios::app);
        if (ipOut.is_open()) {
            ipOut << ipStr << std::endl;
            ipOut.close();
        }

        // 接收螢幕畫面並保存
        std::ofstream out(bmpPath, std::ios::binary);
        char buffer[1024];
        int bytesRead;

        while ((bytesRead = recv(client, buffer, sizeof(buffer), 0)) > 0) {
            out.write(buffer, bytesRead);
        }

        out.close();
        closesocket(client);

        // 顯示接收到的螢幕畫面
        HBITMAP hBmp = LoadBMP(bmpPath.c_str());  // 使用 c_str() 轉換 std::string 為 const char*
        if (hBmp) {
            DrawBMP(hBmp);
            DeleteObject(hBmp);
        }
    }

    closesocket(server);
    WSACleanup();
    return 0;
}

