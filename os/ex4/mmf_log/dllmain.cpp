#include "../g_log/g_log.h"

typedef struct {
	int next_write_index;
	int next_read_index;
} log_control_t;

typedef struct {
	HANDLE hmmf;
	void * pview;
	LOG_ENTRY * entries;          // resides on pview
	log_control_t * ctrl;         // resides on pview

	DWORD log_sequence;
	DWORD num_of_entries;
	DWORD timeout;
} mmf_logger_t;

extern "C" {
	VOID __declspec(dllexport) StopLogging(LHANDLE hLog);
	LHANDLE __declspec(dllexport) StartLogging(LPCTSTR log_name, DWORD dwLogSize, DWORD dwTimeout);
	DWORD __declspec(dllexport) WriteLogEntry(LHANDLE hLog);
	BOOL __declspec(dllexport) PopLogEntry(LHANDLE hLog, LOG_ENTRY * pLogEntry);
	HANDLE __declspec(dllexport) GetLogWaitObject(LHANDLE hLog);
}


/*
This function is used by writers and the viewer to stop logging and to free all unneeded resources (handles, memory ,files)
Accepts handle to a log file previously obtained from StartLogging function
*/
VOID StopLogging(LHANDLE hLog)
{
	mmf_logger_t * logger = (mmf_logger_t*)hLog;

	if (logger->pview != NULL) {
		UnmapViewOfFile(logger->pview);
	}
	if (logger->hmmf != NULL) {
		CloseHandle(logger->hmmf);
	}
	free(logger);
}

/*
This function will be used by writers and readers to initialize MMF or File-based log file
returns (LHANDLE)-1 on error
dwTimeout to use for all synchronizations waits
*/
LHANDLE StartLogging(LPCTSTR log_name, DWORD dwLogSize, DWORD dwTimeout)
{
	mmf_logger_t * logger = NULL;
	_TCHAR buf[MAX_PATH];
	logger = (mmf_logger_t*)malloc(sizeof(mmf_logger_t));

	if (logger == NULL) {
		return (LHANDLE)-1;
	}

	//logger->parent = NULL;
	logger->hmmf = NULL;
	logger->pview = NULL;
	logger->entries = NULL;
	logger->ctrl = NULL;

	logger->log_sequence = 0;
	logger->num_of_entries = dwLogSize;
	logger->timeout = dwTimeout;

	make_obj_name(buf, _T("tomerfiliba.ex4.first-one-evt-"), log_name);
	HANDLE first_evt = CreateEvent(NULL, TRUE, FALSE, buf);

	DWORD mapsize = sizeof(log_control_t) + sizeof(LOG_ENTRY) * dwLogSize;

	if (first_evt == NULL) {
		goto cleanup;
	}
	bool first_mapping = false;
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		WaitForSingleObject(first_evt, INFINITE);
		CloseHandle(first_evt);
	}
	else {
		first_mapping = true;
	}

	make_obj_name(buf, _T("tomerfiliba.ex4.mapping-"), log_name);
	logger->hmmf = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, mapsize, buf);
	if (logger->hmmf == NULL) {
		goto cleanup;
	}

	logger->pview = MapViewOfFile(logger->hmmf, FILE_MAP_WRITE, 0, 0, mapsize);
	if (logger->pview == NULL) {
		goto cleanup;
	}
	if (first_mapping) {
		ZeroMemory(logger->pview, mapsize);
		SetEvent(first_evt);
	}

	logger->ctrl = (log_control_t*)logger->pview;
	logger->entries = (LOG_ENTRY*)((char*)logger->pview + sizeof(log_control_t));

	return logger;

cleanup:
	StopLogging(logger);
	return (LHANDLE)-1;
}

/*
This function is used by writers to write a log entry
Returns a sequence number of successfully written log entry
*/
DWORD WriteLogEntry(LHANDLE hLog)
{
	mmf_logger_t * logger = (mmf_logger_t*)hLog;
	LOG_ENTRY entry;

	entry.dwPID = GetCurrentProcessId();
	entry.dwTime = GetTickCount();
	entry.dwSQN = logger->log_sequence;
	entry.dwSum = entry.dwPID ^ entry.dwTime ^ entry.dwSQN;

	int slot = logger->ctrl->next_write_index % logger->num_of_entries;
	memcpy(&logger->entries[slot], &entry, sizeof(LOG_ENTRY));
	logger->ctrl->next_write_index++;

	// advance sequence
	logger->log_sequence++;
	return logger->log_sequence - 1;
}

/*
This function is used by the viewer to read log entry
If there is no log entries, the function should return false (does not wait if log file is empty)
*/
BOOL PopLogEntry(LHANDLE hLog, LOG_ENTRY * pLogEntry)
{
	// we are only called after the read semaphore has been taken -- 
	// so there surely is something for us to read
	mmf_logger_t * logger = (mmf_logger_t*)hLog;
	int slot = logger->ctrl->next_read_index % logger->num_of_entries;

	memcpy(pLogEntry, &logger->entries[slot], sizeof(LOG_ENTRY));
	logger->ctrl->next_read_index++;
	return TRUE;
}

/*
This function is used by the viewer process to obtain a handle for a win32 waitable object (e.g. semaphore) 
that is signaled when there are log entries in the log file
*/
HANDLE GetLogWaitObject(LHANDLE hLog)
{
	// a generic implementation exists in GLGetLogWaitObject, no need for this func
	return NULL;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

