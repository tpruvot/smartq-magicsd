/****************************************************************
 * $ID: rw_sd_inand.c   Tue, 10 Feb 2009 15:28:21 +0800  root $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  (Guoqiang Wang)  <wgq@hhcn.com>            *
 *                                                              *
 * Copyright (C)  2009  HHTech                                  *
 *   www.hhcn.com, www.hhcn.org                                 *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it               *
 *   under the terms of the GNU General Public Licence (GPL).   *
 *                                                              *
 * Last modified: 二, 27 10月 2009 09:38:11 +0800     by root #
 ****************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
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
#include <linux/input.h>
#include "firmware_header.h"

static int KEY_ADD = 109;
static int KEY_DEC = 104;

typedef struct _screen_buffer {
    short width;         // mem width
    short height;          // mem high
    int bytes_per_pixel;
    unsigned int fb_size;
    unsigned char *buffer;
    unsigned char *fb;
    int dev_fd;
}screen_buffer;

typedef struct __attribute__ ((packed)) tagRGB565 {        // color define 
    unsigned char r;    // red 
    unsigned char g;    // green
    unsigned char b;    // blue 
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

typedef struct section {
   const char * name;
   size_t       stanzaOffset;
} Section;

static Section sects[MAX_SECTIONS] = {
   { "QI",        FW_STANZA_OFFSET(qi),        }, 
   { "UBOOT",     FW_STANZA_OFFSET(u_boot),    },    
   { "ZIMAGE",    FW_STANZA_OFFSET(zimage),    },
   { "INITRAMFS", FW_STANZA_OFFSET(initramfs), },
   { "ROOTFS",    FW_STANZA_OFFSET(rootfs),    },
   { "HOMEFS",    FW_STANZA_OFFSET(homefs),    },
   { "BOOTARGS",  FW_STANZA_OFFSET(bootArgs),  }
};

static RGBLCD COLOR_WHITE = {255, 255, 255};
static RGBLCD COLOR_GREEN = {0, 255, 0};
static RGBLCD COLOR_RED    = {255, 0, 0};
static RGBLCD COLOR_BAR = {75, 237, 41};
static RGBLCD COLOR_TRIM = {200, 200, 200};
static RGBLCD COLOR_BAR_16 = {75, 237, 41};
static RGBLCD COLOR_TRIM_16 = {200, 200, 200};

font_type font_0612 = {S0612, 8, 16};

#define PROGRESS_BAR_X_OFFSET    60
#define PROGRESS_BAR_ZH_OFFSET    240
#define LOADING_TEXT_X_OFFSET    30
#define LOADING_TEXT_ZH_OFFSET   80
#define LOADING_TEXT_EN_OFFSET   100
#define PROGRESS_BAR_WIDTH    (800 - 2 * PROGRESS_BAR_X_OFFSET)
#define PROGRESS_BAR_HEIGHT    15
#define PROGRESS_BAR_TRIM    2

#define FB_DEV_PATH            "/dev/fb0"
#define FB_WIDTH            800
#define FB_HEIGHT            480
#define FB_BYTES_PER_PIXEL        4
#define FB_LOGO_HEIGHT            100
#define FB_BYTES_OF_LOGO        (FB_WIDTH * FB_LOGO_HEIGHT * FB_BYTES_PER_PIXEL)

#define EXT3_HOME_SECTORS        (128 * 1024 * 2)    // 256MB
#define EXT3_SWAP_SECTORS        (128 * 1024 * 2)    // 128MB
#define EXT3_ZIMAGE_INITRAMFS_SECTORS    (16 * 1024 * 2)        // 16MB
#define RESERVED_SECTORS        (4 * 1024 * 2)        // 4MB
#define INAND_BLOCK_SIZE        512            // 512B

static screen_buffer *sb = NULL;
static int fbmemlen;
static int loading_homefs = 0;
static int keep_userzone = 0;
static float ratio, old_ratio;

void draw_string_en(char *str)
{
    draw_string (sb, CHARACTER_ZK, 
           LOADING_TEXT_X_OFFSET, LOADING_TEXT_EN_OFFSET, 
           &font_0612, str, &COLOR_WHITE, 1, sb->width);
}

void draw_string_zh(char *str)
{
    draw_string (sb, CHARACTER_ZK, 
          LOADING_TEXT_X_OFFSET, LOADING_TEXT_ZH_OFFSET, 
          &font_0612, str, &COLOR_WHITE, 1, sb->width);   
}

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
    //    draw_horizontal_line (dx, y, w, v);
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

#define    INAND_SIZE_PER_WRITE    (4 * 1024 * 1024)   // 4MB
#define    ROOTFS_SIZE_PER_WRITE    (256 * 1024)   // 256KB
static firmware_fileheader *fw_fh = NULL;
static int fd_sd, fd_inand;
static char inand_partition_path[30];
static char system_cmd[100];
static unsigned char *buffer = NULL;
static uint32_t inand_check_sum;
static uint32_t total_size_sum, size_sum, old_size_sum;

int loading(char *name_zh, char *name_en, uint32_t total_size, RGBLCD *bar_color, RGBLCD *trim_color)
{
    uint32_t read_size, size, remnant_size;
    char s[100];

    size = 0;
    remnant_size = total_size;
    while(remnant_size / INAND_SIZE_PER_WRITE > 0) {
    read_size = read(fd_sd, buffer, INAND_SIZE_PER_WRITE);
    size += read_size;
    size_sum += read_size;
    if(INAND_SIZE_PER_WRITE != write(fd_inand, buffer, INAND_SIZE_PER_WRITE)) {
        char err[120];
        sprintf(err, "inand write fails %s", strerror(errno)); 
        fprintf(stderr, "%s\n", err);
        draw_string_en(err);
#warning missing chinese
        sleep(5);
        return -1; 
    }
    ratio = size_sum / (float)total_size_sum;
    if((ratio - old_ratio) * 100 > 1) {
        sprintf(s, "%s %d%%\n", name_zh, (int)(ratio * 100));
        draw_string_zh(s);
        sprintf(s, "%s %d%%\n", name_en, (int)(ratio * 100));
        draw_string_en(s);
        draw_progress_bar(ratio, PROGRESS_BAR_X_OFFSET, PROGRESS_BAR_ZH_OFFSET,
            PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT,
            PROGRESS_BAR_TRIM,
            bar_color, trim_color);
        old_ratio = ratio;
    }
    remnant_size = total_size - size;
    }
    read_size = read(fd_sd, buffer, remnant_size);
    size += read_size;
    size_sum += read_size;
    write(fd_inand, buffer, remnant_size);
    ratio = size_sum / (float)total_size_sum;
    sprintf(s, "%s %d%%\n", name_zh, (int)(ratio * 100));
    draw_string_zh(s);
    sprintf(s, "%s %d%%\n", name_en, (int)(ratio * 100));
    draw_string_en(s);
    draw_progress_bar(ratio, PROGRESS_BAR_X_OFFSET, PROGRESS_BAR_ZH_OFFSET,
        PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT,
        PROGRESS_BAR_TRIM,
        bar_color, trim_color);
    old_ratio = ratio;

    return 0;

}

int loading_fs(char *name_zh, char *name_en, uint32_t total_size, RGBLCD *bar_color, RGBLCD *trim_color)
{
    uint32_t read_size, size, remnant_size;
    int ret;
    char s[100];
    int fd[2];
    pid_t pid;
    struct timeval tpStart, tpEnd;
    float timeUse;

    size = 0;
    remnant_size = total_size;
    gettimeofday(&tpStart, NULL);
    
    if(pipe(fd) < 0) {
        fprintf(stderr, "pipe error");
        return -1;
    }

    if((pid = fork()) < 0) {
        fprintf(stderr, "fork error");
        return -1;
    } else if(pid > 0) {    // parent
        close(fd[0]);    // close read end
        /* parent copies argv[1] to pipe */
        while(remnant_size / ROOTFS_SIZE_PER_WRITE > 0) {        
            read_size = read(fd_sd, buffer, ROOTFS_SIZE_PER_WRITE);
            size += read_size;
            size_sum += read_size;
            write(fd[1], buffer, ROOTFS_SIZE_PER_WRITE);
            ratio = size_sum / (float)total_size_sum;
            if((ratio - old_ratio) * 100 > 1) {
                sprintf(s, "%s %d%%\n", name_zh, (int)(ratio * 100));
                draw_string_zh(s);
                sprintf(s, "%s %d%%\n", name_en, (int)(ratio * 100));
                draw_string_en(s);
                draw_progress_bar(ratio, PROGRESS_BAR_X_OFFSET, PROGRESS_BAR_ZH_OFFSET,
                    PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT,
                    PROGRESS_BAR_TRIM,
                    bar_color, trim_color);
                old_ratio = ratio;
            }
            remnant_size = total_size - size;
        }
        read_size = read(fd_sd, buffer, remnant_size);
        size += read_size;
        size_sum += read_size;
        write(fd[1], buffer, remnant_size);
        ratio = size_sum / (float)total_size_sum;
        sprintf(s, "%s %d%%\n", name_zh, (int)(ratio * 100));
        draw_string_zh(s);
        sprintf(s, "%s %d%%\n", name_en, (int)(ratio * 100));
        draw_string_en(s);
        draw_progress_bar(ratio, PROGRESS_BAR_X_OFFSET, PROGRESS_BAR_ZH_OFFSET,
            PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT,
            PROGRESS_BAR_TRIM,
            bar_color, trim_color);
        old_ratio = ratio;
        remnant_size = total_size - size;

        close(fd[1]);    // close write end of pipe for reader
        if(waitpid(pid, NULL, 0) < 0) {
            fprintf(stderr, "waitpid error");
            return -1;
        }
        return 0;
    } else {        // child
        close(fd[1]);    // close write end
        if(fd[0] != STDIN_FILENO) {
            if(dup2(fd[0], STDIN_FILENO) != STDIN_FILENO) {
                fprintf(stderr, "dup2 error to stdin");
                return -1;
            }
            close(fd[0]);   // don't need this after dup2
        } 

        ret = WEXITSTATUS(system("/usr/bin/tar Jxf - -C /mnt/upgrade"));
        if (ret) {
       /* rut roh xz tar failed.  Try gz */
           ret = WEXITSTATUS(system("/usr/bin/tar Jxf - -C /mnt/upgrade"));
           if (ret) {
              fprintf(stderr,"un-tar of %s fails.\n", name_en);
           }
        }
    }

    gettimeofday(&tpEnd, NULL);
    timeUse = (tpEnd.tv_sec - tpStart.tv_sec);
    fprintf(stderr, "Used Time:%f\n", timeUse);
    return 0;
}

static unsigned long get_BLKGETSIZE_sectors(int fd)
{
    uint64_t v64;
    unsigned long longsectors;

    if (ioctl(fd, BLKGETSIZE64, &v64) == 0) {
    /* Got bytes, convert to 512 byte sectors */
    return (v64 >> 9);
    }
    /* Needs temp of type long */
    if (ioctl(fd, BLKGETSIZE, &longsectors))
    longsectors = 0;
    return longsectors;
}

static inline int get_sum(unsigned char *buffer, int buffer_size)
{
    int ret = 0, i;
    for(i = 0; i < buffer_size; i++)
    ret += buffer[i];
    return ret;
}

static uint32_t get_check_sum(int file_size)
{
    int remnant_size, size, ret = 0;
    remnant_size = file_size;
    size = 0;
    while(remnant_size / INAND_SIZE_PER_WRITE > 0) {
    size += read(fd_inand, buffer, INAND_SIZE_PER_WRITE);
    ret += get_sum(buffer, INAND_SIZE_PER_WRITE);
    remnant_size = file_size - size;
    }
    size += read(fd_inand, buffer, remnant_size);
    ret += get_sum(buffer, remnant_size);

    return ret;
}

static int loading_firmware(char *inand_device, RGBLCD *bar_color, RGBLCD *trim_color)
{
    char err[120];

    /* draw the external box of progress bar first */
    fill_broken_rect(sb, PROGRESS_BAR_X_OFFSET, PROGRESS_BAR_ZH_OFFSET, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, trim_color);
    old_ratio = ratio = 0;
    size_sum = 0;

    /* write the firmware_fileheader into the inand */
    lseek(fd_sd, 0, SEEK_SET);
    lseek(fd_inand, -EXT3_ZIMAGE_INITRAMFS_SECTORS * INAND_BLOCK_SIZE, SEEK_END);
    loading("正在升级... ", "Upgrading... ",  fw_fh->fh_size, bar_color, trim_color);
    /* calculate the checksum of firmware_fileheader1 */
    lseek(fd_inand, -EXT3_ZIMAGE_INITRAMFS_SECTORS * INAND_BLOCK_SIZE + 8, SEEK_END);
    inand_check_sum = get_check_sum(fw_fh->fh_size - 8);
    if(inand_check_sum != fw_fh->check_sum) { 
        sprintf(err, "file header1 xsum fail: expected %d calc'ed %d", 
                fw_fh->check_sum, inand_check_sum); 
        fprintf(stderr,"%s\n", err);

#warning missing chinese
        draw_string_en(err);
        sleep(5);
        return -1;

    }
    
    /* write the u-boot into the inand */
    lseek(fd_sd, (fw_fh->u_boot).file.offset, SEEK_SET);
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS + (fw_fh->u_boot).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    loading("正在升级... ", "Upgrading... ", (fw_fh->u_boot).file.size, bar_color, trim_color);
    /* calculate the checksum of u-boot1 */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS + (fw_fh->u_boot).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    inand_check_sum = get_check_sum((fw_fh->u_boot).file.size);
    if(inand_check_sum != (fw_fh->u_boot).check_sum) { 
        sprintf(err, "u_boot1 xsum fail: expected %d calc'ed %d", 
               (fw_fh->u_boot).check_sum, inand_check_sum); 
        fprintf(stderr,"%s\n", err);
#warning missing chinese
        draw_string_en(err);
        sleep(5);
        return -1;

    }
   
    /* write the zimage into the inand */
    lseek(fd_sd, (fw_fh->zimage).file.offset, SEEK_SET);
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS + (fw_fh->zimage).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    loading("正在升级... ", "Upgrading... ", (fw_fh->zimage).file.size, bar_color, trim_color);
    /* calculate the checksum of zimage1 */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS + (fw_fh->zimage).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    inand_check_sum = get_check_sum((fw_fh->zimage).file.size);
    if(inand_check_sum != (fw_fh->zimage).check_sum) { 
        sprintf(err, "zimage1 xsum fail: expected %d calc'ed %d", 
                (fw_fh->zimage).check_sum, inand_check_sum); 
        fprintf(stderr, "%s\n", err);
        draw_string_en(err);
        sleep(5);
        return -1;

    }
    
    /* write the initramfs into the inand */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS + (fw_fh->initramfs).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    loading("正在升级... ", "Upgrading... ", (fw_fh->initramfs).file.size, bar_color, trim_color);
    /* calculate the checksum of initramfs1 */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS + (fw_fh->initramfs).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    inand_check_sum = get_check_sum((fw_fh->initramfs).file.size);
    if(inand_check_sum != (fw_fh->initramfs).check_sum) { 
        sprintf(err, "initramfs1 xsum fail: expected %d calc'ed %d", 
                (fw_fh->initramfs).check_sum, inand_check_sum); 
        fprintf(stderr, "%s\n", err);
        draw_string_en(err);
        sleep(5);
        return -1;
    }
    
    /* write the firmware_fileheader backup into the inand */
    lseek(fd_sd, 0, SEEK_SET);
    lseek(fd_inand, -EXT3_ZIMAGE_INITRAMFS_SECTORS * INAND_BLOCK_SIZE / 2, SEEK_END);
    loading("正在升级... ", "Upgrading... ", fw_fh->fh_size, bar_color, trim_color);
    /* calculate the checksum of firmware_fileheader2 */
    lseek(fd_inand, -EXT3_ZIMAGE_INITRAMFS_SECTORS * INAND_BLOCK_SIZE / 2 + 8, SEEK_END);
    inand_check_sum = get_check_sum(fw_fh->fh_size - 8);
    if(inand_check_sum != fw_fh->check_sum) { 
        sprintf(err, "file header2 xsum fail: expected %d calc'ed %d", 
                fw_fh->check_sum, inand_check_sum); 
        fprintf(stderr, "%s\n", err);
        draw_string_en(err);
        sleep(5);
        return -1;
    }

    /* write the u-boot backup into the inand */
    lseek(fd_sd, (fw_fh->u_boot).file.offset, SEEK_SET);
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS / 2 + (fw_fh->u_boot).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    loading("正在升级... ", "Upgrading... ", (fw_fh->u_boot).file.size, bar_color, trim_color);
    /* calculate the checksum of u-boot2 */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS / 2 + (fw_fh->u_boot).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    inand_check_sum = get_check_sum((fw_fh->u_boot).file.size);
    if(inand_check_sum != (fw_fh->u_boot).check_sum) { 
        sprintf(err, "u_boot2 xsum fail: expected %d calc'ed %d", 
                      (fw_fh->u_boot).check_sum, inand_check_sum); 
        fprintf(stderr, "%s\n", err);
        draw_string_en(err);
        sleep(5);
        return -1;

    }

    /* write the zimage backup into the inand */
    lseek(fd_sd, (fw_fh->zimage).file.offset, SEEK_SET);
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS / 2 + (fw_fh->zimage).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    loading("正在升级... ", "Upgrading... ", (fw_fh->zimage).file.size, bar_color, trim_color);
    /* calculate the checksum of zimage2 */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS / 2 + (fw_fh->zimage).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    inand_check_sum = get_check_sum((fw_fh->zimage).file.size);
    if(inand_check_sum != (fw_fh->zimage).check_sum) { 
        sprintf(err, "zimage2 xsum fail: expected %d calc'ed %d", 
                (fw_fh->zimage).check_sum, inand_check_sum); 
        fprintf(stderr, "%s\n", err);
        draw_string_en(err);
        sleep(5);
        return -1;
    }
    
    /* write the initramfs backup into the inand */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS / 2 + (fw_fh->initramfs).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    loading("正在升级... ", "Upgrading... ", (fw_fh->initramfs).file.size, bar_color, trim_color);
    /* calculate the checksum of initramfs2 */
    lseek(fd_inand, (-EXT3_ZIMAGE_INITRAMFS_SECTORS / 2 + (fw_fh->initramfs).nand.offset) * INAND_BLOCK_SIZE, SEEK_END);
    inand_check_sum = get_check_sum((fw_fh->initramfs).file.size);
    if(inand_check_sum != (fw_fh->initramfs).check_sum) { 
        sprintf(err, "initramfs2 xsum fail: expected %d calc'ed %d", 
                      (fw_fh->initramfs).check_sum, inand_check_sum); 
        fprintf(stderr, "%s\n", err);
        draw_string_en(err);
        sleep(5);
        return -1;
    }

    close(fd_inand);

    old_size_sum = size_sum;
    if(1 == loading_homefs) {
        /* delete /mnt/upgrade first */
        system("rm -rf /mnt/upgrade\n");

        /* mkdir /mnt/upgrade */
        system("mkdir /mnt/upgrade\n");

        if(!keep_userzone) {
            /* mount the inand device to /mnt/upgrade */
            sprintf(inand_partition_path, "%sp%d", inand_device, 2);
            sprintf(system_cmd, "mount -t ext3 %s /mnt/upgrade/", inand_partition_path);
            system(system_cmd);
            fprintf(stderr, "%s\n", system_cmd);

            /* tar the homefs into the inand */
            lseek(fd_sd, (fw_fh->homefs).file.offset, SEEK_SET);
            fprintf(stderr, "(fw_fh->homefs).file.offset = %d\n", (fw_fh->homefs).file.offset);
            if(0 != loading_fs("正在升级... ", "Upgrading... ", (fw_fh->homefs).file.size, bar_color, trim_color))
            return -1;
            sync();
            umount("/mnt/upgrade");
            fprintf(stderr, "umount /mnt/upgrade\n");
            if(old_size_sum == size_sum)
                size_sum += (fw_fh->homefs).file.size;
        }

        /* mount the inand device to /mnt/upgrade */
        sprintf(inand_partition_path, "%sp%d", inand_device, 1);
        sprintf(system_cmd, "mount -t ext3 %s /mnt/upgrade/", inand_partition_path);
        system(system_cmd);
        fprintf(stderr, "%s\n", system_cmd);

        /* tar the rootfs into the inand */
        lseek(fd_sd, (fw_fh->rootfs).file.offset, SEEK_SET);
        fprintf(stderr, "(fw_fh->rootfs).file.offset = %d\n", (fw_fh->rootfs).file.offset);
        if(total_size_sum - size_sum != (fw_fh->rootfs).file.size) {
            fprintf(stderr, "wrong rootfs size\n");
            draw_string_en("Fails: wrong rootfs size");
            sleep(5);

            return -1;
        }
        if(0 != loading_fs("正在升级... ", "Upgrading... ", (fw_fh->rootfs).file.size, bar_color, trim_color))  {
           draw_string_en("Loading rootfs fails");
           sleep(5);
           return -1;
       }
       draw_string_zh("正在升级... 100%");
       draw_string_en("Upgrading... 100%");
       draw_progress_bar(1.0, PROGRESS_BAR_X_OFFSET, PROGRESS_BAR_ZH_OFFSET,
           PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT,
           PROGRESS_BAR_TRIM,
           bar_color, trim_color);
       sync();
       umount("/mnt/upgrade");
       fprintf(stderr, "umount /mnt/upgrade\n");
    }

    return 0;
}

int partition_inand(char *devName, long sectors)
{
    char system_cmd[512];

    static char fdisk_cmd[] = 
       "fdisk %s << __EOF > /dev/null\n"
       "o\n"    // make new partition table
       "n\n"    // new root partition
       "p\n"
       "1\n"    // partition 1
       "1\n"    // starting cyl
       "+%dM\n" // extent in MB
       "n\n"    // new home partition
       "p\n"
       "2\n"    // partition 2
       "\n"     // default end of last partition
       "+%dM\n" // extent in MB
       "n\n"    // new swap partition
       "p\n"
       "3\n"    // partition 3
       "\n"     // default end of last partition
       "+%dM\n" // extent in MB
       "w\n"
       "q\n"
       "__EOF\n";  // end of file marking for "<<" operator
 
    /* calculate the sizes of three partitions */
    umount("/mnt/mmcblk0p1/"); // hardcoded: dumb
    sprintf(system_cmd, fdisk_cmd, devName, 
       (sectors - EXT3_HOME_SECTORS - EXT3_SWAP_SECTORS - 
        EXT3_ZIMAGE_INITRAMFS_SECTORS) / NANDBLKSZ,
        EXT3_HOME_SECTORS / NANDBLKSZ,
        EXT3_SWAP_SECTORS / NANDBLKSZ);
    system(system_cmd);
    fprintf(stderr, "fdisk success\n");

    // create /dev/mmcblk0/1/2
    sleep(5);
    system("mknod /dev/mmcblk0p1 b 179 1");
    system("mknod /dev/mmcblk0p2 b 179 2");
    system("mknod /dev/mmcblk0p3 b 179 3");
 
    return 0;
}
 
int main( int argc, char *argv[] )
{
    int i, ret = 0;
    unsigned long sectors;
    int keys_fd;
    struct input_event event;
    struct timeval tpStart, tpEnd;
    struct stanza *stp;

    if(argc != 4) {
        fprintf(stderr, "usage: ./upgrade firmware_file inand_device 0/1\n"
                        "0: don't upgrade rootfs and homefs\n"
                        "1: upgrade all\n");
        return -1;
    }

    for (i = 0 ; i < argc ; i++)
        fprintf(stderr, "arg[%d] = %s\n", i, argv[i]);

    if(strstr(argv[1], "SmartQ5")) {
        fprintf(stderr, "firmware_file is SmartQ5\n");
        KEY_ADD = 109;
        KEY_DEC = 104;
    } else {
        fprintf(stderr, "firmware_file is SmartQ7\n");
        KEY_ADD = 104;
        KEY_DEC = 109;
    }

    loading_homefs = atoi(argv[3]);
    fprintf(stderr, "loading_homefs = %d\n", loading_homefs);
    
    if( 0 != open_screen() ) {
        fprintf(stderr, "open_screen failed\n");
        return -1;
    } else
        fprintf(stderr, "open_screen success\n");

    if ((ret = open_font(sb)) == -1)
        goto fail1;

    fprintf(stderr, "open_font success\n");

    //memset(sb->buffer, 0xff, fbmemlen);
    //system("/usr/bin/fv /etc/logo.jpg");
    /* different kernels do this differently.  Great... */
#define BACKLIGHT_CMD "echo 70 > "
#define SYS_BL_2_6_24	"/sys/devices/platform/s3c2410-lcd/backlight_level"
#define SYS_BL_2_6_28   "/sys/devices/platform/s3c-lcd/backlight_level"
#define SYS_BL_2_6_31   "/sys/class/backlight/pwm-backlight.0/brightness"

    system(BACKLIGHT_CMD SYS_BL_2_6_24);  // Turn on the LCD backlight
    system(BACKLIGHT_CMD SYS_BL_2_6_28);  // Turn on the LCD backlight
    system(BACKLIGHT_CMD SYS_BL_2_6_31);  // Turn on the LCD backlight

    fd_inand = open(argv[2], O_RDWR);
    if(fd_inand == -1) {
        fprintf(stderr, "main: can't find inand device %s\n", argv[2]);
        draw_string_zh("找不到升级设备!\n");
        draw_string_en("FATAL: inand open fails\n");
        return -1;
    }

    if(1 == loading_homefs) {
        keys_fd = open("/dev/input/event1", O_RDONLY);
        if(keys_fd <= 0)
        {
            fprintf(stderr, "open /dev/input/event1 device error!\n");
            goto fail2;
        }

        draw_string_zh("按'+'号键保留系统设置，否则按'-'号键...\n");
        draw_string_en("Press '+' to keep system setting,or press '-'...\n");

        gettimeofday(&tpStart, NULL);
        gettimeofday(&tpEnd, NULL);
        /* wait for user select until 60 seconds */
        while(tpEnd.tv_sec - tpStart.tv_sec < 60)
        {
            fd_set fdread;
            struct timeval wait_time;
            wait_time.tv_sec = 0;
            wait_time.tv_usec = 0;
            FD_ZERO(&fdread);
            FD_SET(keys_fd, &fdread);
            if (select(keys_fd + 1, &fdread, NULL, NULL, &wait_time) > 0
                    && FD_ISSET(keys_fd, &fdread)) {
                if(read(keys_fd, &event, sizeof(event)) == sizeof(event)) {
                    if(event.type == EV_KEY && event.value == 1 && 
                      (event.code == KEY_ADD || event.code == KEY_DEC)) {
                        fprintf(stderr, "key %d %s\n", event.code, (event.value)?"Pressed":"Released");
                        break;
                    }
                }
            }
            gettimeofday(&tpEnd, NULL);
        }
        close(keys_fd);
        if(event.code == KEY_ADD) {
            draw_string_zh("您选择了保留系统设置\n");
            draw_string_en("Keeping system settings\n");
            keep_userzone = 1;
        } else {
            keep_userzone = 0;
            draw_string_zh("您选择了不保留系统设置\n");
            draw_string_en("Discarding system settings\n");
        }
    }
    fprintf(stderr, "keep_userzone = %d\n", keep_userzone);

    draw_string_zh("正在校验固件...\n");
    draw_string_en("Verifying the firmware...\n");
    fw_fh = (firmware_fileheader *)malloc(sizeof(firmware_fileheader));
    ret = extract(argv[1], fw_fh);
    if(ret != 0) {
        fprintf(stderr, "main: extract firmware file %s failed\n", argv[1]);
        draw_string_zh("校验固件失败！\n");
        draw_string_en("Verifying the firmware failed!\n");
        goto fail2;
    }

    /* bizarre algorithm left for your amusement : WTF, over. */
    total_size_sum = fw_fh->homefs.file.offset;

    /* add in size of homefs if we are using it */
    if(!keep_userzone)
       total_size_sum += fw_fh->homefs.file.size;

    total_size_sum += fw_fh->fh_size + 
                      fw_fh->u_boot.file.size +
                      fw_fh->zimage.file.size + 
                      fw_fh->initramfs.file.size;

    /* got me on this one */
    total_size_sum -= fw_fh->qi.file.size;

    fprintf(stderr, "total_size_sum = %d\n", total_size_sum);

    fd_sd = open(argv[1], O_RDONLY);

    if(fd_sd == -1) {
        fprintf(stderr, "main: open firmware file %s on sd card failed\n", argv[1]);
        goto fail2;
    }

    buffer = malloc(INAND_SIZE_PER_WRITE);
    if(buffer == NULL) {
        fprintf(stderr, "malloc buffer failed\n");
        goto fail2;
    }

    if(1 == loading_homefs) {
        static char strbuf[80];

        /* get the total blocks of INAND */
        sectors = get_BLKGETSIZE_sectors(fd_inand);
        fprintf(stderr, "sectors of inand is %ld\n", sectors);


        // do the partitioning action
        draw_string_zh("正在进行分区...\n");

        sprintf(strbuf, "Partitioning %s (%lu sectors)...", argv[2], sectors);
        draw_string_en(strbuf);

        if (partition_inand(argv[2], sectors) == -1) {
#warning missing chinese
            draw_string_en("FATAL: partition fails!");
            fprintf(stderr, "main: partition of device %s fails\n", argv[2]);
            return -1;
        }

        // prep the filesystems
        draw_string_zh("正在格式化根分区...\n");
        draw_string_en("Formatting root partition...\n");

        sprintf(system_cmd, "mkfs.ext3 %sp%d -L root 1>/dev/null", argv[2], 1);
        system(system_cmd);
        fprintf(stderr, "%s\n", system_cmd);

        if(!keep_userzone) {
            draw_string_zh("正在格式化用户分区...\n");
            draw_string_en("Formatting home partition...\n");
            sprintf(system_cmd, "mkfs.ext3 %sp%d -L home 1>/dev/null", argv[2], 2);
            system(system_cmd);
            fprintf(stderr, "%s\n", system_cmd);
        }

        draw_string_zh("正在格式化交换分区...\n");
        draw_string_en("Formatting swap partition...\n");

        sprintf(system_cmd, "mkswap %sp%d 1>/dev/null", argv[2], 3);
        system(system_cmd);
        fprintf(stderr, "%s\n", system_cmd);
    }

    /* write the sd procedure into the INAND beginning at the last 18 block position */
    lseek(fd_sd, (fw_fh->qi).file.offset, SEEK_SET);
    lseek(fd_inand, -18 * INAND_BLOCK_SIZE, SEEK_END);
    if(sb->bytes_per_pixel == 4) 
        loading("载入进度 (qi)：", "loading (qi)", (fw_fh->qi).file.size, &COLOR_BAR, &COLOR_TRIM);
    else
        loading("载入进度 (qi)：", "loading (qi)", (fw_fh->qi).file.size, &COLOR_BAR_16, &COLOR_TRIM_16);
    /* calculate the checksum of qi */
    lseek(fd_inand, -18 * INAND_BLOCK_SIZE, SEEK_END);
    inand_check_sum = get_check_sum((fw_fh->qi).file.size);
    if(inand_check_sum != (fw_fh->qi).check_sum) { 
        char err[120];
        sprintf(err, "qi xsum failed: expected = %d, calc'ed = %d", 
           (fw_fh->qi).check_sum, inand_check_sum); 
        fprintf(stderr,"%s\n", err);
#warning missing chinese
        draw_string_en(err);
        sleep(5);

        return -1;
    }

    /* write the last 2 blocks of INAND to 0 */
    lseek(fd_inand, -2 * INAND_BLOCK_SIZE, SEEK_END);
    memset(buffer, 0, INAND_BLOCK_SIZE);
    write(fd_inand, buffer, INAND_BLOCK_SIZE);
    write(fd_inand, buffer, INAND_BLOCK_SIZE);
    fprintf(stderr, "write qi and u-boot success\n");
    
    if(sb->bytes_per_pixel == 4)
        ret = loading_firmware(argv[2], &COLOR_BAR, &COLOR_TRIM);
    else
        ret = loading_firmware(argv[2], &COLOR_BAR_16, &COLOR_TRIM_16);

    memset(sb->buffer + FB_BYTES_OF_LOGO, 0, fbmemlen - FB_BYTES_OF_LOGO);
    if(0 == ret) {
        draw_string_zh("升级成功，10秒钟后将自动关机...\n");
        draw_string_en("Upgrade OK,shutdown after 10 seconds...\n");
    } else {
        draw_string_zh("升级失败！\n");
        draw_string_en("Upgrade failed!\n");
    }
    close(fd_sd);
    free(buffer);
fail2:
    free(fw_fh);
    close_font();
fail1:
    close_screen (sb);
    
    if(0 == ret) {
        sleep(10);
        system("reboot");
    }

    return ret;
}



/****************** End Of File: rw_sd_inand.c ******************/
// vim:sts=4:ts=8: 
