/****************************************************************
 * $ID: rw_sd_inand.c   Tue, 10 Feb 2009 15:28:21 +0800  root $ *
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
 * Last modified: Thu, 02 Apr 2009 18:44:05 +0800       by root #
 ****************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/soundcard.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <math.h>
#include <linux/fs.h>
#include <sys/wait.h>
#include "uncompress.h"

typedef struct _screen_buffer {
    short width;		 // mem width
    short height;		  // mem high
    int bytes_per_pixel;
    unsigned int fb_size;
    unsigned char *buffer;
    unsigned char *fb;
    int dev_fd;
}screen_buffer;

typedef struct __attribute__ ((packed)) tagRGB565 {		// color define
    unsigned char r;	// red
    unsigned char g;	// green
    unsigned char b;	// blue
}RGBLCD;

typedef struct _font_type {
    int font_enum;
    unsigned short w;
    unsigned short h;
}font_type;

enum {
    SXX = 0,
    S0612,
    S1212,
    S1824
};

enum {
    CHARACTER_ZK = 0,
    CHARACTER_MATRIX,
    BITMAP_MATRIX
};

static RGBLCD COLOR_WHITE = {255, 255, 255};
static RGBLCD COLOR_GREEN = {0, 255, 0};
static RGBLCD COLOR_RED	= {255, 0, 0};
static RGBLCD COLOR_BAR = {75, 237, 41};
static RGBLCD COLOR_TRIM = {200, 200, 200};
static RGBLCD COLOR_BAR_16 = {75, 237, 41};
static RGBLCD COLOR_TRIM_16 = {200, 200, 200};

font_type font_0612 = {S0612, 8, 16};

#define PROGRESS_BAR_X_OFFSET	60
#define PROGRESS_BAR_Y_OFFSET	240
#define LOADING_TEXT_X_OFFSET	30
#define LOADING_TEXT_Y_OFFSET	100
#define PROGRESS_BAR_WIDTH	(800 - 2 * PROGRESS_BAR_X_OFFSET)
#define PROGRESS_BAR_HEIGHT	15
#define PROGRESS_BAR_TRIM	2

#define FB_DEV_PATH			"/dev/fb0"
#define FB_WIDTH			800
#define FB_HEIGHT			480
#define FB_BYTES_PER_PIXEL		4
#define FB_LOGO_HEIGHT			100
#define FB_BYTES_OF_LOGO		(FB_WIDTH * FB_LOGO_HEIGHT * FB_BYTES_PER_PIXEL)

#define EXT3_HOME_SECTORS		(256 * 1024 * 2)    // 256MB
#define EXT3_SWAP_SECTORS		(128 * 1024 * 2)    // 128MB
#define EXT3_ZIMAGE_INITRAMFS_SECTORS	(16 * 1024 * 2)	    // 16MB
#define INAND_BLOCK_SIZE		512		    // 512B
#define FDS_PATH			"/etc/_fds"
#define FDS2_PATH			"/etc/_fds2"

static screen_buffer *sb = NULL;
static int fbmemlen;
static int loading_homefs = 0;
static float ratio, old_ratio;

int open_screen(void)
{
    struct fb_var_screeninfo fb_vinfo;

    sb = (screen_buffer*)malloc(sizeof (screen_buffer));
    if ((sb->dev_fd = open(FB_DEV_PATH, O_RDWR)) == -1) {
	perror("open");
	return -1;
    }

    if (ioctl(sb->dev_fd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
	sb->width = FB_WIDTH;
	sb->height = FB_HEIGHT;
	sb->bytes_per_pixel = FB_BYTES_PER_PIXEL;
    } else {
	sb->width = fb_vinfo.xres;
	sb->height = fb_vinfo.yres;
	sb->bytes_per_pixel = fb_vinfo.bits_per_pixel / 8;
    }
    if(sb->bytes_per_pixel == 3)
	sb->bytes_per_pixel = 4;

    fbmemlen = sb->width * sb->height * sb->bytes_per_pixel;
    fprintf(stderr, "fbmemlen = %d\n", fbmemlen);

    if ((sb->buffer = (uint8_t *) mmap(0, fbmemlen, PROT_READ | PROT_WRITE, MAP_SHARED, sb->dev_fd, 0)) == (uint8_t *) -1) 
    {
	fprintf (stderr, "rw_sd_inand.c: Can't mmap frame buffer ++\n");
	exit (1);
    }
    memset(sb->buffer, 0, fbmemlen);

    return 0;
}

static inline void put_pixel (int x, int y, RGBLCD *v)
{
    if(sb->bytes_per_pixel == 4) {
	uint8_t *dst = sb->buffer + (sb->width * y + x) * sb->bytes_per_pixel;
	*dst++ = v->r; *dst++ = v->g; *dst++ = v->b; *dst++ = 0;
    } else {
	uint16_t *dst = (uint16_t *)(sb->buffer + (sb->width * y + x) * sb->bytes_per_pixel);
	*dst++ = (((v->r) & 0x1f) << 11) | (((v->g) & 0x2f) << 5) | ((v->b) & 0x1f);
    }
}

static inline void draw_horizontal_line2 (int x, int y, int len, RGBLCD *v)
{
    int i;
    put_pixel (x, y, v);
    put_pixel (x + 1, y, v);
    put_pixel (x + len - 2, y, v);
    put_pixel (x + len - 1, y, v);
}

static inline void draw_horizontal_line (int x, int y, int len, RGBLCD *v)
{
    int i;
    for (i = 0; i < len; i++)
	put_pixel (x + i, y, v);
}

static void fill_rect (screen_buffer *sb, int dx, int dy, int w, int h, RGBLCD *v)
{
    int y;
    for (y = dy; y < dy + h; y++)
	draw_horizontal_line (dx, y, w, v);
}

static void fill_broken_rect (screen_buffer *sb, int dx, int dy, int w, int h, RGBLCD *v)
{
    int y;
    //for (y = dy; y < dy + h; y++)
    //	draw_horizontal_line (dx, y, w, v);
    draw_horizontal_line (dx, dy, w, v);
    draw_horizontal_line (dx, dy + 1, w, v);
    for (y = dy + 2; y < dy + h - 2; y++)
	draw_horizontal_line2 (dx, y, w, v);
    draw_horizontal_line (dx, dy + h - 2, w, v);
    draw_horizontal_line (dx, dy + h - 1, w, v);
}

static void draw_progress_bar(float ratio, /* ratio: 0.0 ~ 1.0 */
	int x, int y, int width, int height, int trim,
	RGBLCD *bar_color, RGBLCD *trim_color)
{
    int bar_x = x + trim;
    int bar_y = y + trim;
    int bar_width = width - trim * 2;
    int bar_height = height - trim * 2;

    //fill_rect(sb, x, y, width, height, trim_color);

    if(ratio <= 0.0)
	return;
    if(ratio > 1.0)
	ratio = 1.0;

    bar_width *= ratio;
    fill_rect(sb, bar_x, bar_y, bar_width, bar_height,
	    bar_color);
}

static void close_screen(screen_buffer *sb)
{
    if(!sb)
	return;
    // Unmap the framebuffer
    munmap(sb->buffer, fbmemlen);
    // Close framebuffer device
    close(sb->dev_fd);
    free(sb);
}

#include "font_zh.c"

static char system_cmd[100];
static unsigned char *buffer = NULL;

int main( int argc, char *argv[] )
{
    int i, ret = 0;

    if( 0 != open_screen() ) {
	fprintf(stderr, "open_screen failed\n");
	return -1;
    } else
	fprintf(stderr, "open_screen success\n");

    if (ret = open_font(sb) == -1)
	goto fail1;
    fprintf(stderr, "open_font success\n");

    memset(sb->buffer, 0xff, fbmemlen);
    system("/usr/bin/fv /etc/logo.jpg");
    memset(sb->buffer, 0x0, fbmemlen);
    sprintf(system_cmd, "echo 50 > /sys/devices/platform/s3c2410-lcd/backlight_level");
    system(system_cmd);  // Turn on the LCD backlight
    fprintf(stderr, system_cmd);

    system("./etc/rootfs.sh");
    system("ifconfig");

    draw_string (sb, CHARACTER_ZK, LOADING_TEXT_X_OFFSET, LOADING_TEXT_Y_OFFSET, &font_0612, "初始化成功，可以telnet到本机了!\n", &COLOR_WHITE, 1, sb->width);
    close_font();
fail1:
    close_screen (sb);
    return ret;
}

/****************** End Of File: debug.c ******************/
// vim:sts=4:ts=8: 
