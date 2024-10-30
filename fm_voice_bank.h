#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "opl_voice.h"
#include "opm_voice.h"
#include "opn_voice.h"

struct fm_voice_bank_position {
	int opl, opm, opn;
};
void fm_voice_bank_position_copy(struct fm_voice_bank_position *to, struct fm_voice_bank_position *from);
void fm_voice_bank_position_init(struct fm_voice_bank_position *pos);

struct fm_voice_bank {
	struct opl_voice *opl_voices;
	int num_opl_voices;
	struct opm_voice *opm_voices;
	int num_opm_voices;
	struct opn_voice *opn_voices;
	int num_opn_voices;
};

void fm_voice_bank_init(struct fm_voice_bank *bank);
void fm_voice_bank_clear(struct fm_voice_bank *bank);
int fm_voice_bank_append_opl_voices(struct fm_voice_bank *bank, struct opl_voice *voices, int num_voices);
struct opl_voice *fm_voice_bank_reserve_opl_voices(struct fm_voice_bank *bank, int num_voices);
int fm_voice_bank_append_opm_voices(struct fm_voice_bank *bank, struct opm_voice *voices, int num_voices);
struct opm_voice *fm_voice_bank_reserve_opm_voices(struct fm_voice_bank *bank, int num_voices);
int fm_voice_bank_append_opn_voices(struct fm_voice_bank *bank, struct opn_voice *voices, int num_voices);
struct opn_voice *fm_voice_bank_reserve_opn_voices(struct fm_voice_bank *bank, int num_voices);
#ifdef HAVE_STDIO
void fm_voice_bank_dump(struct fm_voice_bank *bank);
#endif
int fm_voice_bank_load(struct fm_voice_bank *bank, void *data, size_t data_len);
