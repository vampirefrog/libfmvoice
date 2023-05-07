#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include "platform.h"
#include "syx_fb01.h"

static size_t midi_write_func(void *buf, size_t bytes, void *data_ptr) {
	FILE *f = (FILE *)data_ptr;
	return fwrite(buf, 1, bytes, f);
}

int main(int argc, char **argv) {
	for(int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "rb");
		if(!f) {
			fprintf(stderr, "Could not open \"%s\": %s (%d)\n", argv[i], strerror(errno), errno);
			continue;
		}
		fseek(f, 0, SEEK_END);
		size_t filesize = ftell(f);
		fseek(f, 0, SEEK_SET);
		uint8_t *buf = malloc(filesize);
		if(!buf) {
			fprintf(stderr, "Could not allocate " SIZE_T_FMT " bytes: %s (%d)\n", filesize, strerror(errno), errno);
			fclose(f);
			continue;
		}
		size_t bytesread = fread(buf, 1, filesize, f);
		if(bytesread != filesize) {
			fprintf(stderr, "Could not read " SIZE_T_FMT " bytes: %s (%d)\n", filesize, strerror(errno), errno);
			free(buf);
			continue;
		}
		fclose(f);

		struct fb01_bulk_voice_bank bank;
		fb01_bulk_voice_bank_from_buffer(&bank, buf, filesize);

		fb01_bulk_voice_bank_dump(&bank);
		FILE *out = fopen("out.syx", "wb");
		fb01_bulk_voice_bank_send(&bank, midi_write_func, (void *)out);
		fclose(out);

		free(buf);
	}

	return 0;
}
