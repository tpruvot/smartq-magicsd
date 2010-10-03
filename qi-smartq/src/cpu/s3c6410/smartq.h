#ifndef SMARTQ_H
#define SMARTQ_H

#include <qi.h>

extern const struct board_api board_api_smartq;

void led_set(int);
void led_blink(int, int);
void poweroff(void);
void set_lcd_backlight(int);
int sd_card_block_read_smartq(unsigned char*, unsigned long, int);

void gpio_direction_output(int gpio, int dat);
int gpio_set_value(int gpio, int dat);

#endif
