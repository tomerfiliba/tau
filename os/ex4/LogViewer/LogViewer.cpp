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
	GLHANDLE logger;
	int num_of_writers;
	writer_sequence_t * last_sequences;
} progstate_t;


bool process_log_entry(progstate_t * state, LOG_ENTRY * entry)
{
	// remember last sequence for this pid, and make sure it's incremented by 1 each time
	// so we don't lose any record
	for (int i = 0; i < state->num_of_writers; i++) {
		if (state->last_sequences[i].pid == entry->dwPID) {
			if (record[2] != state->last_sequences[i].last_sequence + 1) {
				_tprintf(_T("ERROR! we missed a record. pid=%d, last known=%d, current=%d\n"),
					entry->dwPID, state->last_sequences[i].last_sequence, entry->dwSQN);
				return false;
			}
			state->last_sequences[i].last_sequence = entry->dwSQN;
			break;
		}
		else if (state->last_sequences[i].pid == -1) {
			state->last_sequences[i].pid = entry->dwPid;
			state->last_sequences[i].last_sequence = entry->dwSQN;
			break;
		}
	}

	_tprintf(_T("Process id 0x%08X produced entry number 0x%08X at time 0x%08X with checksum 0x%08X\n"),
		entry->dwPID, entry->dwSQN, entry->dwTime, entry->dwSum);
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

int consumer_loop(progstate_t * state)
{
	HANDLE sem = GLGetLogWaitObject(logger);
	HANDLE handles[] = {exit_evt, sem};

	while (true) {
		DWORD res = WaitForMultipleObjects(sizeof(handles) / sizeof(handles[0]), handles, FALSE, INFINITE);
		if (res == WAIT_OBJECT_0) {
			// exit_evt
			if (!collect_last_entries(state)) {
				return -1;
			}
			break;
		}
		else if (res == WAIT_OBJECT_0 + 1) {
			LOG_ENTRY entry;
			if (!GLPopLogEntry(handle, &entry)) {
				return -1;
			}
			if (!process_log_entry(state, &entry)) {
				return -1;
			}
		}
		else {
			// some error
			return -1;
		}
	}

	int res = 0;
	for (int i = 0; i < num_of_writers; i++) {
		res ^= state->last_sequences[i].last_sequence;
	}
	return res;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int res = -1;


	return res;
}

