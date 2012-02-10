#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include "../mythreadpool/mythreadpool.h"

#pragma comment(lib, "Ws2_32.lib")

#define MAX_QUEUE_SIZE            (1000)
#define MAX_CLIENT_REQUEST_SIZE   (64 * 1024)
#define MAX_CLIENT_RESPONSE_SIZE  (64 * 1024)

typedef struct {
	WSADATA wsaData;
	int port;
	TCHAR in_dir[1024];
	int num_of_threads;
	SOCKET listener;
	TPHANDLE tpool;
} program_state_t;

typedef BOOL *(pGenerateHTML)(LPCSTR param, LPSTR pBuffer);

typedef struct
{
	program_state_t * state;
	SOCKET client;
} queue_param_t;


bool fini_state(program_state_t * state)
{
	if (state->listener != INVALID_SOCKET) {
		closesocket(state->listener);
	}
	if (state->tpool != NULL) {
		StopThreadPool(state->tpool, 2 * 60 * 1000);
	}
	return true;
}

bool init_state(program_state_t * state)
{
	state->tpool = (TPHANDLE)-1;
	state->listener = INVALID_SOCKET;

	if (WSAStartup(MAKEWORD(2,2), &state->wsaData) != 0) {
		_tprintf(_T("WSAStartup failed\n"));
		goto cleanup;
	}
	state->tpool = StartThreadPool(state->num_of_threads, MAX_QUEUE_SIZE, sizeof(queue_param_t));
	if (state->tpool == (TPHANDLE)-1) {
		_tprintf(_T("StartThreadPool failed\n"));
		goto cleanup;
	}
	state->listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (state->listener == INVALID_SOCKET) {
		_tprintf(_T("create socket failed\n"));
		goto cleanup;
	}

	// Get the local host information
	hostent *localHost = gethostbyname("");
	char *localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
	struct sockaddr_in saServer;
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = inet_addr(localIP);
	saServer.sin_port = htons(state->port);

	if (bind(state->listener, (const struct sockaddr*)&saServer, sizeof(saServer)) != 0) {
		_tprintf(_T("bind socket failed\n"));
		goto cleanup;
	}
	if (listen(state->listener, 20) != 0) {
		_tprintf(_T("socket listen failed\n"));
		goto cleanup;
	}

	return true;
cleanup:
	fini_state(state);
	return false;
}


#undef min
static inline int min(int a, int b) {
	return a < b ? a : b;
}

void handle_dll(program_state_t * state, char * response, const char * file, const char * param)
{
	HMODULE dll = LoadLibraryA(file);
	if (dll == NULL) {
		strcpy(response, "HTTP/1.0 404 Not Found\r\n\r\n");
		return;
	}
	pGenerateHTML * proc = (pGenerateHTML*)GetProcAddress(dll, "GenerateHTML");
	if (proc == NULL) {
		strcpy(response, "HTTP/1.0 404 Not Found\r\n\r\n");
		FreeLibrary(dll);
		return;
	}
	proc(param, response);
	FreeLibrary(dll);
}

void handle_simple(program_state_t * state, char * response, const char * file)
{
	char fullfile[1024];
	sprintf(fullfile, "%s\\%d.txt", state->in_dir, file);
	HANDLE hfile = CreateFileA(fullfile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		strcpy(response, "HTTP/1.0 404 Not Found\r\n\r\n");
		return;
	}
	char * response_begin = response;
	strcpy(response, "HTTP/1.0 200 OK\r\n\r\n");
	response += strlen(response);
	while (true) {
		DWORD readcount;
		if (!ReadFile(hfile, response, 4000, &readcount, NULL)) {
			strcpy(response_begin, "HTTP/1.0 500 Internal Server Error\r\n\r\n");
			return;
		}
		if (readcount == 0) {
			break;
		}
	}

	CloseHandle(hfile);
}

DWORD queue_func(BYTE* pParam, DWORD param_size)
{
	int res = -1;
	queue_param_t * param = (queue_param_t*)pParam;
	char buffer[MAX_CLIENT_REQUEST_SIZE];
	char response[MAX_CLIENT_RESPONSE_SIZE];
	int count = 0;

	while (count <= sizeof(buffer)) {
		int cnt = recv(param->client, buffer + count, min(sizeof(buffer) - count, 16*1024), 0);
		if (cnt == 0) {
			break;
		}
		else if (cnt == SOCKET_ERROR) {
			_tprintf(_T("recv error\n"));
			break;
		}
		count += cnt;
	}
	char * method = strtok(buffer, " ");
	char * url = strtok(buffer, " ");

	if (_stricmp(method, "get") != 0) {
		strcpy(response, "HTTP/1.0 501 Not Implemented\r\n\r\n");
		goto sendresponse;
	}
	char * qmark = strchr(url, '?');
	char * file = url;
	char * getparam = NULL;
	if (qmark != NULL) {
		*qmark = '\0';
		getparam = strchr(qmark + 1, '=') + 1;
	}
	if (strcmp(file + strlen(file) - 5, ".dll") == 0) {
		handle_dll(param->state, response, file, getparam);
	}
	else {
		handle_simple(param->state, response, file);
	}

sendresponse:
	send(param->client, response, strlen(response), 0);
	closesocket(param->client);
	return res;
}

DWORD queue_func_callback(BYTE* pParam, DWORD param_size, DWORD dwExitCode)
{
	// this function is not used
	return 0;
}


bool run_server(program_state_t * state)
{
	while (true) {
		queue_param_t param;
		param.state = state;
		param.client = accept(state->listener, NULL, NULL);
		if (param.client == INVALID_SOCKET) {
			continue;
		}
		if (!QueueWorkItem(state->tpool, (LPTHREAD_FUNC)queue_func, (BYTE*)&param, sizeof(param), 
				(LPCLBCK_FUNC)queue_func_callback)) {
			_tprintf(_T("QueueWorkItem failed\n"));
			break;
		}
		
	}
	return false;
}

int _tmain(int argc, TCHAR *argv[])
{
	int res = 0;
	program_state_t state;

	if (argc != 4) {
		_tprintf(_T("Usage: %s <port> <in folder> <num of threads>\n"), argv[0]);
		return -1;
	}
	state.port = _tstoi(argv[1]);
	_tcscpy_s(state.in_dir, argv[2]);
	state.num_of_threads = _tstoi(argv[3]);

	if (!init_state(&state)) {
		res = -1;
		goto cleanup;
	}
	if (!run_server(&state)) {
		res = -1;
	}

cleanup:
	if (!fini_state(&state)) {
		res = -1;
	}
	return res;
}

