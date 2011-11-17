#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define MAX_FILE_BUFFER  (4 * 1024)


bool open_files(const char * infile, const char * keyfile, const char * outfile,
			   int * infd, int * keyfd, int * outfd)
{
	struct stat stat_res;

	if ((*infd = open(infile, O_RDONLY)) < 0) {
		perror(infile);
		goto cleanup1;
	}

	if ((*keyfd = open(keyfile, O_RDONLY)) < 0) {
		perror(keyfile);
		goto cleanup2;
	}

	if (fstat(*keyfd, &stat_res) < 0) {
		perror(keyfile);
		goto cleanup3;
	}
	if (stat_res.st_size == 0) {
		fprintf(stderr, "Key file '%s' must not be empty\n", keyfile);
		goto cleanup3;
	}

	if ((*outfd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
		perror(outfile);
		goto cleanup3;
	}

	return true;

cleanup3:
	close(*keyfd);
cleanup2:
	close(*infd);
cleanup1:
	return false;
}

void xor_buffers(const unsigned char * inbuf, const unsigned char * keybuf,
			unsigned char * outbuf, ssize_t size)
{
	for (ssize_t i = 0; i < size; i++) {
		outbuf[i] = inbuf[i] ^ keybuf[i];
	}
}

bool decrypt_file(int infd, int keyfd, int outfd)
{
	unsigned char inbuf[MAX_FILE_BUFFER];
	unsigned char keybuf[MAX_FILE_BUFFER];
	unsigned char outbuf[MAX_FILE_BUFFER];
	ssize_t insize = 0, keysize = 0, outsize = 0;

	while (true) {
		/* read key bytes */
		if ((keysize = read(keyfd, keybuf, sizeof(keybuf))) < 0) {
			perror("Unable to read from key file");
			return false;
		}
		if (keysize == 0) {
			/* key file reached EOF -- move fp to origin */
			if (lseek(keyfd, 0, SEEK_SET) == ((off_t)-1)) {
				perror("Unable to seek key file to origin");
				return false;
			}
			if ((keysize = read(keyfd, keybuf, sizeof(keybuf))) < 0) {
				perror("Unable to read from key file");
				return false;
			}
		}

		/* read input bytes */
		if ((insize = read(infd, inbuf, keysize)) < 0) {
			perror("Unable to read from input file");
			return false;
		}
		if (insize == 0) {
			/* EOF means we finished successfully */
			break;
		}

		/* write encrypted data */
		outsize = (keysize > insize) ? insize : keysize;
		xor_buffers(inbuf, keybuf, outbuf, outsize);
		if (write(outfd, outbuf, outsize) != outsize) {
			perror("Unable to write all data to output file");
			return false;
		}
	}

	return true;
}

int main(int argc, char* argv[])
{
	int ret = 1;
	int infd, outfd, keyfd;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <infile> <keyfile> <outfile>\n", argv[0]);
		goto cleanup1;
	}

	if (!open_files(argv[1], argv[2], argv[3], &infd, &keyfd, &outfd)) {
		/* error message printed by open_files */
		goto cleanup1;
	}

	if (!decrypt_file(infd, keyfd, outfd)) {
		/* error message printed by encrypt_files */
		goto cleanup2;
	}

	/* success */
	ret = 0;

cleanup2:
	close(outfd);
	close(keyfd);
	close(infd);
cleanup1:
	return ret;
}

