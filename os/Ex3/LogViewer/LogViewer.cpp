#include <tchar.h>
#include <windows.h>
#include <malloc.h>
#include "..\LogWriter\ex3_common.h"

typedef struct {
	int num_of_writers;
	int * last_sequences;
	int next_read_slot;
	HANDLE ctrl_file;
	HANDLE log_file;
	HANDLE exit_evt;
	bool exit_evt_set;
	HANDLE file_mtx;
	bool file_mtx_owned;
	HANDLE can_write_sem;
	HANDLE can_read_sem;
} program_state_t;


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
		return false;
	}
}

bool read_at(HANDLE hfile, DWORD pos, void * buf, DWORD count)
{
	DWORD actual;
	if (SetFilePointer(hfile, pos, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		return false;
	}
	if (!ReadFile(hfile, buf, count, &actual, NULL)) {
		return false;
	}
	if (actual != count) {
		// oops: actual read count != what we requested
		return false;
	}
	return true;
}

bool write_at(HANDLE hfile, DWORD pos, const void * buf, DWORD count)
{
	DWORD actual;
	if (SetFilePointer(hfile, pos, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		return false;
	}
	if (!WriteFile(hfile, buf, count, &actual, NULL)) {
		return false;
	}
	if (actual != count) {
		// oops: actual write count != what we requested
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
	state->ctrl_file = INVALID_HANDLE_VALUE;
	state->log_file = INVALID_HANDLE_VALUE;

	state->last_sequences = (int*)calloc(num_of_writers, sizeof(int));
	if (state->last_sequences == NULL) {
		return false;
	}

	state->can_read_sem = CreateSemaphore(NULL, 0, 0, EX3_READ_SEM);
	if (state->can_read_sem == NULL) {
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->can_write_sem = CreateSemaphore(NULL, 0, 0, EX3_WRITE_SEM);
	if (state->can_write_sem == NULL) {
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->exit_evt = CreateEvent(NULL, TRUE, FALSE, EX3_EXIT_EVT);
	if (state->exit_evt == NULL) {
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->file_mtx = CreateMutex(NULL, FALSE, EX3_FILE_MTX);
	if (state->file_mtx == NULL) {
		return false;
	}
	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}

	state->log_file = CreateFile(logfile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (state->log_file == INVALID_HANDLE_VALUE) {
		return false;
	}

	/*state->ctrl_file = CreateFile(ctrlfile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (state->ctrl_file == INVALID_HANDLE_VALUE) {
		return false;
	}*/

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
	if (state->ctrl_file != INVALID_HANDLE_VALUE) {
		CloseHandle(state->ctrl_file);
		state->ctrl_file = INVALID_HANDLE_VALUE;
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

	_tprintf(_T("Process id %08X produced entry number %08X at time %08X with checksum %08X"),
		record[0], record[1], record[2], record[3]);

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
	return state->exit_evt_set;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int res = -1;
	program_state_t state;

	if (argc != 4) {
		_tprintf(_T("Error: wrong number of arguments"));
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

	res = calc_XOR_of_last_sequences();

cleanup:
	fini_program_state(&state);
	return res;
}
