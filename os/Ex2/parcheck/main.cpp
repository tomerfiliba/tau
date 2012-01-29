#include <tchar.h>
#include <wchar.h>
#include <windows.h>
#include <time.h>
#include <stdio.h>

#define MB                        (1024*1024)
#define MAX_PARALLEL_PROCESSES    (8)


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
	if (!CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, pi)) {
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

bool start_n_workers(HANDLE * hprocs, int count, const TCHAR * filename, size_t filesize_mb)
{
	PROCESS_INFORMATION pis[MAX_PARALLEL_PROCESSES];
	TCHAR cmdline[256];
	size_t chunk_size_mb = filesize_mb / count;

	for (int i = 0; i < count; i++) {
		size_t offset_mb = i * chunk_size_mb;
		if (i == count - 1) {
			// last worker -- take care of rounding errors
			chunk_size_mb = filesize_mb - offset_mb;
		}
		_stprintf_s(cmdline, _T("sectioncheck.exe %s %d %d"), filename, offset_mb * MB, 
			chunk_size_mb * MB);
		if (!spawn_proc(cmdline, &pis[i])) {
			_tprintf(_T("spawn_proc %s failed\n"), cmdline);
			// kill all previously spawned processes
			for (int j = 0; j < i; j++) {
				TerminateProcess(hprocs[j], 1);
				CloseHandle(hprocs[j]);
			}
			return false;
		}
		hprocs[i] = pis[i].hProcess;
	}

	return true;
}

bool check_with_n_workers(TCHAR * outline, int count, const TCHAR * filename, 
						  size_t filesize_mb)
{
	bool succ = false;
	unsigned char checksum = 0;
	HANDLE hprocs[MAX_PARALLEL_PROCESSES];

	if (count > MAX_PARALLEL_PROCESSES) {
		_tprintf(_T("attempted to create more than %d parallel processes\n"), 
			MAX_PARALLEL_PROCESSES);
		return false;
	}
	if (!start_n_workers(hprocs, count, filename, filesize_mb)) {
		return false;
	}

	clock_t t0 = clock();
	if (WaitForMultipleObjects(count, hprocs, TRUE, INFINITE) == WAIT_FAILED) {
		goto cleanup;
		return false;
	}
	clock_t t1 = clock();
	int total_time_ms = (1000 * (t1 - t0)) / CLOCKS_PER_SEC;

	_stprintf(outline, _T("%d %d "), count, total_time_ms);
	for (; *outline != 0; outline++); // advance outline

	for (int i = 0; i < count; i++) {
		DWORD ec;
		GetExitCodeProcess(hprocs[i], &ec);
		if (ec >= 0 && ec <= 255) {
			_stprintf(outline, _T("%d "), ec);
			for (; *outline != 0; outline++); // advance outline
			checksum ^= (unsigned char)ec;
		}
		else {
			goto cleanup;
		}
	}
	_stprintf(outline, _T("%d\r\n"), checksum);
	succ = true;

cleanup:
	for (int i = 0; i < count; i++) {
		CloseHandle(hprocs[i]);
	}
	return succ;
}



int _tmain(int argc, TCHAR *argv[])
{
	int retcode = 1;

	if (argc != 3) {
		_tprintf(_T("Usage: %s <size in MB> <output file>\n"), argv[0]);
		return 1;
	}

	int size_mb = _tstoi(argv[1]);
	if (size_mb <= 0) {
		_tprintf(_T("'size in MB' must be a positive integer: '%s'\n"), argv[1]);
		return 1;
	}

	TCHAR randfn[MAX_PATH];
	if (GetTempFileName(_T("."), _T("foo"), 0, randfn) == 0) {
		print_last_error(_T("GetTempFileName"));
		return 1;
	}

	HANDLE houtput = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (houtput == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile (output file)"));
		return 1;
	}

	TCHAR cmdline[256];
	_stprintf_s(cmdline, _T("randomfile.exe %d %s"), size_mb, randfn);
 	if (run_proc(cmdline) != 0) {
		_tprintf(_T("'%s' failed\n"), cmdline);
		goto cleanup1;
	}

	int num_of_processes[] = {1, 2, 4, 8};
	for (int i = 0; i < sizeof(num_of_processes) / sizeof(num_of_processes[0]); i++) {
		TCHAR line[256] = {0};
		if (!check_with_n_workers(line, num_of_processes[i], randfn, size_mb)) {
			_tprintf(_T("sectioncheck.exe failed with %d processes\n"), num_of_processes[i]);
			goto cleanup2;
		}

		DWORD linesize = _tcslen(line) * sizeof(TCHAR);
		DWORD written;

		if (!WriteFile(houtput, line, linesize, &written, NULL)) {
			print_last_error(_T("WriteFile of output file"));
			goto cleanup2;
		}
		if (linesize != written) {
			_tprintf(_T("WriteFile wrote less bytes than needed"));
			goto cleanup2;
		}
	}
	// success
	retcode = 0;

cleanup2:
	DeleteFile(randfn);
cleanup1:
	CloseHandle(houtput);
	return retcode;
}

