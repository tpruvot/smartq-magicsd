/** \file
 *  \brief Structure defines for low level interface.
 *  
 *  Includes TWI Structures
 */
#include "lib_Crypto.h"

#ifndef LL_PORT_H
#define LL_PORT_H

/** Low Level TWI Defines */
#define LL_START_TRIES 10 ///< Defines # of Start attempts to send command
#define LL_PWRON_CLKS  15 ///< Number of clocks to reset CM on power up
#define LL_ACK_TRIES    8 ///< Number of times to try to get ACK on a write

/** \brief Bitwise structure for the CrytpoMemory TWI-like Port
 *
 * Define the pin positions for the CryptoMemory I/O port in this structure.
 * This structure assumes that the clock and data pins are in the same I/O port.
 * This definition has the clock pin on the first port bit and the data pin on the
 * second port bit.  You may move these around to reflect your actual hardware
 * implementation.  
 */
typedef struct
{
    uchar clock_pin:1; ///< Clock pin on port pin 0
    uchar data_pin:1;  ///< Data  pin on port pin 1
    uchar jtag_pins:4; ///< DO NOT USE THESE JTAG pins!
    uchar nused_pin:2; ///< Temporarily not used pins
} TWI_PINS ;

/** \brief Hardware registers for PORT, DDR and PIN addresses.
 *
 * You <b> must </b> instantiate one of these in your 'c' files.
 * This structure is set up such that the input register is followed by the
 * data direction register, followed by the output register.
 * It assumes that they are in this order in the hardware addresses for the
 * port chosen.  If you have a different order for these register addresses, 
 * you will want to modify this structure accordingly.
 *
 * \remarks Example:  \n
 * <tt> TWI_PORT *pTWI = (TWI_PORT *)0x33 ; </tt> \n
 * This definition assigns a pointer to this hardware structure at address 0x33.
 * With the structure below, the pin input register is at address 0x33, the port
 * data direction register is at address 0x34, and the port output register is at
 * address 0x35.  You will want to make these match your processor's I/O port
 * addresses.
 */
typedef struct
{
    TWI_PINS pin_reg,        ///< pin input register
             dir_reg,        ///< data direction register
             port_reg;       ///< port output register
} TWI_PORT;

typedef enum {
    SCL_HIGH,
    SCL_LOW,
    SDA_HIGH,
    SDA_LOW,
    SDA_DATA
} I2C_CMD;

extern void ll_Delay(unsigned int ucDelay);
void ll_Clockhigh(void);
void ll_Clocklow(void);
void ll_ClockCycle(void);
void ll_ClockCycles(uchar ucCount);
void ll_Datahigh(void);
void ll_Datalow(void);
uchar ll_Data(void);
void ll_Start(void);
void ll_Stop(void);
void ll_AckNak(uchar ucAck);
void ll_PowerOn(void);
uchar ll_Write(uchar ucData);
uchar ll_Read(void);
RETURN_CODE ll_SendCommand(puchar pucInsBuf, uchar ucLen);
RETURN_CODE ll_ReceiveData(puchar pucRecBuf, uchar ucLen);
RETURN_CODE ll_SendData(puchar pucSendBuf, uchar ucLen);
void ll_WaitClock(uchar ucLoop);

#endif // LL_PORT_H

