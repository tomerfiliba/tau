#ifndef GLOG_H_INCLUDED
#define GLOG_H_INCLUDED

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>


typedef LPVOID GLHANDLE;
typedef LPVOID LHANDLE;

typedef struct 
{
	DWORD dwTime;
	DWORD dwSQN;
	DWORD dwPID;
	DWORD dwSum;
} LOG_ENTRY;

typedef LHANDLE (*StartLogging_t)(LPCTSTR, DWORD, DWORD);
typedef VOID (*StopLogging_t)(LHANDLE);
typedef DWORD (*WriteLogEntry_t)(LHANDLE);
typedef BOOL (*PopLogEntry_t)(LHANDLE, LOG_ENTRY*);
typedef HANDLE (*GetLogWaitObject_t)(LHANDLE hLog);

typedef struct {
	DWORD timeout;
	HANDLE file_mtx;
	bool file_mtx_owned;
	HANDLE can_write_sem;
	bool discard_last_write;
	HANDLE can_read_sem;
	DWORD num_of_entries;
	void * specific_logger;

	HMODULE dll;
	StartLogging_t dll_StartLogging;
	StopLogging_t dll_StopLogging;
	WriteLogEntry_t dll_WriteLogEntry;
	PopLogEntry_t dll_PopLogEntry;
	GetLogWaitObject_t dll_GetLogWaitObject;

} glogger_t;

template <size_t SIZE> void make_obj_name(_TCHAR (&buf)[SIZE], const _TCHAR * first, const _TCHAR * second)
{
	_tcscpy_s(buf, first);
	_tcscat_s(buf, second);
	for (_TCHAR * ch = buf; *ch != 0; ch++) {
		if (*ch == '\\' || *ch == '/' || *ch == ':') {
			*ch = '_';
		}
	}
}

extern "C" 
{
	GLHANDLE __declspec(dllexport) GLStartLogging(LPCTSTR log_name, 
		DWORD dwLogSize, DWORD dwTimeout, BOOL bUseMMF);
	
	VOID __declspec(dllexport) GLStopLogging(GLHANDLE hLog);
	
	DWORD __declspec(dllexport) GLWriteLogEntry(GLHANDLE hLog);
	
	BOOL __declspec(dllexport) GLPopLogEntry(GLHANDLE hLog, LOG_ENTRY * pLogEntry);
	
	HANDLE __declspec(dllexport) GLGetLogWaitObject(GLHANDLE hLog);
}


#endif // GLOG_H_INCLUDED
