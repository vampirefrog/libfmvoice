#ifndef OP3_FILE_H_
#define OP3_FILE_H_

#include <stdint.h>

struct op3_file_instrument_op {
	uint8_t ave_kvm, ksl_tl;
	uint8_t ar_dr, sl_rr;
	uint8_t ws;
};

struct op3_file_instrument {
	uint8_t en_4op, percnotenum;
	struct op3_file_instrument_op op[4];
	uint8_t fb_con12, fb_con34;
};

struct op3_file {
	uint16_t start_melodic, start_percussive;
	uint16_t count_melodic, count_percussive;
	struct op3_file_instrument melodic[128], percussive[128];
};

void op3_file_init(struct op3_file *f);
int op3_file_load(struct op3_file *f, uint8_t *data, size_t data_len);
int op3_file_test(uint8_t *data, size_t data_len);
int op3_file_save(struct op3_file *f, size_t (*write_fn)(void *buf, size_t len, void *data_ptr), void *data_ptr);
void op3_file_dump(struct op3_file *f);

#endif /* OP3_FILE_H_ */
