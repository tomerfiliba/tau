#include <windows.h>
#include <stdlib.h>
#include <stdio.h>


#define MAX_OUTPUT_SIZE (20*1024)

__declspec(dllexport) BOOL GenerateHTML(LPCSTR param, LPSTR pBuffer) 
{
	sprintf_s(pBuffer, MAX_OUTPUT_SIZE, "HTTP/1.0 200 OK\r\n\r\nHello, you passed me '%s'", param);
	return TRUE;
}

