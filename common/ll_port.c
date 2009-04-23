#include <linux/types.h>
#include <common.h>
//#include <stdio.h>
//#include <linux/delay.h>
//#include "unistd.h"
#include <config.h>

//#include <lib_CM.h>
#include <lib_Crypto.h>
#include <ll_port.h>
//#define ll_debug


#define LL_START_TRIES 10
#define LL_PWRON_CLKS 15
#define LL_ACK_TRIES 8
#define GPIO_SCL   (('N'-'A')*16 + 0)	    //HHTECH GPN0
#define GPIO_SDA   (('N'-'A')*16 + 1)	    //HHTECH GPN1

void gpio_init(void)
{
	gpio_direction_output(GPIO_SCL,1);
	gpio_direction_output(GPIO_SDA,1);

}
uchar rand ( void )
{
#if 0 
	unsigned long ll;
	uchar my_choose;

	ll = REG_TCU_TCNT(0)*12345+2345;
	my_choose =(uchar)(ll%100);
	return my_choose;
	//lzcx
#endif
//	    return ( rand () );
//	return(srandom(time(0) | getpid()));
}

#if 1
void ll_Delay(unsigned char ucDelay)
{
	int i;
	for( ; ucDelay != 0 ; ucDelay--)
		udelay(25); //25 is ok
		//udelay(2);
}

void ll_Clockhigh(void)
{
	ll_Delay(1);
	gpio_direction_output(GPIO_SCL,1);
	ll_Delay(1);
}

void ll_Clocklow(void)
{
	ll_Delay(1);
	gpio_direction_output(GPIO_SCL,0);
	ll_Delay(2);
}

void ll_ClockCycle(void)
{
	ll_Clocklow();
	ll_Clockhigh();
}

void ll_ClockCycles(unsigned char ucCount)
{
	for( ; ucCount > 0; ucCount--)
		ll_ClockCycle();
}

void ll_Datahigh(void)
{
	ll_Delay(1);
	gpio_direction_input(GPIO_SDA);
	ll_Delay(2);
}

void ll_Datalow(void)
{
	ll_Delay(1);
	gpio_direction_output(GPIO_SDA,0);
	ll_Delay(2);
}

unsigned char ll_Data(void)
{
	ll_Delay(1);
	gpio_direction_input(GPIO_SDA);
	ll_Delay(2);
	return(gpio_get_value(GPIO_SDA));
}

void ll_Start(void)
{
	ll_Clocklow();
	ll_Datahigh();
	ll_Delay(4);
	ll_Clockhigh();
	ll_Delay(4);
	ll_Datalow();
	ll_Delay(4);
	ll_Clocklow();
	ll_Delay(4);
}

void ll_Stop(void)
{
	ll_Clocklow();
	ll_Datalow();
	ll_Clockhigh();
	ll_Delay(8);
	ll_Datahigh();
	ll_Delay(4);
}

void ll_AckNak(unsigned char ucAck)
{
	ll_Clocklow();
	if(ucAck)
		ll_Datalow();
	else
		ll_Datahigh();
	ll_Clockhigh();
	ll_Clocklow();
}

void ll_PowerOn(void)
{
	gpio_init();
	printf("ll power on\n");
	ll_Datahigh();
	ll_Clocklow();
	ll_ClockCycles(LL_PWRON_CLKS);
}

unsigned char ll_Write(unsigned char ucData)
{
	unsigned char i;
	#ifdef ll_debug
	unsigned char ucData1 = ucData;
	#endif
	for(i=0 ; i<8; i++){
		ll_Clocklow();
		if(ucData&0x80)
			ll_Datahigh();
		else
			ll_Datalow();
		ll_Clockhigh();
		ucData = ucData << 1;
	}
	ll_Clocklow();
	ll_Datahigh();
	ll_Delay(8);
	ll_Clockhigh();
	for(i = 0 ; i < LL_ACK_TRIES; i++)
	{
		if(ll_Data() == 0)
		{
			i = 0 ;
			break;
		}
	}
	ll_Clocklow();
	#ifdef ll_debug
	printf("ll_write ucData1 i: 0x%2x 0x%2x\n",ucData1,i);
	#endif
	return i;
}

	
unsigned char ll_Read(void)
{
	unsigned char i;
	unsigned char rByte = 0;
	ll_Datahigh();
	for(i=0x80; i ; i=i>>1)
	{
		ll_ClockCycle();
		if(ll_Data())
			rByte |= i;
		ll_Clocklow();
	}
	return rByte;
}

RETURN_CODE ll_SendCommand(puchar pucInsBuf, uchar numBytes)
{
	unsigned char i;
	i = LL_START_TRIES;
	while(i){
		ll_Start();
		if(0 == ll_Write(pucInsBuf[0])) break;
		if( --i ==0 ) return FAIL_CMDSTART; 
	}
	for(i = 1 ; i < numBytes ; i++){
		if(ll_Write(pucInsBuf[i]) != 0) return FAIL_CMDSEND;
	}
	#ifdef ll_debug
	printf("send command:");
	for(i = 0 ; i < numBytes ; i++)
		printf("0x%2x ",pucInsBuf[i]);
	printf("\n");
	#endif
	return SUCCESS;
}

RETURN_CODE ll_ReceiveData(puchar pucRecBuf, uchar uclen)
{
	int i = 0;
	#ifdef ll_debug
	uchar ucSize = uclen;
	puchar pucData = pucRecBuf;
	#endif
	if(uclen > 0)
	{

		while(--uclen)
		{
			*pucRecBuf++ = ll_Read();
			ll_AckNak(TRUE);
			i++;
		}
			*pucRecBuf = ll_Read();
			ll_AckNak(FALSE);
	}
	ll_Stop();
	#ifdef ll_debug
	printf("receive Data:");
	for(i = 0; i < ucSize; i++)
		printf("0x%2x ",pucData[i]);
	printf("\n");
	#endif
	return SUCCESS;
}

RETURN_CODE ll_SendData(puchar pucSendBuf, uchar ucLen)
{
	int i;
	for(i=0;i<ucLen;i++){
		if(ll_Write(pucSendBuf[i]) != 0) return FAIL_WRDATA;
	}
	ll_Stop();
	#ifdef ll_debug
	printf("ll senddata:");
	for(i = 0; i < ucLen; i++)
		printf("0x%x ",pucSendBuf[i]);
	printf("\n");
	#endif
	return SUCCESS;
}

void ll_WaitClock(unsigned char loop)
{
	unsigned char i,j;
	for(j=0;j<loop*4;j++){
		ll_Start();
		for(i=0;i<15;i++) ll_ClockCycle();
		ll_Stop();
	}
}

#endif
