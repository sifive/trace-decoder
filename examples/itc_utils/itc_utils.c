/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "itc_utils.h"

//#define	baseAddress	0x20007000
#define	baseAddress	0x10000000

// Register Offsets

#define Offset_teControl		0x0000
#define Offset_teImpl			0x0004
#define	Offset_teSinkBase		0x0010
#define Offset_teSinkBaseHigh	0x0014
#define Offset_teSinkLimit		0x0018
#define Offset_teSinkWP			0x001c
#define Offset_teSinkRP			0x0020
#define	Offset_teSinkData		0x0024
#define	Offset_tsControl		0x0040
#define Offset_tsLower			0x0044
#define Offset_tsUpper			0x0048
#define Offset_xtiControl		0x0050
#define	Offset_xtoControl		0x0054
#define	Offset_wpControl		0x0058
#define Offset_itcTraceEnable	0x0060
#define	Offset_itcTrigEnable	0x0064
#define	Offset_itcStimulus		0x0080
#define	Offset_atbSink			0x0e00
#define Offset_pibSink			0x0f00

// Register Address and types

#define teControl		((uint32_t*)(baseAddress+Offset_teControl))
#define teImpl			((uint32_t*)(baseAddress+Offset_teImpl))
#define	teSinkBase		((uint32_t*)(baseAddress+Offset_teSinkBase))
#define teSinkBaseHigh	((uint32_t*)(baseAddress+Offset_teSinkBaseHigh))
#define teSinkLimit		((uint32_t*)(baseAddress+Offset_teSinkLimit))
#define teSinkWP		((uint32_t*)(baseAddress+Offset_teSinkWP))
#define teSinkRP		((uint32_t*)(baseAddress+Offset_teSinkRP))
#define	teSinkData		((uint32_t*)(baseAddress+Offset_teSinkData))
#define	tsControl		((uint32_t*)(baseAddress+Offset_tsControl))
#define tsLower			((uint32_t*)(baseAddress+Offset_tsLower))
#define tsUpper			((uint32_t*)(baseAddress+Offset_tsUpper))
#define xtiControl		((uint32_t*)(baseAddress+Offset_xtiCtonrol))
#define	xtoControl		((uint32_t*)(baseAddress+Offset_xtoCtonrol))
#define	wpControl		((uint32_t*)(baseAddress+Offset_wpControl))
#define itcTraceEnable	((uint32_t*)(baseAddress+Offset_itcTraceEnable))
#define	itcTrigEnable	((uint32_t*)(baseAddress+Offset_itcTrigEnable))
#define	itcStimulus		((uint32_t*)(baseAddress+Offset_itcStimulus))
#define	atbSink			((uint32_t*)(baseAddress+Offset_atbSink))
#define pibSink			((uint32_t*)(baseAddress+Offset_pibSink))

static int enable_itc(int channel)
{
	if ((channel < 0) || (channel > 31)) {
		return 1;
	}

	*itcTraceEnable |= (1 << channel);

	return 0;
}

static int init_itc(int channel)
{
	static uint32_t inited_mask = 0;

	if (inited_mask & (1 << channel) == 0) {
		int rc = enable_itc(channel);
		if (rc == 0) {
			inited_mask |= (1 << channel);
		}

		return rc;
	}

	return 0;
}

static inline void write_itc(int channel,uint32_t data)
{
	itcStimulus[channel] = data;
}

static inline void write_itc_uint8(int channel,uint8_t data)
{
	uint8_t *itc_uint8 = (uint8_t*)&itcStimulus[channel];
	itc_uint8[3] = data;
}

static inline void write_itc_uint16(int channel,uint16_t data)
{
	uint16_t *itc_uint16 = (uint16_t*)&itcStimulus[channel];
	itc_uint16[1] = data;
}

// itc_fputs(): like itc_puts(), but no \n at end of text. Also takes the itc channel to write to

int itc_fputs(int channel,const char *f)
{
	int rc;

	rc = init_itc(channel);
	if (rc != 0) {
		return 0;
	}

	rc = strlen(f);

	int words = (rc/4)*4;
	int bytes = rc & 0x03;
	int i;
	uint16_t a;

    for (i = 0; i < words; i += 4) {
		write_itc(channel,*(uint32_t*)(f+i));
	}

    switch (bytes) {
    case 0:
    	break;
    case 1:
    	write_itc_uint8(channel,*(uint8_t*)(f+i));
    	break;
    case 2:
    	write_itc_uint16(channel,*(uint16_t*)(f+i));
    	break;
    case 3:
    	a = *(uint16_t*)(f+i);
    	write_itc_uint16(channel,a);

    	a = ((uint16_t)(*(uint8_t*)(f+i+2)));
    	write_itc_uint8(channel,(uint8_t)a);
    	break;
    }

	return rc;
}

// itc_puts(): like C puts(), includes \n at end of text

int itc_puts(const char *f)
{
	int rc;

	rc = init_itc(0);
	if (rc != 0) {
		return 0;
	}

	rc = strlen(f);

	int words = (rc/4)*4;
	int bytes = rc & 0x03;
	int i;
	uint16_t a;

    for (i = 0; i < words; i += 4) {
		write_itc(0,*(uint32_t*)(f+i));
	}

    // we actually have one more byte to send than bytes, because we need to append
    // a \n to the end

    switch (bytes) {
    case 0:
    	write_itc_uint8(0,'\n');
    	break;
    case 1:
    	a = *(uint8_t*)(f+i);
    	write_itc_uint16(0,('\n' << 8) | a);
    	break;
    case 2:
    	write_itc_uint16(0,*(uint16_t*)(f+i));
    	write_itc_uint8(0,'\n');
    	break;
    case 3:
    	a = *(uint16_t*)(f+i);
    	write_itc_uint16(0,a);

    	a = ((uint16_t)(*(uint8_t*)(f+i+2)));
    	write_itc_uint16(0,('\n' << 8) | a);
    	break;
    }

	return rc+1;
}

int itc_printf(const char *f, ... )
{
	char buffer[256];
	va_list args;
	int rc;

	va_start(args, f);
	rc = vsnprintf(buffer,sizeof buffer, f, args);
	va_end(args);

	itc_puts(buffer);
	return rc;
}

int itc_fprintf(int channel,const char *f, ... )
{
	char buffer[256];
	va_list args;
	int rc;

	va_start(args, f);
	rc = vsnprintf(buffer,sizeof buffer, f, args);
	va_end(args);

	itc_fputs(channel,buffer);
	return rc;
}
