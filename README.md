FM voice loading, saving and conversion library
===============================================

| Param        | .OPM  | .DMP | .Y12 | .INS |
|--------------|:-----:|:----:|:----:|:----:|
| Name         |   ✅  |      |      |      |
| LFO Freq     |   ✅  |      |      |      |
| AMD          |   ✅  |      |      |      |
| PMD          |   ✅  |      |      |      |
| WF           |   ✅  |      |      |      |
| Noise Freq   |   ✅  |      |      |      |
| Pan          |   ✅  |      |      |      |
| Feedback     |   ✅  |      |      |      |
| Connection   |   ✅  |      |      |      |
| AMS          |   ✅  |      |      |      |
| PMS          |   ✅  |      |      |      |
| Slot Mask    |   ✅  |      |      |      |
| Noise Enable |   ✅  |      |      |      |
| AR           |   ✅  |      |      |      |
| D1R          |   ✅  |      |      |      |
| D2R          |   ✅  |      |      |      |
| RR           |   ✅  |      |      |      |
| D1L          |   ✅  |      |      |      |
| TL           |   ✅  |      |      |      |
| KS           |   ✅  |      |      |      |
| MUL          |   ✅  |      |      |      |
| DT1          |   ✅  |      |      |      |
| DT2          |   ✅  |      |      |      |
| AMS Enable   |   ✅  |      |      |      |

OPM files
---------

Reading:

```C
uint8_t *data; // Have your data in this buffer
size_t data_len; // with this length
struct opm_file opm;
int r = opm_file_load(&opm, data, data_len);
if(r != OPM_FILE_SUCCESS) {
	fprintf(stderr, "Could not load file: %s (%d)\n", opm_file_error_string(r), r);
	exit(1);
}
```

Writing:
```C
size_t write_fn(void *buf, size_t bufsize, void *data_ptr) {
	FILE *f = (FILE *)data_ptr;
	return fwrite(buf, 1, bufsize, f);
}

struct opm_file opm;
opm_file_init(&opm);

FILE *f = fopen("out.opm", "w");
int opm_file_save(&opm, write_fn, f);
fclose(f);
```

TFI files
---------

TFI files are 42 byte 4-operator OPN FM voices for [Shiru](https://shiru.untergrund.net/index.shtml)'s [TFM Music Maker](https://www.pouet.net/prod.php?which=53467).

Voice libraries:

* The archive [tfmmaker10.rar](http://www.nedopc.com/TURBOSOUND/TFMMAKER/tfmmaker10.rar) from [this thread](http://gendev.spritesmind.net/forum/viewtopic.php?t=125) contains some `.tfi` and `.y12` voice files.
* The archive [tfmmaker152.rar](https://ftp.untergrund.net/users/havoc/POUET/dropbox_backup/tfmmaker152.rar) from [here](https://www.pouet.net/prod.php?which=53467) contains `.tfi`, `.y12` and `.ins` files.

Specification:

* [VGMRips Specification](https://vgmrips.net/wiki/TFI_File_Format)
* [Other specification, includes VGI](https://plutiedev.com/format-tfi)
