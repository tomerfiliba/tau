#include <windows.h>
#include <tchar.h>

/*
NOTE: optimization must be disabled in this dll, or the SecretFunction 
will be inlined...
*/

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
	MessageBox(NULL, _T("I'm the secret"), _T("Shshshs"), MB_OK);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
