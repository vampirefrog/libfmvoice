#include <stdint.h>
#include <stdio.h>
#include "op3_file.h"
#include "tools.h"

int main(int argc, char **argv) {
	for(int i = 1; i < argc; i++) {
		size_t data_len;
		uint8_t *data = load_file(argv[i], &data_len);
		if(!data) {
			fprintf(stderr, "Could not open %s\n", argv[i]);
			continue;
		}
		struct op3_file op3;
		if(op3_file_load(&op3, data, data_len) != 0) {
			fprintf(stderr, "Could not load %s\n", argv[i]);
			continue;
		}

		printf("%s (%lluB)\n", argv[i], data_len);
		op3_file_dump(&op3);
	}
	return 0;
}
