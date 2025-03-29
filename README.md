# DLL Injection (LoadLibraryW) - Technical Breakdown

## Core Injection Process

1. **Target Process Selection**:
   - Takes a running process ID (PID)
   - Opens the process with `PROCESS_ALL_ACCESS` privileges

2. **Memory Allocation**:
   ```cpp
   LPVOID remoteMemory = VirtualAllocEx(hProc, NULL, size, 
       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
   ```
   - Allocates memory inside the target process
   - Space for the full DLL path string

3. **Path Writing**:
   ```cpp
   WriteProcessMemory(hProc, remoteMemory, dllPath.c_str(), size, NULL);
   ```
   - Writes the DLL path into the target process's memory space

4. **Thread Creation**:
   ```cpp
   HANDLE hThread = CreateRemoteThread(hProc, NULL, 0,
       (LPTHREAD_START_ROUTINE)GetProcAddress(
           GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"),
       remoteMemory, 0, NULL);
   ```
   - Creates remote thread that calls `LoadLibraryW`
   - Forces the target process to load your DLL

## Technical Notes
- Uses `LoadLibraryW`

This is standard DLL injection technique - simple but detectable by most anti-cheat systems. The method works by forcing the target process to load the DLL as if it requested it normally.