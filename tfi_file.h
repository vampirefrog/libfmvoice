#ifndef TFI_FILE_H_
#define TFI_FILE_H_

#include <stdint.h>
#include <stdlib.h>

struct tfi_file_operator {
	uint8_t
		mul,    // 0..15
		dt,     // -3..0..3
		tl,     // 0..127
		rs,     // 0..3
		ar,     // 0..31
		dr,     // 0..31
		sr,     // 0..31
		rr,     // 0..15
		sl,     // 0..15
		ssg_eg; // 7..15
};

struct tfi_file {
	uint8_t alg, fb;
	struct tfi_file_operator operators[4];
};

void tfi_file_init(struct tfi_file *f);
int tfi_file_load(struct tfi_file *tfi, uint8_t *data, size_t data_len);
int tfi_file_save(struct tfi_file *f, int (*write_fn)(void *buf, size_t bufsize, void *data_ptr), void *data_ptr);
void tfi_file_dump(struct tfi_file *f);

#endif /* TFI_FILE_H_ */
