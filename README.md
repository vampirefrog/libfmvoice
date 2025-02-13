FM voice loading, saving and conversion library
===============================================

![Project status](https://img.shields.io/badge/Project%20status-Alpha-blue.svg)

[![Linux Build](https://github.com/vampirefrog/libfmvoice/actions/workflows/linux.yml/badge.svg)](https://github.com/vampirefrog/libfmvoice/actions/workflows/linux.yml)

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

Structures
----------

OPL

<table>
	<thead>
		<tr><th rowspan="2">Field</th><th colspan="8">Bit</th><th rowspan="2">Description</th></tr>
		<tr><th>7</th><th>6</th><th>5</th><th>4</th><th>3</th><th>2</th><th>1</th><th>0</th></tr>
	</thead>
	<tbody>
		<tr>
			<td><code>ch_fb_cnt[0]</code></td>
			<td align="center">D</td>
			<td align="center">C</td>
			<td align="center">B</td>
			<td align="center">A</td>
			<td align="center" colspan="3">FB</td>
			<td align="center">CNT</td>
			<td align="center">Channel mask, feedback, connection</td>
		</tr>
		<tr>
			<td><code>dam_dvb_ryt_bd_sd_tom_tc_hh</code></td>
			<td align="center">AM</td>
			<td align="center">VIB</td>
			<td align="center">RYT</td>
			<td align="center">BD</td>
			<td align="center">SD</td>
			<td align="center">TOM</td>
			<td align="center">TC</td>
			<td align="center">HH</td>
			<td align="center"></td>
		</tr>
		<tr>
			<td><code>am_vib_eg_ksr_mul</code></td>
			<td align="center">AM</td>
			<td align="center">VIB</td>
			<td align="center">EGT</td>
			<td align="center">KSR</td>
			<td align="center" colspan="4">MUL</td>
			<td></td>
		</tr>
		<tr>
			<td><code>ksl_tl</code></td>
			<td align="center" colspan="2">KSL</td>
			<td align="center" colspan="6">TL</td>
			<td></td>
		</tr>
		<tr>
			<td><code>ar_dr</code></td>
			<td align="center" colspan="4">AR</td>
			<td align="center" colspan="4">DR</td>
			<td></td>
		</tr>
		<tr>
			<td><code>sl_rr</code></td>
			<td align="center" colspan="4">SL</td>
			<td align="center" colspan="4">RR</td>
			<td></td>
		</tr>
		<tr>
			<td><code>ws</code></td>
			<td align="center" colspan="5"></td>
			<td align="center" colspan="3">WS</td>
			<td>Waveform select</td>
		</tr>
	</tbody>
</table>

Conversion
----------

Conversion between OPL, OPM and OPN voices is only possible in some cases, in other cases it involves approximation, and in others it is not possible.

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

Building with emscripten
------------------------

```sh
emmake make libfmvoice.a
```

Formulae
--------

OPM LFO

$f_{LFO} = \frac{f_{master}}{(8 \times (256 - R))}$

OPN LFO

$f_{LFO} = \frac{f_{master}}{2^{LFO\\_DIV} \times (108 - LFO\\_FREQ)}$
