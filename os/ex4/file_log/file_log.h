#ifndef EX4_LOGGING_H_INCLUDED
#define EX4_LOGGING_H_INCLUDED

typedef LPVOID LHANDLE;

typedef struct 
{
	DWORD dwTime;
	DWORD dwSQN;
	DWORD dwPID;
	DWORD dwSum;
} LOG_ENTRY;

extern "C"
{
	/*
	This function will be used by writers and readers to initialize MMF or File-based log file
	returns (LHANDLE)-1 on error
	dwTimeout to use for all synchronizations waits
	*/
	LHANDLE __declspec(dllexport) StartLogging(LPCTSTR log_name, DWORD dwLogSize, DWORD dwTimeout);

	/*
	This function is used by writers and the viewer to stop  logging and to free all unneeded resources (handles, memory ,files)
	Accepts handle to a log file previously obtained from StartLogging function
	*/
	VOID __declspec(dllexport) StopLogging(LHANDLE hLog);

	/*
	This function is used by writers to write a log entry
	Returns a sequence number of successfully written log entry
	*/
	DWORD __declspec(dllexport) WriteLogEntry(LHANDLE hLog);

	/*
	This function is used by the viewer to read log entry
	If there is no log entries, the function should return false (does not wait if log file is empty)
	*/
	BOOL __declspec(dllexport) PopLogEntry(LHANDLE hLog, LOG_ENTRY * pLogEntry);

	/*
	This function is used by the viewer process to obtain a handle for a win32 waitable object (e.g. semaphore) 
	that is signaled when there are log entries in the log file
	*/
	HANDLE __declspec(dllexport) GetLogWaitObject(LHANDLE hLog);
}





#endif // EX4_LOGGING_H_INCLUDED
