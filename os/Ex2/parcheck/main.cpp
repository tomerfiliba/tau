#include <tchar.h>
#include <wchar.h>
#include <windows.h>
#include <time.h>


void print_last_error(const _TCHAR * text)
{
	void * msg = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&msg, 0, NULL);

	if (text != NULL && text[0] != '\0') {
		_tprintf(_T("%s: %s\n"), text, (LPTSTR)msg);
	}
	else {
		_tprintf(_T("%s\n"), (LPTSTR)msg);
	}
	LocalFree(msg);
}

bool spawn_proc(TCHAR * cmdline, PROCESS_INFORMATION * pi)
{
	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(pi, sizeof(*pi));
	if (!CreateProcess(cmdline, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, pi)) {
		print_last_error(_T("CreateProcess"));
		return false;
	}

	return true;
}

int run_proc(TCHAR * cmdline)
{
	int retcode = -1;
	PROCESS_INFORMATION pi;

	if (!spawn_proc(cmdline, &pi)) {
		return -1;
	}
	if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED) {
		print_last_error(_T("WaitForSingleObject"));
		goto cleanup;
	}
	if (!GetExitCodeProcess(pi.hProcess, (LPDWORD)&retcode)) {
		print_last_error(_T("GetExitCodeProcess"));
		retcode = -1; // might have been changed by GetExitCodeProcess
		goto cleanup;
	}

cleanup:
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return retcode;
}


bool check_with_n_workers(int count, const TCHAR * filename, size_t filesize)
{
	LocalAlloc(
	int * checksums = HeapAlloc((sizeof(int) * count);
	PROCESS_INFORMATION * pis = malloc(sizeof(PROCESS_INFORMATION) * count);


	free(checksums);
	return false;
}



int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 3) {
		_tprintf(_T("Usage: %s <size in MB> <output file>\n"), argv[0]);
		return 1;
	}

	int size = _tstoi(argv[3]);
	if (size <= 0) {
		_tprintf(_T("'size in MB' must be a positive integer: '%s'\n"), argv[1]);
		return 1;
	}

	TCHAR randfn[MAX_PATH];
	if (GetTempFileName(_T("."), _T("foo"), 0, randfn) == 0) {
		print_last_error(_T("GetTempFileName"));
		return 1;
	}
	
	TCHAR cmdline[256];
	_stprintf_s(cmdline, _T("randomfile.exe %d %s"), size, randfn);
 	if (run_proc(cmdline) != 0) {
		_tprintf(_T("randomfile.exe failed\n"));
		return 1;
	}

	int num_of_processes[] = {1, 2, 4, 8};
	for (int i = 0; i < sizeof(num_of_processes) / sizeof(num_of_processes[0]); i++) {
		check_with_n_workers(
	}

	DeleteFile(randfn);
	return 0;
}

