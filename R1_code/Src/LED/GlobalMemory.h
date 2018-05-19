/****************************************************************************************************************
* Copyright (C)  TaiSing Industrial Company Ltd.(Shenzhen).
* Project Name:  Pulse II
* File name:     GlobalMemory.h
* Description:   GlobalMemory module header

* Author: LM Cheong                             Date: 2015-March-07
*****************************************************************************************************************/

/**************************** Edit History *********************************************************************************
* DATE               NAME                 DESCRIPTION
* 2015-Mar-07        LM Cheong           Created
* 2015-Mar-22       LM Cheong           Modified
* 2015-Mar-30       Joyce shen          Delete GPO_EVENT_PARA_E, add BT_CLR_EVENT_PARA_E and AUX_EVENT_PARA_E
* 2015-Jul-25       James Lin           Make this revision for MP releae any other change must add new revision history
***************************************************************************************************************************/

#ifndef GLOBALMEMORY_H
#define GLOBALMEMORY_H

#ifdef    __cplusplus
extern "C" {
#endif

//#include <GenericTypeDefs.h>
//#include "sys_config.h"
#include "..\..\Inc\u_type_def.h"

/************************************************* Type declarations ******************************************/
#define GM_BATTERY_LEVEL                6

#define GM_VOL_MAX                      16
#define GM_VOL_MIN                      0
#define GM_VOL_DEFAULT                  10//9//8
#define GM_VOL_DEMO_DEFAULT             12

#define GM_PATTERN_NUM                  10              /* Maximum no. of defined pattern */
#define GM_RUN_PATTERN_NUM              9               /* Number of active pattern */
#define GM_BTSPK_PATTERN_NUM            11              /* Maximum no. of defined pattern for BTSpk */
#define GM_PATTERN_COLOR_NUM            9               /* Maximum no. of color for a pattern */
#define GM_PATTERN_STATUS_NULL          0xFF            /* Indicate no specific setting, show as default */

#define GM_PATTERN_IDLE                 0x80            /* Set MSB of GMPattern to make it not show */
#define GM_PATTERN_IDLE_MASK            0x80

#define GM_BRIGHTNESS_MAX               255

#define GM_EQBANK_NUM                   8               /* Total no. of EQ bank */
#define GM_EQBANK_BYPASS                0               /* EQ bank no. for bypass mode */

#define GM_POS_MAX                      96              /* Align with the physical no. of pattern LEDs */
#define GM_FREQ_MAX                     100             /* Maximum value of frequency component can be read */

#define GM_EX_INPUT_INVALID             0x7FFF
#define GM_EX_INPUT_ENABLE              0x1FF           /* Enable all external inputs */
#define GM_POS_INVALID                  0xFF
#define MIC_RECORD                      3               /* No. of microphone value record */
#define MAX_BRIGHTNESS                  255
#define DEFAULT_BRIGHTNESS              255

/* Pattern ID enumeration */
/* Patterns under simulator:   0x00 to 0x1F */
/* Patterns for open API:      0x20 to 0x3F */
/* Patterns for video refresh: 0x40 to 0x5F */
typedef enum GM_PATTERN
{
    PatFirework  = 0,
    PatTraffic   = 1,//jet
    PatExplosion = 2,//explosion
    PatWave      = 3,
    PatFirefly   = 4,
    PatCustomized = 5,//customized
    PatFire      = 6,
    PatRainbow   = 7,
    PatHourglass = 8,
    PatBar       = 9,
    PatRave      = 10, 
    PatJet       = 11,

    PatSimMax    = 0x1F,

//    PatOpenAPI   = 0x20,
    PatPicture   = 0x20,
    PatOpenAPIMax= 0x3F,

    PatRefresh   = 0x40,
    PatRefreshMax= 0x5F,

    PatIdle = 0x80
}PATTERN_E;

/*this define is for merge APP pattern id define the APP id 9 is Ripple Device is EQ bar need convert*/
#define APP_EQ_BAR 10
#define APP_WAVE    11
/* BTSpk pattern ID enumeration */
typedef enum GM_BTSPK_PATTERN
{
    BTSpkPatPowerOn = 0x00,
    BTSpkPatPowerOff,
    BTSpkPatBTPair,
    BTSpkPatBTConnected,
    BTSpkPatVolLevel,
    BTSpkPatPickColor,
    BTSpkPatUpgrading,
    BTSpkPatUpgradeSuccess,
    BTSpkPatUpgradeFailed,
    BTSpkPatPhone,
    BTSpkPatCharging,

    BTSpkPatIdle = 0x7F
}BTSPK_PATTERN_E;

/* Tone event para enumeration */
typedef enum GM_TONE_EVENT_PARA
{
    CONNECTED_TONE=0,
    PAIRING_TONE,
    POWER_OFF_TONE,
    POWER_ON_TONE,
    VOL_MAX_TONE,
    JBL_CONNECT_PLUS_TONE
}TONE_EVENT_PARA_E;

//#define UPGRADE_SUCCESS_TONE    IDENTIFY_TONE
//#define UPGRADE_FAILED_TONE     IDENTIFY_TONE
//#define COLOR_CAP_TONE        IDENTIFY_TONE
//#define SHAKE_TONE              IDENTIFY_TONE
#define GMPATTERN_TONE          IDENTIFY_TONE

/* Battery level */
typedef enum BATTERY_LEVEL
{
    CAPACITY_RANGE_BELOW_5_PERCENT,   //for power on not turn on RED LED
    CAPACITY_RANGE_5_TO_15_PERCENT,   //LEDbatRed, <15%, LEDbatRed breath
    CAPACITY_RANGE_15_TO_30_PERCENT,  //LEDbat1    <30%, LEDbat1 on
    CAPACITY_RANGE_30_TO_45_PERCENT,  //LEDbat2    <45%, LEDbat1,LEDbat2 on
    CAPACITY_RANGE_45_TO_60_PERCENT,  //LEDbat3    <60%, LEDbat1,LEDbat2,LEDbat3 on
    CAPACITY_RANGE_60_TO_75_PERCENT,  //LEDbat4    <70%, LEDbat1,LEDbat2,LEDbat3,LEDbat4 on
    CAPACITY_OVER_75_PERCENT,         //LEDbat5    >75%, LEDbat1,LEDbat2,LEDbat3,LEDbat4,LEDbat5 on
    CAPACITY_FULL
}BATTERY_LEVEL_E;

/* Particle system external input type */
typedef enum PS_EX_INPUT_TYPE
{
    psexiDC       = 0,          /* Audio signal average level */
    psexi300Hz    = 1,          /* Audio signal bass level */
    psexi1kHz     = 2,          /* Audio signal Alto level */
    psexi10kHz    = 3,          /* Audio signal high pitch level */
    psexi63Hz     = 4,
    psexi100Hz    = 5,
    psexi160Hz    = 6,
    psexi250Hz    = 7,
    psexi400Hz    = 8,
    psexi630Hz    = 9,
    psexi1000Hz   = 10,
    psexi2000Hz   = 11,
    psexi4000Hz   = 12,
    psexi6300Hz   = 13,
    psexi10000Hz  = 14,
    psexi16000Hz  = 15,            
    psexiMicLevel = 16,          /* Microphone level */
    psexiShake    = 17,          /* G-sensor shake detect */
    psexiColor    = 18,          /* Color pick detect */
    psexiMusic    = 19,          /* Music play length */
    psexiApp      = 20,          /* App input */

    psexiNum
}PS_EX_INPUT_TYPE_E;

/* BT CLR event para enumeration */
typedef enum GM_BT_CLR_EVENT_PARA
{
    NEXT_PAIR = 0,
    NO_PAIR
}BT_CLR_EVENT_PARA_E;

/* AUX event para enumeration */
typedef enum GM_AUX_EVENT_PARA
{
    AUX_INACTIVE = 0,
    AUX_ACTIVE
}AUX_EVENT_PARA_E;

/* Orientation enumeration */
typedef enum ORIENTATION
{
    Ori0   = 0,           //0 degree, put vertical
    Ori90  = 1,          //90 degree, put horizontal
    Ori180 = 2,         //180 degree, put upside down
    Ori270 = 3,         //270 degree, put horizontal upside down

    OriNum = 4
}ORIENTATION_E;

/* Position structure */
typedef struct POSITION
{
    INT16 x;
    INT16 y;
    INT16 z;
}POSITION_T;

/* Color structure */
typedef struct COLOR
{
    UINT8 r;
    UINT8 g;
    UINT8 b;
}COLOR_T;

/* Vector structure */
typedef struct PVECTOR
{
    INT8  x;
    INT8  y;
}PVECTOR_ST;

/* General parameter structure */
typedef union GEN_PARA
{
    INT16      scalar;
    PVECTOR_ST vector;
}GEN_PARA_T;

/* Pattern information, for broatcast pattern info */
typedef struct PATTERN_INFO
{
    UINT8 id;                   /* Pattern id */
    UINT8 status;               /* Pattern status */
                                /* Pattern color info, (0,0,0)=default color */
    COLOR_T pColor[ GM_PATTERN_COLOR_NUM ];
}PATTERN_INFO_T;

typedef enum _THEME_COLOR_MODE
{
    DEFAULT_COLOR = 0,
    APP_COLOR
}THEME_COLOR_MODE;
/************************************************* Global variable declarations *******************************/
GLOBAL UINT8      GMBatteryCap;

GLOBAL UINT8      GMVolume;
GLOBAL UINT8      GMEQBank;

GLOBAL UINT16     GMOrientation;
GLOBAL UINT8      GMPattern;
GLOBAL BOOL volatile GMPatternReload;
GLOBAL UINT8      GMBTSpkShowID;

GLOBAL PATTERN_INFO_T GMPatternInfoRx;              /* For pattern broadcast information received */
GLOBAL PATTERN_INFO_T GMPatternInfoTx;              /* For pattern information broadcast out */

GLOBAL UINT8      GMBrightness;                     /* Maximum 255 */
GLOBAL THEME_COLOR_MODE   GMThemeColorMode;
GLOBAL COLOR_T            GMThemeColor;

/* External input triggers */
GLOBAL GEN_PARA_T GMExInput[ psexiNum ];            /* External input information */
GLOBAL UINT8      GMPosInfo[ GM_POS_MAX ];          /* Position information obtained from App */
                                                    /* Validity depends on GMExInput[ psexiApp ] */
GLOBAL COLOR_T    GMColorPick;                      /* Color obtained from color sensor */
                                                    /* Validity depends on GMExInput[ psexiColor ] */
GLOBAL UINT8      GMPatternStatus[ GM_PATTERN_NUM ];/* Pattern status, for communication with app, also the initial user setup */
                                                  
GLOBAL UINT8      GMMic[ MIC_RECORD + 1 ];          /* Record 2 mic value by the 2 mem ahead, and the 3th record current value return to GMExInput[ mic ] */
GLOBAL BOOL       GMMicUpdateFlag;                        /* Mic were rewriten flag */
GLOBAL BOOL       MicAddFlag;                           /* Every time mic added scalar */

GLOBAL BOOL       g_toneDefaultVolFlag; 
GLOBAL BOOL       g_hfpDefaultVolFlag; 

// To be used in EQ Mapping
GLOBAL INT16 g_lowAvgEQ;
GLOBAL INT16 g_midAvgEQ;
GLOBAL INT16 g_highAvgEQ;

//To be used for absolute Volume in DSP
GLOBAL UINT8 GMVolumeAbs;
GLOBAL UINT8 GMVolumeDSP;

/************************************************* Functional prototypes **************************************/
/************************************************************************
 Function:    GlobalMemoryInit
 Description:    Global memory module initialization
 Input:        Nil
 Return:    VOID
 Remarks:    System initalization
************************************************************************/
GLOBAL VOID GlobalMemoryInit(VOID);

/************************************************************************
 Function:    GMGetOrientation
 Description:    Get the orientation value of the product
 Input:        VOID
 Return:    Orientation
 Remarks:    Called from Renderer
************************************************************************/
GLOBAL ORIENTATION_E GMGetOrientation(VOID);

GLOBAL UINT8 GMBTMPatternMap(UINT8 btmPat);

#ifdef    __cplusplus
}
#endif
#endif /* GLOBALMEMORY_H */
