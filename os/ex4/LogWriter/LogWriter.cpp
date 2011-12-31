#include <windows.h>
#include <tchar.h>
#include "../g_log/g_log.h"


int write_logs(HANDLE exit_evt, GLHANDLE handle)
{
	DWORD seq = 0;
	while (true) {
		if (WaitForSingleObject(exit_evt, 0) == WAIT_OBJECT_0) {
			// we're done here
			break;
		}
		seq = GLWriteLogEntry(handle);
		if (seq < 0) {
			return -1;
		}
	}
	return seq;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = -1;
	if (argc != 5) {
		_tprintf(_T("Usage: %s <logname> <logsize> <timeout> <mmf>\n"));
	}
	int logsize = _tstoi(argv[2]);
	if (logsize <= 0) {
		return -1;
	}
	int timeout = _tstoi(argv[3]);
	if (timeout <= 0) {
		return -1;
	}
	bool mmf = _tcsicmp(argv[4], _T("mmf")) == 0;

	_TCHAR buf[MAX_PATH];
	make_obj_name(buf, _T("tomerfiliba.ex4.exit_evt-"), argv[1]);
	
	HANDLE exit_evt = CreateEvent(NULL, TRUE, FALSE, buf);
	if (exit_evt == NULL) {
		return -1;
	}

	GLHANDLE handle = GLStartLogging(argv[1], logsize, timeout, mmf);
	if (handle == NULL) {
		return -1;
	}

	ret = write_logs(exit_evt, handle);
	GLStopLogging(handle);
	CloseHandle(exit_evt);
	return ret;
}

