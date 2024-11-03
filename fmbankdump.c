#include <stdio.h>

#include "tools.h"
#include "fm_voice_bank.h"

int main(int argc, char **argv) {
	struct fm_voice_bank bank;
	fm_voice_bank_init(&bank);

	for(int i = 1; i < argc; i++) {
		printf("%s\n", argv[i]);
		size_t data_len;
		uint8_t *data = load_file(argv[i], &data_len);
		if(!data) {
			fprintf(stderr, "Could not open %s\n", argv[i]);
			continue;
		}
		fm_voice_bank_load(&bank, data, data_len);
	}

	fm_voice_bank_dump(&bank);
	fm_voice_bank_clear(&bank);

	return 0;
}
