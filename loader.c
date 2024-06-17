#include "loader.h"

extern struct loader bnk_file_loader;
extern struct loader dmp_file_loader;
extern struct loader ins_file_loader;
extern struct loader op3_file_loader;
extern struct loader opm_file_loader;
extern struct loader sbi_file_loader;
extern struct loader syx_dx21_loader;
extern struct loader syx_fb01_loader;
extern struct loader tfi_file_loader;
extern struct loader y12_file_loader;

struct loader *loaders[] = {
	&bnk_file_loader,
	&dmp_file_loader,
	&ins_file_loader,
	&op3_file_loader,
	&opm_file_loader,
	&sbi_file_loader,
	&syx_dx21_loader,
	&syx_fb01_loader,
	&tfi_file_loader,
	&y12_file_loader,
};

struct loader *get_loader_by_name(char *name) {
	for(int i = 0; i < sizeof(loaders) / sizeof(loaders[0]); i++)
		if(!strcmp(loaders[i]->name, name)) return loaders[i];
	return 0;
}

int loader_save(struct loader *loader, struct fm_voice_bank *bank, struct fm_voice_bank_position *pos, int (*write_fn)(void *, size_t, void *), void *data_ptr) {
	if(!loader->save) return -1;
	return loader->save(bank, pos, write_fn, data_ptr);
}
