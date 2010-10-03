/* Write strings directly to the framebuffer.
 *
 * (C) Copyright 2006 Kevin O'Connor <kevin@koconnor.net>
 *
 * This file may be distributed under the terms of the GNU GPL license.
 */
#include <stdarg.h> // va_list
//#include <string.h> // memset

#include <s3c6410.h>
#include <utils.h>

//#include "linboot.h" // __preload

#include "smartq.h"
#include "fbwrite.h"

#define __preload

#define BIGFONT 1

#ifdef BIGFONT
	#define FONTDATAMAX 2048
	#define FONTWIDTH 6
	#define FONTHEIGHT 8
#else
	#define FONTDATAMAX 1536
	#define FONTWIDTH 4
	#define FONTHEIGHT 6
#endif

#ifdef BIGFONT
	#include "font_6x8.h"
	//extern const unsigned char fontdata_6x8[256][8];
#else
	#include "font_mini_4x6.h"
	//extern const unsigned char fontdata_mini_4x6[FONTDATAMAX];
#endif

#define BPP 2
#define videoW 800
#define videoH 480

#define GPIO_S3C_GPN5 (('N'-'A')*16 + 5)
#define S3C_CLKCON_HCLK_LCD		(1<<3)
#define S3C_SPCON 0x7F0081A0

static const int MAXOUTBUF = 2*1024;
static const int PADOUTBUF = 32;

static struct fbinfo fbi_store;

#define writel(v,a) (*(volatile unsigned int *)(a) = (v))
#define readl(a) (*(volatile unsigned int *)(a)) 

/*
int
convertNL(char *outbuf, int maxlen, const char *inbuf, int len)
{
    // Convert CR to CR/LF since telnet requires this
    const char *s = inbuf, *s_end = &inbuf[len];
    char *d = outbuf;
    char *d_end = &outbuf[maxlen - 3];
    while (s < s_end && d < d_end) {
        if (*s == '\n')
            *d++ = '\r';
        *d++ = *s++;
    }

    // A trailing tab character is an indicator to not add in a
    // trailing newline - in all other cases add the newline.
    if (d > outbuf && d[-1] == '\t') {
        d--;
    } else {
        *d++ = '\r';
        *d++ = '\n';
    }

    *d = '\0';
    return d - outbuf;
}

// Output message to screen/logs/socket.
void
Output(const char *format, ...)
{
    // Check for error indicator (eg, format starting with "<0>")
    int code = 7;
    if (format[0] == '<'
        && format[1] >= '0' && format[1] <= '9'
        && format[2] == '>') {
        code = format[1] - '0';
        format += 3;
    }

    // Format output string.
    char rawbuf[MAXOUTBUF];
    va_list args;
    va_start(args, format);
    int rawlen = vsnprintf2(rawbuf, sizeof(rawbuf) - PADOUTBUF, format, args);
    va_end(args);

    // Convert newline characters
    char buf[MAXOUTBUF];
    //int len = 
    convertNL(buf, sizeof(buf), rawbuf, rawlen);

	//fb_puts(fbi,buf);

  writeLog(buf, len);
    outputfn *ofn = getOutputFn();
    if (!ofn && code < 6) {
        Complain(rawbuf, rawlen, code-1);
        return;
    }
    if (code <= 6)
        writeScreen(buf, len);
    if (ofn && code <= 7)
        ofn->sendMessage(buf, len);
}
*/

// Write a character to the screen.
void __preload
blit_char(struct fbinfo *fbi, char c)
{
	int x,y;
	if (fbi->x > fbi->maxx || fbi->y > fbi->maxy)
		return;

	const unsigned char *font = &fbi->fonts[c * FONTHEIGHT];
	uint16 *fbpos = &fbi->fb[fbi->scrx * fbi->y * FONTHEIGHT
							 + fbi->x * FONTWIDTH];
	for (y=0; y<FONTHEIGHT; y++) {
		for (x=0; x<FONTWIDTH; x++) {
			if (font[y] & (1<<(FONTWIDTH-x-1)))
				fbpos[x] = 0xFFFF;
		}
		fbpos += fbi->scrx;
	}
}

void __preload
goNewLine(struct fbinfo *fbi)
{
	fbi->x = 0;
	if (fbi->y < fbi->maxy-1) {
		fbi->y++;
		return;
	}
	int linebytes = fbi->scrx * FONTHEIGHT;
	memcpy((char *)fbi->fb
			, (char *)&fbi->fb[linebytes]
			, (fbi->scrx * fbi->scry - linebytes) * BPP);
}

// Write a charcter to the framebuffer.
void __preload
fb_putc(struct fbinfo *fbi, char c)
{
	if (c == '\n') {
		goNewLine(fbi);
		return;
	}
	blit_char(fbi, c);
	fbi->x++;
	if (fbi->x >= fbi->maxx)
		goNewLine(fbi);
}

// Write a string to the framebuffer.
void __preload
fb_puts(struct fbinfo *fbi, const char *s)
{
	for (; *s; s++)
		fb_putc(fbi, *s);
}

// Write an unsigned integer to the screen.
void __preload
fb_putuint(struct fbinfo *fbi, uint32 val)
{
	char buf[12];
	char *d = &buf[sizeof(buf) - 1];
	*d-- = '\0';
	for (;;) {
		*d = (val % 10) + '0';
		val /= 10;
		if (!val)
			break;
		d--;
	}
	fb_puts(fbi, d);
}

// Write a single digit hex character to the screen.
inline void __preload
fb_putsinglehex(struct fbinfo *fbi, uint32 val)
{
	if (val <= 9)
		val = '0' + val;
	else
		val = 'a' + val - 10;
	fb_putc(fbi, val);
}

// Write an integer in hexadecimal to the screen.
void __preload
fb_puthex(struct fbinfo *fbi, uint32 val)
{
	fb_putsinglehex(fbi, (val >> 28) & 0xf);
	fb_putsinglehex(fbi, (val >> 24) & 0xf);
	fb_putsinglehex(fbi, (val >> 20) & 0xf);
	fb_putsinglehex(fbi, (val >> 16) & 0xf);
	fb_putsinglehex(fbi, (val >> 12) & 0xf);
	fb_putsinglehex(fbi, (val >> 8) & 0xf);
	fb_putsinglehex(fbi, (val >> 4) & 0xf);
	fb_putsinglehex(fbi, (val >> 0) & 0xf);
}

// Write a string to the framebuffer.
void __preload
fb_printf(struct fbinfo *fbi, const char *fmt, ...)
{
	if (!fbi->fb)
		return;

	va_list args;
	va_start(args, fmt);
	const char *s = fmt;
	for (; *s; s++) {
		if (*s != '%') {
			fb_putc(fbi, *s);
			continue;
		}
		const char *n = s+1;
		int32 val;
		const char *sarg;
		switch (*n) {
		case '%':
			fb_putc(fbi, '%');
			break;
		case 'd':
			val = va_arg(args, int32);
			if (val < 0) {
				fb_putc(fbi, '-');
				val = -val;
			}
			fb_putuint(fbi, val);
			break;
		case 'u':
			val = va_arg(args, int32);
			fb_putuint(fbi, val);
			break;
		case 'x':
			val = va_arg(args, int32);
			fb_puthex(fbi, val);
			break;
		case 's':
			sarg = va_arg(args, const char *);
			fb_puts(fbi, sarg);
			break;
		default:
			fb_putc(fbi, *s);
			n = s;
		}
		s = n;
	}
	va_end(args);
}

// Clear the screen.
void
fb_clear(struct fbinfo *fbi)
{
	if (!fbi->fb)
		return;
	memset(fbi->fb, 0, fbi->scrx * fbi->scry * BPP);
	fbi->x = fbi->y = 0;
}

#define S3CVID_GPIO_OFFSET(pin) ((pin) & 31)
void
gpio_cfg_vid_pin(unsigned int con, unsigned int num, int func)
{
	uint32 val, mask;
	volatile uint32 *gpio_con = (volatile uint32 *)(con);
	
	mask = 3 << S3CVID_GPIO_OFFSET(num)*2;
	
	func &= 3;
	func <<= S3CVID_GPIO_OFFSET(num)*2;
	
	val = readl(gpio_con);
	val &= ~mask;
	val |= func;
	writel(val,gpio_con);
}

// Initialize an fbi structure and display.
void
fb_init(struct fbinfo *fbi)
{
	uint32 val;
	uint32 FBMEM = 0x54000000; //uint16 FBMEM[videoW*videoH*BPP] 0x000bb800;
	
	//memset(&fbi_store, 0, sizeof(fbi_store));
	fbi = &fbi_store;

	fbi->fb = (uint16 *) FBMEM;

	/* Must be '0' for Normal-path instead of By-pass */
	writel(0x0, S3C_HOSTIFB_MIFPCON);
	
	/* enable clock to LCD */
	val = readl(S3C_HCLK_GATE);
	val |= S3C_CLKCON_HCLK_LCD;
	writel(val, S3C_HCLK_GATE);
	
	/* select TFT LCD type (RGB I/F) */
	val = readl(S3C_SPCON);
	val &= ~0x3;
	val |= (1 << 0);
	writel(val, S3C_SPCON);
	
	/* VD */
	gpio_cfg_vid_pin(S3C_GPICON, 0, S3C_GPI0_LCD_VD0);
	gpio_cfg_vid_pin(S3C_GPICON, 1, S3C_GPI1_LCD_VD1);
	gpio_cfg_vid_pin(S3C_GPICON, 2, S3C_GPI2_LCD_VD2);
	gpio_cfg_vid_pin(S3C_GPICON, 3, S3C_GPI3_LCD_VD3);
	gpio_cfg_vid_pin(S3C_GPICON, 4, S3C_GPI4_LCD_VD4);
	gpio_cfg_vid_pin(S3C_GPICON, 5, S3C_GPI5_LCD_VD5);
	gpio_cfg_vid_pin(S3C_GPICON, 6, S3C_GPI6_LCD_VD6);
	gpio_cfg_vid_pin(S3C_GPICON, 7, S3C_GPI7_LCD_VD7);
	gpio_cfg_vid_pin(S3C_GPICON, 8, S3C_GPI8_LCD_VD8);
	gpio_cfg_vid_pin(S3C_GPICON, 9, S3C_GPI9_LCD_VD9);
	gpio_cfg_vid_pin(S3C_GPICON, 10, S3C_GPI10_LCD_VD10);
	gpio_cfg_vid_pin(S3C_GPICON, 11, S3C_GPI11_LCD_VD11);
	gpio_cfg_vid_pin(S3C_GPICON, 12, S3C_GPI12_LCD_VD12);
	gpio_cfg_vid_pin(S3C_GPICON, 13, S3C_GPI13_LCD_VD13);
	gpio_cfg_vid_pin(S3C_GPICON, 14, S3C_GPI14_LCD_VD14);
	gpio_cfg_vid_pin(S3C_GPICON, 15, S3C_GPI15_LCD_VD15);

	gpio_cfg_vid_pin(S3C_GPJCON, 0, S3C_GPJ0_LCD_VD16);
	gpio_cfg_vid_pin(S3C_GPJCON, 1, S3C_GPJ1_LCD_VD17);
	gpio_cfg_vid_pin(S3C_GPJCON, 2, S3C_GPJ2_LCD_VD18);
	gpio_cfg_vid_pin(S3C_GPJCON, 3, S3C_GPJ3_LCD_VD19);
	gpio_cfg_vid_pin(S3C_GPJCON, 4, S3C_GPJ4_LCD_VD20);
	gpio_cfg_vid_pin(S3C_GPJCON, 5, S3C_GPJ5_LCD_VD21);
	gpio_cfg_vid_pin(S3C_GPJCON, 6, S3C_GPJ6_LCD_VD22);
	gpio_cfg_vid_pin(S3C_GPJCON, 7, S3C_GPJ7_LCD_VD23);
	gpio_cfg_vid_pin(S3C_GPJCON, 8, S3C_GPJ8_LCD_HSYNC);
	gpio_cfg_vid_pin(S3C_GPJCON, 9, S3C_GPJ9_LCD_VSYNC);
	gpio_cfg_vid_pin(S3C_GPJCON, 10, S3C_GPJ10_LCD_VDEN);
	gpio_cfg_vid_pin(S3C_GPJCON, 11, S3C_GPJ11_LCD_VCLK);

	/* module reset */
	gpio_direction_output(GPIO_S3C_GPN5, 1);
	udelay(200000);
	
	gpio_set_value(GPIO_S3C_GPN5, 0);
	udelay(20000);
	
	gpio_set_value(GPIO_S3C_GPN5, 1);
	udelay(20000);

	writel(FBMEM,     S3C_VIDW01ADD0B0);
	writel(0x000bb800,S3C_VIDW01ADD1B0);
	
	writel(0x00000110,S3C_VIDCON0); //disable 0bxx00
	writel(0x01d0c0e0,S3C_VIDCON1);
	writel(0x00000380,S3C_VIDCON2);

	writel(0x00010101,S3C_VIDTCON0);
	writel(0x00130909,S3C_VIDTCON1);
	writel(0x0018f9df,S3C_VIDTCON2);

	writel(0x00000000,S3C_WINCON0);
	writel(0x00010015,S3C_WINCON1);
	writel(0x00000000,S3C_WINCON2);
	
	writel(0x00000000,S3C_VIDOSD1A);
	writel(0x000efb1f,S3C_VIDOSD1B); //OSD_RightBotY_F OSD_RightBotX_F
	writel(0x00000000,S3C_VIDOSD1C);
	writel(0x0005dc00,S3C_VIDOSD1D); //OSDSIZE=384000. words


	fbi->x =0;
	fbi->y =0;
	fbi->scrx = videoW;
	fbi->scry = videoH;
	fbi->maxx = 80;//videoW / FONTWIDTH;
	fbi->maxy = 100;//videoH / FONTHEIGHT;
	
	memset(fbi->fb, 0x70, videoW*videoH);

	writel(readl(S3C_WINCON1) | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON1);

	writel(0x00000113,S3C_VIDCON0); //enable 0bxx11
	
	
	#ifdef BIGFONT
		fbi->fonts = (unsigned char*) &fontdata_6x8[0][0];
	#else
		fbi->fonts = &fontdata_mini_4x6[0];
	#endif

//	Output("Video buffer at %p sx=%d sy=%d mx=%d my=%d"
//		   , fbi->fb, fbi->scrx, fbi->scry, fbi->maxx, fbi->maxy);
}
