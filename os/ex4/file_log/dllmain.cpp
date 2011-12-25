#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include "file_log.h"


typedef struct {
	HANDLE ctrl_file;
	HANDLE log_file;
	//HANDLE exit_evt;
	//bool exit_evt_set;
	DWORD timeout;
	HANDLE file_mtx;
	bool file_mtx_owned;
	HANDLE can_write_sem;
	bool discard_last_write;
	HANDLE can_read_sem;
	DWORD log_sequence;
	DWORD num_of_entries;
	int next_read_slot;
} file_logger_t;


bool read_at(HANDLE hfile, DWORD pos, void * buf, DWORD count)
{
	DWORD actual;
	if (SetFilePointer(hfile, pos, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		//print_last_error(_T("SetFilePointer"));
		return false;
	}
	if (!ReadFile(hfile, buf, count, &actual, NULL)) {
		//print_last_error(_T("ReadFile"));
		return false;
	}
	if (actual != count) {
		// oops: actual read count != what we requested
		//_tprintf(_T("read_at: actual != count\n"));
		return false;
	}
	return true;
}

bool write_at(HANDLE hfile, DWORD pos, const void * buf, DWORD count)
{
	DWORD actual;
	if (SetFilePointer(hfile, pos, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		//print_last_error(_T("SetFilePointer"));
		return false;
	}
	if (!WriteFile(hfile, buf, count, &actual, NULL)) {
		//print_last_error(_T("WriteFile"));
		return false;
	}
	if (actual != count) {
		// oops: actual write count != what we requested
		//_tprintf(_T("write_at: actual != count\n"));
		return false;
	}
	return true;
}

/*
This function will be used by writers and readers to initialize MMF or File-based log file
returns (LHANDLE)-1 on error
dwTimeout to use for all synchronizations waits
*/
LHANDLE StartLogging(LPCTSTR log_name, DWORD dwLogSize, DWORD dwTimeout)
{
	file_logger_t * logger = NULL;
	_TCHAR buf[MAX_PATH];
	logger = (file_logger_t*)malloc(sizeof(file_logger_t));

	logger->can_read_sem = NULL;
	logger->can_write_sem = NULL;
	logger->discard_last_write = false;
	logger->file_mtx = NULL;
	logger->file_mtx_owned = false;
	logger->ctrl_file = INVALID_HANDLE_VALUE;
	logger->log_file = INVALID_HANDLE_VALUE;
	logger->log_sequence = 0;
	logger->num_of_entries = dwLogSize;
	logger->timeout = dwTimeout;
	logger->next_read_slot = 0;

	_tcscpy_s(buf, _T("tomerfiliba.ex4.readsem-"));
	_tcscat_s(buf, log_name);
	logger->can_read_sem = CreateSemaphore(NULL, dwLogSize, dwLogSize, buf);
	if (logger->can_read_sem == NULL) {
		//print_last_error(_T("CreateSemaphore 1"));
		return (LHANDLE)-1;
	}

	_tcscpy_s(buf, _T("tomerfiliba.ex4.writesem-"));
	_tcscat_s(buf, log_name);
	logger->can_write_sem = CreateSemaphore(NULL, 0, dwLogSize, buf);
	if (logger->can_write_sem == NULL) {
		//print_last_error(_T("CreateSemaphore 2"));
		return (LHANDLE)-1;
	}

	/*logger->exit_evt = CreateEvent(NULL, TRUE, FALSE, EX3_EXIT_EVT);
	if (logger->exit_evt == NULL) {
		print_last_error(_T("CreateEvent"));
		return (LHANDLE)-1;
	}*/

	_tcscpy_s(buf, _T("tomerfiliba.ex4.filemutex-"));
	_tcscat_s(buf, log_name);
	logger->file_mtx = CreateMutex(NULL, FALSE, buf);
	if (logger->file_mtx == NULL) {
		//print_last_error(_T("CreateMutex"));
		return (LHANDLE)-1;
	}

	logger->log_file = CreateFile(log_name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
	if (logger->log_file == INVALID_HANDLE_VALUE) {
		//print_last_error(_T("CreateFile 1"));
		return (LHANDLE)-1;
	}

	_tcscpy_s(buf, log_name);
	_tcscat_s(buf, _T(".ctrl"));
	logger->ctrl_file = CreateFile(buf, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
	if (logger->ctrl_file == INVALID_HANDLE_VALUE) {
		//print_last_error(_T("CreateFile 2"));
		return (LHANDLE)-1;
	}

	return (LHANDLE)logger;
}

/*
This function is used by writers and the viewer to stop logging and to free all unneeded resources (handles, memory ,files)
Accepts handle to a log file previously obtained from StartLogging function
*/
VOID StopLogging(LHANDLE hLog)
{
	file_logger_t * logger = (file_logger_t*)hLog;

	if (logger->can_read_sem != NULL) {
		CloseHandle(logger->can_read_sem);
		logger->can_read_sem = NULL;
	}
	if (logger->can_write_sem != NULL) {
		if (logger->discard_last_write) {
			// the last write needs to be discarded (otherwise we're 
			// taking a slot for nothing)
			ReleaseSemaphore(logger->can_write_sem, 1, NULL);
			logger->discard_last_write = false;
		}
		CloseHandle(logger->can_write_sem);
		logger->can_write_sem = NULL;
	}
	if (logger->file_mtx != NULL) {
		if (logger->file_mtx_owned) {
			ReleaseMutex(logger->file_mtx);
			logger->file_mtx_owned = false;
		}
		CloseHandle(logger->file_mtx);
		logger->file_mtx = NULL;
	}
	/*if (logger->exit_evt != NULL) {
		CloseHandle(logger->exit_evt);
		logger->exit_evt = NULL;
	}*/
	if (logger->ctrl_file != INVALID_HANDLE_VALUE) {
		CloseHandle(logger->ctrl_file);
		logger->ctrl_file = INVALID_HANDLE_VALUE;
	}
	if (logger->log_file != INVALID_HANDLE_VALUE) {
		CloseHandle(logger->log_file);
		logger->log_file = INVALID_HANDLE_VALUE;
	}

	free(logger);
}

bool write_log_record(file_logger_t * logger)
{
	DWORD slot;
	DWORD record[4];

	record[0] = GetCurrentProcessId();
	record[1] = GetTickCount();
	record[2] = logger->log_sequence;
	record[3] = record[0] ^ record[1] ^ record[2];

	if (!read_at(logger->ctrl_file, 0, &slot, sizeof(slot))) {
		return false;
	}
	if (!write_at(logger->log_file, (slot % logger->num_of_entries) * sizeof(record), record, sizeof(record))) {
		return false;
	}
	
	slot++;
	if (!write_at(logger->ctrl_file, 0, &slot, sizeof(slot))) {
		return false;
	}

	// advance sequence
	logger->log_sequence++;
	return true;
}

/*
This function is used by writers to write a log entry
Returns a sequence number of successfully written log entry
*/
DWORD WriteLogEntry(LHANDLE hLog)
{
	file_logger_t * logger = (file_logger_t*)hLog;

	if (WaitForSingleObject(logger->can_write_sem, logger->timeout) != WAIT_OBJECT_0) {
		return -1;
	}
	// we got the semaphore, meaning we have room in the log file
	// now let's take the file mutex
	logger->discard_last_write = true;
	if (WaitForSingleObject(logger->file_mtx, logger->timeout) != WAIT_OBJECT_0) {
		return -1;
	}
	logger->file_mtx_owned = true;

	// read index from ctrl file, write log and update ctrl file
	if (!write_log_record(logger)) {
		return -1;
	}

	// mark the write as successful
	logger->discard_last_write = false;
	logger->file_mtx_owned = false;
	ReleaseMutex(logger->file_mtx);
	// tell reader it can read now and release mutex
	ReleaseSemaphore(logger->can_read_sem, 1, NULL);

	return logger->log_sequence - 1;
}

/*
This function is used by the viewer to read log entry
If there is no log entries, the function should return false (does not wait if log file is empty)
*/
BOOL PopLogEntry(LHANDLE hLog, LOG_ENTRY * pLogEntry)
{
	file_logger_t * logger = (file_logger_t*)hLog;
	DWORD record[4];

	if (!read_at(logger->log_file, (logger->next_read_slot % logger->num_of_entries) * sizeof(record),
				record, sizeof(record))) {
		return FALSE;
	}
	logger->next_read_slot++;

	return TRUE;
}


/*
This function is used by the viewer process to obtain a handle for a win32 waitable object (e.g. semaphore) 
that is signaled when there are log entries in the log file
*/
HANDLE GetLogWaitObject(LHANDLE hLog)
{
	file_logger_t * logger = (file_logger_t*)hLog;
	return logger->can_read_sem;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

