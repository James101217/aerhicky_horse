#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h> // ���F INET_ADDRSTRLEN
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

void HideFile(const char* filename) {
    SetFileAttributesA(filename, FILE_ATTRIBUTE_HIDDEN);
}

std::string GetParentPath(const std::string& currentPath) {
    size_t pos = currentPath.find_last_of("\\/");
    if (pos != std::string::npos) {
        return currentPath.substr(0, pos);
    }
    return currentPath;
}

int main() {
    // �����e exe ���|
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string currentDir = GetParentPath(exePath); // �����e��Ƨ�

    // �]�w screen.bmp ���|
    std::string bmpPath = currentDir + "\\screen.bmp";
    std::string ipLogPath = currentDir + "\\iplog.txt";

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (SOCKADDR*)&addr, sizeof(addr));
    listen(server, 1);

    std::cout << "���ݹϤ�����..." << std::endl;

    SOCKET client;
    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);

    while (true) {
        client = accept(server, (SOCKADDR*)&clientAddr, &clientSize);
        if (client == INVALID_SOCKET) continue;

        // ��ܨðO���s�u IP
        const char* clientIP = inet_ntoa(clientAddr.sin_addr); // ���ϥΥ��ŧi�}�C
        MessageBoxA(NULL, clientIP, "�s�s�u�Ӧ�", MB_OK);

        std::ofstream iplog(ipLogPath, std::ios::app);
        iplog << clientIP << std::endl;
        iplog.close();

        // ���� BMP
        std::ofstream out(bmpPath, std::ios::binary);
        char buffer[1024];
        int bytesRead;
        while ((bytesRead = recv(client, buffer, sizeof(buffer), 0)) > 0) {
            out.write(buffer, bytesRead);
        }
        out.close();
        closesocket(client);

        // �]�w��������
        HideFile(bmpPath.c_str());

        // ��ܱ����쪺 BMP
        HBITMAP hBmp = LoadBMP(bmpPath.c_str());
        if (hBmp) {
            DrawBMP(hBmp);
            DeleteObject(hBmp);
        }
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
