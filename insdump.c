#include <stdint.h>
#include <stdio.h>
#include "ins_file.h"
#include "tools.h"

int main(int argc, char **argv) {
	for(int i = 1; i < argc; i++) {
		size_t data_len;
		uint8_t *data = load_file(argv[i], &data_len);
		if(!data) {
			fprintf(stderr, "Could not open %s\n", argv[i]);
			continue;
		}
		struct ins_file ins;
		if(ins_file_load(&ins, data, data_len) != 0) {
			fprintf(stderr, "Could not load %s\n", argv[i]);
			continue;
		}

		printf("%s\n", argv[i]);
		ins_file_dump(&ins);
	}
	return 0;
}
