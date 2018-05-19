/***************************************************************************************************************************
* Copyright (C)  TaiSing Industrial Company Ltd.(Shenzhen).
* Project Name:  Pulse II
* File name:     drv_spi.c
* Description:   This file for SPI function driver

* Author: Jamesl      	                                                                               Date: 2015-March-27
***************************************************************************************************************************/
/**************************************************** Edit History *********************************************************
* 2015-Jul-25       James Lin                Make this revision for MP releae any other change must add new revision history
* 2016-Nov-10       Sumit Gambhir            Pulse-3 LEDs changes
***************************************************************************************************************************/
//#include <plib.h>
//#include "..\..\..\Inc\sys_common.h"
#include "..\Rendering.h"

//#include "stm32f0xx_hal.h"

extern void SPI1_SendData (uint8_t *pData, uint16_t Size);


//#define HW_V3
//#define LED_TEST
#define LED_COUNT          96
#define LED_ROW            8
#define LED_COL            12
#define PRE_LED_DATA_LEN   12
#define PRE_LED_BIT        24

#define RED    0x5B0000
#define BLUE   0x00005B
#define GREE   0x005B00
#define WHITE  0x5B5B5B
 
#define LED_DATA_BUF_SIZE       ( LED_COUNT * 12 )                 /* 1188 is needed */
#define PHY_COLOR_ELEMENT_NUM   288                                /* REN_PHY_FRAME_W * REN_PHY_FRAME_H * 3 */

/************************************************* Local variables definitions *********************************/
static const uint8_t ledConvertData[] = { 0x88, 0x8E, 0xE8, 0xEE };
/*
LOCAL CONST UINT8 rgbCalibration[ 256 ] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,3,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,
	7,7,8,8,8,9,9,9,10,10,11,11,11,12,12,13,13,13,14,
	14,15,15,16,16,17,17,18,18,19,19,20,20,21,22,22,23,
	23,24,25,25,26,26,27,28,28,29,30,30,31,32,33,33,34,
	35,35,36,37,38,39,39,40,41,42,43,43,44,45,46,47,48,
	49,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	65,66,67,68,69,70,71,73,74,75,76,77,78,79,81,82,83,
	84,85,87,88,89,90,91,93,94,95,97,98,99,100,102,103,
	105,106,107,109,110,111,113,114,116,117,119,120,121,
	123,124,126,127,129,130,132,133,135,137,138,140,141,
	143,145,146,148,149,151,153,154,156,158,159,161,163,
	165,166,168,170,172,173,175,177,179,181,182,184,186,
	188,190,192,194,196,197,199,201,203,205,207,209,211,
	213,215,217,219,221,223,225,227,229,231,234,236,238,
	240,242,244,246,248,251,253,255
};
*/
static  uint8_t txferTxBuff[ LED_DATA_BUF_SIZE ];   // the buffer containing the data to be transmitted
//DmaChannel  dmaTxChn = DMA_CHANNEL1;            // DMA channel to use for our example
                                                // NOTE: the DMA ISR setting has to match the channel number
//SpiChannel  spiTxChn = SPI_CHANNEL1;            // the transmitting SPI channel to use in our example

/*********************************************************************
 * Function:        DmaDoM2Spi
 *
 * PreCondition:    None
 *
 * Input:	    None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:	   Example for sending a memory buffer to an SPI channel using the DMA and SPI Peripheral Lib.
 * 		   The DMA interrupts will  be enabled and will signal when the transfer is completed.
 *
 * Note:            None.
 ********************************************************************/

#ifdef LED_TEST
PHYDIS_T test;
#endif

void DmaDoM2Spi(void)
{
#if 0 //def LED_TEST
    UINT16 h, w, color;
#endif
    uint16_t idx, k, rgb_data;
    uint16_t colorInd;
    uint8_t *colorPtr;

#if 0 //def LED_TEST
    UINT8 testdatar,testdatag,testdatab;

    for(h=0;h<REN_PHY_FRAME_H;h++)
    {

            switch(h)
            {
                case 0:
                testdatar=0x30;
                testdatag=0x00;
                testdatab=0x00;
                break;
                case 1:
                testdatar=0x00;
                testdatag=0x30;
                testdatab=0x00;
                break;
                case 2:
                testdatar=0x00;
                testdatag=0x00;
                testdatab=0x30;
                break;
                case 3:
                testdatar=0x30;
                testdatag=0x30;
                testdatab=0x30;
                break;
                case 4:
                testdatar=0x30;
                testdatag=0x10;
                testdatab=0x00;
                break;
                case 5:
                testdatar=0x10;
                testdatag=0x30;
                testdatab=0x00;
                break;
                case 6:
                testdatar=0x10;
                testdatag=0x00;
                testdatab=0x30;
                break;
                case 7:
                testdatar=0x30;
                testdatag=0x00;
                testdatab=0x30;
                break;
                case 8:
                testdatar=0x00;
                testdatag=0x00;
                testdatab=0x00;
                break;
                case 9:
                testdatar=0x10;
                testdatag=0x30;
                testdatab=0x30;
                break;
                case 10:
                testdatar=0x0;
                testdatag=0x0;
                testdatab=0x0;
                break;                
            }

        
        for(w=0;w<REN_PHY_FRAME_W;w++)
        {
            test.phyDisColor[h][w].r=testdatar;
            test.phyDisColor[h][w].g=testdatag;
            test.phyDisColor[h][w].b=testdatab;
                
        }
    }


    
    for(w=0;w<REN_PHY_FRAME_W;w++)
    {
        for(h=0;h<REN_PHY_FRAME_H;h++)
        {
            if(w&1)
            {
                idx=(w*REN_PHY_FRAME_H*PRE_LED_DATA_LEN)+((8-h)*PRE_LED_DATA_LEN);
            }
            else
            {
                idx=(w*REN_PHY_FRAME_H*PRE_LED_DATA_LEN)+(h*PRE_LED_DATA_LEN);
            }
            for(color=0;color<3;color++)
            {
                switch(color)
                {
                    case 0:
                        rgb_data=test.phyDisColor[h][w].g;
                    break;
                    case 1:
                        rgb_data=test.phyDisColor[h][w].r;
                    break;
                    case 2:
                        rgb_data=test.phyDisColor[h][w].b;
                    break;
                        
                }

                for(k=0;k<8;k++)
                {
                        if(k&1)
                        {
                            if(rgb_data&0x80)
                                txferTxBuff[idx]|=0x0E;
                            else
                                txferTxBuff[idx]|=0x08;
                            idx++;
                        }
                        else
                        {
                            if(rgb_data&0x80)
                                txferTxBuff[idx]|=0xE0;
                            else
                                txferTxBuff[idx]|=0x80;
                        }
                        rgb_data=rgb_data<<1;
                    }
                }
            }
            
        }
#else    
    idx = 0;
    colorPtr = ( uint8_t * )&PhyDis[ PhyDisActiveInd ];
    for( colorInd = 0; colorInd < PHY_COLOR_ELEMENT_NUM; colorInd++  )
    {
#if 0 //def HW_V3
        /* @LM: No optimization and should not match with the latest implementation!! */
        if(w&1)
        {
            idx=(w*REN_PHY_FRAME_H*PRE_LED_DATA_LEN)+((8-h)*PRE_LED_DATA_LEN);
        }
        else
        {
            idx=(w*REN_PHY_FRAME_H*PRE_LED_DATA_LEN)+(h*PRE_LED_DATA_LEN);
        }
#endif

//        rgb_data = rgbCalibration[ *( colorPtr + colorInd ) ];             /* Color is arranged in the order: r -> g -> b */
        rgb_data = *( colorPtr + colorInd );                              /* Color is arranged in the order: r -> g -> b */

        idx += 4;
        for( k = 1; k < 5; k++ )
        {
            txferTxBuff[ idx - k ] = ledConvertData[ ( rgb_data & 0x03 ) ];
            rgb_data = rgb_data >> 2;
        }
    }
#endif /* LED_TEST */

    //DmaChnStartTxfer( dmaTxChn, DMA_WAIT_NOT, 0 );	// force the DMA transfer: the SPI TBE flag it's already been active

	//LED_Color_Different_8bit (txferTxBuff, PHY_COLOR_ELEMENT_NUM);
	//LED_Color_8Bit_Complement(txferTxBuff, LED_DATA_BUF_SIZE);
	//HAL_SPI_Transmit (&hspi1, txferTxBuff, LED_DATA_BUF_SIZE, 100);
    SPI1_SendData(txferTxBuff, LED_DATA_BUF_SIZE);

}



