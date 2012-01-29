#include <windows.h>
#include <tchar.h>
#include "../g_log/g_log.h"


typedef struct {
	_TCHAR log_name[256];
	int num_of_writers;
	int writer_delay_ms;
	HANDLE * writer_hprocs;
	HANDLE viewer_hproc;
	HANDLE exit_evt;
	int timeout;
	int num_of_entries;
	bool is_mmf;
} program_state_t;


void print_last_error(const _TCHAR * text)
{
	void * msg = NULL;
	DWORD code = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL);

	if (text != NULL && text[0] != '\0') {
		_tprintf(_T("%s: %s (%d)\n"), text, (LPTSTR)msg, code);
	}
	else {
		_tprintf(_T("%s (%d)\n"), (LPTSTR)msg, code);
	}
	LocalFree(msg);
}

bool spawn_proc(_TCHAR * cmdline, DWORD creation_flags, HANDLE * hproc)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (!CreateProcess(NULL, cmdline, NULL, NULL, FALSE, creation_flags, NULL, NULL, &si, &pi)) {
		print_last_error(_T("CreateProcess"));
		return false;
	}

	*hproc = pi.hProcess;
	CloseHandle(pi.hThread);
	return true;
}

bool startup(program_state_t * state)
{
	_TCHAR cmdline[2000];

	_stprintf_s(cmdline, _T("LogViewer.exe %s %d %d %d %d"), state->log_name, 
		state->num_of_writers, state->num_of_entries, state->timeout, state->is_mmf ? 1 : 0);
	if (!spawn_proc(cmdline, CREATE_NEW_CONSOLE, &state->viewer_hproc)) { 
		return false;
	}

	_stprintf_s(cmdline, _T("LogWriter.exe %s %d %d %d %d"), state->log_name, 
		state->num_of_entries, state->timeout, state->is_mmf ? 1 : 0, state->writer_delay_ms);
	for (int i = 0; i < state->num_of_writers; i++) {
		if (!spawn_proc(cmdline, CREATE_NO_WINDOW, &state->writer_hprocs[i])) {
			// tell everyone who's already started to exit gracefully
			SetEvent(state->exit_evt);
			return false;
		}
	}

	return true;
}

bool wait_all_workers(program_state_t * state)
{
	int remaining = state->num_of_writers;

	// windows sucks, so we have to use WaitForMultipleObjects repeatedly in chunks of 64
	// processes each time. why are we learning an OS course on windows?!

	for (int i = 0; remaining > 0; i += MAXIMUM_WAIT_OBJECTS) {
		int count = (remaining > MAXIMUM_WAIT_OBJECTS) ? MAXIMUM_WAIT_OBJECTS : remaining;
		if (WaitForMultipleObjects(count, &state->writer_hprocs[i], TRUE, INFINITE) == WAIT_FAILED) {
			print_last_error(_T("WaitForMultipleObjects"));
			return false;
		}
		remaining -= count;
	}
	return true;
}

bool wrapup(program_state_t * state)
{
	if (!SetEvent(state->exit_evt)) {
		print_last_error(_T("SetEvent"));
		return false;
	}
	if (!wait_all_workers(state)) {
		return false;
	}
	if (WaitForSingleObject(state->viewer_hproc, INFINITE) != WAIT_OBJECT_0) {
		print_last_error(_T("WaitForSingleObject"));
		return false;
	}

	// print the summary
	for (int i = 0; i < state->num_of_writers; i++) {
		DWORD ec;
		if (!GetExitCodeProcess(state->writer_hprocs[i], &ec)) {
			ec = -1;
		}
		_tprintf(_T("Process id 0x%08X, last log entry: 0x%08X\n"),
			GetProcessId(state->writer_hprocs[i]), ec);
	}

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = -1;
	program_state_t state;

	if (argc != 6) {
		_tprintf(_T("Usage: %s <num-of-writers> <log-name> <mmf 0/1> <writer-delay-ms> <lifespan-sec>\n"), 
			argv[0]);
		return -1;
	}
	state.num_of_writers = _tstoi(argv[1]);
	if (state.num_of_writers <= 0) {
		_tprintf(_T("num of writers must be > 0\n"));
		return -1;
	}
	_tcscpy_s(state.log_name, argv[2]);
	state.num_of_entries = 10; // defined in the spec
	state.timeout = INFINITE;
	state.is_mmf = _tcscmp(argv[3], _T("1")) == 0;
	state.writer_delay_ms = _tstoi(argv[4]);
	if (state.writer_delay_ms < 0) {
		_tprintf(_T("write_delay_ms must be >= 0\n"));
		return -1;
	}
	int lifespan_sec = _tstoi(argv[5]);
	if (lifespan_sec <= 0) {
		_tprintf(_T("time to live must be > 0\n"));
		return -1;
	}

	state.viewer_hproc = NULL;
	state.writer_hprocs = NULL;

	_TCHAR buf[MAX_PATH];
	make_obj_name(buf, _T("tomerfiliba.ex4.exit_evt-"), argv[2]);
	state.exit_evt = CreateEvent(NULL, TRUE, FALSE, buf);
	if (state.exit_evt == NULL) {
		print_last_error(_T("CreateEvent"));
		return -1;
	}
	state.writer_hprocs = (HANDLE*)calloc(state.num_of_writers, sizeof(HANDLE));
	if (state.writer_hprocs == NULL) {
		_tprintf(_T("malloc failed\n"));
		goto cleanup;
	}

	if (!startup(&state)) {
		goto cleanup2;
	}
	Sleep(lifespan_sec * 1000);
	if (wrapup(&state)) {
		ret = 0;
	}

cleanup2:
	// hack -- remove files if not mmf
	if (!state.is_mmf) {
		DeleteFile(argv[2]);
		_TCHAR ctrlfile[256];
		_tcscpy_s(ctrlfile, argv[2]);
		_tcscat_s(ctrlfile, _T(".ctrl"));
		DeleteFile(ctrlfile);
	}

	free(state.writer_hprocs);
cleanup:
	CloseHandle(state.exit_evt);
	return ret;
}

