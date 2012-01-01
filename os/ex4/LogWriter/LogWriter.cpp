#include <windows.h>
#include <tchar.h>
#include "../g_log/g_log.h"


int write_logs(HANDLE exit_evt, GLHANDLE handle, int delay_ms)
{
	DWORD seq = 0;
	while (true) {
		seq = GLWriteLogEntry(handle);
		if (seq == -1) {
			return -1;
		}
		if (WaitForSingleObject(exit_evt, delay_ms) == WAIT_OBJECT_0) {
			// we're done here
			break;
		}
	}
	return seq;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = -1;
	if (argc != 6) {
		_tprintf(_T("Usage: %s <logname> <logsize> <timeout> <mmf 1/0> <delay ms>\n"));
	}
	int logsize = _tstoi(argv[2]);
	if (logsize <= 0) {
		_tprintf(_T("logsize must be > 0\n"));
		return -1;
	}
	int timeout = _tstoi(argv[3]);
	bool mmf = _tcscmp(argv[4], _T("1")) == 0;
	int delay_ms = _tstoi(argv[5]);
	if (delay_ms < 0) {
		_tprintf(_T("delay-ms must be >= 0\n"));
		return -1;
	}

	_TCHAR buf[MAX_PATH];
	make_obj_name(buf, _T("tomerfiliba.ex4.exit_evt-"), argv[1]);
	
	HANDLE exit_evt = CreateEvent(NULL, TRUE, FALSE, buf);
	if (exit_evt == NULL) {
		_tprintf(_T("CreateEvent exit_evt failed\n"));
		return -1;
	}

	GLHANDLE handle = GLStartLogging(argv[1], logsize, timeout, mmf);
	if (handle == (GLHANDLE)-1) {
		_tprintf(_T("GLStartLogging failed\n"));
		return -1;
	}

	ret = write_logs(exit_evt, handle, delay_ms);
	GLStopLogging(handle);
	CloseHandle(exit_evt);
	return ret;
}

