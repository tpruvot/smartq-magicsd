/****************************************************************
 * $ID: font_zh.c       Sun, 15 Feb 2009 14:48:00 +0800  root $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  (Guoqiang Wang)  <wgq@hhcn.com>			*
 *                                                              *
 * Copyright (C)  2009  HHTech                                  *
 *   www.hhcn.com, www.hhcn.org                                 *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 *                                                              *
 * Last modified: Tue, 10 Mar 2009 15:23:47 +0800       by root #
 ****************************************************************/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define MENU_ITEM_WIDTH			280
#define MENU_ITEM_HEIGHT		28
#define FONT_PATH			"/etc/font"
#define CH_FONT_FILE_1212		FONT_PATH"/shzk1616"    // gb2312 font
#define EN_FONT_FILE_0612		FONT_PATH"/asc0816"     // English font
#define ASC_FONT_FILE_1212		FONT_PATH"/asc1616"     // asc font 

#define SAFE_MALLOC	malloc
#define SAFE_FREE	free
#define ERROR		fprintf
#define USE_ZH_CN_LANG

#ifdef USE_ZH_TW_LANG
static int gbktab_size = 0;
static uint8_t *gbktab_data = NULL;
#endif
#if defined(USE_ZH_CN_LANG) || defined(USE_ZH_TW_LANG)
static FILE *ch_font_1212_fp = NULL;
#endif
static FILE *en_font_0612_fp = NULL;
static FILE *asc_font_1212_fp = NULL;
static short masktab[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
RGBLCD COLOR_MASK = {168, 168, 168};  
static uint8_t *icon_buf, *sb_buf;

static void alloc_swap_buffer ()
{
    int s;
    s = MENU_ITEM_HEIGHT * MENU_ITEM_HEIGHT * FB_BYTES_PER_PIXEL + 16;
    sb_buf = (uint8_t *)SAFE_MALLOC (s);
    s = MENU_ITEM_HEIGHT * MENU_ITEM_HEIGHT * FB_BYTES_PER_PIXEL * 4 + 16;
    icon_buf = (uint8_t *)SAFE_MALLOC (s);
}

static void free_swap_buffer ()
{
    SAFE_FREE (icon_buf);
    SAFE_FREE (sb_buf);
}

void close_font ()
{
#if defined(USE_ZH_CN_LANG) || defined(USE_ZH_TW_LANG)
    if (ch_font_1212_fp) {
	fclose (ch_font_1212_fp);
	ch_font_1212_fp = NULL;
    }
#endif

#ifdef USE_ZH_TW_LANG
    if (gbktab_data) {
	SAFE_FREE (gbktab_data);
	gbktab_size = 0;
    }
#endif
    if (en_font_0612_fp) {
	fclose (en_font_0612_fp);
	en_font_0612_fp = NULL;
    }
    if (asc_font_1212_fp) {
	fclose (asc_font_1212_fp);
	asc_font_1212_fp = NULL;
    }
    free_swap_buffer ();
}

int open_font (screen_buffer *sb)   
{   
#if defined(USE_ZH_CN_LANG) || defined(USE_ZH_TW_LANG)
    if( (ch_font_1212_fp = fopen(CH_FONT_FILE_1212, "rb")) == NULL) {
	ERROR (stderr, "open %s failed", CH_FONT_FILE_1212);
	return -1;
    }
#endif
#ifdef USE_ZH_TW_LANG
    FILE *fp;
    int len;

    if((fp = fopen(BIG2GBK_TABLE, "rb")) == NULL) {
	ERROR (stderr, "open %s failed", BIG2GBK_TABLE);
	fclose (ch_font_1212_fp);
	return -1;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    gbktab_size = len + 2;
    gbktab_data = (uint8_t *)SAFE_MALLOC(gbktab_size);
    if(fread(gbktab_data, len, 1, fp) < 0) {
	ERROR (stderr, "read %s failed", BIG2GBK_TABLE);
	fclose(fp);
	goto failed;
    }

    fclose(fp);
#endif
    if(!(en_font_0612_fp = fopen(EN_FONT_FILE_0612, "rb"))) {
	ERROR (stderr, "open %s failed", EN_FONT_FILE_0612);
	goto failed;
    }

    if(!(asc_font_1212_fp = fopen(ASC_FONT_FILE_1212, "rb"))) {
	ERROR (stderr, "open %s failed", ASC_FONT_FILE_1212);
	goto failed;
    }

    alloc_swap_buffer ();
    return 0;
failed:
    close_font ();
    return -1;
}

static int draw_matrix (screen_buffer *sb, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
	const uint8_t *pixel, const RGBLCD *c)
{
    int i, j, pos;
    int word, bit, draw;
    uint8_t *dst;

    assert (sb);
    assert (pixel);
    /*
       if (x + w > sb->width || y + h > sb->height) {
       ERROR (stderr, "invalid position (%d:%d) and size (%d,%d)", x, y, w, h);
       return -1; 
       }
     */
    for (i = 0; i < h; i++) {
	dst = sb->buffer + (sb->width * (i +  y) + x) * FB_BYTES_PER_PIXEL;
	pos = ((w - 1) / 8 + 1) * i;
	for (j = 0; j < w; j++) {
	    word = j / 8;
	    bit = j % 8;
	    draw = (pixel[word+pos] & masktab[bit]) ? 1 : 0;
	    if(sb->bytes_per_pixel == 2) {
		if (draw) {
		    *dst = (((c->r) & 0x1f) << 3) | (((c->g) & 0x38) >> 3);
		    *(dst + 1) = (((c->g) & 0x7) << 5) | ((c->b) & 0x1f);
		}
		dst += 2;
	    } else {
		if (draw) {
		    *dst = c->r;
		    *(dst + 1) = c->g;
		    *(dst + 2) = c->b;
		    *(dst + 3) = 0;
		}
		dst += 4;
	    }
	}
    }
    return w;
}

static int draw_big_matrix (screen_buffer *sb, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
	const uint8_t *pixel, const RGBLCD *c)
{
    int i, j, iw, ih;
    screen_buffer tmp_sb;
    uint8_t *dst, *p, *even, *odd;

    memset (&tmp_sb, 0, sizeof (screen_buffer));

    tmp_sb.buffer = sb_buf;
    tmp_sb.width = w + 1;
    tmp_sb.height = h + 1;
    iw = w * 2;
    ih = h * 2;

    fill_rect (&tmp_sb, 0, 0, tmp_sb.width, tmp_sb.height, &COLOR_MASK);
    draw_matrix (&tmp_sb, 0, 0, w, h, pixel, c);

#define scale_pixel(dst,p,k) \
    memcpy(dst + k * 2 * FB_BYTES_PER_PIXEL, p + k * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL); \
    if (!memcmp(p + k * FB_BYTES_PER_PIXEL, p + (k + 1) * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL)) \
    memcpy(dst + (k * 2 + 1) * FB_BYTES_PER_PIXEL, p + k * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL); \
    else {\
	*(dst + (k * 2 + 1) * FB_BYTES_PER_PIXEL) = ((*(p+k*3))>>1) + ((*(p+(k+1)*3))>>1);  \
	*(dst + (k * 2 + 1) * FB_BYTES_PER_PIXEL + 1) = ((*(p+k*3+1))>>1) + ((*(p+(k+1)*3+1))>>1); \
	*(dst + (k * 2 + 1) * FB_BYTES_PER_PIXEL + 2) = ((*(p+k*3+2))>>1) + ((*(p+(k+1)*3+2))>>1); \
    }

#define scale_pixel2(dst,even,odd,j) \
    if (!memcmp(dst + j * 2 * FB_BYTES_PER_PIXEL, odd + j * 2 * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL)) \
    memcpy(even + j * 2 * FB_BYTES_PER_PIXEL, dst + j * 2 * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL); \
    else {\
	*(even+j*2*FB_BYTES_PER_PIXEL) = ((*(dst+j*2*FB_BYTES_PER_PIXEL)) >> 1)+((*(odd+j*2*FB_BYTES_PER_PIXEL)) >> 1);\
	*(even+j*2*FB_BYTES_PER_PIXEL+1) = ((*(dst+j*2*FB_BYTES_PER_PIXEL+1)) >> 1)+((*(odd+j*2*FB_BYTES_PER_PIXEL+1)) >> 1);\
	*(even+j*2*FB_BYTES_PER_PIXEL+2) = ((*(dst+j*2*FB_BYTES_PER_PIXEL+2)) >> 1)+((*(odd+j*2*FB_BYTES_PER_PIXEL+2)) >> 1);\
    } \
    if (!memcmp(dst + (j*2+1)*FB_BYTES_PER_PIXEL, odd + (j*2+1)*FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL)) \
    memcpy(even + (j*2+1)*FB_BYTES_PER_PIXEL, dst + (j*2+1)*FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL); \
    else {\
	*(even+(j*2+1)*FB_BYTES_PER_PIXEL)=((*(dst+(j*2+1)*FB_BYTES_PER_PIXEL))>>1)+((*(odd+(j*2+1)*FB_BYTES_PER_PIXEL))>>1);\
	*(even+(j*2+1)*FB_BYTES_PER_PIXEL+1)=((*(dst+(j*2+1)*FB_BYTES_PER_PIXEL+1))>>1)+((*(odd+(j*2+1)*FB_BYTES_PER_PIXEL+1))>>1);\
	*(even+(j*2+1)*FB_BYTES_PER_PIXEL+2)=((*(dst+(j*2+1)*FB_BYTES_PER_PIXEL+2))>>1)+((*(odd+(j*2+1)*FB_BYTES_PER_PIXEL+2))>>1);\
    }


    p = sb_buf;
    dst = icon_buf;
    for (j = 0; j < w - 1; j++) {
	scale_pixel (dst, p, j);
    }

    memcpy(dst + j * 2 *FB_BYTES_PER_PIXEL, p + j * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL);
    memcpy(dst + (j * 2 + 1) * FB_BYTES_PER_PIXEL, p + j * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL);

    for (i = 1; i < h; i++) {
	p = sb_buf + tmp_sb.width * i * FB_BYTES_PER_PIXEL;
	dst = icon_buf + iw * (i - 1) * 2 * FB_BYTES_PER_PIXEL;
	even = icon_buf + iw * (i * 2 - 1) * FB_BYTES_PER_PIXEL;
	odd = icon_buf + iw * i * 2 * FB_BYTES_PER_PIXEL;

	for (j = 0; j < w; j++) {
	    scale_pixel (odd, p, j);
	    scale_pixel2 (dst, even, odd, j);
	}

	memcpy(odd + j * 2 *FB_BYTES_PER_PIXEL, p + j * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL);
	memcpy(dst + (j * 2 + 1) * FB_BYTES_PER_PIXEL, p + j * FB_BYTES_PER_PIXEL, FB_BYTES_PER_PIXEL);

	scale_pixel2 (dst, even, odd, j);
    }
    memcpy (icon_buf + (ih - 1) * iw * FB_BYTES_PER_PIXEL,
	    icon_buf + (ih - 2) * iw * FB_BYTES_PER_PIXEL,
	    iw * FB_BYTES_PER_PIXEL);

    for (i = 0; i < ih; i++) {
	dst = sb->buffer + ((y + i) * sb->width + x) * FB_BYTES_PER_PIXEL;
	p = icon_buf + (i * iw) * FB_BYTES_PER_PIXEL;
	for (j = 0; j < iw; j++) {
	    if (memcmp(&COLOR_MASK, p, FB_BYTES_PER_PIXEL)) {
		memcpy(dst, p, FB_BYTES_PER_PIXEL);
	    }
	    p += FB_BYTES_PER_PIXEL;
	    dst += FB_BYTES_PER_PIXEL;
	}
    }
    return w * 2;
}

static int draw_font_from_file (screen_buffer *sb, uint16_t x, uint16_t y, 
	uint16_t w, uint16_t h, const RGBLCD *c, FILE *fp, int offset, int scale)
{
    int size;
    uint8_t pixel[32];

    if (!fp)
	return w;
    size = h * (((w - 1) / 8) + 1);
    if (fseek(fp, offset, SEEK_SET) == -1){
	ERROR (stderr, "fseek error");
	return -1;
    }	

    fread (pixel, size, 1, fp);
    if (ferror (fp)) {
	ERROR (stderr, "fread error");
	return -1;
    }

    if (scale == 1)
	return draw_matrix (sb, x, y, w, h, pixel, c);
    else if (scale == 2)
	return draw_big_matrix (sb, x, y, w, h, pixel, c);
    else
	return -1;
}

static int draw_0612_en_character (screen_buffer *sb, uint16_t x, uint16_t y, char c, 
	RGBLCD *color, int scale)
{
    int w, h, offset;

    w = 8;
    h = 16;
    offset = c * 16;

    return draw_font_from_file(sb, x, y, w, h, color, en_font_0612_fp, offset, scale);
}

static inline int draw_0612_asc_character (screen_buffer *sb, uint16_t x, uint16_t y, 
	char c, RGBLCD *color, int scale)
{
    int w, h, offset;

    w = h = 16;
    offset = c * 32;

    return draw_font_from_file(sb, x, y, w, h, color, asc_font_1212_fp, offset, scale);
}

#ifdef USE_ZH_CN_LANG
static inline int draw_0612_character (screen_buffer *sb, uint16_t x, uint16_t y, 
	uint8_t *str, RGBLCD *color, int scale)
{
    int w, h, offset;

    w = h = 16;
    offset = ((str[0] - 0x81) * 190 + (str[1] - 0x40) - (str[1] / 128)) * 32;

    return draw_font_from_file(sb, x, y, w, h, color, ch_font_1212_fp, offset, scale);
}
#endif
#ifdef USE_ZH_TW_LANG
static inline int draw_0612_zh2_character (screen_buffer *sb, uint16_t x, uint16_t y, 
	uint8_t *str, RGBLCD *color, int scale)
{
    int w, h, tab, offset;

    w = h = 16;
    tab = ((str[0] - 0xA1) * 157 + (str[1] - 0xA1 + 63)) * 2;
    if (tab > gbktab_size) 
	offset = 6176 * 32;
    else
	offset = *(uint16_t *)&gbktab_data[tab] * 32;

    return draw_font_from_file(sb, x, y, w, h, color, ch_font_1212_fp, offset, scale);
}
static inline int draw_0612_zh1_character (screen_buffer *sb, uint16_t x, uint16_t y, 
	uint8_t *str, RGBLCD *color, int scale)
{
    int w, h, tab, offset;

    w = h = 16;
    tab = ((str[0] - 0xA1) * 157 + (str[1] - 0x40)) * 2; 
    if (tab > gbktab_size) 
	offset = 6176 * 32;
    else
	offset = *(uint16_t *)&gbktab_data[tab] * 32;

    return draw_font_from_file(sb, x, y, w, h, color, ch_font_1212_fp, offset, scale);
}
#endif

int draw_string (screen_buffer *sb, int type, int x, int y, const font_type *font,	
	const char *str, RGBLCD *c, int scale, int board)
{
    uint16_t w, h;
    int i, len, width_x, pos;

    if (!str || !sb || x >= sb->width || y >= sb->height)
	return 0;

    w = font->w;
    h = font->h;
    len = strlen ((const char*)str);

    memset(sb->buffer + (y * sb->width + x) * FB_BYTES_PER_PIXEL, 0, h * sb->width * FB_BYTES_PER_PIXEL);
    pos = x;
    switch (type) {
	case CHARACTER_ZK:
	    for (i = 0; i <	len && pos < board; pos += width_x) {
#ifdef USE_ZH_TW_LANG
		if (str[i] < 160) {
		    width_x = draw_0612_en_character (sb, pos, y, str[i], c, scale);
		    i++;
		    continue;
		} 
		if ((str[i] >= 0xA0 && str[i + 1] >= 0x40 && str[i + 1] <= 0x7E) && 
			font->font_enum == S0612 ) {
		    width_x = draw_0612_zh1_character (sb, pos, y, (uint8_t*)&str[i], c, scale);
		    i += 2;
		    continue;
		}
		if (str[i] >= 0xA0 && str[i + 1] >= 0xA1 && str[i + 1] <= 0xFE && 
			font->font_enum == S0612 ) {
		    width_x = draw_0612_zh2_character (sb, pos, y, (uint8_t*)&str[i], c, scale);
		    i += 2;
		    continue;
		}
#else
#ifdef USE_ZH_CN_LANG
		if (((uint8_t)str[i] >= 0x81 && (uint8_t)str[i+1] >= 0x40) && 
			font->font_enum == S0612) {
		    width_x = draw_0612_character (sb, pos, y, (uint8_t*)&str[i], c, scale);
		    i += 2;
		    continue;
		}

		if ((uint8_t)str[i] < 161) {
		    width_x = draw_0612_en_character (sb, pos, y, str[i], c, scale);
		    i++;
		    continue;
		} 
#endif
		ERROR (stderr, "unknow font");
		return -1;
#endif
	    }
	    break;
	case CHARACTER_MATRIX:
	    if (font->font_enum != S1212) {
		ERROR (stderr, "unknow font");
		return -1;
	    }

	    for (i = 0; i < len; i++) {
		assert ((str[i] & 0x80) == 0);
		width_x = draw_0612_asc_character (sb, pos, y, str[i], c, scale);
	    }
	    break;
    }

    return pos - x;
}
/******************** End Of File: font_zh.c ********************/
// vim:sts=4:ts=8: 
