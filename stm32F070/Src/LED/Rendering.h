/***************************************************************************************************************************
Copyright (C)  TaiSing Industrial Company Ltd.(Shenzhen).
Project Name:  Pulse II
File name:     Rendering.h
Description:   Rendering design for Pulse II, mainly for color mapping and control of LED displayer

Author: YunSong WEi                             Date: 2015-March-01
***************************************************************************************************************************/

/**************************** Edit History *********************************************************************************
* DATE              NAME                  DESCRIPTION
* 2015-Mar-01       YunSong WEI           Created
* 2015-Mar-22       LMCheong              Modified
* 2015-Jul-25       James Lin             Make this revision for MP releae any other change must add new revision history
***************************************************************************************************************************/

#ifndef RENDERING_H
#define RENDERING_H

#ifdef    __cplusplus
extern "C" {
#endif


#include "GlobalMemory.h"

/************************************************* Type declarations ******************************************/

#define REN_SIM_FRAME               256                         /* Frame width of the simulation system */
#define REN_SIM_SCALE               4                           /* Scaling factor for mapping simulation system to rendering system */
#define REN_FRAME2VIR_FRAME         16
#define REN_VIR_DIS_SCALE           16                          /* Scaling factor for mapping virtual display to simulation sysem */
#define REN_VIR_D_FACTOR            ( REN_VIR_DIS_SCALE / 2 )^2 /* Distance square to determine in scope or not */
#define REN_PHY_DIS_NUM             4                           /* No. of physical display */
#define REN_PHY_DIS_VIR_NUM         2                           /* No. of physical display available for virtual display map */
#define REN_PHY_DIS_CON_OFFSET      REN_PHY_DIS_VIR_NUM         /* Offset of phyical display index for construct display */
#define REN_PHY_DIS_CON_NUM         ( REN_PHY_DIS_NUM - REN_PHY_DIS_VIR_NUM )

#define REN_VIR_DIS_FRAME           12                          /* Frame width of a virtual display in the rendering system */
#define REN_PHY_FRAME_W             12                          /* Frame width of the physical display */
#define REN_PHY_FRAME_H             8                           /* Frame height of the physical display */
#define REN_POWERONOFF_BAR1         3    
#define REN_POWERONOFF_BAR2         6
#define REN_POWERONOFF_BAR3         9
    
#define SIM256_TO_VIR192            192                         /* REN_VIR_DIS_FRAME * REN_VIR_DIS_SCALE = 12 X 16 */
#define SIM128_TO_VIR96             96                          /* SIM256_TO_VIR192 / 2 = 96 */

#define PAT_SHAPE_W                 12//8                           /* Pattern shape width, for particle shape definition */
#define PAT_SHAPE_H                 12//8                           /* Pattern shape height, for particle shape definition */
#define REN_TEXTURE_FRAME           8                           /* Maximum frame width/height of a texture */
#define REN_VIR_DIS_NUM             2                           /* No. of virtual display */
#define REN_VIR_DIS_FRAME           12                          /* Frame width of a virtual display in the rendering system */

#define REN_COLOR_ARRAY_LEN         48                          /* The length of colors array */
#define REN_COLOR_OPERATION_NUM     8                           /* Total no. of color operations */
#define CIND_SCALE                  5        /* Scale up color map index */
#define OPEN_COLOR_SENSOR_ADJ_ENABLE    

#define RUN_ONE_TIME_ONLY_DAT_REFRESH_FRAME

/***************************************************************************************************************
Description:
    The Virtual display 12x12 LED structure, So as to assure the display on
physical LED fully display.
***************************************************************************************************************/
typedef struct VIRDIS
{
    COLOR_T virDisColor[ REN_VIR_DIS_FRAME ][ REN_VIR_DIS_FRAME ];   /* Virtual display buffer color */
}VIRDIS_T;

/***************************************************************************************************************
Description:
    Image information
***************************************************************************************************************/
typedef struct PS_IMAGE_INFO
{
    UINT8    psiFlag;                                           /* Enable/disable image features */
    UINT8    seedColorInd;                                      /* Seed color index */
    UINT8    initColorInd;                                      /* Particle init color process index */
    UINT8    icNum;                                             /* Init color process number */
    UINT8    updateColorInd;                                    /* Update color process index */
    UINT8    ucNum;                                             /* Update color process number */
    UINT8    sColorMethod;                                      /* Size/shape color method */
    UINT8    colorMapInd;                                       /* Start index of color map in colorMap */
    UINT8    colorMapLen;                                       /* Length of the color map */
    UINT8    colorTargetInd;                                    /* Index in color map for target */
    UINT8    shapeFlag;                                         /* Shape flag */
    UINT8    bgFlag;                                            /* Background flag */
}PS_IMAGE_INFO_T;

#define REN_PSI_OLOCK0      0x01                                /* Orientation lock, 0 degree */
#define REN_PSI_COLOR_SNAP  0x02                                /* Same color regardless the weighted factor */
#define REN_PSI_COLOR_OL    0x04                                /* Color overlap for particle update */
#define REN_PSI_SHOW_TARGET 0x08                                /* Show target */
#define REN_PSI_PCOLOR_UPD  0x10                                /* Particle color update for each cycle */
#define REN_PSI_CSUM        0x60                                /* Color seek update method, 2 bits */
#define REN_PSI_OLOCK90     0x80                                /* Orientation lock, 90 degree */

/* Color operations */
typedef struct PS_COLOR_OP
{
    UINT8 pID;                                                  /* Parameter ID */
    UINT8 opType;                                               /* Operation type */
    UINT8 sFactor;                                              /* Scaling factor */
    UINT8 opCode;                                               /* Operator code for the calculation formula */
}PS_COLOR_OP_T;

#define REN_CO_OC0(n)        ( n & 0x03 )                       /* Op-code 0 is defined at the first 2-bit of opCode */
#define REN_CO_OC1(n)        ( ( n & 0x0C ) >> 2 )              /* Op-code 1 is defined at the 3rd and 4th bit of opCode */
#define REN_CO_OC0_SWAP      0x10                               /* Op-code 0 operand swap */
#define REN_CO_OC0_SWAP_MASK 0x10                               /* Op-code 0 operand swap mask */
#define REN_CO_OC1_SWAP      0x20                               /* Op-code 1 operand swap */
#define REN_CO_OC1_SWAP_MASK 0x20                               /* Op-code 1 operand swap mask */
#define REN_CO_OC0_FIRST     0x40                               /* Op-code 0 operation before op-code 1 operation */
#define REN_CO_OC1_FIRST     0x00                               /* Op-code 1 operation before op-code 0 operation */
#define REN_CO_OC_FIRST_MASK 0x40
#define REN_CO_OPTION(n)     ( n & 0xF0 )

/* Maths operation type */
typedef enum MATHS_OP_TYPE
{
    moAdd = 0,                               /* Add */
    moSub = 1,                               /* Substract */
    moMul = 2,                               /* Multiple */
    moDiv = 3                                /* Divide */
}MATHS_OP_TYPE_E;

/* Color setting method, PS_COLOR_OP_T.opType */
typedef enum PS_COLOR_OP_TYPE
{
    coColorInd         = 0x00,               /* Color index to colorMap */
    coBrightness       = 0x01,               /* Brightness */
    coHue              = 0x02,               /* Hue */
    coBrightnessNew    = 0x03,               /* New brightness calculated from sFactor and data */
    coHueNew           = 0x04,               /* New hue calculated from sFactor and data */
    coRed              = 0x05,               /* Red */
    coGreen            = 0x06,               /* Green */
    coBlue             = 0x07,               /* Blue */
    coBreath           = 0x08,               /* Breath */
    coHueFire          = 0x09,

    coColorIndAbs      = 0x80,               /* Absolute color index to colorMap */
    coBrightnessAbs    = 0x81,               /* Absolute brightness */
    coHueAbs           = 0x82,               /* Absolute hue */
    coBrightnessNewAbs = 0x83,               /* Absolute new brightness calculated from sFactor and data */
    coHueNewAbs        = 0x84,               /* Absolute new hue calculated from sFactor and data */
    coRedAbs           = 0x85,               /* Absolute red */
    coGreenAbs         = 0x86,               /* Absolute green */
    coBlueAbs          = 0x87,               /* Absolute blue */
    coBreathAbs        = 0x88                /* Absolute breath */

}PS_COLOR_OP_TYPE_E;

#define REN_CO_TYPE_ABS_MASK 0x80
#define REN_CO_TYPE_MASK     0x7F

/* Size/shape color method, 8 bits, PS_IMAGE_INFO_T.sColorMethod */
typedef enum PS_SS_METHOD
{
    ssmSolid     = 0x00,                     /* Solid color */
    ssmOutline   = 0x01,                     /* Color at outline only */
    ssmFading    = 0x02,                     /* Fade out at edge */
    ssmSpreadHue = 0x03                      /* Spread out hue */
}PS_SS_METHOD_E;

/* Color seek update method, 2 bits, REN_PSI_CSUM */
typedef enum CS_UPD
{
    csuFixed     = 0x20,                     /* Fixed color seed index */
    csuIncrement = 0x40                      /* Increase by 1 for every seed produced */
}CS_UPD_E;

/* Background flag enumerations */
typedef enum BG_TYPE
{
    bgtInvalid  = 0x00,                                         /* Invalid background, make it the same as black background */
    bgtBlack    = 0x10,                                         /* Static background, always in black color */
    bgtStatic   = 0x20,                                         /* Static background, always the same */
    bgtColorMap = 0x30,                                         /* Static background, always in color from colorMap */
    bgtEq       = 0x40                                          /* Dynamic background, depends on EQ high, mid or low value */
}BG_TYPE_E;

typedef enum CON_DIS_CTYPE
{
    centerR          = 0x00,
    centerG          = 0x01,
    centerB          = 0x02,
    rowChangeHue     = 0x03,
    columnChangeHue  = 0x04,
    volFadeColor     = 0x05,
    unChange         = 0x10
}CON_DIS_CTYPE_E;

#define REN_BG_IND_MASK  0x0F
#define REN_BG_TYPE_MASK 0xF0

/* Shape flag enumerations */
typedef enum SHAPE_TYPE
{
    stNull     = 0x00,                                          /* No shape is defined */
    stExplosion= 0x10,                                          /* Star shape */
    stFirework = 0x20,                                          /* Firework shape */
    stFixedShape    = 0x30,                                          /* Heart shape */
    stWave     = 0x40,                                          /* Wave shape */
    stHGStack  = 0x50,                                          /* Hourglass stack shape */
    stRave     = 0x60,
    stDotTrack = 0x70,                                           /*For Cross theme*/
    stCustomized     = 0x80,
    stBar      = 0x90,
    stRainbow  = 0xA0,
    stImage    = 0xB0                                           /* Image type shape, need to copy shape image */
    /* For more than 1 images are defined, set shape = ( stImage + ( n - 1 )*0x10 ) where n is the no. of shape images */
}SHAPE_TYPE_E;

#define REN_SHAPE_IND_MASK  0x0F
#define REN_SHAPE_TYPE_MASK 0xF0

/* A pixel in a shape is defined by a bit in the shape array */
typedef struct SHAPE
{
    UINT16 shape[ PAT_SHAPE_H ];
}SHAPE_T;

#define REN_SHAPE_CENTER_H  ( PAT_SHAPE_H / 2 - 1 )
#define REN_SHAPE_CENTER_W  ( PAT_SHAPE_W / 2 - 1 )

/***************************************************************************************************************
Description:
    this is a texture structure type, information include the texture's height
,width,offset,state and RGB information.
***************************************************************************************************************/
typedef struct TEXTURE
{
    UINT8 height;                                               /* Height of the texture */
    UINT8 width;                                                /* Width of the texture */
    UINT8 offset;                                               /* Offset from the left top corner of texture to */
                                                                /* the center of the particle, in pixel count */
    UINT8 state;                                                /* State of the texture buffer: FREE, OCCUPIED */
    UINT8 r[ REN_TEXTURE_FRAME * REN_TEXTURE_FRAME ];           /* Array of red color information */
    UINT8 g[ REN_TEXTURE_FRAME * REN_TEXTURE_FRAME ];           /* Array of green color information */
    UINT8 b[ REN_TEXTURE_FRAME * REN_TEXTURE_FRAME ];           /* Array of blue color information */
}TEXTURE_T;

/***************************************************************************************************************
Description:
    The physical 9x11 LED display color array
***************************************************************************************************************/
typedef struct PHYDIS
{
    COLOR_T phyDisColor[ REN_PHY_FRAME_W ][ REN_PHY_FRAME_H ];      /* Physical display buffer, red color */
}PHYDIS_T;

/* HSL color */
typedef struct HSL
{
    INT16 hue;              /* Hue value, 0 to 360 */
    UINT16 saturation;      /* Saturation value, 0 to 512, S' = 512 * (cMax - cMin)/(510 - L'), for L' > 255 */
                            /*                                = 512 * (cMax - cMin)/ L', for L' <= 255       */
                            /*                                = 0, for cMax = cMin                           */
    INT16 lightness;        /* Lightness, 0 to 510, L' = cMax + cMin */
}HSL_T;

/* Particle structure (temporary put here...) */
typedef struct Particle
{
    UINT8  systemID;                // ID of the particle system this particle belongs to, invalid =SIM_PARTICLE_INVALID
    UINT8  layer;
    UINT8  angle;                   // Angle of the particle center, for color mapping calculation, it is in [0, 180)
                                    // with one unit equals to 2 degrees
    UINT8  aVelocity;               // Angular velocity of the particle, for color mapping calculation, also in 2 degrees per unit
    UINT8  grid;

    UINT8  aAmplitude;              // Angular amplitude of the particle, for color mapping calculation, valid for size>0 only, decrement for every timetick

    UINT8  size;
    INT8   grow;
    UINT8  mass;
    UINT8  maxSpeed;

    UINT8  maxSteer;                // Maximum steering force the particle can support
    UINT8  seq;                     // Sequence no. of the particle, valid for particle chain only, default to 0
    UINT8  state;                   // State of life of the particle
    UINT8  accCnt;                  // Run-time acceleration counter

    UINT16 lifespan;
    UINT16 effectCnt;               // Special effect count down timer.   Special effect input would be accepted only if effectCnt = 0

    PVECTOR_ST  location;
    PVECTOR_ST  velocity;
    PVECTOR_ST  acceleration;

    COLOR_T color;
    
    UINT8 seqEnd;                    // For sequence end identifier

    struct Particle* nextPPtr;      // Pointer to the next particle in a particle chain

}PARTICLE_ST;

typedef struct RAVE_XPOS_T
{
    INT8 x1;
    INT8 x2;
}RAVE_XPOS_T;

typedef struct RAVE_YPOS_T
{
    INT8 y1;
    INT8 y2;
}RAVE_YPOS_T;

typedef struct RAVE_POS_T
{
    RAVE_XPOS_T Xpos;
    RAVE_YPOS_T Ypos;
}RAVE_POS_T;

typedef struct COLOR_EQBAR_T
{
    COLOR_T cColor;
    COLOR_T tColor;
}COLOR_EQBAR_T;

typedef struct RAVE_MODE_INFO
{
    UINT8 bRave_st;
    UINT8 bRave_flag;
    UINT8 bRave_music_flag;
    UINT8 bRave_EXflag;
    
    UINT8 bRave_Reverse;
    UINT8 bRave_Random_flag1;
    UINT8 bRave_Random_flag2;
    UINT8 bRave_Random_flag3;
    
    UINT8 bRave_Random_Check1;
    UINT8 bRave_Random_Check2;
    UINT8 bRave_Random_Check3;
    
    COLOR_T tRave_Color_buf;  
    
    UINT8 bRave_Breath_Cnt;
    
    //UINT8 bRave_Low_to_Mid_Cnt;
}RAVE_MODE_INFO_T;

extern RAVE_MODE_INFO_T tRave_Mode_Info;

typedef  struct
{
    UINT16 x;
    UINT16 y;
}Cus_partical;

typedef struct CUS_MODE
{
    UINT8 bcus_low_mod ;
    UINT8 bcus_mid_mod ;
    UINT8 bcus_high_mod ;
    
    COLOR_T bcus_col ;
    COLOR_T bcus_col1 ;
    COLOR_T bcus_col2 ;
    COLOR_T bcus_col3;

    
    UINT8 bcus_busy[10] ;
    PVECTOR_ST cus_loc;
    PVECTOR_ST cus_loc1;
//    UINT8 bcus_lifecnt1 ;
//    UINT8 bcus_lifecnt2 ;
//    UINT8 bcus_lifecnt3 ;
    Cus_partical partical1;
    Cus_partical partical2;
    Cus_partical partical3;
    UINT8 bcus_lifecnt4 ;
    UINT8 bcus_lifecnt5 ;
    UINT8 bcus_lifecnt6 ;
    UINT8 bcus_lifecnt7 ;
    UINT8 bcus_lifecnt8 ;
    UINT8 bcus_lifecnt9 ; 

    UINT8 bcus_speed ;
    UINT8 bcus_normal_mode_flag ;
    UINT8 bcus_music_flag ;
    UINT8 detect_mode;
    volatile UINT32 bcus_timer_cnt;
}CUS_MODE_INFO_T;

extern CUS_MODE_INFO_T cus;
typedef struct JET_MODE_PSM_INFO
{
    COLOR_T    bJET_Color_buf;
    UINT8      bJET_music_EXflag;
    BOOL       bResetData;
}JET_MODE_PSM_INFO_T;

extern JET_MODE_PSM_INFO_T tJet_Mode_PSM_Info;

LOCAL UINT8 SeedArr[12] = { 2, 2, 4, 5, 7, 8, 9, 9, 8, 7, 5, 4 };
  
/************************************************* Global variable declarations *******************************/
GLOBAL UINT8	    PhyDisActiveInd;
GLOBAL UINT8	    PhyDisLoadInd;
GLOBAL PHYDIS_T	    PhyDis[REN_PHY_DIS_NUM];		        /* Physical display buffer */
GLOBAL COLOR_T      colorMap[  ];              /* Color map for particle systems */
GLOBAL UINT8        IniColor_f;
GLOBAL UINT8        BarApp;

/************************************************* Functional prototypes **************************************/

/************************************************************************
 Function:    RenderingInitialize
 Description: Initialize the display rendering module
 Input:       VOID
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL VOID RenderingInitialize(VOID);

/************************************************************************
 Function:    InitRendererDisBuf
 Description: Initialize the virtual and physical display buffer and its
              buffer index
 Input:          VOID
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL VOID InitRendererDisBuf(VOID);

/************************************************************************
 Function:    InitialMapColor
 Description: Initialize color map and color operations
 Input:       cPtr:   pointer to initialize color data set of a pattern
              cOpPtr: pointer to initial color operation array
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL VOID InitialMapColor(COLOR_T *cPtr, PS_COLOR_OP_T *cOpPtr);

/************************************************************************
 Function:    Renderer
 Description: Render the particle system to a virtual display
 Input:       VOID
 Return:      VOID
 Remarks:     Call from system main loop
************************************************************************/
GLOBAL VOID Renderer(VOID);

/************************************************************************
 Function:    DisplayManager
 Description: Select the appropriate display channel to update physical display
 Input:       conDisFlag: TRUE: Construct display has to update
                          FALSE: Construct display doesn't update
 Return:      VOID
 Remarks:     Call from system main loop
************************************************************************/
GLOBAL VOID DisplayManager(BOOL conDisFlag);

/************************************************************************
 Function:    SetInitColor
 Description: Set the initial color of a particle
 Input:       pPtr :    pointer to the particle
              psInd:    particle system index
              preColor: preset color to start the calculation
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL VOID SetInitColor(PARTICLE_ST *pPtr, UINT16 psInd, COLOR_T preColor);

/************************************************************************
 Function:    SetBackground
 Description: Copy the background image to the Renderer run-time
 Input:       bPtr: pointer to a background image
 Return:      background index in Renderer run-time
 Remarks:
************************************************************************/
GLOBAL UINT8 SetBackground(VIRDIS_T *bPtr);

/************************************************************************
 Function:    SetShape
 Description: Copy the shape image to the Renderer run-time
 Input:       sPtr: pointer to a shape image
 Return:      shape index in Renderer run-time
 Remarks:
************************************************************************/
GLOBAL UINT8 SetShape(SHAPE_T *sPtr);

/************************************************************************
 Function:    SetPrismColorMap
 Description: Set color map according to color captured by color sensor
              Prism feature
 Input:       VOID
 Return:      VOID
 Remarks:     Set to SimPS[0] only
************************************************************************/
GLOBAL VOID SetPrismColorMap(VOID);

/************************************************************************
 Function:    InterchangeCanvasColor
 Description: process Canvas's shake input
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to Canvas shake interaction
************************************************************************/
GLOBAL VOID InterchangeCanvasColor(UINT8 psInd);

/************************************************************************
 Function:    SetPatternBroadcastColor
 Description: Set colors according to pattern broadcast information received
 Input:       pInfoPtr: pointer to pattern broadcast information
 Return:      VOID
 Remarks:     Handle data from GMPatternInfoRx
************************************************************************/
GLOBAL VOID SetPatternBroadcastColor(PATTERN_INFO_T *pInfoPtr);

/************************************************************************
 Function:    RecvFrameColor
 Description: Receive color data from BT and write them into display buffer
 Input:       i: LED index
              pColor: a LED color
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL VOID RecvFrameColor( UINT8 i, COLOR_T pColor );

/************************************************************************
 Function:    MapEQLevelToLEDLevel
 Description: Maps EQLevel value from 0 to 65535 to corresponding level between -REN_SIM_FRAME/2 to REN_SIM_FRAME/2
 Output:      Val: Corresponding EQ level in LED              
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL UINT8 MapEQLevelToLEDLevel(UINT16 *Val, UINT8 Index);
GLOBAL VOID  PreparationEQLevelToIndexLevel(UINT8 *Val, UINT8 Index);
//GLOBAL UINT8 MapEQLevelToIndexLevel(UINT8 Index);
GLOBAL VOID  MapIndexLevelToEQBarLEDLevel(UINT8* Val, UINT8 Index);

GLOBAL VOID setAudioUnDetect(void);
GLOBAL VOID setAudioDetect(void);

GLOBAL VOID ResetBarStat(void);
GLOBAL VOID DrawEQBar( UINT16 dInd );

GLOBAL VOID SetJetThemeNormalColor(VOID);
GLOBAL VOID SetWaveThemeColor(VOID);

GLOBAL VOID ResetRainBowStat(void);
GLOBAL VOID DrawRainBow( UINT16 dInd );

#ifdef OPEN_COLOR_SENSOR_ADJ_ENABLE
/************************************************************************
 Function:    adjustPickColor
 Description: Adjust the color at GMColorPick according to environmental effect
 Input:       VOID
 Return:      VOID
 Remarks:     Called by runColorPick() in Prism function and
 *            in Canvas pattern processing
************************************************************************/

GLOBAL VOID adjustPickColor(VOID);
#endif

#ifdef RUN_ONE_TIME_ONLY_DAT_REFRESH_FRAME
GLOBAL PHYDIS_T *GetPhyDisplay_Addr(VOID);
#endif

#ifdef    __cplusplus
}
#endif

#endif /* RENDERING_H */


