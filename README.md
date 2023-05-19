FM voice loading, saving and conversion library
===============================================

Code for loading and saving various FM voice formats meant for the Yamaha YM series of chips (OPL, OPM, OPN etc.), format autodetection code, and conversion between the various formats.

This library is meant to be used in [fmtoy](https://github.com/vampirefrog/fmtoy) and [vgm2x](https://github.com/vampirefrog/vgm2x).

File Formats
------------

| Format | Type | Op | Max voices |
|-------:|:----:|:--:|:----------:|
| [INS](https://vgmrips.net/wiki/INS_File_Format) | OPN | 4 | 1 |
| [DMP](https://vgmrips.net/wiki/DMP_File_Format) | OPN | 4 | 1 |
| [OPM](https://vgmrips.net/wiki/OPM_File_Format) | OPM | 4 | unlimited |
| [TFI](https://vgmrips.net/wiki/TFI_File_Format) | OPN | 4 | 1 |
| [Y12](https://vgmrips.net/wiki/Y12_File_Format) | OPN | 4 | 1 |
| [SBI](https://vgmrips.net/wiki/SBI_File_Format) | OPL | 2 | 1 |
| [INS](https://moddingwiki.shikadi.net/wiki/AdLib_Instrument_Format) | OPL | 2 | 1 |
| [BNK](https://moddingwiki.shikadi.net/wiki/AdLib_Instrument_Bank_Format) | OPL | 2 | 65536 |
| [IBK](https://moddingwiki.shikadi.net/wiki/IBK_Format) | OPL | 2 | 128 |
| [CMF](https://moddingwiki.shikadi.net/wiki/CMF_Format) | OPL | 2 | 128/256/65536 |

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
