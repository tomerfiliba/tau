#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include "../mythreadpool/mythreadpool.h"

#pragma comment(lib, "Ws2_32.lib")

#define EXIT_EVT_NAME  _T("com.tomerfiliba.ex7.stress-exit-evt")
#define MAX_QUEUE_SIZE  (2000)
#define NUM_OF_REQUESTS (5)
#define MAX_REQUEST_SIZE (16*1024)

typedef struct {
	DWORD length;
	BYTE buffer[MAX_REQUEST_SIZE];
} request_data_t;

typedef struct {
	WSADATA wsaData;
	TCHAR host[256];
	int port;
	TCHAR in_dir[1024];
	TCHAR out_dir[1024];
	request_data_t requests[NUM_OF_REQUESTS];
	int interval_ms;
	int num_of_threads;
	ADDRINFOT * addrinfo;

	TPHANDLE tpool;
	HANDLE exit_evt;
} program_state_t;

bool fini_state(program_state_t * state)
{
	if (state->addrinfo == NULL) {
		FreeAddrInfo(state->addrinfo);
	}
	if (state->exit_evt != NULL) {
		CloseHandle(state->exit_evt);
	}
	if (state->tpool != (TPHANDLE)-1) {
		StopThreadPool(state->tpool, 2 * 60 * 1000);
	}
	return true;
}

typedef struct
{
	program_state_t * state;
} queue_param_t;

bool init_state(program_state_t * state)
{
	state->addrinfo = NULL;
	state->exit_evt = NULL;
	state->tpool = (TPHANDLE)-1;

	if (WSAStartup(MAKEWORD(2,2), &state->wsaData) != 0) {
		_tprintf(_T("WSAStartup failed\n"));
		goto cleanup;
	}
	state->exit_evt = CreateEvent(NULL, TRUE, FALSE, EXIT_EVT_NAME);
	if (state->exit_evt == NULL) {
		_tprintf(_T("CreateEvent failed: exit-evt\n"));
		goto cleanup;
	}
	state->tpool = StartThreadPool(state->num_of_threads, MAX_QUEUE_SIZE, sizeof(queue_param_t));
	if (state->tpool == (TPHANDLE)-1) {
		_tprintf(_T("StartThreadPool failed\n"));
		goto cleanup;
	}

	if (GetAddrInfo(state->host, NULL, NULL, &state->addrinfo) != 0) {
		_tprintf(_T("GetAddrInfo failed\n"));
		goto cleanup;
	}

	for (int i = 1; i <= NUM_OF_REQUESTS; i++) {
		TCHAR filename[1024];
		_stprintf_s(filename, _T("%s\\%d.txt"), state->in_dir, i);
		HANDLE hfile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (hfile == INVALID_HANDLE_VALUE) {
			goto cleanup;
		}
		if (!ReadFile(hfile, state->requests[i-1].buffer, MAX_REQUEST_SIZE, &state->requests[i-1].length, NULL)) {
			CloseHandle(hfile);
			goto cleanup;
		}
		CloseHandle(hfile);
	}

	return true;
cleanup:
	fini_state(state);
	return false;
}

DWORD queue_func(BYTE* pParam, DWORD param_size)
{
	int res = -1;
	queue_param_t * param = (queue_param_t*)pParam;
	TCHAR tempfilename[MAX_PATH];

	if (!GetTempFileName(param->state->out_dir, _T("out"), 0, tempfilename)) {
		_tprintf(_T("GetTempFileName failed\n"));
		return -1;
	}
	HANDLE hfile = CreateFile(tempfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 0);
	if (hfile == INVALID_HANDLE_VALUE) {
		_tprintf(_T("CreateFile failed: %s\n"), tempfilename);
		return -1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		_tprintf(_T("create socket failed\n"));
		goto cleanup;
	}
	if (connect(sock, param->state->addrinfo->ai_addr, param->state->addrinfo->ai_addrlen) != 0) {
		_tprintf(_T("connect() failed\n"));
		goto cleanup2;
	}

	int i = rand() % NUM_OF_REQUESTS;
	int remaining = param->state->requests[i].length;
	const char * buf = (const char*)param->state->requests[i].buffer;
	while (remaining > 0) {
		int count = send(sock, buf, remaining, 0);
		if (count == SOCKET_ERROR) {
			_tprintf(_T("send() failed\n"));
			goto cleanup2;
		}
		remaining -= count;
		buf += count;
	}
	shutdown(sock, SD_SEND);
	char recvbuf[4096];
	while (true) {
		int count = recv(sock, recvbuf, sizeof(recvbuf), 0);
		if (count == 0) {
			break; // EOF
		}
		else if (count == SOCKET_ERROR) {
			_tprintf(_T("recv() failed\n"));
			goto cleanup2;
		}
		DWORD writecount;
		if (!WriteFile(hfile, recvbuf, count, &writecount, NULL)) {
			_tprintf(_T("WriteFile() failed\n"));
			goto cleanup2;
		}
	}
	shutdown(sock, SD_RECEIVE);

	res = 0;
cleanup2:
	closesocket(sock);
cleanup:
	CloseHandle(hfile);
	return res;
}

DWORD queue_func_callback(BYTE* pParam, DWORD param_size, DWORD dwExitCode)
{
	// this function is not used
	return 0;
}


void signal_all_processes_to_exit()
{
	HANDLE evt = CreateEvent(NULL, TRUE, FALSE, EXIT_EVT_NAME);
	SetEvent(evt);
	CloseHandle(evt);
}

bool run_stress(program_state_t * state)
{
	while (true) {
		queue_param_t param;
		param.state = state;
		if (!QueueWorkItem(state->tpool, (LPTHREAD_FUNC)queue_func, (BYTE*)&param, sizeof(param), 
				(LPCLBCK_FUNC)queue_func_callback)) {
			_tprintf(_T("QueueWorkItem failed\n"));
			break;
		}
		int res = WaitForSingleObject(state->exit_evt, state->interval_ms);
		if (res == WAIT_OBJECT_0) {
			// exit event
			return true;
		}
		else if (res != WAIT_TIMEOUT) {
			// some error
			break;
		}
	}
	return false;
}

int _tmain(int argc, TCHAR *argv[])
{
	int res = 0;
	program_state_t state;

	if (argc == 1) {
		// called without any arguments -- signal everybody to exit
		signal_all_processes_to_exit();
		return 0;
	}
	
	if (argc != 7) {
		_tprintf(_T("Usage: %s <ip> <port> <in folder> <out folder> <interval ms> <num of threads>\n"), argv[0]);
		return -1;
	}
	_tcscpy_s(state.host, argv[1]);
	state.port = _tstoi(argv[2]);
	_tcscpy_s(state.in_dir, argv[3]);
	_tcscpy_s(state.out_dir, argv[4]);
	state.interval_ms = _tstoi(argv[5]);
	state.num_of_threads = _tstoi(argv[6]);

	if (!init_state(&state)) {
		res = -1;
		goto cleanup;
	}
	if (!run_stress(&state)) {
		res = -1;
		goto cleanup;
	}

cleanup:
	if (!fini_state(&state)) {
		res = -1;
	}
	return res;
}
