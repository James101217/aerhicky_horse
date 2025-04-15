 #pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>

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

int main() {
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

    while (true) {
        client = accept(server, (SOCKADDR*)&clientAddr, &clientSize);
        if (client == INVALID_SOCKET) continue;

        std::ofstream out("screen.bmp", std::ios::binary);
        char buffer[1024];
        int bytesRead;

        while ((bytesRead = recv(client, buffer, sizeof(buffer), 0)) > 0) {
            out.write(buffer, bytesRead);
        }

        out.close();
        closesocket(client);

        HBITMAP hBmp = LoadBMP("screen.bmp");
        if (hBmp) {
            DrawBMP(hBmp);
            DeleteObject(hBmp);
        }
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
