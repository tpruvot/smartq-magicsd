/****************************************************************
 * $ID: cmd_identify.c  Thu, 26 Mar 2009 09:20:17 +0800  root $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  (Meihui Fan)  <mhfan@hhcn.com>            *
 *                                                              *
 * Copyright (C)  2009  HHTech                                  *
 *   www.hhcn.com, www.hhcn.org                                 *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 *                                                              *
 * Last modified: Sat, 28 Mar 2009 10:37:01 +0800       by root #
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "lib_Crypto.h"
#include "lib_Data.h"
#include "ll_port.h"

extern int fd;

static void mdelay(unsigned int time_count)
{
    ll_Delay(time_count * 1000);
}

static void set_value_7(unsigned int value1, unsigned int value2, uchar val[])
{
	val[0] = (uchar)(value1 >> 24); val[1] = (uchar)(value1 >> 16);
	val[2] = (uchar)(value1 >> 8); val[3] = (uchar)(value1);
	val[4] = (uchar)(value2 >> 24); val[5] = (uchar)(value2 >> 16);
	val[6] = (uchar)(value2 >> 8);
}

static void set_value_8(unsigned int value1, unsigned int value2, uchar val[])
{
	val[0] = (uchar)(value1 >> 24); val[1] = (uchar)(value1 >> 16);
	val[2] = (uchar)(value1 >> 8); val[3] = (uchar)(value1);
	val[4] = (uchar)(value2 >> 24); val[5] = (uchar)(value2 >> 16);
	val[6] = (uchar)(value2 >> 8); val[7] = (uchar)(value2);
}

unsigned char read_user_zone(int zone_num)
{
	unsigned char ret;
	long long ll;
	unsigned char i;
	unsigned char read_user[9];

	switch(zone_num){
	    case 0:	    
		//use user 0
		printf("begin read user zone0: \n");
		if(ret = cm_SetUserZone(0xb,0x0,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0xb,passwd1,0x01,0x01)) return ret;
		mdelay(20);
		if(ret = cm_ReadUserZone(0x0b,0x00,read_user,8)) return ret;
		mdelay(20);
		for(i=0;i<8;i++)
		    printf("%02x", read_user[i]);
		printf("\n");
		break;
	    case 1:
		//use user 1
		printf("begin read user zone1: \n");
		if(ret = cm_SetUserZone(0xb,0x1,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0xb,passwd2,0x02,0x01)) return ret;
		mdelay(20);
		if(ret = cm_ReadUserZone(0x0b,0x00,read_user,8)) return ret;
		mdelay(20);
		for(i=0;i<8;i++)
		    printf("%02x", read_user[i]);
		printf("\n");
		break;
	    case 2:
		//use user 2
		printf("begin read user zone2: \n");
		if(ret = cm_SetUserZone(0xb,0x2,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0xb,passwd0,0x00,0x01)) return ret;
		mdelay(20);
		if(ret = cm_ReadUserZone(0x0b,0x00,read_user,9)) return ret;
		mdelay(20);
		for(i=0;i<8;i++)
		    printf("%02x", read_user[i]);
		printf("\n");
		break;
	    case 3:
		//use user 3
		printf("begin read user zone3: \n");
		if(ret = cm_SetUserZone(0xb,0x3,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0xb,passwd7,0x07,0x01)) return ret;
		mdelay(20);
		if(ret = cm_ReadUserZone(0x0b,0x00,read_user,9)) return ret;
		mdelay(20);
		for(i=0;i<8;i++)
		    printf("%02x", read_user[i]);
		printf("\n");
		break;
	    default:
		break;
	}
	return ret;
}

int main(int argc, char *argv[])
{
    RETURN_CODE ret;
    unsigned char buf[0xf0];
    unsigned char obuf[2];
    unsigned char i,readback;
    unsigned long int i_NC, i_C2, i_G2, i_G3, i_Tmp, i_Tmp2;
    char str[9];
    uchar a[8], b[8];

    buf[0]=0x55;
    buf[1]=0xaa;

    if(argc != 2) {
	printf("usage: ./cmd_encrypt password_G3\n");
	return 0;
    }

    if((fd = open("/dev/smartq5_encrypt", O_RDONLY)) == -1) {
	printf("open /dev/smartq5_encrypt failed\n");
	return -1;
    }

    memcpy(str, argv[1], 8);
    str[8] = '\n';
    i_Tmp = strtoul(str,NULL,16);
    printf("the i_Tmp is %0x\n", i_Tmp);

    memcpy(str, &(argv[1][8]), 8);
    str[8] = '\n';
    i_Tmp2 = strtoul(str,NULL,16);
    printf("the i_Tmp2 is %0x\n", i_Tmp2);

    set_value_8(i_Tmp, i_Tmp2, G3);
    printf("G3 is: ");
    for(i = 0; i < 8; i++)
	printf("%02x", G3[i]);
    printf("\n");

    /* mtz test */
    printf("mtz test\n");
    ll_PowerOn();
    mdelay(20);
    if(SUCCESS != cm_Init()) printf("err init cm\n");
    ret = cm_WriteConfigZone(0xb,0xa,buf,2,0x0);
    if(ret!=SUCCESS) {printf("err write to mtz,return 0x%x\n",ret);
	return -1;
    }
    mdelay(20);
    ret = cm_ReadConfigZone(0xb,0xa,obuf,2);
    if(ret!=SUCCESS) {printf("err read from mtz,return 0x%x\n",ret);
	printf("read mtz 0x%x 0x%x\n",obuf[0],obuf[1]);
	return -1;
    }
    if((obuf[0]!=0x55)||(obuf[1]!=0xaa)) return -1;
    mdelay(20);
    printf("mtz test success\n");

    /* read NC */
    ret = cm_ReadConfigZone(0xb,0x19,buf,7);
    if(ret!=SUCCESS) {
	printf("err read from NC,return 0x%x\n",ret);
	return -1;
    } else {
	printf("NC is: ");
	for(i = 0; i < 7; i++)
	    printf("%02x", buf[i]);
	printf("\n");
    }
    i_NC = (unsigned int)(buf[0] << 24) | (unsigned int)(buf[1] << 16) | (unsigned int)(buf[2] << 8) | (unsigned int)buf[3]; 
    printf("i_NC = %x\n", i_NC);

#if 0
    /* read C0 */
    ret = cm_ReadConfigZone(0xb,0x51,buf,7);
    if(ret!=SUCCESS) {
	printf("err read from C0,return 0x%x\n",ret);
	return -1;
    } else {
	printf("C0 is: ");
	for(i = 0; i < 7; i++)
	    printf("%02x", buf[i]);
	printf("\n");
    }

    /* read C1 */
    ret = cm_ReadConfigZone(0xb,0x51,buf,7);
    if(ret!=SUCCESS) {
	printf("err read from C1,return 0x%x\n",ret);
	return -1;
    } else {
	printf("C1 is: ");
	for(i = 0; i < 7; i++)
	    printf("%02x", buf[i]);
	printf("\n");
    }
#endif

    /* read C2 */
    /*ret = cm_ReadConfigZone(0xb,0x71,buf,7);
    if(ret!=SUCCESS) {
	printf("err read from C2,return 0x%x\n",ret);
	return -1;
    } else {
	printf("C2 is: ");
	for(i = 0; i < 7; i++)
	    printf("%02x", buf[i]);
	printf("\n");
    }
    i_C2 = (unsigned int)(buf[0] << 24) | (unsigned int)(buf[1] << 16) | (unsigned int)(buf[2] << 8) | (unsigned int)buf[3]; 
    printf("i_C2 = %x\n", i_C2);*/

    srandom((unsigned int)i_NC);
    i_C2 = random();
    printf("i_C2 = %0x\n", i_C2);

    i_G2 = 2 * i_NC + 100;
    printf("i_G2 = %0x\n", i_G2);
    set_value_8(i_G2, i_G2, G2);
    printf("G2 is: ");
    for(i = 0; i < 8; i++)
	printf("%02x", G2[i]);
    printf("\n");

#if 0
    /* read C3 */
    ret = cm_ReadConfigZone(0xb,0x71,buf,7);
    if(ret!=SUCCESS) {
	printf("err read from C3,return 0x%x\n",ret);
	return -1;
    } else {
	printf("C3 is: ");
	for(i = 0; i < 7; i++)
	    printf("%02x", buf[i]);
	printf("\n");
    }
#endif

    /* can't read G0 */
    /*ret = cm_ReadConfigZone(0xb,0x90,buf,8);
    if(ret!=SUCCESS) {
	printf("err read from G0,return 0x%x\n",ret);
	return -1;
    } else {
	printf("G0 is: ");
	for(i = 0; i < 8; i++)
	    printf("%02x", buf[i]);
	printf("\n");
    }*/

    /* identify using G2 */
    for(i = 0; i < 8; i++) {
	a[i] = G2[i];
	b[i] = G2[i];
    }
    ret = cm_VerifyCrypto(0xb, 2, a, NULL, 0x0);
    if (ret != 0x00) {
	printf("fail identification 0 using G2\n");
	return -1;
    } else
	printf("succeed identification 0 using G2\n");
    ret = cm_VerifyCrypto(0xb, 2, b, NULL, 0x1);
    if (ret != 0x00) {
	printf("fail identification 1 using G2\n");
	return -1;
    } else
	printf("succeed identification 1 using G2\n");

    /* identify using G3 */
    for(i = 0; i < 8; i++) {
	a[i] = G3[i];
	b[i] = G3[i];
    }
    ret = cm_VerifyCrypto(0xb, 3, a, NULL, 0x0);
    if (ret != 0x00) {
	printf("fail identification 0 using G3\n");
	return -1;
    } else
	printf("succeed identification 0 using G3\n");
    ret = cm_VerifyCrypto(0xb, 3, b, NULL, 0x1);
    if (ret != 0x00) {
	printf("fail identification 1 using G3\n");
	return -1;
    } else
	printf("succeed identification 1 using G3\n");

    /* identify using G3 */
    /*ret = cm_VerifyCrypto(0xb, 3, G3, NULL, 0x0);
    if (ret != 0x00) {
	printf("fail identification using G3\n");
	return -1;
    }
    
    ret = cm_VerifyCrypto(0xb, 3, G1, NULL, 0x0);
    if (ret != 0x00) {
	printf("fail identification using G1\n");
	return -1;
    }*/

    read_user_zone(0);
    read_user_zone(1);
    read_user_zone(2);
    read_user_zone(3);

    return 0;
}

/***************** End Of File: cmd_identify.c *****************/
// vim:sts=4:ts=8: 
