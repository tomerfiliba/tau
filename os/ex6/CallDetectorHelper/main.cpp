#include <windows.h>
#include <tchar.h>


typedef void (*Run_t)(DWORD i);
typedef void (*SecretFunction_t)(void);

#define INT_03  (0xCC)

BYTE patch_function(void * ptr)
{
	MEMORY_BASIC_INFORMATION meminfo;
	BYTE * pByte = (BYTE*)ptr;
	DWORD oldprotect;
	if (!VirtualQuery(ptr, &meminfo, sizeof(meminfo))) {
		abort();
	}
	if (!VirtualProtect(meminfo.BaseAddress, meminfo.RegionSize, PAGE_EXECUTE_READWRITE, &oldprotect)) {
		abort();
	}
	BYTE orig = *pByte;
	*pByte = INT_03;
	return orig;
}

void unpatch_function(void * ptr, BYTE orig)
{
	BYTE * pByte = (BYTE*)ptr;
	*pByte = orig;
}

int _tmain(int argc, TCHAR *argv[])
{
	int ret = 1;
	if (argc != 4) {
		_tprintf(_T("Usage: <dll> <from> <to>\n"));
		return 1;
	}

	HMODULE dll = LoadLibrary(argv[1]);
	if (dll == NULL) {
		_tprintf(_T("Could not find function 'Run' in the dll\n"));
		goto cleanup;
	}
	Run_t run_func = (Run_t)GetProcAddress(dll, "Run");
	if (run_func == NULL) {
		_tprintf(_T("Could not find function 'Run' in the dll\n"));
		goto cleanup;
	}
	SecretFunction_t secret_func = (SecretFunction_t)GetProcAddress(dll, "SecretFunction");
	if (secret_func == NULL) {
		_tprintf(_T("Could not find function 'SecretFunction' in the dll\n"));
		goto cleanup;
	}

	int from = _tstoi(argv[2]);
	int to = _tstoi(argv[3]);

	BYTE prev = patch_function(secret_func);
	for (int i = from; i <= to; i++) {
		run_func(i);
	}
	unpatch_function(secret_func, prev);

cleanup:
	if (dll != NULL) {
		FreeLibrary(dll);
	}
	return ret;
}
