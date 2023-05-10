#include <stdint.h>
#include <stdio.h>
#include "bnk_file.h"
#include "tools.h"

int main(int argc, char **argv) {
	for(int i = 1; i < argc; i++) {
		size_t data_len;
		uint8_t *data = load_file(argv[i], &data_len);
		if(!data) {
			fprintf(stderr, "Could not open %s\n", argv[i]);
			continue;
		}
		struct bnk_file y12;
		if(bnk_file_load(&y12, data, data_len) != 0) {
			fprintf(stderr, "Could not load %s\n", argv[i]);
			continue;
		}

		printf("%s\n", argv[i]);
		bnk_file_dump(&y12);
	}
	return 0;
}
