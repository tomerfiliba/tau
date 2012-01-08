#include <windows.h>

extern "C" 
{
	void __declspec(dllexport) Run(DWORD i);
	void __declspec(dllexport) SecretFunction();
}

void Run(DWORD i)
{
	if (i == 78) {
		SecretFunction();
	}
}

void SecretFunction()
{
	Beep(480, 100);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
