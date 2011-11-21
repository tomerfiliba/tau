#include <tchar.h>
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MB           (1024 * 1024)
#define CHUNK_SIZE   (64 * 1024)


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

bool generate_file(long size, const char * filename)
{
	unsigned char buffer[CHUNK_SIZE];

	HANDLE hfile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL , NULL);

	if (hfile == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "CreateFile error\n");
		return false;
	}

	for (int i = 0; i < size / CHUNK_SIZE; i++) {
		for (int j = 0; j < CHUNK_SIZE; j++) {
			buffer[j] = rand() & 0xFF;
		}
		DWORD written;
		if (!WriteFile(hfile, buffer, CHUNK_SIZE, &written, NULL)) {
			fprintf(stderr, "WriteFile error\n");
			goto cleanup;
		}
		if (written != CHUNK_SIZE) {
			fprintf(stderr, "WriteFile error\n");
			goto cleanup;
		}
	}

	CloseHandle(hfile);
	return true;

cleanup:
	CloseHandle(hfile);
	DeleteFile((LPCWSTR)filename);
	return false;
}


int main(int argc, const char * argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <size in MB> <filename>\n", argv[0]);
		return 1;
	}

	char * end = NULL;
	long size = strtol(argv[1], &end, 10);
	if (size <= 0) {
		fprintf(stderr, "Size in MB must be a number >= 1, not '%s'\n", argv[1]);
		return 1;
	}
	size *= MB;

	srand((unsigned int)time(NULL));

	if (!generate_file(size, argv[2])) {
		return 1;
	}

	return 0;
}
