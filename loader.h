#pragma once

#include "fm_voice_bank.h"

struct loader_save_result {
	int num_opl_voices_written;
	int num_opm_voices_written;
	int num_opn_voices_written;
};

struct loader {
	int (*load)(void *data, int data_len, struct fm_voice_bank *bank);
	int (*save)(struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr);
	const char *name, *description;
	const char *file_ext;
	int max_opl_voices, max_opm_voices, max_opn_voices;
};

extern struct loader *loaders[];
struct loader *get_loader_by_name(char *name);
int loader_save(struct loader *loader, struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr);
