#include "g_log.h"


/*
This function is used by writers and the viewer to stop logging and to free all unneeded resources (handles, memory ,files)
Accepts handle to a log file previously obtained from StartLogging function
*/
VOID GLStopLogging(GLHANDLE hLog)
{
	glogger_t * logger = (glogger_t*)hLog;

	if (logger->specific_logger != NULL) {
		logger->dll_StopLogging(logger->specific_logger);
	}
	if (logger->can_read_sem != NULL) {
		CloseHandle(logger->can_read_sem);
	}
	if (logger->can_write_sem != NULL) {
		if (logger->discard_last_write) {
			// the last write needs to be discarded (otherwise we're 
			// taking a slot for nothing)
			ReleaseSemaphore(logger->can_write_sem, 1, NULL);
		}
		CloseHandle(logger->can_write_sem);
	}
	if (logger->file_mtx != NULL) {
		if (logger->file_mtx_owned) {
			ReleaseMutex(logger->file_mtx);
		}
		CloseHandle(logger->file_mtx);
	}
	if (logger->dll != NULL) {
		FreeLibrary(logger->dll);
	}

	free(logger);
}


/*
This function will be used by writers and readers to initialize MMF or File-based log file
returns (LHANDLE)-1 on error
dwTimeout to use for all synchronizations waits
*/
GLHANDLE GLStartLogging(LPCTSTR log_name, DWORD dwLogSize, DWORD dwTimeout, BOOL bUseMMF)
{
	glogger_t * logger = NULL;
	_TCHAR buf[MAX_PATH];
	logger = (glogger_t*)malloc(sizeof(glogger_t));

	if (logger == NULL) {
		return (GLHANDLE)-1;
	}
	logger->can_read_sem = NULL;
	logger->can_write_sem = NULL;
	logger->discard_last_write = false;
	logger->file_mtx = NULL;
	logger->file_mtx_owned = false;
	logger->num_of_entries = dwLogSize;
	logger->timeout = dwTimeout;
	logger->specific_logger = NULL;
	logger->dll = NULL;
	logger->dll_StartLogging = NULL;
	logger->dll_GetLogWaitObject = NULL;
	logger->dll_StopLogging = NULL;
	logger->dll_PopLogEntry = NULL;
	logger->dll_WriteLogEntry = NULL;

	//
	// load DLL and functions
	//
	if (bUseMMF) {
		logger->dll = LoadLibrary(_T("mmf_log.dll"));
	}
	else {
		logger->dll = LoadLibrary(_T("file_log.dll"));
	}
	if (logger->dll == NULL) {
		goto cleanup;
	}
	logger->dll_StartLogging = (StartLogging_t)GetProcAddress(logger->dll, "StartLogging");
	if (logger->dll_StartLogging == NULL) {
		goto cleanup;
	}
	logger->dll_GetLogWaitObject = (GetLogWaitObject_t)GetProcAddress(logger->dll, "GetLogWaitObject");
	if (logger->dll_GetLogWaitObject == NULL) {
		goto cleanup;
	}
	logger->dll_StopLogging = (StopLogging_t)GetProcAddress(logger->dll, "StopLogging");
	if (logger->dll_StopLogging == NULL) {
		goto cleanup;
	}
	logger->dll_PopLogEntry = (PopLogEntry_t)GetProcAddress(logger->dll, "PopLogEntry");
	if (logger->dll_PopLogEntry == NULL) {
		goto cleanup;
	}
	logger->dll_WriteLogEntry = (WriteLogEntry_t)GetProcAddress(logger->dll, "WriteLogEntry");
	if (logger->dll_WriteLogEntry == NULL) {
		goto cleanup;
	}

	//
	// create semaphores, etc.
	//
	make_obj_name(buf, _T("tomerfiliba.ex4.readsem-"), log_name);
	logger->can_read_sem = CreateSemaphore(NULL, dwLogSize, dwLogSize, buf);
	if (logger->can_read_sem == NULL) {
		//print_last_error(_T("CreateSemaphore 1"));
		goto cleanup;
	}

	make_obj_name(buf, _T("tomerfiliba.ex4.writesem-"), log_name);
	logger->can_write_sem = CreateSemaphore(NULL, 0, dwLogSize, buf);
	if (logger->can_write_sem == NULL) {
		//print_last_error(_T("CreateSemaphore 2"));
		goto cleanup;
	}

	make_obj_name(buf, _T("tomerfiliba.ex4.fielmtx-"), log_name);
	logger->file_mtx = CreateMutex(NULL, FALSE, buf);
	if (logger->file_mtx == NULL) {
		//print_last_error(_T("CreateMutex"));
		goto cleanup;
	}

	logger->specific_logger = logger->dll_StartLogging(log_name, dwLogSize, dwTimeout);
	if (logger->specific_logger == NULL) {
		goto cleanup;
	}
	// i need to pass this to the specific logger somehow... wtf do you specify the
	// signature of the implementation-specific DLL function? let me specify it.
	// arrrrghh.
	//((specific_logger_t*)logger->specific_logger)->parent = logger;

	return logger;

cleanup:
	GLStopLogging(logger);
	return (GLHANDLE)-1;
}

/*
This function is used by writers to write a log entry
Returns a sequence number of successfully written log entry
*/
DWORD GLWriteLogEntry(GLHANDLE hLog)
{
	glogger_t * logger = (glogger_t*)hLog;

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

	DWORD seq = logger->dll_WriteLogEntry(logger->specific_logger);
	if (seq == -1) {
		return -1;
	}

	// mark the write as successful
	logger->discard_last_write = false;
	logger->file_mtx_owned = false;
	ReleaseMutex(logger->file_mtx);
	// tell reader it can read now and release mutex
	ReleaseSemaphore(logger->can_read_sem, 1, NULL);

	return seq;
}

/*
This function is used by the viewer to read log entry
If there is no log entries, the function should return false (does not wait if log file is empty)
*/
BOOL GLPopLogEntry(GLHANDLE hLog, LOG_ENTRY * pLogEntry)
{
	glogger_t * logger = (glogger_t*)hLog;

	if (WaitForSingleObject(logger->file_mtx, logger->timeout) != WAIT_OBJECT_0) {
		return FALSE;
	}
	logger->file_mtx_owned = true;
	
	BOOL succ = logger->dll_PopLogEntry(logger->specific_logger, pLogEntry);

	logger->file_mtx_owned = false;
	ReleaseMutex(logger->file_mtx);

	if (succ) {
		ReleaseSemaphore(state->can_write_sem, 1, NULL);
	}
	return succ;
}

/*
This function is used by the viewer process to obtain a handle for a win32 waitable object (e.g. semaphore) 
that is signaled when there are log entries in the log file
*/
HANDLE GLGetLogWaitObject(GLHANDLE hLog)
{
	glogger_t * logger = (glogger_t*)hLog;
	return logger->can_read_sem;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	return TRUE;
}

