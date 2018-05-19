/***************************************************************************************************************************
* Copyright (C)  TaiSing Industrial Company Ltd.(Shenzhen).
* Project Name:  Pulse II
* File name:     Rendering.c
* Description:   Rendering handler

* Author: YunSong Wei                                   Date: 2015-March-05
***************************************************************************************************************************/
/**************************** Edit History *********************************************************************************
* DATE               NAME                      DESCRIPTION
* 2015-Mar-05       YunSong Wei               Created
* 2015-Mar-22       LMCheong                  Modified
* 2015-Jul-25       James Lin                 Make this revision for MP releae any other change must add new revision history
* 2015-Jul-27       LM Cheong                 Fix bug in stImage handling in mapColor()
* 2015-Aug-21       JamesLin                  Fix bug when firework shake handle not active
 *2016-Nov-10       Sumit Gambhir             Pulse-3 System light theme changes  
 *2016-Nov-18       Rajiv B                   Pulse-3 Rendering EQ Bar Light theme 
***************************************************************************************************************************/

#include "..\..\Inc\sys_common.h"

#include <string.h>

#define MAXHUE                360
#define ROWOFFSET             2

LOCAL BOOL     g_bPickColor   = FALSE;


/** For Eq bar theme */
//#define BAR_NORMAL_STYLE_1
//#define BAR_DEBUG
#define BAR_HUEDEGREE          60
#define BAR_HUESWTDEGREE       2
#define BAR_SWTINTERVL         10
#define BAR_MOVEINTERVL        2
#define BAR_EQMODEOFFSET       0
#define BAR_EQDOWNSPD          3    // too big, too slow
// for improving
typedef struct BARSINDESC
{
    UINT8   baseCount;
    UINT8   coverCount;
    UINT8   coverLightness[2];
}BARSINDESC_T;

LOCAL   COLOR_T   g_bar_C          = { 255, 0, 0 };
LOCAL   UINT8     g_bar_DimStat    = 0;
LOCAL   UINT8     g_bar_Pos        = 0;

#ifdef BAR_DEBUG 
LOCAL UINT8 s_EqBarWatcher = 0;
#endif

// For RainBow effection.
//#define RB_ENABLE_APP_COLOR_TEST
#define RB_RUNTIMES                      20
#define RB_COLOR_TIMES                   3
#define RB_HORI_DIR_VAR                  30
#define RB_VERT_DIR_VAR                  21
#define RB_HORI_VAR                      32
#define RB_VERT_VAR                      30
#define RB_HORI_INC                      1
#define RB_VERT_INC                      1
#define RB_PICK_HORI_DIR_VAR             80
#define RB_PICK_VERT_DIR_VAR             56
#define RB_PICK_HORI_INC                 1
#define RB_PICK_VERT_INC                 1
#define RB_PICKC_H_VAR                   8
#define RB_PICKC_V_VAR                   5

#define RB_EQ_HORI_SPEED                 4
#define RB_EQ_VERT_SPEED                 4
#define RB_EQ_HORI_MAX_PULLBACK          3
#define RB_EQ_VERT_MAX_PULLBACK          3
#define RB_EQ_HORI_PULLBACK_LOW         -10
#define RB_EQ_VERT_PULLBACK_LOW         -10
#define RB_EQ_DIR_SWT_VAR                200
#define RB_EQ_FREQ_WIDTH                 3


/** Rain Bow direction */
typedef enum RB_DIR
{
    RB_BOTT_UP,
    RB_RIGHT_LEFT,
    RB_UP_BOTT,
    RB_LEFT_RIGHT,
} RB_DIR_E;

typedef struct RAINBOWHUE
{
    INT16   currHue;
    INT16   targetHue;
} RAINBOWHUE_T;

typedef enum RB_COLOR_DIR
{
   RB_COLOR_KEEP     = 0,
   RB_COLOR_INC      = 1,
   RB_COLOR_DEC      = 2,
} RB_COLOR_DIR_E;

LOCAL RB_DIR_E g_rb_Dir                 = RB_BOTT_UP;
LOCAL UINT8    g_rb_RunTimes            = 0;
LOCAL UINT8    g_rb_SwtTimes            = 0;
LOCAL HSL_T    g_rb_BaseHSL             = { 255, 0, 0 };
LOCAL INT16    g_rb_PickColorHue        = 0;
LOCAL INT8     g_rb_CurrPickIndex       = 0;
LOCAL UINT8    g_rb_PickColorDir        = 0;
LOCAL INT16    g_rb_CvtHoriPickColorHue = 0;
LOCAL INT16    g_rb_CvtVertPickColorHue = 0;
LOCAL RAINBOWHUE_T g_rb_HoriHues[REN_COLOR_OPERATION_NUM];
LOCAL RAINBOWHUE_T g_rb_VertHues[REN_VIR_DIS_FRAME];

/************************************************* Function declarations ******************************************/
LOCAL VOID copyBackground(UINT16 dInd, UINT16 pInd);
LOCAL VOID mapColor(UINT16 dInd, UINT16 psInd);

LOCAL VOID prepareConstructDisplay(UINT16 id);
LOCAL VOID constructDisplay(VOID);
LOCAL VOID refreshFrame(VOID);

LOCAL VOID mapPhyDisplay(VOID);
LOCAL VOID clearVirBuf(UINT16 dInd);
LOCAL VOID clearPhyBuf(UINT16 dInd);

LOCAL VOID updateColor(COLOR_T color, UINT16 cOpInd, PARTICLE_ST *pPtr, UINT16 psInd, COLOR_T *cPtr);
LOCAL VOID mapLocationColor(PVECTOR_ST pos, COLOR_T color, UINT16 dInd, UINT16 psInd, PVECTOR_ST vel);
LOCAL VOID mapNormalJetThemeColor(PVECTOR_ST pos, COLOR_T color, UINT16 dInd, UINT16 psInd, PVECTOR_ST vel);
LOCAL VOID mapVirLocationColor(PVECTOR_ST pos, COLOR_T color, UINT16 dInd);

LOCAL BOOL runVol(VOID);
LOCAL BOOL runFlash(VOID);
LOCAL BOOL runRefresh(VOID);
LOCAL BOOL runBreath(VOID);
LOCAL BOOL runColorPick(VOID);
#ifndef OPEN_COLOR_SENSOR_ADJ_ENABLE
LOCAL VOID adjustPickColor(VOID);
#endif /* OPEN_COLOR_SENSOR_ADJ_ENABLE */
LOCAL VOID initConDisBuf(COLOR_T seedColor, CON_DIS_CTYPE_E colorType, UINT8 colorStep);
LOCAL BOOL runPowerOnOff(VOID);
LOCAL BOOL runBluetoothConnect(VOID);
LOCAL BOOL runBluetoothComplete(VOID);
LOCAL BOOL runCharge(VOID);
LOCAL BOOL runFadeOut(VOID);


LOCAL VOID rgb2hsl(COLOR_T rgb, HSL_T *hPtr);
LOCAL VOID hsl2rgb(HSL_T hsl, COLOR_T *cPtr);
LOCAL VOID addColor(COLOR_T c1, COLOR_T c2, COLOR_T *cPtr);

LOCAL INT16 colorOpFormula(INT16 operand0, INT16 operand1, INT16 operand2, UINT16 oc0, UINT16 oc1, UINT8 ocOption);
LOCAL INT16 arithmetic(INT16 operand0, INT16 operand1, UINT16 oc);

VOID DrawExplosion(PARTICLE_ST *pPtr, COLOR_T cColor, UINT16 dInd, UINT16 psInd);
LOCAL VOID AlphaColor(COLOR_T upperColor, COLOR_T lowerColor, UINT8 uAlphaRate, COLOR_T *pRltColor);

/************************************************* Type declarations ******************************************/
/* Dimension definitions */
#define REN_FRAME                   48       /* Frame width of the rendering system */
#define REN_TEXTURE_FRAME           8        /* Maximum frame width/height of a texture */

/* Number definitions */
#define REN_VIR_DIS_NUM             2        /* No. of virtual display */
#define REN_TEXTURE_NUM             5        /* No. of texture */
#define REN_BACKGROUND_NUM          2        /* No. of background */
#define REN_SHAPE_NUM               4        /* No. of shape */

/* Other operation constants */
#define    MAX_RGB                     0xFF     /* Value of maximum color intensity */


#define WEIGHTMATRIX_W              4
#define SIXTEEN_TO_FOUR_MATRIX      4
#define WEIGHT_FACTOR_RATIO         3
#define VOL_TO_DIS                  2        /* Set volume and render on leds */
#define MAX_DEGREE                  360      /* In searching link module ,set the max degree is 360 degree */

#define HUE_OR_COLOR_SNAP           4

/* Orienation */
#define ORI0_MASK                   0x01
#define ORI90_MASK                  0x02
#define ORI_OPTION                  4

/* Specific definitions for heart shape firework tailor made display */
/* Assume heart shape firework can be exploded to limited size */
#define SIZE1                       3       /* sr = 1, vx, vy in the range +/-1 */
#define SIZE2                       5       /* sr = 2, vx, vy in the range +/-2 */
#define SIZE3                       7       /* sr = 3, vx, vy in the range +/-3 */
#define SIZE4                       9       /* sr = 4, vx, vy in the range +/-4 */
#define FLL_0                       16      /* Lightness = ( color lightness * FLL_X ) / 16 */
#define FLL_1                       11
#define FLL_2                       8
#define FLL_3                       0

#define FIREWORK_EXPLODE_NUM        4       /* No. of points added during explosion */
#define WAVE_HUE_CHANGE             25      /* Hue change scalar for adjacent layers when capturing color */

/* Construct display section */
#define REN_CON_DIS_IDLE            0xFF
#define REN_CON_DIS_SEQ_INFINITE    0xFF
#define REN_CON_DIS_FULL            0xFF

#define NO_DB_VALUE   //Rave mode
//#define LOW_MID_HIGH_TRANSITION_PROCESS //Rave mode

#define  PULSE3_EQ_THRESHOLD -54

#define COUNT_FIRST_EDGE_FLAG       0x01
#define HALF_CYCLE_EDGE_COUNT       REN_VIR_DIS_FRAME
#define MAX_CYCLE_EDGE_COUNT        (HALF_CYCLE_EDGE_COUNT*2)


/* Construct display operations */
typedef enum CON_DIS_OPERATION
{
    conDisVol          = 0,                  /* volume display */
    conDisFlash        = 1,                  /* Display flashing */
    conDisRefresh      = 2,                  /* Refresh a display with defined display maps */
    conDisBreath       = 3,                  /* Breath color display */
    conDisColorPick    = 4,
    conDisPowerOnOff   = 5,
    conDisBTConnected  = 6,
    conDisCharge       = 7,
    conDisBTComplete   = 8,
    conDisFadeOut      = 9
}CON_DIS_OPERATION_E;

/* Operation data structures */
/* Volume  operation  */
typedef struct CON_DIS_VOL
{
    UINT8   uRate;
    UINT8   foStep;                         /* Step to start fade out */
    UINT8   showVol;
    UINT8   lStep;                          /* Step of lightness change */
    COLOR_T showColor[ 2 ];                 /* showColor[ 0 ] is full lightness display , showColor[ 1 ] thirty percent lightness display */
}CON_DIS_VOL_T;

typedef struct CON_DIS_VSET
{
    UINT8 vHeight;                          /* Display height of the volume setting */
    UINT8 cInd;                             /* Color index */
}CON_DIS_VSET_T;

/* Flash operation */
typedef struct CON_DIS_FLASH
{
    UINT8   onTime;
    UINT8   offTime;
    UINT8   count;
    COLOR_T color;
}CON_DIS_FLASH_T;

#define CON_DIS_FLASH_ON        0
#define CON_DIS_FLASH_OFF       1

/* Refresh operation */
typedef struct CON_DIS_REFRESH
{
    UINT8 startInd;
    UINT8 endInd;
    UINT8 rRate;                             /* Refresh rate, no. of time tick per update */
    UINT8 rowStartInd;
    UINT8 rowEndInd;
    PHYDIS_T *rfPtr;
}CON_DIS_REFRESH_T;

/* Breath operation */
typedef struct CON_DIS_BREATH
{
    CON_DIS_CTYPE_E colorType;
    UINT8   bgStepNum;
    UINT8   bRate;                           /* Breath rate, no. of time tick per update  */
    INT8    lStep;                           /* Change of L' value per update */
    INT16   stepCnt;
    INT16   maxStepNum;                      /* Maximum lightness */
    COLOR_T hColor;
}CON_DIS_BREATH_T;

#define CON_DIS_BREATH_UP       0
#define CON_DIS_BREATH_DOWN     1

/* Color pick operation */
/* A color display region is defined as the area bounded by 2 equations: */
/* 1. y >= x + lowOffset; and */
/* 2. y <  x + upOffset */
/* x belongs to the range from 0 to (REN_PHY_FRAME_W-1) */
/* y belongs to the range from 0 to (REN_PHY_FRAME_H-1) */
/* Display color = picked color in HSL with hue -> (hue-hueOffset), lightness -> (lightness-lightnessOffset) */
typedef struct CON_DIS_CP_REGION
{
    INT8  lowOffset;
    INT8  upOffset;
    INT16 hueOffset;
    INT16 lightnessOffset;
}CON_DIS_CP_REGION_T;

typedef struct CON_DIS_COLOR_PICK
{
    UINT8   uBRate;
    UINT8   brightnessStep;
    UINT8   processCnt;
    COLOR_T color;
}CON_DIS_COLOR_PICK_T;

typedef struct CON_DIS_POWER_ONOFF
{
    INT8    next;
    UINT8   step;
    UINT8   pRate;
    UINT8   endStep;
    COLOR_T color;
}CON_DIS_POWER_ONOFF_T;

typedef struct CON_DIS_BT_CONNECTED
{
    UINT8   uBRate;
    UINT8   brightnessStep;
    UINT8   processCnt;
    UINT8   originalColor;
}CON_DIS_BT_CONNECTED_T;

typedef struct CON_DIS_BT_COMPLETE
{
    UINT8   uBRate;
    UINT8   brightnessStep;
    UINT8   processCnt;
    COLOR_T color;
}CON_DIS_BT_COMPLETE_T;


typedef struct CON_DIS_CHARGING
{
    UINT8   cRate;
    UINT8   phase1;
    UINT8   phase2;
    UINT8   endRow;
    UINT8   decreaseStep;
    COLOR_T disColor[ 2 ];
}CON_DIS_CHARGING_T;

typedef struct CON_DIS_FADE_OUT
{
    CON_DIS_CTYPE_E colorType;
    UINT8   bRate;                           /* Breath rate, no. of time tick per update  */
    INT8    lStep;                           /* Change of L' value per update */
    UINT8   stepCnt;
    COLOR_T hColor;
}CON_DIS_FADE_OUT_T;


/* Run time memory union */
typedef union CON_DIS_RUNTIME
{
    CON_DIS_VOL_T           volume;
    CON_DIS_FLASH_T         flash;
    CON_DIS_REFRESH_T       refresh;
    CON_DIS_BREATH_T        breath;
    CON_DIS_COLOR_PICK_T    colorPick;
    CON_DIS_POWER_ONOFF_T   powerOnOff;
    CON_DIS_BT_CONNECTED_T  bTConnected;
    CON_DIS_CHARGING_T      charge;
    CON_DIS_BT_COMPLETE_T   bTComplete;
    CON_DIS_FADE_OUT_T      fadeOut;
}CON_DIS_RUNTIME_T;

/* Operation sequence */
typedef struct CON_DIS_OP_SEQ
{
    UINT8 op;                                /* From CON_DIS_OPERATION */
    UINT8 offset;
}CON_DIS_OP_SEQ_T;

/* Construct display info */
typedef struct CON_DIS_INFO
{
    UINT8 startSeq;
    UINT8 endSeq;
    UINT8 repeatCnt;
}CON_DIS_INFO_T;

typedef enum JET_MODE_PS_TYPE
{
    jet_a  = 0x01,                                       
    jet_b  = 0x02,                                       
    jet_c  = 0x03,                                        
    jet_d  = 0x04,
    jet_e  = 0x05,  
    jet_f  = 0x06  
    
}JET_MODE_PS_TYPE_E;

typedef struct JET_MODE_PS_INFO
{
    PVECTOR_ST bJET_PS_1;
    PVECTOR_ST bJET_PS_2;
    PVECTOR_ST bJET_PS_3;
    PVECTOR_ST bJET_PS_4;

    UINT8      bJET_Movestate;
    UINT8      bJET_randomflag;
    UINT8      ShowCount;
    
}JET_MODE_PS_INFO_T;



typedef struct _CycleEdgeInfo
{
    INT16  m_loc_x;
    INT16  m_loc_y;
    UINT8  m_count;
    UINT8  m_flag;
}CycleEdgeInfo;
/************************************************* Global variable declarations *******************************/
PHYDIS_T              PhyDis[ REN_PHY_DIS_NUM ];                    /* Physical display buffer */
UINT8                 PhyDisActiveInd;
UINT8                 PhyDisLoadInd;
UINT8                 IniColor_f=FALSE;
UINT8                 BarApp;
/************************************************* Local variable definitions *********************************/
/* Weighted matrix for calculating the color, all of data left shilft four bits */
LOCAL CONST UINT8     weightMatrix[ WEIGHTMATRIX_W ][ WEIGHTMATRIX_W ] = { {1,3,3,1}, {3,8,8,3}, {3,8,8,3}, {1,3,3,1} };
/* save a copy of DbLevelIndexArr */
//LOCAL INT16    g_DbValAry[12] = { 0 };

/* power on message */
LOCAL COLOR_T     conDisBuf[ REN_PHY_FRAME_W ][ REN_PHY_FRAME_H ] = { 0 };
LOCAL CONST INT8  powerOnOffMatrix[ REN_PHY_FRAME_W ][ REN_PHY_FRAME_H ] = {
0x1,    0xD,    0x19,    0x25,    0x31,    0x3D,    0x49,    0x55,
0x2,    0xE,    0x1A,    0x26,    0x32,    0x3E,    0x4A,    0x56,
0x3,    0xF,    0x1B,    0x27,    0x33,    0x3F,    0x4B,    0x57,
0x4,    0x10,    0x1C,    0x28,    0x34,    0x40,    0x4C,    0x58,
0x5,    0x11,    0x1D,    0x29,    0x35,    0x41,    0x4D,    0x59,
0x6,    0x12,    0x1E,    0x2A,    0x36,    0x42,    0x4E,    0x5A,
0x7,    0x13,    0x1F,    0x2B,    0x37,    0x43,    0x4F,    0x5B,
0x8,    0x14,    0x20,    0x2C,    0x38,    0x44,    0x50,    0x5C,
0x9,    0x15,    0x21,    0x2D,    0x39,    0x45,    0x51,    0x5D,
0xA,    0x16,    0x22,    0x2E,    0x3A,    0x46,    0x52,    0x5E,
0xB,    0x17,    0x23,    0x2F,    0x3B,    0x47,    0x53,    0x5F,
0xC,    0x18,    0x24,    0x30,    0x3C,    0x48,    0x54,    0x60,

                                    };

/* Orientation */
LOCAL CONST UINT8 oriMatrix[ ORI_OPTION ][ OriNum ] = { { Ori0, Ori90, Ori180, Ori270 },
                                                        { Ori0, Ori90, Ori0,   Ori270 },
                                                        { Ori0, Ori0,  Ori180, Ori0   },
                                                        { Ori0, Ori0,  Ori0,   Ori0   } };

/***************************************************************************************************************************/
/* Color maps */
COLOR_T         colorMap[ REN_COLOR_ARRAY_LEN ];              /* Color map for particle systems */

LOCAL PS_COLOR_OP_T   colorOp[ REN_COLOR_OPERATION_NUM ];           /* Color operations */

LOCAL UINT8           masterBrightness;                             /* Master pattern brightness */

/* Display buffers */
LOCAL VIRDIS_T        virDis[ REN_VIR_DIS_NUM ];                    /* Virtual display buffer */
LOCAL VIRDIS_T        background[ REN_BACKGROUND_NUM ];             /* Background image buffer */
LOCAL SHAPE_T         shape[ REN_SHAPE_NUM ];                       /* Shape image buffer */

LOCAL UINT8           fireworkExplodeList[ FIREWORK_EXPLODE_NUM ];  /* For firework pattern only */
LOCAL CycleEdgeInfo   CycleEdgeList[MAX_CYCLE_EDGE_COUNT];
/* Local running indexes */
LOCAL UINT8           virDisActiveInd;
LOCAL UINT8           virDisLoadInd;
LOCAL UINT8           bgCnt;                                        /* No. of defined background */
LOCAL UINT8           shapeCnt;                                     /* No. of defined shape */
LOCAL UINT8           orientationLock;                              /* Physical display orientation lock */
LOCAL UINT8           pOri, cOri;                                   /* Previous and current orientation */

/* Construct display section */
/* Operation variables */
LOCAL UINT8  conDisID;               /* Construct display show ID */
LOCAL UINT8  conDisSeq;              /* Construct display sequence */
LOCAL UINT16 conDisState;            /* Construct display current state, define according to operation */
LOCAL UINT8  conDisCnt;              /* Construct display sequence repeat count */
    
LOCAL JET_MODE_PS_INFO_T tJet_Mode_PS_A;
LOCAL JET_MODE_PS_INFO_T tJet_Mode_PS_B;
LOCAL JET_MODE_PS_INFO_T tJet_Mode_PS_C;
LOCAL JET_MODE_PS_INFO_T tJet_Mode_PS_D;
LOCAL JET_MODE_PS_INFO_T tJet_Mode_PS_E;
LOCAL JET_MODE_PS_INFO_T tJet_Mode_PS_F;

RAVE_MODE_INFO_T tRave_Mode_Info;
CUS_MODE_INFO_T  cus;
JET_MODE_PSM_INFO_T tJet_Mode_PSM_Info;

LOCAL CONST INT16 wRaveBreath[] =
{   
0x0000,0x0000,
0x0009,0x0009, 0x0013,0x0013, 0x001c,0x001c, 0x0026,0x0026, 0x0030,0x0030, 0x0039,0x0039, 0x0043,0x0043,
0x004c,0x004c, 0x0056,0x0056, 0x0060,0x0060, 0x0069,0x0069, 0x0073,0x0073, 0x007d,0x007d, 0x0086,0x0086,
0x0090,0x0090, 0x0099,0x0099, 0x00a3,0x00a3, 0x00ad,0x00ad, 0x00b6,0x00b6, 0x00c0,0x00c0, 0x00c9,0x00c9, 
0x00d3,0x00d3, 0x00dd,0x00dd, 0x00e6,0x00e6, 0x00f0,0x00f0, 0x00fa,0x00fa,
0x00f0,0x00f0, 0x00e6,0x00e6, 0x00dd,0x00dd, 0x00d3,0x00d3, 0x00c9,0x00c9, 0x00c0,0x00c0, 0x00b6,0x00b6,
0x00ad,0x00ad, 0x00a3,0x00a3, 0x0099,0x0099, 0x0090,0x0090, 0x0086,0x0086, 0x007d,0x007d, 0x0073,0x0073,
0x0069,0x0069, 0x0060,0x0060, 0x0056,0x0056, 0x004c,0x004c, 0x0043,0x0043, 0x0039,0x0039, 0x0030,0x0030, 
0x0026,0x0026, 0x001c,0x001c, 0x0013,0x0013, 0x0009,0x0009, 0x0000,0x0000
};

/* Current operation running structures */
LOCAL CON_DIS_RUNTIME_T curConDis;

/* Operation parameters */
/* Firework display specific operation data */
/* Firework data for sr = 1, 2, 3 sets */
LOCAL CONST UINT8 datHeart1[ SIZE1 ][ SIZE1 ] =
{
    { FLL_1, FLL_2, FLL_1 },
    { FLL_0, FLL_0, FLL_0 },
    { FLL_3, FLL_1, FLL_3 }
};

LOCAL CONST UINT8 datHeart2[ SIZE2 ][ SIZE2 ] =
{
    { FLL_3, FLL_3, FLL_3, FLL_3, FLL_3 },
    { FLL_3, FLL_1, FLL_3, FLL_1, FLL_3 },
    { FLL_0, FLL_2, FLL_3, FLL_2, FLL_0 },
    { FLL_3, FLL_0, FLL_2, FLL_0, FLL_3 },
    { FLL_3, FLL_3, FLL_0, FLL_3, FLL_3 }
};

LOCAL CONST UINT8 datHeart3[ SIZE3 ][ SIZE3 ] =
{
    { FLL_3, FLL_2, FLL_0, FLL_3, FLL_0, FLL_2, FLL_3 },
    { FLL_3, FLL_0, FLL_0, FLL_2, FLL_0, FLL_0, FLL_3 },
    { FLL_1, FLL_0, FLL_1, FLL_2, FLL_1, FLL_0, FLL_1 },
    { FLL_0, FLL_0, FLL_1, FLL_1, FLL_1, FLL_0, FLL_0 },
    { FLL_1, FLL_0, FLL_1, FLL_1, FLL_1, FLL_0, FLL_1 },
    { FLL_3, FLL_2, FLL_0, FLL_0, FLL_0, FLL_2, FLL_3 },
    { FLL_3, FLL_3, FLL_1, FLL_0, FLL_1, FLL_3, FLL_3 }
};

/* Index of operations in datOpSeq for each BTSpk pattern, indexed by BTSPK_PATTERN_E */
LOCAL CONST CON_DIS_INFO_T    datConDisInfo[] = { { 0, 3, 1 },                                              /* BTSpkPatPowerOn */
                                                  { 4, 4, 1 },                                              /* BTSpkPatPowerOff */
                                                  { 5, 5, REN_CON_DIS_SEQ_INFINITE },                       /* BTSpkPatBTPair */
                                                  { 6, 7, 1 },                                              /* BTSpkPatBTConnected */
                                                  { 8, 9, 1 },                                              /* BTSpkPatVolLevel */
                                                  { 10, 10, 1 },                                              /* BTSpkPatPickColor */
                                                  { 11, 11, REN_CON_DIS_SEQ_INFINITE },                     /* BTSpkPatUpgrading */
                                                  { 12, 12, 1 },                                            /* BTSpkPatUpgradeSuccess */
                                                  { 13, 13, 1 },                                            /* BTSpkPatUpgradeFailed */
                                                  { 14, 14, REN_CON_DIS_SEQ_INFINITE },                     /* BTSpkPatPhone */
                                                  { 15, 15, 1 }                                             /* BTSpkPatCharging */
                                                };

/* Operation sequence definitions for each BTSpk pattern */
LOCAL CONST CON_DIS_OP_SEQ_T  datOpSeq[] = { { conDisPowerOnOff, 0 }, { conDisBreath, 0 }, { conDisBreath, 0 }, { conDisBreath, 1 },/* BTSpkPatPowerOn */
                                             { conDisPowerOnOff, 1 },                                           /* BTSpkPatPowerOff */
                                             { conDisRefresh, 0 },                                              /* BTSpkPatBTPair */
                                             { conDisBTConnected, 0 }, { conDisBTComplete, 0 },                 /* BTSpkPatBTConnected */
                                             { conDisVol, 0 }, { conDisFadeOut, 0 },                            /* BTSpkPatVolLevel */
                                             { conDisColorPick, 0 },                                            /* BTSpkPatPickColor */
                                             { conDisBreath, 3 },                                               /* BTSpkPatUpgrading */
                                             { conDisFlash, 0 },                                                /* BTSpkPatUpgradeSuccess */
                                             { conDisFlash, 1 },                                                /* BTSpkPatUpgradeFailed */
                                             { conDisBreath, 2 },                                               /* BTSpkPatPhone */
                                             { conDisCharge, 0 }                                                /* BTSpkPatCharging */
                                           };
                                                                                                                /* BTSpkPatVolLevel, volDisHeight=runtime determine */
LOCAL CONST CON_DIS_VOL_T   datVol[] = { { 102, 51, GM_VOL_MAX + 1, 3, { { 80, 80, 80 }, { 30, 30, 30 } } } };

LOCAL CONST CON_DIS_VSET_T  datVolSet[] = { { 0, 0 }, { 1, 0 }, { 2, 1 }, { 2, 0 }, { 3, 1 }, { 3, 0 },
                                            { 4, 1 }, { 4, 0 }, { 5, 1 }, { 5, 0 }, { 6, 1 }, { 6, 0 },
                                            { 7, 1 }, { 7, 0 }, { 8, 1 }, { 8, 0 }, { 9, 0 } };

LOCAL CONST CON_DIS_FLASH_T datFlash[] = { { 15, 15, 3, { 0, 255, 0 } },                                         /* BTSpkPatUpgradeSuccess */
                                           { 15, 15, 3, { 255, 0, 0 } }                                          /* BTSpkPatUpgradeFailed */
                                         };
                                         
LOCAL CONST CON_DIS_REFRESH_T datRefresh[] = { { 0, 5, 2, 0, 5, NULL },                                               /* BTSpkPatBTPair */
                                             };
LOCAL CONST CON_DIS_BREATH_T datBreath[] = { { centerR, 4, 1, -10, 0, 15, { 100, 0, 0 } },                        /* BTSpkPatPowerOn */
                                             { centerR, 0, 1,   0, 0, 30, { 100, 0, 0 } },                      
                                             { centerR, 4, 3, -15, 0, 15, { 100, 0, 0 } },
                                             { unChange, 0, 1, 16, 0, 24, { 23, 23, 23 } },                       /* BTSpkPatUpgrading */
                                           };

LOCAL CONST CON_DIS_COLOR_PICK_T datColorPick[] = { {32, 12, 13, {0,0,0}} };                                                  /* BTSpkPatPickColor */

LOCAL CONST CON_DIS_CP_REGION_T datCPRegion[] = { { 6, REN_PHY_FRAME_H, -30, -154 },
                                                  { 3, 5, -15, -77 },
                                                  { -6, 2, 0, 0 },
                                                  { -7, -5, 15, -77 },
                                                  { -REN_PHY_FRAME_W , -8, 30, -154 } };

LOCAL CONST CON_DIS_POWER_ONOFF_T datPowerOnOff[] = { { 3, 0, 1, 96, { 100, 0, 0 } },                             /* BTSpkPatPowerOn */
                                                      { -3, 96, 1, 0, { 100, 0, 0 } }                             /* BTSpkPatPowerOff */
                                                    };
LOCAL CONST CON_DIS_BT_CONNECTED_T datBTConnected[] = { { 6, 6, 6, 255 } };                                     /* BTSpkPatBTConnected */

LOCAL CONST CON_DIS_BT_COMPLETE_T datBTComplete[] = { {50, 6, 40, {0,0,255}} };

LOCAL CONST CON_DIS_CHARGING_T     datCharge[] = { { 103, 100, 50, 0, 6,{ { 50, 85, 0 }, { 0, 255, 0 } } } };   /* BTSpkPatCharging */

LOCAL CONST CON_DIS_FADE_OUT_T datFadeOut[] = { { volFadeColor, 25, 12, 0, { 0, 0, 0 } } };

LOCAL UINT8 volDisHeight;
LOCAL COLOR_T volAdjustColor;

 /****** Refresh frame, to be constructed *************/
 #ifdef RUN_ONE_TIME_ONLY_DAT_REFRESH_FRAME
 
 LOCAL CONST UINT8 datRefreshFrame[] =
 {
  
                              /****************refresh frame  0******************/                              
  0, 0, 63,       0, 0, 63,     0, 0, 63,       0, 0, 63,     0, 0, 63,       0, 0, 63,     0, 0, 63,       0, 0, 63,
  0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,
  0, 0, 191,      0, 0, 191,    0, 0, 191,      0, 0, 191,    0, 0, 191,      0, 0, 191,    0, 0, 191,      0, 0, 191,    
  0, 0, 255,      0, 0, 255,    0, 0, 255,      0, 0, 255,    0, 0, 255,      0, 0, 255,    0, 0, 255,      0, 0, 255,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 63,       0, 0, 63,     0, 0, 63,       0, 0, 63,     0, 0, 63,       0, 0, 63,     0, 0, 63,       0, 0, 63,
  0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,
  0, 0, 191,      0, 0, 191,    0, 0, 191,      0, 0, 191,    0, 0, 191,      0, 0, 191,    0, 0, 191,      0, 0, 191,
  0, 0, 255,      0, 0, 255,    0, 0, 255,      0, 0, 255,    0, 0, 255,      0, 0, 255,    0, 0, 255,      0, 0, 255,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
                              /***********end refresh frame  0********************/

                              
                              /****************refresh frame  1******************/                               
  0, 0, 52,       0, 0, 52,     0, 0, 52,       0, 0, 52,     0, 0, 52,       0, 0, 52,     0, 0, 52,       0, 0, 52,
  0, 0, 116,      0, 0, 116,    0, 0, 116,      0, 0, 116,    0, 0, 116,      0, 0, 116,    0, 0, 116,      0, 0, 116,
  0, 0, 180,      0, 0, 180,    0, 0, 180,      0, 0, 180,    0, 0, 180,      0, 0, 180,    0, 0, 180,      0, 0, 180,    
  0, 0, 244,      0, 0, 244,    0, 0, 244,      0, 0, 244,    0, 0, 244,      0, 0, 244,    0, 0, 244,      0, 0, 244,
  0, 0, 42,       0, 0, 42,     0, 0, 42,       0, 0, 42,     0, 0, 42,       0, 0, 42,     0, 0, 42,       0, 0, 42,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 52,       0, 0, 52,     0, 0, 52,       0, 0, 52,     0, 0, 52,       0, 0, 52,     0, 0, 52,       0, 0, 52,
  0, 0, 116,      0, 0, 116,    0, 0, 116,      0, 0, 116,    0, 0, 116,      0, 0, 116,    0, 0, 116,      0, 0, 116,
  0, 0, 180,      0, 0, 180,    0, 0, 180,      0, 0, 180,    0, 0, 180,      0, 0, 180,    0, 0, 180,      0, 0, 180,
  0, 0, 244,      0, 0, 244,    0, 0, 244,      0, 0, 244,    0, 0, 244,      0, 0, 244,    0, 0, 244,      0, 0, 244,
  0, 0, 42,       0, 0, 42,     0, 0, 42,       0, 0, 42,     0, 0, 42,       0, 0, 42,     0, 0, 42,       0, 0, 42,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
                              /***********end refresh frame    1*******************/
                              
 
                              /****************refresh frame  2*******************/                               
  0, 0, 41,       0, 0, 41,     0, 0, 41,       0, 0, 41,     0, 0, 41,       0, 0, 41,     0, 0, 41,       0, 0, 41,
  0, 0, 105,      0, 0, 105,    0, 0, 105,      0, 0, 105,    0, 0, 105,      0, 0, 105,    0, 0, 105,      0, 0, 105,
  0, 0, 169,      0, 0, 169,    0, 0, 169,      0, 0, 169,    0, 0, 169,      0, 0, 169,    0, 0, 169,      0, 0, 169,    
  0, 0, 233,      0, 0, 233,    0, 0, 233,      0, 0, 233,    0, 0, 233,      0, 0, 233,    0, 0, 233,      0, 0, 233,
  0, 0, 85,       0, 0, 85,     0, 0, 85,       0, 0, 85,     0, 0, 85,       0, 0, 85,     0, 0, 85,       0, 0, 85,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 41,       0, 0, 41,     0, 0, 41,       0, 0, 41,     0, 0, 41,       0, 0, 41,     0, 0, 41,       0, 0, 41,
  0, 0, 105,      0, 0, 105,    0, 0, 105,      0, 0, 105,    0, 0, 105,      0, 0, 105,    0, 0, 105,      0, 0, 105,
  0, 0, 169,      0, 0, 169,    0, 0, 169,      0, 0, 169,    0, 0, 169,      0, 0, 169,    0, 0, 169,      0, 0, 169,
  0, 0, 233,      0, 0, 233,    0, 0, 233,      0, 0, 233,    0, 0, 233,      0, 0, 233,    0, 0, 233,      0, 0, 233,
  0, 0, 85,       0, 0, 85,     0, 0, 85,       0, 0, 85,     0, 0, 85,       0, 0, 85,     0, 0, 85,       0, 0, 85,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
                              /***********end refresh frame    2********************/     
  
 
                              /****************refresh frame  3*******************/                               
  0, 0, 30,       0, 0, 30,     0, 0, 30,       0, 0, 30,     0, 0, 30,       0, 0, 30,     0, 0, 30,       0, 0, 30,
  0, 0, 93,       0, 0, 93,     0, 0, 93,       0, 0, 93,     0, 0, 93,       0, 0, 93,     0, 0, 93,       0, 0, 93,
  0, 0, 157,      0, 0, 157,    0, 0, 157,      0, 0, 157,    0, 0, 157,      0, 0, 157,    0, 0, 157,      0, 0, 157,    
  0, 0, 221,      0, 0, 221,    0, 0, 221,      0, 0, 221,    0, 0, 221,      0, 0, 221,    0, 0, 221,      0, 0, 221,
  0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 30,       0, 0, 30,     0, 0, 30,       0, 0, 30,     0, 0, 30,       0, 0, 30,     0, 0, 30,       0, 0, 30,
  0, 0, 93,       0, 0, 93,     0, 0, 93,       0, 0, 93,     0, 0, 93,       0, 0, 93,     0, 0, 93,       0, 0, 93,
  0, 0, 157,      0, 0, 157,    0, 0, 157,      0, 0, 157,    0, 0, 157,      0, 0, 157,    0, 0, 157,      0, 0, 157,
  0, 0, 221,      0, 0, 221,    0, 0, 221,      0, 0, 221,    0, 0, 221,      0, 0, 221,    0, 0, 221,      0, 0, 221,
  0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,    0, 0, 127,      0, 0, 127,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
                              /***********end refresh frame    3********************/
                              
 
                              /****************refresh frame  4*******************/                               
  0, 0, 19,       0, 0, 19,     0, 0, 19,       0, 0, 19,     0, 0, 19,       0, 0, 19,     0, 0, 19,       0, 0, 19,
  0, 0, 82,       0, 0, 82,     0, 0, 82,       0, 0, 82,     0, 0, 82,       0, 0, 82,     0, 0, 82,       0, 0, 82,
  0, 0, 146,      0, 0, 146,    0, 0, 146,      0, 0, 146,    0, 0, 146,      0, 0, 146,    0, 0, 146,      0, 0, 146,    
  0, 0, 210,      0, 0, 210,    0, 0, 210,      0, 0, 210,    0, 0, 210,      0, 0, 210,    0, 0, 210,      0, 0, 210,
  0, 0, 170,      0, 0, 170,    0, 0, 170,      0, 0, 170,    0, 0, 170,      0, 0, 170,    0, 0, 170,      0, 0, 170,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 19,       0, 0, 19,     0, 0, 19,       0, 0, 19,     0, 0, 19,       0, 0, 19,     0, 0, 19,       0, 0, 19,
  0, 0, 82,       0, 0, 82,     0, 0, 82,       0, 0, 82,     0, 0, 82,       0, 0, 82,     0, 0, 82,       0, 0, 82,
  0, 0, 146,      0, 0, 146,    0, 0, 146,      0, 0, 146,    0, 0, 146,      0, 0, 146,    0, 0, 146,      0, 0, 146,
  0, 0, 210,      0, 0, 210,    0, 0, 210,      0, 0, 210,    0, 0, 210,      0, 0, 210,    0, 0, 210,      0, 0, 210,
  0, 0, 170,      0, 0, 170,    0, 0, 170,      0, 0, 170,    0, 0, 170,      0, 0, 170,    0, 0, 170,      0, 0, 170,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
                              /***********end refresh frame    4*******************/
 
 
                              /****************refresh frame  5******************/                             
  0, 0, 10,       0, 0, 10,     0, 0, 10,       0, 0, 10,     0, 0, 10,       0, 0, 10,     0, 0, 10,       0, 0, 10,
  0, 0, 72,       0, 0, 72,     0, 0, 72,       0, 0, 72,     0, 0, 72,       0, 0, 72,     0, 0, 72,       0, 0, 72,
  0, 0, 136,      0, 0, 136,    0, 0, 136,      0, 0, 136,    0, 0, 136,      0, 0, 136,    0, 0, 136,      0, 0, 136,    
  0, 0, 200,      0, 0, 200,    0, 0, 200,      0, 0, 200,    0, 0, 200,      0, 0, 200,    0, 0, 200,      0, 0, 200,
  0, 0, 213,      0, 0, 213,    0, 0, 213,      0, 0, 213,    0, 0, 213,      0, 0, 213,    0, 0, 213,      0, 0, 213,
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
  0, 0, 10,       0, 0, 10,     0, 0, 10,       0, 0, 10,     0, 0, 10,       0, 0, 10,     0, 0, 10,       0, 0, 10,
  0, 0, 72,       0, 0, 72,     0, 0, 72,       0, 0, 72,     0, 0, 72,       0, 0, 72,     0, 0, 72,       0, 0, 72,
  0, 0, 136,      0, 0, 136,    0, 0, 136,      0, 0, 136,    0, 0, 136,      0, 0, 136,    0, 0, 136,      0, 0, 136,
  0, 0, 200,      0, 0, 200,    0, 0, 200,      0, 0, 200,    0, 0, 200,      0, 0, 200,    0, 0, 200,      0, 0, 200,
  0, 0, 213,      0, 0, 213,    0, 0, 213,      0, 0, 213,    0, 0, 213,      0, 0, 213,    0, 0, 213,      0, 0, 213, 
  0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,      0, 0, 0,        0, 0, 0,
                              /***********end refresh frame   5*******************/                                                                       
 };
 

#endif

/************************************************************************
 Function:    RenderingInitialize
 Description: Initialize the display rendering module
 Input:       VOID
 Return:      VOID
 Remarks:
************************************************************************/
VOID RenderingInitialize(VOID)
{
    /* Step 1 -- Initialize construct display parameters */
    conDisID  = BTSpkPatIdle;
    conDisSeq = REN_CON_DIS_IDLE;

    /* Step 2 -- Initialize other parameters */
    orientationLock = 0;
    pOri = Ori0;            /* Default to 0 degree first */
    cOri = Ori0;
    bgCnt    = 0;
    shapeCnt = 0;
}

/************************************************************************
 Function:    InitRendererDisBuf
 Description: Initialize the virtual and physical display buffer and its
              buffer index for Renderer
 Input:       VOID
 Return:      VOID
 Remarks:
************************************************************************/
VOID InitRendererDisBuf(VOID)
{
    UINT16 i;

    /* Step 1 -- Clean up virtual display buffers */
    for( i = 0; i < REN_VIR_DIS_NUM; i++ )
        clearVirBuf( i );

    /* Step 2 -- Clean up physical display buffers */
    for( i = 0; i < REN_PHY_DIS_VIR_NUM; i++ )
        clearPhyBuf( i );

    /* Step 3 -- Set initial display indexes, active -> 0; load -> 1 */
    virDisActiveInd = 0;
    virDisLoadInd   = 1;

    PhyDisActiveInd = 0;
    PhyDisLoadInd   = 1;
    BarApp = 0;

    /* Step 4 -- Set orientation lock from particle system info */
    orientationLock = 0;
    for( i = 0; i < SIM_PS_NUM; i++ )
    {
        if( SimPS[ i ].psBase.id != psInvalid )
        {
            if( ( SimPS[ i ].psImage.psiFlag & REN_PSI_OLOCK0 ) == REN_PSI_OLOCK0 )
                orientationLock |= ORI0_MASK;

            if( ( SimPS[ i ].psImage.psiFlag & REN_PSI_OLOCK90 ) == REN_PSI_OLOCK90 )
                orientationLock |= ORI90_MASK;
        }
    }

    /* Step 5 -- Set other parameters */
    bgCnt    = 0;
    shapeCnt = 0;
    pOri     = ( UINT8 )GMGetOrientation();         /* Reset orientation info */
    cOri     = pOri;

    /* Scale up color map */
    for( i = 0; i < SIM_PS_NUM; i++ )
    {
        SimPS[ i ].psImage.colorMapInd  *= CIND_SCALE;
        SimPS[ i ].psImage.colorMapLen  *= CIND_SCALE;
        SimPS[ i ].psImage.seedColorInd *= CIND_SCALE;
    }
}

/************************************************************************
 Function:    InitialMapColor
 Description: Initialize color map and color operations
 Input:       cPtr:   pointer to initialize color data set of a pattern
              cOpPtr: pointer to initial color operation array
 Return:      VOID
 Remarks:
************************************************************************/
VOID InitialMapColor(COLOR_T *cPtr, PS_COLOR_OP_T *cOpPtr)
{
    UINT16  i;
    if(IniColor_f==FALSE)
    {
        for( i = 0; i < REN_COLOR_ARRAY_LEN; i++ )
            colorMap[ i ] = *( cPtr + i );
    }
    IniColor_f=FALSE;
    for( i = 0; i < REN_COLOR_OPERATION_NUM; i++ )
        colorOp[ i ] = *( cOpPtr + i );
}

/************************************************************************
 Function:    Renderer
 Description: Render the particle system to a virtual display
 Input:       Nil
 Return:      VOID
 Remarks:     Call from regular timer tick
************************************************************************/
VOID Renderer(VOID)
{
    UINT16  i, disInd;
    BOOL    showFlag;

    //update theme brightness value
    masterBrightness = GMBrightness;

    showFlag = FALSE;
    disInd = virDisLoadInd;
    for( i = 0; i < SIM_PS_NUM; i++ )
    {
        if( ( SimPS[ i ].psBase.id != psInvalid ) && ( ( SimPS[ i ].psBase.psFlag & SIM_PSB_SHOW ) == SIM_PSB_SHOW ) )
        {
            /* Step 1 -- Copy background */
            copyBackground( disInd, i );

            /* Step 2 -- Map colors */
            mapColor( disInd, i );

            showFlag = TRUE;
        }
    }
    /* Step 3 -- Set the virtual display */
    if( showFlag )
    {
        virDisActiveInd = ( UINT8 )disInd;
        virDisLoadInd   = ( UINT8 )( ( disInd + 1 ) % REN_VIR_DIS_NUM );
    }
}

/************************************************************************
 Function:    DisplayManager
 Description: Select the appropriate display channel to update physical display
 Input:       conDisFlag: TRUE: Construct display has to update
                          FALSE: Construct display doesn't update
 Return:      VOID
 Remarks:     Call from system main loop
************************************************************************/
VOID DisplayManager(BOOL conDisFlag)
{
    /* Determine which display method should be applied */
    if( GMBTSpkShowID != BTSpkPatIdle )
    {
        /* BTSpk needs to show something, make it.  It is the highest priority */
        if( ( GMBTSpkShowID != conDisID ) && ( ( conDisID & GM_PATTERN_IDLE_MASK ) == 0 ) )
        {
            /* It is a new display, prepare for it */
            PhyDisLoadInd = REN_PHY_DIS_CON_OFFSET;

            conDisID  = GMBTSpkShowID;
            conDisCnt = datConDisInfo[ conDisID ].repeatCnt;
            prepareConstructDisplay( datConDisInfo[ conDisID ].startSeq );
        }
        if( ( conDisFlag == TRUE ) && ( ( conDisID & GM_PATTERN_IDLE_MASK ) == 0 ) )
            constructDisplay();
    }
    else
    {
        /* Always clear conDisID */
        conDisID = BTSpkPatIdle;

        /* Check if any pattern needs to show */
        if( GMPattern <= PatRefreshMax )
        {
            if( GMPattern <= PatSimMax)
            {
                /* Particle system show */
                mapPhyDisplay();
            }
            else
            {
                /* Image frame show */
                refreshFrame();
            }
        }
        else
        {
            clearPhyBuf( PhyDisLoadInd );
            PhyDisActiveInd = PhyDisLoadInd;
            PhyDisLoadInd = ( PhyDisActiveInd + 1 ) % REN_PHY_DIS_NUM;
        }
    }
}

VOID SetExplosionColor(PARTICLE_ST *pPtr, UINT16 psInd, COLOR_T preColor)
{
    HSL_T   hsl;
    
    if(APP_COLOR != ThemeColorMode)
    {
        hsl.lightness   = 255;
        hsl.saturation  = 512;
        hsl.hue         = SimPS[ psInd ].psBase.cohesion/EXPLOSION_HUE_FACTOR;
        hsl2rgb(hsl, &(pPtr->color));
    }
    else
    {
        pPtr->color = ThemeColor;
    }
    
}

/************************************************************************
 Function:    SetInitColor
 Description: Set the initial color of a particle
 Input:       pPtr :    pointer to the particle
              psInd:    particle system index
              preColor: preset color to start the calculation
 Return:      VOID
 Remarks:
************************************************************************/
VOID SetInitColor(PARTICLE_ST *pPtr, UINT16 psInd, COLOR_T preColor)
{
    UINT16  i, ind;
    COLOR_T color;
    BOOL   csuFlag = FALSE;

    /* Step 1 -- Get initial seed color from particle system */
    if( ( preColor.r | preColor.g | preColor.b ) == 0 )
    {
        ind = SimPS[ psInd ].psImage.seedColorInd;
        color = colorMap[ ind / CIND_SCALE ];
        csuFlag = TRUE;
    }
    else
        color = preColor;

    /* Step 2 -- Handle initial color processes */
    if( SimPS[ psInd ].psImage.icNum > 0 )
    {
        for( i = 0; i < SimPS[ psInd ].psImage.icNum; i++ )
        {
            /* Update color for each defined process one by one */
            updateColor( color, SimPS[ psInd ].psImage.initColorInd + i, pPtr, psInd, &color );
        }
    }

    /* Step 3 -- Set the resultant color to particle */
    pPtr -> color = color;

    /* Step 4 -- Update seed index */
    switch( SimPS[ psInd ].psImage.psiFlag & REN_PSI_CSUM )
    {
        case csuIncrement:
            if( csuFlag == TRUE )
            {
                if( SimPS[ psInd ].psBase.maxCnt < CIND_SCALE )
                    i = CIND_SCALE;
                else if( SimPS[ psInd ].psBase.maxCnt < ( CIND_SCALE * 2 ) )
                    i = CIND_SCALE * 2 - SimPS[ psInd ].psBase.maxCnt;
                else
//                    i = 1;
                    i = CIND_SCALE;

                ind = ( ind + i - SimPS[ psInd ].psImage.colorMapInd ) % SimPS[ psInd ].psImage.colorMapLen +
                  SimPS[ psInd ].psImage.colorMapInd;

                SimPS[ psInd ].psImage.seedColorInd = ind;
            }
            break;

        case csuFixed:
        default:
            break;
    }
}

/************************************************************************
 Function:    SetBackground
 Description: Copy the background image to the Renderer run-time
 Input:       bPtr: pointer to a background image
 Return:      background index in Renderer run-time
 Remarks:
************************************************************************/
UINT8 SetBackground(VIRDIS_T *bPtr)
{
    background[ bgCnt ] = *bPtr;
    bgCnt++;

    return ( bgCnt - 1 );
}

/************************************************************************
 Function:    SetShape
 Description: Copy the shape image to the Renderer run-time
 Input:       sPtr: pointer to a shape image
 Return:      shape index in Renderer run-time
 Remarks:
************************************************************************/
UINT8 SetShape(SHAPE_T *sPtr)
{
    shape[ shapeCnt ] = *sPtr;
    shapeCnt++;

    return ( shapeCnt - 1 );
}

/************************************************************************
 Function:    SetPrismColorMap
 Description: Set color map according to color captured by color sensor
              Prism feature
 Input:       VOID
 Return:      VOID
 Remarks:     Set to SimPS[0] only
************************************************************************/
VOID SetPrismColorMap(VOID)
{
    HSL_T   hslColor, baseColor;
    COLOR_T rgbColor;
    INT16   i,j;

    /* Step 1 -- Set base and derived colors, lightness: -77; hue: +/-15 */
    rgb2hsl( GMColorPick, &baseColor );

    /* Step 1.1 -- Black color check */
    if( ( baseColor.lightness == 0 ) && ( baseColor.saturation == 0 ) )
    {
        /* It is black, avoid to set it to pattern */
        return;
    }

    /* Step 1.2 -- Set the prism color to colorMap */
    hslColor.saturation = baseColor.saturation;
    for( i = -2; i <= 2; i++ )
    {
        if( i > 0 )
            hslColor.lightness = baseColor.lightness - ( i * 77 );
        else
            hslColor.lightness = baseColor.lightness + ( i * 77 );
        hslColor.hue = baseColor.hue + ( i * 15 );

        hsl2rgb( hslColor, &rgbColor );
        colorMap[ ( i + 5 ) % 5 ] = rgbColor;
    }

    /* Step 2 -- Set particle system parameters */
    SimPS[ 0 ].psImage.seedColorInd = 0;                  /* To the base color */
    SimPS[ 0 ].psImage.colorMapInd  = 0;
    SimPS[ 0 ].psImage.colorMapLen  = 5 * CIND_SCALE;

    /* Step 3 -- Special case treatment */
    if( SimPS[ 0 ].psBase.id == psFireworkRaiseUp )
    {
        /* Make the target color be the required color */
        hslColor.hue = baseColor.hue + 60;//% 360;
        hsl2rgb( hslColor, &rgbColor );
        colorMap[ SimPS[ 0 ].psImage.colorTargetInd ] = rgbColor;

        colorMap[ 46 ] = colorMap[ 0 ];     /* Copy shape color */
    }

    else if( SimPS[ 0 ].psBase.id == psCustomized )
    {
        /* Only set 1 color and the second one as reserve */
        SimPS[ 0 ].psImage.colorMapLen  = CIND_SCALE;

        /* Set base color as the lightning color */
        SimPS[ 2 ].psImage.colorMapInd  = 0;
        SimPS[ 2 ].psImage.seedColorInd = 0;
    }
    else if( SimPS[ 0 ].psBase.id == psFire )
    {
        /* Need to remove specific color operations for Fire */
        SimPS[ 0 ].psImage.ucNum = 2;                       /* Remove the blue removal operations */
        SimPS[ 0 ].psImage.seedColorInd = CIND_SCALE;       /* Increase initial hue to make capture color upward */
    }
    else if( SimPS[ 1 ].psBase.id == psHourglassStack )
    {
        for( i = 0; i < SimPMax; i++ )
        {
            if( SimP[ i ].systemID == psHourglassStack )
            {
                /* Change the particle color to the picked color */
                SimP[ i ].color = GMColorPick;
                break;
            }
        }

        SimPS[ 1 ].psImage.seedColorInd = 0;                /* For psHourglassStack */
        SimPS[ 2 ].psImage.seedColorInd = 5 * CIND_SCALE;   /* For psHourglassStackBlink */
        SimPS[ 2 ].psImage.colorMapInd  = 5 * CIND_SCALE;
        SimPS[ 2 ].psImage.colorMapLen  = CIND_SCALE;

        colorMap[ 6 ] = GMColorPick;
    }
    else if( SimPS[ 0 ].psBase.id == psBar )
    {
        hslColor.hue = baseColor.hue - 60;
        hsl2rgb( hslColor, &rgbColor );
        for( i = 0; i < SimPMax; i++ )
        {
            if( SimP[ i ].systemID == psBar )
            {
                /* Change the particle color to the picked color */
                SimP[ i ].color = rgbColor;
                SimP[ i ].aVelocity = 0;
                SimP[ i ].angle = 0;
            }
        }
        SimPS[ 0 ].psImage.ucNum = 0;
        /* Get the dot color */
        colorMap[ 10 ] = GMColorPick;
    }
    else if( GMPattern == PatWave )
    {
        for( i = 3; i >= 0; i-- )
        {
            hslColor.hue = baseColor.hue + ( 3 - i ) * WAVE_HUE_CHANGE;
            hsl2rgb( hslColor, &rgbColor );
            for( j = 0; j < SimPMax; j++ )
            {
                if( SimP[ j ].systemID == SimPS[ i ].psBase.id )
                    SimP[ j ].color = rgbColor;
            }
            
            SimPS[ i ].psImage.ucNum = 0;
        }
        
        hslColor.hue = baseColor.hue + 180;
        hsl2rgb( hslColor, &rgbColor );
        colorMap[ 5 ] = rgbColor;
    }
    
    /* Step 4 -- Prepare broadcast info, color */
    GMPatternInfoTx.pColor[ 0 ] = colorMap[ 0 ];
}

/************************************************************************
 Function:    InterchangeCanvasColor
 Description: process Canvas's shake input by interchanging colors
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to Canvas shake interaction
************************************************************************/
VOID InterchangeCanvasColor( UINT8 psInd )
{
    UINT8 i, colorLen;
    COLOR_T tempColor;

    if( psInd != 0 )
        return ;
    
    colorLen = SimPS[ psInd ].psImage.colorMapLen / CIND_SCALE;
    /* swap color */
    for( i = 0; i < ( colorLen >> 1 ); i++ )
    {
        if( ( colorMap[ i ].r != colorMap[ colorLen - 1 - i ].r ) || ( colorMap[ i ].g != colorMap[ colorLen - 1 - i ].g ) || ( colorMap[ i ].b != colorMap[ colorLen - 1 - i ].b ) )
        {
            tempColor = colorMap[ i ];
            colorMap[ i ] = colorMap[ colorLen - 1 - i ];
            colorMap[ colorLen - 1 - i ] = tempColor;
        }
    }
}

/************************************************************************
 Function:    SetPatternBroadcastColor
 Description: Set colors according to pattern broadcast information received
 Input:       pInfoPtr: pointer to pattern broadcast information
 Return:      VOID
 Remarks:     Handle data from GMPatternInfoRx
************************************************************************/
VOID SetPatternBroadcastColor(PATTERN_INFO_T *pInfoPtr)
{
    /* Not in Canvas, then set as Prism */
    GMColorPick = pInfoPtr -> pColor[ 0 ];
    SetPrismColorMap();         /* Set the color by Prism */
}

/************************************************************************
 Function:    RecvFrameColor
 Description: Receive color data from BT and write them into display buffer
 Input:       i: LED index
              pColor: a LED color
 Return:      VOID
 Remarks:
************************************************************************/
VOID RecvFrameColor( UINT8 i, COLOR_T pColor )
{
    /* Copy color to particle */
    UINT8 row,column;
    row = i / REN_PHY_FRAME_W;
    column = i % REN_PHY_FRAME_W;

    PhyDis[ 0 ].phyDisColor[ column ][ row ] = pColor;
    PhyDis[ 1 ].phyDisColor[ column ][ row ] = pColor;
}

LOCAL VOID mapVirLocationColor_Alpha(PVECTOR_ST pos, COLOR_T color, UINT16 dInd)
{
    INT16   vx, vy;                                 /* Mapped pixel coordinates in virtual display */
    COLOR_T wc, tempC;
    HSL_T   hsl;

    //used for global lightness control
    rgb2hsl( color, &hsl );
    hsl.lightness = hsl.lightness * masterBrightness / GM_BRIGHTNESS_MAX;
    hsl2rgb( hsl, &wc );
    vy =  pos.y;//posH / REN_FRAME2VIR_FRAME;
    vx =  pos.x;//posW / REN_FRAME2VIR_FRAME;
    tempC = virDis[ dInd ].virDisColor[ vx ][ vy ] ;
    if(0 != (tempC.r | tempC.g | tempC.b))
        AlphaColor(wc, tempC,128, &wc);//128
    virDis[ dInd ].virDisColor[ vx ][ vy ] = wc;
}

LOCAL void mapCusLocationColor(PVECTOR_ST pos, COLOR_T color, UINT16 dInd, UINT8 step_cnt, PVECTOR_ST vel)
{
    HSL_T   hsl;
    COLOR_T color_pr, color_ta;
    color_pr = color;
    color_ta = color;
    INT16 vx,vy;
    
    #if 1
    if((vel.y != 0) ||(vel.x!=0))               
    {
        rgb2hsl( color, &hsl );
        hsl.lightness = (hsl.lightness / cus.bcus_speed)* (cus.bcus_speed - step_cnt) ;
        hsl2rgb( hsl, &color );        
        vx = pos.x;
        vy = pos.y;
        if((pos.x >= -6) && (pos.x <= 5)&&(pos.y < 8 && pos.y >-9))
        {
            pos.x += 6;
            pos.y += 6;
            mapVirLocationColor(pos, color, dInd);
        }
        
       rgb2hsl( color_pr, &hsl );
       hsl.lightness = (hsl.lightness  / cus.bcus_speed)* step_cnt;       
       hsl2rgb( hsl, &color_pr );
       pos.y = vy + vel.y;
       pos.x = vx + vel.x;
        if((pos.x >= -6) && (pos.x <= 5)&&(pos.y < 8 && pos.y >-9))
        {
            pos.x += 6;
            pos.y += 6;
            mapVirLocationColor(pos, color_pr, dInd);
        }
#if 0
    if(cus.bcus_busy[7] == 0 && cus.bcus_busy[8] == 0 && cus.bcus_busy[9] == 0  )
        {
       rgb2hsl( color_ta, &hsl );
       hsl.lightness = (hsl.lightness  / cus.bcus_speed) * (cus.bcus_speed - psInd);
       hsl2rgb( hsl, &color_ta );
       pos.y = vy - vel.y;
       pos.x = vx - vel.x;
        if((pos.x >= -6) && (pos.x <= 5)&&(pos.y < 8 && pos.y >-9))
        {
            pos.x += 6;
            pos.y += 6;
            mapVirLocationColor(pos, color_ta, dInd);
        }
        }
        #endif
    }
    #endif

    if((vel.x == 0) && (vel.y == 0))
     {
        if((pos.x >= -6) && (pos.x <= 5)&&(pos.y < 8 && pos.y >-9))
            {
            pos.x += 6;
            pos.y += 6;
            mapVirLocationColor(pos, color, dInd);
            }
      }    
#if 0
    if((pos.x >= -6) && (pos.x <= 5)&&(pos.y < 8 && pos.y >-9))
    {
    
#if 1
        pos.x += 6;
        pos.y += 6;

        mapVirLocationColor(pos, color, dInd);
#endif
        #if 0
        pos.x = pos.x * 16;
        pos.y = pos.y * 16;
        vel.x = 3;// 2
        vel.y = 3;// 3
        mapLocationColor( pos, color, dInd, psInd, vel );
        #endif
    }
    #endif
}

void set_A_line( COLOR_T color, UINT16 dInd, UINT8 step_cnt, PVECTOR_ST vel,UINT8 loc,INT16 val)
{
    PVECTOR_ST  pos;
    INT16 vx,vy;
    UINT8 cnt;
    if(loc==1)//Y=val
    {
       vx = 0;
       vy = val;
       for(cnt = 0;cnt<12;cnt++)
       {
            vx = cnt - 6;
            pos.x = (INT8)vx;
            pos.y = (INT8)vy;
            mapCusLocationColor( pos, color, dInd, step_cnt, vel );
       }
    }
    if(loc==0)//X=val
    {
       vx = val;
       vy = 0;
       for(cnt = 0;cnt<8;cnt++)
       {
            vy= cnt - 4;
            pos.x = (INT8)vx;
            pos.y = (INT8)vy;
            mapCusLocationColor( pos, color, dInd, step_cnt, vel );
       }
    }
    
}

void Customized_test1(UINT16 dInd,COLOR_T col)
{
    PVECTOR_ST  pos;
    PVECTOR_ST acc;
    Cus_partical    partical_pr;
    Cus_partical    partical_ta; 
    COLOR_T color;
    HSL_T   hsl;
    UINT8 sx,sy;
    INT8 i;
    
    cus.bcus_busy[1] = 1;
    acc.x = 3;
    acc.y = -3;
    if (AudioDet==TRUE)
    {
        acc.x = 12;
        acc.y = -12;
    }
//    if( cus.bcus_timer_cnt%cus.bcus_speed == 0) 
    {
        cus.partical1.x += acc.x;
        cus.partical1.y += acc.y;  
    }
    partical_pr.x = cus.partical1.x + 16;
    partical_ta.x =  cus.partical1.x - 16;

    partical_pr.y = cus.partical1.y - 16;
    partical_ta.y =  cus.partical1.y + 16;
/*left*/    
    pos.x = cus.partical1.x/16;
    for(i = 2;i< 10;i++)
    {
        pos.y = i;
        if(pos.x>=6)  
            mapVirLocationColor_Alpha(pos, col, dInd);
    }

/*right*/
    pos.x = cus.partical1.y/16;
    for(i = 2;i< 10;i++)
    {
        pos.y = i;
        if(pos.x<=5)            
            mapVirLocationColor_Alpha(pos, col, dInd);
    }
    
/*left*/    
    pos.x = partical_pr.x/16;
    sx = partical_pr.x%16;
    sy = partical_pr.y%16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*sx/16;
    hsl2rgb( hsl, &color );
    for(i = 2;i< 10;i++)
    {
        pos.y = i;
        mapVirLocationColor_Alpha(pos, color, dInd);
    }
    
/*right*/
    pos.x = partical_pr.y/16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*(16-sy)/16;
    hsl2rgb( hsl, &color );
    for(i = 2;i< 10;i++)
    {
        pos.y = i;
        if(pos.x<=5)
        mapVirLocationColor_Alpha(pos, color, dInd);
    }

/*left*/    
    pos.x = partical_ta.x/16;
    sx = partical_ta.x%16;
    sy = partical_ta.y%16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*(16-sx)/16;
    hsl2rgb( hsl, &color );
    for(i = 2;i< 10;i++)
    {
        pos.y = i;
        if(pos.x>=6) 
        mapVirLocationColor_Alpha(pos, color, dInd);
    }

/*right*/
    pos.x = partical_ta.y/16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*sy/16;
    hsl2rgb( hsl, &color );
    for(i = 2;i< 10;i++)
    {
        pos.y = i;
        if(pos.x<=5) 
        mapVirLocationColor_Alpha(pos, color, dInd);
    }

   
   if(cus.partical1.x >= 208)
    {
        cus.partical1.x = 80;
        cus.partical1.y = 112;       

        cus.bcus_normal_mode_flag = 1;
        cus.bcus_music_flag = 0;
        cus.bcus_busy[1] = 0;
    }

}

void Customized_test2(UINT16 dInd,COLOR_T col)
{
    PVECTOR_ST  pos;
    PVECTOR_ST acc;
    Cus_partical    partical_pr;
    Cus_partical    partical_ta; 
    COLOR_T color;
    HSL_T   hsl;
    UINT8 sx,sy;
    INT8 i;
    
    cus.bcus_busy[2] = 2;
    acc.x = 0;
    acc.y = -3;
        if (AudioDet==TRUE)
    {
        acc.x = 0;
        acc.y = -12;
    }
//    if( cus.bcus_timer_cnt%cus.bcus_speed == 0) 
    {
        cus.partical2.x += acc.x;
        cus.partical2.y += acc.y;  
    }
    partical_pr.y = cus.partical2.y - 16;
    partical_ta.y =  cus.partical2.y + 16;
    pos.x = cus.partical2.x/16;
    pos.y = cus.partical2.y/16;
    for(i = 0;i< 12;i++)
    {
        pos.x = i;
        mapVirLocationColor_Alpha(pos, col, dInd);
    }

    pos.x = partical_pr.x/16;
    pos.y = partical_pr.y/16;
    sx = partical_pr.x%16;
    sy = partical_pr.y%16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*(16-(sy)) /16;
    hsl2rgb( hsl, &color );
    for(i = 0;i< 12;i++)
    {
        pos.x = i;
        mapVirLocationColor_Alpha(pos, color, dInd);
    }

    pos.x = partical_ta.x/16;
    pos.y = partical_ta.y/16;
    sx = partical_ta.x%16;
    sy = partical_ta.y%16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*(sy) /16;
    hsl2rgb( hsl, &color );
    for(i = 0;i< 12;i++)
    {
        pos.x = i;
        mapVirLocationColor_Alpha(pos, color, dInd);
    }
   
   if(cus.partical2.y <= 16)
    {
        cus.partical2.y = 176;

        cus.bcus_normal_mode_flag = 2;
        cus.bcus_music_flag = 0;
        cus.bcus_busy[2] = 0;
    }

}

void Customized_test3(UINT16 dInd,COLOR_T col)
{
    PVECTOR_ST  pos;
    PVECTOR_ST acc;
    Cus_partical    partical_pr;
    Cus_partical    partical_ta; 
    COLOR_T color;
    HSL_T   hsl;
    UINT8 sx,sy;
    INT8 i;
    
    cus.bcus_busy[3] = 3;
    acc.x = 0;
    acc.y = 3;
        if (AudioDet==TRUE)
    {
        acc.x = 0;
        acc.y = 12;
    }
//    if( cus.bcus_timer_cnt%cus.bcus_speed == 0) 
    {
        cus.partical3.x += acc.x;
        cus.partical3.y += acc.y;  
    }
    partical_pr.y = cus.partical3.y + 16;
    partical_ta.y =  cus.partical3.y - 16;
    pos.x = cus.partical3.x/16;
    pos.y = cus.partical3.y/16;
    for(i = 0;i< 12;i++)
    {
        pos.x = i;
        mapVirLocationColor_Alpha(pos, col, dInd);
    }

    pos.x = partical_pr.x/16;
    pos.y = partical_pr.y/16;
    sx = partical_pr.x%16;
    sy = partical_pr.y%16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*sy/16;
    hsl2rgb( hsl, &color );
    for(i = 0;i< 12;i++)
    {
        pos.x = i;
        mapVirLocationColor_Alpha(pos, color, dInd);
    }

    pos.x = partical_ta.x/16;
    pos.y = partical_ta.y/16;
    sx = partical_ta.x%16;
    sy = partical_ta.y%16;
    rgb2hsl( col, &hsl );
    hsl.lightness = (hsl.lightness)*(16-sy)/16;
    hsl2rgb( hsl, &color );
    for(i = 0;i< 12;i++)
    {
        pos.x = i;
        mapVirLocationColor_Alpha(pos, color, dInd);
    }
   
   if(cus.partical3.y >= 176)
    {
        cus.partical3.y = 16;

        cus.bcus_normal_mode_flag = 3;
        cus.bcus_music_flag = 0;
        cus.bcus_busy[3] = 0;
    }

}
void Customized_test4(UINT16 dInd,COLOR_T col)
{
    PVECTOR_ST  pos,vel;
    HSL_T   hsl;
    COLOR_T color,color_temp;
    INT16 vx,vy;
    INT16 lightness_pr,lightness_ta,lightness_gap;
    signed int j;
    UINT8 length,thickness,step_cnt;

    cus.bcus_busy[4] = 4;
    length = 5;
    thickness = 3;
    vx = 5;
    vy = -4;    
    color = col;
    color_temp = col;
    if (AudioDet==TRUE)
        cus.bcus_speed = 2;
    step_cnt = cus.bcus_timer_cnt % cus.bcus_speed;
    step_cnt +=1;

    vel.x = 0;
    vel.y = 0;
       if( cus.bcus_timer_cnt%cus.bcus_speed == 0)
       {    
            cus.bcus_lifecnt4++;
            if (AudioDet==TRUE)
                cus.bcus_lifecnt4++;
        }
       vx -=  cus.bcus_lifecnt4 ; 
       vy = cus.cus_loc.y;
       for(j = 0;(j<=length);j++)
        {                      
               pos.x = vx;
               pos.y = vy;
               
               if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               
               #if 1
               if(vy >= 0)
            {
               pos.y = vy - (thickness+1);
               pos.x = (-vx) - 1;
                 if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }                     
                   
                   
                    }
               else
                    {
                   pos.y = vy + (thickness+1);
                   pos.x = (-vx) - 1;
                   if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
                    }
  #endif         
           vx = vx + 1;
        }
         if(cus.bcus_lifecnt4 >= 12 + length)
        {
            cus.bcus_lifecnt4 = 0;
//            cus.cus_loc.y = SimRandom(0 , 7) - 4;
            cus.cus_loc.y += 1;
            if(cus.cus_loc.y >= 4)
                cus.cus_loc.y = -4;
            cus.bcus_normal_mode_flag = 4;
            cus.bcus_music_flag = 0;
            cus.bcus_busy[4] = 0;
        }

}


void Customized_test5(UINT16 dInd,COLOR_T col)
{
    PVECTOR_ST  pos,vel;
    COLOR_T color,color_temp;
    HSL_T   hsl;
    INT16 lightness_pr,lightness_ta,lightness_gap;
    INT16 vx,vy;
    UINT8 j;
    UINT8 length,thickness,step_cnt;

    length = 5;
    thickness = 5;
    vx = 5;
    vy = 3;  
    color = col;
    color_temp = col;
    if (AudioDet==TRUE)
        cus.bcus_speed = 2;
    step_cnt = cus.bcus_timer_cnt % cus.bcus_speed;
    step_cnt += 1;
    vel.x = 0;
    vel.y = 0;
    cus.bcus_busy[5] = 5;
       if( cus.bcus_timer_cnt%cus.bcus_speed == 0)
       {    
            cus.bcus_lifecnt5++;
            if (AudioDet==TRUE)
                cus.bcus_lifecnt5++;      
       }
       
       vy = cus.bcus_lifecnt5 - 4;
       vx = cus.cus_loc.x; 
#if 1

       for(j = 0;(j<=length);j++)
        {                      


               pos.x = vx;
               pos.y = vy;

               if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
#if 1
               if(vx >= 0)
                {
                   pos.x = vx - (thickness+1);
                   pos.y = (-vy) -1;

                  if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
                }
               else
                {
                   pos.x = vx + (thickness+1);
                   pos.y = (-vy) -1;

                  if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
                }
#endif          
           vy = vy - 1;
        }

       
       #endif 
       if( cus.bcus_lifecnt5 >= (8 + length) )
        {
            cus.bcus_lifecnt5 = 0;
//            cus.cus_loc.x = (SimRandom(0 , 11) - 6);
            cus.cus_loc.x -= 1;
            if(cus.cus_loc.x <= -7)
                cus.cus_loc.x = 5;
            cus.bcus_normal_mode_flag = 5;
            cus.bcus_music_flag = 0;
            cus.bcus_busy[5] = 0;
        }

}
void Customized_test6(UINT16 dInd,COLOR_T col)
{
    PVECTOR_ST  pos,vel;
    COLOR_T color,color_temp;
    INT16 lightness_pr,lightness_ta,lightness_gap;
    HSL_T   hsl;
    INT16 vx,vy,sx,sy;
    UINT8 j;
    UINT8 length,thickness,step_cnt;

    cus.bcus_busy[6] = 6;
    length = 5;//8
    thickness = 5;
    vx = 5;
    vy = -4;
    vx = cus.cus_loc1.x;
    color = col;
    color_temp = col;
    if (AudioDet==TRUE)
        cus.bcus_speed = 2;
    step_cnt = cus.bcus_timer_cnt % cus.bcus_speed;
    step_cnt += 1;
    vel.x = 0;
    vel.y = 0;
       if( cus.bcus_timer_cnt%cus.bcus_speed == 0)
       {    
            cus.bcus_lifecnt6++;
            if (AudioDet==TRUE)
                cus.bcus_lifecnt6++;
       }
        vx = vx - cus.bcus_lifecnt6;
        vy = vy +  cus.bcus_lifecnt6;
        pos.x = vx;
        pos.y = vy;
        
        if(pos.x <= -7)
            pos.x += 12;
        
        sx = vx;
        sy = vy;
#if 1           
       for(j = 0;j<=length;j++)
        {
           pos.x = sx ;
           pos.y = sy ;
             if(pos.x <= -7)
                pos.x += 12;
                if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

           pos.x = sx - 6 ;
           pos.y = (-sy) - 1;
           
            if(pos.x <= -7)
                pos.x += 12;
           if(j==0)
                {
                rgb2hsl( color_temp, &hsl );
                hsl.lightness = ((hsl.lightness/4) / cus.bcus_speed) * step_cnt;                
                hsl2rgb( hsl, &col );
                mapCusLocationColor( pos,  col,  dInd,  step_cnt,  vel);
                } 
               if(j==1)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = hsl.lightness/4;//127                
                lightness_ta = hsl.lightness;//255               
                lightness_gap = lightness_ta - lightness_pr;
                hsl.lightness = lightness_pr + ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==2)
                {
                rgb2hsl( color_temp, &hsl );                
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//255                
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//204               
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }

               if(j==3)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//204
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//153
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
               if(j==4)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//153
                lightness_ta = (hsl.lightness/length)*(length-(j-1));//102
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
 //               hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                } 
               if(j==5)
                {
                rgb2hsl( color_temp, &hsl );
                lightness_pr = (hsl.lightness/length)*(length-(j-2));//102
                //lightness_ta = (hsl.lightness/length)*(length-j);//51
                lightness_ta = 0;
                lightness_gap = lightness_pr - lightness_ta;
                hsl.lightness = lightness_pr - ((lightness_gap/cus.bcus_speed) * step_cnt);
//                hsl.lightness = lightness_pr;
                hsl2rgb( hsl, &color );
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                }
           
           sx = sx + 1;
           sy = sy - 1;
        }
#endif
        if(cus.bcus_lifecnt6 >= length + 8)
       {
            cus.cus_loc1.x -= 1;
            if(cus.cus_loc1.x <= -7)
                cus.cus_loc1.x = 5;
            cus.bcus_lifecnt6 = 0;
            cus.bcus_normal_mode_flag = 6;
            cus.bcus_music_flag = 0;
            cus.bcus_busy[6] = 0;
            
       }

}

void Customized_test7(UINT16 dInd, COLOR_T col)
{
    PVECTOR_ST  pos,vel;
    COLOR_T color;
    INT16 vx,vy;
    UINT8 j,step_cnt;
    color = col;
    vx = 0;
    vy = 0;
    vel.x = 0;
    vel.y = 0;
    cus.bcus_busy[7] = 7;
    if (AudioDet==TRUE)
        cus.bcus_speed = 2;
    step_cnt = (cus.bcus_timer_cnt%cus.bcus_speed);   
    step_cnt += 1;
    if( cus.bcus_timer_cnt%cus.bcus_speed == 0)
    {    
        cus.bcus_lifecnt7++;
//        cus.bcus_lifecnt7++;
    }   

    if(cus.bcus_lifecnt7 == 0)
    {
        pos.x = 0;
        pos.y = 0;
        vel.x = 0;
        vel.y = 0;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
    }
    else
        {
            vx += (cus.bcus_lifecnt7 -1);
            vy += (cus.bcus_lifecnt7 -1);
            for(j = 0;j<=((cus.bcus_lifecnt7 -1) * 2);j++)
            {
                pos.x = vx;
                pos.y = vy - j;
                vel.x = 1;
                vel.y = 0;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x = vx - j;
                pos.y = vy;
                vel.x = 0;
                vel.y = 1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x = (-vx) + j;
                pos.y = -vy - 1;
                vel.x = 0;
                vel.y = -1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x = -vx;
                pos.y = (-vy -1) + j;  
                vel.x = -1;
                vel.y = 0;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        //
                pos.x = -vx;
                pos.y = vy - j;
                vel.x = -1;
                vel.y = 0;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x = vx - j;
                pos.y = -vy -1;
                vel.x = 0;
                vel.y = -1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x = (-vx) + j;
                pos.y = vy;
                vel.x = 0;
                vel.y = 1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x = vx;
                pos.y = (-vy -1) + j;        
                vel.x = 1;
                vel.y = 0;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
            }
        }
     if(cus.bcus_lifecnt7 >= 8)
    {
        cus.bcus_lifecnt7 = 0;
        cus.bcus_normal_mode_flag = 7;
        cus.bcus_music_flag = 0;
        cus.bcus_busy[7] = 0;
    }
}

void Customized_test8(UINT16 dInd, COLOR_T col)
{
    PVECTOR_ST  pos,vel;
    COLOR_T color;
    HSL_T   hsl;
    INT16 vx,vy,sx,sy;
    UINT8 step_cnt;
    color = col;    
    cus.bcus_busy[8] = 8;
    vx = 0;
    vy = 0;
    sx = 0;
    sy = -1;
    vel.x = 0;
    vel.y = 0;
    if (AudioDet==TRUE)
        cus.bcus_speed = 2;
    step_cnt = (UINT8)(cus.bcus_timer_cnt%cus.bcus_speed);   
    step_cnt +=1;
    if( cus.bcus_timer_cnt%cus.bcus_speed == 0)
    {    
        cus.bcus_lifecnt8++;
//        cus.bcus_lifecnt8++;
    }
    switch(cus.bcus_lifecnt8)
    {
        case 0:
        pos.x = 0;
        pos.y = 0;
        rgb2hsl( color, &hsl );
        hsl.lightness = (hsl.lightness  / cus.bcus_speed)* step_cnt;       
        hsl2rgb( hsl, &color );
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

        pos.x = -6;
        pos.y = 0;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);        
        break;
        case 1:
        pos.x = 0;
        pos.y = 0; 
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        pos.x = -6;
        pos.y = 0;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel); 
        pos.x = 0;
        pos.y = -1;
        rgb2hsl( color, &hsl );
        hsl.lightness = (hsl.lightness  / cus.bcus_speed)* step_cnt;       
        hsl2rgb( hsl, &color );
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        pos.x = -6;
        pos.y = -1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        break;
        case 2:
        pos.x = 0;
        pos.y = 0;
        vel.x = 1;
        vel.y = 1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        vel.x = -1;
        vel.y = 1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        
        pos.x = 0;
        pos.y = -1;
        vel.x = 1;
        vel.y = -1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        vel.x = -1;
        vel.y = -1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

        pos.x = -6;
        pos.y = 0;
        vel.x = 1;
        vel.y = 1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        vel.x = -1;
        vel.y = 1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        
        pos.x = -6;
        pos.y = -1;
        vel.x = 1;
        vel.y = -1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        vel.x = -1;
        vel.y = -1;
        mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        break;

        case 3:
        case 4:
        case 5:
        case 6:
                vx += (cus.bcus_lifecnt8 -2);// 1,2,3,4
                vy += (cus.bcus_lifecnt8 -2);// 1,2,3,4
                pos.x = vx;
                pos.y = vy;
                vel.x = 1;
                vel.y = 1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                
                pos.x -= 6;
                if(pos.x <= -7)
                    pos.x += 12;
                vel.x = 1;
                vel.y = 1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//shadow
//                pos.x -= 1;
//                pos.y -= 1;
//                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//
                pos.x = -vx;
                pos.y = vy;
                vel.x = -1;
                vel.y = 1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
                
                pos.x -= 6;
                if(pos.x <= -7)
                    pos.x += 12;
                vel.x = -1;
                vel.y = 1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//shadow
//                pos.x += 1;
//               pos.y -= 1;
//                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//

                sx += (cus.bcus_lifecnt8 -2);
                sy -= (cus.bcus_lifecnt8 -2);
                pos.x = sx;
                pos.y = sy;
                vel.x = 1;
                vel.y = -1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x -= 6;
                if(pos.x <= -7)
                    pos.x += 12;
                vel.x = 1;
                vel.y = -1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//shadow
//                pos.x -= 1;
//                pos.y += 1;
//                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//
                pos.x = -sx;
                pos.y = sy;
                vel.x = -1;
                vel.y = -1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);

                pos.x -= 6;
                if(pos.x <= -7)
                    pos.x += 12;
                vel.x = -1;
                vel.y = -1;
                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//shadow
 //               pos.x += 1;
//                pos.y += 1;
//                mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);//
            break;
            case 7:
                cus.bcus_lifecnt8 = 0;
                cus.bcus_normal_mode_flag = 8;
                cus.bcus_music_flag = 0;
                cus.bcus_busy[8] = 0;
            break;
            default:
                cus.bcus_lifecnt8 = 0;
                break;
    }
}

void Customized_test9(UINT16 dInd, COLOR_T col)
{
    PVECTOR_ST  pos,vel;
    COLOR_T color,empty_color;
    HSL_T   hsl;
    UINT8 step_cnt;
    color = col;
    vel.x = 0;
    vel.y = 0;
    empty_color.r = 0;
    empty_color.g = 0;
    empty_color.b = 0;
    if (AudioDet==TRUE)
        cus.bcus_speed = 2;
    step_cnt = (UINT8)(cus.bcus_timer_cnt%cus.bcus_speed);   
    step_cnt +=1;
    cus.bcus_busy[9] = 9;
    if( cus.bcus_timer_cnt%cus.bcus_speed == 0)
    {    
        cus.bcus_lifecnt9++;
//        cus.bcus_lifecnt9++;

    }
    switch(cus.bcus_lifecnt9)
    {
        case 0:
            vel.x = 0;
            vel.y = 0;
            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness  / cus.bcus_speed)* step_cnt;
            hsl2rgb( hsl, &color );
            set_A_line(color, dInd, step_cnt, vel, 1,-4);//8
            set_A_line(color, dInd, step_cnt,  vel, 1,-3);
            set_A_line(color, dInd, step_cnt,  vel, 1,-2);
            set_A_line(color, dInd, step_cnt,  vel, 1,-1);
            set_A_line(color, dInd, step_cnt,  vel, 1,0);
            set_A_line(color, dInd, step_cnt,  vel, 1,1);
            set_A_line(color, dInd, step_cnt,  vel, 1,2);
            set_A_line(color, dInd, step_cnt,  vel, 1,3);
            break;
        case 1:
            set_A_line(color, dInd, step_cnt,  vel, 1,-4);//8
            set_A_line(color, dInd, step_cnt,  vel, 1,-3);
            set_A_line(color, dInd, step_cnt,  vel, 1,-2);
            set_A_line(color, dInd, step_cnt,  vel, 1,-1);
            set_A_line(color, dInd, step_cnt,  vel, 1,0);
            set_A_line(color, dInd, step_cnt,  vel, 1,1);
            set_A_line(color, dInd, step_cnt,  vel, 1,2);
            set_A_line(color, dInd, step_cnt,  vel, 1,3);
            
            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness / cus.bcus_speed)* (cus.bcus_speed - step_cnt) ;
            hsl2rgb( hsl, &color );
            set_A_line(color, dInd, step_cnt,  vel, 0,-6); 
        break;
        case 2:
            set_A_line(color, dInd, step_cnt,  vel, 0,4);//7            
            set_A_line(color, dInd, step_cnt,  vel, 0,3);
            set_A_line(color, dInd, step_cnt,  vel, 0,2);
            set_A_line(color, dInd, step_cnt,  vel, 0,1);
            set_A_line(color, dInd, step_cnt,  vel, 0,0);
            set_A_line(color, dInd, step_cnt,  vel, 0,-1);
            set_A_line(color, dInd, step_cnt,  vel, 0,-2);
            set_A_line(color, dInd, step_cnt,  vel, 0,-3);
            set_A_line(color, dInd, step_cnt,  vel, 0,-4);
            
            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness / cus.bcus_speed) * (cus.bcus_speed - step_cnt);
            hsl2rgb( hsl, &color );
            set_A_line(color, dInd, step_cnt,  vel, 0,5);
            set_A_line(color, dInd, step_cnt,  vel, 0,-5);            
        break;        
        case 3:
            set_A_line(color, dInd, step_cnt,  vel, 0,3);//6            
            set_A_line(color, dInd, step_cnt,  vel, 0,2);
            set_A_line(color, dInd, step_cnt,  vel, 0,1);
            set_A_line(color, dInd, step_cnt,  vel, 0,0);
            set_A_line(color, dInd, step_cnt,  vel, 0,-1);
            set_A_line(color, dInd, step_cnt,  vel, 0,-2);
            set_A_line(color, dInd, step_cnt,  vel, 0,-3);
            
            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness / cus.bcus_speed) * (cus.bcus_speed - step_cnt);
            hsl2rgb( hsl, &color );
            set_A_line(color, dInd, step_cnt,  vel, 0,4);
            set_A_line(color, dInd, step_cnt,  vel, 0,-4); 
        break;        
        case 4:
            set_A_line(color, dInd, step_cnt,  vel, 0,2);//5
            set_A_line(color, dInd, step_cnt,  vel, 0,1);
            set_A_line(color, dInd, step_cnt,  vel, 0,0);
            set_A_line(color, dInd, step_cnt,  vel, 0,-1);
            set_A_line(color, dInd, step_cnt,  vel, 0,-2);
            
            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness / cus.bcus_speed) * (cus.bcus_speed - step_cnt);
            hsl2rgb( hsl, &color );
            set_A_line(color, dInd, step_cnt,  vel, 0,3);
            set_A_line(color, dInd, step_cnt,  vel, 0,-3);
            set_A_line(color, dInd, step_cnt,  vel, 1,3);
            set_A_line(color, dInd, step_cnt,  vel, 1,-4);
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-5);   
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-6); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,5); 
            
        break;
        case 5:
            set_A_line(color, dInd, step_cnt,  vel, 0,1);// 4            
            set_A_line(color, dInd, step_cnt,  vel, 0,0);
            set_A_line(color, dInd, step_cnt,  vel, 0,-1);

            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness / cus.bcus_speed) * (cus.bcus_speed - step_cnt);
            hsl2rgb( hsl, &color );
            set_A_line(color, dInd, step_cnt,  vel, 0,2);
            set_A_line(color, dInd, step_cnt,  vel, 0,-2);
            set_A_line(color, dInd, step_cnt,  vel, 1,2);
            set_A_line(color, dInd, step_cnt,  vel, 1,-3);
            set_A_line(empty_color, dInd, step_cnt, vel, 1,-4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 1,3);
            
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-3); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-5);   
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-6); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,3); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,5); 
        break;
        case 6:
            set_A_line(color, dInd, step_cnt,  vel, 0,0);// 3
            
            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness / cus.bcus_speed) * (cus.bcus_speed - step_cnt);
            hsl2rgb( hsl, &color );
            set_A_line(color, dInd, step_cnt,  vel, 0,1);
            set_A_line(color, dInd, step_cnt,  vel, 0,-1);
            set_A_line(color, dInd, step_cnt,  vel, 1,1);
            set_A_line(color, dInd, step_cnt,  vel, 1,-2);

            set_A_line(empty_color, dInd, step_cnt, vel, 1,-4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 1,3);   
            set_A_line(empty_color, dInd, step_cnt, vel, 1,-3); 
            set_A_line(empty_color, dInd, step_cnt, vel, 1,2);
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-2); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-3); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-5);   
            set_A_line(empty_color, dInd, step_cnt, vel, 0,-6); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,2); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,3); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,4); 
            set_A_line(empty_color, dInd, step_cnt, vel, 0,5); 
        break;
        case 7:
             pos.x = 0;
             pos.y = 0;
             mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);// 2
             pos.x = 0;
             pos.y = -1;
             rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness / cus.bcus_speed) * (cus.bcus_speed - step_cnt);
             hsl2rgb( hsl, &color );
             mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);
        break;
        case 8:
            pos.x = 0;
            pos.y = 0;
            rgb2hsl( color, &hsl );
            hsl.lightness = (hsl.lightness /cus.bcus_speed)*(cus.bcus_speed - step_cnt) ;
            hsl2rgb( hsl, &color );
            mapCusLocationColor( pos,  color,  dInd,  step_cnt,  vel);// 1
        break;
        case 9:
            cus.bcus_lifecnt9 = 0;
            cus.bcus_normal_mode_flag = 9;
            cus.bcus_music_flag = 0;
            cus.bcus_busy[9] = 0;
        break;
        default:
            cus.bcus_lifecnt9 = 0;
            break;
    }
 #if 0   
    if(cus.bcus_lifecnt9 >= 8)
    {
        cus.bcus_lifecnt9 = 0;
        cus.bcus_normal_mode_flag = 9;
        cus.bcus_music_flag = 0;
        cus.bcus_busy[9] = 0;
    }
#endif
}
LOCAL VOID Customize_Normal(UINT16 dInd)
{
    HSL_T   hsl;
    cus.bcus_speed = 5;//15 normal mode speed
//                Customized_test8(dInd, cus.bcus_col);
#if 1   
//loop the choosed light effects in order.
        /*low mode*/
        if (cus.bcus_busy[cus.bcus_low_mod] == 0 && cus.bcus_busy[cus.bcus_high_mod] == 0 && cus.bcus_busy[cus.bcus_mid_mod] == 0)
            cus.bcus_busy[cus.bcus_low_mod] = cus.bcus_low_mod;
        if (cus.bcus_low_mod != 0)//low 
        {
            if (cus.bcus_busy[1] == cus.bcus_low_mod)
                Customized_test1(dInd, cus.bcus_col);
            if (cus.bcus_busy[2] == cus.bcus_low_mod)
                Customized_test2(dInd, cus.bcus_col);
            if (cus.bcus_busy[3] == cus.bcus_low_mod)
                Customized_test3(dInd, cus.bcus_col);
            if (cus.bcus_busy[4] == cus.bcus_low_mod)
                Customized_test4(dInd, cus.bcus_col);
            if (cus.bcus_busy[5] == cus.bcus_low_mod)
                Customized_test5(dInd, cus.bcus_col);
            if (cus.bcus_busy[6] == cus.bcus_low_mod)
                Customized_test6(dInd, cus.bcus_col);
            if (cus.bcus_busy[7] == cus.bcus_low_mod)
                Customized_test7(dInd, cus.bcus_col);
            if (cus.bcus_busy[8] == cus.bcus_low_mod)
                Customized_test8(dInd, cus.bcus_col);
            if (cus.bcus_busy[9] == cus.bcus_low_mod)
                Customized_test9(dInd, cus.bcus_col);
        }
        /*high mode*/
 if(cus.bcus_busy[cus.bcus_low_mod] == 0 && cus.bcus_busy[cus.bcus_high_mod] == 0 && cus.bcus_busy[cus.bcus_mid_mod] == 0)
    {
        cus.bcus_busy[cus.bcus_high_mod] = cus.bcus_high_mod;
        rgb2hsl( cus.bcus_col, &hsl );
        hsl.hue += 30;
        hsl2rgb( hsl, &cus.bcus_col );
    }
        if (cus.bcus_high_mod != 0)//high
        {
            if (cus.bcus_busy[1] == cus.bcus_high_mod)
                Customized_test1(dInd, cus.bcus_col);
            if (cus.bcus_busy[2] == cus.bcus_high_mod)
                Customized_test2(dInd, cus.bcus_col);
            if (cus.bcus_busy[3] == cus.bcus_high_mod)
                Customized_test3(dInd, cus.bcus_col);
            if (cus.bcus_busy[4] == cus.bcus_high_mod)
                Customized_test4(dInd, cus.bcus_col);
            if (cus.bcus_busy[5] == cus.bcus_high_mod)
                Customized_test5(dInd, cus.bcus_col);
            if (cus.bcus_busy[6] == cus.bcus_high_mod)
                Customized_test6(dInd, cus.bcus_col);
            if (cus.bcus_busy[7] == cus.bcus_high_mod)
                Customized_test7(dInd, cus.bcus_col);
            if (cus.bcus_busy[8] == cus.bcus_high_mod)
                Customized_test8(dInd, cus.bcus_col);
            if (cus.bcus_busy[9] == cus.bcus_high_mod)
                Customized_test9(dInd, cus.bcus_col);
        }
        /*mid mode*/
  if(cus.bcus_busy[cus.bcus_low_mod] == 0 && cus.bcus_busy[cus.bcus_high_mod] == 0 && cus.bcus_busy[cus.bcus_mid_mod] == 0)
  {
            cus.bcus_busy[cus.bcus_mid_mod] = cus.bcus_mid_mod;
            rgb2hsl(cus.bcus_col, &hsl);
            hsl.hue += 30;
            hsl2rgb(hsl, &cus.bcus_col);
        }
        if (cus.bcus_mid_mod != 0)//mid
        {
            if (cus.bcus_busy[1] == cus.bcus_mid_mod)
                Customized_test1(dInd, cus.bcus_col);
            if (cus.bcus_busy[2] == cus.bcus_mid_mod)
                Customized_test2(dInd, cus.bcus_col);
            if (cus.bcus_busy[3] == cus.bcus_mid_mod)
                Customized_test3(dInd, cus.bcus_col);
            if (cus.bcus_busy[4] == cus.bcus_mid_mod)
                Customized_test4(dInd, cus.bcus_col);
            if (cus.bcus_busy[5] == cus.bcus_mid_mod)
                Customized_test5(dInd, cus.bcus_col);
            if (cus.bcus_busy[6] == cus.bcus_mid_mod)
                Customized_test6(dInd, cus.bcus_col);
            if (cus.bcus_busy[7] == cus.bcus_mid_mod)
                Customized_test7(dInd, cus.bcus_col);
            if (cus.bcus_busy[8] == cus.bcus_mid_mod)
                Customized_test8(dInd, cus.bcus_col);
            if (cus.bcus_busy[9] == cus.bcus_mid_mod)
                Customized_test9(dInd, cus.bcus_col);
        }
 if(cus.bcus_busy[cus.bcus_low_mod] == 0 && cus.bcus_busy[cus.bcus_high_mod] == 0 && cus.bcus_busy[cus.bcus_mid_mod] == 0)
  {
            rgb2hsl(cus.bcus_col, &hsl);
            hsl.hue += 30;
            hsl2rgb(hsl, &cus.bcus_col);
        }
#endif
#if 0    
//loop all effects from 1st to 9th
    switch(cus.bcus_normal_mode_flag)
        {
            case 0:
                Customized_test1(dInd, cus.bcus_col);
                break;
            case 1:
                Customized_test2(dInd, cus.bcus_col);
                break;
            case 2:
                Customized_test3(dInd, cus.bcus_col);
                break;
            case 3:
                Customized_test4(dInd, cus.bcus_col);
                break;
            case 4:
                Customized_test5(dInd, cus.bcus_col);
                break;
            case 5:
                Customized_test6(dInd, cus.bcus_col);
                break;
            case 6:
                Customized_test7(dInd, cus.bcus_col);
                break;
            case 7:
                Customized_test8(dInd, cus.bcus_col);
                break;
            case 8:
                Customized_test9(dInd, cus.bcus_col);
                break;
            case 9:
                rgb2hsl(cus.bcus_col, &hsl);
                hsl.hue += 30;
                hsl2rgb(hsl, &cus.bcus_col);
                cus.bcus_normal_mode_flag = 0;
                break;
            default:
                cus.bcus_normal_mode_flag = 0;
                break;
        }
#endif
                    
}
LOCAL VOID Customize_Music(UINT16 dInd)
{
    HSL_T   hsl;
   cus.detect_mode = 1;//DetectLowMidHigh();
#if 1
     if((cus.detect_mode&0x01)&& cus.bcus_busy[cus.bcus_low_mod]==0 && cus.bcus_low_mod != 0)  //low
    {
        cus.bcus_busy[cus.bcus_low_mod] = cus.bcus_low_mod;//low
        cus.bcus_music_flag = 1;
        
        rgb2hsl( cus.bcus_col1, &hsl );
        hsl.hue += 10;
        hsl2rgb( hsl, &cus.bcus_col1 );      

        if(cus.bcus_busy[cus.bcus_mid_mod] == 0)
            cus.bcus_col2 = cus.bcus_col1;
        if(cus.bcus_busy[cus.bcus_high_mod] == 0)
            cus.bcus_col3 = cus.bcus_col1;
    }                
     if((cus.detect_mode&0x02)&&cus.bcus_busy[cus.bcus_mid_mod]==0&&cus.bcus_mid_mod!=0)  //mid
     {
        cus.bcus_busy[cus.bcus_mid_mod] = cus.bcus_mid_mod;//mid
        cus.bcus_music_flag = 1;

        if(cus.bcus_col2.r == cus.bcus_col1.r && cus.bcus_col2.g == cus.bcus_col1.g && cus.bcus_col2.b == cus.bcus_col1.b )
        {
        rgb2hsl( cus.bcus_col2, &hsl );
        hsl.hue -= 18;
        hsl2rgb( hsl, &cus.bcus_col2 );
        }
        else
        {
        rgb2hsl( cus.bcus_col2, &hsl );
        hsl.hue += 10;
        hsl2rgb( hsl, &cus.bcus_col2 );
        }
        
     }
      if((cus.detect_mode&0x04)&&cus.bcus_busy[cus.bcus_high_mod]==0&&cus.bcus_high_mod!=0) //high
      {
        cus.bcus_busy[cus.bcus_high_mod] = cus.bcus_high_mod;//high
        cus.bcus_music_flag = 1;

        if(cus.bcus_col3.r == cus.bcus_col1.r && cus.bcus_col3.g == cus.bcus_col1.g && cus.bcus_col3.b == cus.bcus_col1.b )
        {
        rgb2hsl( cus.bcus_col3, &hsl );
        hsl.hue += 18;
        hsl2rgb( hsl, &cus.bcus_col3 );
        }
        else
        {
        rgb2hsl( cus.bcus_col3, &hsl );
        hsl.hue += 10;
        hsl2rgb( hsl, &cus.bcus_col3 );
        }
      } 
          cus.bcus_speed = 2;  //eq mode speed   //3  
      
       if(cus.bcus_mid_mod != 0)//mid
        {
          if(cus.bcus_busy[1] == cus.bcus_mid_mod)
            Customized_test1(dInd,cus.bcus_col2);
          if(cus.bcus_busy[2] == cus.bcus_mid_mod)
            Customized_test2(dInd,cus.bcus_col2);
          if(cus.bcus_busy[3] == cus.bcus_mid_mod)
            Customized_test3(dInd,cus.bcus_col2);
          if(cus.bcus_busy[4] == cus.bcus_mid_mod)
            Customized_test4(dInd,cus.bcus_col2);
          if(cus.bcus_busy[5] == cus.bcus_mid_mod)
            Customized_test5(dInd,cus.bcus_col2);
          if(cus.bcus_busy[6] == cus.bcus_mid_mod)
            Customized_test6(dInd,cus.bcus_col2);
          if(cus.bcus_busy[7] == cus.bcus_mid_mod)
            Customized_test7(dInd,cus.bcus_col2);
           if(cus.bcus_busy[8] == cus.bcus_mid_mod)
            Customized_test8(dInd,cus.bcus_col2);
          if(cus.bcus_busy[9] == cus.bcus_mid_mod)
            Customized_test9(dInd,cus.bcus_col2);
        }

     if(cus.bcus_high_mod != 0)//high
     {
          if(cus.bcus_busy[1] == cus.bcus_high_mod)
            Customized_test1(dInd,cus.bcus_col3);
          if(cus.bcus_busy[2] == cus.bcus_high_mod)
            Customized_test2(dInd,cus.bcus_col3);
          if(cus.bcus_busy[3] == cus.bcus_high_mod)
            Customized_test3(dInd,cus.bcus_col3);
          if(cus.bcus_busy[4] == cus.bcus_high_mod)
            Customized_test4(dInd,cus.bcus_col3);
          if(cus.bcus_busy[5] == cus.bcus_high_mod)
            Customized_test5(dInd,cus.bcus_col3);
          if(cus.bcus_busy[6] == cus.bcus_high_mod)
            Customized_test6(dInd,cus.bcus_col3);
          if(cus.bcus_busy[7] == cus.bcus_high_mod)
            Customized_test7(dInd,cus.bcus_col3);
          if(cus.bcus_busy[8] == cus.bcus_high_mod)
            Customized_test8(dInd,cus.bcus_col3);
          if(cus.bcus_busy[9] == cus.bcus_high_mod)
            Customized_test9(dInd,cus.bcus_col3);
     }
     if(cus.bcus_low_mod != 0)//low 
        {
          if(cus.bcus_busy[1] == cus.bcus_low_mod)
            Customized_test1(dInd,cus.bcus_col1);
          if(cus.bcus_busy[2] == cus.bcus_low_mod)
            Customized_test2(dInd,cus.bcus_col1);
          if(cus.bcus_busy[3] == cus.bcus_low_mod)
            Customized_test3(dInd,cus.bcus_col1);
          if(cus.bcus_busy[4] == cus.bcus_low_mod)
            Customized_test4(dInd,cus.bcus_col1);
          if(cus.bcus_busy[5] == cus.bcus_low_mod)
            Customized_test5(dInd,cus.bcus_col1);
          if(cus.bcus_busy[6] == cus.bcus_low_mod)
            Customized_test6(dInd,cus.bcus_col1);
          if(cus.bcus_busy[7] == cus.bcus_low_mod)
            Customized_test7(dInd,cus.bcus_col1);
          if(cus.bcus_busy[8] == cus.bcus_low_mod)
            Customized_test8(dInd,cus.bcus_col1);
          if(cus.bcus_busy[9] == cus.bcus_low_mod)
            Customized_test9(dInd,cus.bcus_col1);
        }
    #endif
}
LOCAL VOID Customize_Theme(UINT16 dInd)
{
#if 1
        if(ThemeColor.r >= 0 && ThemeColor.r <= 9)
        cus.bcus_low_mod = ThemeColor.r;
        if(ThemeColor.g >= 0 && ThemeColor.g <= 9)
        cus.bcus_mid_mod = ThemeColor.g;
        if(ThemeColor.b >= 0 && ThemeColor.b <= 9)
        cus.bcus_high_mod = ThemeColor.b; 
#endif
        cus.bcus_timer_cnt++;
//        cus.bcus_low_mod = 4;
//        cus.bcus_mid_mod = 5;
//        cus.bcus_high_mod = 6;
        
    if ( (AudioDet==FALSE) && (cus.bcus_music_flag == 0))
        Customize_Normal(dInd);
    else
        Customize_Music(dInd);


}
/************************************************************************
 Function:    copyBackground
 Description: copy the background, if any, to the virtual display
 Input:       dInd:  virtual display index
              psInd: particle system index
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID copyBackground(UINT16 dInd, UINT16 psInd)
{
    UINT16 i, j;

    /* Step 1 -- Update background according to bgFlag */
    switch( SimPS[ psInd ].psImage.bgFlag & REN_BG_TYPE_MASK )
    {
        case bgtStatic:
            /* Static background, just copy it */
            virDis[ dInd ] = background[ SimPS[ psInd ].psImage.bgFlag & REN_BG_IND_MASK ];
            break;

        case bgtColorMap:
            /* Calculate background update factor according to prism */
            for( i = 0; i < REN_VIR_DIS_FRAME; i++ )
                for( j = 0; j < REN_VIR_DIS_FRAME; j++ )
                    virDis[ dInd ].virDisColor[ i ][ j ] = colorMap[ SimPS[ psInd ].psImage.bgFlag & REN_BG_IND_MASK ];
            break;

        case bgtEq:
            /* Calculate background update factor according to eq values */
            break;

        case bgtInvalid:
            break;

        case bgtBlack:
        default:
            /* No background or just black, clear the buffer then */
            clearVirBuf( dInd );
            break;                    //Return immedately, no need to copy the background buffer
    }
}

/************************************************************************
 Function:    PreparationEQLevelToLEDLevel
 Description: Prepares Display LED levels based on EQLevel maximum in the set
 Output:      Val: Corresponding LED level of EQ Val              
 Return:      VOID
 Remarks:
************************************************************************/

#if 0
VOID PreparationEQLevelToLEDLevel(UINT8 *Val, UINT8 Index)
{
    UINT8 maxLevel, i, index;
    maxLevel = DbLevelIndexArr[0];
    for (i= 0; i< _AK7755_SP_ANA_BAND_SIZE; i++)
    {
            if (DbLevelIndexArr[i] >= maxLevel)
            {
                maxLevel  = DbLevelIndexArr[i];
                index = i;
            }
    }
        
    for (i= 0; i< _AK7755_SP_ANA_BAND_SIZE; i++)
    {
        if ((i != index) && ((maxLevel-DbLevelIndexArr[i]) < 4))
        {
            DbLevelIndexArr[i] = 9 - (maxLevel-DbLevelIndexArr[i]);
            if (DbLevelIndexArr[i] < 0)
                DbLevelIndexArr[i] = 5;
        }
    }
    if ((maxLevel-DbLevelIndexArr[i]) < 4)
        *Val = DbLevelIndexArr[Index-4];
}
#endif

/************************************************************************
 Function:    MapEQLevelToLEDLevel
 Description: Maps EQLevel value from -3/-6dB to -80/-100dB to corresponding LED level between 2 & 9 on Physical Display
 Output:      Val: Corresponding EQ level in LED              
 Return:      VOID
 Remarks:
************************************************************************/

VOID MapIndexLevelToEQBarLEDLevel(UINT8* Val, UINT8 Index)
{
    INT16 DbVal;
    DbVal = -(GMExInput[ Index ].scalar);
    switch (DbVal)
    {
        case 12: *Val = 9;
                break;
        case 18: *Val = 9;
                break;
        case 24: *Val = 8;
                break;
        case 30: *Val = 7;
                break;
        case 36: *Val = 6;
                break;
        case 42: *Val = 5;
                break;
        case 48: *Val = 4;
                break;                
        case 54: *Val = 3;
                break;                
    }
}

VOID setAudioDetect()
{
    AudioDet = TRUE;
}

VOID setAudioUnDetect()
{
    AudioDet = FALSE;
}
VOID MapEQLevelToRaveLevel( UINT16 *flag, UINT8 Index)
{
    ;
}
#if 0
VOID MapEQLevelToRaveLevel( UINT16 *flag, UINT8 Index)
{
    UINT8 i, dbLevel;


    for (i=0; i<=_AK7755_SP_ANA_VAL_SIZE; )
    {
        if ( GMExInput[ Index ].scalar <= sp_AnaMap[i].dbVal)
            i++;
        else
            break;
    }
    dbLevel = i;
    *flag = dbLevel;
    
    #if 0//ndef NO_DB_VALUE
    switch (dbLevel)
    {
        case 0:

            tRave_Mode_Info.bRave_Random_flag1 =0;
            tRave_Mode_Info.bRave_Random_flag3 =0;
            tRave_Mode_Info.bRave_Reverse=0;

                        break;

        case 1:

            tRave_Mode_Info.bRave_Random_flag1 =1;
            tRave_Mode_Info.bRave_Random_flag3 =1;
            tRave_Mode_Info.bRave_Reverse=1;

            break;

        case 2:

            tRave_Mode_Info.bRave_Random_flag1 =2;
            tRave_Mode_Info.bRave_Random_flag3 =2;
            tRave_Mode_Info.bRave_Reverse=0;
            break;

        case 3:

            tRave_Mode_Info.bRave_Random_flag1 =3;
            tRave_Mode_Info.bRave_Random_flag3 =3;
            tRave_Mode_Info.bRave_Reverse=1;
            break;

        case 4:

            tRave_Mode_Info.bRave_Random_flag1 =4;
            tRave_Mode_Info.bRave_Random_flag3 =4;
            tRave_Mode_Info.bRave_Reverse=0;
            break;

        case 5:

            tRave_Mode_Info.bRave_Random_flag1 =5;
            tRave_Mode_Info.bRave_Random_flag3 =5;
            tRave_Mode_Info.bRave_Reverse=1;

            break;

        case 6:

            tRave_Mode_Info.bRave_Random_flag1 =0;
            tRave_Mode_Info.bRave_Random_flag3 =0;
            tRave_Mode_Info.bRave_Reverse=1;

            break;
            
        case 7:
            
            tRave_Mode_Info.bRave_Random_flag1 =1;
            tRave_Mode_Info.bRave_Random_flag3 =1;
            tRave_Mode_Info.bRave_Reverse=0;
            
            break;

        default:
            break;
    }
    #endif
    
}
#endif
#if 0
void LowMidHighDBChange (UINT8 exflag)
{
    UINT8 a,b,c,d;
    UINT8 i,j,k,dbLevel;
    INT16 tempVal;

    if (exflag == 2)
    {
        k = 0;
    }

    if (exflag == 3)
    {
        k = 4;
    }

    for (i=0; i< 4; i++)
    {

        for (j=0; j< _AK7755_SP_ANA_VAL_SIZE; )
        {
            if ( GMExInput[ i+8+k ].scalar <= sp_AnaMap[j].dbVal )
            {
                if (i==0)
                {
                    a=j;
                }

                if (i==1)
                {
                    b=j;
                }

                if (i==2)
                {
                    c=j;
                }

                if (i==3)
                {
                    d=j;
                }

                j++;
            }
            else
            {
                break;
            }
        }


    }

    if (a<b)
    {
        dbLevel = a;
    }
    else
    {
        dbLevel = b;
    }

    if (dbLevel<c)
    {
        dbLevel = c;
    }

    if (dbLevel<d)
    {
        dbLevel = d;
    }

    switch (dbLevel)
    {
        case 0:

            tRave_Mode_Info.bRave_Random_flag1 =0;
            tRave_Mode_Info.bRave_Random_flag3 =0;
            tRave_Mode_Info.bRave_Reverse=0;

            break;

        case 1:

            tRave_Mode_Info.bRave_Random_flag1 =1;
            tRave_Mode_Info.bRave_Random_flag3 =1;
            tRave_Mode_Info.bRave_Reverse=1;

            break;

        case 2:

            tRave_Mode_Info.bRave_Random_flag1 =2;
            tRave_Mode_Info.bRave_Random_flag3 =2;
            tRave_Mode_Info.bRave_Reverse=0;
            break;

        case 3:

            tRave_Mode_Info.bRave_Random_flag1 =3;
            tRave_Mode_Info.bRave_Random_flag3 =3;
            tRave_Mode_Info.bRave_Reverse=1;
            break;

        case 4:

            tRave_Mode_Info.bRave_Random_flag1 =4;
            tRave_Mode_Info.bRave_Random_flag3 =4;
            tRave_Mode_Info.bRave_Reverse=0;
            break;

        case 5:

            tRave_Mode_Info.bRave_Random_flag1 =5;
            tRave_Mode_Info.bRave_Random_flag3 =5;
            tRave_Mode_Info.bRave_Reverse=1;

            break;

        case 6:

            tRave_Mode_Info.bRave_Random_flag1 =0;
            tRave_Mode_Info.bRave_Random_flag3 =0;
            tRave_Mode_Info.bRave_Reverse=1;

            break;

        default:
            break;
    }


}
#endif
VOID mapVirLocationColor(PVECTOR_ST pos, COLOR_T color, UINT16 dInd)
{
    INT16   vx, vy;                                 /* Mapped pixel coordinates in virtual display */
    COLOR_T wc;
    HSL_T   hsl;

    //used for global lightness control
    rgb2hsl( color, &hsl );
    hsl.lightness = hsl.lightness * masterBrightness / GM_BRIGHTNESS_MAX;
    hsl2rgb( hsl, &wc );

    vy =  pos.y;
    vx =  pos.x;

    virDis[ dInd ].virDisColor[ vx ][ vy ] = wc;
}

PVECTOR_ST mapJetMove(PVECTOR_ST pos, UINT8 Movestate)
{
    switch(Movestate)
    {
        case 1:                
            pos.x = pos.x-1;
            pos.y = pos.y-1;
            break;
        case 2:                
            pos.y = pos.y-1;
            break; 
        case 3:
            pos.x = pos.x+1;
            pos.y = pos.y-1;
            break;
        case 4:
            pos.x = pos.x+1;
            break; 
        case 5:
            pos.x = pos.x+1;
            pos.y = pos.y+1;
            break;
        case 6:
            pos.y = pos.y+1;
            break; 
        case 7:
            pos.x = pos.x-1;
            pos.y = pos.y+1;
            break;
        case 8:
            pos.x = pos.x-1;
            break;                                 
         default:
            break;
    }
    return pos;    
}

UINT8 mapJetMoveEdge(PVECTOR_ST pos, UINT8 Movestate)
{   
    
    if(pos.y == 0x02)
    {
        Movestate = ( UINT8 ) SimRandom ( 5, 7);                      
    }

    if(pos.y == 0x09)
    {
        Movestate = ( UINT8 ) SimRandom ( 1, 3);                      
    }

    if(pos.x == 0x00)
    {
        Movestate = ( UINT8 ) SimRandom ( 3, 5);         
    }

    if(pos.x == 0x0b)
    {
        Movestate = ( UINT8 ) SimRandom ( 6, 8);
        if(Movestate==6)
         {
            Movestate = 1;  
         }
    }

    if ((pos.x == 0x00)&&(pos.y == 0x02))
    {
        Movestate = 5;
    }
    if ((pos.x == 0x0b)&&(pos.y == 0x02))
    {
        Movestate = 7;
    } 
    if ((pos.x == 0x00)&&(pos.y == 0x09))
    {
        Movestate = 3;
    }
    if ((pos.x == 0x0b)&&(pos.y == 0x09))
    {
        Movestate = 1;
    } 
            
    return Movestate;
}

VOID mapJetEQShow(UINT16 dInd,UINT8 cntvel,JET_MODE_PS_INFO_T *pstr, JET_MODE_PSM_INFO_T *psmtr,JET_MODE_PS_TYPE_E ps,BOOL bMoveEnable)
{
    PVECTOR_ST  jet_pos1,jet_pos2,jet_pos3,jet_pos4;
    COLOR_T     jet_cColor1,jet_cColor2,jet_cColor3,jet_cColor4;
    HSL_T       hsl;
    INT16       lightbuf2 = 0x00a3;
    INT16       lightbuf3 = 0x0060;
    INT16       lightbuf4 = 0x001c;
 
    jet_pos1.x = ( INT8 )pstr->bJET_PS_1.x;
    jet_pos1.y = ( INT8 )pstr->bJET_PS_1.y;
    jet_pos2.x = ( INT8 )pstr->bJET_PS_2.x;
    jet_pos2.y = ( INT8 )pstr->bJET_PS_2.y;  
    jet_pos3.x = ( INT8 )pstr->bJET_PS_3.x;
    jet_pos3.y = ( INT8 )pstr->bJET_PS_3.y;
    jet_pos4.x = ( INT8 )pstr->bJET_PS_4.x;
    jet_pos4.y = ( INT8 )pstr->bJET_PS_4.y;

    if((pstr->bJET_PS_1.x == 0x00)&&(pstr->bJET_PS_1.y == 0x00))
    {
        pstr->bJET_PS_1.x = ( INT8 ) SimRandom ( 0, 11);
        pstr->bJET_PS_1.y = ( INT8 ) SimRandom ( 2, 9);
        
        pstr->bJET_Movestate = ( UINT8 ) SimRandom ( 1, 8);
        pstr->bJET_randomflag = ( UINT8 ) SimRandom ( 16, 19);
            
        pstr->bJET_PS_4.x = pstr->bJET_PS_1.x;        
        pstr->bJET_PS_4.y = pstr->bJET_PS_1.y;
        
        pstr->bJET_PS_3.x = pstr->bJET_PS_1.x;
        pstr->bJET_PS_3.y = pstr->bJET_PS_1.y;

        pstr->bJET_PS_2.x = pstr->bJET_PS_1.x;
        pstr->bJET_PS_2.y = pstr->bJET_PS_1.y;            
    }
    
    if(--pstr->ShowCount == 0)
    {
        pstr->bJET_PS_4.x = pstr->bJET_PS_3.x;        
        pstr->bJET_PS_4.y = pstr->bJET_PS_3.y;

        pstr->bJET_PS_3.x = pstr->bJET_PS_2.x;
        pstr->bJET_PS_3.y = pstr->bJET_PS_2.y;

        pstr->bJET_PS_2.x = pstr->bJET_PS_1.x;
        pstr->bJET_PS_2.y = pstr->bJET_PS_1.y;

        if( pstr->bJET_randomflag >= 13) 
        {    
            pstr->bJET_Movestate = mapJetMoveEdge(pstr->bJET_PS_1, pstr->bJET_Movestate);
            if( bMoveEnable == TRUE)
            {
                pstr->bJET_PS_1 = mapJetMove(pstr->bJET_PS_1, pstr->bJET_Movestate);
            }
        }   

        if( pstr->bJET_randomflag == 0)
        {
            pstr->bJET_Movestate = ( UINT8 ) SimRandom ( 1, 8);
            pstr->bJET_randomflag = ( UINT8 ) SimRandom ( 16, 19);            
        }

        if( pstr->bJET_randomflag>0)
          pstr->bJET_randomflag--;

        pstr->ShowCount=1*cntvel;
    }

    rgb2hsl(psmtr->bJET_Color_buf, &hsl );
    if (ps == jet_b)
        hsl.hue += 120;
    if (ps == jet_c)
        hsl.hue += 240;

    if (ps == jet_d)
        hsl.hue += 40;
    if (ps == jet_e)
        hsl.hue += 160;
    if (ps == jet_f)
        hsl.hue += 280;
    
    if(hsl.hue >= 360)
        hsl.hue -= 360;

    hsl2rgb(hsl, &jet_cColor1); 
    
    rgb2hsl(jet_cColor1, &hsl );
    hsl.lightness= lightbuf2;
    hsl2rgb( hsl, &jet_cColor2 );
    hsl.lightness= lightbuf3;
    hsl2rgb( hsl, &jet_cColor3 );
    hsl.lightness= lightbuf4;
    hsl2rgb( hsl, &jet_cColor4 );           
    mapVirLocationColor( jet_pos4, jet_cColor4, dInd ); 
    mapVirLocationColor( jet_pos3, jet_cColor3, dInd ); 
    mapVirLocationColor( jet_pos2, jet_cColor2, dInd ); 
    mapVirLocationColor( jet_pos1, jet_cColor1, dInd );         
    
}

LOCAL BOOL checkMusicLevel()
{
    
    if( (GMExInput[psexi63Hz].scalar    > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi100Hz].scalar   > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi160Hz].scalar   > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi250Hz].scalar   > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi400Hz].scalar   > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi630Hz].scalar   > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi1000Hz].scalar  > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi2000Hz].scalar  > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi4000Hz].scalar  > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi6300Hz].scalar  > PULSE3_EQ_THRESHOLD)  ||
        (GMExInput[psexi10000Hz].scalar  > PULSE3_EQ_THRESHOLD) ||
        (GMExInput[psexi16000Hz].scalar  > PULSE3_EQ_THRESHOLD)        
      )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

LOCAL VOID FireFrameProcess(UINT16 disInd, UINT16 psInd)
{
    signed int x, y;
    COLOR_T maskColor, disColor;
    HSL_T   hsl;
    
    maskColor = colorMap[ 0 ];
    rgb2hsl(maskColor, &hsl);
    for(y = 2; y <= 9; y++)
    {
        hsl2rgb(hsl, &maskColor);
        for(x = 0; x < REN_PHY_FRAME_W; x++)
        {
            disColor = virDis[ disInd ].virDisColor[ x ][ y ];
            AlphaColor(maskColor, disColor, 96, &disColor);
            virDis[ disInd ].virDisColor[ x ][ y ] = disColor;
        }

        hsl.hue -= 9;
        if(y > 3)
            hsl.lightness -= 40;
        
        if(hsl.lightness < 0)
            hsl.lightness = 0;
        if(hsl.hue < 0)
            hsl.hue += 360;
    }
}

LOCAL VOID FrameProcess(UINT16 disInd, UINT16 psInd)
{    
    if(psFire == SimPS[ psInd ].psBase.id)
    {
        FireFrameProcess(disInd, psInd);
    }
}

/************************************************************************
 Function:    mapColor
 Description: Map the color of particles in a particle system to
              a virtual display
 Input:       dInd:  virtual display index
              psInd: particle system index
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID mapColor(UINT16 dInd, UINT16 psInd)
{
    INT16       i, ax, ay, ch, cl;
    INT16       sr, vx, vy, sx, sy;
    PARTICLE_ST *pPtr;
    PVECTOR_ST  pos, vel;
    COLOR_T     cColor, color;
    HSL_T       hsl;
//    BOOL        scFlag;
    UINT8       *dPtr;
    UINT8       j;
    const UINT8 uJetEqSpeed = 12;
    static UINT8 k =0;    
    LOCAL UINT8 DetCount = 8;
    LOCAL UINT8 LowDetCount = 10;
    LOCAL UINT8 WaitDetCount = 8;
    BOOL        bLowDect, bMidDect, bHiDect;
    INT16       nHueDiff;
    INT16       rPos;
    COLOR_T     tmpC;
    
    /* Calculate particle aspect ratio */
    if( SimPS[ psInd ].psBase.sizeAspect == SIM_SA_Y_INFINITE )
    {
        /* Only determine by y value and valid for all x */
        ax = 0;
        ay = 240;        /* Put down a value big enough so that only vy = 0 can be shown */
    }
    else if( SimPS[ psInd ].psBase.sizeAspect == SIM_SA_X_INFINITE )
    {
        /* Only determine by x value and valid for all y */
        ax = 240;        /* Put down a value big enough so that only vx = 0 can be shown */
        ay = 0;
    }
    else
    {
        if( SimPS[ psInd ].psBase.sizeAspect > 0 )
        {
            ax = ( UINT16 )SimPS[ psInd ].psBase.sizeAspect;
            ay = 1;
        }
        else
        {
            ax = 1;
            ay = ( UINT16 )SimPS[ psInd ].psBase.sizeAspect;
        }
    }

    /* Update particle color */
    pPtr = &SimP[ 0 ];
    while( pPtr < &SimP[ SimPMax ] )
    {
        if( ( pPtr -> layer == psInd ) && ( pPtr -> systemID != SIM_PARTICLE_INVALID ) )
        {
            /* It is a particle belongs to the particle system, handle it */
            /* Get the particle color first */
            cColor = pPtr -> color;
            if( SimPS[ psInd ].psImage.ucNum > 0 )
            {
                for( i = 0; i < SimPS[ psInd ].psImage.ucNum; i++ )
                {
                    /* Update color for each defined process one by one */
                    updateColor( cColor, SimPS[ psInd ].psImage.updateColorInd + i, pPtr, psInd, &cColor );
                }
            }
            if(SimPS[ psInd ].psBase.id == psFire)
            {
                rgb2hsl(cColor, &hsl);
                if(pPtr->location.y >= -40)
                {
                    nHueDiff = ((pPtr->location.y + 40) * 7 / 8);
                    if(nHueDiff > 36)
                        nHueDiff = 36;
                    hsl.hue -= nHueDiff;
                }
                if(pPtr->location.y >= 0)
                {
                    hsl.lightness -= ((pPtr->location.y) * 2);
                }
                if(pPtr->location.y >= 32)
                {
                    hsl.lightness -= ((pPtr->location.y-32));
                    hsl.saturation += ((pPtr->location.y - 32)*3);
                }
                if(hsl.hue < 0)
                    hsl.hue += 360;
                if(hsl.lightness < 0)
                    hsl.lightness = 0;
                if(hsl.saturation > 512)
                    hsl.saturation = 512;
                hsl2rgb(hsl, &cColor);
            }

            /* Handle size */
            sr = ( ( pPtr -> size / REN_FRAME2VIR_FRAME ) - 1 ) >> 1;
            if( sr < 0 )
                sr = 0;

            vel.x = 0;      /* Default velocity to zero first */
            vel.y = 0;

            switch( SimPS[ psInd ].psImage.shapeFlag & REN_SHAPE_TYPE_MASK )
            {
                case stExplosion:
                    DrawExplosion(pPtr, cColor, dInd, psInd);
                    break;
                case stBar:
                    DrawEQBar(dInd);
                    break;
                case stRainbow:
                    DrawRainBow(dInd);
                    break;

                case stCustomized:
                    Customize_Theme(dInd);  
                    break;
                case stRave:
                    {
                        vel.x = 0;
                        vel.y = 0;
                        //pPtr -> aAmplitude = ( UINT8 )( SIM128_TO_VIR96 >> 5 );
                        //vx = ( SIM128_TO_VIR96 ) ;//- REN_FRAME2VIR_FRAME; //<< 2;//8 tiao
                        vx = 26; //SIM128_TO_VIR96*2;//( SIM128_TO_VIR96 )- REN_FRAME2VIR_FRAME;//9 tiao
#if 1
                        //if ( checkMusicLevel()==FALSE )
                        if (AudioDet == FALSE) {
                            // pPtr = &SimP[ 0 ];
                            cColor = tRave_Mode_Info.tRave_Color_buf; //pPtr->color;
                            rgb2hsl(cColor, &hsl);
                            //ch = hsl.hue;
                            //cl = hsl.lightness;

                            hsl.lightness = wRaveBreath[tRave_Mode_Info.bRave_Breath_Cnt];

                            // hsl.lightness = i;     /* Update to cColor for updating to particle */
                            hsl2rgb(hsl, &cColor);

                            tRave_Mode_Info.bRave_Breath_Cnt++;

                            if (tRave_Mode_Info.bRave_Breath_Cnt == 105) {
                                tRave_Mode_Info.bRave_Breath_Cnt = 0;
                                tRave_Mode_Info.bRave_st = 0;
                                tRave_Mode_Info.bRave_flag = (UINT8) SimRandom(0, 2);
                                tRave_Mode_Info.bRave_Random_flag1 = (UINT8) SimRandom(0, 5); // 6+10//30;
                                tRave_Mode_Info.bRave_Random_flag2 = (UINT8) SimRandom(0, 9);
                                tRave_Mode_Info.bRave_Random_flag3 = (UINT8) SimRandom(0, 5);

                                if (APP_COLOR != ThemeColorMode) {
                                    cColor = tRave_Mode_Info.tRave_Color_buf; //pPtr->color;
                                    rgb2hsl(cColor, &hsl);

                                    hsl.hue = (hsl.hue) + 10;

                                    hsl2rgb(hsl, &cColor);
                                    tRave_Mode_Info.tRave_Color_buf = cColor;
                                }

                                // bRave_EXflag = ( UINT8 ) SimRandom( 0, 5 );

                                while (tRave_Mode_Info.bRave_Random_Check1 == tRave_Mode_Info.bRave_Random_flag1) {
                                    tRave_Mode_Info.bRave_Random_flag1 = (UINT8) SimRandom(0, 5);
                                }
                                tRave_Mode_Info.bRave_Random_Check1 = tRave_Mode_Info.bRave_Random_flag1;

                                while (tRave_Mode_Info.bRave_Random_Check2 == tRave_Mode_Info.bRave_Random_flag2) {
                                    tRave_Mode_Info.bRave_Random_flag2 = (UINT8) SimRandom(0, 9);
                                }
                                tRave_Mode_Info.bRave_Random_Check2 = tRave_Mode_Info.bRave_Random_flag2;

                                while (tRave_Mode_Info.bRave_Random_Check3 == tRave_Mode_Info.bRave_Random_flag3) {
                                    tRave_Mode_Info.bRave_Random_flag3 = (UINT8) SimRandom(0, 5);
                                }
                                tRave_Mode_Info.bRave_Random_Check3 = tRave_Mode_Info.bRave_Random_flag3;

                                if (tRave_Mode_Info.bRave_Reverse == 0) {
                                    tRave_Mode_Info.bRave_Reverse = 1;
                                } else {
                                    tRave_Mode_Info.bRave_Reverse = 0;
                                }
                                break;
                            }

                            if (tRave_Mode_Info.bRave_flag == 0) //mid
                            {
                                for (j = 0; j < 16; j++) {

                                    pos.x = (INT8) vx + (64 - 16 * tRave_Mode_Info.bRave_Random_flag1);

                                    for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                    {
                                        pos.y = (INT8) (-vy);
                                        mapLocationColor(pos, cColor, dInd, psInd, vel);
                                    }

                                    pos.x = (INT8) vx + (-32 - 16 * tRave_Mode_Info.bRave_Random_flag1);

                                    for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                    {
                                        pos.y = (INT8) (-vy);
                                        mapLocationColor(pos, cColor, dInd, psInd, vel);
                                    }

                                    //vx -= 16;
                                    pPtr++;
                                }
                            }

                            if (tRave_Mode_Info.bRave_flag == 1) {
                                for (j = 0; j < 16; j++) {

                                    pos.x = (INT8) vx + (64 - 16 * tRave_Mode_Info.bRave_Random_flag2);
                                    for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                    {
                                        pos.y = (INT8) (-vy);
                                        mapLocationColor(pos, cColor, dInd, psInd, vel);
                                    }

                                    pos.x = (INT8) vx + (48 - 16 * tRave_Mode_Info.bRave_Random_flag2);

                                    for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                    {
                                        pos.y = (INT8) (-vy);
                                        mapLocationColor(pos, cColor, dInd, psInd, vel);
                                    }

                                    pos.x = (INT8) vx + (32 - 16 * tRave_Mode_Info.bRave_Random_flag2);

                                    for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                    {
                                        pos.y = (INT8) (-vy);
                                        mapLocationColor(pos, cColor, dInd, psInd, vel);
                                    }

                                    //vx -= 16;
                                    pPtr++;
                                }
                            }

                            if (tRave_Mode_Info.bRave_flag == 2) //high
                            {
                                if (tRave_Mode_Info.bRave_Reverse == 0) {
                                    //pPtr = &SimP[ 0 ];

                                    for (j = 0; j < 16; j++) {
                                        pos.x = (INT8) vx;

                                        // cColor = colorMap[ j ];
                                        //for ( vy = - ( (REN_SIM_FRAME/2)-48); vy < REN_SIM_FRAME/2; vy += 64) //REN_FRAME2VIR_FRAME )
                                        {
                                            pos.y = (-64 + 16 * tRave_Mode_Info.bRave_Random_flag3); //( INT8 ) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                            //vy=vy+16;
                                            pos.y = (-48 + 16 * tRave_Mode_Info.bRave_Random_flag3); //( INT8 ) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                            pos.y = (-32 + 16 * tRave_Mode_Info.bRave_Random_flag3); //( INT8 ) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                        }

                                        vx -= 16;
                                        pPtr++;
                                    }
                                }
                                if (tRave_Mode_Info.bRave_Reverse == 1) {
                                    //pPtr = &SimP[ 0 ];

                                    for (j = 0; j < 16; j++) {
                                        pos.x = (INT8) vx;

                                        // cColor = colorMap[ j ];
                                        for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 80) //REN_FRAME2VIR_FRAME )
                                        {
                                            pos.y = (INT8) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                            vy = vy + 16;
                                            pos.y = (INT8) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                        }

                                        vx -= 16;
                                        pPtr++;
                                    }
                                }
                            }

                        } else
#endif
#if 1 // ELSE PART
                        {
#ifdef LOW_MID_HIGH_TRANSITION_PROCESS
                           if (--WaitDetCount == 0) {
                            for (j = 0; j < 12; j++) {
                                cColor = pPtr->color;
                                cColor.r = 255;
                                cColor.g = 0;
                                cColor.b = 0;
                                switch (j) {
                                    case 0:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 8); //mid
                                        if ((Val < 6)) //mid
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 2;
                                        }
                                        break;
                                    case 1:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 9);
                                        if ((Val < 4)) //mid
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 2;
                                        }
                                        break;
                                    case 2:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 10); //1//1~4
                                        if ((Val < 4)) //mid
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 2;
                                        }
                                        break;
                                    case 3:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 11);
                                        if ((Val < 3)) //mid
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 2;
                                        }
                                        break;
                                    case 4:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 4); //low
                                        if ((Val < 4)) //low
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 1;
                                        }

                                        break;
                                    case 5:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 5);
                                        if ((Val < 4)) //low
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 1;
                                        }

                                        break;
                                    case 6:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 6);
                                        if ((Val < 4)) //low
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 1;
                                        }
                                        break;
                                    case 7:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 7);
                                        if ((Val < 3)) //low
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 1;
                                        }
                                        break;
                                    case 8:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 12); //high
                                        if ((Val < 6))//high
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 4;
                                        }
                                        break;
                                    case 9:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 13);
                                        if ((Val < 4))//high
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 3;
                                        }
                                        break;
                                    case 10:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 14);
                                        if ((Val < 4))//high
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 3;
                                        }
                                        break;
                                    case 11:
                                        MapEQLevelToRaveLevel(&Val, (UINT8) 15);
                                        if ((Val < 4))//high
                                        {
                                            tRave_Mode_Info.bRave_EXflag = 3;
                                        }
                                        break;

                                    default:
                                        break;
                                }
                            }
                            WaitDetCount = 8;
                           }
#endif
#ifndef LOW_MID_HIGH_TRANSITION_PROCESS
                            cColor = pPtr->color;
                            cColor.r = 255;
                            cColor.g = 0;
                            cColor.b = 0;

                            //tRave_Mode_Info.bRave_EXflag=DetectLowMidHigh();
                            if (--WaitDetCount == 0) {
                                tRave_Mode_Info.bRave_EXflag = 1;//RaveDetectLowMidHigh();
                                if (tRave_Mode_Info.bRave_EXflag&0x01) //low
                                {
                                   WaitDetCount = 5;
                                }
                                else
                                {
                                  WaitDetCount = 4;
                                }
                            }
#ifndef NO_DB_VALUE
                           ;// LowMidHighDBChange(tRave_Mode_Info.bRave_EXflag);
#endif
#endif
#ifdef NO_DB_VALUE
                            if (--DetCount == 0) {
                                DetCount = 8;
                                tRave_Mode_Info.bRave_Random_flag1 = (UINT8) SimRandom(0, 5); // 6+10//30;
                                tRave_Mode_Info.bRave_Random_flag3 = (UINT8) SimRandom(0, 5);

                                while (tRave_Mode_Info.bRave_Random_Check1 == tRave_Mode_Info.bRave_Random_flag1) {
                                    tRave_Mode_Info.bRave_Random_flag1 = (UINT8) SimRandom(0, 5);
                                }
                                tRave_Mode_Info.bRave_Random_Check1 = tRave_Mode_Info.bRave_Random_flag1;


                                while (tRave_Mode_Info.bRave_Random_Check3 == tRave_Mode_Info.bRave_Random_flag3) {
                                    tRave_Mode_Info.bRave_Random_flag3 = (UINT8) SimRandom(0, 5);
                                }
                                tRave_Mode_Info.bRave_Random_Check3 = tRave_Mode_Info.bRave_Random_flag3;

                                if (tRave_Mode_Info.bRave_Reverse == 0) {
                                    tRave_Mode_Info.bRave_Reverse = 1;
                                } else {
                                    tRave_Mode_Info.bRave_Reverse = 0;
                                }
                            }
#endif
                            //if (tRave_Mode_Info.bRave_EXflag == 1) //low
                            if (tRave_Mode_Info.bRave_EXflag&0x01) //low
                            {
                                cColor = tRave_Mode_Info.tRave_Color_buf; //pPtr->color;
                                //tRave_Mode_Info.bRave_Low_to_Mid_Cnt = 60;
                                rgb2hsl(cColor, &hsl);
                                //add count for 10
#if 1
                                if (--LowDetCount == 0) {
                                    //if(APP_COLOR != ThemeColorMode)
                                    {
                                        hsl.hue = (hsl.hue) + 40;
                                    }
                                    LowDetCount = 20;
                                }
#endif
                                hsl2rgb(hsl, &cColor);
                                tRave_Mode_Info.tRave_Color_buf = cColor;

                                {
                                    for (k = 0; k < 16; k++) {
                                        pos.x = (INT8) vx;

                                        for (vy = -(REN_SIM_FRAME / 2); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                        {
                                            pos.y = (INT8) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                        }

                                        vx -= 16;
                                        pPtr++;
                                    }
                                }
                            }

                            //if ((tRave_Mode_Info.bRave_EXflag == 2) || (tRave_Mode_Info.bRave_EXflag == 4)) //mid
                            if (tRave_Mode_Info.bRave_EXflag&0x02)  //mid
                            {
                                cColor = tRave_Mode_Info.tRave_Color_buf;
                               #if 0
                                if (tRave_Mode_Info.bRave_Low_to_Mid_Cnt != 0) {
                                    rgb2hsl(cColor, &hsl);
                                    hsl.hue = (hsl.hue) + 40;
                                    hsl2rgb(hsl, &cColor);
                                    for (k = 0; k < 16; k++) {
                                        pos.x = (INT8) vx;

                                        for (vy = -(REN_SIM_FRAME / 2); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                        {
                                            pos.y = (INT8) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                        }

                                        vx -= 16;
                                        pPtr++;
                                    }
                                    tRave_Mode_Info.bRave_Low_to_Mid_Cnt--;
                                }
                                
                                cColor = tRave_Mode_Info.tRave_Color_buf;
                                #endif

                                for (j = 0; j < 16; j++) {

                                    pos.x = (INT8) vx + (64 - 16 * tRave_Mode_Info.bRave_Random_flag1);

                                    for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                    {
                                        pos.y = (INT8) (-vy);
                                        mapLocationColor(pos, cColor, dInd, psInd, vel);
                                    }

                                    pos.x = (INT8) vx + (-32 - 16 * tRave_Mode_Info.bRave_Random_flag1);

                                    for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 16) //REN_FRAME2VIR_FRAME )
                                    {
                                        pos.y = (INT8) (-vy);
                                        mapLocationColor(pos, cColor, dInd, psInd, vel);
                                    }

                                    //vx -= 16;
                                    pPtr++;
                                }
                            }

                            //if ((tRave_Mode_Info.bRave_EXflag == 3) || (tRave_Mode_Info.bRave_EXflag == 4))//high
                            if (tRave_Mode_Info.bRave_EXflag&0x04)  //high
                            {
                                cColor = tRave_Mode_Info.tRave_Color_buf;
                                #if 0
                                rgb2hsl(cColor, &hsl);
                                if (--LowDetCount == 0) {
                                    if (APP_COLOR != ThemeColorMode) {
                                        hsl.hue = (hsl.hue) + 40;
                                    }
                                    LowDetCount = 10;
                                }
                                hsl2rgb(hsl, &cColor);
                                tRave_Mode_Info.tRave_Color_buf = cColor;
                                #endif
                                //tRave_Mode_Info.bRave_Low_to_Mid_Cnt = 0;
                                if (tRave_Mode_Info.bRave_Reverse == 0) {
                                    //pPtr = &SimP[ 0 ];

                                    for (j = 0; j < 16; j++) {
                                        pos.x = (INT8) vx;

                                        // cColor = colorMap[ j ];
                                        //for ( vy = - ( (REN_SIM_FRAME/2)-48); vy < REN_SIM_FRAME/2; vy += 64) //REN_FRAME2VIR_FRAME )
                                        {
                                            pos.y = (-64 + 16 * tRave_Mode_Info.bRave_Random_flag3); //( INT8 ) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                            //vy=vy+16;
                                            pos.y = (-48 + 16 * tRave_Mode_Info.bRave_Random_flag3); //( INT8 ) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                            pos.y = (-32 + 16 * tRave_Mode_Info.bRave_Random_flag3); //( INT8 ) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                        }

                                        vx -= 16;
                                        pPtr++;
                                    }
                                }
                                if (tRave_Mode_Info.bRave_Reverse == 1) {
                                    //pPtr = &SimP[ 0 ];

                                    for (j = 0; j < 16; j++) {
                                        pos.x = (INT8) vx;

                                        // cColor = colorMap[ j ];
                                        for (vy = -((REN_SIM_FRAME / 2) + 16); vy < REN_SIM_FRAME / 2; vy += 80) //REN_FRAME2VIR_FRAME )
                                        {
                                            pos.y = (INT8) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                            vy = vy + 16;
                                            pos.y = (INT8) (-vy);
                                            mapLocationColor(pos, cColor, dInd, psInd, vel);
                                        }

                                        vx -= 16;
                                        pPtr++;
                                    }
                                }
                            }
                        }
#endif
                    }
                    break;

                case stFirework:
                    /* Special treatment with defined data */
                    rgb2hsl( cColor, &hsl );
                    cl = hsl.lightness;

                    /* Use angle as a timer to count the life of the particle */
                    if( pPtr -> angle < 12 )
                    {
                        /* Stage 1/2: center particle and 4 corner die down together */
                        for( vx = -1; vx < 2; vx++ )
                        {
                            sx = vx * REN_FRAME2VIR_FRAME + pPtr -> location.x;
                            pos.x = ( INT8 )sx;
                            for( vy = -1; vy < 2 ; vy++ )
                            {
                                if( ( vx == 0 ) && ( vy == 0 ) )
                                {
                                    hsl.lightness = cl - pPtr -> aAmplitude * 4 * pPtr -> angle;       /* aAmplitude as delta lightness */
                                }
                                else if( ( vx == 0 ) || ( vy == 0 ) )
                                {
                                    /* Keep original lightness at 4 corners */
                                    hsl.lightness = cl - pPtr -> aAmplitude * 2 * ( pPtr -> angle );
                                }
                                else
                                    hsl.lightness = 0;
                                hsl2rgb( hsl, &color );

                                sy = vy * REN_FRAME2VIR_FRAME + pPtr -> location.y;
                                pos.y = ( INT8 )sy;
                                mapLocationColor( pos, color, dInd, psInd, vel );
                            }
                        }
                    }

                    if( pPtr -> angle > 7 )
                    {
                        /* Stage 3: Explode out */
                        /* Prepare random exploded list */
                        for( vx = 0; vx < FIREWORK_EXPLODE_NUM; vx++ )
                        {
                            fireworkExplodeList[ vx ] = ( UINT8 )SimRandom( 0, 8 );
                            if( fireworkExplodeList[ vx ] == 4 )
                            {
                                vx--;
                                continue;
                            }

                            for( vy = 0; vy < vx; vy++ )
                            {
                                if( fireworkExplodeList[ vy ] == fireworkExplodeList[ vx ] )
                                {
                                    /* Duplicated number, retry it */
                                    vx--;
                                    break;
                                }
                            }
                        }

                        /* Show random particles */
                        for( vx = 0; vx < FIREWORK_EXPLODE_NUM; vx++ )
                        {
                            sx = ( fireworkExplodeList[ vx ] % 3 - 1 ) * REN_FRAME2VIR_FRAME + pPtr -> location.x;
                            sy = ( fireworkExplodeList[ vx ] / 3 - 1 ) * REN_FRAME2VIR_FRAME + pPtr -> location.y;
                            pos.x = ( INT8 )sx;
                            pos.y = ( INT8 )sy;

                            i = cl - ( pPtr -> aAmplitude * pPtr -> angle ) - 56;
                            if( i > 0 )
                                i = SimRandom( 0, i );
                            hsl.lightness = i;
                            hsl2rgb( hsl, &color );
                            mapLocationColor( pos, color, dInd, psInd, vel );
                        }
                    }
                    break;

                case stFixedShape:
                    /* Special treatment with defined data */
                    rgb2hsl( cColor, &hsl );
                    cl = hsl.lightness;

                    switch( sr )
                    {
                        case 1:
                            dPtr = ( UINT8 * )&datHeart1[ 0 ];
                            break;

                        case 2:
                            dPtr = ( UINT8 * )&datHeart2[ 0 ];
                            break;

                        case 3:
                        default: /* sr = 0 is meaningless, just assign an arbitrary pointer */
                            dPtr = ( UINT8 * )&datHeart3[ 0 ];
                            break;
                    }
                    for( vx = -sr; vx <= sr; vx++ )
                        for( vy = -sr; vy <= sr; vy++ )
                        {
                            if( sr > 0 )
                            {
                                hsl.lightness = ( cl * ( *( dPtr + ( ( sr - vy ) * ( sr * 2 + 1 ) + ( sr + vx ) ) ) ) ) >> 4;
                                hsl2rgb( hsl, &color );
                            }
                            else
                                color = cColor;

                            sx = vx * REN_FRAME2VIR_FRAME + pPtr -> location.x;
                            sy = vy * REN_FRAME2VIR_FRAME + pPtr -> location.y;
                            if( ( sx < REN_SIM_FRAME/2 ) && ( sx > -REN_SIM_FRAME/2 ) &&
                                ( sy < REN_SIM_FRAME/2 ) && ( sy > -REN_SIM_FRAME/2 ) )
                            {
                                /* Avoid overflow of coordinates */
                                pos.x = ( INT8 )sx;
                                pos.y = ( INT8 )sy;
                                mapLocationColor( pos, color, dInd, psInd, vel );
                            }
                        }
                    break;

//                case stTriangle:
//                    dPtr = ( UINT8 * )&datTriangle[ 0 ];
//                    break;


                case stWave:
                    {
                        color = colorMap[psInd];
                        if(DEFAULT_COLOR == ThemeColorMode)
                        {
                            rgb2hsl(color, &hsl);
                            hsl.hue += SimPS[ psInd ].psBase.cohesion;
                            if(hsl.hue >= 360)
                                hsl.hue -= 360;
                            hsl2rgb(hsl, &color);
                        }

                        pos.x = pPtr -> location.x;
                        vy =( INT16 )pPtr -> location.y;

                        //add smooth pixel
                        rPos = (vy + SIM128_TO_VIR96) % REN_FRAME2VIR_FRAME;
                        if(rPos && psInd > 0)
                        {
                            //get lower layer color as background
                            tmpC = colorMap[psInd - 1];

                            if(0 != (tmpC.r | tmpC.g | tmpC.b))
                            {
                                if(DEFAULT_COLOR == ThemeColorMode)
                                {
                                    rgb2hsl(tmpC, &hsl);
                                    hsl.hue += SimPS[ psInd ].psBase.cohesion;
                                    if(hsl.hue >= 360)
                                        hsl.hue -= 360;
                                    hsl2rgb(hsl, &tmpC);
                                }

                                AlphaColor(color, tmpC, rPos << 4, &tmpC);

                                pos.y = vy + REN_FRAME2VIR_FRAME - rPos;
                                mapLocationColor( pos, tmpC, dInd, psInd, vel );
                            }
                        }

                        for( ; vy > -REN_SIM_FRAME/2; vy -= REN_FRAME2VIR_FRAME )
                        {
                            pos.y = ( INT8 )vy;
                            mapLocationColor( pos, color, dInd, psInd, vel );
                        }
                    }
                    break;

                case stDotTrack:
                    if (AudioDet == TRUE)
                    {
                        PARTICLE_SYSTEM_ST *psPtr = &SimPS[psInd];

                        if(TRUE == tJet_Mode_PSM_Info.bResetData)
                        {
                            memset(&tJet_Mode_PS_A, 0, sizeof(JET_MODE_PS_INFO_T));
                            memset(&tJet_Mode_PS_B, 0, sizeof(JET_MODE_PS_INFO_T));
                            memset(&tJet_Mode_PS_C, 0, sizeof(JET_MODE_PS_INFO_T));
                            memset(&tJet_Mode_PS_D, 0, sizeof(JET_MODE_PS_INFO_T));
                            memset(&tJet_Mode_PS_E, 0, sizeof(JET_MODE_PS_INFO_T));
                            memset(&tJet_Mode_PS_F, 0, sizeof(JET_MODE_PS_INFO_T));
                            tJet_Mode_PSM_Info.bResetData = FALSE;
                        }
                        
                        tJet_Mode_PSM_Info.bJET_music_EXflag = 1;//JetDetectLowMidHigh();

                        bLowDect = ((tJet_Mode_PSM_Info.bJET_music_EXflag & 0x01) ? TRUE : FALSE);
                        bMidDect = ((tJet_Mode_PSM_Info.bJET_music_EXflag & 0x02) ? TRUE : FALSE);
                        bHiDect  = ((tJet_Mode_PSM_Info.bJET_music_EXflag & 0x04) ? TRUE : FALSE);

                        mapJetEQShow(dInd, uJetEqSpeed, &tJet_Mode_PS_A, &tJet_Mode_PSM_Info, jet_a, bLowDect);
                        mapJetEQShow(dInd, uJetEqSpeed, &tJet_Mode_PS_B, &tJet_Mode_PSM_Info, jet_b, bMidDect);
                        mapJetEQShow(dInd, uJetEqSpeed, &tJet_Mode_PS_C, &tJet_Mode_PSM_Info, jet_c, bHiDect);

                        mapJetEQShow(dInd, uJetEqSpeed, &tJet_Mode_PS_D, &tJet_Mode_PSM_Info, jet_d, bLowDect);
                        mapJetEQShow(dInd, uJetEqSpeed, &tJet_Mode_PS_E, &tJet_Mode_PSM_Info, jet_e, bMidDect);
                        mapJetEQShow(dInd, uJetEqSpeed, &tJet_Mode_PS_F, &tJet_Mode_PSM_Info, jet_f, bHiDect);
                    }
                    else
                    {
                        const int LOC_TOLERANCE = REN_FRAME2VIR_FRAME/2;
                        const int TRACK_LEN     = 1;
                        const int LIGHT_COEFF   = REN_FRAME2VIR_FRAME * 3 / 4;
                        int nPosSteps;
                        int nDestColumn;
                        int nLocDist;
                        int nLoopCount;
                        int rPos = 0;
                        
                        rgb2hsl( cColor, &hsl );
                        cl = hsl.lightness;
                        pos.y = pPtr->location.y;
                        vx =( INT16 )pPtr -> location.x;
                        vel = pPtr->velocity;
                        if(pPtr->velocity.x > 0)
                        {
                            nPosSteps = -REN_FRAME2VIR_FRAME;
                            nDestColumn = SimPS[ psInd ].psBase.locMin.vector.x;
                            nLocDist  = vx - nDestColumn + REN_FRAME2VIR_FRAME;
                        }
                        else
                        {
                            nPosSteps = REN_FRAME2VIR_FRAME;
                            nDestColumn = SimPS[ psInd ].psBase.locMax.vector.x;
                            nLocDist  = nDestColumn - vx + REN_FRAME2VIR_FRAME;
                        }
                        
                        rPos = (vx + SIM128_TO_VIR96 + REN_FRAME2VIR_FRAME*3) %  REN_FRAME2VIR_FRAME;
                        //put head if necessary
                        {
                            if(pPtr->velocity.x < 0)
                            {
                                {
                                    hsl.lightness = cl * (REN_FRAME2VIR_FRAME - rPos - 1) / REN_FRAME2VIR_FRAME;
                                    hsl2rgb(hsl, &color);
                                    pos.x = ( INT8 )(vx - 0);
                                    mapNormalJetThemeColor( pos, color, dInd, psInd, vel );
                                }
                                vx = vx + REN_FRAME2VIR_FRAME;
                            }

                            if(pPtr->velocity.x > 0)
                            {
                                //if( rPos <= REN_FRAME2VIR_FRAME/2 )
                                {
                                    hsl.lightness = cl * (rPos) / REN_FRAME2VIR_FRAME;
                                    hsl2rgb(hsl, &color);
                                    pos.x = ( INT8 )(vx + REN_FRAME2VIR_FRAME);
                                    mapNormalJetThemeColor( pos, color, dInd, psInd, vel );
                                }
                            }
                        }
                        //put body
                        for( nLoopCount = 0; 
                             nLoopCount < TRACK_LEN; 
                             vx += nPosSteps, nLocDist -= REN_FRAME2VIR_FRAME, nLoopCount++)
                        {
                            pos.x = ( INT8 )(vx);
                            hsl.lightness = cl;
                            hsl2rgb(hsl, &color);
                            mapNormalJetThemeColor( pos, color, dInd, psInd, vel );
                            //cl = cl / 2;
                        }
                        //put tail if necessary
                        {
                            if(pPtr->velocity.x > 0)
                            {
                                hsl.lightness = cl * (REN_FRAME2VIR_FRAME - rPos -1) / REN_FRAME2VIR_FRAME;
                                hsl2rgb(hsl, &color);
                                pos.x = ( INT8 )vx;
                                mapNormalJetThemeColor( pos, color, dInd, psInd, vel );
                            }
                            if(pPtr->velocity.x < 0)
                            {
                                hsl.lightness = cl * (rPos) / REN_FRAME2VIR_FRAME;
                                hsl2rgb(hsl, &color);
                                pos.x = ( INT8 )(vx);
                                mapNormalJetThemeColor( pos, color, dInd, psInd, vel );
                            }
                        }
                    }
                    break;
                case stHGStack:
                    /* Update stack location, special treatment */
                    /* Use angle as counter to time, update location upon angle=0 */
                    /* Use size as the pause condition */
                    /* To pause the hourglass, set size > 1 */
                    if( pPtr -> size > 0 )
                    {
                        if( pPtr -> maxSpeed > 0 )
                            pPtr -> maxSpeed--;
                        else
                        {
                            /* Update location */
                            pPtr -> location.y++;
                            pPtr -> maxSpeed = SimPS[ psInd ].psBase.maxSpeedMin;
                        }
                    }

                    /* Check sand drop location */
                    for( i = 0; i < SimPMax; i++ )
                    {
                        /* Get the sand drop particle */
                        if( SimP[ i ].systemID == psHourglass )
                            break;
                    }

                    if( ( SimP[ i ].location.y <= pPtr -> location.y ) &&
                        ( SimP[ i ].location.y >= ( pPtr -> location.y - REN_FRAME2VIR_FRAME ) ) && ( i < SimPMax ) )
                    {
                        /* The sand drop has already reached the stack, but not too far away, update the stack */
                        pPtr -> aAmplitude = ( UINT8 )( pPtr -> location.y + SIM128_TO_VIR96 );
                    }
                    else
                    {
                        /* Keep showing the last updated stack*/
                    }
                    vy = ( pPtr -> aAmplitude - SIM128_TO_VIR96 ) % REN_FRAME2VIR_FRAME;

                    /* The top one varies with intensity */
                    //vy = pPtr -> location.y % REN_FRAME2VIR_FRAME;
                    if( vy < 0 )
                        vy = REN_FRAME2VIR_FRAME + vy;
                    if( vy > 0 )
                    {
                        /* The top one with varied intensity */
                        rgb2hsl( cColor, &hsl );
                        cl = hsl.lightness;
                        i = ( vy * 11 ) >> 1;        /* Convert to the horizontal distance from the center */

                        pos.y = pPtr -> aAmplitude - SIM128_TO_VIR96;
                        for( vx = 0; vx < REN_SIM_FRAME / 2; vx += REN_FRAME2VIR_FRAME )
                        {
                            if( vx == 0 )
                            {
                                if( i < 8 )
                                {
                                    hsl.lightness = ( cl * i ) >> 3;
                                    i = 0;
                                }
                                else
                                {
                                    hsl.lightness = cl;
                                    i -= 8;
                                }
                            }
                            else
                            {
                                if( i < 16 )
                                {
                                    hsl.lightness = ( cl * i ) >> 4;
                                    i = 0;
                                }
                                else
                                {
                                    hsl.lightness = cl;
                                    i -= 16;
                                }
                            }
                            hsl2rgb( hsl, &color );

                            pos.x = ( INT8 )vx;
                            mapLocationColor( pos, color, dInd, psInd, vel );

                            if( vx != 0 )
                            {
                                pos.x = ( INT8 )( -vx );
                                mapLocationColor( pos, color, dInd, psInd, vel );
                            }

                            if( i <= 0 )
                                break;
                        }
                    }

                    /* The subsequent ones with original color to the bottom */
                    //vy = pPtr -> location.y - REN_FRAME2VIR_FRAME;
                    vy = ( pPtr -> aAmplitude - SIM128_TO_VIR96 ) - REN_FRAME2VIR_FRAME;
                    for( ; vy > -REN_SIM_FRAME/2; vy -= REN_FRAME2VIR_FRAME )
                        for( vx = -REN_SIM_FRAME/2; vx < REN_SIM_FRAME/2; vx += REN_FRAME2VIR_FRAME )
                        {
                            pos.x = ( INT8 )vx;
                            pos.y = ( INT8 )vy;
                            mapLocationColor( pos, cColor, dInd, psInd, vel );
                        }
                    break;

                case stImage:
                    /* Use sr as the index to the shape array */
                    sr += SimPS[ psInd ].psImage.shapeFlag & REN_SHAPE_IND_MASK;

                    for( vx = 0; vx < PAT_SHAPE_W; vx++ )
                    {
                        i = 1 << vx;     /* Bit mask to get x info */
                        sx = ( ( vx - REN_SHAPE_CENTER_W ) * REN_FRAME2VIR_FRAME ) + pPtr -> location.x;
                        if( ( sx < REN_SIM_FRAME/2 ) && ( sx > -REN_SIM_FRAME/2 ) )
                        {
                            /* Avoid overflow of coordinates */
                            pos.x = ( INT8 )sx;
                            for( vy = 0; vy < PAT_SHAPE_H; vy++ )
                            {
                                if( shape[ sr ].shape[ PAT_SHAPE_H - vy - 1 ] & i )
                                {
                                    /* The location is valid for the shape, show it */
                                    sy = ( ( vy - REN_SHAPE_CENTER_H ) * REN_FRAME2VIR_FRAME ) + pPtr -> location.y;
                                    if( ( sy < REN_SIM_FRAME/2 ) && ( sy > -REN_SIM_FRAME/2 ) )
                                    {
                                        /* Avoid overflow of coordinates */
                                        pos.y = ( INT8 )sy;
                                        mapLocationColor( pos, cColor, dInd, psInd, vel );
                                    }
                                }
                            }
                        }
                    }
                    break;

                case stNull:
                default:
                    if( ( SimPS[ psInd ].psImage.sColorMethod == ssmSpreadHue ) ||
                        ( SimPS[ psInd ].psImage.sColorMethod == ssmFading ) )
                    {
                        rgb2hsl( cColor, &hsl );
                        ch = hsl.hue;
                        cl = hsl.lightness;
                    }

                    for( vx = -sr; vx <= sr; vx++ )
                        for( vy = -sr; vy <= sr; vy++ )
                        {
                            if( ( ax * vx * vx + ay * vy * vy ) <= sr * sr )
                            {
                                color = cColor;
//                                scFlag = FALSE;
                                /* It is not the center of the particle, depends on size/shape color method */
                                switch( SimPS[ psInd ].psImage.sColorMethod )
                                {
//                                    case ssmOutline:
//                                        if( ( ( ax * ( vx + 1 ) * ( vx + 1 ) + ay * vy * vy ) > sr * sr ) ||
//                                            ( ( ax * ( vx - 1 ) * ( vx - 1 ) + ay * vy * vy ) > sr * sr ) ||
//                                            ( ( ax * vx * vx + ay * ( vy + 1 ) * ( vy + 1 ) ) > sr * sr ) ||
//                                            ( ( ax * vx * vx + ay * ( vy - 1 ) * ( vy - 1 ) ) > sr * sr ) )
//                                        {
//                                            /* It is at the outline of the shape, map color to it */
//                                            color = cColor;
//                                            scFlag = TRUE;
//                                        }
//                                        break;
//
                                    case ssmFading:
                                        /* Set brightness to inversely proportional to distance from center */
                                        if( sr != 0 )
                                        {
                                            hsl.lightness = cl / ( ( ax * vx * vx + ay * vy * vy ) + 1 );
                                            hsl2rgb( hsl, &color );
                                        }
                                        else
                                            color = cColor;
//                                        scFlag = TRUE;
                                        break;

                                    case ssmSpreadHue:
//                                        hsl.hue = ch - ( vy * 3 + 20 );
                                        hsl.hue = ch - vy * 3;
                                        hsl2rgb( hsl, &color );
                                        if( vy == sr )
                                            color = colorMap[ SimPS[ psInd ].psImage.bgFlag & REN_BG_IND_MASK ];
//                                        scFlag = TRUE;
                                        break;

                                    case ssmSolid:
                                    default:
                                        /* Same color as the center */
                                        color = cColor;
//                                        scFlag = TRUE;
                                        break;
                                }

//                                if( scFlag )
//                                {
                                    sx = vx * REN_FRAME2VIR_FRAME + pPtr -> location.x;
                                    sy = vy * REN_FRAME2VIR_FRAME + pPtr -> location.y;
                                    if( ( sx < REN_SIM_FRAME/2 ) && ( sx > -REN_SIM_FRAME/2 ) &&
                                        ( sy < REN_SIM_FRAME/2 ) && ( sy > -REN_SIM_FRAME/2 ) )
                                    {
                                        /* Avoid overflow of coordinates */
                                        pos.x = ( INT8 )sx;
                                        pos.y = ( INT8 )sy;

                                        if( ( ( vx == 0 ) && ( vy == 0 ) && ( SimPS[ psInd ].psBase.id != psFirefly ) ) ||
                                            ( SimPS[ psInd ].psBase.id == psHourglass ) )
                                        {
                                            /* Consider particle velocity information */
                                            /* Special case for hourglass, sand falling in all around */
                                            vel.x = pPtr -> velocity.x;
                                            vel.y = pPtr -> velocity.y;
                                        }
                                        else
                                        {
                                            /* Take firefly as a special case that always without velocity information */
                                            vel.x = 0;
                                            vel.y = 0;
                                        }
                                        mapLocationColor( pos, color, dInd, psInd, vel );
                                    }
//                                }
                            }
                        }
                    break;
            }

            if( SimPS[ psInd ].psImage.psiFlag & REN_PSI_PCOLOR_UPD )
                pPtr -> color = cColor;
        }

        pPtr++;
    }

    /* Put down target if necessary */
    if( SimPS[ psInd ].psImage.psiFlag & REN_PSI_SHOW_TARGET )
    {
        /* Need to show the target, show it */
        color = colorMap[ SimPS[ psInd ].psImage.colorTargetInd ];
        vel.x = 0;
        vel.y = 0;
        mapLocationColor( SimPS[ psInd ].psBase.target, color, dInd, psInd, vel );
    }
    
    //process whole frame picture
    FrameProcess(dInd, psInd);
}

/************************************************************************
 Function:    prepareConstructDisplay
 Description: Initialize construct display runtime variables
 Input:       seq: construct display sequence no.
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID prepareConstructDisplay(UINT16 seq)
{
    UINT16 offset;
    HSL_T  colorRgbHsl;

    /* Step 1 -- Copy info data */
    conDisSeq = seq;

    /* Step 2 -- Copy general operation parameters */
    offset = datOpSeq[ seq ].offset;
    switch( datOpSeq[ seq ].op )
    {
        case conDisVol:
            initConDisBuf( datPowerOnOff[ offset ].color, centerR, HUE_OR_COLOR_SNAP );
            curConDis.volume = datVol[ offset ];
            break;
            
        case conDisFlash:
            curConDis.flash = datFlash[ offset ];
            conDisState  = CON_DIS_FLASH_ON;        /* Starting state of Flash operation */
            break;
            
        case conDisRefresh:
            curConDis.refresh = datRefresh[ offset ];
#ifdef RUN_ONE_TIME_ONLY_DAT_REFRESH_FRAME
            curConDis.refresh.rfPtr = ( PHYDIS_T * )datRefreshFrame + curConDis.refresh.startInd;
#endif
            break;

        case conDisBreath:
            curConDis.breath = datBreath[ offset ];
            initConDisBuf( datBreath[ offset ].hColor, curConDis.breath.colorType, curConDis.breath.bgStepNum );
            conDisState = CON_DIS_BREATH_UP;        /* Starting state of Breath operation */
            break;

        case conDisColorPick:
            curConDis.colorPick = datColorPick[ offset ];
            curConDis.colorPick.color = GMColorPick;
            rgb2hsl( curConDis.colorPick.color, &colorRgbHsl );
            curConDis.colorPick.brightnessStep = colorRgbHsl.lightness >> 5;
            break;

        case conDisPowerOnOff:
            curConDis.powerOnOff = datPowerOnOff[ offset ];
            initConDisBuf( datPowerOnOff[ offset ].color, centerR, HUE_OR_COLOR_SNAP );
            break;

        case conDisBTConnected:
            conDisState = 0;
            curConDis.bTConnected = datBTConnected[ offset ];
            break;
            
         case conDisBTComplete:
             conDisState = 0;
             curConDis.bTComplete = datBTComplete[ offset ];
            break;

        case conDisCharge:
            conDisState = 0;
            curConDis.charge = datCharge[ offset ];
            break;

        case conDisFadeOut:
            conDisState = 0;
            curConDis.fadeOut = datFadeOut[ offset ];
            if(0 == offset)
            {
                curConDis.fadeOut.stepCnt = volDisHeight;
            }
            initConDisBuf( curConDis.fadeOut.hColor, curConDis.fadeOut.colorType, curConDis.fadeOut.stepCnt );
            break;

        default:
            break;
    }
}

/************************************************************************
 Function:    constructDisplay
 Description: Run a construct display at a physical display directly
 Input:       VOID
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID constructDisplay(VOID)
{
    BOOL continueFlag;

    /* Step 1 -- Run the current construct display */
    switch( datOpSeq[ conDisSeq ].op )
    {
        case conDisVol:
            continueFlag = runVol();
            break;
        case conDisFlash:
            continueFlag = runFlash();
            break;
        case conDisRefresh:
            continueFlag = runRefresh();
            break;
        case conDisBreath:
            continueFlag = runBreath();
            break;
        case conDisColorPick:
            continueFlag = runColorPick();
            break;
        case conDisPowerOnOff:
            continueFlag = runPowerOnOff();
            break;
        case conDisBTConnected:
            continueFlag = runBluetoothConnect();
            break;
         case conDisBTComplete:
             continueFlag = runBluetoothComplete();
            break;
        case conDisCharge:
            continueFlag = runCharge();
            break;
        case conDisFadeOut:
            continueFlag = runFadeOut();
            break;
        default:
            break;
    }

    /* Step 2 -- Check for operation transition */
    if( continueFlag == FALSE )
    {
        /* The current operation is ended, transit to the next */
        conDisSeq++;
        if( conDisSeq > datConDisInfo[ conDisID ].endSeq )
        {
            /* The sequence is completed, check if needs to repeat */
            if( conDisCnt != REN_CON_DIS_SEQ_INFINITE )
                conDisCnt--;

            if( conDisCnt > 0 )
            {
                /* Need to repeat the sequence */
                prepareConstructDisplay( datConDisInfo[ conDisID ].startSeq );
            }
            else
            {
                /* All repeat completed, end the whole process */
                if( conDisID == GMBTSpkShowID )
                    GMBTSpkShowID = BTSpkPatIdle;           /* @LM: end the process by reset GMBTSpkShowID.  Check if any conflict with OP... */
                //conDisID |= GM_PATTERN_IDLE;
                return;
            }
        }
        else
        {
            /* Prepare for the next operation in the sequence */
            prepareConstructDisplay( conDisSeq );
        }
    }

    /* Step 3 -- Update display index */
    PhyDisActiveInd = PhyDisLoadInd;
    PhyDisLoadInd   = ( PhyDisLoadInd - REN_PHY_DIS_CON_OFFSET + 1 ) % REN_PHY_DIS_CON_NUM  + REN_PHY_DIS_CON_OFFSET;
}

/************************************************************************
 Function:    refreshFrame
 Description: Set up a physical display from frames read from
              Flash driver
 Input:       VOID
 Return:      VOID
 Remarks:        
************************************************************************/
LOCAL VOID refreshFrame(VOID)
{
    /* Step 1 -- Run the active index to the next physical display */
    PhyDisActiveInd = ( PhyDisActiveInd + 1 ) % REN_PHY_DIS_NUM;

    /* Step 2 -- Trigger the loading of the next frame */
    PhyDisLoadInd = ( PhyDisLoadInd + 1 ) % REN_PHY_DIS_NUM;
}

/************************************************************************
 Function:    mapPhyDisplay
 Description: Map a virtual display to a physical display
 Input:       VOID
 Return:      VOID
 Remarks:        
************************************************************************/
LOCAL VOID mapPhyDisplay(VOID)
{
    UINT16 i, j, showDirection;
    BOOL   ouFlag;
    ORIENTATION_E ori;
    
    if( PhyDisLoadInd >= REN_PHY_DIS_VIR_NUM )
    {
        /* Last display is not virtual display map, reset it */
        PhyDisLoadInd = 0;
    }

    /* Read orientation */
    ori = GMGetOrientation();
    if( ori != cOri )
    {
        /* Orientation has been changed, check it */
        ouFlag = TRUE;
        if( GMPattern == PatHourglass )
        {
            /* Special orientation treatment for hourglass */
            /* Feature on hold according to meeting on Jun 13, 2015 */
            if( ( ori == Ori90 ) || ( ori == Ori270 ) )
            {
                if( ( cOri == Ori90 ) || ( cOri == Ori270 ) )
                {
                    /* The product is rolled, ignore the change */
                    ouFlag = FALSE;
                }
                else
                {
                    /* Pause the hourglass*/
//                    PauseHourglass();
                }
            }
            else if( ( cOri == Ori0 ) || ( cOri == Ori180 ) || ( pOri != ori ) )
            {
                /* The hourglass is put upside down, restart it */
                RestartHourglass();
                //SendEventNP( SIM_RESET_HOURGLASS_OP, OP, 0 );

            }
            else
            {
                /* The hourglass is returned to previous orientation, resume it */
//                ResumeHourglass();
            }
        }

        /* Update orientation record */
        if( ouFlag )
        {
            pOri = cOri;
            cOri = ori;
        }
    }

    showDirection = oriMatrix[ orientationLock ][ ori ];
    if( showDirection == Ori0 )
    {
        /* Display orientation is locked, directly copy virtual display to physical display */
        for( j = 0; j < REN_PHY_FRAME_W; j++ )
            for( i = 0; i < REN_PHY_FRAME_H; i++ )
                PhyDis[ PhyDisLoadInd ].phyDisColor[ j ][ i ] = virDis[ virDisActiveInd ].virDisColor[ j ][ i + 2 ];
    }
    else
    {
        /* Read current position to determine the turning degree of the virtual during mapping */
        if( showDirection == Ori90 )        //remark: in GlobalMemory.h ,variable GMOrientation had changed with ORIENTATION_E type
        {
            for( j = 0; j < REN_PHY_FRAME_W; j++ )
                for( i = 0; i < REN_PHY_FRAME_H; i++ )
                    PhyDis[ PhyDisLoadInd ].phyDisColor[ j ][ REN_PHY_FRAME_H - 1 - i ] = virDis[ virDisActiveInd ].virDisColor[ i + 2 ][ j ];
        }
        else if( showDirection == Ori180 )
        {
            for( j = 0; j < REN_PHY_FRAME_W; j++ )
                for( i = 0; i < REN_PHY_FRAME_H; i++ )
                    PhyDis[ PhyDisLoadInd ].phyDisColor[ REN_PHY_FRAME_W - 1 - j ][ REN_PHY_FRAME_H - 1 - i ] = virDis[ virDisActiveInd ].virDisColor[ j ][ i + 2 ];
        }
        else
        {
            for( j = 0; j < REN_PHY_FRAME_W; j++ )
                for( i = 0; i < REN_PHY_FRAME_H; i++ )
                    PhyDis[ PhyDisLoadInd ].phyDisColor[ REN_PHY_FRAME_W - 1 - j ][ i ] = virDis[ virDisActiveInd ].virDisColor[ i + 2 ][ j ];
        }
    }

    /* Update display indexes */
    PhyDisActiveInd = PhyDisLoadInd;
    PhyDisLoadInd   = ( PhyDisLoadInd + 1 ) % REN_PHY_DIS_VIR_NUM;
}

/************************************************************************
 Function:    clearVirBuf
 Description: Clear a virtual display
 Input:       dInd: virtual display index
 Return:      VOID
 Remarks:     Put all elements to 0
************************************************************************/
LOCAL VOID clearVirBuf(UINT16 dInd)
{
    UINT16 i, j;
    COLOR_T clearColor = { 0, 0, 0 };

    /* Step 1 -- Check buffer index validity */
    if( dInd >= REN_VIR_DIS_NUM )
    {
        /* Invalid buffer index, ignore it */
        return;
    }

    /* Step 2 -- Clear the buffer */
    for( i = 0; i < REN_VIR_DIS_FRAME; i++ )
        for( j = 0; j < REN_VIR_DIS_FRAME; j++ )
            virDis[ dInd ].virDisColor[ i ][ j ] = clearColor;
}

/************************************************************************
 Function:    clearPhyBuf
 Description: Clear a physical display
 Input:       dInd: physical display index
 Return:      VOID
 Remarks:     Put all elements to 0
************************************************************************/
LOCAL VOID clearPhyBuf(UINT16 dInd)
{
    UINT16 i, j;
    COLOR_T clearColor = { 0, 0, 0 };
    
    /* Step 1 -- Check buffer index validity */
    if( dInd >= REN_PHY_DIS_NUM )
    {
        /* Invalid buffer index, ignore it */
        return;
    }

    /* Step 2 -- Clear the buffer */
    for( i = 0; i < REN_PHY_FRAME_W; i++ )
        for( j = 0; j < REN_PHY_FRAME_H; j++ )
        {
            PhyDis[ dInd ].phyDisColor[ i ][ j ] = clearColor;
        }
}

/************************************************************************
 Function:    updateColor
 Description: Update color according to color operation
 Input:       color:  initial color
              cOpInd: color operation index
              pPtr:   pointer to particle
              psInd:  particle system index
              cPtr:   pointer to storage of result color
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID updateColor(COLOR_T color, UINT16 cOpInd, PARTICLE_ST *pPtr, UINT16 psInd, COLOR_T *cPtr)
{
    UINT16  paraInd, ind;
    INT16   cDat;
    UINT8   *ptr;
    INT32   dat;
    HSL_T   hsl;
    COLOR_T rc;

    /* Step 1 -- Check the parameter type */
    paraInd = colorOp[ cOpInd ].pID;
    switch( SimParaProperty[ paraInd ].pGroup )
    {
        case psgP:
            ptr = ( UINT8 * )( pPtr );
            break;

        case psgPsb:
            ptr = ( UINT8 * )( &SimPS[ psInd ].psBase );
            break;

        case psgPsi:
            ptr = ( UINT8 * )( &SimPS[ psInd ].psImage );
            break;

        default:
            return;
    }
    ptr += SimParaProperty[ paraInd ].offset;

    /* Step 2 -- Read out the parameter */
    switch( SimParaProperty[ paraInd ].pType )
    {
        case pstUINT8:
            dat = ( INT32 )( *ptr );
            break;

        case pstINT8:
            dat = ( INT32 )( *( INT8 * )ptr );
            break;

        case pstUINT16:
            dat = ( INT32 )( * ( UINT16 * )ptr );
            break;

        case pstINT16:
            dat = ( INT32 )( *( INT16 * )ptr );
            break;

        case pstPTR:
            /* No pointer comparision... */
            return;

        case pstVector:
            dat = SimVectorMag2( *( PVECTOR_ST * )ptr );
            break;

        default:
            return;
    }

    /* Step 3 -- Operation */
    switch( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK )
    {
        case coBrightness:
        case coBrightnessNew:
        case coBreath:
            rgb2hsl( color, &hsl );
            if( ( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK ) == coBreath )
            {
                cDat = colorOpFormula( hsl.lightness, colorOp[ cOpInd ].sFactor, ( INT16 )dat,
                                       REN_CO_OC0( colorOp[ cOpInd ].opCode ), REN_CO_OC1( colorOp[ cOpInd ].opCode ),
                                       REN_CO_OPTION( colorOp[ cOpInd ].opCode ) );
                if( cDat < 0 )
                    cDat = -cDat;

                hsl.lightness -= cDat / colorOp[ cOpInd ].sFactor;
            }
            else
            {
                if( ( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK ) == coBrightnessNew )
                {
                    if( dat < 0 )
                        dat = -dat;
                }

                hsl.lightness = colorOpFormula( hsl.lightness, colorOp[ cOpInd ].sFactor, ( INT16 )dat,
                                                REN_CO_OC0( colorOp[ cOpInd ].opCode ), REN_CO_OC1( colorOp[ cOpInd ].opCode ),
                                                REN_CO_OPTION( colorOp[ cOpInd ].opCode ) );
            }
            hsl2rgb( hsl, &rc );
            break;

        case coHue:
        case coHueNew:
            rgb2hsl( color, &hsl );
            if( ( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK ) == coHueNew )
                hsl.hue = 0;

            hsl.hue = colorOpFormula( hsl.hue, colorOp[ cOpInd ].sFactor, ( INT16 )dat,
                                      REN_CO_OC0( colorOp[ cOpInd ].opCode ), REN_CO_OC1( colorOp[ cOpInd ].opCode ),
                                      REN_CO_OPTION( colorOp[ cOpInd ].opCode ) );
            hsl2rgb( hsl, &rc );
            break;

        case coRed:
        case coGreen:
        case coBlue:
            rc = color;
            if( dat < 0 )
                dat = -dat;

            if( ( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK ) == coRed )
                cDat = rc.r;
            else if( ( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK ) == coGreen )
                cDat = rc.g;
            else
                cDat = rc.b;

            cDat = colorOpFormula( cDat, colorOp[ cOpInd ].sFactor, ( INT16 )dat,
                                   REN_CO_OC0( colorOp[ cOpInd ].opCode ), REN_CO_OC1( colorOp[ cOpInd ].opCode ),
                                   REN_CO_OPTION( colorOp[ cOpInd ].opCode ) );

            if( cDat < 0 )
                cDat = 0;
            else if( cDat > MAX_RGB )
                cDat = MAX_RGB;

            if( ( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK ) == coRed )
                rc.r = ( UINT8 )cDat;
            else if( ( colorOp[ cOpInd ].opType & REN_CO_TYPE_MASK ) == coGreen )
                rc.g = ( UINT8 )cDat;
            else
                rc.b = ( UINT8 )cDat;
            break;

        case coColorInd:
        default:
            ind = ( UINT16 )colorOpFormula( colorOp[ cOpInd ].sFactor, ( INT16 )dat, 0,
                                            REN_CO_OC0( colorOp[ cOpInd ].opCode ), 0,
                                            REN_CO_OPTION( colorOp[ cOpInd ].opCode ) );
            ind %= SimPS[ psInd ].psImage.colorMapLen;
            ind += SimPS[ psInd ].psImage.colorMapInd;
            rc = colorMap[ ind ];
            break;
    }

    /* Step 4 -- Add to initial color and set to result color */
    if( colorOp[ cOpInd ].opType & REN_CO_TYPE_ABS_MASK )
    {
        /* Need absolute color */
        *cPtr = rc;
    }
    else
    {
        /* Add up the color */
        addColor( color, rc, cPtr );
    }
}

//uAlphaRate:0~255, 0:transparent
LOCAL VOID AlphaColor(COLOR_T upperColor, COLOR_T lowerColor, UINT8 uAlphaRate, COLOR_T *pRltColor)
{
    UINT32  uUpVec, uLowVec;
    
    uUpVec = upperColor.r; uLowVec = lowerColor.r;
    pRltColor->r = (uAlphaRate * uUpVec + (256 - uAlphaRate) * uLowVec) / 256;
    
    uUpVec = upperColor.g; uLowVec = lowerColor.g;
    pRltColor->g = (uAlphaRate * uUpVec + (256 - uAlphaRate) * uLowVec) / 256;
    
    uUpVec = upperColor.b; uLowVec = lowerColor.b;
    pRltColor->b = (uAlphaRate * uUpVec + (256 - uAlphaRate) * uLowVec) / 256;
}

/************************************************************************
 Function:    mapLocationColor
 Description: Map the color of location into a virtual display
 Input:       pos:   a location vector, in the simulator coordinates
              color: the color for display
              dInd:  the virtual display index
              psInd: particle system index
              vel:   velocity of a particle
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID mapLocationColor(PVECTOR_ST pos, COLOR_T color, UINT16 dInd, UINT16 psInd, PVECTOR_ST vel)
{
    INT16   vx, vy, sx, sy;                                 /* Mapped pixel coordinates in virtual display */
    INT16   posH, posW, rPos;
    UINT16  wf;
    COLOR_T wc, tempC;
    HSL_T   hsl;
    INT16   l, nl, pl, tl, v;
    INT32   tempB;

    /* Step 1 -- Judge whether the position that particle mapping is in vitual frame  area */
    if( pos.x > ( SIM128_TO_VIR96  ) || pos.x < (  - SIM128_TO_VIR96  ) ||
        pos.y > ( SIM128_TO_VIR96 ) || pos.y < (  - SIM128_TO_VIR96 ) )
        return;

    posH = pos.y + SIM128_TO_VIR96;
    posW = pos.x + SIM128_TO_VIR96;

    /* Step 2 -- Determine the which pixel pos belongs to */
    vy =  posH / REN_FRAME2VIR_FRAME;
    vx =  posW / REN_FRAME2VIR_FRAME;
    
    /* Check whether the position that particle mapping is in vitual display area */
//    if( vy > ( REN_VIR_DIS_FRAME - 1 ) || vx > ( REN_VIR_DIS_FRAME - 1 ) || vy < 0 || vx < 0 )
    if( vy > ( REN_VIR_DIS_FRAME - 3 ) || vx > ( REN_VIR_DIS_FRAME - 1 ) || vy < 2 || vx < 0 )
        return;
  
    if( SimPS[ psInd ].psImage.psiFlag & REN_PSI_COLOR_SNAP ) 
    {
        /* Just copy the color */
        wc = color;            
    }
    else
    {
        /* Step 3 -- Calculate the weighted factor of color according to the position relative to the pixel */
        wf = weightMatrix[ posH % REN_FRAME2VIR_FRAME / SIXTEEN_TO_FOUR_MATRIX ][ posW % REN_FRAME2VIR_FRAME / SIXTEEN_TO_FOUR_MATRIX ];

        /* Step 4 -- Update the color according to the weighted factor */
        /* Step 4.1 -- Convert to HSL values */
        rgb2hsl( color, &hsl );

        /* Step 4.2 -- Scale L' value */
        hsl.lightness = ( hsl.lightness * wf ) >> WEIGHT_FACTOR_RATIO;

        /* Step 4.3 -- Convert back to r, g, b */
        hsl2rgb( hsl, &wc );
    }
        
    /* Step 5 -- Set color addition or overlap */
    if( SimPS[ psInd ].psImage.psiFlag & REN_PSI_COLOR_OL )
    {
        /* Overlap the color, no need to revise wc */
    }
    else
    {
        tempC = virDis[ dInd ].virDisColor[ vx ][ vy ];
        rgb2hsl( tempC, &hsl );             /* Convert the color to HSL model */
                                            /* Scaling back to standard brightness */
        tempB = masterBrightness ? hsl.lightness * GM_BRIGHTNESS_MAX / masterBrightness : 0;
        hsl.lightness = ( INT16 )tempB;
        hsl2rgb( hsl, &tempC );
        addColor( tempC, wc, &wc );
    }
    
    if(SimPS[ psInd ].psBase.id == psExplosion)
    {
        tempC = virDis[ dInd ].virDisColor[ vx ][ vy ];
        if( 0 != (tempC.r | tempC.g | tempC.b))
            AlphaColor(wc, tempC, 96, &wc);
    }

    /* Step 6 -- Add velocity information and put to virtual display */
    rgb2hsl( wc, &hsl );
    l = hsl.lightness;

    /* Step 6.1 -- Put down the current particle */
    tempB = l * masterBrightness / GM_BRIGHTNESS_MAX;
    hsl.lightness = ( INT16 )tempB;
    hsl2rgb( hsl, &wc );

    virDis[ dInd ].virDisColor[ vx ][ vy ] = wc;
    if( ( ( vel.x != 0 ) && ( vel.y != 0 ) ) || ( ( vel.x == 0 ) && ( vel.y == 0 ) ) )
    {
        /* No movement or move in both x and y, ignore in the moment */
        return;
    }
           
    /* Step 6.2 -- Calculate subsequent LED lightness */
    if( vel.x != 0 )
    {
        /* In x direction movement */
        if( vel.x  > 0 )
        {
            rPos = posW % REN_FRAME2VIR_FRAME;
            v = vel.x;
        }
        else
        {
            rPos = REN_FRAME2VIR_FRAME - 1 - ( posW % REN_FRAME2VIR_FRAME );
            v = -vel.x;
        }
    }
    else
    {
        /* In y direction movement */
        if( vel.y > 0 )
        {
            rPos = posH % REN_FRAME2VIR_FRAME;
            v = vel.y;
        }
        else
        {
            rPos = REN_FRAME2VIR_FRAME - 1 - ( posH % REN_FRAME2VIR_FRAME );
            v = -vel.y;
        }
    }

    /* Set previous LED ligthness according to velocity */
    if( vel.y != 0 )
    {
        pl = l * ( 31 - rPos ) / 32;
        tl = l * ( 15 - rPos ) / 32;
    }
    else if( v < 5 )
    {
        pl = l * ( 23 - rPos ) / 24;
        if( rPos < 8 )
            tl = l * ( 8 - rPos ) / 24;
        else
            tl = 0;
    }
    else
    {
        pl = l * ( 15 - rPos ) / 16;
        tl = 0;     /* No need for the 4th LED to light up */
    }

    nl = l * rPos / 16;

    /* Step 6.3 -- Put down the next LED */
    tempB = nl * masterBrightness / GM_BRIGHTNESS_MAX;
    hsl.lightness = ( INT16 )tempB;
    hsl2rgb( hsl, &wc );
    if( vel.x != 0 )
    {
        if( vel.x > 0 )
            sx = vx + 1;
        else
            sx = vx - 1;

        sy = vy;
    }
    else
    {
        if( vel.y > 0 )
            sy = vy + 1;
        else
            sy = vy - 1;

        sx = vx;
    }
    if( ( sx < REN_VIR_DIS_FRAME ) && ( sx >= 0 ) && ( sy < ( REN_VIR_DIS_FRAME - 1 ) ) && ( sy > 1 ) )
    {
        /* Move in positive x direction */
        virDis[ dInd ].virDisColor[ sx ][ sy ] = wc;
    }

    /* Step 6.4 -- Put down the previous LED */
    tempB = pl * masterBrightness / GM_BRIGHTNESS_MAX;
    hsl.lightness = ( INT16 ) tempB;
    hsl2rgb( hsl, &wc );
    if( vel.x != 0 )
    {
        if( vel.x > 0 )
            sx = vx - 1;
        else
            sx = vx + 1;

        sy = vy;
    }
    else
    {
        if( vel.y > 0 )
            sy = vy - 1;
        else
            sy = vy + 1;

        sx = vx;
    }
    if( ( sx < REN_VIR_DIS_FRAME ) && ( sx >= 0 ) && ( sy < ( REN_VIR_DIS_FRAME - 1 ) ) && ( sy > 1 ) )
    {
        /* Move in positive x direction */
        virDis[ dInd ].virDisColor[ sx ][ sy ] = wc;
    }
    else
        return;

    /* Step 6.5 -- Put down the tail LED, if necessary */
    if( tl > 0 )
    {
        tempB = tl * masterBrightness / GM_BRIGHTNESS_MAX;
        hsl.lightness = ( INT16 )tempB;
        hsl2rgb( hsl, &wc );
        if( vel.x != 0 )
        {
            if( vel.x > 0 )
                sx = vx - 2;
            else
                sx = vx + 2;

            sy = vy;
        }
        else
        {
            if( vel.y > 0 )
                sy = vy - 2;
            else
                sy = vy + 2;

            sx = vx;
        }
        if( ( sx < REN_VIR_DIS_FRAME ) && ( sx >= 0 ) && ( sy < ( REN_VIR_DIS_FRAME - 1 ) ) && ( sy > 1 ) )
        {
            /* Move in positive x direction */
            virDis[ dInd ].virDisColor[ sx ][ sy ] = wc;
        }
    }
}

/************************************************************************
 Function:    runVol
 Description: run volume level display
 Input:       VOID
 Return:      TRUE: continue run it; FALSE: complete
 Remarks:
************************************************************************/
LOCAL BOOL runVol(VOID)
{
    UINT16 h, w;
    UINT8 brightnessFactor;

    /* Step 1 -- Check if volume has been pressed */
    if( GMVolumeAbs != curConDis.volume.showVol )
    {
        /* Reload running parameters */
        curConDis.volume = datVol[ datOpSeq[ conDisSeq ].offset ];
        curConDis.volume.showVol = GMVolumeAbs;        /* Update to the latest volume */
        initConDisBuf( datPowerOnOff[ 0 ].color, centerR, HUE_OR_COLOR_SNAP );
    }

    /* Step 2 -- Check process end condition */
    if( --curConDis.volume.uRate == 0 )
    {
        /* Update timeout, end the process */
        return FALSE;
    }

    /* Step 3 -- Set display height and show color index */
    volDisHeight = (curConDis.volume.showVol+3)>>2;
        

    /* Step 4 -- Show the volume level */
    clearPhyBuf( PhyDisLoadInd );

    if( volDisHeight > 0 )
    {
        for( w = 0; w < REN_PHY_FRAME_W; w++ )
        {
            for( h = 0; h < volDisHeight; h++ )
            {
                PhyDis[ PhyDisLoadInd ].phyDisColor[ w ][ h ] = conDisBuf[ w ][ h ] ;                
            }
        }
        brightnessFactor = curConDis.volume.showVol%4;
        if(brightnessFactor)
        {
            for( w = 0; w < REN_PHY_FRAME_W; w++ )
            {
                volAdjustColor.r = ((conDisBuf[ w ][ volDisHeight-1 ].r+3)>>2)*brightnessFactor;
                volAdjustColor.b = ((conDisBuf[ w ][ volDisHeight-1 ].b+3)>>2)*brightnessFactor;
                volAdjustColor.g = ((conDisBuf[ w ][ volDisHeight-1 ].g+3)>>2)*brightnessFactor;

                PhyDis[ PhyDisLoadInd ].phyDisColor[ w ][ volDisHeight-1 ] = volAdjustColor;            
            }
        }
    }

    return TRUE;
}
/************************************************************************
 Function:    runFlash
 Description: Run a Flash construct display
 Input:       VOID
 Return:      TRUE: continue run it; FALSE: complete
 Remarks:
************************************************************************/
LOCAL BOOL runFlash(VOID)
{
    UINT16 h, w;

    if( conDisState == CON_DIS_FLASH_ON )
    {
        if( --curConDis.flash.onTime == 0 )
        {
            /* End of on state, go to off state */
            conDisState = CON_DIS_FLASH_OFF;
        }
        else
        {
            /* Copy color to display */
            for( w = 0; w < REN_PHY_FRAME_W; w++ )
                for( h = 0; h < REN_PHY_FRAME_H; h++ )
                {
                    PhyDis[ PhyDisLoadInd ].phyDisColor[ w ][ h ] = curConDis.flash.color;
                }
        }
    }
    else
    {
        /* Off state, check off timer */
        if( --curConDis.flash.offTime == 0 )
        {
            /* End of off state, check repeat count */
            if( --curConDis.flash.count > 0 )
            {
                /* Need to repeat the process, reload parameters */
                conDisState = CON_DIS_FLASH_ON;
                curConDis.flash.onTime  = datFlash[ datOpSeq[ conDisSeq ].offset ].onTime;
                curConDis.flash.offTime = datFlash[ datOpSeq[ conDisSeq ].offset ].offTime;
            }
            else
            {
                /* All repeats completed, end the process */
                return FALSE;
            }
        }
        else
        {
            /* Clear the display */
            clearPhyBuf( PhyDisLoadInd );
        }
    }

    return TRUE;
}

/************************************************************************
 Function:    runRefresh
 Description: Run a Refresh construct display
 Input:       VOID
 Return:      TRUE: continue run it; FALSE: complete
 Remarks:
************************************************************************/
LOCAL BOOL runRefresh(VOID)
{
#ifdef RUN_ONE_TIME_ONLY_DAT_REFRESH_FRAME
    UINT16 h, w;
#endif

    if( --curConDis.refresh.rRate == 0 )
    {
        /* The current frame has to be refreshed */
        if( curConDis.refresh.startInd == curConDis.refresh.endInd )
        {
            if(curConDis.refresh.rowStartInd == curConDis.refresh.rowEndInd)
            {
                /* All frames are shown, end the process */
                return FALSE;
            }
            else
            {
                curConDis.refresh.startInd = datRefresh[ datOpSeq[ conDisSeq ].offset ].startInd;
                curConDis.refresh.rfPtr = ( PHYDIS_T * )datRefreshFrame + curConDis.refresh.startInd;
                curConDis.refresh.rowStartInd++;
            }
        }
        else
        {
            /* Move on to the next frame */
            curConDis.refresh.startInd++;
            curConDis.refresh.rfPtr++;
        }
        curConDis.refresh.rRate = datRefresh[ datOpSeq[ conDisSeq ].offset ].rRate;
    }

#ifndef RUN_ONE_TIME_ONLY_DAT_REFRESH_FRAME
    APP_FlashRead_DatRefreshFrame( curConDis.refresh.startInd, &PhyDis[ PhyDisLoadInd ] );
#else
    clearPhyBuf(PhyDisLoadInd);
    /* Copy frame to display buffer */
    for( w = 0; w < REN_PHY_FRAME_W - curConDis.refresh.rowStartInd; w++ )
        for( h = 0; h < REN_PHY_FRAME_H; h++ )
            PhyDis[ PhyDisLoadInd ].phyDisColor[ w + curConDis.refresh.rowStartInd ][ h ] = curConDis.refresh.rfPtr -> phyDisColor[ w ][ h ];


    for( w = 0; w < curConDis.refresh.rowStartInd; w++ )
        for( h = 0; h < REN_PHY_FRAME_H; h++ )
            PhyDis[ PhyDisLoadInd ].phyDisColor[ w ][ h ] = curConDis.refresh.rfPtr -> phyDisColor[ REN_PHY_FRAME_W - curConDis.refresh.rowStartInd + w ][ h ];
#endif

    return TRUE;
}

/************************************************************************
 Function:    runBreath
 Description: Run a Breath construct display
 Input:       VOID
 Return:      TRUE: continue run it; FALSE: complete
 Remarks:
************************************************************************/
LOCAL BOOL runBreath(VOID)
{
    UINT16  i, j;
    HSL_T   hslColor;

    /* Step 1 -- Update brightness if necessary */
    if( --curConDis.breath.bRate == 0 )
    {
        curConDis.breath.bRate = datBreath[ datOpSeq[ conDisSeq ].offset ].bRate;
        curConDis.breath.stepCnt++;
        if( curConDis.breath.stepCnt == curConDis.breath.maxStepNum )
        {
            /* Already reach the maximum L, go the down state */
            curConDis.breath.lStep = -curConDis.breath.lStep;
        }
        else
        {
            /* Update lightness value */
            for( j = 0; j < REN_PHY_FRAME_W; j++ )
                for( i = 0; i < REN_PHY_FRAME_H; i++ )
                {
                    rgb2hsl( conDisBuf[ j ][ i ], &hslColor );
                    if (curConDis.breath.lStep)
                        hslColor.lightness += curConDis.breath.lStep;
                    else
                    {
                        if (hslColor.lightness >= 5)
                            hslColor.lightness -= 5;
                    }
                    hsl2rgb( hslColor, &conDisBuf[ j ][ i ] );
                }
        }
        if( curConDis.breath.stepCnt == ( curConDis.breath.maxStepNum * 2 ) )
            return FALSE;
    }

    for( j = 0; j < REN_PHY_FRAME_W; j++ )
        for( i = 0; i < REN_PHY_FRAME_H; i++ )
            PhyDis[ PhyDisLoadInd ].phyDisColor[ j ][ i ] = conDisBuf[ j ][ i ];

    return TRUE;
}

/************************************************************************
 Function:    runColorPick
 Description: Run a color pick construct display
 Input:       VOID
 Return:      TRUE: continue run it; FALSE: complete
 Remarks:
************************************************************************/
LOCAL BOOL runColorPick(VOID)
{
    UINT16  i, j;
    HSL_T  colorPickHsl;

    if( curConDis.colorPick.processCnt > 0 )
    {
       for( i = 0; i < REN_PHY_FRAME_W; i++ )
       {
         for( j = 0; j < REN_PHY_FRAME_H; j++ )
         {
            PhyDis[ PhyDisLoadInd ].phyDisColor[ i ][ j ] = curConDis.colorPick.color;
         }           
       }
       curConDis.colorPick.processCnt--;
       return TRUE;
    }

    rgb2hsl( curConDis.colorPick.color, &colorPickHsl );
    colorPickHsl.lightness -= curConDis.colorPick.brightnessStep;
    hsl2rgb( colorPickHsl, &curConDis.colorPick.color );

    clearPhyBuf( PhyDisLoadInd );    
    for( i = 0; i < REN_PHY_FRAME_W; i++ )
    {
        for( j = 0; j < REN_PHY_FRAME_H; j++ )
        {
            PhyDis[ PhyDisLoadInd ].phyDisColor[ i ][ j ] = curConDis.colorPick.color;
        }           
    }

    if( --curConDis.colorPick.uBRate == 0 )
    {
       return FALSE;
    }
    else
       return TRUE;
}

/************************************************************************
 Function:    adjustPickColor
 Description: Adjust the color at GMColorPick according to environmental effect
 Input:       VOID
 Return:      VOID
 Remarks:     Called by runColorPick() in Prism function
 *            in Canvas pattern processing
************************************************************************/
#ifdef OPEN_COLOR_SENSOR_ADJ_ENABLE
VOID adjustPickColor(VOID)
#else
LOCAL VOID adjustPickColor(VOID)
#endif /* OPEN_COLOR_SENSOR_ADJ_ENABLE */
{
    HSL_T hColor;
    INT16 deltaC;
    INT32 tempB;

    rgb2hsl( GMColorPick, &hColor );

    /* Step 1 -- Adjust saturation */
    if( hColor.lightness < 255 )
        deltaC = ( hColor.saturation * hColor.lightness ) / 512;
    else
        deltaC = ( hColor.saturation * ( 510 - hColor.lightness ) ) / 512;

    /* Step 2 -- Calculate lightness according to master brightness */
    if( deltaC < 20 )               /* A range for white color */
    {
        /* Keep its own brightness for white color */
        hColor.saturation = 0;
        tempB = hColor.lightness;
        if( tempB < 20 )
           tempB = 0;  
    }
    else if( hColor.saturation < 500 )
    {
        /* Referenc lightness at half of the lightness scale */
        hColor.saturation = 500;    /* Set to a high level */
        tempB = 255;
    }
    else
        tempB = 255;

    hColor.lightness = ( INT16 )tempB;

    hsl2rgb( hColor, &GMColorPick );
}


/************************************************************************
 Function:    initConDisBuf
 Description: Initialize construct display buffer according to seed color and color type
 Input:       seedColor: calculate the power on background according to the seedColor
              colorType: according the colorType to change the background
              colorStep: step hue or step color increment
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID initConDisBuf(COLOR_T seedColor, CON_DIS_CTYPE_E colorType, UINT8 colorStep)
{
    UINT16  i, j;
    HSL_T   bgHsl;
    COLOR_T addColor0 , addColor1;

    addColor0 = seedColor;
    addColor1  = seedColor;

    for( j = 0; j < REN_PHY_FRAME_W; j++ )
        for( i = 0; i < REN_PHY_FRAME_H; i++ )
        {
            switch( colorType )
            {
                case centerR:
                    addColor0.r = 255;
                    switch( i )
                    {
                        case 0: addColor0.g = 0;
                                addColor0.b = 134;
                                break;
                        case 1: addColor0.g = 0;
                                addColor0.b = 92;
                                break;
                        case 2: addColor0.g = 0;
                                addColor0.b = 49;
                                break;                                
                        case 3: addColor0.g = 42;
                                addColor0.b = 0;
                                break;                            
                        case 4: addColor0.g = 85;
                                addColor0.b = 0;
                                break;                                
                        case 5: addColor0.g = 127;
                                addColor0.b = 0;
                                break;                                
                        case 6: addColor0.g = 170;
                                addColor0.b = 0;
                                break;                                
                        case 7: addColor0.g = 212;
                                addColor0.b = 0;
                                break;                                
                    }
                    conDisBuf[ j ][ i ] = addColor0;
                    break;

                case centerG:
                    if( ( 5 * i ) >= ( 4 * j ) )
                    {
                        addColor0.b = ( ( ( 5 * i ) - ( 4 * j ) ) * colorStep );
                        conDisBuf[ j ][ i ] = addColor0;
                    }
                    else
                    {
                        addColor1.r = ( ( ( 4 * j ) - ( 5 * i ) ) * colorStep );
                        conDisBuf[ j ][ i ] = addColor1;
                    }
                    break;

                case centerB:
                    if( ( 5 * i ) >= ( 4 * j ) )
                    {
                        addColor0.r = ( ( ( 5 * i ) - ( 4 * j ) ) * colorStep );
                        conDisBuf[ j ][ i ] = addColor0;
                    }
                    else
                    {
                        addColor1.g = ( ( ( 4 * j ) - ( 5 * i ) ) * colorStep );
                        conDisBuf[ j ][ i ] = addColor1;
                    }
                    break;

                case rowChangeHue:
                    rgb2hsl( addColor0, &bgHsl );
                    bgHsl.hue = i * colorStep;
                    hsl2rgb( bgHsl, &addColor0 );
                    conDisBuf[ j ][ i ] = addColor0;
                    break;

                case columnChangeHue:
                    rgb2hsl( addColor0, &bgHsl );
                    bgHsl.hue = j * colorStep;
                    hsl2rgb( bgHsl, &addColor0 );
                    conDisBuf[ j ][ i ] = addColor0;
                    break;

                case volFadeColor:
                    if( i == (volDisHeight-1) )
                        conDisBuf[ j ][ i ] = volAdjustColor;
                    break;

                case unChange:
                default:
                    conDisBuf[ j ][ i ] = seedColor;
                    break;
            }
        }

}

/************************************************************************
 Function:    runPowerOnOff
 Description: Show power on/off random color display
 Input:       VOID
 Return:      TRUE: continue executing this function  FALSE: break executing
 Remarks:
************************************************************************/
LOCAL BOOL runPowerOnOff(VOID)
{
    UINT16 row, column;
    INT16  tl;
    HSL_T       bgHsl;
    COLOR_T     tColor = { 0, 0, 0 };
    COLOR_T clearColor = { 0, 0, 0 };

    if( --curConDis.powerOnOff.pRate == 0 )
    {
        curConDis.powerOnOff.pRate = datPowerOnOff[ datOpSeq[ conDisSeq ].offset ].pRate;

        curConDis.powerOnOff.step += curConDis.powerOnOff.next;
        if( curConDis.powerOnOff.step == curConDis.powerOnOff.endStep )
            return FALSE;
    }
    for( column = 0; column < REN_PHY_FRAME_W; column++ )
        for( row = 0; row < REN_PHY_FRAME_H; row++ )
        {
            if( ( curConDis.powerOnOff.step > powerOnOffMatrix[ column][ row ] ) )
            {
                rgb2hsl( conDisBuf[ column ][ row ], &bgHsl );
                tl = curConDis.powerOnOff.step - (row * REN_PHY_FRAME_W);
                
                if(tl <= REN_POWERONOFF_BAR1)
                    bgHsl.lightness = bgHsl.lightness >> 2;
                else if(tl <= REN_POWERONOFF_BAR2)
                    bgHsl.lightness = bgHsl.lightness >> 1;
                else if(tl <= REN_POWERONOFF_BAR3)
                    bgHsl.lightness = (bgHsl.lightness*3) >> 2;

                hsl2rgb( bgHsl, &tColor);
                PhyDis[ PhyDisLoadInd ].phyDisColor[ column ][ row ] = tColor;
            }
            else
            {
                PhyDis[ PhyDisLoadInd ].phyDisColor[ column ][ row ] = clearColor;
            }
    }

    return TRUE;
}


LOCAL BOOL runBluetoothComplete(VOID)
{
    UINT16  i, j;
    HSL_T  btCompleteHsl;

    if( curConDis.bTComplete.processCnt > 0 )
    {
       for( i = 0; i < REN_PHY_FRAME_W; i++ )
       {
         for( j = 0; j < REN_PHY_FRAME_H; j++ )
         {
            PhyDis[ PhyDisLoadInd ].phyDisColor[ i ][j] = curConDis.bTComplete.color;
         }           
       }
       curConDis.bTComplete.processCnt--;
       return TRUE;
    }

    rgb2hsl( curConDis.bTComplete.color, &btCompleteHsl );
    btCompleteHsl.lightness -= curConDis.bTConnected.brightnessStep;
    hsl2rgb( btCompleteHsl, &curConDis.bTComplete.color );

    clearPhyBuf( PhyDisLoadInd );    
    for( i = 0; i < REN_PHY_FRAME_W; i++ )
    {
        for( j = 0; j < REN_PHY_FRAME_H; j++ )
        {
            PhyDis[ PhyDisLoadInd ].phyDisColor[ i ][j] = curConDis.bTComplete.color;
        }           
    }

    if( --curConDis.bTComplete.uBRate == 0 )
    {
       return FALSE;
    }
    else
       return TRUE;

}
/************************************************************************
 Function:    runBluetoothConnect
 Description: brightness decreace with the distance form center point
 Input:       VOID
 Return:      TRUE : continue executing this function  FALSE: break executing
 Remarks:
************************************************************************/
LOCAL BOOL runBluetoothConnect(VOID)
{
    UINT16  i, j;
    //INT16   reserveColor;
    UINT16  tempColor;
    //INT16   displayR;

    tempColor = curConDis.bTConnected.originalColor;
    if( -- curConDis.bTConnected.uBRate == 0 )
    {
        curConDis.bTConnected.uBRate = datBTConnected[ datOpSeq[ conDisSeq ].offset ].uBRate;
        if( curConDis.bTConnected.processCnt-- == 0 )
            return FALSE;
    }

    clearPhyBuf( PhyDisLoadInd );    
    for( i = 0; i <= 6-curConDis.bTConnected.processCnt; i++ )
    {
        for( j = 0; j < REN_PHY_FRAME_H; j++ )
        {
            PhyDis[ PhyDisLoadInd ].phyDisColor[ i ][ j ].b = tempColor;
            if( 0 != i )
            {
                PhyDis[ PhyDisLoadInd ].phyDisColor[ REN_PHY_FRAME_W - i ][ j ].b = tempColor;
            }
        }
    }
    if( 0 == curConDis.bTConnected.processCnt )
    {
         for( j = 0; j < REN_PHY_FRAME_H; j++ )
        {
            PhyDis[ PhyDisLoadInd ].phyDisColor[ 6 ][ j ].b = tempColor;
        }
    }
        

    return TRUE;
}
/************************************************************************
 Function:    runCharge
 Description: display the the charge view, from bottom to top increase brightness
 Input:       VOID
 Return:      TRUE : continue executing this function  FALSE: break executing
 Remarks:
************************************************************************/
LOCAL BOOL runCharge(VOID)
{
    UINT16 disRow, disColumn;
    HSL_T  chargeHsl;
    COLOR_T black = { 0, 0, 0 };
    clearPhyBuf( PhyDisLoadInd ); 

    if( curConDis.charge.cRate > curConDis.charge.phase1 )
    {
        /* calculate the brightness of the top row */
        if( conDisState == 1 )
        {
             if( curConDis.charge.endRow < GMBatteryCap )
             {
                 curConDis.charge.endRow++;
                 curConDis.charge.cRate = datCharge[ datOpSeq[ conDisSeq ].offset ].cRate;
                 conDisState = 0;
             }
        }
        if( curConDis.charge.cRate % 6 == 0 )
        {
            /* increase the color index */
            conDisState++;
        }
    }
    /* down brightness of the display area */
    else if( curConDis.charge.cRate < curConDis.charge.phase2 )
    {
        rgb2hsl( curConDis.charge.disColor[1], &chargeHsl );
        chargeHsl.lightness -= curConDis.charge.decreaseStep;
        hsl2rgb( chargeHsl, &curConDis.charge.disColor[1] );
//        conDisState = 1;

        /* complete the process , and return the complete flag */
        if( curConDis.charge.cRate == 0 )
        {
            return FALSE;
        }
    }

    /* decrease the cRate ,so as to control the the time of every phase */
    curConDis.charge.cRate--;

     /* display the the specified brightness color below the top row */
    for( disColumn = 0; disColumn < REN_PHY_FRAME_W; disColumn++ )
        for( disRow = 0; disRow < curConDis.charge.endRow; disRow ++ )
            PhyDis[ PhyDisLoadInd ].phyDisColor[ disColumn ][ disRow ] = curConDis.charge.disColor[ 1 ];

    /* display brightness color of top row */
    for( disColumn = 0; disColumn < REN_PHY_FRAME_W; disColumn++ )
        PhyDis[ PhyDisLoadInd ].phyDisColor[ disColumn ][ disRow ] = curConDis.charge.disColor[ conDisState ];

for( disColumn = 0; disColumn < REN_PHY_FRAME_W; disColumn++ )
    for( disRow += 1; disRow < REN_PHY_FRAME_H; disRow ++ )
        PhyDis[ PhyDisLoadInd ].phyDisColor[ disColumn ][ disRow ] = black;

    return TRUE;
}

/************************************************************************
 Function:    runFadeOut
 Description: display the fade out effect
 Input:       VOID
 Return:      TRUE : continue executing this function  FALSE: break executing
 Remarks:
************************************************************************/
LOCAL BOOL runFadeOut(VOID)
{
    UINT16 i, j;
    HSL_T hslColor;
    
    
    if( --curConDis.fadeOut.bRate == 0 )
    {
            return FALSE;
    }
    
     /* Update lightness value */
    for( j = 0; j < REN_PHY_FRAME_W; j++ )
    {
        for( i = 0; i < volDisHeight; i++ )
        {
            rgb2hsl( conDisBuf[ j ][ i ], &hslColor );
            hslColor.lightness -= curConDis.fadeOut.lStep;              
            hsl2rgb( hslColor, &conDisBuf[ j ][ i ] );
    
            PhyDis[ PhyDisLoadInd ].phyDisColor[ j ][ i ] = conDisBuf[ j ][ i ];
        }
    }

    return TRUE;
}


/************************************************************************
 Function:    rgb2hsl
 Description: Convert RGB to HSL encoding
 Input:       rgb:  RGB color
              hPtr: pointer to HSL structure
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID rgb2hsl(COLOR_T rgb, HSL_T *hPtr)
{
    UINT16 cMax, cMin;
    INT16  c1, c2, cDelta, cMaxInd;
    UINT32 temp;
 
    /* Determine cMax */
    cMax = rgb.g;
    cMaxInd = 1;
    if( rgb.b > cMax )
    {
        cMax = rgb.b;
        cMaxInd = 2;
    }

    if( rgb.r > cMax )
    {
        cMax = rgb.r;
        cMaxInd = 3;
    }

    /* Determine cMin */
    cMin = rgb.r;
    if( rgb.g < cMin )
        cMin = rgb.g;

    if( rgb.b < cMin )
        cMin = rgb.b;

    hPtr -> lightness = cMax + cMin;

    cDelta = cMax - cMin;
    if( cDelta == 0 )
    {
        hPtr -> hue = 0;
        hPtr -> saturation = 0;
    }
    else
    {
        if( hPtr -> lightness > 255 )
            hPtr -> saturation = ( cDelta * 512 ) / ( 510 - hPtr -> lightness );
        else
            hPtr -> saturation = ( cDelta * 512 ) / hPtr -> lightness;

        switch( cMaxInd )
        {
            case 1:     /* G is maximum */
                c1 = rgb.b;
                c2 = rgb.r;
                break;

            case 2:     /* B is maximum */
                c1 = rgb.r;
                c2 = rgb.g;
                break;

            case 3:     /* R is maximum */
                c1 = rgb.g;
                c2 = rgb.b;
                break;
        }
        temp = cMaxInd * cDelta * 120 + ( c1 - c2 ) * 60;
        hPtr -> hue = ( UINT16 )( temp / cDelta );
        if( hPtr -> hue >= 360 )
            hPtr -> hue -= 360;
    }
}

/************************************************************************
 Function:    hsl2rgb
 Description: Convert HSL to RGB encoding
 Input:       hsl:  HSL color
              cPtr: pointer to RGB structure
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID hsl2rgb(HSL_T hsl, COLOR_T *cPtr)
{
    INT16  deltaH, deltaC;
    UINT16 cMax, cMin;
    UINT32 delta;

    if( hsl.saturation == 0 )
    {
        /* All three colors are the same */
        if( hsl.lightness < 0 )
            cMax = 0;
        else
            cMax = hsl.lightness >> 1;
        cPtr -> r = ( UINT8 )cMax;
        cPtr -> g = ( UINT8 )cMax;
        cPtr -> b = ( UINT8 )cMax;
    }
    else
    {
        if( hsl.lightness > 510 )
            hsl.lightness = 510;
        else if( hsl.lightness < 0 )
            hsl.lightness = 0;

        if( hsl.saturation > 512 )
            hsl.saturation = 512;

        if( hsl.lightness > 255 )
            delta = hsl.saturation * ( 510 - hsl.lightness );
        else
            delta = hsl.saturation * hsl.lightness;

        delta /= 512;

        cMax = ( hsl.lightness + delta ) >> 1;
        cMin = ( hsl.lightness - delta ) >> 1;

        deltaH  = hsl.hue % 360;     /* Convert from 0 to 360 degree */
        if( deltaH < 0 )
            deltaH = 360 + deltaH;

        hsl.hue = deltaH;
        deltaH %= 60;
        deltaH  = deltaH + ( 60 - 2 * deltaH ) * ( ( hsl.hue / 60 ) % 2 );
        deltaC  = ( ( 60 - deltaH ) * cMin + deltaH * cMax ) / 60;

        switch( hsl.hue / 60 )
        {
            case 0:     /* 0 to 60 degree */
                cPtr -> r = cMax;
                cPtr -> g = ( UINT8 )deltaC;
                cPtr -> b = cMin;
                break;

            case 1:     /* 60 to 120 degree */
                cPtr -> r = ( UINT8 )deltaC;
                cPtr -> g = cMax;
                cPtr -> b = cMin;
                break;

            case 2:     /* 120 to 180 degree */
                cPtr -> r = cMin;
                cPtr -> g = cMax;
                cPtr -> b = ( UINT8 )deltaC;
                break;

            case 3:     /* 180 to 240 degree */
                cPtr -> r = cMin;
                cPtr -> g = ( UINT8 )deltaC;
                cPtr -> b = cMax;
                break;

            case 4:     /* 240 to 300 degree */
                cPtr -> r = ( UINT8 )deltaC;
                cPtr -> g = cMin;
                cPtr -> b = cMax;
                break;

            case 5:     /* 300 to 360 degree */
                cPtr -> r = cMax;
                cPtr -> g = cMin;
                cPtr -> b = ( UINT8 )deltaC;
                break;
        }
    }
}

/************************************************************************
 Function:    addColor
 Description: Add two colors
 Input:       c1:   1st color
              c2:   2nd color
              cPtr: pointer to resultant color
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID addColor(COLOR_T c1, COLOR_T c2, COLOR_T *cPtr)
{
    UINT16 addC;

    addC = c1.r + c2.r;
    if( addC > MAX_RGB )
        cPtr -> r = MAX_RGB;
    else
        cPtr -> r = addC;

    addC = c1.g + c2.g;
    if( addC > MAX_RGB )
        cPtr -> g = MAX_RGB;
    else
        cPtr -> g = addC;

    addC = c1.b + c2.b;
    if( addC > MAX_RGB )
        cPtr -> b = MAX_RGB;
    else
        cPtr -> b = addC;
}

/************************************************************************
 Function:    colorOpFormula
 Description: Calculate the formula:
              Result = ( operandA oc0 operandB ) oc1 operandC   (1) or
              Result = operandA oc0 ( operandB oc1 operandC )   (2)
 Input:       operand0: the first operand
              operand1: the second operand
              operand2: the third operand
              oc0:      the first operation code
              oc1:      the second operation code
              ocOption: REN_CO_OC0_FIRST: execute (1); otherwise: execute (2)
                        REN_CO_OC0_SWAP: swap operands for oc0 operation
                        REN_CO_OC1_SWAP: swap operands for oc0 operation
 Return:      Result
 Remarks:
************************************************************************/
LOCAL INT16 colorOpFormula(INT16 operand0, INT16 operand1, INT16 operand2, UINT16 oc0, UINT16 oc1, UINT8 ocOption)
{
    INT32 result;

    if( ocOption & REN_CO_OC_FIRST_MASK )
    {
        /* Execute formula (1) */
        if( ocOption & REN_CO_OC0_SWAP_MASK )
            result = arithmetic( operand1, operand0, oc0 );
        else
            result = arithmetic( operand0, operand1, oc0 );

        if( ocOption & REN_CO_OC1_SWAP_MASK )
            result = arithmetic( operand2, ( INT16 )result, oc1 );
        else
            result = arithmetic( ( INT16 )result, operand2, oc1 );
    }
    else
    {
        /* Execute formula (2) */
        if( ocOption & REN_CO_OC1_SWAP_MASK )
            result = arithmetic( operand2, operand1, oc1 );
        else
            result = arithmetic( operand1, operand2, oc1 );

        if( ocOption & REN_CO_OC0_SWAP_MASK )
            result = arithmetic( ( INT16 )result,operand0, oc0 );
        else
            result = arithmetic( operand0, ( INT16 )result, oc0 );
    }

    return ( INT16 )result;
}

/************************************************************************
 Function:    arithmetic
 Description: Calculate an arithmetic operation
              Result = operand0 oc operand1
 Input:       operand0: the first operand
              operand1: the second operand
              oc:       the operation code
 Return:      Result
 Remarks:
************************************************************************/
LOCAL INT16 arithmetic(INT16 operand0, INT16 operand1, UINT16 oc)
{
    INT32 result;

    switch( oc )
    {
        case moSub:
            result = operand0 - operand1;
            break;
        case moMul:
            result = operand0 * operand1;
            break;

        case moDiv:
            result = operand0 / operand1;
            break;

        case moAdd:
        default:
            result = operand0 + operand1;
            break;
    }

    return ( INT16 )result;
}

/************************************************************************
 Function:    mapNormalJetThemeColor
 Description: Map cross theme coordinate into another coordinate
 *            In simulator cross particle moves from left to right or right
 *            to left. Through this map function, we could let particle moves
 *            from center column to edge and round back from another edge.
 Input:       pos:   a location vector, in the simulator coordinates
              color: the color for display
              dInd:  the virtual display index
              psInd: particle system index
              vel:   velocity of a particle
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID mapNormalJetThemeColor(PVECTOR_ST pos, COLOR_T color, UINT16 dInd, UINT16 psInd, PVECTOR_ST vel)
{
    int       TARGET_X;
    const int GLOOM_LEN = 4;
    int       nDist  = -1;
    //int       nIdx;
    int       nLightnessPara;
    HSL_T     hsl;
    
    if(vel.x < 0)
    {
        if(96 > pos.x && pos.x >= -16)
        {//map col 5~11 to 0~6
            pos.x -= 80;
        }
        else if(-16 >pos.x &&pos.x >= -96)
        {//map col 0~4 to 7~11
            pos.x += 112;
        }
        else
        {
            return;
        }
        TARGET_X = 16;
        nDist = pos.x - TARGET_X;
    }
    else
    {
        
        if(96 > pos.x && pos.x >= 0)
        {//map col 6~11 to 0~5
            pos.x -= 96;
        }
        else if(0 >pos.x &&pos.x >= -96)
        {//map col 0~5 to 6~11
            pos.x += 96;
        }
        else
        {
            return;
        }
        TARGET_X = 31;
        nDist = TARGET_X - pos.x;
    }

    if(nDist>=0)
    {
        nDist /= REN_FRAME2VIR_FRAME;
        if(nDist < GLOOM_LEN)
        {
            //nDist = GLOOM_LEN - nDist;
            rgb2hsl( color, &hsl );
            nLightnessPara = hsl.lightness / (GLOOM_LEN);
            //for(nIdx = 0; nIdx < nDist; nIdx++)
                //hsl.lightness /= 2;
            hsl.lightness = (nLightnessPara * (nDist + 1));
            hsl2rgb( hsl, &color );
        }
    }
    vel.x = 0; vel.y = 0;
    mapLocationColor(pos, color, dInd, psInd, vel);
}

void CalulatePhyPos(INT16 X, INT16 Y, PVECTOR_ST *pPhyPos)
{
    const INT16 compensation = SIM128_TO_VIR96 * 2;
    //assume -8~+7 match to one real pixel, 
    //need move 8 unit to align
    X += 8;
    Y += 8;
    pPhyPos->x = (X - (X + compensation) % REN_FRAME2VIR_FRAME);
    pPhyPos->y = (Y - (Y + compensation) % REN_FRAME2VIR_FRAME);
}

void CountAreaByLine(INT16 locX, INT16 locY, CycleEdgeInfo *pEdgeInfo)
{
    INT16   OutX, BottomY, InX;
     //assume -8~+7 match to one real pixel, 
    OutX    = pEdgeInfo->m_loc_x + 7;
    InX     = pEdgeInfo->m_loc_x - 8;
    BottomY = pEdgeInfo->m_loc_y - 8;
    
    //if first time cut the box edge
    if( COUNT_FIRST_EDGE_FLAG != (pEdgeInfo->m_flag & COUNT_FIRST_EDGE_FLAG) &&
        locX >= OutX && locY >= BottomY)
    {//count all lines under current line
        //assert locY >= BottomY
        for(;BottomY <= locY; BottomY++)
        {//each full line have 16 count
            pEdgeInfo->m_count += REN_FRAME2VIR_FRAME;
        }
        pEdgeInfo->m_flag |= COUNT_FIRST_EDGE_FLAG;
    }
    else
    {
        pEdgeInfo->m_count += (locX - InX  + 1);
    }
}
void WriteEdgePoint(INT16 x0, INT16 DeltX, INT16 y0, INT16 DeltY, int *pEdgIdx, BOOL bLineHead)
{
    PVECTOR_ST  PhyPos;
    PVECTOR_ST  PhyPosYX;
    PVECTOR_ST  PhyPosX_Y;
    CycleEdgeInfo   *pEdgeInfo;
    
    //index should be 0~REN_VIR_DIS_FRAME-1
    if(*pEdgIdx >= HALF_CYCLE_EDGE_COUNT && *pEdgIdx < -1)
        return;
    
    pEdgeInfo = &(CycleEdgeList[*pEdgIdx]);

    //putpixel(x0 + x, y0 + y);
    CalulatePhyPos( x0+DeltX, y0+DeltY, &PhyPos);
    //putpixel(x0 + y, y0 + x);
    CalulatePhyPos( x0+DeltY, y0+DeltX, &PhyPosYX);
    //putpixel(x0 + x, y0 - y);
    CalulatePhyPos( x0+DeltX, y0-DeltY, &PhyPosX_Y);
    
    if(*pEdgIdx == -1 ||
       (PhyPos.x != pEdgeInfo->m_loc_x ||
        PhyPos.y != pEdgeInfo->m_loc_y))
    {//move to next Phy pixel
        (*pEdgIdx)++;
        if(*pEdgIdx >= HALF_CYCLE_EDGE_COUNT)
            return;
        pEdgeInfo ++;

        pEdgeInfo->m_count = 0;
        pEdgeInfo->m_flag  = 0;
        pEdgeInfo->m_loc_x = PhyPos.x;
        pEdgeInfo->m_loc_y = PhyPos.y;
    }
    
    if(bLineHead)
    {
        CountAreaByLine( x0+DeltX, y0+DeltY, pEdgeInfo);
        //special process for first&last pixel
        if(PhyPosYX.x == PhyPos.x && 
           PhyPosYX.y == PhyPos.y)
        {//last pixel, symmetrical pixel
            if(x0+DeltY != x0+DeltX &&
               y0+DeltX != y0+DeltY)
                CountAreaByLine( x0+DeltY, y0+DeltX, pEdgeInfo);
        }
        else if(PhyPosX_Y.x == PhyPos.x &&
                PhyPosX_Y.y == PhyPos.y)
        {//first 1 or 2 pixel, symmetrical pixel
            if(COUNT_FIRST_EDGE_FLAG == (pEdgeInfo->m_flag & COUNT_FIRST_EDGE_FLAG))
            {
                //256 - pEdgeInfo->m_count: rest area
                //(rest area)*2 :total rest area(because of symmetrical image )
                //256 - (total rest area)):valid area
                pEdgeInfo->m_count = (256 - (256 - pEdgeInfo->m_count) * 2);
            }
            else if(y0-DeltY != y0+DeltY)
            {
                CountAreaByLine( x0+DeltX, y0-DeltY, pEdgeInfo);
            }
        }
    }
}

int DrawCycle(int x0, int y0, int radius)
{
    int x;
    int y = 0;
    int err = 0;
    
    int     EdgIndx     = -1;
    BOOL    bLineHead   = TRUE;
    x = radius;

    while (x >= y)
    {

        WriteEdgePoint(x0, x, y0, y, &EdgIndx, bLineHead);
        bLineHead = FALSE;
        if (err <= 0)
        {
            y += 1;
            err += 2*y + 1;
            bLineHead = TRUE;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }
    }
    return EdgIndx + 1;
}


int MakeHalfCycle(INT16 x0, INT16 y0, int nCycleEdgeCount)
{
    int nIdx;
    int nCpyIdx;
    INT16   nDeltX, nDeltY;
    
    if(nCycleEdgeCount + nCycleEdgeCount > MAX_CYCLE_EDGE_COUNT)
        return 0;
    //make 1/4 cycle from 1/8
    for(nIdx = nCycleEdgeCount - 1 , nCpyIdx = nCycleEdgeCount; 
        nIdx >= 0; 
        nIdx--)
    {
        nDeltX = CycleEdgeList[nIdx].m_loc_x - x0;
        nDeltY = CycleEdgeList[nIdx].m_loc_y - y0;

        CycleEdgeList[nCpyIdx].m_loc_x = x0 + nDeltY;
        CycleEdgeList[nCpyIdx].m_loc_y = y0 + nDeltX;
        CycleEdgeList[nCpyIdx].m_count = CycleEdgeList[nIdx].m_count;
        if(CycleEdgeList[nCpyIdx].m_loc_x != CycleEdgeList[nIdx].m_loc_x ||
           CycleEdgeList[nCpyIdx].m_loc_y != CycleEdgeList[nIdx].m_loc_y)
        {
            nCpyIdx++;
        }
    }
    nCycleEdgeCount = nCpyIdx;
    
    if(nCycleEdgeCount + nCycleEdgeCount > MAX_CYCLE_EDGE_COUNT)
        return 0;
    //make 1/2 cycle from 1/4
    for(nIdx = nCycleEdgeCount - 1 , nCpyIdx = nCycleEdgeCount; 
        nIdx >= 0; 
        nIdx--)
    {
        nDeltX = CycleEdgeList[nIdx].m_loc_x - x0;

        CycleEdgeList[nCpyIdx].m_loc_x = x0 - nDeltX;
        CycleEdgeList[nCpyIdx].m_loc_y = CycleEdgeList[nIdx].m_loc_y;
        CycleEdgeList[nCpyIdx].m_count = CycleEdgeList[nIdx].m_count;
        if(CycleEdgeList[nCpyIdx].m_loc_x != CycleEdgeList[nIdx].m_loc_x)
        {
            nCpyIdx++;
        }
    }
    nCycleEdgeCount = nCpyIdx;
    return nCycleEdgeCount;
}

VOID mapBackAroundColor(INT16 x,INT16 y, COLOR_T color, UINT16 dInd, UINT16 psInd)
{
    PVECTOR_ST  pos, vel;
    
    vel.x = 0; vel.y = 0;

    if(x < -96)
    {//map x < col 0 back around to col 11
        x += 192;
    }
    else if(x >= 96)
    {//map x > col 11 back around to col 0
        x -= 192;
    }
    pos.x = x;
    pos.y = y;
    mapLocationColor(pos, color, dInd, psInd ,vel);
}

VOID mapExplosionColor(INT16 y0, int nBegin, int nEnd, COLOR_T color, UINT16 dInd, UINT16 psInd)
{
    INT16   nDeltY, nBottomY;
    INT16   nInnerIdx;
    INT16   x,y;
    int     nIdx, nRowIdx, nStep;
    HSL_T       hsl;
    COLOR_T     EdgeColor;
    
    nStep = (nBegin <= nEnd ? 1 : -1);
    nEnd += nStep;
    
    for(nInnerIdx=nIdx=nBegin; nIdx!= nEnd; nIdx+=nStep)
    {
        rgb2hsl(color, &hsl);
        hsl.lightness = (INT16)(((INT32)hsl.lightness) * ((INT32)(CycleEdgeList[nIdx].m_count)) / 256);
        hsl2rgb(hsl, &EdgeColor);
        
        nDeltY = CycleEdgeList[nIdx].m_loc_y - y0;
        if(0 != nDeltY)
        {
            nBottomY =  y0 - nDeltY;
            if(CycleEdgeList[nIdx].m_loc_x != CycleEdgeList[nInnerIdx].m_loc_x)
            {
                nInnerIdx = nIdx;
            }
            //if(nInnerIdx == nIdx)
            {
                //draw first
                x = CycleEdgeList[nIdx].m_loc_x;
                y = CycleEdgeList[nIdx].m_loc_y;
                mapBackAroundColor(x, y, EdgeColor, dInd, psInd);
                //fill middle
                if(nInnerIdx == nIdx)
                {
                    nRowIdx = CycleEdgeList[nIdx].m_loc_y - REN_FRAME2VIR_FRAME;
                    for(; nRowIdx > nBottomY; nRowIdx-=REN_FRAME2VIR_FRAME)
                    {
                        y = nRowIdx;
                        mapBackAroundColor(x, y, color, dInd, psInd);
                    }
                }
                //draw last
                y = nBottomY;
                mapBackAroundColor(x, y, EdgeColor, dInd, psInd);
            }
            /*else
            {//only draw outside pixel
                pos.x = CycleEdgeList[nIdx].m_loc_x;
                pos.y = CycleEdgeList[nIdx].m_loc_y;
                mapLocationColor(pos, color, dInd, psInd, vel);
                
                pos.y = y0 - nDeltY;
                mapLocationColor(pos, color, dInd, psInd, vel);
            }*/
        }
        else
        {//only draw one pixel
            x = CycleEdgeList[nIdx].m_loc_x;
            y = CycleEdgeList[nIdx].m_loc_y;
            mapBackAroundColor(x, y, EdgeColor, dInd, psInd);
        }
    }
}

void DrawExplosion(PARTICLE_ST *pPtr, COLOR_T cColor, UINT16 dInd, UINT16 psInd)
{
    const INT16 SINGLE_PIXEL_R = REN_FRAME2VIR_FRAME * 3 / 4;
    int     nCycleEdgeCount;
    int     nHalfCount;
    INT16   x0, y0;
    INT16   radius;
    INT16   nPeak;
    HSL_T   hsl;
    int     nHueLeft;
    int     nHueRight;

    PARTICLE_SYSTEM_ST *psPtr = &SimPS[ psInd ];
    
    x0 = pPtr->location.x;
    y0 = pPtr->location.y;
    
    nPeak = ((INT16)(pPtr->angle)) * 90 / 100;
    rgb2hsl( cColor, &hsl );
    if(pPtr -> lifespan >= nPeak)
    {
        radius = pPtr->size *(pPtr->angle - pPtr->lifespan) / (pPtr->angle - nPeak);
    }
    else
    {
        radius = pPtr->size * (pPtr->lifespan) / nPeak;
        
        hsl.lightness = hsl.lightness * (pPtr->lifespan) / nPeak;
        /*if(hsl.lightness < 24)
            hsl.lightness = 24;*/
    }
   
    //process hue, random in some range based on current color
    nHueLeft = (psPtr->psBase.aAmpMax - psPtr->psBase.aAmpMin + 1) / 3;
    nHueRight= nHueLeft + nHueLeft;
    if(pPtr->aAmplitude <= nHueLeft)
        hsl.hue -= 18;
    else if(pPtr->aAmplitude <= nHueRight)
        hsl.hue += 18;
    
    if(hsl.hue >= 360)
        hsl.hue -= 360;
    if(hsl.hue < 0)
        hsl.hue += 360;
    
    if(radius <= SINGLE_PIXEL_R)
    {//only one pixel
        hsl.lightness = hsl.lightness * radius / (SINGLE_PIXEL_R);
        hsl2rgb(hsl, &cColor);

        mapBackAroundColor(x0, y0, cColor, dInd, psInd);
        return;
    }
    hsl2rgb(hsl, &cColor);
    //location must align with PHY pos
    nCycleEdgeCount = DrawCycle(x0, y0, radius);
    nCycleEdgeCount = MakeHalfCycle(x0, y0, nCycleEdgeCount);
    //rend out
    //nInnerIdx = 0;
    nHalfCount  =   nCycleEdgeCount/2;
    nHalfCount  +=  (nCycleEdgeCount%2);
    //from begin to middle
    mapExplosionColor(y0, 0, nHalfCount-1, cColor, dInd, psInd);
    //from end to middle
    mapExplosionColor(y0, nCycleEdgeCount - 1, nHalfCount, cColor, dInd, psInd);
}

VOID SetWaveThemeColor(VOID)
{
    const int HUE_DIFF = 30;
    int       nIdx;
    HSL_T     tmpHsl;
    COLOR_T   tmpColor;
    
    if(APP_COLOR == ThemeColorMode)
    {
        rgb2hsl(ThemeColor, &tmpHsl);

        tmpHsl.hue += (HUE_DIFF * 2);
        if(tmpHsl.hue >= 360)
            tmpHsl.hue -= 360;

        for(nIdx = 0; nIdx < 4; nIdx++)
        {
            hsl2rgb(tmpHsl, &tmpColor);
            colorMap[nIdx] = tmpColor;

            tmpHsl.hue -= HUE_DIFF;
            if(tmpHsl.hue < 0)
                tmpHsl.hue += 360;
        }
    }
}

VOID SetJetThemeNormalColor(VOID)
{
    const int HUE_DIFF = 20;
    int       nIdx;
    HSL_T     tmpHsl;
    COLOR_T   tmpColor;
    
    if(APP_COLOR == ThemeColorMode)
    {
        rgb2hsl(ThemeColor, &tmpHsl);
        
        tmpHsl.hue += HUE_DIFF;
        if(tmpHsl.hue >= 360)
            tmpHsl.hue -= 360;

        for(nIdx = 0; nIdx < REN_PHY_FRAME_H; nIdx++)
        {
            hsl2rgb(tmpHsl, &tmpColor);
            colorMap[nIdx] = tmpColor;
            
            tmpHsl.hue -= HUE_DIFF;
            if(tmpHsl.hue < 0)
                tmpHsl.hue += 360;
        }
    }
}

LOCAL VOID RainBowUpdateDefaultHues( RAINBOWHUE_T HuesArray[], UINT8 count, INT16 hueVar,  INT16 maxHue, BOOL hueInc )
{
    UINT8 index = 0;
    for( ; index < count; index++ )
    {
        if( hueInc )
        {
            HuesArray[index].currHue       = HuesArray[index].targetHue;
            HuesArray[index].targetHue    += hueVar;
            HuesArray[index].targetHue    %= maxHue;
        }
        else
        {
            HuesArray[index].currHue       = HuesArray[index].targetHue;
            HuesArray[index].targetHue    -= hueVar;
            if( HuesArray[index].targetHue < 0 )
                HuesArray[index].targetHue = maxHue + HuesArray[index].targetHue;
        }
    }
}

LOCAL VOID RainBowUpdatePickHues( RAINBOWHUE_T HuesArray[],
                                         UINT8 count,
                                         INT16 hueVar,
                                         INT16 maxHue,
                                         UINT8 hueInc,
                                         UINT8 currPickInd,
                                         INT16 pickHue,
                                         INT16 cvtPickHue )
{
    UINT8 index = 0;
    UINT8 pickOffset;
    for( ; index < count; index++ )
    {
        HuesArray[index].currHue = HuesArray[index].targetHue;
        pickOffset               = currPickInd > index ? currPickInd - index : index - currPickInd ;
        pickOffset              *= hueVar;
        if( !hueInc )
        {
            HuesArray[index].targetHue    = pickHue + pickOffset;
            HuesArray[index].targetHue   %= maxHue;
        }
        else
        {
            HuesArray[index].targetHue    = cvtPickHue >= pickOffset ? cvtPickHue - pickOffset : maxHue + cvtPickHue - pickOffset;
        }
    }
}

LOCAL VOID RefreshNormalRainBow( UINT16 dInd )
{
    RB_COLOR_DIR_E colorDir;
    COLOR_T        color;
    HSL_T          hslC;
    UINT8          row;
    UINT8          col;
    PVECTOR_ST     VirPos;

    hslC.lightness  = g_rb_BaseHSL.lightness;
    hslC.saturation = g_rb_BaseHSL.saturation;
    switch( g_rb_Dir )
    {
    case RB_BOTT_UP:
    case RB_UP_BOTT:
        for( row = 0; row < REN_COLOR_OPERATION_NUM; row++ )
        {
            colorDir  = RB_COLOR_KEEP;
            if( g_rb_HoriHues[row].currHue == g_rb_HoriHues[row].targetHue )
            {
                //do nothing.
            }
            else
            {
                if( g_rb_HoriHues[row].targetHue - g_rb_HoriHues[row].currHue  > MAXHUE/3 )
                {
                      colorDir = RB_COLOR_DEC;
                }
                else if( g_rb_HoriHues[row].currHue - g_rb_HoriHues[row].targetHue > MAXHUE/3 )
                {
                      colorDir = RB_COLOR_INC;
                }
                else if( g_rb_HoriHues[row].currHue < g_rb_HoriHues[row].targetHue )
                {
                    colorDir = RB_COLOR_INC;
                }
                else
                    colorDir = RB_COLOR_DEC;
            }
            
            switch( colorDir )
            {
            case RB_COLOR_INC:
                if( g_bPickColor )
                {
                    g_rb_HoriHues[row].currHue += RB_PICK_HORI_INC;
                }
                else
                {
                    g_rb_HoriHues[row].currHue += RB_HORI_INC;
                }
                g_rb_HoriHues[row].currHue %= MAXHUE;
                break;
            case RB_COLOR_DEC:                
                if( g_bPickColor )
                {
                    g_rb_HoriHues[row].currHue -= RB_PICK_HORI_INC;
                }
                else
                {
                    g_rb_HoriHues[row].currHue -= RB_HORI_INC;
                }
                if( g_rb_HoriHues[row].currHue < 0 )
                    g_rb_HoriHues[row].currHue += MAXHUE; 
                break;
            default:
                break;
            }

            hslC.hue = g_rb_HoriHues[row].currHue;
            hsl2rgb( hslC, &color );
            for( col = 0; col < REN_VIR_DIS_FRAME; col++ )
            {
                VirPos.x = col;
                VirPos.y = row + ROWOFFSET;
                mapVirLocationColor( VirPos, color , dInd );
            }
        }
        break;
    case RB_RIGHT_LEFT:
    case RB_LEFT_RIGHT:
        for( col = 0; col < REN_VIR_DIS_FRAME; col++ )
        {
            colorDir  = RB_COLOR_KEEP;
            if( g_rb_VertHues[col].currHue == g_rb_VertHues[col].targetHue )
            {
                // do nothing.
            }
            else
            {
                if( g_rb_VertHues[col].targetHue - g_rb_VertHues[col].currHue  > MAXHUE/3 )
                {
                      colorDir = RB_COLOR_DEC;
                }
                else if( g_rb_VertHues[col].currHue - g_rb_VertHues[col].targetHue > MAXHUE/3 )
                {
                      colorDir = RB_COLOR_INC;
                }
                else if( g_rb_VertHues[col].currHue < g_rb_VertHues[col].targetHue )
                {
                    colorDir = RB_COLOR_INC;
                }
                else
                    colorDir = RB_COLOR_DEC;
            }
            switch( colorDir )
            {
            case RB_COLOR_INC:
                if( g_bPickColor )
                {
                    g_rb_VertHues[col].currHue += RB_PICK_VERT_INC;
                }
                else
                {
                    g_rb_VertHues[col].currHue += RB_VERT_INC;
                }
                g_rb_VertHues[col].currHue %= MAXHUE;
                break;
            case RB_COLOR_DEC:                
                if( g_bPickColor )
                {
                    g_rb_VertHues[col].currHue -= RB_PICK_VERT_INC;
                }
                else
                {
                    g_rb_VertHues[col].currHue -= RB_VERT_INC;
                }
                if( g_rb_VertHues[col].currHue < 0 )
                    g_rb_VertHues[col].currHue += MAXHUE; 
                break;
            default:
                break;
            }
            hslC.hue = g_rb_VertHues[col].currHue;
            hsl2rgb( hslC, &color );
            for( row = 0; row < REN_COLOR_OPERATION_NUM; row++ )
            {
                VirPos.x = col;
                VirPos.y = row + ROWOFFSET;
                mapVirLocationColor( VirPos, color , dInd );
            }  
        }
        break;
    default:
        break;
    }   
    switch( g_rb_Dir )
    {
    case RB_BOTT_UP:
    case RB_UP_BOTT:
        if( ( g_bPickColor && g_rb_RunTimes > RB_PICK_HORI_DIR_VAR ) ||
            ( !g_bPickColor && g_rb_RunTimes > RB_HORI_DIR_VAR ) )
        {
            g_rb_RunTimes = 0;
            g_rb_SwtTimes = 0;
            if( RB_BOTT_UP == g_rb_Dir )
                g_rb_Dir              = RB_RIGHT_LEFT;
            else
                g_rb_Dir              = RB_LEFT_RIGHT;
        }
        
        break;
    case RB_RIGHT_LEFT:
    case RB_LEFT_RIGHT:
        if( ( g_bPickColor && g_rb_RunTimes > RB_PICK_VERT_DIR_VAR ) ||
            ( !g_bPickColor && g_rb_RunTimes > RB_VERT_DIR_VAR ) )
        {
            g_rb_RunTimes = 0;
            g_rb_SwtTimes = 0;            
            if( RB_RIGHT_LEFT == g_rb_Dir )
                g_rb_Dir              = RB_UP_BOTT;
            else
                g_rb_Dir              = RB_BOTT_UP;
        }
    default:
        break;
    }

    g_rb_SwtTimes++;
    switch( g_rb_Dir )
    {
    case RB_BOTT_UP:
    case RB_UP_BOTT: 
        if( g_bPickColor )
        {
            if( g_rb_SwtTimes > RB_PICKC_H_VAR )
            {
               g_rb_SwtTimes = 0;
               g_rb_RunTimes++;
               if( RB_BOTT_UP == g_rb_Dir )
                {
                    g_rb_CurrPickIndex++;
                    if( g_rb_CurrPickIndex >= REN_COLOR_OPERATION_NUM )
                    {
                        g_rb_CurrPickIndex = 0;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }
                else
                {
                    g_rb_CurrPickIndex--;
                    if( g_rb_CurrPickIndex < 0 )
                    {
                        g_rb_CurrPickIndex = REN_COLOR_OPERATION_NUM - 1;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }

                RainBowUpdatePickHues( g_rb_HoriHues,
                                       REN_COLOR_OPERATION_NUM,
                                       RB_PICKC_H_VAR,
                                       MAXHUE,
                                       g_rb_PickColorDir,
                                       g_rb_CurrPickIndex,
                                       g_rb_PickColorHue,
                                       g_rb_CvtHoriPickColorHue );
            }
        }
        else
        {
            if( g_rb_SwtTimes > RB_HORI_VAR )
            {
                g_rb_SwtTimes = 0;
                g_rb_RunTimes++;
                RainBowUpdateDefaultHues( g_rb_HoriHues,
                                          REN_COLOR_OPERATION_NUM,
                                          RB_HORI_VAR,
                                          MAXHUE,
                                          RB_UP_BOTT == g_rb_Dir );
            }                
        }
        break;
    case RB_LEFT_RIGHT:
    case RB_RIGHT_LEFT:  
        if( g_bPickColor )
        {
            if( g_rb_SwtTimes > RB_PICKC_V_VAR )
            {
                g_rb_SwtTimes = 0;
                g_rb_RunTimes++;
                if( RB_LEFT_RIGHT == g_rb_Dir )
                {
                    g_rb_CurrPickIndex--;
                    if( g_rb_CurrPickIndex < 0 )
                    {
                        g_rb_CurrPickIndex = REN_VIR_DIS_FRAME - 1;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }
                else
                {
                    g_rb_CurrPickIndex++;
                    if( g_rb_CurrPickIndex >= REN_VIR_DIS_FRAME )
                    {
                        g_rb_CurrPickIndex = 0;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }

                RainBowUpdatePickHues( g_rb_VertHues,
                                       REN_VIR_DIS_FRAME,
                                       RB_PICKC_V_VAR,
                                       MAXHUE,
                                       g_rb_PickColorDir,
                                       g_rb_CurrPickIndex,
                                       g_rb_PickColorHue,
                                       g_rb_CvtVertPickColorHue );
            }
        }
        else
        {
            if( g_rb_SwtTimes > RB_VERT_VAR )
            {
                g_rb_SwtTimes = 0;
                g_rb_RunTimes++;
                RainBowUpdateDefaultHues( g_rb_VertHues,
                                          REN_VIR_DIS_FRAME,
                                          RB_VERT_VAR,
                                          MAXHUE,
                                          RB_LEFT_RIGHT == g_rb_Dir );
            }
        }
        break;
    default:
        break;
    }
}
#if 0
LOCAL VOID RefreshEQRainBow( UINT16 dInd )
{
    LOCAL INT8            lowLevel      = 0;
    LOCAL INT8            hiLevel       = 0;
    LOCAL INT8            pullBackStep  = RB_EQ_HORI_PULLBACK_LOW;
    LOCAL INT16           hiSwitchStep  = RB_EQ_DIR_SWT_VAR;
    LOCAL RB_COLOR_DIR_E  colorDir;
    COLOR_T               color;
    HSL_T                 hslC;
    UINT8                 row;
    UINT8                 col;
    UINT8                 pickOffset;
    INT16                 lowFre        = 0;
    INT16                 hiFre         = 0;
    
    PVECTOR_ST            VirPos;

    hslC.lightness  = g_rb_BaseHSL.lightness;
    hslC.saturation = g_rb_BaseHSL.saturation;

    //g_rb_Dir = RB_RIGHT_LEFT;
    if( hiSwitchStep > 0 )
        hiSwitchStep--;

    if(  RB_EQ_HORI_PULLBACK_LOW >= pullBackStep )
    {
        GetDbValues( g_DbValAry );

        for( col = 1 ; col < REN_VIR_DIS_FRAME - 1; col++ )
        {
            if( col < 4 )
                lowFre += g_DbValAry[col];
            else if ( col > 7 )
                hiFre  += g_DbValAry[col];
        }
        lowFre  /= RB_EQ_FREQ_WIDTH;
        hiFre   /= RB_EQ_FREQ_WIDTH;
        lowLevel = -1;
        hiLevel  = -1;
        for( col = 1 ; col < _AK7755_SP_ANA_VAL_SIZE; col+=2 )
        {        
            if( lowLevel >= 0 && hiLevel >= 0 )
                break;
            if( lowLevel < 0 && lowFre >= sp_AnaMap[col].dbVal )
            {
                lowLevel = (_AK7755_SP_ANA_VAL_SIZE - 1 - col) >> 1;
            }
            if( hiLevel < 0 && hiFre >= sp_AnaMap[col].dbVal )
            {
                hiLevel = (_AK7755_SP_ANA_VAL_SIZE - 1 - col) >> 1;
            }        
        }
        lowLevel    -= 1;
        hiLevel     -= 1;
        pullBackStep = RB_EQ_HORI_PULLBACK_LOW;
    }
    if( ( lowLevel > 0  ) && ( RB_EQ_HORI_PULLBACK_LOW >= pullBackStep ) )
    {
        switch( g_rb_Dir )
        {
        case RB_BOTT_UP:
        case RB_UP_BOTT:
            if( g_bPickColor )
            {
                lowLevel = lowLevel > RB_EQ_HORI_MAX_PULLBACK ? RB_EQ_HORI_MAX_PULLBACK : lowLevel;
                for( row = 0; row < lowLevel; row++ )
                {
                    if( RB_BOTT_UP == g_rb_Dir )
                    {
                        g_rb_CurrPickIndex--;
                        if( g_rb_CurrPickIndex < 0 )
                        {
                            g_rb_CurrPickIndex  = REN_COLOR_OPERATION_NUM - 1;
                            g_rb_PickColorDir   = g_rb_PickColorDir ? 0 : 1;
                        }
                    }
                    else
                    {
                        g_rb_CurrPickIndex++;
                        if( g_rb_CurrPickIndex >= REN_COLOR_OPERATION_NUM )
                        {
                            g_rb_CurrPickIndex = 0;
                            g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                        }
                    }
                    RainBowUpdatePickHues( g_rb_HoriHues,
                                           REN_COLOR_OPERATION_NUM,
                                           RB_PICKC_H_VAR,
                                           MAXHUE,
                                           g_rb_PickColorDir,
                                           g_rb_CurrPickIndex,
                                           g_rb_PickColorHue,
                                           g_rb_CvtHoriPickColorHue );
                }
            }
            else
            {
                pullBackStep = RB_HORI_VAR;
                lowLevel = lowLevel > RB_EQ_HORI_MAX_PULLBACK ? RB_EQ_HORI_MAX_PULLBACK : lowLevel;
                for( row = 0; row < lowLevel; row++ )
                {
                    RainBowUpdateDefaultHues( g_rb_HoriHues,
                                              REN_COLOR_OPERATION_NUM,
                                              RB_HORI_VAR,
                                              MAXHUE,
                                              RB_BOTT_UP != g_rb_Dir);
                }
            }
            g_rb_SwtTimes = RB_PICKC_H_VAR - 2 * RB_EQ_HORI_SPEED;
            break;
        case RB_RIGHT_LEFT:
        case RB_LEFT_RIGHT:
            if( g_bPickColor )
            {
                lowLevel = lowLevel > RB_EQ_VERT_MAX_PULLBACK ? RB_EQ_VERT_MAX_PULLBACK : lowLevel;
                for( col = 0; col < lowLevel; col++ )
                {
                    if( RB_RIGHT_LEFT == g_rb_Dir )
                    {
                        g_rb_CurrPickIndex--;
                        if( g_rb_CurrPickIndex < 0 )
                        {
                            g_rb_CurrPickIndex  = REN_VIR_DIS_FRAME - 1;
                            g_rb_PickColorDir   = g_rb_PickColorDir ? 0 : 1;
                        }
                    }
                    else
                    {
                        g_rb_CurrPickIndex++;
                        if( g_rb_CurrPickIndex >= REN_VIR_DIS_FRAME )
                        {
                            g_rb_CurrPickIndex = 0;
                            g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                        }
                    }
                    RainBowUpdatePickHues( g_rb_VertHues,
                                           REN_VIR_DIS_FRAME,
                                           RB_PICKC_V_VAR,
                                           MAXHUE,
                                           g_rb_PickColorDir,
                                           g_rb_CurrPickIndex,
                                           g_rb_PickColorHue,
                                           g_rb_CvtVertPickColorHue );
                }                
            }
            else
            {
                pullBackStep = RB_VERT_VAR;
                lowLevel = lowLevel > RB_EQ_VERT_MAX_PULLBACK ? RB_EQ_VERT_MAX_PULLBACK : lowLevel;
                for( row = 0; row < lowLevel; row++ )
                {
                    RainBowUpdateDefaultHues( g_rb_VertHues,
                                              REN_VIR_DIS_FRAME,
                                              RB_VERT_VAR,
                                              MAXHUE,
                                              RB_LEFT_RIGHT != g_rb_Dir);
                }
            }
            g_rb_SwtTimes = RB_PICKC_V_VAR - 2 * RB_EQ_VERT_SPEED;
            break;
        }
    }
    else if( hiLevel > 0 && hiSwitchStep <= 0 )
    {
        hiSwitchStep = RB_EQ_DIR_SWT_VAR;
        switch( g_rb_Dir )
        {
        case RB_BOTT_UP:
        case RB_UP_BOTT:
            g_rb_SwtTimes = 0;
            if( RB_BOTT_UP == g_rb_Dir )
                g_rb_Dir              = RB_RIGHT_LEFT;
            else
                g_rb_Dir              = RB_LEFT_RIGHT;            
            break;
        case RB_RIGHT_LEFT:
        case RB_LEFT_RIGHT:
            g_rb_SwtTimes = 0;
            if( RB_RIGHT_LEFT == g_rb_Dir )
                g_rb_Dir              = RB_UP_BOTT;
            else
                g_rb_Dir              = RB_BOTT_UP;
        default:
            break;
        }
    }

    switch( g_rb_Dir )
    {
    case RB_BOTT_UP:
    case RB_UP_BOTT:
        for( row = 0; row < REN_COLOR_OPERATION_NUM; row++ )
        {
            colorDir  = RB_COLOR_KEEP;
            if( g_rb_HoriHues[row].currHue == g_rb_HoriHues[row].targetHue )
            {
                //do nothing.
            }
            else
            {
                if( g_rb_HoriHues[row].targetHue - g_rb_HoriHues[row].currHue  > MAXHUE/2 )
                {
                    colorDir = RB_COLOR_DEC;
                }
                else if( g_rb_HoriHues[row].currHue - g_rb_HoriHues[row].targetHue > MAXHUE/2 )
                {
                    colorDir = RB_COLOR_INC;
                }
                else if( g_rb_HoriHues[row].currHue < g_rb_HoriHues[row].targetHue )
                {
                    colorDir = RB_COLOR_INC;
                }
                else
                    colorDir = RB_COLOR_DEC;
            }
            
            switch( colorDir )
            {
            case RB_COLOR_INC:
                if( g_bPickColor )
                {
                    g_rb_HoriHues[row].currHue += RB_EQ_HORI_SPEED * RB_PICK_HORI_INC;
                    g_rb_HoriHues[row].currHue  = g_rb_HoriHues[row].currHue > g_rb_HoriHues[row].targetHue ? g_rb_HoriHues[row].targetHue : g_rb_HoriHues[row].currHue; 
                }
                else
                {
                    g_rb_HoriHues[row].currHue += RB_EQ_HORI_SPEED * RB_HORI_INC;
                }
                g_rb_HoriHues[row].currHue %= MAXHUE;
                break;
            case RB_COLOR_DEC:
                if( g_bPickColor )
                {
                    g_rb_HoriHues[row].currHue -= RB_EQ_HORI_SPEED * RB_PICK_HORI_INC;
                    g_rb_HoriHues[row].currHue  = g_rb_HoriHues[row].currHue < g_rb_HoriHues[row].targetHue ? g_rb_HoriHues[row].targetHue : g_rb_HoriHues[row].currHue; 
                }
                else
                {
                    g_rb_HoriHues[row].currHue -= RB_EQ_HORI_SPEED * RB_HORI_INC;
                }
                if( g_rb_HoriHues[row].currHue < 0 )
                    g_rb_HoriHues[row].currHue += MAXHUE; 
                break;
            default:
                break;
            }

            hslC.hue = g_rb_HoriHues[row].currHue;
            hsl2rgb( hslC, &color );
            for( col = 0; col < REN_VIR_DIS_FRAME; col++ )
            {
                VirPos.x = col;
                VirPos.y = row + ROWOFFSET;
                mapVirLocationColor( VirPos, color , dInd );
            }
        }
        if( pullBackStep > RB_EQ_HORI_PULLBACK_LOW )
            pullBackStep -= RB_EQ_HORI_SPEED;
        break;
    case RB_RIGHT_LEFT:
    case RB_LEFT_RIGHT:
        for( col = 0; col < REN_VIR_DIS_FRAME; col++ )
        {
            colorDir  = RB_COLOR_KEEP;
            if( g_rb_VertHues[col].currHue == g_rb_VertHues[col].targetHue )
            {
                // do nothing.
            }
            else
            {
                if( g_rb_VertHues[col].targetHue - g_rb_VertHues[col].currHue  > MAXHUE/2 )
                {
                      colorDir = RB_COLOR_DEC;
                }
                else if( g_rb_VertHues[col].currHue - g_rb_VertHues[col].targetHue > MAXHUE/2 )
                {
                      colorDir = RB_COLOR_INC;
                }
                else if( g_rb_VertHues[col].currHue < g_rb_VertHues[col].targetHue )
                {
                    colorDir = RB_COLOR_INC;
                }
                else
                    colorDir = RB_COLOR_DEC;
            }
            switch( colorDir )
            {
            case RB_COLOR_INC:
                if( g_bPickColor )
                {
                    g_rb_VertHues[col].currHue += RB_EQ_VERT_SPEED * RB_PICK_VERT_INC;
                    g_rb_VertHues[col].currHue  = g_rb_VertHues[row].currHue > g_rb_VertHues[col].targetHue ? g_rb_VertHues[col].targetHue : g_rb_VertHues[col].currHue; 
                }
                else
                {
                    g_rb_VertHues[col].currHue += RB_EQ_VERT_SPEED * RB_VERT_INC;
                }
                g_rb_VertHues[col].currHue %= MAXHUE;
                break;
            case RB_COLOR_DEC:                
                if( g_bPickColor )
                {
                    g_rb_VertHues[col].currHue -= RB_EQ_VERT_SPEED * RB_PICK_VERT_INC;
                    g_rb_VertHues[col].currHue  = g_rb_VertHues[col].currHue < g_rb_VertHues[col].targetHue ? g_rb_VertHues[col].targetHue : g_rb_VertHues[col].currHue; 
                }
                else
                {
                    g_rb_VertHues[col].currHue -= RB_EQ_VERT_SPEED * RB_VERT_INC;
                }
                if( g_rb_VertHues[col].currHue < 0 )
                    g_rb_VertHues[col].currHue += MAXHUE; 
                break;
            default:
                break;
            }
            hslC.hue = g_rb_VertHues[col].currHue;
            hsl2rgb( hslC, &color );
            for( row = 0; row < REN_COLOR_OPERATION_NUM; row++ )
            {
                VirPos.x = col;
                VirPos.y = row + ROWOFFSET;
                mapVirLocationColor( VirPos, color , dInd );
            }  
        }
        if( pullBackStep > RB_EQ_VERT_PULLBACK_LOW )
            pullBackStep -= RB_EQ_VERT_SPEED;
        break;
    default:
        break;
    }

    switch( g_rb_Dir )
    {
    case RB_BOTT_UP:
    case RB_UP_BOTT: 
        g_rb_SwtTimes += RB_EQ_HORI_SPEED;
        if( g_bPickColor )
        {
            if( g_rb_SwtTimes > RB_PICKC_H_VAR )
            {
               g_rb_SwtTimes = 0;
               if( RB_BOTT_UP == g_rb_Dir )
                {
                    g_rb_CurrPickIndex++;
                    if( g_rb_CurrPickIndex >= REN_COLOR_OPERATION_NUM )
                    {
                        g_rb_CurrPickIndex = 0;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }
                else
                {
                    g_rb_CurrPickIndex--;
                    if( g_rb_CurrPickIndex < 0 )
                    {
                        g_rb_CurrPickIndex = REN_COLOR_OPERATION_NUM - 1;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }

                RainBowUpdatePickHues( g_rb_HoriHues,
                                       REN_COLOR_OPERATION_NUM,
                                       RB_PICKC_H_VAR,
                                       MAXHUE,
                                       g_rb_PickColorDir,
                                       g_rb_CurrPickIndex,
                                       g_rb_PickColorHue,
                                       g_rb_CvtHoriPickColorHue );

            }
            
        }
        else
        {
            if( g_rb_SwtTimes > RB_HORI_VAR )
            {
                g_rb_SwtTimes = 0;
                RainBowUpdateDefaultHues( g_rb_HoriHues,
                                          REN_COLOR_OPERATION_NUM,
                                          RB_HORI_VAR,
                                          MAXHUE,
                                          RB_UP_BOTT == g_rb_Dir );
            }
        }
        break;
    case RB_LEFT_RIGHT:
    case RB_RIGHT_LEFT: 
        g_rb_SwtTimes += RB_EQ_VERT_SPEED;
        if( g_bPickColor )
        {
            if( g_rb_SwtTimes > RB_PICKC_V_VAR )
            {
                g_rb_SwtTimes = 0;
                if( RB_LEFT_RIGHT == g_rb_Dir )
                {
                    g_rb_CurrPickIndex--;
                    if( g_rb_CurrPickIndex < 0 )
                    {
                        g_rb_CurrPickIndex = REN_VIR_DIS_FRAME - 1;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }
                else
                {
                    g_rb_CurrPickIndex++;
                    if( g_rb_CurrPickIndex >= REN_VIR_DIS_FRAME )
                    {
                        g_rb_CurrPickIndex = 0;
                        g_rb_PickColorDir  = g_rb_PickColorDir ? 0 : 1;
                    }
                }

                RainBowUpdatePickHues( g_rb_VertHues,
                                       REN_VIR_DIS_FRAME,
                                       RB_PICKC_V_VAR,
                                       MAXHUE,
                                       g_rb_PickColorDir,
                                       g_rb_CurrPickIndex,
                                       g_rb_PickColorHue,
                                       g_rb_CvtVertPickColorHue );
            }
        }
        else
        {
            if( g_rb_SwtTimes > RB_VERT_VAR )
            {
                g_rb_SwtTimes = 0;
                RainBowUpdateDefaultHues( g_rb_VertHues,
                                          REN_VIR_DIS_FRAME,
                                          RB_VERT_VAR,
                                          MAXHUE,
                                          RB_RIGHT_LEFT == g_rb_Dir );
            }
        }
        break;
    default:
        break;
    }
}
#endif
LOCAL VOID SetRainBowPickColor( BOOL bPickColor, COLOR_T pickColor )
{
    HSL_T                   hslC;
    UINT8                   row;
    UINT8                   col;
    UINT8                   pickOffset;

    rgb2hsl( pickColor, &hslC );
    g_rb_BaseHSL.hue           = hslC.hue;
    g_rb_BaseHSL.lightness     = hslC.lightness;
    g_rb_BaseHSL.saturation    = hslC.saturation;
    if( bPickColor )
    {
        g_rb_CurrPickIndex         = 0;
        g_rb_PickColorHue          = hslC.hue;
        g_rb_CvtHoriPickColorHue   = ( g_rb_PickColorHue + RB_PICKC_H_VAR * ( REN_COLOR_OPERATION_NUM - 1 ) ) % MAXHUE;
        g_rb_CvtVertPickColorHue   = ( g_rb_PickColorHue + RB_PICKC_V_VAR * ( REN_VIR_DIS_FRAME - 1 ) )  % MAXHUE;
        for( row = 0; row < REN_COLOR_OPERATION_NUM; row++ )
        {
            pickOffset  = g_rb_CurrPickIndex > row ? g_rb_CurrPickIndex - row : row - g_rb_CurrPickIndex;
            pickOffset *= RB_PICKC_H_VAR;
            
            if( !g_rb_PickColorDir )
            {
                g_rb_HoriHues[row].currHue    = g_rb_PickColorHue + pickOffset;
                g_rb_HoriHues[row].currHue   %= MAXHUE;
            }
            else
            {
                g_rb_HoriHues[row].currHue    = g_rb_CvtHoriPickColorHue >= pickOffset ? g_rb_CvtHoriPickColorHue - pickOffset : MAXHUE + g_rb_CvtHoriPickColorHue - pickOffset;
            }
            g_rb_HoriHues[row].targetHue = g_rb_HoriHues[row].currHue;
        }
        g_rb_CurrPickIndex = 0;
        for( col = 0; col < REN_VIR_DIS_FRAME; col++ )
        {
            pickOffset  = g_rb_CurrPickIndex > col ? g_rb_CurrPickIndex - col : col - g_rb_CurrPickIndex ;
            pickOffset *= RB_PICKC_V_VAR;
            if( !g_rb_PickColorDir )
            {
                g_rb_VertHues[col].currHue    = g_rb_PickColorHue + pickOffset;
                g_rb_VertHues[col].currHue   %= MAXHUE;
            }
            else
            {
                g_rb_VertHues[col].currHue    = g_rb_CvtVertPickColorHue >= pickOffset ? g_rb_CvtVertPickColorHue - pickOffset : MAXHUE + g_rb_CvtVertPickColorHue - pickOffset;
            }
            g_rb_VertHues[col].targetHue = g_rb_VertHues[col].currHue;
        }
    }
    else
    {
        for( row = 0; row < REN_COLOR_OPERATION_NUM; row++ )
        {
            g_rb_HoriHues[row].currHue   = hslC.hue;
            g_rb_HoriHues[row].targetHue = hslC.hue;
            hslC.hue += RB_HORI_VAR;
            hslC.hue %= MAXHUE;
        }
        hslC.hue = 0;
        for( col = 0; col < REN_VIR_DIS_FRAME; col++ )
        {
            g_rb_VertHues[col].currHue   = hslC.hue;
            g_rb_VertHues[col].targetHue = hslC.hue;
            hslC.hue += RB_VERT_VAR;
            hslC.hue %= MAXHUE;
        }
    }
}

VOID ResetRainBowStat()
{
    COLOR_T      color          = { 255, 0, 0 };
    g_rb_RunTimes               = 0;
    g_rb_SwtTimes               = 0;
    g_rb_PickColorDir           = 0;
    g_rb_PickColorHue           = 0;
    g_rb_Dir                    = RB_BOTT_UP;
    g_bPickColor                = ( BOOL )( APP_COLOR == ThemeColorMode ); 
    g_bPickColor                = FALSE;
#ifdef RB_ENABLE_APP_COLOR_TEST    
    //ThemeColorMode = DEFAULT_COLOR;
    ThemeColor.r   = 255;
    ThemeColor.g   = 0;
    ThemeColor.b   = 0;
#endif
    SetRainBowPickColor( g_bPickColor, g_bPickColor ? ThemeColor : color );
}

VOID DrawRainBow( UINT16 dInd )
{
    if( !AudioDet )
        RefreshNormalRainBow( dInd );
    else
        ;//RefreshEQRainBow( dInd );
}

#ifdef BAR_DEBUG
VOID EQBarDebug()
{
    s_EqBarWatcher = s_EqBarWatcher ? 0 : 1;
}
#endif

LOCAL VOID RefreshNormalBar( CONST COLOR_T barC, UINT16 dInd )
{
    CONST BARSINDESC_T     *pBarDesc;
    COLOR_T                 coverC;
    COLOR_T                 color;
    HSL_T                   hslC;
    PVECTOR_ST              VirPos;
    CONST COLOR_T           bgC        = { 0, 0, 0 };
    UINT16                  lightN_C;
    UINT8                   row;
    UINT8                   col;

#ifdef BAR_NORMAL_STYLE_1
    // 2.3*sin(x*2*PI/12) + 4.3
    LOCAL CONST BARSINDESC_T EQ_BAR_BASE_SINEARY[REN_PHY_FRAME_W] =
    {
        { 4, 2, { 79, 8 } }, { 5, 2, { 84, 6 } }, { 6, 1, { 45, 0 } }, { 6, 1, { 45, 0 } }, { 5, 2, { 84, 6 } }, { 4, 2, { 79, 8 } },
        { 3, 2, { 68, 4 } }, { 2, 2, { 70, 1 } },  { 1, 2, { 95, 12 } }, { 1, 2, { 95, 12 } }, { 2, 2, { 70, 1 } }, { 3, 2, { 68, 4 } }
    };
#else
    // 1.6*sin(x*2*PI/12) + 4.7
    LOCAL CONST BARSINDESC_T EQ_BAR_BASE_SINEARY[REN_PHY_FRAME_W] =
    {
        { 4, 2, { 95, 16 } }, { 5, 2, { 80, 1 } }, { 6, 1, { 40, 0 } }, { 6, 1, { 40, 0 } }, { 5, 2, { 80, 1 } }, { 4, 2, { 95, 16 } },
        { 3, 2, { 99, 31 } }, { 2, 2, { 100, 60 } },  { 1, 2, { 100, 20 } }, { 1, 2, { 100, 20 } }, { 2, 2, { 100, 60 } }, { 3, 2, { 99, 31 } }
    };
#endif
    rgb2hsl( barC, &hslC );
    lightN_C = hslC.lightness;
    if( g_bar_DimStat >= BAR_MOVEINTERVL )
        g_bar_DimStat = 0;
    else
        g_bar_DimStat++;

     hslC.hue = ( hslC.hue + BAR_HUEDEGREE ) % MAXHUE;
     lightN_C = hslC.lightness;

    for( col = 0 ; col < REN_VIR_DIS_FRAME; col++ )
    {
        pBarDesc = &EQ_BAR_BASE_SINEARY[ ( col - g_bar_Pos + REN_VIR_DIS_FRAME ) % REN_VIR_DIS_FRAME ];
        for( row = 0; row < REN_COLOR_OPERATION_NUM; row ++ )
        {
            if( row < pBarDesc->baseCount )
            {
                 color = barC;
            }
            else if( row < pBarDesc->baseCount + pBarDesc->coverCount )
            {
                if( row == pBarDesc->baseCount )
                {
                    hslC.lightness = ( lightN_C * pBarDesc->coverLightness[0] ) / 100;
                }
                else
                    hslC.lightness = ( lightN_C * pBarDesc->coverLightness[1] ) / 100;

                hsl2rgb( hslC, &coverC );
                color = coverC;    
            }
            else
                color = bgC;
            VirPos.x = col;
            VirPos.y = row + ROWOFFSET;
            mapVirLocationColor(VirPos, color, dInd);
        }
    }
    if( g_bar_DimStat == 0 )
    {
#ifdef BAR_DEBUG
        if( s_EqBarWatcher )
            return;
#endif
        if( g_bar_Pos < REN_VIR_DIS_FRAME - 1 )
            g_bar_Pos++;
        else
            g_bar_Pos = 0;
    }
}
#if 0
LOCAL VOID RefreshEqBar( CONST COLOR_T barC, UINT16 dInd )
{
    LOCAL UINT8     s_inter          = 0;
    LOCAL UINT8     s_CoverPos[12]  = { 0 };
    LOCAL UINT8     s_CoverPosChg    = 0;
#ifdef BAR_DEBUG
    LOCAL UINT8     s_EQInter  = 0;
#endif
    COLOR_T         coverC;
    COLOR_T         color;
    HSL_T           hslC;
    PVECTOR_ST      VirPos;
    CONST COLOR_T   bgC        = { 0, 0, 0 };
    INT16           dbVal;
    UINT8           row;
    UINT8           col;
    UINT8           i;
    UINT8           hi;
    CONST UINT8     MAXHI      = 7;

    if( s_inter > 0 )
         s_inter = 0;   
    if( s_inter == 0 )
    {
        GetDbValues( g_DbValAry );
#ifdef BAR_DEBUG
        if( s_EQInter > 10 )
            s_EQInter = 0;
        if( s_EQInter == 0 )
        {
            for( col = 0 ; col < REN_VIR_DIS_FRAME; col++ )
            {
                UartPrintf( "[%d, %d] ", col, g_DbValAry[col] );
            }
            UartPrintf( "\n" );
        }
        ++s_EQInter;
#endif
    }
    s_inter++;
    rgb2hsl( barC, &hslC );
    hslC.hue = ( hslC.hue + BAR_HUEDEGREE ) % MAXHUE;
    hsl2rgb( hslC, &coverC );
    for( col = 0 ; col < REN_VIR_DIS_FRAME; col++ )
    {
        dbVal = g_DbValAry[col % 12];
        for( i = 1; i < _AK7755_SP_ANA_VAL_SIZE; i+=2 )
        {
             if( dbVal == sp_AnaMap[i].dbVal )
             {
                 hi = MAXHI - (i >> 1) + BAR_EQMODEOFFSET;
                 break;
             }
        }
        hi = hi > MAXHI ? MAXHI : hi;
        if( s_CoverPos[col] < hi )
        {
            s_CoverPos[col] = hi;
            s_CoverPosChg   = BAR_EQDOWNSPD;
        }
        else if( s_CoverPos[col] > hi )
        {
            if( s_CoverPosChg >= BAR_EQDOWNSPD )
            {
                s_CoverPos[col] -= 1;
                s_CoverPosChg    = 0;
            }
            else
                s_CoverPosChg++;
        }
        s_CoverPos[col] = s_CoverPos[col] < BAR_EQMODEOFFSET ? BAR_EQMODEOFFSET : s_CoverPos[col];

        for( row = 0; row < REN_COLOR_OPERATION_NUM; row ++ )
        {
            if( row < hi )
            {
                color = barC;
            }
            else if( row == s_CoverPos[col] )
            {

                color = coverC;    
            }
            else if( row < s_CoverPos[col] )
                color = barC;
            else
                color = bgC;
            VirPos.x = col;
            VirPos.y = row + ROWOFFSET;
            mapVirLocationColor(VirPos, color, dInd);
        }
    }
}
#endif
VOID  ResetBarStat()
{
    g_bPickColor = ( BOOL )( APP_COLOR == ThemeColorMode );
    if( !g_bPickColor )
    {
        g_bar_C.r        = 255;
        g_bar_C.g        = 0;
        g_bar_C.b        = 0;
    }
    else
    {
        g_bar_C.r        = ThemeColor.r;
        g_bar_C.g        = ThemeColor.g;
        g_bar_C.b        = ThemeColor.b;
    }
    g_bar_DimStat    = 0;
    g_bar_Pos        = 0;
    
}

VOID  DrawEQBar( UINT16 dInd )
{
    LOCAL UINT8     swtInter   = 0;
    HSL_T           hslC;

    if( !g_bPickColor )
    {
        if( swtInter > BAR_SWTINTERVL )
        {
#ifdef BAR_DEBUG
            if( !s_EqBarWatcher )
#endif
            {
                rgb2hsl( g_bar_C, &hslC );
                hslC.hue = ( hslC.hue + BAR_HUESWTDEGREE ) % MAXHUE;
                hsl2rgb( hslC, &g_bar_C );
                swtInter = 0;
            }        
        }
    }
    if( AudioDet )
        ;//RefreshEqBar( g_bar_C, dInd );
    else
        RefreshNormalBar( g_bar_C, dInd );

    ++swtInter;
}

#ifdef RUN_ONE_TIME_ONLY_DAT_REFRESH_FRAME
PHYDIS_T *GetPhyDisplay_Addr(VOID)
{
    return  (PHYDIS_T *)(&datRefreshFrame[0]);
}
#endif
