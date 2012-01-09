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

bool found_breakpoint = false;

int exception_filter(DWORD code, void * ptr, BYTE orig, int i)
{
	_tprintf(_T("FOOOO\n"));
	if (code == EXCEPTION_BREAKPOINT) {
		unpatch_function(ptr, orig);
		_tprintf(_T("The SecretCode function was called using parameter value=%d\n"),i);
		found_breakpoint = true;
		// let the execution continue, we don't want to interfere with the normal path
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
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

	BYTE orig = patch_function(secret_func);
	for (int i = from; i < to; i++) {
		__try {
			run_func(i);
		}
		__except(exception_filter(GetExceptionCode(), secret_func, orig, i)) {
			// this will never be called -- the handling is done in exception_filter
			abort();
		}
	}
	if (!found_breakpoint) {
		_tprintf(_T("No call to SecretFunction was detected\n"));
	}

cleanup:
	if (dll != NULL) {
		FreeLibrary(dll);
	}
	return ret;
}
