#ifndef BNK_FILE_H_
#define BNK_FILE_H_

#include <stdint.h>

struct bnk_file_operator {
	uint8_t ksl, mul, fb, ar, sl, eg, dr, rr, tl, am, vib, ksr, con, wave_sel;
};

struct bnk_file_instrument {
	uint8_t percussive, voice_num;
	struct bnk_file_operator operators[2];
};

struct bnk_file_name {
	uint16_t index;
	uint8_t flags;
	char name[9];
};

struct bnk_file {
	uint8_t ver_major, ver_minor;
	uint16_t num_used, num_instruments;
	struct bnk_file_instrument *instruments;
	struct bnk_file_name *names;
};

void bnk_file_init(struct bnk_file *f);
int bnk_file_load(struct bnk_file *f, uint8_t *data, size_t data_len);
int bnk_file_test(uint8_t *data, size_t data_len);
int bnk_file_save(struct bnk_file *f, size_t (*write_fn)(void *buf, size_t len, void *data_ptr), void *data_ptr);
void bnk_file_dump(struct bnk_file *f);

#endif /* BNK_FILE_H_ */
