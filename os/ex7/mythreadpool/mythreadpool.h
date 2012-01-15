#ifndef mythreadpool_H_INCLUDED
#define mythreadpool_H_INCLUDED

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>


typedef VOID * TPHANDLE;
typedef DWORD (*LPTHREAD_FUNC)(BYTE* pParam, DWORD param_size);
typedef DWORD (*LPCLBCK_FUNC)(BYTE* pParam, DWORD param_size, DWORD dwExitCode);

typedef TPHANDLE (*StartThreadPool_t)(DWORD dwNumThreads, DWORD dwQueueSize, DWORD dwMaxParamSize);
typedef BOOL (*StopThreadPool_t)(TPHANDLE hThreadPool, DWORD dwTimeout);
typedef BOOL (*QueueWorkItem_t)(TPHANDLE hThreadPool, LPTHREAD_FUNC lpfnThreadDunc, 
								BYTE * pParam, DWORD dwParamSize, LPCLBCK_FUNC lpfnCallback);

#ifndef mythreadpool_DLL_ITSELF
extern "C" {
	TPHANDLE __declspec(dllimport) StartThreadPool(DWORD dwNumThreads, DWORD dwQueueSize, DWORD dwMaxParamSize);
	BOOL __declspec(dllimport) StopThreadPool(TPHANDLE hThreadPool, DWORD dwTimeout);
	BOOL __declspec(dllimport) QueueWorkItem(TPHANDLE hThreadPool, LPTHREAD_FUNC lpfnThreadDunc, 
											 BYTE * pParam, DWORD dwParamSize, LPCLBCK_FUNC lpfnCallback);
}
#endif

#endif // mythreadpool_H_INCLUDED
