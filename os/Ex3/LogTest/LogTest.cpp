#include <tchar.h>
#include <windows.h>
#include "ex3_common.h"


bool spawn_proc(const TCHAR * cmdline, PROCESS_INFORMATION * pi)
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

bool spawn_writers(num_of_writers, log_file_path, ctrl_file_path, writer_delay_ms)
{
	return false;
}


int _tmain(int argc, _TCHAR* argv[])
{
	int ret = ERROR_RETCODE;

	if (argc != 6) {
		_tprintf(_T("Usage: %s <num-of-writers> <log-path> <ctrl-path> <writer-delay-ms> "
			"<lifespan-sec>\n"));
		return ERROR_RETCODE;
	}

	int num_of_writers = _tstoi(argv[1]);
	if (num_of_writers <= 0) {
		_tprintf(_T("<num-of-writers> must be a positive integer: '%s'\n"), argv[1]);
		return ERROR_RETCODE;
	}

	TCHAR * log_file_path = argv[2];
	TCHAR * ctrl_file_path = argv[3];

	int writer_delay_ms = _tstoi(argv[4]);
	if (writer_delay_ms <= 0) {
		_tprintf(_T("<writer-delay-ms> must be a positive integer: '%s'\n"), argv[4]);
		return ERROR_RETCODE;
	}

	int lifespan_sec = _tstoi(argv[5]);
	if (lifespan_sec <= 0) {
		_tprintf(_T("<lifespan-sec> must be a positive integer: '%s'\n"), argv[5]);
		return ERROR_RETCODE;
	}

	if (!spawn_writers(num_of_writers, log_file_path, ctrl_file_path, writer_delay_ms)) {
		return ERROR_RETCODE;
	}

	if (!spawn_viewer(log_file_path, ctrl_file_path)) {
		close_workers();
		return ERROR_RETCODE;
	}

	Sleep(lifespan_sec * 1000);

	close_workers();
	close_viewer();

	return ret;
}

