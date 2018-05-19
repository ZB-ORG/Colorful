/****************************************************************************************************************
* Copyright (C)  TaiSing Industrial Company Ltd.(Shenzhen).
* Project Name:  Pulse II
* File name:     GlobalMemory.c
* Description:   GlobalMemory module

* Author: LM Cheong                             Date: 2015-March-15
*****************************************************************************************************************/
/**************************** Edit History **********************************************************************
* DATE               NAME                 DESCRIPTION
* 2015-Mar-15        LM Cheong               Create.
* 2015-Jul-25       James Lin                Make this revision for MP releae any other change must add new revision history
***************************************************************************************************************************/
//#include "App_bat.h"
#include "GlobalMemory.h"
//#include "SPIflash.h"

/************************************************* Type declarations ******************************************/
#define GM_ORIENTATION_REG_NUM          5           /* No. of orientation region */
#define OLD_MIC                         0           /* Old nic value index in array GMMic[ MIC_RECORD + 1 ] */
#define NEW_MIC                         1           /* New nic value index in array GMMic[ MIC_RECORD + 1 ] */
#define MIC_ADD_SCAL                    2           /* Mic add scalar every time */                    
#define MIC_MORE_STEPS                  10
//#define MIC_LESS_STEPS                  3
#define MIC_UP_GAP                      10

/************************************************* Global variable declarations *******************************/
UINT8       GMBatteryCap;

UINT8       GMVolume;
UINT8       GMEQBank;

UINT16      GMOrientation;
UINT8       GMPattern;
volatile BOOL GMPatternReload;
UINT8       GMBTSpkShowID;

PATTERN_INFO_T GMPatternInfoRx;                     /* For pattern broadcast information received */
PATTERN_INFO_T GMPatternInfoTx;                     /* For pattern information broadcast out */

UINT8       GMBrightness;                           /* Maximum 255 */
THEME_COLOR_MODE   GMThemeColorMode;
COLOR_T            GMThemeColor;

GEN_PARA_T  GMExInput[ psexiNum ];                  /* External input information */
UINT8       GMPosInfo[ GM_POS_MAX ];                /* Position information obtained from App */
                                                    /* Validity depends on GMExInput[ psexiApp ] */
COLOR_T     GMColorPick;                            /* Color obtained from color sensor */
                                                    /* Validity depends on GMExInput[ psexiColor ] */
UINT8       GMPatternStatus[ GM_PATTERN_NUM ];      /* Pattern status, for communication with app, also the initial user setup */
UINT8       GMMic[ MIC_RECORD + 1 ];                /* Record 2 mic value by the 2 mem ahead, and the 3th record current value return to GMExInput[ mic ] */
BOOL        GMMicUpdateFlag;                              /* Mic were rewriten flag */
BOOL        MicAddFlag;                             /* Every time mic added scalar */

// To be used in EQ Mapping
INT16 g_lowAvgEQ;
INT16 g_midAvgEQ;
INT16 g_highAvgEQ;

//To be used for absolute Volume in DSP
UINT8       GMVolumeAbs;
UINT8       GMVolumeDSP;
BOOL        g_toneDefaultVolFlag  = FALSE;
BOOL        g_hfpDefaultVolFlag  = FALSE;

PATTERN_E pattern_flag = PatFirework;

/************************************************* Local variable definitions *********************************/
LOCAL CONST UINT16 oriLowLimit[] = { 0, 46, 136, 226, 316 };
LOCAL CONST UINT16 oriUpLimit[]  = { 45, 135, 225, 315, 360 };
LOCAL CONST UINT8 oriRegion[]    = { Ori0, Ori90, Ori180, Ori270, Ori0 };
LOCAL CONST UINT8 btmPatMap[10]  = { PatIdle, PatWave, PatJet, PatExplosion, PatBar, PatRave, PatRainbow, PatFire, PatCustomized, 0xff };

/************************************************************************
 Function:    GlobalMemoryInit
 Description:    Global memory module initialization
 Input:        Nil
 Return:    VOID
 Remarks:    System initalization
************************************************************************/
VOID GlobalMemoryInit(VOID)
{
    UINT16 i;

    /* Step 1 -- Initialize variables by default values */
    GMBTSpkShowID = BTSpkPatIdle;
//    GMPattern     = PatIdle;

    GMPatternInfoRx.id = pattern_flag;
    GMPatternInfoTx.id = pattern_flag;

    GMOrientation = 0;
    GMMicUpdateFlag    = FALSE;
    MicAddFlag         = FALSE;

    GMThemeColorMode = DEFAULT_COLOR;
    GMThemeColor.r   = 0;
    GMThemeColor.g   = 0;
    GMThemeColor.b   = 0;

    for( i = 0; i < psexiNum; i++ )
        GMExInput[ i ].scalar = GM_EX_INPUT_INVALID;

    for( i = 0; i < GM_POS_MAX; i++ )
        GMPosInfo[ i ] = GM_POS_INVALID;

    for( i = 0; i < GM_PATTERN_NUM; i++ )                   /* Default pattern status */
        GMPatternStatus[ i ] = GM_PATTERN_STATUS_NULL;

    for( i = 0; i < MIC_RECORD + 1; i++ )                   /* Default mic value 0 */
        GMMic[ i ] = 0;

    GMPatternReload = FALSE;
    /* Step 2 -- Read from flash memory */
    #if 0
    GMPattern     = Gnvmpar1.GMPattern | GM_PATTERN_IDLE;
    GMBatteryCap  = Gnvmpar1.GMBatteryCap;
    GMVolume      = Gnvmpar1.GMVolume;
    GMEQBank      = Gnvmpar1.GMEQBank;
    #endif
    GMBrightness  = DEFAULT_BRIGHTNESS;

    /* Step 3 -- Read from Bluetooth module */
    /* Delay execute... */
}

/************************************************************************
 Function:    GMGetOrientation
 Description:    Get the orientation value of the product
 Input:        VOID
 Return:    Orientation
 Remarks:    Called from Renderer
************************************************************************/
ORIENTATION_E GMGetOrientation(VOID)
{
    UINT16 i;
    INT16 ori;

    ori = GMOrientation;
    for( i = 0; i < GM_ORIENTATION_REG_NUM; i++ )
    {
        if( ( ori >= oriLowLimit[ i ] ) && ( ori <= oriUpLimit[ i ] ) )
        {
            /* Fall in the region */
            return oriRegion[ i ];
        }
    }

    return oriRegion[ 0 ];
}

/************************************************************************
 Function:    ReadMic
 Description:    Get the mic value of the product
 Input:        VOID
 Return:    mic scalar
 Remarks:    Called from SPKProcess
************************************************************************/
VOID ReadMic( VOID )
{
    INT16 gap; 
    /* New mic value */
    if( GMMicUpdateFlag )
    {
        GMMicUpdateFlag = FALSE;
        /* Record it and renew other parameters */
        GMMic[ OLD_MIC ] = GMMic[ NEW_MIC ];
        GMMic[ NEW_MIC ] = GMMic[ MIC_RECORD ];

        gap = GMMic[ NEW_MIC ] - GMExInput[ psexiMicLevel ].scalar;
        /* New mic is a bigger one, get the new up speed */
        if( gap > 0 )
        {        
            if( gap > MIC_UP_GAP )
                GMMic[ MIC_ADD_SCAL ] = gap / MIC_MORE_STEPS;
            else
                GMMic[ MIC_ADD_SCAL ] = 1;
            MicAddFlag = TRUE;
        }
        /* New mic is a smaller one, return it */
        else
        {
            GMExInput[ psexiMicLevel ].scalar = 0;
            MicAddFlag = FALSE;
            return;
        }
    }
    /* No new mic value */   
    if( GMExInput[ psexiMicLevel ].scalar >= GMMic[ NEW_MIC ] )
    {
        GMExInput[ psexiMicLevel ].scalar -= GMMic[ MIC_ADD_SCAL ];
        MicAddFlag = FALSE;
    }
    else
    {
        if( MicAddFlag )
            GMExInput[ psexiMicLevel ].scalar += GMMic[ MIC_ADD_SCAL ];

        if( !(MicAddFlag) )
        {
            gap = GMMic[ MIC_ADD_SCAL ] << 1;
            if( GMExInput[ psexiMicLevel ].scalar > gap )
                GMExInput[ psexiMicLevel ].scalar -= gap;
            else
                GMExInput[ psexiMicLevel ].scalar = 0;
        }
    }
}
/*
*   ret:0xff error
*/
UINT8 GMBTMPatternMap(UINT8 btmPat)
{
    if(btmPat > 9)
        return 0xff;
    else
        return btmPatMap[btmPat];
}

