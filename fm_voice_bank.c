#ifdef HAVE_STDIO
#include <stdio.h>
#endif
#include <string.h>

#include "fm_voice_bank.h"
#include "opl_voice.h"
#include "opm_voice.h"
#include "opn_voice.h"

void fm_voice_bank_position_copy(struct fm_voice_bank_position *to, struct fm_voice_bank_position *from) {
	memcpy(to, from, sizeof(*to));
}

void fm_voice_bank_position_init(struct fm_voice_bank_position *pos) {
	memset(pos, 0, sizeof(*pos));
}

void fm_voice_bank_init(struct fm_voice_bank *bank) {
	memset(bank, 0, sizeof(*bank));
}

void fm_voice_bank_clear(struct fm_voice_bank *bank) {
	if(bank->opl_voices) free(bank->opl_voices);
	if(bank->opm_voices) free(bank->opm_voices);
	if(bank->opn_voices) free(bank->opn_voices);
	fm_voice_bank_init(bank);
}

int fm_voice_bank_append_opl_voices(struct fm_voice_bank *bank, struct opl_voice *voices, int num_voices) {
	struct opl_voice *newvoices = fm_voice_bank_reserve_opl_voices(bank, num_voices);
	if(!newvoices) return -1;
	memcpy(newvoices, voices, num_voices * sizeof(*voices));
	return 0;
}

int fm_voice_bank_append_opm_voices(struct fm_voice_bank *bank, struct opm_voice *voices, int num_voices) {
	struct opm_voice *newvoices = fm_voice_bank_reserve_opm_voices(bank, num_voices);
	if(!newvoices) return -1;
	memcpy(newvoices, voices, num_voices * sizeof(*voices));
	return 0;
}

int fm_voice_bank_append_opn_voices(struct fm_voice_bank *bank, struct opn_voice *voices, int num_voices) {
	struct opn_voice *newvoices = fm_voice_bank_reserve_opn_voices(bank, num_voices);
	if(!newvoices) return -1;
	memcpy(newvoices, voices, num_voices * sizeof(*voices));
	return 0;
}

struct opl_voice *fm_voice_bank_reserve_opl_voices(struct fm_voice_bank *bank, int num_voices) {
	int old_num = bank->num_opl_voices;
	bank->num_opl_voices += num_voices;
	bank->opl_voices = realloc(bank->opl_voices, bank->num_opl_voices * sizeof(*bank->opl_voices));
	if(!bank->opl_voices) return 0;
	return bank->opl_voices + old_num;
}

struct opm_voice *fm_voice_bank_reserve_opm_voices(struct fm_voice_bank *bank, int num_voices) {
	int old_num = bank->num_opm_voices;
	bank->num_opm_voices += num_voices;
	bank->opm_voices = realloc(bank->opm_voices, bank->num_opm_voices * sizeof(*bank->opm_voices));
	if(!bank->opm_voices) return 0;
	return bank->opm_voices + old_num;
}

struct opn_voice *fm_voice_bank_reserve_opn_voices(struct fm_voice_bank *bank, int num_voices) {
	int old_num = bank->num_opn_voices;
	bank->num_opn_voices += num_voices;
	bank->opn_voices = realloc(bank->opn_voices, bank->num_opn_voices * sizeof(*bank->opn_voices));
	if(!bank->opn_voices) return 0;
	return bank->opn_voices + old_num;
}

#ifdef HAVE_STDIO
void fm_voice_bank_dump(struct fm_voice_bank *bank) {
	for(int i = 0; i < bank->num_opl_voices; i++) {
		printf("Voice %d (OPL): ", i);
		opl_voice_dump(&bank->opl_voices[i]);
	}
	for(int i = 0; i < bank->num_opm_voices; i++) {
		printf("Voice %d (OPM): ", i);
		opm_voice_dump(&bank->opm_voices[i]);
	}
	for(int i = 0; i < bank->num_opn_voices; i++) {
		printf("Voice %d (OPN): ", i);
		opn_voice_dump(&bank->opn_voices[i]);
	}
}
#endif
