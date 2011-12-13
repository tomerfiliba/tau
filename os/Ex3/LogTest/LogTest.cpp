#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "..\LogWriter\ex3_common.h"


typedef struct {
	_TCHAR logfile[256];
	_TCHAR ctrlfile[256];
	int num_of_writers;
	int writer_delay_ms;
	HANDLE * writer_hprocs;
	HANDLE viewer_hproc;

	HANDLE exit_evt;
	HANDLE file_mtx;
	HANDLE can_write_sem;
	HANDLE can_read_sem;
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

bool init_program_state(program_state_t * state, const _TCHAR * logfile, const _TCHAR * ctrlfile, 
						int num_of_writers, int writer_delay_ms)
{
	state->num_of_writers = num_of_writers;
	state->writer_delay_ms = writer_delay_ms;
	state->viewer_hproc = NULL;
	state->writer_hprocs = NULL;
	state->file_mtx = NULL;
	state->can_read_sem = NULL;
	state->can_write_sem = NULL;
	state->exit_evt = NULL;

	_tcscpy_s(state->logfile, logfile);
	_tcscpy_s(state->ctrlfile, ctrlfile);

	state->writer_hprocs = (HANDLE*)malloc(num_of_writers * sizeof(HANDLE));
	if (state->writer_hprocs == NULL) {
		return false;
	}

	state->can_read_sem = CreateSemaphore(NULL, 0, MAX_LOG_RECORDS, EX3_READ_SEM);
	if (state->can_read_sem == NULL) {
		print_last_error(_T("CreateSemaphore"));
		return false;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// don't allow it if the object already exists
		return false;
	}

	state->can_write_sem = CreateSemaphore(NULL, MAX_LOG_RECORDS, MAX_LOG_RECORDS, EX3_WRITE_SEM);
	if (state->can_write_sem == NULL) {
		print_last_error(_T("CreateSemaphore"));
		return false;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// don't allow it if the object already exists
		return false;
	}

	state->exit_evt = CreateEvent(NULL, TRUE, FALSE, EX3_EXIT_EVT);
	if (state->exit_evt == NULL) {
		print_last_error(_T("CreateEvent"));
		return false;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// don't allow it if the object already exists
		return false;
	}

	state->file_mtx = CreateMutex(NULL, FALSE, EX3_FILE_MTX);
	if (state->file_mtx == NULL) {
		print_last_error(_T("CreateMutex"));
		return false;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// don't allow it if the object already exists
		return false;
	}

	// create the log file (truncate if already exists)
	HANDLE hfile = CreateFile(logfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile 1"));
		return false;
	}
	DWORD buffer[MAX_LOG_RECORDS * 4];
	DWORD written;
	memset(buffer, 0, sizeof(buffer));
	if (!WriteFile(hfile, buffer, sizeof(buffer), &written, NULL)) {
		print_last_error(_T("WriteFile 1"));
		return false;
	}
	CloseHandle(hfile);

	// create the ctrl file (truncate if already exists)
	hfile = CreateFile(ctrlfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile 2"));
		return false;
	}
	if (!WriteFile(hfile, buffer, sizeof(DWORD), &written, NULL)) {
		print_last_error(_T("WriteFile 2"));
		return false;
	}
	CloseHandle(hfile);

	return true;
}

void fini_program_state(program_state_t * state)
{
	DeleteFile(state->logfile);
	DeleteFile(state->ctrlfile);

	if (state->writer_hprocs != NULL) {
		free(state->writer_hprocs);
		state->writer_hprocs = NULL;
	}
	if (state->can_read_sem != NULL) {
		CloseHandle(state->can_read_sem);
		state->can_read_sem = NULL;
	}
	if (state->can_write_sem != NULL) {
		CloseHandle(state->can_write_sem);
		state->can_write_sem = NULL;
	}
	if (state->file_mtx != NULL) {
		CloseHandle(state->file_mtx);
		state->file_mtx = NULL;
	}
	if (state->exit_evt != NULL) {
		CloseHandle(state->exit_evt);
		state->exit_evt = NULL;
	}
}

bool startup(program_state_t * state)
{
	_TCHAR cmdline[256];

	_stprintf_s(cmdline, _T("LogViewer.exe %s %s %d"), state->logfile, 
		state->ctrlfile, state->num_of_writers);
	if (!spawn_proc(cmdline, CREATE_NEW_CONSOLE, &state->viewer_hproc)) {
		return false;
	}

	_stprintf_s(cmdline, _T("LogWriter.exe %s %s %d"), state->logfile, 
		state->ctrlfile, state->writer_delay_ms);
	for (int i = 0; i < state->num_of_writers; i++) {
		if (!spawn_proc(cmdline, CREATE_NO_WINDOW, &state->writer_hprocs[i])) {
			// tell everyone who's already started to exit gracefully
			SetEvent(state->exit_evt);
			return false;
		}
	}

	return true;
}

bool wrapup(program_state_t * state)
{
	if (!SetEvent(state->exit_evt)) {
		print_last_error(_T("SetEvent"));
		return false;
	}
	if (WaitForMultipleObjects(state->num_of_writers, state->writer_hprocs, TRUE, INFINITE)) {
		print_last_error(_T("WaitForMultipleObjects"));
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
		_tprintf(_T("Process id %08X, last log entry: %08X\n"), 
			GetProcessId(state->writer_hprocs[i]), ec);
	}

	return true;
}


int _tmain(int argc, _TCHAR* argv[])
{
	int ret = -1;
	program_state_t state;

	if (argc != 6) {
		_tprintf(_T("Usage: %s <num-of-writers> <log-path> <ctrl-path> <writer-delay-ms> <lifespan-sec>\n"), 
			argv[0]);
		return -1;
	}

	int num_of_writers = _tstoi(argv[1]);
	if (num_of_writers <= 0) {
		_tprintf(_T("<num-of-writers> must be a positive integer: '%s'\n"), argv[1]);
		return -1;
	}

	int writer_delay_ms = _tstoi(argv[4]);
	if (writer_delay_ms < 0) {
		_tprintf(_T("<writer-delay-ms> must be a non-negative integer: '%s'\n"), argv[4]);
		return -1;
	}

	int lifespan_sec = _tstoi(argv[5]);
	if (lifespan_sec <= 0) {
		_tprintf(_T("<lifespan-sec> must be a positive integer: '%s'\n"), argv[5]);
		return -1;
	}

	if (!init_program_state(&state, argv[2], argv[3], num_of_writers, writer_delay_ms)) {
		return -1;
	}

	if (!startup(&state)) {
		goto cleanup;
	}
	Sleep(lifespan_sec * 1000);
	wrapup(&state);

cleanup:
	fini_program_state(&state);
	return ret;
}

