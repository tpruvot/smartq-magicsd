/*
 *  S3C64XX encryption 
 *
 *  Copyright (C) 2008 - 2009 HHTECH.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "lib_Crypto.h"
#include "lib_Data.h"
#include "ll_port.h"

//#define DEBUG 1
#ifdef DEBUG
#define dbg	printf
#else
#define dbg(format, arg...) do {} while (0)
#endif

//#define	LCD_4

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

static char save_file[100];
#if 1 
//int do_wr_fuse(int argc, char *argv[])
int main(int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[0xf0];
	unsigned char obuf[2];
	unsigned char ps7[3]={0xdd,0x42,0x97};    // first time
	//unsigned char ps7[3]={0x33,0x56,0x78};    // latter time
	unsigned char i,readback;
	unsigned long int i_NC, i_C2, i_G2, i_G3, i_Tmp, i_Tmp2;
	time_t now_time;
	static char str[100];
	FILE *fp;

	buf[0]=0x55;
	buf[1]=0xaa;

	if(argc != 2) {
	    printf("usage: ./cmd_encrypt identify_num\n");
	    return 0;
	}

	if((fd = open("/dev/smartq5_encrypt", O_RDONLY)) == -1) {
	    printf("open /dev/smartq5_encrypt failed\n");
	    return -1;
	}
	
	dbg("mtz test\n");
	ll_PowerOn();
	mdelay(30);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	ret = cm_WriteConfigZone(0xb,0xa,buf,2,0x0);
	if(ret!=SUCCESS) {printf("err write to mtz,return 0x%x\n",ret);
	return -1;
	}
	mdelay(30);
	ret = cm_ReadConfigZone(0xb,0xa,obuf,2);
	if(ret!=SUCCESS) {printf("err read from mtz,return 0x%x\n",ret);
	printf("read mtz 0x%x 0x%x\n",obuf[0],obuf[1]);
	return -1;
	}
	if((obuf[0]!=0x55)||(obuf[1]!=0xaa)) {
	    printf("mtz test failed\n");
	    return -1;
	}
	mdelay(30);
	dbg("mtz test success\n");
	
	i_NC = strtoul(argv[1],NULL,16);
#ifdef	LCD_4
	i_NC = 0x03000000 + (i_NC << 8) + 0x21;
#else
	i_NC = 0x02000000 + (i_NC << 8) + 0x25;
#endif
	//printf("the Identification Numer is %0x\n", i_NC);
	set_value_7(i_NC, i_NC, nc);
	dbg("nc is: ");
	for(i = 0; i < 7; i++)
	    dbg("%02x", nc[i]);
	dbg("\n");
#ifdef  LCD_4
	sprintf(str, "NC is: H9%08x106\n", i_NC);
#else
	sprintf(str, "NC is: H9%08x107\n", i_NC);
#endif
	printf("%s", str);
#ifdef	LCD_4
	sprintf(save_file, "H9%08x106.txt", i_NC);
#else
	sprintf(save_file, "H9%08x107.txt", i_NC);
#endif
	fp = fopen(save_file, "a+");
	if(fp == NULL) {
	    printf("open %s failed\n", save_file);
	    return -1;
	}
	fputs(str, fp);

	srandom((unsigned int)i_NC);
	i_C2 = random();
	dbg("i_C2 = %0x\n", i_C2);
	set_value_7(i_C2, i_C2, C2);
	dbg("C2 is: ");
	for(i = 0; i < 7; i++)
	    dbg("%02x", C2[i]);
	dbg("\n");

	i_G2 = 2 * i_NC + 100;
	dbg("i_G2 = %0x\n", i_G2);
	set_value_8(i_G2, i_G2, G2);
	printf("G2 is: ");
	for(i = 0; i < 8; i++)
	    printf("%02x", G2[i]);
	printf("\n");
	sprintf(str, "G2 is: %02x%02x%02x%02x%02x%02x%02x%02x\n", G2[0],G2[1],G2[2],G2[3],G2[4],G2[5],G2[6],G2[7]);
	fputs(str, fp);

	time(&now_time);
	srandom(now_time);
	i_Tmp = random() + now_time;
	sleep(1);
	time(&now_time);
	srandom(now_time * 2);
	i_Tmp2 = random() + now_time * 2;
	set_value_8(i_Tmp, i_Tmp2, G3);
	printf("G3 is: ");
	for(i = 0; i < 8; i++)
	    printf("%02x", G3[i]);
	printf("\n");
	sprintf(str, "G3 is: %02x%02x%02x%02x%02x%02x%02x%02x\n", G3[0],G3[1],G3[2],G3[3],G3[4],G3[5],G3[6],G3[7]);
	fputs(str, fp);

#if 1
	dbg("write zone\n");
	//write to zone0
	time(&now_time);
	srandom(now_time);
	i_Tmp = random() + now_time;
	if(ret = cm_SetUserZone(0xb,0x0,0x0)) 
	{printf("err set user zone0 %x\n",ret);
	return ret;
	}
	mdelay(30);
	time(&now_time);
	srandom(now_time * 2);
	i_Tmp2 = random() + now_time * 2;
	set_value_8(i_Tmp, i_Tmp2, zone0);
	printf("zone0 is :");
	for(i = 0; i < 8; i++)
	    printf("%02x", zone0[i]);
	printf("\n");
	if(ret = cm_WriteUserZone(0xb,0x0,zone0,8)) 
	{	
		printf("err write user zone0 %x\n",ret);
		return ret;
	}
	sprintf(str, "zone0 is: %02x%02x%02x%02x%02x%02x%02x%02x\n", zone0[0],zone0[1],zone0[2],zone0[3],zone0[4],zone0[5],zone0[6],zone0[7]);
	fputs(str, fp);
	
	//write to zone1
	sleep(1);
	time(&now_time);
	srandom(now_time);
	i_Tmp = random() + now_time;
	if(ret = cm_SetUserZone(0xb,0x1,0x0)) 
	{
		printf("err set user zone1 %x\n",ret);
		return ret;
	}
	mdelay(30);
	time(&now_time);
	srandom(now_time * 3);
	i_Tmp2 = random() + now_time * 3;
	set_value_8(i_Tmp, i_Tmp2, zone1);
	printf("zone1 is :");
	for(i = 0; i < 8; i++)
	    printf("%02x", zone1[i]);
	printf("\n");
	if(ret = cm_WriteUserZone(0xb,0x0,zone1,8))
	{
		printf("err write user zone1 %x\n",ret);
		return ret;
	}
	sprintf(str, "zone1 is: %02x%02x%02x%02x%02x%02x%02x%02x\n", zone1[0],zone1[1],zone1[2],zone1[3],zone1[4],zone1[5],zone1[6],zone1[7]);
	fputs(str, fp);

	//write to zone2
	sleep(1);
	time(&now_time);
	srandom(now_time);
	i_Tmp = random() + now_time;
	if(ret = cm_SetUserZone(0xb,0x2,0x0)) {
		printf("err set user zone2 %x\n",ret);
		return ret;
	}
	mdelay(30);
	time(&now_time);
	srandom(now_time * 4);
	i_Tmp2 = random() + now_time * 4;
	set_value_8(i_Tmp, i_Tmp2, zone2);
	printf("zone2 is :");
	for(i = 0; i < 8; i++)
	    printf("%02x", zone2[i]);
	printf("\n");
	if(ret = cm_WriteUserZone(0xb,0x0,zone2,8)) 
	{
		printf("err write user zone2 %x\n",ret);
		return ret;
	}
	sprintf(str, "zone2 is: %02x%02x%02x%02x%02x%02x%02x%02x\n", zone2[0],zone2[1],zone2[2],zone2[3],zone2[4],zone2[5],zone2[6],zone2[7]);
	fputs(str, fp);

	//write to zone3
	sleep(1);
	time(&now_time);
	srandom(now_time);
	i_Tmp = random() + now_time;
	if(ret = cm_SetUserZone(0xb,0x3,0x0)) {
		printf("err set user zone3 %x\n",ret);
		return ret;
	}
	mdelay(30);
	time(&now_time);
	srandom(now_time * 5);
	i_Tmp2 = random() + now_time * 5;
	set_value_8(i_Tmp, i_Tmp2, zone3);
	printf("zone3 is :");
	for(i = 0; i < 8; i++)
	    printf("%02x", zone3[i]);
	printf("\n");
	if(ret = cm_WriteUserZone(0xb,0x0,zone3,8)) {
		printf("err write user zone3 %x\n",ret);
		return ret;
	}
	sprintf(str, "zone3 is: %02x%02x%02x%02x%02x%02x%02x%02x\n\n", zone3[0],zone3[1],zone3[2],zone3[3],zone3[4],zone3[5],zone3[6],zone3[7]);
	fputs(str, fp);
	system("sync");
	fclose(fp);
	system("sync");

#endif
	mdelay(30);
	dbg("read PAC first\n");
#if 0 
	if(ret = cm_ReadConfigZone(0xb,0xe8,&readback,0x01)) {
		printf("err read unlock memory %x\n",ret);
		return ret;
	}
	mdelay(30);
	dbg("PAC of Write7 is %x\n", readback);
	//if(readback!=0xff) {printf("verify ps7 has err\n");return -1;}
#endif

	dbg("unlock memory \n");
	//printf("addr 0x%x 0x%x\n",&(ps7[0]),ps7);
	//unlock memory config
	if(ret = cm_VerifyPassword(0xb,ps7,0x07,0x00)) {
		printf("err unlock memory %x\n",ret);
		return ret;
	}
	mdelay(30);
//	ll_PowerOn();//test 
//	if(SUCCESS != cm_Init()) printf("err init cm\n");
	mdelay(30);
#if 0 
	if(ret = cm_ReadConfigZone(0xb,0xe9,buf,0x03)) {
		printf("err read unlock memory %x\n",ret);
		return ret;
	}
	printf("unlock memory ok read back %x %x %x\n",buf[0],buf[1],buf[2]);
	mdelay(30);
//	if(readback!=0xff) {printf("verify ps7 err\n");return -1;}
#endif
#if	1
	dbg("write memory config\n");
	//write mfg_code
	if(ret = cm_WriteConfigZone(0xb,0xc,mfg_code,4,0x0)){
		printf("err write mfg_code %x\n",ret);
		return ret;
	}
	mdelay(30);
	//write NC
	if(ret = cm_WriteConfigZone(0xb,0x19,nc,7,0x0)) {
		printf("err write NC %x\n",ret);
		return ret;
	}
	mdelay(30);
	//write issuer code
	if(ret = cm_WriteConfigZone(0xb,0x40,issuer,16,0x0)) {
		printf("err write issuer %x\n",ret);
		return ret;
	}
	mdelay(30);
	//write ARPR
	if(ret = cm_WriteConfigZone(0xb,0x20,arpr,8,0x0)) {
		printf("err write ARPR %x\n",ret);
		return ret;
	}
	mdelay(30);
	//write Ci Si Gi
	dbg("write ci si gi\n");
	if(ret = cm_WriteConfigZone(0xb,0x51,C0,7,0x0)) {
		printf("err write C0 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0x61,C1,7,0x0)) {
		printf("err write C1 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0x71,C2,7,0x0)) {
		printf("err write C2 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0x81,C3,7,0x0)) {
		printf("err write C3 %x\n",ret);
		return ret;
	}
	mdelay(30);

	if(ret = cm_WriteConfigZone(0xb,0x58,S0,8,0x0)) {
		printf("err write S0 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0x68,S1,8,0x0)) {
		printf("err write S1 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0x78,S2,8,0x0)) {
		printf("err write S2 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0x88,S3,8,0x0)) {
		printf("err write S3 %x\n",ret);
		return ret;
	}
	mdelay(30);

	if(ret = cm_WriteConfigZone(0xb,0x90,G0,8,0x0)) {
		printf("err write G0 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0x98,G1,8,0x0)) {
		printf("err write G1 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0xa0,G2,8,0x0)) {
		printf("err write G2 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0xa8,G3,8,0x0)) {
		printf("err write G3 %x\n",ret);
		return ret;
	}
	mdelay(30);
	//set passwd
	dbg("set passwd\n");
	if(ret = cm_WriteConfigZone(0xb,0xb1,setpasswd0,7,0x0)) {
		printf("err write ps0 %x\n",ret);
		return ret;
	}
	
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0xb9,setpasswd1,7,0x0)) {
		printf("err write ps1 %x\n",ret);
		return ret;
	}
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0xc1,setpasswd2,7,0x0)) {
		printf("err write ps2 %x\n",ret);
		return ret;
	}
#endif
	mdelay(30);
	if(ret = cm_WriteConfigZone(0xb,0xe9,setpasswd7,7,0x0)) {
		printf("err write ps7 %x\n",ret);
		return ret;
	}
	mdelay(30);
	//readback data;

	if(ret = cm_ReadConfigZone(0xb,0x0,buf,0xf0)) {
		printf("err readback buf %x\n",ret);
		return ret;
	}
	mdelay(10);
#if 0
	printf("readback data:......\n");
	for(i=0;i<0xf0;i++){
		printf("%x ",buf[i]);
		if(!((i+1)%16)) printf("\n");
	}
#endif

	if(ret = cm_WriteFuse(0x0b,0x06)) {printf("err write fuse 06 %x\n",ret);return ret;
	}
	mdelay(30);
	if(ret = cm_WriteFuse(0x0b,0x04)) {printf("err write fuse 04 %x\n",ret);return ret;
	}	
	mdelay(30);
	if(ret = cm_WriteFuse(0x0b,0x00)) {printf("err write fuse 00 %x\n",ret);return ret;
	}
	mdelay(30);
	if(ret = cm_ReadFuse(0x0b,obuf)) {printf("err read fuse %x\n",ret);return ret;
	}
	if(obuf[0]!=0) printf("something err in write fuse\n");

	sprintf(str, "cat %s", save_file);
	printf("%s: \n", str);
	system(str);
	printf("fuse write over! only once ! don't write again\n");

	close(fd);

	return 0;

}
#endif
int do_rd_fuse(int argc, char *argv[])
{
	RETURN_CODE ret;
	uchar obuf[0];
	ll_PowerOn();
	mdelay(30);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	if(ret = cm_ReadFuse(0x0b,obuf)) {printf("err read fuse %x obuf is 0x%x\n",ret,obuf[0]);return ret;
	}
	printf("read back fuse is 0x%x\n",obuf[0]);
	return 0;
}
int do_mtz (int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[2];
	ll_PowerOn();
	mdelay(30);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	printf("argc is %d\n",argc);
	buf[0] = strtoul(argv[1], NULL, 16);
	buf[1] = strtoul(argv[2], NULL, 16);
	printf("write mtz 0x%x 0x%x\n",buf[0],buf[1]);
	ret = cm_WriteConfigZone(0xb,0xa,buf,2,0x0);
	if(ret!=SUCCESS) {printf("err write to mtz,return 0x%x\n",ret);
	return -1;
	}
	mdelay(30);
	ret = cm_ReadConfigZone(0xb,0xa,obuf,2);
	if(ret!=SUCCESS) {printf("err read from mtz,return 0x%x\n",ret);
	printf("read mtz 0x%x 0x%x\n",obuf[0],obuf[1]);
	return -1;
	}
	printf("read mtz 0x%x 0x%x\n",obuf[0],obuf[1]);
	return 0;
}
int do_add_fuse (int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[2];
	ll_PowerOn();
	mdelay(30);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = strtoul(argv[1], NULL, 16);
	if(ret = cm_WriteFuse(0x0b,buf[0])) {printf("err write fuse 0x%x 0x%x\n",buf[0],ret);return ret;
	}
	mdelay(30);
	return 0;
}

int do_passwd7 (int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char which;
	which = strtoul(argv[1], NULL, 16);
//	printf(" %x %x %x\n",passwd7[0],passwd7[1],passwd7[2]);
//	ll_PowerOn();
	mdelay(30);
//	if(SUCCESS != cm_Init()) printf("err init cm\n");
	
	mdelay(30);
	if(0x7 == which){
	ret = cm_VerifyPassword(0xb,passwd7,0x07,0x01);
	if (ret != SUCCESS) printf("passwd7 fail 0x%x\n",ret);
	}
	else if(0x2 == which){
	ret = cm_VerifyPassword(0xb,passwd2,0x02,0x01);
	if (ret != SUCCESS) printf("passwd2 fail 0x%x\n",ret);
	}
	else if(0x1 == which){
	ret = cm_VerifyPassword(0xb,passwd1,0x01,0x01);
	if (ret != SUCCESS) printf("passwd1 fail 0x%x\n",ret);
	}
	else if(0x0 == which){
	ret = cm_VerifyPassword(0xb,passwd0,0x00,0x01);
	if (ret != SUCCESS) printf("passwd0 fail 0x%x\n",ret);
	}
	else printf("not support other password verify\n");

	printf("verify passwd ok\n");
	return 0;
}

int do_passwd (int argc, char *argv[])
{
	RETURN_CODE ret;
	int i;
	int which;
	unsigned char a[8],b[8];
	which = strtoul(argv[1], NULL, 16);
	ll_PowerOn();
	mdelay(30);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	
	mdelay(30);
	if(1 == which){
	for(i=0;i<8;i++){
		a[i] = G1[i];
		b[i] = G1[i];
	}
	ret = cm_VerifyCrypto(0xb, 1, a, NULL, 0x0);
	if (ret != SUCCESS) printf("fail 0x%x\n",ret);

	mdelay(30);
	ret = cm_VerifyCrypto(0xb, 1, b, NULL, 0x1);
	if (ret != SUCCESS) printf("fail 2 0x%x\n",ret);
	}
	else if(0 == which){
	for(i=0;i<8;i++){
		a[i] = G0[i];
		b[i] = G0[i];
	}

	ret = cm_VerifyCrypto(0xb, 0, a, NULL, 0x0);
	if (ret != SUCCESS) printf("fail 0x%x\n",ret);

	mdelay(30);
	ret = cm_VerifyCrypto(0xb, 0, b, NULL, 0x1);
	if (ret != SUCCESS) printf("fail 2 0x%x\n",ret);
	}
	else if(2 == which){
	for(i=0;i<8;i++){
		a[i] = G2[i];
		b[i] = G2[i];
	}

	ret = cm_VerifyCrypto(0xb, 2, a, NULL, 0x0);
	if (ret != SUCCESS) printf("fail 0x%x\n",ret);

	mdelay(30);
	ret = cm_VerifyCrypto(0xb, 2, b, NULL, 0x1);
	if (ret != SUCCESS) printf("fail 2 0x%x\n",ret);
	}

	else if(3 == which){
//	printf("prinf G3\n");
	for(i=0;i<8;i++){
//		printf(" 0x%x ",G3[i]);
		a[i] = G3[i];
		b[i] = G3[i];
	}
//	printf("\n");
	ret = cm_VerifyCrypto(0xb, 3, a, NULL, 0x0);
	if (ret != SUCCESS) printf("fail 0x%x\n",ret);
	mdelay(30);
	ret = cm_VerifyCrypto(0xb, 3, b, NULL, 0x1);
	if (ret != SUCCESS) printf("fail 2 0x%x\n",ret);
	}


	printf("verify Crypto ok\n");
	return 0;
}

int do_nopasswd (int argc, char *argv[])
{
	RETURN_CODE ret;
	ll_PowerOn();
	mdelay(30);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	mdelay(30);
	ret = cm_ResetCrypto(0xb);
	if (ret != SUCCESS) printf("fail 0x%x\n",ret);
	printf("reset  Crypto ok\n");
	return 0;
}

int do_readcfg (int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[128];
	unsigned char i;
	ll_PowerOn();
	//if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = strtoul(argv[1], NULL, 16); //addr
	buf[1] = strtoul(argv[2], NULL, 16); //count
	ret = cm_ReadConfigZone(0xb,buf[0],obuf,buf[1]);
	if(ret!=SUCCESS) {
	printf("err read from cfgmem,return 0x%x\n",ret);
	printf("read  0x%x 0x%x\n",obuf[0],obuf[1]);
	return -1;
	}
	for(i=0;i<buf[1];i++){
	printf("0x%x ",obuf[i]);
	if(!((1+i)%16)) printf("\n");
	}
	return 0;
}

int do_writecfg (int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[128];
	unsigned char i;
	ll_PowerOn();
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = strtoul(argv[1], NULL, 16); //addr
	buf[1] = strtoul(argv[2], NULL, 16); //data
	ret = cm_VerifyPassword(0xb,passwd77,0x07,0x00);
	if(ret!=SUCCESS) {
		printf("verifypassword err\n");
	}
	mdelay(30);
	ll_PowerOn();
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	ret = cm_WriteConfigZone(0xb,buf[0],buf[1],1,FALSE);
	if(ret!=SUCCESS) {
	printf("err write cfgmem,return 0x%x\n",ret);
	printf("write  0x%x 0x%x\n",buf[0],buf[1]);
	return -1;
	}
	return 0;
}

int do_readzone (int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[3];
	unsigned char obuf[128];
	unsigned char i;
	ll_PowerOn();
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = strtoul(argv[1], NULL, 16); //zone
	buf[1] = strtoul(argv[2], NULL, 16); //addr
	buf[2] = strtoul(argv[3], NULL, 16); //count

	if(ret = cm_SetUserZone(0xb,buf[0],0x0)) { 
		printf("error set user zone %x\n",ret);
		return -1;
	}
	ret = cm_ReadUserZone(0x0,buf[1],obuf,buf[2]);
	if(ret!=SUCCESS) {
	printf("err read from zone,return 0x%x\n",ret);
	return -1;
	}
	for(i=0;i<buf[2];i++){
	printf("0x%x ",obuf[i]);
	if(!((1+i)%16)) printf("\n");
	}
	return 0;
}

#if 0
U_BOOT_CMD(
	mtz,     3,     1,      do_mtz,
	"mtz      - memory test\n",
	"data1 data2\n    - memory readback display\n"
);

U_BOOT_CMD(
	add_fuse,     2,     1,      do_add_fuse,
	"add_fuse      - fuse test\n",
	"data 06 04 00\n    - write fuse test\n"
);


U_BOOT_CMD(
	readcfg,     3,     1,      do_readcfg,
	"readcfg      - read memory config\n",
	"addr count\n    - memory readback display\n"
);
U_BOOT_CMD(
	writecfg,     3,     1,      do_writecfg,
	"writecfg      - write memory config\n",
	"addr data\n    - write memory one byte\n"
);

U_BOOT_CMD(
	readzone,     4,     1,      do_readzone,
	"readzone      - read user zone memory\n",
	"zone addr count\n    - memory readback display\n"
);

#if 1 
U_BOOT_CMD(
	rd_fuse,     1,     1,      do_rd_fuse,
	"rd_fuse      - read fuse\n",
	"warning\n    - read fuse back\n"
);
#endif

#if 1 
U_BOOT_CMD(
	wr_fuse,     1,     1,      do_wr_fuse,
	"wr_fuse      - write fuse\n",
	"warning\n    - must do mtz first\n"
);
#endif
U_BOOT_CMD(
	passwd7,     2,     1,      do_passwd7,
	"do_passwd7      - passwd7\n",
	"which passwd\n    - for verify passwd \n"
);

U_BOOT_CMD(
	passwd,     2,     1,      do_passwd,
	"do_passwd      - passwd\n",
	"warning\n    - for test crycro mode\n"
);

U_BOOT_CMD(
	nopasswd,     1,     1,      do_nopasswd,
	"do_nopasswd      - reset crycro\n",
	"warning\n    - for test crycro mode\n"
);
#endif
