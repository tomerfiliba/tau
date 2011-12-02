#include <tchar.h>
#include <windows.h>


#define CHUNK_SIZE      (16 * 1024)
#define MIN(a,b)        ((a<b)?(a):(b))

void print_last_error(const _TCHAR * text)
{
	void * msg = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&msg, 0, NULL);

	if (text != NULL && text[0] != '\0') {
		_tprintf(_T("%s: %s\n"), text, (LPTSTR)msg);
	}
	else {
		_tprintf(_T("%s\n"), (LPTSTR)msg);
	}
	LocalFree(msg);
}

unsigned char xor_buffer(unsigned char * buf, size_t size)
{
	unsigned char xor = 0;
	for (size_t i = 0; i < size; i++) {
		xor ^= buf[i];
	}
	return xor;
}


int compute_checksum(TCHAR * filename, size_t offset, size_t size)
{
	int ret = -1;
	unsigned char checksum = 0;
	unsigned char buffer[CHUNK_SIZE];
	DWORD remaining = size;

	HANDLE hfile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile"));
		return -1;
	}
	if (SetFilePointer(hfile, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER && 
			GetLastError() != NO_ERROR) {
		print_last_error(_T("SetFilePointer"));
		goto cleanup;
	}

	while (remaining > 0) {
		DWORD readcount;
		if (!ReadFile(hfile, buffer, MIN(remaining, sizeof(buffer)), &readcount, NULL)) {
			goto cleanup;
		}
		remaining -= readcount;
		checksum ^= xor_buffer(buffer, readcount);
	}

	ret = checksum;

cleanup:
	CloseHandle(hfile);
	return ret;
}


int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 4) {
		_tprintf(_T("Usage: %s <full path> <offset in bytes> <size in bytes>\n"), argv[0]);
		return 1;
	}

	int offset = _tstoi(argv[2]);
	if (offset < 0) {
		_tprintf(_T("'offset' must be a positive integer: '%s'\n"), argv[2]);
		return 1;
	}

	int size = _tstoi(argv[3]);
	if (size <= 0) {
		_tprintf(_T("'size' must be a positive integer: '%s'\n"), argv[3]);
		return 1;
	}

	return compute_checksum(argv[1], offset, size);
}

