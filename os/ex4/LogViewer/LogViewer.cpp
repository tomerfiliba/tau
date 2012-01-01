#include <windows.h>
#include <tchar.h>
#include "../g_log/g_log.h"


typedef struct {
	int pid;
	int last_sequence;
} writer_sequence_t;

typedef struct
{
	HANDLE exit_evt;
	DWORD timeout;
	GLHANDLE hlog;
	int num_of_writers;
	DWORD num_of_entries;
	writer_sequence_t * last_sequences;
} program_state_t;


bool process_log_entry(program_state_t * state)
{
	LOG_ENTRY entry;
	if (!GLPopLogEntry(state->hlog, &entry)) {
		return false;
	}

	// remember last sequence for this pid, and make sure it's incremented by 1 each time
	// so we don't lose any record
	for (int i = 0; i < state->num_of_writers; i++) {
		if (state->last_sequences[i].pid == entry.dwPID) {
			if (entry.dwSQN != state->last_sequences[i].last_sequence + 1) {
				_tprintf(_T("ERROR! we missed a record. pid=%d, last known=%d, current=%d\n"),
					entry.dwPID, state->last_sequences[i].last_sequence, entry.dwSQN);
				return false;
			}
			state->last_sequences[i].last_sequence = entry.dwSQN;
			break;
		}
		else if (state->last_sequences[i].pid == -1) {
			state->last_sequences[i].pid = entry.dwPID;
			state->last_sequences[i].last_sequence = entry.dwSQN;
			break;
		}
	}

	_tprintf(_T("Process id 0x%08X produced entry number 0x%08X at time 0x%08X with checksum 0x%08X\n"),
		entry.dwPID, entry.dwSQN, entry.dwTime, entry.dwSum);
	return true;
}

int collect_last_entries(program_state_t * state)
{
	// wait a little for all processes to die and get the file mutex 
	Sleep(1500);

	// read all entries as long as the semaphore allows
	while (WaitForSingleObject(GLGetLogWaitObject(state->hlog), 0) == WAIT_OBJECT_0) {
		if (!process_log_entry(state)) {
			return -1;
		}
	}

	// xor the last sequences
	int res = 0;
	for (int i = 0; i < state->num_of_writers; i++) {
		res ^= state->last_sequences[i].last_sequence;
	}
	return res;
}

int consumer_loop(program_state_t * state)
{
	HANDLE sem = GLGetLogWaitObject(state->hlog);
	HANDLE handles[] = {state->exit_evt, sem};

	while (true) {
		DWORD res = WaitForMultipleObjects(sizeof(handles) / sizeof(handles[0]), handles, FALSE, INFINITE);
		if (res == WAIT_OBJECT_0) {
			// exit_evt
			return collect_last_entries(state);
		}
		else if (res == WAIT_OBJECT_0 + 1) {
			// the semaphore
			if (!process_log_entry(state)) {
				return -1;
			}
		}
		else {
			// some other error
			return -1;
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	int res = -1;
	program_state_t state;

	if (argc != 6) {
		_tprintf(_T("Usage: %s <logname> <num-of-writers> <logsize> <timeout> <mmf 0/1>\n"));
	}
	state.num_of_writers = _tstoi(argv[2]);
	if (state.num_of_writers <= 0) {
		return -1;
	}
	state.num_of_entries = _tstoi(argv[3]);
	if (state.num_of_entries <= 0) {
		return -1;
	}
	state.timeout = _tstoi(argv[4]);
	if (state.timeout <= 0) {
		return -1;
	}
	bool mmf = _tcscmp(argv[5], _T("1")) == 0;

	_TCHAR buf[MAX_PATH];
	make_obj_name(buf, _T("tomerfiliba.ex4.exit_evt-"), argv[1]);

	state.exit_evt = CreateEvent(NULL, TRUE, FALSE, buf);
	if (state.exit_evt == NULL) {
		return -1;
	}

	state.last_sequences = (writer_sequence_t*)malloc(state.num_of_writers * sizeof(writer_sequence_t));
	if (state.last_sequences == NULL) {
		goto cleanup;
	}
	for (int i = 0; i < state.num_of_writers; i++) {
		state.last_sequences[i].pid = -1;
		state.last_sequences[i].last_sequence = 0;
	}

	state.hlog = GLStartLogging(argv[1], state.num_of_entries, state.timeout, mmf);
	if (state.hlog == (GLHANDLE)-1) {
		goto cleanup2;
	}
	res = consumer_loop(&state);
	GLStopLogging(state.hlog);
cleanup2:
	free(state.last_sequences);
cleanup:
	CloseHandle(state.exit_evt);
	return res;
}

