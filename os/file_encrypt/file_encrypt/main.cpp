#include <tchar.h>
#include <windows.h>


#define MAX_FILE_BUFFER  (4 * 1024)


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

bool open_files(const _TCHAR * infile, const _TCHAR * keyfile, const _TCHAR * outfile,
			   HANDLE * hin, HANDLE * hkey, HANDLE * hout)
{
	*hin = CreateFile(infile, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (*hin == INVALID_HANDLE_VALUE) {
		print_last_error(infile);
		goto cleanup1;
	}

	*hkey = CreateFile(keyfile, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (*hkey == INVALID_HANDLE_VALUE) {
		print_last_error(keyfile);
		goto cleanup2;
	}

	DWORD hi = 0;
	DWORD lo = GetFileSize(hkey, &hi);
	if (lo == 0 && hi == 0) {
		_tprintf(_T("Key file '%s' must not be empty\n"), keyfile);
		goto cleanup3;
	}

	*hout = CreateFile(outfile, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (*hout == INVALID_HANDLE_VALUE) {
		print_last_error(outfile);
		goto cleanup3;
	}

	return true;

cleanup3:
	CloseHandle(hkey);
cleanup2:
	CloseHandle(hin);
cleanup1:
	return false;
}

void xor_buffers(const unsigned char * inbuf, const unsigned char * keybuf,
			unsigned char * outbuf, int size)
{
	for (int i = 0; i < size; i++) {
		outbuf[i] = inbuf[i] ^ keybuf[i];
	}
}

bool encrypt_file(HANDLE hin, HANDLE hkey, HANDLE hout)
{
	unsigned char inbuf[MAX_FILE_BUFFER];
	unsigned char keybuf[MAX_FILE_BUFFER];
	unsigned char outbuf[MAX_FILE_BUFFER];
	DWORD insize = 0, keysize = 0, outsize = 0, written = 0;

	while (true) {
		/* read key bytes */
		if (!ReadFile(hkey, keybuf, sizeof(keybuf), &keysize, NULL)) {
			print_last_error(_T("Unable to read from key file"));
			return false;
		}
		if (keysize == 0) {
			/* key file reached EOF -- move fp to origin */
			if (SetFilePointer(hkey, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
				print_last_error(_T("Unable to seek key file to origin"));
				return false;
			}
			if (!ReadFile(hkey, keybuf, sizeof(keybuf), &keysize, NULL)) {
				print_last_error(_T("Unable to read from key file"));
				return false;
			}
		}

		/* read input bytes */
		if (!ReadFile(hin, inbuf, keysize, &insize, NULL)) {
			print_last_error(_T("Unable to read from input file"));
			return false;
		}
		if (insize == 0) {
			/* EOF means we finished successfully */
			break;
		}

		/* write encrypted data */
		outsize = (keysize > insize) ? insize : keysize;
		xor_buffers(inbuf, keybuf, outbuf, outsize);
		if (!WriteFile(hout, outbuf, outsize, &written, NULL)) {
			print_last_error(_T("Unable to write to output file"));
			return false;
		}
		if (outsize != written) {
			_tprintf(_T("Unable to write to output file: Wrote fewer bytes"));
			return false;
		}
	}

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int ret = 1;
	HANDLE hin, hout, hkey;

	if (argc != 4) {
		_tprintf(_T("Usage: %s <infile> <keyfile> <outfile>"), argv[0]);
		goto cleanup1;
	}

	if (!open_files(argv[1], argv[2], argv[3], &hin, &hkey, &hout)) {
		/* error message printed by open_files */
		goto cleanup1;
	}

	if (!encrypt_file(hin, hkey, hout)) {
		/* error message printed by encrypt_files */
		goto cleanup2;
	}

	/* success */
	ret = 0;

cleanup2:
	CloseHandle(hout);
	CloseHandle(hkey);
	CloseHandle(hin);
cleanup1:
	return ret;
}

