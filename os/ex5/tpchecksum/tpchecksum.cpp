#include <windows.h>
#include <tchar.h>
#include "../mythreadpool/mythreadpool.h"

typedef struct {
	HMODULE dll;
	StartThreadPool_t start_func;
	StopThreadPool_t stop_func;
	QueueWorkItem_t enqueue_func;
	TCHAR filename[256];
	DWORD filesize;
	HANDLE completion_sem;
	int num_of_threads;
	int num_of_chunks;
	DWORD chunk_size;
	unsigned char checksum;
} program_state_t;


void print_last_error(const _TCHAR * text)
{
	void * msg = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&msg, 0, NULL);

	if (text != NULL && text[0] != '\0') {
		_tprintf(_T("%s: %s\n"), text, (LPTSTR)msg);
	}
	else {
		_tprintf(_T("%s\n"), (LPTSTR)msg);
	}
	LocalFree(msg);
}

bool init_state(program_state_t * state)
{
	state->dll = NULL;
	state->completion_sem = NULL;
	state->checksum = 0;

	state->dll = LoadLibrary(_T("mythreadpool.dll"));
	if (state->dll == NULL) {
		print_last_error(_T("LoadLibrary"));
		return false;
	}
	state->start_func = (StartThreadPool_t)GetProcAddress(state->dll, "StartThreadPool");
	if (state->start_func == NULL) {
		print_last_error(_T("GetProcAddress: StartThreadPool"));
		return false;
	}
	state->stop_func = (StopThreadPool_t)GetProcAddress(state->dll, "StopThreadPool");
	if (state->stop_func == NULL) {
		print_last_error(_T("GetProcAddress: StopThreadPool"));
		return false;
	}
	state->enqueue_func = (QueueWorkItem_t)GetProcAddress(state->dll, "QueueWorkItem");
	if (state->enqueue_func == NULL) {
		print_last_error(_T("GetProcAddress: QueueWorkItem"));
		return false;
	}

	HANDLE hfile = CreateFile(state->filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile"));
		return false;
	}
	state->filesize = GetFileSize(hfile, NULL);
	CloseHandle(hfile);

	state->num_of_chunks = state->filesize / state->chunk_size;
	if (state->filesize % state->chunk_size != 0) {
		// one more chunk for the overflow
		state->num_of_chunks++;
	}

	state->completion_sem = CreateSemaphore(NULL, 0, state->num_of_chunks, NULL);
	if (state->completion_sem == NULL) {
		print_last_error(_T("CreateSemaphore: completetion_sem"));
		return false;
	}

	return true;
}

void fini_state(program_state_t * state)
{
	if (state->dll != NULL) {
		FreeLibrary(state->dll);
		state->dll = NULL;
	}
	if (state->completion_sem != NULL) {
		CloseHandle(state->completion_sem);
		state->completion_sem = NULL;
	}
}

unsigned char xor_buffer(unsigned char * buf, size_t size)
{
	unsigned char xor = 0;
	for (size_t i = 0; i < size; i++) {
		xor ^= buf[i];
	}
	return xor;
}

typedef struct
{
	TCHAR * filename;
	int offset;
	int size;
	bool succ;
	HANDLE completion_sem;
	unsigned char * pChecksum;
} queue_func_params_t;


DWORD queue_func(BYTE* pParam, DWORD param_size)
{
	queue_func_params_t * params = (queue_func_params_t*)pParam;
	unsigned char checksum = 0;
	unsigned char buffer[16 * 1024];
	params->succ = false;
	DWORD ret = -1;

	// we must open the file each time, because we need separate file pointers
	HANDLE hfile = CreateFile(params->filename, GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile"));
		return -1;
	}
	// seek to offset
	if (SetFilePointer(hfile, params->offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		print_last_error(_T("SetFilePointer"));
		goto cleanup;
	}
	// read the file in chunks and compute the checksum
	for (int i = 0; i < params->size; ) {
		DWORD readcount;
		if (!ReadFile(hfile, buffer, sizeof(buffer), &readcount, NULL)) {
			print_last_error(_T("ReadFile"));
			goto cleanup;
		}
		if (readcount <= 0) {
			_tprintf(_T("read count is not > 0\n"));
			goto cleanup;
		}
		i += readcount;
		checksum ^= xor_buffer(buffer, readcount);
	}
	params->succ = true;
	ret = checksum;

cleanup:
	CloseHandle(hfile);
	return ret;
}

DWORD queue_callback_func(BYTE* pParam, DWORD param_size, DWORD dwExitCode)
{
	queue_func_params_t * params = (queue_func_params_t*)pParam;
	if (params->succ) {
		_tprintf(_T("Byte Range from %d till %d has checksum %d\n"),
			params->offset, params->offset + params->size, dwExitCode);
		InterlockedXor8(params->pChecksum, (char)dwExitCode);
	}
	else {
		_tprintf(_T("Error computing checksum for range %d-%d\n"),
			params->offset, params->offset + params->size);
	}
	ReleaseSemaphore(params->completion_sem, 1, NULL);
	return 0;
}

bool compute_checksums(program_state_t * state)
{
	TPHANDLE tpool = state->start_func(state->num_of_threads, state->num_of_chunks, 
		sizeof(queue_func_params_t));
	bool succ = false;

	if (tpool == (TPHANDLE)-1) {
		_tprintf(_T("StartThreadPool failed\n"));
		return false;
	}

	// enqueue all tasks
	DWORD offset = 0;
	for (int i = 0; i < state->num_of_chunks; i++) {
		queue_func_params_t params;
		params.completion_sem = state->completion_sem;
		params.succ = false;
		params.pChecksum = &state->checksum;
		params.filename = state->filename;
		params.offset = offset;
		DWORD remaining = state->filesize - offset;
		params.size = remaining > state->chunk_size ? state->chunk_size : remaining;
		offset += params.size;

		if (!state->enqueue_func(tpool, queue_func, (BYTE*)&params, sizeof(params), queue_callback_func)) {
			_tprintf(_T("QueueWorkItem failed\n"));
			goto cleanup;
		}
	}

	// wait for all of the tasks to finish
	for (int i = 0; i < state->num_of_chunks; i++) {
		if (WaitForSingleObject(state->completion_sem, INFINITE) != WAIT_OBJECT_0) {
			print_last_error(_T("WaitForSingleObject: completion_sem"));
			goto cleanup;
		}
	}

	succ = true;

cleanup:
	state->stop_func(tpool, 1000);
	return succ;
}

int _tmain(int argc, TCHAR *argv[])
{
	int res = -1;
	program_state_t state;

	if (argc != 4) {
		_tprintf(_T("Usage: %s <file> <num of threads> <max chunk size (bytes)>\n"), argv[0]);
		return -1;
	}
	_tcscpy_s(state.filename, argv[1]);
	state.num_of_threads = _tstoi(argv[2]);
	state.chunk_size = _tstoi(argv[3]);

	if (!init_state(&state)) {
		goto cleanup;
	}
	if (compute_checksums(&state)) {
		res = 0;
	}

cleanup:
	fini_state(&state);
	return res;
}