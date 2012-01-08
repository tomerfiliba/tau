#include <windows.h>
#include <tchar.h>


HANDLE spawn_proc(TCHAR * cmdline, DWORD creation_flags)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(pi, sizeof(*pi));
	if (!CreateProcess(NULL, cmdline, NULL, NULL, FALSE, creation_flags, NULL, NULL, &si, pi)) {
		_tprintf(_T("CreateProcess failed"));
		return NULL;
	}
	CloseHandle(pi.hThread);
	return pi.hProcess;
}



int _tmain(int argc, TCHAR *argv[])
{
	int ret = 1;
	if (argc != 4) {
		_tprintf(_T("Usage: <dll> <from> <to>\n"));
		return 1;
	}

	TCHAR cmdline[1024];
	_stprintf_s(cmdline, _T("CallDetectorHelper.exe %s %s %s"), argv[1], argv[2], argv[3]);
	HANDLE hproc = spawn_proc(cmdline, DEBUG_ONLY_THIS_PROCESS);
	DEBUG_EVENT de;
	if (!WaitForDebugEvent(&de, 3000)) {
		_tprintf(_T("The secret function was not called"));
	}
	else {

	}
	if (WaitForSingleObject(hproc, INFINITE) != WAIT_OBJECT_0) {
		_tprintf(_T("WaitForSingleObject failed"));
	}
	CloseHandle(hproc);
}