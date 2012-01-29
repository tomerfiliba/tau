#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>


#define MB               (1024 * 1024)
#define RAND_BUFF_SIZE   (16 * 1024) // MUST be a power of 2

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

void fill_randbuf(unsigned char * buffer, size_t size)
{
	for(size_t i = 0; i < size; i++) {
		buffer[i] = rand() & 0xFF;
	}
}

bool generate_randfile(size_t size, const TCHAR * filename)
{
	unsigned char buf[RAND_BUFF_SIZE];
	bool succ = false;

	HANDLE hfile = CreateFile(filename, GENERIC_WRITE, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		print_last_error(_T("CreateFile"));
		return false;
	}

	// since the file size is in whole MB, and the buffer we use is a power of 2,
	// we know that (size / sizeof(buf)) is a whole number.
	for (size_t i = 0; i < size / sizeof(buf); i++) {
		fill_randbuf(buf, sizeof(buf));
		DWORD written;
		if (!WriteFile(hfile, buf, sizeof(buf), &written, NULL)) {
			print_last_error(_T("WriteFile"));
			goto cleanup;
		}
		if (written != sizeof(buf)) {
			_tprintf(_T("Failed to write whole buffer\n"));
			goto cleanup;
		}
	}

	succ = true;

cleanup:
	CloseHandle(hfile);
	return succ;
}


int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 3) {
		_tprintf(_T("Usage: %s <size in MB> <output file>\n"), argv[0]);
		return 1;
	}

	int size = _tstoi(argv[1]);
	if (size <= 0) {
		_tprintf(_T("'Size in MB' must be a positive integer: '%s'\n"), argv[1]);
		return 1;
	}

	srand((unsigned int)time(NULL));
	if (!generate_randfile(size * MB, argv[2])) {
		// error message printed by generate_randfile
		return 1;
	}

	return 0;
}

