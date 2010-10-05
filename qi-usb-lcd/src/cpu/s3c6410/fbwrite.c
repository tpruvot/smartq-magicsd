/* Write strings directly to the framebuffer.
 *
 * Tanguy PRUVOT 2010
 * Based on Kevin O'Connor HaReT code
 *
 * This file may be distributed under the terms of the GNU GPL license.
 */
#include <stdarg.h> // va_list
//#include <string.h> // memset

#include <s3c6410.h>
#include <utils.h>

#include "smartq.h" // led_blink, gpio_set_value
#include "fbwrite.h"

#define BIGFONT 1

#if BIGFONT == 1
	#define FONTWIDTH 6
	#define FONTHEIGHT 8
	#if FULL_ASCII == 1
		#define FONTDATAMAX 2048
	#else
		#define FONTDATAMAX 1024
	#endif
#else
	#define FONTDATAMAX 1536
	#define FONTWIDTH 4
	#define FONTHEIGHT 6
#endif

#if BIGFONT == 1
	#include "font_6x8.h"
	//extern const unsigned char fontdata_6x8[256][8];
#else
	#include "font_mini_4x6.h"
	//extern const unsigned char fontdata_mini_4x6[FONTDATAMAX];
#endif

#define BPP 2
#define videoW 800
#define videoH 480

//#define RGB565(r,g,b) (r&0x1f<<11) | (g&0x3c<<6) | (b&0x1f)
#define RGB555(r,g,b) (r&0x1f<<10) | (g&0x1f<<5) | (b&0x1f)

//16 bpp colors
#define BLACK 0x0000
#define WHITE 0x7fff

//16 bpp colors (RGB 5/6/5):
//#define RED   0xf800
//#define GREEN 0x07e0//#define GREEN 0x05e0
//#define BLUE  0x001f

//16 bpp colors (RGB 5/5/5):
#define RED   0x7C00
#define GREEN 0x03E0
#define BLUE  0x001F

//or on 1 byte (memset)
//0xff : white, 0xf0 : red, 0x3c : blue, 0x0f : green, 0x77 : light green, 0x55 : gray

//static const int MAXOUTBUF = 2*1024;
//static const int PADOUTBUF = 32;

static struct fbinfo fbi_store;

#define writel(v,a) (*(volatile unsigned int *)(a) = (v))
#define readl(a) (*(volatile unsigned int *)(a)) 

#define GPIO_S3C_GPN5 (('N'-'A')*16 + 5)
#define S3C_CLKCON_HCLK_LCD		(1<<3)
#define S3C_SPCON 0x7F0081A0

/*
int convertNL(char *outbuf, int maxlen, const char *inbuf, int len)
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
void Output(const char *format, ...)
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
void blit_char(struct fbinfo *fbi, unsigned char c)
{
	uint8 x,y;
	uint32 n;
	const unsigned char * font;
	uint16 * fbpos;

	if (fbi->x > fbi->maxx || fbi->y > fbi->maxy) {
		led_blink(2,3);
		return;
	}
	//if (fbi->fb == 0)
	//	return;
	
	#if FULL_ASCII == 0
		if (c & 0x80) return;
	#endif
	
	n = c * FONTHEIGHT;
	font = &fbi->fonts[n];

	n = fbi->y * FONTHEIGHT;
	n *= fbi->scrx;
	n += fbi->x * FONTWIDTH;
	
	fbpos = &fbi->fb[n];
	for (y=0; y<FONTHEIGHT; y++) {
		for (x=0; x<FONTWIDTH; x++) {
			if (font[y] & (1<<(FONTWIDTH-x-1)))
				fbpos[x] = fbi->color;
		}
		fbpos += fbi->scrx;
	}
}

void goNewLine(struct fbinfo *fbi)
{
	int linebytes;
	//uint32 b;

	fbi->x = 0;
	if (fbi->y < fbi->maxy-1) {
		fbi->y++;
		return;
	}
	linebytes = fbi->scrx * FONTHEIGHT;
	//for (b=0; b<(fbi->scrx * fbi->scry - linebytes) * BPP; b++) {
	//	*((char*)(uint32)fbi->fb + b) = *((char*) &fbi->fb[linebytes]);
	//}
/*	
	memcpy((char *)fbi->fb
			, (char *)&fbi->fb[linebytes]
			, (fbi->scrx * fbi->scry - linebytes) * BPP);
*/
}

// Write a charcter to the framebuffer.
void fb_putc(struct fbinfo *fbi, char c)
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
void fb_puts(struct fbinfo *fbi, const char *s)
{
	for (; *s; s++)
		fb_putc(fbi, *s);
}

// Write an unsigned integer to the screen.
void fb_putuint(struct fbinfo *fbi, uint32 val)
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
inline void fb_putsinglehex(struct fbinfo *fbi, uint32 val)
{
	if (val <= 9)
		val = '0' + val;
	else
		val = 'a' + val - 10;
	fb_putc(fbi, val);
}

// Write an integer in hexadecimal to the screen.
void fb_puthex(struct fbinfo *fbi, uint32 val)
{
	fb_puts(fbi, "0x");
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
void fb_printf(struct fbinfo *fbi, const char *fmt, ...)
{
	int32 val;
	char* sarg;
	char* n;
	char* s;
	va_list args;

	if (!fbi->fb)
		return;

	va_start(args, fmt);
	for (s = (char *) &fmt[0]; *s; s++) {
		if (*s != '%') {
			fb_putc(fbi, *s);
			continue;
		}
		n = (char *) s+1;
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
			sarg = (char *) va_arg(args, const char *);
			fb_puts(fbi, sarg);
			break;
		case '\0':
			break;
		default:
			fb_putc(fbi, *s);
			n = (char *) s;
		}
		s = n;
	}
	va_end(args);
}
/**/

// Clear the screen.
void fb_clear(struct fbinfo *fbi)
{
	if (!fbi->fb)
		return;
	memset16(fbi->fb, 0, fbi->scrx * fbi->scry);
	fbi->x = fbi->y = 0;
}

//Some value checks... usefull when importing weird kernel headers
#if S3C_GPICON != 0x7f008100
	#error S3C_GPICON != 0x7f008100 !!
#endif

/*
#define S3CVID_GPIO_OFFSET_CHECK(pin) ((pin) & 31)
void gpio_cfg_vid_pin(unsigned int con, uint32 pin, uint32 func)
{
	uint32 val, newval, mask;
	volatile uint32 *gpio_con = (volatile uint32 *)(con);
	
	mask = 0b0011 << (S3CVID_GPIO_OFFSET_CHECK(pin)*2);
	
	func &= 0b0011;
	func <<= (S3CVID_GPIO_OFFSET_CHECK(pin)*2);
	
	newval = val = readl(gpio_con);
	newval &= ~mask;
	newval |= func;	
	
	if (newval != val) {
		writel(newval,gpio_con);
		udelay(10);
		val = readl(gpio_con);
		if (newval != val) {
			func = (val & mask) >> (S3CVID_GPIO_OFFSET_CHECK(pin)*2);
			led_blink(2,func);
		}
	}
}
*/

// Initialize fbi structure and display.
struct fbinfo * fb_init(void)
{
	struct fbinfo * fbi;
	uint32 val;
	uint32 FBMEM = 0x5d000000;
	
	//memset(&fbi_store, 0, sizeof(fbi_store));
	fbi = &fbi_store;

	fbi->fb = (uint16 *) FBMEM;
	
	fbi->x = fbi->y = 0;
	fbi->scrx = videoW;
	fbi->scry = videoH;
	
	fbi->memsz = 0xbb800;//videoW*videoH;
	
	fbi->color = WHITE;
	
	#if BIGFONT == 1
		fbi->fonts = &fontdata_6x8[0][0];
		fbi->maxx = 133;//videoW / FONTWIDTH;
		fbi->maxy = 60;//videoH / FONTHEIGHT;
	#else
		fbi->fonts = &fontdata_mini_4x6[0];
		fbi->maxx = 200;//videoW / FONTWIDTH;
		fbi->maxy = 80;//videoH / FONTHEIGHT;		
	#endif

	/****************** LCD INIT **********************/

	/* Must be '0' for Normal-path instead of By-pass */
	writel(0x0, S3C_HOSTIFB_MIFPCON);
	
	writel(0x00000000,S3C_VIDCON0); //disable LCD 0bxx00
	/* enable clock to LCD */
	val = readl(S3C_HCLK_GATE);
	val |= S3C_CLKCON_HCLK_LCD;
	writel(val, S3C_HCLK_GATE);
	
	/* select TFT LCD type (RGB I/F) */
	val = readl(S3C_SPCON);
	val &= ~0x3;
	val |= 1; //(1 << 0);
	writel(val, S3C_SPCON);


	/* GPIO PINS FUNCTION FOR LCD (2) */

	writel(0,S3C_GPIDAT);
	writel(0,S3C_GPIPU);
	writel(0xAAAAAAAA,S3C_GPICON);

	writel(0,S3C_GPJDAT);
	writel(0,S3C_GPJPU);
	writel(0x00AAAAAA,S3C_GPJCON);

	//DRVCON_LCD [25:24]LCD Port Drive strength 
	//00 = 2mA       01 = 4mA      10 = 7mA      11 = 9mA 
	//gpio_cfg_vid_pin(S3C_SPCON, 12, 0b01);
		
	//val = (0xbb800*1);
	writel(FBMEM,			S3C_VIDW01ADD0B0);
	writel(fbi->memsz,		S3C_VIDW01ADD1B0);  //VBASE LOW = VBASE UP(0) + (PAGEWIDTH(800)+OFFSIZE(0)) x (LINEVAL+1(480)) : 0x0bb800
	//writel(  0x05dc00,		S3C_VIDW01ADD1B0);  //VBASE LOW = VBASE UP(0) + (PAGEWIDTH(800)+OFFSIZE(0)) x (LINEVAL+1(480))
	writel(0x00000320,		S3C_VIDW01ADD2); //PAGEWIDTH_F (1600) = 800*2

	/*
	val = (0xbb800*1);
	writel(FBMEM+val,		S3C_VIDW01ADD0B0);
	writel(  0x0bb800+val, 	S3C_VIDW01ADD1B0);  //VBASE LOW = VBASE UP(0) + (PAGEWIDTH(800)+OFFSIZE(0)) x (LINEVAL+1(480))

	val = (0xbb800*2);
	writel(FBMEM+val, 		S3C_VIDW00ADD0B0);
	writel(  0x000000+val, 	S3C_VIDW00ADD1B0);
	writel(0x00000640,		S3C_VIDW00ADD2);

	val = (0xbb800*3);
	writel(FBMEM+val,		S3C_VIDW02ADD0);
	writel(0x0bb800+val, 	S3C_VIDW02ADD1);
	writel(0x00000640,		S3C_VIDW02ADD2);

	val = (0xbb800*4);
	writel(FBMEM+val,		S3C_VIDW03ADD0);
	writel(0x0bb800+val, 	S3C_VIDW03ADD1);
	writel(0x00000640,		S3C_VIDW03ADD2);
	*/
	
	//writel(0x00000113,S3C_VIDCON0); set at end of proc

	//writel(0x01d0c0e0,S3C_VIDCON1); //Acer M900
	writel( 0x0000050,S3C_VIDCON1);
	writel(0x00000380,S3C_VIDCON2);
	
	writel(0x00240909,S3C_VIDTCON0); //Synch Pos V
	writel(0x005f6276,S3C_VIDTCON1); //Synch Pos H
	
	//writel(0x18f9df,S3C_VIDTCON2); //480x800  1 1101 1111  011 0001 1111
	writel(0x0efb1f,S3C_VIDTCON2);   //800x480 11 0001 1111  001 1101 1111

	//writel(0x00010015,S3C_WINCON0); //565 RGB
	writel(0x00000000,S3C_WINCON0);
	writel(0x00010019,S3C_WINCON1); //1555 ARGB
	writel(0x00000000,S3C_WINCON2);
	writel(0x00000000,S3C_WINCON3);
	writel(0x00000000,S3C_WINCON4);
	
	writel(0x000000,S3C_VIDOSD1A); //X:Y LeftTop
	//writel(0x0efb1f,S3C_VIDOSD1B); //479:799
	writel(0x18f9df,S3C_VIDOSD1B); //X:Y RightBot 799:479
	writel(0x000000,S3C_VIDOSD1C);
	writel(0x05dc00,S3C_VIDOSD1D); //OSDSIZE=384000. words
	
	writel(0b111,S3C_VIDINTCON1); //clear int
	writel(0x03f19001,S3C_VIDINTCON0);
	
	//memset16(fbi->fb, RGB(0xff,0xff,0xff), videoW*videoH);
	
	memset16(fbi->fb, RED, videoW*videoH); 

	for (val=0;val<videoH;val++) {
		memset16(fbi->fb + (val*videoW) + val, 0, 1);
		memset16(fbi->fb + ((videoH-val-1)*videoW) - val, 0, 1);
	}

	//memset16(fbi->fb + (2*videoW), GREEN, videoW); 

	writel(readl(S3C_WINCON1) | S3C_WINCONx_ENWIN_F_ENABLE, S3C_WINCON1);
	
	/* module reset */
	//gpio_direction_output(GPIO_S3C_GPN5, 1);
	//udelay(100000);
	
	gpio_set_value(GPIO_S3C_GPN5, 0);
	udelay(1000);
	
	gpio_set_value(GPIO_S3C_GPN5, 1);
	udelay(1000);

	//memset16(fbi->fb + videoW*18, RED,  videoW*20); 
	
	//fbi->x = fbi->y = 6;
	//blit_char(fbi, 'O');
	//blit_char(fbi, 'K');
	
	//for (val=30;val<256;val++) {
	//	fb_putc(fbi, val);
	//}

	writel(0x00000113,S3C_VIDCON0); //enable 0bxx11 | CLK_SRC | CKL_DIV
	//ou 0x13 | (0 << 2) | (4 << 6) |  (1 << 5)|  (1 << 16)

	fbi->x = fbi->y = 0;
	
	fb_puts(fbi, "MagicSD Qi USB v1.0\n");
	
	//fb_puthex(fbi,fbi->x);
	
	//led_blink(0,1);
	//delay(10);
	//fb_clear(fbi);
	//led_set(0);
	
	//memset16(fbi->fb + (2*videoW), BLACK, videoW); 
//	Output("Video buffer at %p sx=%d sy=%d mx=%d my=%d"
//		   , fbi->fb, fbi->scrx, fbi->scry, fbi->maxx, fbi->maxy);

	return fbi;
}

struct fbinfo * fb_get(void)
{
	return &fbi_store;
}
