#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <windows.h>
#include <shlwapi.h>
#include <ws2tcpip.h>  // �����]�t�o���Y��
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

// ���m iplog.txt�A�M�ť�
void ResetIPLog(const std::string& ipLogPath) {
    std::ofstream ipOut(ipLogPath, std::ios::trunc);
    if (ipOut.is_open()) {
        ipOut.close();
    }
}

int main() {
    std::string basePath = GetExecutableDir();
    std::string bmpPath = basePath + "\\screen.bmp";
    std::string ipLogPath = basePath + "\\iplog.txt";  // �קאּ iplog.txt

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

    bool isMessageShown = false;  // �T�O�u��ܤ@���T��

    // �}�Үɭ��m iplog.txt
    ResetIPLog(ipLogPath);

    while (true) {
        client = accept(server, (SOCKADDR*)&clientAddr, &clientSize);
        if (client == INVALID_SOCKET) continue;

        // ? ���o client IP �����
        char ipStr[INET_ADDRSTRLEN];
        strcpy(ipStr, inet_ntoa(clientAddr.sin_addr));

        // �u��ܤ@�� IP ���ܵ���
        if (!isMessageShown) {
            MessageBoxA(NULL, ipStr, "�s�s�u�Ӧ�", MB_OK);
            isMessageShown = true;  // �]�m���w��ܡA���ᤣ�A���
        }

        // �N�s�� IP �g�J iplog.txt
        std::ofstream ipOut(ipLogPath, std::ios::app);
        if (ipOut.is_open()) {
            ipOut << ipStr << std::endl;
            ipOut.close();
        }

        // �����ù��e���ëO�s
        std::ofstream out(bmpPath, std::ios::binary);
        char buffer[1024];
        int bytesRead;

        while ((bytesRead = recv(client, buffer, sizeof(buffer), 0)) > 0) {
            out.write(buffer, bytesRead);
        }

        out.close();
        closesocket(client);

        // ��ܱ����쪺�ù��e��
        HBITMAP hBmp = LoadBMP(bmpPath.c_str());  // �ϥ� c_str() �ഫ std::string �� const char*
        if (hBmp) {
            DrawBMP(hBmp);
            DeleteObject(hBmp);
        }
    }

    closesocket(server);
    WSACleanup();
    return 0;
}

