#include <stdlib.h>

#include "midi_file.h"

void fb01_parameter_change(struct midi_track *t, uint8_t parameter, uint8_t value) {
	uint8_t sysex[] = {
		0x43,
		0x75,
		0x00,
		0x18,
		parameter,
		value & 0x0f,
		value >> 4,
		0xf7,
	};
	midi_track_write_sysex(t, 0, sysex, sizeof(sysex));
}

int main(int argc, char **argv) {
	struct midi_file f;
	if(midi_file_init(&f, MIDI_FORMAT_MULTI_TRACKS, 1, 48) != MIDI_SUCCESS) {
		fprintf(stderr, "Could not init MIDI file\n");
		return 1;
	}

	struct midi_track *t = f.tracks;

	midi_track_write_program_change(t, 0, 0, 0);

	printf(
		"Velocity\t"
		"KSL0\tVel Sens\tKSLD\tTL Adj\tAME\tAR Vel Sens\tD1R\t"
		"KSL0\tVel Sens\tKSLD\tTL Adj\tAME\tAR Vel Sens\tD1R\t"
		"KSL0\tVel Sens\tKSLD\tTL Adj\tAME\tAR Vel Sens\tD1R\t"
		"KSL0\tVel Sens\tKSLD\tTL Adj\tAME\tAR Vel Sens\tD1R\n"
	);

	srand(12345);
	for(int i = 0; i < 256; i++) {
		uint8_t random_data[3 * 4];
		for(int j = 0; j < sizeof(random_data) / sizeof(random_data[0]); j++) {
			random_data[j] = rand();
		}

		for(int j = 1; j <= 127; j+=10) {
			printf("%d\t", j);

			for(int k = 0; k < 4; k++) {
				uint8_t ksl0_tl_vel_sens = random_data[k * 3 + 0] & 0xf0;
				uint8_t ksld_tl_adjust = random_data[k * 3 + 1];
				uint8_t ame_ar_vel_sens_d1r = (random_data[k * 3 + 2] & 0x60) | 0x1f;
				printf(
					"%d\t%d\t%d\t%d\t%d\t%d\t%d\t",
					ksl0_tl_vel_sens >> 7,
					ksl0_tl_vel_sens >> 4 & 0x07,
					ksld_tl_adjust >> 4,
					ksld_tl_adjust & 0x0f,
					ame_ar_vel_sens_d1r >> 7,
					ame_ar_vel_sens_d1r >> 5 & 0x03,
					ame_ar_vel_sens_d1r & 0x1f
				);
				if(j == 1) {
					fb01_parameter_change(t, 0x51 + k * 8, ksl0_tl_vel_sens);
					fb01_parameter_change(t, 0x52 + k * 8, ksld_tl_adjust);
					fb01_parameter_change(t, 0x55 + k * 8, ame_ar_vel_sens_d1r);
				}
			}

			midi_track_write_note_on(t, j == 1 ? 12 : 0, 0, 64, j);
			midi_track_write_note_off(t, 12, 0, 64, j);
			printf("\n");
		}
	}

	midi_track_write_track_end(t, 48);

	struct file_stream fs;
	if(file_stream_init(&fs, "fb01spam.mid", "wb")) {
		fprintf(stderr, "Could not open fb01spam.mid\n");
		return 1;
	}

	midi_file_write(&f, (struct stream *)&fs);

	if(stream_close((struct stream *)&fs)) {
		fprintf(stderr, "Could not close file\n");
		return 1;
	}

	return 0;
}
