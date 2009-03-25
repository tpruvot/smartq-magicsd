/*
 *  S3C64XX encryption 
 *
 *  Copyright (C) 2008 - 2009 HHTECH.
 *
 */

#include <common.h>
#include <command.h>
#include "../cpu/s3c64xx/ata.h"
#include <lib_Crypto.h>
#include <lib_Data.h>

#if 1 
int do_wr_fuse( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[0xf0];
	unsigned char obuf[2];
	unsigned char ps7[3]={0xdd,0x42,0x97};
	unsigned char i,readback;
	buf[0]=0x55;
	buf[1]=0xaa;
	printf("mtz test\n");
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
#if 1 
	printf("write zone\n");
	//write to zone0
	if(ret = cm_SetUserZone(0xb,0x0,0x0)) 
	{printf("err set user zone0 %x\n",ret);
	return ret;
	}
	mdelay(20);
	if(ret = cm_WriteUserZone(0xb,0x0,zone0,9)) 
	{	
		printf("err write user zone0 %x\n",ret);
		return ret;
	}
	mdelay(20);
	//write to zone1
	if(ret = cm_SetUserZone(0xb,0x1,0x0)) 
	{
		printf("err set user zone1 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteUserZone(0xb,0x0,zone1,9))
	{
		printf("err write user zone1 %x\n",ret);
		return ret;
	}
	mdelay(20);
	//write to zone2
	if(ret = cm_SetUserZone(0xb,0x2,0x0)) {
		printf("err set user zone2 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteUserZone(0xb,0x0,zone2,9)) 
	{
		printf("err write user zone2 %x\n",ret);
		return ret;
	}
	mdelay(20);
	//write to zone3
	if(ret = cm_SetUserZone(0xb,0x3,0x0)) {
		printf("err set user zone3 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteUserZone(0xb,0x0,zone3,9)) {
		printf("err write user zone3 %x\n",ret);
		return ret;
	}
	mdelay(20);
#endif
	printf("read PAC first\n");
#if 1 
	if(ret = cm_ReadConfigZone(0xb,0xe8,&readback,0x01)) {
		printf("err read unlock memory %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(readback!=0xff) {printf("verify ps7 has err\n");return -1;}
#endif

	printf("unlock memory \n");
	//printf("addr 0x%x 0x%x\n",&(ps7[0]),ps7);
	//unlock memory config
	if(ret = cm_VerifyPassword(0xb,ps7,0x07,0x00)) {
		printf("err unlock memory %x\n",ret);
		return ret;
	}
	mdelay(20);
//	ll_PowerOn();//test 
//	if(SUCCESS != cm_Init()) printf("err init cm\n");
	mdelay(20);
#if 0 
	if(ret = cm_ReadConfigZone(0xb,0xe9,buf,0x03)) {
		printf("err read unlock memory %x\n",ret);
		return ret;
	}
	printf("unlock memory ok read back %x %x %x\n",buf[0],buf[1],buf[2]);
	mdelay(20);
//	if(readback!=0xff) {printf("verify ps7 err\n");return -1;}
#endif
	printf("write memory config\n");
	//write mfg_code
	if(ret = cm_WriteConfigZone(0xb,0xc,mfg_code,4,0x0)){
		printf("err write mfg_code %x\n",ret);
		return ret;
	}
	mdelay(20);
	//write NC
	if(ret = cm_WriteConfigZone(0xb,0x19,nc,7,0x0)) {
		printf("err write NC %x\n",ret);
		return ret;
	}
	mdelay(20);
	//write issuer code
	if(ret = cm_WriteConfigZone(0xb,0x40,issuer,16,0x0)) {
		printf("err write issuer %x\n",ret);
		return ret;
	}
	mdelay(20);
	//write ARPR
	if(ret = cm_WriteConfigZone(0xb,0x20,arpr,8,0x0)) {
		printf("err write ARPR %x\n",ret);
		return ret;
	}
	mdelay(20);
	//write Ci Si Gi
	printf("write ci si gi\n");
	if(ret = cm_WriteConfigZone(0xb,0x51,C0,7,0x0)) {
		printf("err write C0 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0x61,C1,7,0x0)) {
		printf("err write C1 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0x71,C2,7,0x0)) {
		printf("err write C2 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0x81,C3,7,0x0)) {
		printf("err write C3 %x\n",ret);
		return ret;
	}
	mdelay(20);

	if(ret = cm_WriteConfigZone(0xb,0x58,S0,8,0x0)) {
		printf("err write S0 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0x68,S1,8,0x0)) {
		printf("err write S1 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0x78,S2,8,0x0)) {
		printf("err write S2 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0x88,S3,8,0x0)) {
		printf("err write S3 %x\n",ret);
		return ret;
	}
	mdelay(20);

	if(ret = cm_WriteConfigZone(0xb,0x90,G0,8,0x0)) {
		printf("err write G0 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0x98,G1,8,0x0)) {
		printf("err write G1 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0xa0,G2,8,0x0)) {
		printf("err write G2 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0xa8,G3,8,0x0)) {
		printf("err write G3 %x\n",ret);
		return ret;
	}
	mdelay(20);
	//set passwd
	printf("set passwd\n");
	if(ret = cm_WriteConfigZone(0xb,0xb1,setpasswd0,7,0x0)) {
		printf("err write ps0 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0xb9,setpasswd1,7,0x0)) {
		printf("err write ps1 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0xc1,setpasswd2,7,0x0)) {
		printf("err write ps2 %x\n",ret);
		return ret;
	}
	mdelay(20);
	if(ret = cm_WriteConfigZone(0xb,0xe9,setpasswd7,7,0x0)) {
		printf("err write ps7 %x\n",ret);
		return ret;
	}
	mdelay(20);
	//readback data;

	if(ret = cm_ReadConfigZone(0xb,0x0,buf,0xf0)) {
		printf("err readback buf %x\n",ret);
		return ret;
	}
	mdelay(10);
	printf("readback data:......\n");
	for(i=0;i<0xf0;i++){
		printf("%x ",buf[i]);
		if(!((i+1)%16)) printf("\n");
	}
	if(ret = cm_WriteFuse(0x0b,0x06)) {printf("err write fuse 06 %x\n",ret);return ret;
	}
	mdelay(20);
	if(ret = cm_WriteFuse(0x0b,0x04)) {printf("err write fuse 04 %x\n",ret);return ret;
	}	
	mdelay(20);
	if(ret = cm_WriteFuse(0x0b,0x00)) {printf("err write fuse 00 %x\n",ret);return ret;
	}
	mdelay(20);
	if(ret = cm_ReadFuse(0x0b,obuf)) {printf("err read fuse %x\n",ret);return ret;
	}
	if(obuf[0]!=0) printf("something err in write fuse\n");

	printf("fuse write over! only once ! don't write again\n");
	
	return 0;

}
#endif
int do_rd_fuse( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	uchar obuf[0];
	ll_PowerOn();
	mdelay(20);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	if(ret = cm_ReadFuse(0x0b,obuf)) {printf("err read fuse %x obuf is 0x%x\n",ret,obuf[0]);return ret;
	}
	printf("read back fuse is 0x%x\n",obuf[0]);
	return 0;
}
int do_mtz ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[2];
	ll_PowerOn();
	mdelay(20);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	printf("argc is %d\n",argc);
	buf[0] = simple_strtoul(argv[1], NULL, 16);
	buf[1] = simple_strtoul(argv[2], NULL, 16);
	printf("write mtz 0x%x 0x%x\n",buf[0],buf[1]);
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
	printf("read mtz 0x%x 0x%x\n",obuf[0],obuf[1]);
	return 0;
}
int do_add_fuse ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[2];
	ll_PowerOn();
	mdelay(20);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = simple_strtoul(argv[1], NULL, 16);
	if(ret = cm_WriteFuse(0x0b,buf[0])) {printf("err write fuse 0x%x 0x%x\n",buf[0],ret);return ret;
	}
	mdelay(20);
	return 0;
}

int do_passwd7 ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char which;
	which = simple_strtoul(argv[1], NULL, 16);
//	printf(" %x %x %x\n",passwd7[0],passwd7[1],passwd7[2]);
//	ll_PowerOn();
	mdelay(20);
//	if(SUCCESS != cm_Init()) printf("err init cm\n");
	
	mdelay(20);
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

int do_passwd ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	int i;
	int which;
	unsigned char a[8],b[8];
	which = simple_strtoul(argv[1], NULL, 16);
	ll_PowerOn();
	mdelay(20);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	
	mdelay(20);
	if(1 == which){
	for(i=0;i<8;i++){
		a[i] = G1[i];
		b[i] = G1[i];
	}
	ret = cm_VerifyCrypto(0xb, 1, a, NULL, 0x0);
	if (ret != SUCCESS) printf("fail 0x%x\n",ret);

	mdelay(20);
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

	mdelay(20);
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

	mdelay(20);
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
	mdelay(20);
	ret = cm_VerifyCrypto(0xb, 3, b, NULL, 0x1);
	if (ret != SUCCESS) printf("fail 2 0x%x\n",ret);
	}


	printf("verify Crypto ok\n");
	return 0;
}

int do_nopasswd ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	ll_PowerOn();
	mdelay(20);
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	mdelay(20);
	ret = cm_ResetCrypto(0xb);
	if (ret != SUCCESS) printf("fail 0x%x\n",ret);
	printf("reset  Crypto ok\n");
	return 0;
}

int do_readcfg ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[128];
	unsigned char i;
	ll_PowerOn();
	//if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = simple_strtoul(argv[1], NULL, 16); //addr
	buf[1] = simple_strtoul(argv[2], NULL, 16); //count
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

int do_writecfg ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[2];
	unsigned char obuf[128];
	unsigned char i;
	ll_PowerOn();
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = simple_strtoul(argv[1], NULL, 16); //addr
	buf[1] = simple_strtoul(argv[2], NULL, 16); //data
	ret = cm_VerifyPassword(0xb,passwd77,0x07,0x00);
	if(ret!=SUCCESS) {
		printf("verifypassword err\n");
	}
	mdelay(20);
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

int do_readzone ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
	unsigned char buf[3];
	unsigned char obuf[128];
	unsigned char i;
	ll_PowerOn();
	if(SUCCESS != cm_Init()) printf("err init cm\n");
	buf[0] = simple_strtoul(argv[1], NULL, 16); //zone
	buf[1] = simple_strtoul(argv[2], NULL, 16); //addr
	buf[2] = simple_strtoul(argv[3], NULL, 16); //count

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
int do_test_passwd ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	RETURN_CODE ret;
#if 0
	uchar read_user[9];
	uchar readback=0x00;
	uchar my_choose=0,i;
	long long ll;
	
	ll = REG_TCU_TCNT(0)*12345+2345;
	my_choose =(uchar)(ll%100);
	printf("random choose is %d\n",my_choose);
	if(ret = cm_Init()) return ret;
	
	if(my_choose<40){//use user 0
		if(ret = cm_SetUserZone(0x0,0x0,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0x0,passwd1,0x01,0x01)) return ret;
		mdelay(20);
		if(ret = cm_Init()) return ret;
		mdelay(20);
		if(ret = cm_ReadUserZone(0x00,0x00,read_user,9)) return ret;
		mdelay(20);
		for(i=0;i<9;i++){
			if(read_user[i] != zone0[i]) return ERROR_MAX;
		}
	}else if((my_choose>=40)&&(my_choose<70)){ //use user 1
		if(ret = cm_SetUserZone(0x0,0x1,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0x0,passwd2,0x02,0x01)) return ret;
		mdelay(20);
		if(ret = cm_Init()) return ret;
		mdelay(20);

		if(ret = cm_ReadUserZone(0x00,0x00,read_user,9)) return ret;
		mdelay(20);
		for(i=0;i<9;i++){
			if(read_user[i] != zone1[i]) return ERROR_MAX;
		}

	}else if((my_choose>=70)&&(my_choose<90)){ //user 2
		if(ret = cm_SetUserZone(0x0,0x2,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0x0,passwd0,0x00,0x01)) return ret;
		mdelay(20);
		if(ret = cm_Init()) return ret;
		mdelay(20);
		if(ret = cm_ReadUserZone(0x00,0x00,read_user,9)) return ret;
		mdelay(20);
		for(i=0;i<9;i++){
			if(read_user[i] != zone2[i]) return ERROR_MAX;
		}

	}else if(my_choose>=90){
		if(ret = cm_SetUserZone(0x0,0x3,0x0)) return ret;
		mdelay(20);
		if(ret = cm_VerifyPassword(0x0,passwd7,0x07,0x01)) return ret;
		mdelay(20);
		if(ret = cm_Init()) return ret;
		mdelay(20);
		if(ret = cm_ReadUserZone(0x00,0x00,read_user,9)) return ret;
		mdelay(20);
		for(i=0;i<9;i++){
			if(read_user[i] != zone3[i]) return ERROR_MAX;
		}
	}
#endif
	ret = cm_randomread();
	if(ret) printf("err verify passwd, 0x%x\n",ret);
	else if(ret==SUCCESS) printf("pass \n");
	return 0;
}
#endif

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

#if 0
U_BOOT_CMD(
	test_ps,     1,     1,      do_test_passwd,
	"test_ps      - passwd test\n",
	"\n    - \n"
);
#endif
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
