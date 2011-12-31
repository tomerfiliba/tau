#include "../g_log/g_log.h"


typedef struct {
	//glogger_t * parent;
	HANDLE ctrl_file;
	HANDLE log_file;
	DWORD log_sequence;
	DWORD num_of_entries;
	DWORD timeout;
	int next_read_slot;
} file_logger_t;


bool read_at(HANDLE hfile, DWORD pos, void * buf, DWORD count, bool check_read_count)
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
	if (check_read_count && actual != count) {
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
This function is used by writers and the viewer to stop logging and to free all unneeded resources (handles, memory ,files)
Accepts handle to a log file previously obtained from StartLogging function
*/
VOID __declspec(dllexport) StopLogging(LHANDLE hLog)
{
	file_logger_t * logger = (file_logger_t*)hLog;

	if (logger->ctrl_file != INVALID_HANDLE_VALUE) {
		CloseHandle(logger->ctrl_file);
	}
	if (logger->log_file != INVALID_HANDLE_VALUE) {
		CloseHandle(logger->log_file);
	}

	free(logger);
}

/*
This function will be used by writers and readers to initialize MMF or File-based log file
returns (LHANDLE)-1 on error
dwTimeout to use for all synchronizations waits
*/
LHANDLE __declspec(dllexport) StartLogging(LPCTSTR log_name, DWORD dwLogSize, DWORD dwTimeout)
{
	file_logger_t * logger = NULL;
	_TCHAR buf[MAX_PATH];
	logger = (file_logger_t*)malloc(sizeof(file_logger_t));

	if (logger == NULL) {
		return (LHANDLE)-1;
	}

	//logger->parent = NULL;
	logger->ctrl_file = INVALID_HANDLE_VALUE;
	logger->log_file = INVALID_HANDLE_VALUE;
	logger->log_sequence = 0;
	logger->num_of_entries = dwLogSize;
	logger->next_read_slot = 0;
	logger->timeout = dwTimeout;

	logger->log_file = CreateFile(log_name, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL , NULL);
	if (logger->log_file == INVALID_HANDLE_VALUE) {
		//print_last_error(_T("CreateFile 1"));
		goto cleanup;
	}

	_tcscpy_s(buf, log_name);
	_tcscat_s(buf, _T(".ctrl"));
	logger->ctrl_file = CreateFile(buf, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL , NULL);
	if (logger->ctrl_file == INVALID_HANDLE_VALUE) {
		//print_last_error(_T("CreateFile 2"));
		goto cleanup;
	}

	return (LHANDLE)logger;

cleanup:
	StopLogging(logger);
	return (LHANDLE)-1;
}

/*
This function is used by writers to write a log entry
Returns a sequence number of successfully written log entry
*/
DWORD __declspec(dllexport) WriteLogEntry(LHANDLE hLog)
{
	file_logger_t * logger = (file_logger_t*)hLog;
	DWORD slot = 0;
	DWORD record[4];

	record[0] = GetCurrentProcessId();
	record[1] = GetTickCount();
	record[2] = logger->log_sequence;
	record[3] = record[0] ^ record[1] ^ record[2];

	if (!read_at(logger->ctrl_file, 0, &slot, sizeof(slot), false)) {
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
	return logger->log_sequence - 1;
}

/*
This function is used by the viewer to read log entry
If there is no log entries, the function should return false (does not wait if log file is empty)
*/
BOOL __declspec(dllexport) PopLogEntry(LHANDLE hLog, LOG_ENTRY * pLogEntry)
{
	file_logger_t * logger = (file_logger_t*)hLog;
	DWORD record[4];

	if (!read_at(logger->log_file, (logger->next_read_slot % logger->num_of_entries) * sizeof(record),
				record, sizeof(record), true)) {
		return FALSE;
	}
	logger->next_read_slot++;
	pLogEntry->dwPID = record[0];
	pLogEntry->dwTime = record[1];
	pLogEntry->dwSQN = record[2];
	pLogEntry->dwSum = record[3];


	return TRUE;
}


/*
This function is used by the viewer process to obtain a handle for a win32 waitable object (e.g. semaphore) 
that is signaled when there are log entries in the log file
*/
HANDLE __declspec(dllexport) GetLogWaitObject(LHANDLE hLog)
{
	// a generic implementation exists in GLGetLogWaitObject, no need for this func
	return NULL;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

