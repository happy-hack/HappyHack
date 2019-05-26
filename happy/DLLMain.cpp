#include <thread>
#include "Hooks.h"
#include "Utils\Utils.h"
#include "Utils\GlobalVars.h"

HINSTANCE HThisModule;

int OnDllAttach()
{
	Hooks::Init();

	while (!(GetAsyncKeyState(VK_END) && GetAsyncKeyState(VK_CONTROL)))
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	Hooks::Restore();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	FreeLibraryAndExitThread(HThisModule, 0);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH && GetModuleHandleA("csgo.exe"))
	{
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)OnDllAttach, NULL, NULL, NULL);
	}

	return TRUE;
}