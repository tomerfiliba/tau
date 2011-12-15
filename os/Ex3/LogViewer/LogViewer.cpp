#include <tchar.h>
#include <windows.h>
#include <malloc.h>
#include "..\LogWriter\ex3_common.h"


typedef struct {
	int pid;
	int last_sequence;
} writer_sequence_t;

typedef struct {
	int num_of_writers;
	writer_sequence_t * last_sequences;
	int next_read_slot;
	HANDLE log_file;
	HANDLE exit_evt;
	bool exit_evt_set;
	HANDLE file_mtx;
	bool file_mtx_owned;
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

bool wait_for_object(program_state_t * state, HANDLE hobj)
{
	HANDLE waitees[] = {state->exit_evt, hobj};
	DWORD res = WaitForMultipleObjects(sizeof(waitees) / sizeof(waitees[0]), waitees, 
		FALSE, INFINITE);
	if (res == WAIT_OBJECT_0) {
		state->exit_evt_set = true;
		return false;
	}
	else if (res == WAIT_OBJECT_0 + 1) {
		return true;
	}
	else {
		print_last_error(_T("WaitForMultipleObjects"));
		return false;
	}
}

bool read_at(HANDLE hfile, DWORD pos, void * buf, DWORD count)
{
	DWORD actual;
	if (SetFilePointer(hfile, pos, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		print_last_error(_T("SetFilePointer"));
		return false;
	}
	if (!ReadFile(hfile, buf, count, &actual, NULL)) {
		print_last_error(_T("ReadFile"));
		return false;
	}
	if (actual != count) {
		// oops: actual read count != what we requested
		return false;
	}
	return true;
}

bool init_program_state(program_state_t * state, _TCHAR * logfile, int num_of_writers)
{
	state->num_of_writers = num_of_writers;
	state->last_sequences = NULL;
	state->next_read_slot = 0;
	state->can_read_sem = NULL;
	state->can_write_sem = NULL;
	state->exit_evt = NULL;
	state->exit_evt_set = false;
	state->file_mtx = NULL;
	state->file_mtx_owned = false;
	state->log_file = INVALID_HANDLE_VALUE;

	state->last_sequences = (writer_sequence_t*)malloc(num_of_writers * sizeof(writer_sequence_t));
	if (state->last_sequences == NULL) {
		_tprintf(_T("malloc failed"));
		return false;
	}
	for (int i = 0; i < num_of_writers; i++) {
		state->last_sequences[i].pid = -1;
		state->last_sequences[i].last_sequence = 0;
	}

	state->can_read_sem = CreateSemaphore(NULL, 0, MAX_LOG_RECORDS, EX3_READ_SEM);
	if (state->can_read_sem == NULL) {
		print_last_error(_T("[v1] CreateSemaphore"));
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->can_write_sem = CreateSemaphore(NULL, 0, MAX_LOG_RECORDS, EX3_WRITE_SEM);
	if (state->can_write_sem == NULL) {
		print_last_error(_T("[v2] CreateSemaphore"));
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->exit_evt = CreateEvent(NULL, TRUE, FALSE, EX3_EXIT_EVT);
	if (state->exit_evt == NULL) {
		print_last_error(_T("CreateEvent"));
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->file_mtx = CreateMutex(NULL, FALSE, EX3_FILE_MTX);
	if (state->file_mtx == NULL) {
		print_last_error(_T("CreateMutex"));
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->log_file = CreateFile(logfile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (state->log_file == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile"));
		return false;
	}

	return true;
}

void fini_program_state(program_state_t * state)
{
	if (state->last_sequences != NULL) {
		free(state->last_sequences);
		state->last_sequences = NULL;
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
		if (state->file_mtx_owned) {
			ReleaseMutex(state->file_mtx);
			state->file_mtx_owned = false;
		}
		CloseHandle(state->file_mtx);
		state->file_mtx = NULL;
	}
	if (state->exit_evt != NULL) {
		CloseHandle(state->exit_evt);
		state->exit_evt = NULL;
	}
	if (state->log_file != INVALID_HANDLE_VALUE) {
		CloseHandle(state->log_file);
		state->log_file = INVALID_HANDLE_VALUE;
	}
}

bool read_log_entry(program_state_t * state)
{
	DWORD record[4];

	if (!read_at(state->log_file, (state->next_read_slot % MAX_LOG_RECORDS) * sizeof(record),
				record, sizeof(record))) {
		return false;
	}
	state->next_read_slot++;

	// remember last sequence for this pid, and make sure it's incremented by 1 each time
	// so we don't lose any record
	for (int i = 0; i < state->num_of_writers; i++) {
		if (state->last_sequences[i].pid == record[0]) {
			if (record[2] != state->last_sequences[i].last_sequence + 1) {
				_tprintf(_T("ERROR! we missed a record. pid=%d, last known=%d, current=%d\n"),
						record[0], state->last_sequences[i].last_sequence, record[2]);
				return false;
			}
			state->last_sequences[i].last_sequence = record[2];
			break;
		}
		else if (state->last_sequences[i].pid == -1) {
			state->last_sequences[i].pid = record[0];
			state->last_sequences[i].last_sequence = record[2];
			break;
		}
	}

	_tprintf(_T("Process id 0x%08X produced entry number 0x%08X at time 0x%08X with checksum 0x%08X\n"),
		record[0], record[2], record[1], record[3]);

	return true;
}

bool collect_last_entries(program_state_t * state)
{
	// wait a little for all processes to die and get the file mutex 
	Sleep(1000);
	WaitForSingleObject(state->file_mtx, INFINITE);
	state->file_mtx_owned = true;

	// read all entries as long as the semaphore allows
	while (WaitForSingleObject(state->can_read_sem, 0) == WAIT_OBJECT_0) {
		if (!read_log_entry(state)) {
			return false;
		}
	}
	return true;
}

bool consumer_loop(program_state_t * state)
{
	while (true) {
		if (!wait_for_object(state, state->can_read_sem)) {
			break;
		}
		if (!wait_for_object(state, state->file_mtx)) {
			break;
		}
		state->file_mtx_owned = true;
		if (!read_log_entry(state)) {
			break;
		}

		state->file_mtx_owned = false;
		ReleaseMutex(state->file_mtx);
		// tell the writers that we've freed one write slot
		ReleaseSemaphore(state->can_write_sem, 1, NULL);
	}

	if (state->exit_evt_set) {
		return collect_last_entries(state);
	}
	else {
		return false;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	int res = -1;
	program_state_t state;

	if (argc != 4) {
		_tprintf(_T("Error: wrong number of arguments\n"));
		return -1;
	}

	int num_of_writers = _tstoi(argv[3]);
	if (num_of_writers <= 0) {
		return -1;
	}

	// we don't use the ctrl file here (argv[2])

	if (!init_program_state(&state, argv[1], num_of_writers)) {
		goto cleanup;
	}
	if (!consumer_loop(&state)) {
		goto cleanup;
	}

	// calc xor of last sequence numbers
	res = 0;
	for (int i = 0; i < num_of_writers; i++) {
		res ^= state.last_sequences[i].last_sequence;
	}

cleanup:
	fini_program_state(&state);
	return res;
}
