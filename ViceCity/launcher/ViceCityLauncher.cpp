#include <Windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <shlobj.h>
#include <shellapi.h>
#include "ConfigLoader.hpp"
#include <nlohmann/json.hpp>

namespace Launcher {

Config g_config = Config::Load();

std::string GetSteamPath() {
    return g_config.steamPath;
}

bool IsProcessRunning(const std::string& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    
    bool found = false;
    if (Process32First(snapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName.c_str()) == 0) {
                found = true;
                break;
            }
        } while (Process32Next(snapshot, &pe32));
    }
    
    CloseHandle(snapshot);
    return found;
}

DWORD GetProcessId(const std::string& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    
    DWORD pid = 0;
    if (Process32First(snapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName.c_str()) == 0) {
                pid = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &pe32));
    }
    
    CloseHandle(snapshot);
    return pid;
}

bool StartSteam() {
    if (IsProcessRunning("steam.exe")) {
        std::cout << "[+] Steam already running\n";
        return true;
    }
    
    std::string steamPath = GetSteamPath();
    if (steamPath.empty()) {
        std::cerr << "[-] Could not find Steam installation\n";
        return false;
    }
    
    std::string exePath = steamPath + "\\steam.exe";
    if (!std::filesystem::exists(exePath)) {
        std::cerr << "[-] steam.exe not found at: " << exePath << "\n";
        return false;
    }
    
    std::cout << "[+] Starting Steam...\n";
    
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    if (!CreateProcessA(
        exePath.c_str(),
        const_cast<char*>("-silent"),
        nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        steamPath.c_str(),
        &si, &pi
    )) {
        std::cerr << "[-] Failed to start Steam: " << GetLastError() << "\n";
        return false;
    }
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    // Wait for Steam to fully start
    for (int i = 0; i < 30; ++i) {
        if (IsProcessRunning("steam.exe")) {
            std::cout << "[+] Steam started\n";
            return true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cerr << "[-] Steam start timeout\n";
    return false;
}

bool StartCS2() {
    std::string steamPath = GetSteamPath();
    if (steamPath.empty()) return false;
    
    std::string steamExe = steamPath + "\\steam.exe";
    std::string args = "steam://rungameid/" + g_config.cs2AppId;
    
    std::cout << "[+] Launching CS2...\n";
    
    HINSTANCE result = ShellExecuteA(
        nullptr, "open", steamExe.c_str(), args.c_str(), steamPath.c_str(), SW_SHOWNORMAL
    );
    
    if ((intptr_t)result <= 32) {
        std::cerr << "[-] Failed to launch CS2 via Steam: " << (intptr_t)result << "\n";
        return false;
    }
    
    return true;
}

DWORD WaitForCS2(int timeoutSeconds = 60) {
    std::cout << "[+] Waiting for cs2.exe...\n";
    
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        DWORD pid = GetProcessId("cs2.exe");
        if (pid != 0) {
            std::cout << "[+] Found cs2.exe (PID: " << pid << ")\n";
            return pid;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start
        ).count();
        
        if (elapsed >= timeoutSeconds) {
            std::cerr << "[-] Timeout waiting for cs2.exe\n";
            return 0;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool InjectDLL(DWORD pid, const std::string& dllPath) {
    if (!std::filesystem::exists(dllPath)) {
        std::cerr << "[-] DLL not found: " << dllPath << "\n";
        return false;
    }
    
    std::cout << "[+] Injecting " << dllPath << " into PID " << pid << "...\n";
    
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, pid
    );
    
    if (!hProcess) {
        std::cerr << "[-] OpenProcess failed: " << GetLastError() << "\n";
        return false;
    }
    
    // Allocate memory for DLL path
    LPVOID remotePath = VirtualAllocEx(hProcess, nullptr, dllPath.size() + 1, 
                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePath) {
        std::cerr << "[-] VirtualAllocEx failed: " << GetLastError() << "\n";
        CloseHandle(hProcess);
        return false;
    }
    
    if (!WriteProcessMemory(hProcess, remotePath, dllPath.c_str(), dllPath.size() + 1, nullptr)) {
        std::cerr << "[-] WriteProcessMemory failed: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // Get LoadLibraryA address
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID loadLibraryAddr = GetProcAddress(hKernel32, "LoadLibraryA");
    if (!loadLibraryAddr) {
        std::cerr << "[-] GetProcAddress failed: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // Create remote thread
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
                                       (LPTHREAD_START_ROUTINE)loadLibraryAddr,
                                       remotePath, 0, nullptr);
    if (!hThread) {
        std::cerr << "[-] CreateRemoteThread failed: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // Wait for injection to complete
    WaitForSingleObject(hThread, 5000);
    
    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    
    if (exitCode == 0) {
        std::cerr << "[-] Injection failed (LoadLibrary returned 0)\n";
        return false;
    }
    
    std::cout << "[+] Injection successful! (Base: 0x" << std::hex << exitCode << std::dec << ")\n";
    return true;
}

void PrintBanner() {
    std::cout << R"(
╔════════════════════════════════════════════════════╗
║           ViceCity CS2 Launcher v1.0.0            ║
║              Pink-Yellow Warmth Edition           ║
╚════════════════════════════════════════════════════╝
)" << std::endl;
}

int Run() {
    PrintBanner();
    
    // Get DLL path (same directory as launcher)
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    std::string exeDir = std::filesystem::path(exePath).parent_path().string();
    std::string dllPath = exeDir + "\\" + g_config.dllName;
    
    std::cout << "[+] Launcher directory: " << exeDir << "\n";
    std::cout << "[+] Target DLL: " << dllPath << "\n";
    std::cout << "[+] Steam path: " << g_config.steamPath << "\n";
    std::cout << "[+] Inject delay: " << g_config.injectDelay << "ms\n\n";
    
    // Step 1: Start Steam
    if (!StartSteam()) return 1;
    
    // Step 2: Launch CS2
    if (!StartCS2()) return 1;
    
    // Step 3: Wait for CS2 process
    DWORD pid = WaitForCS2(60);
    if (pid == 0) return 1;
    
    // Step 4: Wait for game to initialize
    std::cout << "[+] Waiting " << g_config.injectDelay << "ms for game initialization...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(g_config.injectDelay));
    
    // Step 5: Inject DLL
    if (!InjectDLL(pid, dllPath)) return 1;
    
    std::cout << "\n[+] ViceCity injected successfully!\n";
    std::cout << "[+] Press INSERT in-game to open menu\n";
    
    if (g_config.autoClose) {
        std::cout << "[+] Closing in 3 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    } else {
        std::cout << "[+] Press Enter to exit...";
        std::cin.get();
    }
    
    return 0;
}

} // namespace Launcher

int main() {
    try {
        return Launcher::Run();
    } catch (const std::exception& e) {
        std::cerr << "[-] Fatal error: " << e.what() << "\n";
        std::cin.get();
        return 1;
    }
}