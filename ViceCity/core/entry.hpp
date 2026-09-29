#pragma once

#include <Windows.h>

namespace core {

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);
DWORD WINAPI MainThread(LPVOID lpParam);
void InitializeCheat();
void ShutdownCheat();
void OnUnload();

} // namespace core