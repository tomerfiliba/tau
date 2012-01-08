#include "mythreadpool.h"


extern "C" 
{
	TPHANDLE __declspec(dllexport) StartThreadPool(DWORD dwNumThreads, DWORD dwQueueSize, DWORD dwMaxParamSize);
	BOOL __declspec(dllexport) StopThreadPool(TPHANDLE hThreadPool, DWORD dwTimeout);
	BOOL __declspec(dllexport) QueueWorkItem(TPHANDLE hThreadPool, LPTHREAD_FUNC lpfnThreadDunc, 
											 BYTE * pParam, DWORD dwParamSize, LPCLBCK_FUNC lpfnCallback);
}

typedef struct {
	LPTHREAD_FUNC lpfnThreadFunc;
	BYTE * pParam;
	DWORD dwParamSize;
	LPCLBCK_FUNC lpfnCallback;
} queue_item_t;

typedef struct {
	int num_of_threads;
	HANDLE * hthreads;
	int queue_size;
	int read_index;
	int write_index;
	int queue_count;
	DWORD max_param_size;
	queue_item_t * queue;
	HANDLE queue_mtx;
	HANDLE work_count_sem;
	HANDLE stop_evt;
} threadpool_t;


void handle_queue_item(threadpool_t * tpool, void * thread_local_buffer)
{
	// we have the queue mutex and we have the semaphore (e.g., we have work)
	// copy the item out of the queue and let others use it
	queue_item_t item = tpool->queue[tpool->read_index];
	tpool->read_index = (tpool->read_index + 1) % tpool->queue_size;
	tpool->queue_count--;
	memcpy(thread_local_buffer, item.pParam, item.dwParamSize);
	// let others use the queue now
	ReleaseMutex(tpool->queue_mtx);

	DWORD exitcode = item.lpfnThreadFunc(item.pParam, item.dwParamSize);
	(void)item.lpfnCallback(item.pParam, item.dwParamSize, exitcode);
}

DWORD WINAPI WorkerThreadProc(LPVOID lpParameter)
{
	threadpool_t * tpool = (threadpool_t*)lpParameter;
	HANDLE first_handles[2] = {tpool->work_count_sem, tpool->stop_evt};
	HANDLE second_handles[2] = {tpool->queue_mtx, tpool->stop_evt};
	void * thread_local_buffer = malloc(tpool->max_param_size);
	if (thread_local_buffer == NULL) {
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	DWORD retval = 0;
	DWORD res;

	while (true) {
		res = WaitForMultipleObjects(sizeof(first_handles) / sizeof(first_handles[0]), first_handles, FALSE, INFINITE);
		if (res == WAIT_OBJECT_0) {
			// has work item
			res = WaitForMultipleObjects(sizeof(second_handles) / sizeof(second_handles[0]), second_handles, FALSE, INFINITE);
			if (res == WAIT_OBJECT_0) {
				// got queue mutex
				handle_queue_item(tpool, thread_local_buffer);
			}
			else if (res == WAIT_OBJECT_0 + 1) {
				// exit event 
				retval = 0;
				break;
			}
			else {
				// some error
				retval = GetLastError();
				break;
			}
		}
		else if (res == WAIT_OBJECT_0 + 1) {
			// exit event
			retval = 0;
			break;
		}
		else {
			// some error
			retval = GetLastError();
			break;
		}
	}

	free(thread_local_buffer);
	return retval;
}

void fini_threadpool(threadpool_t * tpool)
{
	if (tpool->hthreads != NULL) {
		for (int i = 0; i < tpool->num_of_threads; i++) {
			TerminateThread(tpool->hthreads[i], NULL);
			CloseHandle(tpool->hthreads[i]);
		}
		free(tpool->hthreads);
	}

	if (tpool->queue != NULL) {
		for (int i = 0; i < tpool->queue_size; i++) {
			free(tpool->queue[i].pParam);
		}
		free(tpool->queue);
	}

	if (tpool->queue_mtx != NULL) {
		CloseHandle(tpool->queue_mtx);
	}
	if (tpool->work_count_sem != NULL) {
		CloseHandle(tpool->work_count_sem);
	}
	if (tpool->stop_evt != NULL) {
		CloseHandle(tpool->stop_evt);
	}
}


/*
dwNumThreads – is number of threads
dwQueueSize – is a number of work item
dwMaxParamSize- is maximum size(in bytes) of a parameter passed to a work item
Function returns a handle(id) to a thread pool 
*/
TPHANDLE StartThreadPool(DWORD dwNumThreads, DWORD dwQueueSize, DWORD dwMaxParamSize)
{
	threadpool_t * tpool = NULL;
	tpool = (threadpool_t*)malloc(sizeof(threadpool_t));
	if (tpool == NULL) {
		return (TPHANDLE)-1;
	}
	tpool->num_of_threads = dwNumThreads;
	tpool->queue_size = dwQueueSize;
	tpool->read_index = 0;
	tpool->write_index = 0;
	tpool->queue_count = 0;
	tpool->max_param_size = dwMaxParamSize;
	tpool->hthreads = NULL;
	tpool->queue = NULL;
	tpool->queue_mtx = NULL;
	tpool->work_count_sem = NULL;
	tpool->stop_evt = NULL;

	tpool->queue_mtx = CreateMutex(NULL, FALSE, NULL);
	if (tpool->queue_mtx == NULL) {
		goto cleanup;
	}
	tpool->work_count_sem = CreateSemaphore(NULL, 0, tpool->queue_size, NULL);
	if (tpool->work_count_sem == NULL) {
		goto cleanup;
	}
	tpool->stop_evt = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (tpool->stop_evt == NULL) {
		goto cleanup;
	}

	tpool->hthreads = (HANDLE*)calloc(tpool->num_of_threads, sizeof(HANDLE));
	if (tpool->hthreads == NULL) {
		goto cleanup;
	}
	for (int i = 0; i < tpool->num_of_threads; i++) {
		tpool->hthreads[i] = CreateThread(NULL, 0, WorkerThreadProc, (LPVOID)tpool, 0, NULL);
		if (tpool->hthreads[i] == NULL) {
			goto cleanup;
		}
	}

	tpool->queue = (queue_item_t*)calloc(tpool->queue_size, sizeof(queue_item_t));
	if (tpool->queue == NULL) {
		goto cleanup;
	}
	for (int i = 0; i < tpool->queue_size; i++) {
		tpool->queue[i].pParam = (BYTE*)malloc(tpool->max_param_size);
		if (tpool->queue[i].pParam == NULL) {
			goto cleanup;
		}
	}

	return tpool;

cleanup:
	fini_threadpool(tpool);
	return (TPHANDLE)-1;
}

/*
hThreadPool is a handle to a thread pool obtained from start function
dwTimeout is amount of time to wait till all work items that are currently being processed or waiting in queue will finish.
Function return TRUE if all queued work items were successfully completed
*/
BOOL StopThreadPool(TPHANDLE hThreadPool, DWORD dwTimeout)
{
	threadpool_t * tpool = (threadpool_t*)hThreadPool;
	BOOL succ = TRUE;

	SetEvent(tpool->stop_evt);

	for (int i = 0; i < tpool->num_of_threads; i += MAXIMUM_WAIT_OBJECTS) {
		int count = (tpool->num_of_threads - i) > MAXIMUM_WAIT_OBJECTS ? MAXIMUM_WAIT_OBJECTS : (tpool->num_of_threads - i);
		DWORD res = WaitForMultipleObjects(count, tpool->hthreads, TRUE, dwTimeout);
		if (res < WAIT_OBJECT_0 || res >= WAIT_OBJECT_0 + count) {
			// some error
			succ = FALSE;
			break;
		}
		// after the first wait, set timeout to 0
		dwTimeout = 0;
	}

	fini_threadpool(tpool);
	return succ;
}

/*
hTreadPool is a handle to a thread pool
lpfnThreadFunc is a pointer to a function to be executed by thread
pParam and dwParamSize is a buffer to be passed to the function above
lpfnCallback is a pointer to a function to be called after work item finished(a callback function)
dwExitCode is an exit code returned by lpfnThreadFunc
Function return FALSE if there is no more place in the queue
*/
BOOL QueueWorkItem(TPHANDLE hThreadPool, LPTHREAD_FUNC lpfnThreadFunc, BYTE * pParam, DWORD dwParamSize, LPCLBCK_FUNC lpfnCallback)
{
	threadpool_t * tpool = (threadpool_t*)hThreadPool;
	HANDLE handles[] = {tpool->queue_mtx, tpool->stop_evt};
	BOOL succ = FALSE;

	if (dwParamSize > tpool->max_param_size) {
		// param too big
		return FALSE;
	}
	if (WaitForMultipleObjects(sizeof(handles) / sizeof(handles[0]), handles, FALSE, INFINITE) != WAIT_OBJECT_0) {
		// either stop event or some error
		return FALSE;
	}
	// we have the mutex
	if (tpool->queue_count >= tpool->queue_size) {
		goto cleanup;
	}
	tpool->queue_count++;
	queue_item_t * item = &tpool->queue[tpool->write_index];
	tpool->write_index = (tpool->write_index + 1) % (tpool->queue_size);
	item->dwParamSize = dwParamSize;
	item->lpfnCallback = lpfnCallback;
	memcpy(item->pParam, pParam, dwParamSize);
	item->lpfnThreadFunc = lpfnThreadFunc;
	// tell workers we have a new item
	ReleaseSemaphore(tpool->work_count_sem, 1, NULL);
	succ = TRUE;

cleanup:
	ReleaseMutex(tpool->queue_mtx);
	return succ;
}


