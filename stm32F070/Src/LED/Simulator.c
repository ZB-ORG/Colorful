/**************************************************************************************************************************
  Copyright (C)  TaiSing Industrial Company Ltd.(Shenzhen).
  Project Name:  Pulse II    
  File name:     Simulator.c
  Description:   Realize the particle system's simulate 

* Author: Terry.Li                  Date: 2015-Mar-07
***************************************************************************************************************************/
/**************************** Edit History *********************************************************************************
* DATE              NAME                  DESCRIPTION
* 2015-Mar-01       Terry.Li               Created
* 2015-Mar-22       LMCheong               Modified
* 2015-Jul-25       James Lin              Make this revision for MP releae any other change must add new revision history
* 2015-Jul-27       LM Cheong              Revise parameters in Fire, Traffic, Customized, Lightning, Firefly according to comments from Harman
* 2015-Aug-21       JamesLin               Fix bug when firework shake handle not active 
***************************************************************************************************************************/

#include "..\..\Inc\sys_common.h"

/***************************************** Function declarations **************************************************/
LOCAL VOID systemCleanUp(VOID);
LOCAL BOOL dataInitializer(VOID);
LOCAL VOID loadPattern(UINT16 id);
LOCAL VOID updateConditions(VOID);

LOCAL VOID emitter(VOID);
LOCAL VOID killParticles(VOID);
LOCAL VOID killOneParticle(PARTICLE_ST *pPtr);
LOCAL VOID enterNextState(PARTICLE_ST  *pPtr);
LOCAL VOID createParticles(UINT16 psInd, UINT16 pNum);
LOCAL VOID initParticle(PARTICLE_ST *pPtr, UINT16 pInd);

LOCAL VOID physicsEngine(VOID);
LOCAL VOID runParticle(PARTICLE_SYSTEM_ST *psPtr);
LOCAL VOID updateTarget(PARTICLE_SYSTEM_ST *psPtr);

LOCAL VOID iTargetSeek(PARTICLE_ST *pPtr, PARTICLE_SYSTEM_ST *psPtr);
LOCAL VOID seekTarget(PARTICLE_ST *pPtr, PVECTOR_ST target);
LOCAL VOID updateOneParticle(PARTICLE_ST* pPtr, PS_BASE_T* psbPtr);

LOCAL UINT8 setGrid(PVECTOR_ST posPtr);

LOCAL VOID pVectorAdd(PVECTOR_ST v1, PVECTOR_ST v2, PVECTOR_ST *rPtr);
//LOCAL VOID pVectorSub(PVECTOR_ST v1, PVECTOR_ST v2, PVECTOR_ST *rPtr);
LOCAL VOID pVectorMult(PVECTOR_ST v1, INT8 num, PVECTOR_ST *rPtr);
//LOCAL VOID pVectorDiv(PVECTOR_ST v1, INT8 num, PVECTOR_ST *rPtr);
LOCAL VOID pVectorScaling(PVECTOR_ST v1, INT8 num, PVECTOR_ST *rPtr);

LOCAL inline BOOL pVectorNull(PVECTOR_ST v1);

LOCAL BOOL inputOperate(INT16 inVal, UINT16 opInd, UINT16 psInd);
LOCAL BOOL checkProcess(UINT16 cInd, UINT16 pInd, UINT16 psInd, INT16 inputValue );
LOCAL VOID actionProcess(UINT16 aInd, UINT16 pInd, UINT16 psInd, INT16 iVal);
LOCAL BOOL compareScalar(INT32 s1, INT32 s2, UINT16 cType );
LOCAL BOOL compareVector(PVECTOR_ST v1, PVECTOR_ST v2, UINT16 cType);

LOCAL UINT16 genRandScalar(UINT16 min, UINT16 max, UINT16 mask);
LOCAL VOID genRandVector(GEN_PARA_T vMin, GEN_PARA_T vMax, UINT16 mask, GEN_PARA_T *rPtr);
LOCAL VOID prepareGridDataList(UINT16 min, UINT16 max, UINT16 not1);

//LOCAL VOID loadCanvas( UINT16 psInd );
LOCAL VOID loadBar( UINT16 psInd );
LOCAL VOID loadFixedLoc( UINT8 start, UINT16 num );
//LOCAL VOID loadFountain( UINT16 psInd, UINT16 num );
//LOCAL VOID loadPicture( UINT8 psInd );
LOCAL VOID loadRave( void );
LOCAL VOID loadEQJet( void );
LOCAL VOID LoadCustomizedTheme(VOID);

//LOCAL VOID setFireworkParticles( UINT16 psInd );
LOCAL VOID createSpecialFireworks( UINT16 psInd, INT16 exIndex );
LOCAL VOID setTrafficTarget(UINT16 psInd);
LOCAL VOID setMeteorParticle(UINT16 psInd);
LOCAL VOID setStarAndMeteorShowerParticleSystem(UINT16 psInd);
LOCAL VOID setWaveLevel(VOID);
LOCAL VOID setWaveShake(VOID);

LOCAL VOID setFireflyParticles(UINT16 psInd);
LOCAL VOID setFireLevel(UINT16 psInd, UINT16 fireLevel);
LOCAL VOID setCanvasParticles(UINT16 psInd);
LOCAL VOID setHourglassParticle(UINT16 psInd, UINT16 tValue);
LOCAL VOID setBarShake( UINT16 scalar );

LOCAL VOID copySymmetryLocation( PARTICLE_ST *pPtr, UINT8 maxSeq );
LOCAL VOID updateFountainLocation( PARTICLE_ST *pPtr, INT16 inputScalar, INT16 aScalar, UINT8 psInd );

LOCAL VOID UpdateThemeColorStatus(VOID);

/************************************************* Type declarations ******************************************/

#define   SIM_GRID_LEN          64     /* Length in x or y of one grid space */

#define   SIM_GRID_NUM          16     /* Number of the grid */
#define      SIM_OUT_SCOPE         0x80   /* Set MSB of grid by SIM_OUT_SCOPE when it is already moved outside */

#define   SIM_SEQ_END           0xF0   /* Indicate it is the end of a particle sequence */

#define   SIM_SIN_NUM           16     /* the total number of sine value */

#define   ANGLE_INTERVAL        6      /* angle interval are 6 degrees */

#define   EIGHT_BITS            8      /* To avoid a smaller no. divid a bigger one equal 0 */

#define   SIM_CREATE_NUM_MASK   0x1F   /* Create rate mask */
#define   SIM_CREATE_P_MASK     0xE0   /* Create probability mask */

#define   SIM_ACC_MASK          0x0F   /* Acceleration mask */
#define   SIM_ACC_CNT_MASK      0xF0
#define   SIM_ACC_CNT           0x10

#define   RANDOM_HISTORY        12      /* Random location record no., to avoid duplicate location generated */
#define   INVALID_LOC           0x6E6E /* An arbitrary location out of the virtual display scope */
#define   INVALID_GD_NUM        0xFF   /* Invalid grid data number */

#define   ITARGET_SEEK          10     /* Arbitrary state no. to distinguish from ordinary state */
                                       /* For individual target seeking, Firefly specific */

#define  FIREWORK_SHAKE_CNT     39     /* Special function needed to record the total number of particles making up the heart firework */
#define  SPECIAL_LOC_INDEX      38     /* White patticle's location index in its pattern grid data array */

#define  SHAKE_LIFE             180    /* 180 timetick ,7.2s ,per timetick is 40 ms, for firework shake effect time */
#define  APPS_LIFE              71     /* 107 timetick ,2.8s ,per timetick is 40 ms, for firework apps effect time */

#define  HIGHEST_LED_CENTER     72     /* The center line of the highest LED */
#define  LOWEST_LED_CENTER      -56    /* The center line of the lowest LED */
#define  LOWEST_LINE            -64    /* The lowest line to map to physical LED */
#define  ONE_LED_SIZE           32     /* The size to bright 1 LED  */

#define  METEOR_MIN_LOC         10     /* The minimum location for meteor system */
#define  METEOR_MAX_LOC         39     /* The maximum location for meteor system */

const UINT16 EXPLOSION_HUE_FACTOR = 2;
const UINT16 EXPLOSION_MAX_HUE    = 360 * 2;

extern PATTERN_E pattern_flag;

/* Patterns' shake and apps enum */
typedef enum exInput
{
    firework_ExInput2 = 110,
    firework_ExInput3 = 111,

    traffic_app_end   = 115,
            
    fire_shake2       = 120,    

    firefly_app2      = 150,
    firefly_app3      = 151,

    general_shake_set    = 1,
    general_shake_reset  = 0
}EX_INPUT_E;


/************************************************* Global variable declarations *******************************/
PARTICLE_SYSTEM_ST SimPS[ SIM_PS_NUM ];
PARTICLE_ST        SimP [ SIM_PARTICLE_NUM ];

UINT8              SimPMax; /* Maximum no. of particle for the running pattern */
BOOL               AudioDet;
THEME_COLOR_MODE   ThemeColorMode = DEFAULT_COLOR;
COLOR_T            ThemeColor;


extern VOID SetExplosionColor(PARTICLE_ST *pPtr, UINT16 psInd, COLOR_T preColor);

/* Particle system parameter property definitions, indexed by SIM_PARA_E */
CONST PS_PARA_PROPERTY_T SimParaProperty[] =
{
    /* Particle section */

    /* { pGroup, offset, pType } */                  /* offset in paraSet */
    { psgP, 0, pstUINT8 },      /* Particle: systemID   0x00 */
    { psgP, 1, pstUINT8 },      /* Particle: layer      0x01 */
    { psgP, 2, pstUINT8 },      /* Particle: angle      0x02 */
    { psgP, 3, pstUINT8 },      /* Particle: aVelocity  0x03 */
    { psgP, 4, pstUINT8 },      /* Particle: grid       0x04 */
    { psgP, 5, pstUINT8 },      /* Particle: aAmplitude 0x05 */
    { psgP, 6, pstUINT8 },      /* Particle: size       0x06 */

    
    /* { pGroup, offset, pType } */
    { psgP, 7, pstUINT8 },      /* Particle: grow     0x07 */
    { psgP, 8, pstUINT8 },      /* Particle: mass     0x08 */
    { psgP, 9, pstUINT8 },      /* Particle: maxSpeed 0x09 */
    { psgP, 10, pstUINT8 },     /* Particle: maxSteer 0x0A */
    { psgP, 11, pstUINT8 },     /* Particle: seq      0x0B */
    { psgP, 12, pstUINT8 },     /* Particle: state    0x0C */


    /* { pGroup, offset, pType } */
    { psgP, 13, pstUINT8 },     /* Particle: texture      0x0D */
    { psgP, 14, pstUINT16 },    /* Particle: lifespan     0x0E */
    { psgP, 16, pstUINT16 },    /* Particle: effectCnt    0x0F */
    { psgP, 18, pstVector },    /* Particle: location     0x10 */
    { psgP, 20, pstVector },    /* Particle: velocity     0x11 */
    { psgP, 22, pstVector },    /* Particle: acceleration 0x12 */
    { psgP, 24, pstColor },     /* Particle: color        0x13 */
    { psgP, 27, pstUINT8 },     /* Particle: seqEnd       0x14 */


    /* Particle system base section */
    /* { pGroup, offset, pType } */
    { psgPsb, 0, pstUINT8 },    /* Particle system base: id         0x15 */
    { psgPsb, 1, pstUINT8 },    /* Particle system base: seqMax     0x16 */
    { psgPsb, 2, pstUINT8 },    /* Particle system base: createRate 0x17 */
    { psgPsb, 3, pstUINT8 },    /* Particle system base: maxCnt     0x18 */
    { psgPsb, 4, pstUINT8 },    /* Particle system base: currentCnt 0x19 */
    { psgPsb, 5, pstUINT8 },    /* Particle system base: maxState   0x1A */


    /* { pGroup, offset, pType } */
    { psgPsb, 6, pstINT8 },     /* Particle system base: timeMul  0x1B */
    { psgPsb, 7, pstUINT8 },    /* Particle system base: angleMin 0x1C */
    { psgPsb, 8, pstUINT8 },    /* Particle system base: angleMax 0x1D */
    { psgPsb, 9, pstUINT8 },    /* Particle system base: aVelMin  0x1E */
    { psgPsb, 10, pstUINT8 },   /* Particle system base: aVelMax  0x1F */
    { psgPsb, 11, pstUINT8 },   /* Particle system base: aAmpMin  0x20 */


    /* { pGroup, offset, pType } */
    { psgPsb, 12, pstUINT8 },   /* Particle system base: aAmpMax 0x21 */
    { psgPsb, 13, pstUINT8 },   /* Particle system base: sizeMin 0x22 */
    { psgPsb, 14, pstUINT8 },   /* Particle system base: sizeMax 0x23 */
    { psgPsb, 15, pstUINT8 },   /* Particle system base: growMin 0x24 */
    { psgPsb, 16, pstUINT8 },   /* Particle system base: growMax 0x25 */
    { psgPsb, 17, pstUINT8 },   /* Particle system base: massMin 0x26 */


    /* { pGroup, offset, pType } */
    { psgPsb, 18, pstUINT8 },   /* Particle system base: massMax     0x27 */
    { psgPsb, 19, pstUINT8 },   /* Particle system base: maxSpeedMin 0x28 */
    { psgPsb, 20, pstUINT8 },   /* Particle system base: maxSpeedMax 0x29 */
    { psgPsb, 21, pstUINT8 },   /* Particle system base: maxSteerMin 0x2A */
    { psgPsb, 22, pstUINT8 },   /* Particle system base: maxSteerMax 0x2B */
    { psgPsb, 23, pstINT8 },    /* Particle system base: sizeAspect  0x2C */


    /* { pGroup, offset, pType } */
    { psgPsb, 24, pstUINT16 },  /* Particle system base: separation 0x2D */
    { psgPsb, 26, pstUINT16 },  /* Particle system base: cohesion   0x2E */
    { psgPsb, 28, pstUINT16 },  /* Particle system base: lifeMin    0x2F */
    { psgPsb, 30, pstUINT16 },  /* Particle system base: lifeMax    0x30 */
    { psgPsb, 32, pstUINT16 },  /* Particle system base: EffectCnt  0x31 */
    { psgPsb, 34, pstVector },  /* Particle system base: target     0x32 */


    /* { pGroup, offset, pType } */
    { psgPsb, 36, pstVector },  /* Particle system base: locMin 0x33 */
    { psgPsb, 38, pstVector },  /* Particle system base: locMax 0x34 */
    { psgPsb, 40, pstVector },  /* Particle system base: velMin 0x35 */
    { psgPsb, 42, pstVector },  /* Particle system base: velmax 0x36 */
    { psgPsb, 44, pstVector },  /* Particle system base: accMin 0x37 */
    { psgPsb, 46, pstVector },  /* Particle system base: accMax 0x38 */


    /* { pGroup, offset, pType } */
    { psgPsb, 48, pstPTR },     /* Particle system base: windPtr  0x39 */
    { psgPsb, 52, pstPTR },     /* Particle system base: forcePtr 0x3A */
    { psgPsb, 56, pstUINT16 },  /* Particle system base: psFlag   0x3B */
    { psgPsb, 58, pstUINT16 },  /* Particle system base: randSnap 0x3C */

    /* Particle system image information section */

    /* { pGroup, offset, pType } */
    { psgPsi, 0, pstUINT8 },    /* Particle system image info: psiFlag        0x3D */
    { psgPsi, 1, pstUINT8 },    /* Particle system image info: seedColorInd   0x3E */
    { psgPsi, 2, pstUINT8 },    /* Particle system image info: initColorInd   0x3F */
    { psgPsi, 3, pstUINT8 },    /* Particle system image info: icNum          0x40 */
    { psgPsi, 4, pstUINT8 },    /* Particle system image info: updateColorInd 0x41 */
    { psgPsi, 5, pstUINT8 },    /* Particle system image info: ucNum          0x42 */
    { psgPsi, 6, pstUINT8 },    /* Particle system image info: sColorMethod   0x43 */
    { psgPsi, 7, pstUINT8 },    /* Particle system image info: colorMapInd    0x44 */
    { psgPsi, 8, pstUINT8 },    /* Particle system image info: colorMapLen    0x45 */
    { psgPsi, 9, pstUINT8 },    /* Particle system image info: colorTargetInd 0x46 */
    { psgPsi, 10, pstUINT8 },   /* Particle system image info: shapeFlag      0x47 */
    { psgPsi, 11, pstUINT8 },   /* Particle system image info: bgFlag         0x48 */

    /* Extra added */
    { psgP, 18, pstINT8 },      /* Particle: locationX                        0x49 */
    { psgP, 19, pstINT8 },      /* Particle: locationY                        0x4A */
    { psgP, 20, pstINT8 },      /* Particle: velocityX                        0x4B */
    { psgP, 21, pstINT8 },      /* Particle: velocityY                        0x4C */
    { psgP, 21, pstLoc2LED},   /* Particle: pLEDLocY                        0x4D */
    { psgP, 6,  pstSize2LED}    /* Particle: pLEDSize                        0x4E */
};

/************************************************* Local variables definitions *********************************/
/*************************************************************************************************
* Array name:  sine
* Description: Store 16 angles' sine value
* Remarks:     sin[0] = sin( 6 ) * 64,sin[3] = sin( 24 ) * 64; 6 means 6 degrees,24 means 24 degrees
*
*************************************************************************************************/
LOCAL CONST INT8 sine[ SIM_SIN_NUM ] =
{
    0,       // 0  degree
    3 ,//7,     // 6  degrees
    5 ,//14,   // 12 degrees
    7 ,//20,   // 18 degrees
    10,//26,   // 24 degrees

    12,//32,   // 30 degrees
    14,//38,   // 36 degrees
    16,//43,   // 42 degrees
    18,//47,   // 48 degrees
    19,//52,   // 54 degrees

    21,//55,  // 60 degrees
    22,//58,  // 66 degrees
    23,//61,  // 72 degrees
    23,//62,  // 78 degrees
    24,//63,  // 84 degrees
    24 //64   // 90 degrees
};

/* Pattern data storage arrays */
/* 1. Pattern metadata */
/*************************************************************************************************
* Array name:  patMeta
* Description: Pattern metadata
* Remarks:     patMeta[2] = {1,2,0,..}, means the pattern data is indexed at 1 of data arrays and
*              there are 2 particle systems defined for the pattern
*************************************************************************************************/
LOCAL CONST PATTERN_REC_ST patMeta[] =
{
    { /* PatFirework */
        psFireworkRaiseUp /* psInd */, 2 /* psNum */, 1 /* delayCnt */, 0 /* exInd */, {5,1,0,0} /* exNum[SIM_PS_NUM] */, 
        0 /* exOpInd */, 7 /* exOpNum */, 0 /* exCheckInd */, 0 /* exCheckNum */, 0 /* exActionInd */, 16 /* exActionNum */,
        {  /* color map */
            {255,0,0}, {255,43,1}, {255,86,1}, {255,128,1}, {255,170,1}, {255,231,1}, {255,255,1}, {219,255,1}, {176,255,0},
            {134,255,1}, {92,255,1}, {49,255,1}, {7,255,1}, {1,255,31}, {1,255,80}, {1,255,122}, {1,255,164}, {1,255,207},
            {1,255,249}, {1,255,255}, {1,182,255}, {1,140,255}, {1,98,255}, {1,55,255}, {1,13,255}, {31,1,255}, {1,170,255},
            {1,128,255}, {1,128,255}, {1,86,255}, {13,1,255}, {55,1,255}, {98,1,255}, {140,1,255}, {146,1,255}, {188,1,255},
            {188,1,255}, {231,1,255}, {255,1,243}, {255,1,201}, {255,1,208}, {255,1,116}, {255,1,90}, {255,1,37}, {255,1,1},
            { 128,128,128 }, {255,0,0}, {0,0,0} }, 
        { /* color operation */
            { /* operation id=0 */ pAngle /* pID */, coBreathAbs /* opType */, 90 /* sFactor */, ( REN_CO_OC0_SWAP | ( moSub << 2 ) | moMul ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        64 /* pMax */,
        { /* grid data array */
            0xF808 /* (8,-8) 0 */, 0x3828 /* (40,56) 1 */,
            0x38E8 /* (-24,56) 2 */, 0x2838 /* (56,40) 3 */,
            0x2828 /* (40,40) 4 */, 0x2818 /* (24,40) 5 */,
            0x28F8 /* (-8,40) 6 */, 0x28E8 /* (-24,40) 7 */,
            0x28D8 /* (-40,40) 8 */, 0x1838 /* (56,24) 9 */,
            0x1828 /* (40,24) 10 */, 0x1818 /* (24,24) 11 */,
            0x18F8 /* (-8,24) 12 */, 0x18E8 /* (-24,24) 13 */,
            0x18D8 /* (-40,24) 14 */, 0x0838 /* (56,8) 15 */,
            0x0828 /* (40,8) 16 */, 0x0818 /* (24,8) 17 */,
            0x0808 /* (8,8) 18 */, 0x08F8 /* (-8,8) 19 */,
            0x08E8 /* (-24,8) 20 */, 0x08D8 /* (-40,8) 21 */,
            0xF838 /* (56,-8) 22 */, 0xF828 /* (40,-8) 23 */,
            0xF818 /* (24,-8) 24 */, 0xF8F8 /* (-8,-8) 25 */,
            0xF8E8 /* (-24,-8) 26 */, 0xF8D8 /* (-40,-8) 27 */,
            0xE828 /* (40,-24) 28 */, 0xE818 /* (24,-24) 29 */,
            0xE808 /* (8,-24) 30 */, 0xE8F8 /* (-8,-24) 31 */,
            0xE8E8 /* (-24,-24) 32 */, 0xD818 /* (24,-40) 33 */,
            0xD808 /* (8,-40) 34 */, 0xD8F8 /* (-8,-40) 35 */,
            0xC808 /* (8,-56) 36 */, 0x1808 /* (8,24) 37 */,
            0x1808 /* (8,24) 38 */, 0 /* (0) 39 */ }    },

    { /* PatTraffic */
        psTraffic /* psInd */, 1 /* psNum */, 1 /* delayCnt */, 6 /* exInd */, {10,0,0,0} /* exNum[SIM_PS_NUM] */, 
        7 /* exOpInd */, 13 /* exOpNum */, 0 /* exCheckInd */, 4 /* exCheckNum */, 16 /* exActionInd */, 31 /* exActionNum */,
        {  /* color map */
            {255,0,0}, {255,43,1}, {255,86,1}, {255,128,1}, {255,170,1}, {255,231,1}, {255,255,1}, {219,255,1}, {176,255,0},
            {134,255,1}, {92,255,1}, {49,255,1}, {7,255,1}, {1,255,31}, {1,255,80}, {1,255,122}, {1,255,164}, {1,255,207},
            {1,255,249}, {1,255,255}, {1,182,255}, {1,140,255}, {1,98,255}, {1,55,255}, {1,13,255}, {31,1,255}, {1,170,255},
            {1,128,255}, {1,128,255}, {1,86,255}, {13,1,255}, {55,1,255}, {98,1,255}, {140,1,255}, {146,1,255}, {188,1,255},
            {188,1,255}, {231,1,255}, {255,1,243}, {255,1,201}, {255,1,208}, {255,1,116}, {255,1,90}, {255,1,37}, {255,1,1},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { /* operation id=0 */ pSeq /* pID */, coBrightnessAbs /* opType */, 60 /* sFactor */, ( ( moMul << 2 ) | moSub ) /* opCode */ },
            { /* operation id=1 */ pSystemID /* pID */, coBrightnessAbs /* opType */, 57 /* sFactor */, ( ( moAdd << 2 ) | moAdd ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        32 /* pMax */,
        { /* grid data array */
            0xA8B8 /* (-72,-88) 0 */, 0xA0C8 /* (-56,-96) 1 */,
            0xA4D8 /* (-40,-92) 2 */, 0xA2E8 /* (-24,-94) 3 */,
            0x9EF8 /* (-8,-98) 4 */, 0xA708 /* (8,-89) 5 */,
            0x9D18 /* (24,-99) 6 */, 0x9C28 /* (40,-100) 7 */,
            0xA838 /* (56,-88) 8 */, 0xB27A /* (122,-78) 9 */,
            0xCA7B /* (123,-54) 10 */, 0xD879 /* (121,-40) 11 */,
            0xE87D /* (125,-24) 12 */, 0xF878 /* (120,-8) 13 */,
            0x0879 /* (121,8) 14 */, 0x1879 /* (121,24) 15 */,
            0x287E /* (126,40) 16 */, 0x387D /* (125,56) 17 */,
            0x4897 /* (-105,72) 18 */, 0x3894 /* (-108,56) 19 */,
            0x2892 /* (-110,40) 20 */, 0x1896 /* (-106,24) 21 */,
            0x0890 /* (-112,8) 22 */, 0xF895 /* (-107,-8) 23 */,
            0xE893 /* (-109,-24) 24 */, 0xD896 /* (-106,-40) 25 */,
            0x6858 /* (88,104) 26 */, 0x6A48 /* (72,106) 27 */,
            0x6C38 /* (56,108) 28 */, 0x6928 /* (40,105) 29 */,
            0x6B18 /* (24,107) 30 */, 0x6E08 /* (8,110) 31 */,
            0x70F8 /* (-8,112) 32 */, 0x72E8 /* (-24,114) 33 */,
            0x6AD8 /* (-40,106) 34 */, 0x6CC8 /* (-56,108) 35 */,
            0x0500 /* (0,5) 36 */, 0xFB00 /* (0,-5) 37 */,
            0x0005 /* (5,0) 38 */, 0x00FB /* (-5,0) 39 */ }    },

    { /* PatExplosion */
        psExplosion /* psInd */, 1 /* psNum */, 1 /* delayCnt */, 16 /* exInd */, {0,0,0,0} /* exNum[SIM_PS_NUM] */, 
        20 /* exOpInd */, 14 /* exOpNum */, 0 /* exCheckInd */, 0 /* exCheckNum */, 47 /* exActionInd */, 26 /* exActionNum */,
        {  /* color map */
            {255,255,255}, {91,207,252}, {18,115,255}, {113,171,255}, {21,55,255}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},                   
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            //{ /* operation id=0 */ pAAmplitude /* pID */, coBrightnessAbs /* opType */, 0 /* sFactor */, ( ( moAdd << 2 ) | moSub ) /* opCode */ },
            //{ /* operation id=1 */ pAAmplitude /* pID */, coHueAbs /* opType */, 0 /* sFactor */, ( ( moAdd << 2 ) | moAdd ) /* opCode */ },
            //{ /* operation id=2 */ pSeq /* pID */, coBrightnessAbs /* opType */, 80 /* sFactor */, ( ( moMul << 2 ) | moSub ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
		36 /* pMax */,
        { /* grid data array */
            0x40E0 /* (-32,64) 0 */, 0x3020 /* (32,48) 1 */,
            0x4050 /* (80,64) 2 */, 0x20C0 /* (-64,32) 3 */,
            0x10F0 /* (-16,16) 4 */, 0xF030 /* (48,-16) 5 */,
            0xE0D0 /* (-48,-32) 6 */, 0xD010 /* (16,-48) 7 */,
            0xD050 /* (80,-48) 8 */, 0 /* (0) 9 */,
            0x4817 /* (23,72) 10 */, 0x3817 /* (23,56) 11 */,
            0x2817 /* (23,40) 12 */, 0x1817 /* (23,24) 13 */,
            0x0817 /* (23,8) 14 */, 0xF817 /* (23,-8) 15 */,
            0x48F7 /* (-9,72) 16 */, 0x3807 /* (7,56) 17 */,
            0x2807 /* (7,40) 18 */, 0x1807 /* (7,24) 19 */,
            0x08B7 /* (-73,8) 20 */, 0x18B7 /* (-73,24) 21 */,
            0x28B7 /* (-73,40) 22 */, 0x38B7 /* (-73,56) 23 */,
            0x48B7 /* (-73,72) 24 */, 0x48C7 /* (-57,72) 25 */,
            0x48D7 /* (-41,72) 26 */, 0x48D7 /* (-41,72) 27 */,
            0x48F7 /* (-9,72) 28 */, 0x4807 /* (7,72) 29 */,
            0x0807 /* (7,8) 30 */, 0x38F7 /* (-9,56) 31 */,
            0x28F7 /* (-9,40) 32 */, 0x18F7 /* (-9,24) 33 */,
            0x38E7 /* (-25,56) 34 */, 0x28E7 /* (-25,40) 35 */,
            0x18E7 /* (-25,24) 36 */, 0x38D7 /* (-41,56) 37 */,
            0x18D7 /* (-41,24) 38 */, 0x28D7 /* (-41,40) 39 */ }    },

    { /* PatWave */
        psWaveHighPitch /* psInd */, 4 /* psNum */, 2 /* delayCnt */, 26 /* exInd */, {2,2,2,4} /* exNum[SIM_PS_NUM] */, 
        34 /* exOpInd */, 18 /* exOpNum */, 0 /* exCheckInd */, 0 /* exCheckNum */, 73 /* exActionInd */, 4 /* exActionNum */,
        {  /* color map */
            {255,255,0}, {102,255,0}, {0,255,51}, {0,255,204}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { /* operation id=0 */ pSize /* pID */, coHueAbs /* opType */, 172 /* sFactor */, ( REN_CO_OC1_SWAP | ( moAdd << 2 ) | moAdd ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        48 /* pMax */,
        { /* grid data array */
            0 /* (0) 0 */, 0 /* (0) 1 */,
            0 /* (0) 2 */, 0 /* (0) 3 */,
            0 /* (0) 4 */, 0 /* (0) 5 */,
            0 /* (0) 6 */, 0 /* (0) 7 */,
            0 /* (0) 8 */, 0 /* (0) 9 */,
            0 /* (0) 10 */, 0 /* (0) 11 */,
            0 /* (0) 12 */, 0 /* (0) 13 */,
            0 /* (0) 14 */, 0 /* (0) 15 */,
            0 /* (0) 16 */, 0 /* (0) 17 */,
            0 /* (0) 18 */, 0 /* (0) 19 */,
            0 /* (0) 20 */, 0 /* (0) 21 */,
            0 /* (0) 22 */, 0 /* (0) 23 */,
            0 /* (0) 24 */, 0 /* (0) 25 */,
            0 /* (0) 26 */, 0 /* (0) 27 */,
            0 /* (0) 28 */, 0 /* (0) 29 */,
            0 /* (0) 30 */, 0 /* (0) 31 */,
            0 /* (0) 32 */, 0 /* (0) 33 */,
            0 /* (0) 34 */, 0 /* (0) 35 */,
            0 /* (0) 36 */, 0 /* (0) 37 */,
            0 /* (0) 38 */, 0 /* (0) 39 */ }    },

    { /* PatFirefly */
        psFirefly /* psInd */, 1 /* psNum */, 4 /* delayCnt */, 36 /* exInd */, {6,0,0,0} /* exNum[SIM_PS_NUM] */, 
        52 /* exOpInd */, 10 /* exOpNum */, 4 /* exCheckInd */, 2 /* exCheckNum */, 77 /* exActionInd */, 15 /* exActionNum */,
        {  /* color map */
            {200,255,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {200,255,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { /* operation id=0 */ pAAmplitude /* pID */, coBrightnessAbs /* opType */, 0 /* sFactor */, ( ( moAdd << 2 ) | moSub ) /* opCode */ },
            { /* operation id=1 */ pAngle /* pID */, coBreathAbs /* opType */, 90 /* sFactor */, ( REN_CO_OC0_SWAP | ( moSub << 2 ) | moMul ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        198 /* pMax */,
        { /* grid data array */
            0x0101 /* (1,1) 0 */, 0xFF01 /* (1,-1) 1 */,
            0x01FF /* (-1,1) 2 */, 0xFFFF /* (-1,-1) 3 */,
            0x0000 /* (0,0) 4 */, 0x0000 /* (0,0) 5 */,
            0x0000 /* (0,0) 6 */, 0x0000 /* (0,0) 7 */,
            0 /* (0) 8 */, 0 /* (0) 9 */,
            0 /* (0) 10 */, 0 /* (0) 11 */,
            0 /* (0) 12 */, 0 /* (0) 13 */,
            0 /* (0) 14 */, 0 /* (0) 15 */,
            0 /* (0) 16 */, 0 /* (0) 17 */,
            0 /* (0) 18 */, 0 /* (0) 19 */,
            0 /* (0) 20 */, 0 /* (0) 21 */,
            0 /* (0) 22 */, 0 /* (0) 23 */,
            0 /* (0) 24 */, 0 /* (0) 25 */,
            0 /* (0) 26 */, 0 /* (0) 27 */,
            0 /* (0) 28 */, 0 /* (0) 29 */,
            0 /* (0) 30 */, 0 /* (0) 31 */,
            0 /* (0) 32 */, 0 /* (0) 33 */,
            0 /* (0) 34 */, 0 /* (0) 35 */,
            0 /* (0) 36 */, 0 /* (0) 37 */,
            0 /* (0) 38 */, 0 /* (0) 39 */ }    },

    { /* PatCustomized */
        psCustomized /* psInd */, 1 /* psNum */, 0 /* delayCnt */, 0 /* exInd */, {0,0,0,0} /* exNum[SIM_PS_NUM] */, 
        0 /* exOpInd */, 0 /* exOpNum */, 0 /* exCheckInd */, 0 /* exCheckNum */, 0 /* exActionInd */, 0 /* exActionNum */,
        {  /* color map */
            { 255,  0, 0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            //{ /* operation id=0 */ pAAmplitude /* pID */, coBrightnessAbs /* opType */, 0 /* sFactor */, ( ( moAdd << 2 ) | moSub ) /* opCode */ },
            //{ /* operation id=1 */ pAAmplitude /* pID */, coHueAbs /* opType */, 0 /* sFactor */, ( ( moAdd << 2 ) | moAdd ) /* opCode */ },
            //{ /* operation id=2 */ pSeq /* pID */, coBrightnessAbs /* opType */, 80 /* sFactor */, ( ( moMul << 2 ) | moSub ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        1 /* pMax */,
        { /* grid data array */
            0x38E8 /* (-24,56) 0 */, 0x2818 /* (24,40) 1 */,
            0x3848 /* (72,56) 2 */, 0x18C8 /* (-56,24) 3 */,
            0x08F8 /* (-8,8) 4 */, 0xF828 /* (40,-8) 5 */,
            0xE8D8 /* (-40,-24) 6 */, 0xD808 /* (8,-40) 7 */,
            0xD848 /* (72,-40) 8 */, 0 /* (0) 9 */,
            0x4817 /* (23,72) 10 */, 0x3817 /* (23,56) 11 */,
            0x2817 /* (23,40) 12 */, 0x1817 /* (23,24) 13 */,
            0x0817 /* (23,8) 14 */, 0xF817 /* (23,-8) 15 */,
            0x48F7 /* (-9,72) 16 */, 0x3807 /* (7,56) 17 */,
            0x2807 /* (7,40) 18 */, 0x1807 /* (7,24) 19 */,
            0x08B7 /* (-73,8) 20 */, 0x18B7 /* (-73,24) 21 */,
            0x28B7 /* (-73,40) 22 */, 0x38B7 /* (-73,56) 23 */,
            0x48B7 /* (-73,72) 24 */, 0x48C7 /* (-57,72) 25 */,
            0x48D7 /* (-41,72) 26 */, 0x48D7 /* (-41,72) 27 */,
            0x48F7 /* (-9,72) 28 */, 0x4807 /* (7,72) 29 */,
            0x0807 /* (7,8) 30 */, 0x38F7 /* (-9,56) 31 */,
            0x28F7 /* (-9,40) 32 */, 0x18F7 /* (-9,24) 33 */,
            0x38E7 /* (-25,56) 34 */, 0x28E7 /* (-25,40) 35 */,
            0x18E7 /* (-25,24) 36 */, 0x38D7 /* (-41,56) 37 */,
            0x18D7 /* (-41,24) 38 */, 0x28D7 /* (-41,40) 39 */ }    },

    { /* PatFire */
        psFire /* psInd */, 1 /* psNum */, 1 /* delayCnt */, 48 /* exInd */, {5,0,0,0} /* exNum[SIM_PS_NUM] */, 
        73 /* exOpInd */, 8 /* exOpNum */, 6 /* exCheckInd */, 2 /* exCheckNum */, 109 /* exActionInd */, 15 /* exActionNum */,
        {  /* color map */
            {255,245,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { /* operation id=0 */ pLocationX /* pID */, coGreenAbs /* opType */, 2 /* sFactor */, ( REN_CO_OC1_SWAP | ( moMul << 2 ) | moSub ) /* opCode */ },
            //{ /* operation id=1 */ pLocationX /* pID */, coBrightnessNewAbs /* opType */, 2 /* sFactor */, ( REN_CO_OC1_SWAP | ( moMul << 2 ) | moSub ) /* opCode */ },
            { /* operation id=2 */ pAngle /* pID */, coHueAbs /* opType */, 0 /* sFactor */, ( ( moSub << 2 ) | moAdd ) /* opCode */ },
            { /* operation id=3 */ pVelocityY /* pID */, coBrightnessAbs /* opType */, 90 /* sFactor */, ( REN_CO_OC1_SWAP | ( moAdd << 2 ) | moSub ) /* opCode */ },
            { /* operation id=4 */ pLocationY /* pID */, coBlueAbs /* opType */, 2 /* sFactor */, ( REN_CO_OC1_SWAP | ( moMul << 2 ) | moSub ) /* opCode */ },
            { /* operation id=5 */ pLocationX /* pID */, coBlueAbs /* opType */, 2 /* sFactor */, ( REN_CO_OC1_SWAP | ( moMul << 2 ) | moSub ) /* opCode */ },
            { /* operation id=6 */ pLocationY /* pID */, coRedAbs /* opType */, 1 /* sFactor */, ( REN_CO_OC1_SWAP | ( moMul << 2 ) | moSub ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        108 /* pMax */,
        { /* grid data array */
            0xC050 /* (80,-64) 0 */, 0xC040 /* (64,-64) 1 */,
            0xC030 /* (48,-64) 2 */, 0xC020 /* (32,-64) 3 */,
            0xC010 /* (16,-64) 4 */, 0xC000 /* (0,-64) 5 */,
            0xC0F0 /* (-16,-64) 6 */, 0xC0E0 /* (-32,-64) 7 */,
            0xC0D0 /* (-48,-64) 8 */, 0xC0C0 /* (-64,-64) 9 */,
            0xC0B0 /* (-80,-64) 10 */, 0xC0A0 /* (-96,-64) 11 */,
            0xC4D8 /* (-40,-60) 12 */, 0xBCD8 /* (-40,-68) 13 */,
            0xBAC8 /* (-56,-70) 14 */, 0xB5B8 /* (-72,-75) 15 */,
            0xC608 /* (8,-58) 16 */, 0xBEF8 /* (-8,-66) 17 */,
            0xBB28 /* (40,-69) 18 */, 0xC6E8 /* (-24,-58) 19 */,
            0xBB48 /* (72,-69) 20 */, 0xC6A8 /* (-88,-58) 21 */,
            0xBB58 /* (88,-69) 22 */, 0xC4A8 /* (-88,-60) 23 */,
            0xB958 /* (88,-71) 24 */, 0xBAA8 /* (-88,-70) 25 */,
            0 /* (0) 26 */, 0 /* (0) 27 */,
            0 /* (0) 28 */, 0 /* (0) 29 */,
            0 /* (0) 30 */, 0 /* (0) 31 */,
            0x0100 /* (0,1) 32 */, 0x0400 /* (0,4) 33 */,
            0x0300 /* (0,3) 34 */, 0x0200 /* (0,2) 35 */,
            0x0301 /* (1,3) 36 */, 0x0402 /* (2,4) 37 */,
            0x03FF /* (-1,3) 38 */, 0x04FE /* (-2,4) 39 */ }    },

    { /* PatRainbow */
        psRainbow /* psInd */, 1 /* psNum */, 1 /* delayCnt */, 53 /* exInd */, {0,0,0,0} /* exNum[SIM_PS_NUM] */, 
        81 /* exOpInd */, 0 /* exOpNum */, 8 /* exCheckInd */, 0 /* exCheckNum */, 124 /* exActionInd */, 0 /* exActionNum */,
        {  /* color map */
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        1 /* pMax */,
        { /* grid data array */
            0 /* (0) 0 */, 0 /* (0) 1 */,
            0 /* (0) 2 */, 0 /* (0) 3 */,
            0 /* (0) 4 */, 0 /* (0) 5 */,
            0 /* (0) 6 */, 0 /* (0) 7 */,
            0 /* (0) 8 */, 0 /* (0) 9 */,
            0 /* (0) 10 */, 0 /* (0) 11 */,
            0 /* (0) 12 */, 0 /* (0) 13 */,
            0 /* (0) 14 */, 0 /* (0) 15 */,
            0 /* (0) 16 */, 0 /* (0) 17 */,
            0 /* (0) 18 */, 0 /* (0) 19 */,
            0 /* (0) 20 */, 0 /* (0) 21 */,
            0 /* (0) 22 */, 0 /* (0) 23 */,
            0 /* (0) 24 */, 0 /* (0) 25 */,
            0 /* (0) 26 */, 0 /* (0) 27 */,
            0 /* (0) 28 */, 0 /* (0) 29 */,
            0 /* (0) 30 */, 0 /* (0) 31 */,
            0 /* (0) 32 */, 0 /* (0) 33 */,
            0 /* (0) 34 */, 0 /* (0) 35 */,
            0 /* (0) 36 */, 0 /* (0) 37 */,
            0 /* (0) 38 */, 0 /* (0) 39 */ }    },

    { /* PatHourglass */
        psHourglass /* psInd */, 3 /* psNum */, 1 /* delayCnt */, 60 /* exInd */, {2,2,0,0} /* exNum[SIM_PS_NUM] */, 
        92 /* exOpInd */, 8 /* exOpNum */, 0 /* exCheckInd */, 0 /* exCheckNum */, 138 /* exActionInd */, 9 /* exActionNum */,
        {  /* color map */
            {0, 0, 0}, {255,255,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { /* operation id=0 */ pLifespan /* pID */, coColorIndAbs /* opType */, 1 /* sFactor */, ( ( moAdd << 2 ) | moSub ) /* opCode */ },
            { /* operation id=1 */ pSeq /* pID */, coBrightnessAbs /* opType */, 100 /* sFactor */, ( ( moMul << 2 ) | moSub ) /* opCode */ },
            { /* operation id=2 */ pSize /* pID */, coBrightnessAbs /* opType */, 1 /* sFactor */, ( ( moMul << 2 ) | moSub ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        32 /* pMax */,
        { /* grid data array */
            0 /* (0) 0 */, 0 /* (0) 1 */,
            0 /* (0) 2 */, 0 /* (0) 3 */,
            0 /* (0) 4 */, 0 /* (0) 5 */,
            0 /* (0) 6 */, 0 /* (0) 7 */,
            0 /* (0) 8 */, 0 /* (0) 9 */,
            0 /* (0) 10 */, 0 /* (0) 11 */,
            0 /* (0) 12 */, 0 /* (0) 13 */,
            0 /* (0) 14 */, 0 /* (0) 15 */,
            0 /* (0) 16 */, 0 /* (0) 17 */,
            0 /* (0) 18 */, 0 /* (0) 19 */,
            0 /* (0) 20 */, 0 /* (0) 21 */,
            0 /* (0) 22 */, 0 /* (0) 23 */,
            0 /* (0) 24 */, 0 /* (0) 25 */,
            0 /* (0) 26 */, 0 /* (0) 27 */,
            0 /* (0) 28 */, 0 /* (0) 29 */,
            0 /* (0) 30 */, 0 /* (0) 31 */,
            0 /* (0) 32 */, 0 /* (0) 33 */,
            0 /* (0) 34 */, 0 /* (0) 35 */,
            0 /* (0) 36 */, 0 /* (0) 37 */,
            0 /* (0) 38 */, 0 /* (0) 39 */ }    },

    { /* PatBar */
        psBar /* psInd */, 1 /* psNum */, 0 /* delayCnt */, 64 /* exInd */, {9,1,0,0} /* exNum[SIM_PS_NUM] */, 
        100 /* exOpInd */, 24 /* exOpNum */, 13 /* exCheckInd */, 13 /* exCheckNum */, 147 /* exActionInd */, 9 /* exActionNum */,
        {  /* color map */
            {255,0,0}, {255,255,0}, {0,255,0}, {0,153,204}, {0,0,255}, {255,0,255}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {255,255,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { /* operation id=0 */ pAngle /* pID */, coHueAbs /* opType */, 2 /* sFactor */, ( REN_CO_OC0_FIRST | ( moMul << 2 ) | moAdd ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        1 /* pMax */,
        { /* grid data array */
            0xC858 /* (88,-56) 0 */, 0xC848 /* (72,-56) 1 */,
            0xC838 /* (56,-56) 2 */, 0xC828 /* (40,-56) 3 */,
            0xC818 /* (24,-56) 4 */, 0xC808 /* (8,-56) 5 */,
            0xC8F8 /* (-8,-56) 6 */, 0xC8E8 /* (-24,-56) 7 */,
            0xC8D8 /* (-40,-56) 8 */, 0xC8C8 /* (-56,-56) 9 */,
            0xC8B8 /* (-72,-56) 10 */, 0xC8A8/* (-88,-56) 11 */,
            0 /* (0) 12 */, 0 /* (0) 13 */,
            0 /* (0) 14 */, 0 /* (0) 15 */,
            0 /* (0) 16 */, 0 /* (0) 17 */,
            0 /* (0) 18 */, 0 /* (0) 19 */,
            0 /* (0) 20 */, 0 /* (0) 21 */,
            0 /* (0) 22 */, 0 /* (0) 23 */,
            0 /* (0) 24 */, 0 /* (0) 25 */,
            0 /* (0) 26 */, 0 /* (0) 27 */,
            0 /* (0) 28 */, 0 /* (0) 29 */,
            0 /* (0) 30 */, 0 /* (0) 31 */,
            0 /* (0) 32 */, 0 /* (0) 33 */,
            0 /* (0) 34 */, 0 /* (0) 35 */,
            0 /* (0) 36 */, 0 /* (0) 37 */,
            0 /* (0) 38 */, 0 /* (0) 39 */ }    },
   { /* PatRave */
        psRave /* psInd */, 1 /* psNum */, 1 /* delayCnt */, 74 /* exInd */, {6,0,0,0} /* exNum[SIM_PS_NUM] */, 
        124 /* exOpInd */, 14 /* exOpNum */, 0 /* exCheckInd */, 0 /* exCheckNum */, 156 /* exActionInd */, 26 /* exActionNum */,
        {  /* color map */
            { 216,  10, 0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            //{ /* operation id=0 */ pAAmplitude /* pID */, coBrightnessAbs /* opType */, 0 /* sFactor */, ( ( moAdd << 2 ) | moSub ) /* opCode */ },
            //{ /* operation id=1 */ pAAmplitude /* pID */, coHueAbs /* opType */, 0 /* sFactor */, ( ( moAdd << 2 ) | moAdd ) /* opCode */ },
            //{ /* operation id=2 */ pSeq /* pID */, coBrightnessAbs /* opType */, 80 /* sFactor */, ( ( moMul << 2 ) | moSub ) /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        1 /* pMax */,
        { /* grid data array */
                0 /* (0) 0 */, 0 /* (0) 1 */,
                0 /* (0) 2 */, 0 /* (0) 3 */,
                0 /* (0) 4 */, 0 /* (0) 5 */,
                0 /* (0) 6 */, 0 /* (0) 7 */,
                0 /* (0) 8 */, 0 /* (0) 9 */,
                0 /* (0) 10 */, 0 /* (0) 11 */,
                0 /* (0) 12 */, 0 /* (0) 13 */,
                0 /* (0) 14 */, 0 /* (0) 15 */,
                0 /* (0) 16 */, 0 /* (0) 17 */,
                0 /* (0) 18 */, 0 /* (0) 19 */,
                0 /* (0) 20 */, 0 /* (0) 21 */,
                0 /* (0) 22 */, 0 /* (0) 23 */,
                0 /* (0) 24 */, 0 /* (0) 25 */,
                0 /* (0) 26 */, 0 /* (0) 27 */,
                0 /* (0) 28 */, 0 /* (0) 29 */,
                0 /* (0) 30 */, 0 /* (0) 31 */,
                0 /* (0) 32 */, 0 /* (0) 33 */,
                0 /* (0) 34 */, 0 /* (0) 35 */,
                0 /* (0) 36 */, 0 /* (0) 37 */,
                0 /* (0) 38 */, 0 /* (0) 39 */ }    },
    { /* PatJet */
        psJet /* psInd */, 1 /* psNum */, 1 /* delayCnt */, 6 /* exInd */, {10,0,0,0} /* exNum[SIM_PS_NUM] */, 
        7 /* exOpInd */, 13 /* exOpNum */, 0 /* exCheckInd */, 4 /* exCheckNum */, 16 /* exActionInd */, 31 /* exActionNum */,
        {  /* color map */
            {255,85,0}, {255,0,0}, {255,0,104}, {255,0,183}, {255,0,255}, {161,0,255}, {62,0,255}, {0,31,255}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
            {0,0,0}, {0,0,0}, {0,0,0} }, 
        { /* color operation */
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ },
            { 0 /*pID */, 0 /*opType */, 0 /* sFactor */, 0 /* opCode */ } }, 
        32 /* pMax */,
        { /* grid data array */
            0x3890 /* (-112,56) 0 */, 0x2860 /* (96,40) 1 */,
            0x1890 /* (-112,24) 2 */, 0x0860 /* (96,8) 3 */,
            0xF890 /* (-112,-8) 4 */, 0xE860 /* (96,-24) 5 */,
            0xD890 /* (-112,-40) 6 */, 0xC860 /* (96,-56) 7 */,
            0 /* (0) 8 */, 0 /* (0) 9 */,
            0 /* (0) 10 */, 0 /* (0) 11 */,
            0 /* (0) 12 */, 0 /* (0) 13 */,
            0 /* (0) 14 */, 0 /* (0) 15 */,
            0 /* (0) 16 */, 0 /* (0) 17 */,
            0 /* (0) 18 */, 0 /* (0) 19 */,
            0 /* (0) 20 */, 0 /* (0) 21 */,
            0 /* (0) 22 */, 0 /* (0) 23 */,
            0 /* (0) 24 */, 0 /* (0) 25 */,
            0 /* (0) 26 */, 0 /* (0) 27 */,
            0 /* (0) 28 */, 0 /* (0) 29 */,
            0 /* (0) 30 */, 0 /* (0) 31 */,
            0 /* (0) 32 */, 0 /* (0) 33 */,
            0 /* (0) 34 */, 0 /* (0) 35 */,
            0 /* (0) 36 */, 0 /* (0) 37 */,
            0 /* (0) 38 */, 0 /* (0) 39 */ }    }
};

/* 2. Particle system basic */
/**********************************************************************************************
* Array name:  patPsb
* Description: Pattern particle system data
* Remarks:     patPsb[2] is the particle system base information for system with id = 2
*
**********************************************************************************************/
LOCAL CONST PS_BASE_T patPsb[] =
{

    { /* psFireworkRaiseUp */
        psFireworkRaiseUp /* id */, 1 /* seqMax */, 10 /* createRate */, 3 /* maxCnt */, 0 /* currentCnt */, 2 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 5 /* aVelMin */, 5 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 16 /* maxSpeedMin */, 16 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        80 /* lifeMin */, 80 /* lifeMax */, 25 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0xC8C8 /* (-56,-56) locMin */, 0xC846 /* (70,-56) locMax */,
        0x0500 /* (0,5) velMin */, 0x0600 /* (0,6) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_PAT_MAX_CNT | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psFireworkExplode */
        psFireworkExplode /* id */, 1 /* seqMax */, 0 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 1 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 1 /* aVelMin */, 1 /* aVelMax */, 8 /* aAmpMin */, 8 /* aAmpMax */,
        90 /* sizeMin */, 90 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        25 /* lifeMin */, 25 /* lifeMax */, 250 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x0000 /* (0,0) locMin */, 0x0000 /* (0,0) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psTraffic */
        psTraffic /* id */, 1 /* seqMax */, 10 /* createRate */, 6 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 5 /* maxSpeedMin */, 5 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 250 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0 /* (0) locMin */, 35 /* (35) locMax */,
        36 /* (36) velMin */, 39 /* (39) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_TARGET_UPD_FIX ) /* psFlag */,
        ( 0 | SIM_PSB_RS_LOC | SIM_PSB_RS_VEL | 0 ) /* randSnap */ },

    { /* psExplosion */
        psExplosion /* id */, 1 /* seqMax */, 1 /* createRate */, 8 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 2 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 0 /* aAmpMin */, 59 /* aAmpMax */,
        8 /* sizeMin */, 32 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        80 /* 50 lifeMin */, 89 /* 59 lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0xC0A0 /* (-96, -64) locMin */, 0x3050 /* (80, 48) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_RAND_CREATE | 0 ) /* psFlag */,
        ( 0 | SIM_PSB_RS_ALIGN_LOC | 0 ) /* randSnap */ },

    { /* psMeteor */
        psMeteor /* id */, 1 /* seqMax */, 0 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 180 /* aAmpMin */, 180 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 20 /* maxSpeedMin */, 20 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        40 /* lifeMin */, 40 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        10 /* (10) locMin */, 39 /* (39) locMax */,
        0xF808 /* (8,-8) velMin */, 0xF808 /* (8,-8) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | SIM_PSB_RS_LOC | 0 ) /* randSnap */ },

    { /* psMeteorShower */
        psMeteorShower /* id */, 3 /* seqMax */, 0 /* createRate */, 9 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 16 /* maxSpeedMin */, 16 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        15 /* lifeMin */, 40 /* lifeMax */, 250 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        20 /* (20) locMin */, 29 /* (29) locMax */,
        0xF808 /* (8,-8) velMin */, 0xF808 /* (8,-8) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( 0 | SIM_PSB_RAND_CREATE | 0 ) /* psFlag */,
        ( 0 | SIM_PSB_RS_LOC | 0 ) /* randSnap */ },

    { /* psWaveHighPitch */
        psWaveHighPitch /* id */, 1 /* seqMax */, 0 /* createRate */, 12 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 45 /* angleMax */, 12 /* aVelMin */, 12 /* aVelMax */, 1 /* aAmpMin */, 1 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 1 /* growMin */, 1 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x48A0 /* (-96,72) locMin */, 0x48A0 /* (-96,72) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_SHM | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psWaveBass */
        psWaveBass /* id */, 1 /* seqMax */, 0 /* createRate */, 12 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        90 /* angleMin */, 135 /* angleMax */, 12 /* aVelMin */, 12 /* aVelMax */, 1 /* aAmpMin */, 1 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 1 /* growMin */, 1 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x18A0 /* (-96,24) locMin */, 0x18A0 /* (-96,24) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_SHM | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psWaveBasic2 */
        psWaveBasic2 /* id */, 1 /* seqMax */, 0 /* createRate */, 12 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 45 /* angleMax */, 12 /* aVelMin */, 12 /* aVelMax */, 1 /* aAmpMin */, 1 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 1 /* growMin */, 1 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0xF8A0 /* (-96,-8) locMin */, 0xF8A0 /* (-96,-8) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_SHM | SIM_PSB_TARGET_UPD_MAXY ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psWaveBasic1 */
        psWaveBasic1 /* id */, 1 /* seqMax */, 0 /* createRate */, 12 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        90 /* angleMin */, 135 /* angleMax */, 12 /* aVelMin */, 12 /* aVelMax */, 1 /* aAmpMin */, 1 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 1 /* growMin */, 1 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0xD8A0 /* (-96,-40) locMin */, 0xD8A0 /* (-96,-40) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_SHM | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psFirefly */
        psFirefly /* id */, 2 /* seqMax */, 1 /* createRate */, 18 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 8 /* aVelMin */, 8 /* aVelMax */, 50 /* aAmpMin */, 150 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 4 /* massMax */, 2 /* maxSpeedMin */, 3 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        35 /* lifeMin */, 45 /* lifeMax */, 125 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0xCEB8 /* (-72,-50) locMin */, 0x4258 /* (88,66) locMax */,
        0 /* (0) velMin */, 7 /* (7) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_RAND_CREATE | 0 ) /* psFlag */,
        ( 0 | SIM_PSB_RS_VEL | 0 ) /* randSnap */ },

    { /* psCustomized */
        psCustomized/* id */, 1 /* seqMax */, 1 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 50 /* aAmpMin */, 220 /* aAmpMax */,
        0 /* sizeMin */, 0/* sizeMax */, 0 /* growMin */, 0 /* growMax */, 0 /* massMin */, 0 /* massMax */, 0/* maxSpeedMin */, 0/* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 0 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /*59 lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0 /* (0) locMin */, 0 /* (8) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 ) /* randSnap */ },

    { /* psCustomizedBounce */
        psCustomizedBounce /* id */, 1 /* seqMax */, 0 /* createRate */, 4 /* maxCnt */, 0 /* currentCnt */, 1 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 250 /* aAmpMin */, 250 /* aAmpMax */,
        50 /* sizeMin */, 50 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 0 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        3 /* lifeMin */, 3 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x0000 /* (0,0) locMin */, 0x0000 /* (0,0) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psLightning */
        psLightning /* id */, 1 /* seqMax */, 1 /* createRate */, 0 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        145 /* angleMin */, 175 /* angleMax */, 30 /* aVelMin */, 60 /* aVelMax */, 15 /* aAmpMin */, 52 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        3 /* lifeMin */, 6 /* lifeMax */, 125 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x00E8 /* (-24,0) locMin */, 0x5018 /* (24,80) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psFire */
        psFire /* id */, 1 /* seqMax */, 13 /* createRate */, 96 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 1 /* aVelMin */, 1 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 5 /* maxSpeedMin */, 5 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        5 /* lifeMin */, 48 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0 /* (0) locMin */, 11 /* (11) locMax */,
        35 /* (35) velMin */, 35 /* (35) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_RAND_CREATE | 0 ) /* psFlag */,
        ( 0 | SIM_PSB_RS_LOC | SIM_PSB_RS_VEL | 0 ) /* randSnap */ },

    { /* psRainbow */
        psRainbow /* id */, 1 /* seqMax */, 1 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 0 /* massMin */, 0 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 0 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x0 /* (0,0) locMin */, 0x00 /* (0,0) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psHourglass */
        psHourglass /* id */, 1 /* seqMax */, 1 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 5 /* maxSpeedMin */, 5 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 0 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        75 /* lifeMin */, 75 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x5000 /* (0,80) locMin */, 0x5000 /* (0,80) locMax */,
        0xFE00 /* (0,-2) velMin */, 0xFE00 /* (0,-2) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psHourglassStack */
        psHourglassStack /* id */, 1 /* seqMax */, 1 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 1 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        1 /* sizeMin */, 1 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 20 /* maxSpeedMin */, 20 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 0 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        1500 /* lifeMin */, 1500 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0xBA00 /* (0,-70) locMin */, 0xBA00 /* (0,-70) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psHourglassStackBlink */
        psHourglassStackBlink /* id */, 1 /* seqMax */, 0 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 1 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 168 /* aAmpMin */, 168 /* aAmpMax */,
        1 /* sizeMin */, 1 /* sizeMax */, 10 /* growMin */, 10 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 0 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        25 /* lifeMin */, 25 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x5800 /* (0,88) locMin */, 0x5800 /* (0,88) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psBar */
        psBar /* id */, 1 /* seqMax */, 1 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        1 /* angleMin */, 1 /* angleMax */, 1 /* aVelMin */, 1 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        0 /* sizeMin */, 0 /* sizeMax */, -6 /* growMin */, -6 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, -1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 0 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0 /* (0) locMin */, 11 /* (11) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | 0 ) /* psFlag */,
        ( 0 | SIM_PSB_RS_LOC | 0 ) /* randSnap */ },

    { /* psRave */
        psRave/* id */, 1 /* seqMax */, 1 /* createRate */, 1 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 1 /* aVelMin */, 1 /* aVelMax */, 50 /* aAmpMin */, 220 /* aAmpMax */,
        50 /* sizeMin */, 50 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 0 /* maxSpeedMin */, 0 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 0 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 10 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0 /* (0) locMin */, 0 /* (8) locMax */,
        0x0000 /* (0,0) velMin */, 0x0000 /* (0,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW | SIM_PSB_RAND_CREATE | SIM_PSB_RAND_SEQ | 0 ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ },

    { /* psJet */
        psJet /* id */, 1 /* seqMax */, 1 /* createRate */, 32 /* maxCnt */, 0 /* currentCnt */, 0 /* maxState */, 1 /* timeMul */,
        0 /* angleMin */, 0 /* angleMax */, 0 /* aVelMin */, 0 /* aVelMax */, 0 /* aAmpMin */, 0 /* aAmpMax */,
        3 /* sizeMin */, 3 /* sizeMax */, 0 /* growMin */, 0 /* growMax */, 1 /* massMin */, 1 /* massMax */, 5 /* maxSpeedMin */, 5 /* maxSpeedMax */,
        0 /* maxSteerMin */, 0 /* maxSteerMax */, 1 /*sizeAspect */, 0 /* separation */, 7 /* cohesion */,
        65535 /* lifeMin */, 65535 /* lifeMax */, 1 /* effectCnt */,
        {0, 0} /* (0,0) target */,
        0x38A8 /* (-88, 56) locMin */, 0x3858 /* (88, 56) locMax */,
        0x00FB /* (-5,0) velMin */, 0x0005 /* (5,0) velMax */,
        0x0000 /* (0,0) accMin */, 0x0000 /* (0,0) accMax */,
        NULL /* windPtr */, NULL /* forcePtr */,
        ( SIM_PSB_SHOW ) /* psFlag */,
        ( 0 | 0 ) /* randSnap */ }
};

/* 3. Particle system image information */
/**********************************************************************************************
* Array name:  patPsi
* Description: Pattern image information
* Remarks:     patPsi[2] is the image information of particle system with id = 2
*
**********************************************************************************************/
LOCAL CONST PS_IMAGE_INFO_T patPsi[] =
{

    { /* psFireworkRaiseUp */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuIncrement ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 45 /* colorMapLen */, 45 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psFireworkExplode */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 45 /* colorMapLen */, 0 /* colorTargetInd */,
        stFirework /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psTraffic */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuIncrement ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 45 /* colorMapLen */, 6 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psExplosion */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuIncrement ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 1 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 5 /* colorMapLen */, 0 /* colorTargetInd */,
        stExplosion /* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psMeteor */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuIncrement ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 1 /* updateColorInd */, 1 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 5 /* colorMapLen */, 0 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psMeteorShower */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuIncrement ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 2 /* updateColorInd */, 1 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 5 /* colorMapLen */, 0 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psWaveHighPitch */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 5 /* colorMapLen */, 0 /* colorTargetInd */,
        stWave /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psWaveBass */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        1 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 5 /* colorMapLen */, 0 /* colorTargetInd */,
        stWave /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psWaveBasic2 */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        2 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 5 /* colorMapLen */, 0 /* colorTargetInd */,
        stWave /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psWaveBasic1 */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        3 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 5 /* colorMapLen */, 0 /* colorTargetInd */,
        stWave /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psFirefly */
        ( 0 | REN_PSI_OLOCK90 | csuFixed ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 1 /* icNum */, 1 /* updateColorInd */, 1 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psCustomized */
    ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed )/*( 0 | REN_PSI_OLOCK90 | csuIncrement )*/ /* psiFlag */,
    0 /* seedColorInd */,
    0 /* initColorInd */, 1 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
    0 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
    stCustomized/* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psCustomizedBounce */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuFixed ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 1 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psLightning */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        2 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 1 /* updateColorInd */, 2 /* ucNum */, ssmSolid /* sColorMethod */,
        2 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        (stImage + 48 + 0) /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psFire */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuFixed ) /* psiFlag */,
        0 /* seedColorInd */,
        1 /* initColorInd */, 2 /* icNum */, 2 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psRainbow */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuFixed ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 1 /* icNum */, 0 /* updateColorInd */, 2 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 9 /* colorMapLen */, 0 /* colorTargetInd */,
        stRainbow /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psHourglass */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuFixed ) /* psiFlag */,
        1 /* seedColorInd */,
        1 /* initColorInd */, 1 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        1 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stNull /* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psHourglassStack */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        1 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        1 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stHGStack /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psHourglassStackBlink */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        1 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 2 /* updateColorInd */, 1 /* ucNum */, ssmSolid /* sColorMethod */,
        1 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stHGStack /* shapeFlag */, bgtInvalid /* bgFlag */ },

    { /* psBar */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 1 /* ucNum */, ssmSpreadHue /* sColorMethod */,
        0 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stBar /* shapeFlag */, bgtBlack /* bgFlag */ },
        
     { /* psRave */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuFixed )/*( 0 | REN_PSI_OLOCK90 | csuIncrement )*/ /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 1 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmSolid /* sColorMethod */,
        0 /* colorMapInd */, 1 /* colorMapLen */, 0 /* colorTargetInd */,
        stRave /* shapeFlag */, bgtBlack /* bgFlag */ },

    { /* psJet */
        ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | REN_PSI_COLOR_OL | csuIncrement ) /* psiFlag */,
        0 /* seedColorInd */,
        0 /* initColorInd */, 0 /* icNum */, 0 /* updateColorInd */, 0 /* ucNum */, ssmFading /* sColorMethod */,
        0 /* colorMapInd */, 45 /* colorMapLen */, 6 /* colorTargetInd */,
        stDotTrack /* shapeFlag */, bgtBlack /* bgFlag */ }
};

/* 4. Particle system background image */
/**********************************************************************************************
* Array name:  patBgi
* Description: Pattern background image
* Remarks:
*
**********************************************************************************************/
LOCAL CONST VIRDIS_T patBgi[] =
{
//    { /* PS_FIRE 10 */
//        { /* UINT8 r[REN_VIR_DIS_FRAME][REN_VIR_DIS_FRAME]; 12 * 12  Virtual display buffer, red color */
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0
//        },
//        { /* UINT8 g[REN_VIR_DIS_FRAME][REN_VIR_DIS_FRAME]; 12 * 12 Virtual display buffer, green color */
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0
//        },
//        { /* UINT8 g[REN_VIR_DIS_FRAME][REN_VIR_DIS_FRAME]; 12 * 12 Virtual display buffer, green color */
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0,
//            0,0,0,0, 0,0,0,0, 0,0,0,0
//        }
//    }
0};

/* 5. Particle system shape image */
/**********************************************************************************************
* Array name:  patSi
* Description: Pattern shape image
* Remarks:     UINT8 shape[PAT_SHAPE_H]; 8 * 8   Array of the basic shape of the particles
*
**********************************************************************************************/
LOCAL CONST SHAPE_T patSi[] =
{
    /* PS_LIGHTNING 9, small lightning 1 */
    //{ 0x00, 0x08, 0x10, 0x3C, 0x08, 0x10, 0x00, 0x00 },
    { 0x000, 0x000, 0x000, 0x001, 0x003, 0x03C, 0x0E0, 0x0C0, 0x180, 0x000, 0x000, 0x000 }/* big */,
 
    /* PS_LIGHTNING 9, big lightning 2 */
    //{ 0x0C, 0x18, 0x10, 0x3C, 0x08, 0x10, 0x20, 0x00 }
    //{ 0x800, 0xC00, 0x3C0, 0x070, 0x030, 0x018, 0x000, 0x000, 0x000 } /* big */,
    { 0x000, 0x000, 0x800, 0x400, 0x180, 0x040, 0x020, 0x020, 0x010, 0x008, 0x006, 0x000 }/* big */,
    { 0x000, 0x000, 0x000, 0x300, 0x080, 0x040, 0x020, 0x030, 0x008, 0x004, 0x000, 0x000 }/* small */,
    { 0x000, 0x000, 0x000, 0x00C, 0x010, 0x020, 0x040, 0x0C0, 0x100, 0x200, 0x000, 0x000 }/* small */
};

/* 6. Particle system external input information */
/* 6.1 External effect */
/**********************************************************************************************
* Array name:  patExEffect
* Description: Pattern external input information
* Remarks:     patExEffect[ ] is the first external input information of particle system
*              indicated in patMeta[].exInd
**********************************************************************************************/
LOCAL CONST SIM_EX_EFFECT_T patExEffect[] =
{

    /* psFireworkRaiseUp */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 3 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 5 /* opOffset */, 1 /* opNum */ },

    /* psFireworkExplode */
    { /* effect id=5 */ sdtAlways /* decisionType */, 6 /* opOffset */, 1 /* opNum */ },

    /* psTraffic */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 3 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 5 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 6 /* opOffset */, 1 /* opNum */ },
    { /* effect id=6 */ sdtAlways /* decisionType */, 7 /* opOffset */, 1 /* opNum */ },
    { /* effect id=7 */ sdtAlways /* decisionType */, 8 /* opOffset */, 1 /* opNum */ },
    { /* effect id=8 */ sdtAlways /* decisionType */, 9 /* opOffset */, 1 /* opNum */ },
    { /* effect id=9 */ sdtAlways /* decisionType */, 12 /* opOffset */, 1 /* opNum */ },

    /* psExplosion */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 1 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 5 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 7 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },

    /* psMeteor */
    { /* effect id=6 */ sdtAlways /* decisionType */, 8 /* opOffset */, 1 /* opNum */ },
    { /* effect id=7 */ sdtAlways /* decisionType */, 9 /* opOffset */, 0 /* opNum */ },
    { /* effect id=8 */ sdtAlways /* decisionType */, 11 /* opOffset */, 0 /* opNum */ },

    /* psMeteorShower */
    { /* effect id=9 */ sdtAlways /* decisionType */, 13 /* opOffset */, 1 /* opNum */ },

    /* psWaveHighPitch */
    { /* effect id=0 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },

    /* psWaveBass */
    { /* effect id=2 */ sdtAlways /* decisionType */, 6 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },

    /* psWaveBasic2 */
    { /* effect id=4 */ sdtAlways /* decisionType */, 8 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 10 /* opOffset */, 1 /* opNum */ },

    /* psWaveBasic1 */
    { /* effect id=6 */ sdtAlways /* decisionType */, 14 /* opOffset */, 1 /* opNum */ },
    { /* effect id=7 */ sdtAlways /* decisionType */, 12 /* opOffset */, 1 /* opNum */ },
    { /* effect id=8 */ sdtAlways /* decisionType */, 15 /* opOffset */, 1 /* opNum */ },
    { /* effect id=9 */ sdtAlways /* decisionType */, 16 /* opOffset */, 1 /* opNum */ },

    /* psFirefly */
    { /* effect id=0 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 5 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 6 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 1 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },

    /* psCustomized */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 1 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },

    /* psCustomizedBounce */

    /* psLightning */
    { /* effect id=3 */ sdtAlways /* decisionType */, 5 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 9 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 7 /* opOffset */, 1 /* opNum */ },

    /* psFire */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 3 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 7 /* opOffset */, 1 /* opNum */ },

    /* psRainbow */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 1 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 3 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 6 /* opOffset */, 1 /* opNum */ },
    { /* effect id=6 */ sdtAlways /* decisionType */, 10 /* opOffset */, 1 /* opNum */ },

    /* psHourglass */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 3 /* opOffset */, 1 /* opNum */ },

    /* psHourglassStack */
    { /* effect id=2 */ sdtAlways /* decisionType */, 6 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 7 /* opOffset */, 1 /* opNum */ },

    /* psHourglassStackBlink */

    /* psBar */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 3 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 6 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 8 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 11 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 13 /* opOffset */, 1 /* opNum */ },
    { /* effect id=6 */ sdtAlways /* decisionType */, 16 /* opOffset */, 1 /* opNum */ },
    { /* effect id=7 */ sdtAlways /* decisionType */, 19 /* opOffset */, 1 /* opNum */ },
    { /* effect id=8 */ sdtAlways /* decisionType */, 21 /* opOffset */, 1 /* opNum */ },

    /* psFountainNoUp */
    { /* effect id=9 */ sdtAlways /* decisionType */, 23 /* opOffset */, 1 /* opNum */ },

    /* psFountainNoDown */
    /* psRave */
    { /* effect id=0 */ sdtAlways /* decisionType */, 0 /* opOffset */, 1 /* opNum */ },
    { /* effect id=1 */ sdtAlways /* decisionType */, 1 /* opOffset */, 1 /* opNum */ },
    { /* effect id=2 */ sdtAlways /* decisionType */, 2 /* opOffset */, 1 /* opNum */ },
    { /* effect id=3 */ sdtAlways /* decisionType */, 5 /* opOffset */, 1 /* opNum */ },
    { /* effect id=4 */ sdtAlways /* decisionType */, 7 /* opOffset */, 1 /* opNum */ },
    { /* effect id=5 */ sdtAlways /* decisionType */, 4 /* opOffset */, 1 /* opNum */ }
};

/* 6.2 Effect operation */
/**********************************************************************************************
* Array name:  patExOp
* Description: Pattern external input opeartions
* Remarks:     Indexed from external effect record
*
**********************************************************************************************/
LOCAL CONST SIM_OPERATION_T patExOp[] =
{

    /* psFireworkRaiseUp */
    { /* operation id=0 */ psexiMicLevel /* inputID */, 1 /* threshold */, 1 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=1 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 8 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=2 */ psexi300Hz /* inputID */, 5 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=3 */ psexi10kHz /* inputID */, 5 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 3 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=4 */ psexiApp /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 13 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=5 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 14 /* actionOffset */, 1 /* actionNum */ },

    /* psFireworkExplode */
    { /* operation id=6 */ psexiDC /* inputID */, 66 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 15 /* actionOffset */, 1 /* actionNum */ },

    /* psTraffic */
    { /* operation id=0 */ psexiMicLevel /* inputID */, 1 /* threshold */, 1 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=1 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=2 */ psexi300Hz /* inputID */, 33 /* threshold */, 11 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 6 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=3 */ psexi1kHz /* inputID */, 33 /* threshold */, 11 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 10 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=4 */ psexi10kHz /* inputID */, 33 /* threshold */, 11 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 15 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=5 */ psexiDC /* inputID */, 66 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 1 /* checkNum */, 24 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=6 */ psexiDC /* inputID */, 66 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 1 /* checkOffset */, 1 /* checkNum */, 21 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=7 */ psexiDC /* inputID */, 66 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 2 /* checkOffset */, 1 /* checkNum */, 25 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=8 */ psexiDC /* inputID */, 66 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 3 /* checkOffset */, 1 /* checkNum */, 20 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=9 */ psexiShake /* inputID */, 1 /* threshold */, 10 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 28 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=10 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 29 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=11 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=12 */ psexiApp /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 30 /* actionOffset */, 1 /* actionNum */ },

    /* psExplosion */
    { /* operation id=0 */ psexi300Hz /* inputID */, 33 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=1 */ psexi1kHz /* inputID */, 33 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 4 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=2 */ psexi10kHz /* inputID */, 33 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 9 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=3 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 13 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=4 */ psexiDC /* inputID */, 80 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 19 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=5 */ psexiMicLevel /* inputID */, 20 /* threshold */, 3 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 19 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=6 */ psexiMicLevel /* inputID */, 5 /* threshold */, 3 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 18 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=7 */ psexiMicLevel /* inputID */, 45 /* threshold */, 6 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 20 /* actionOffset */, 1 /* actionNum */ },

    /* psMeteor */
    { /* operation id=8 */ psexiApp /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 21 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=9 */ psexi300Hz /* inputID */, 80 /* threshold */, 10 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 22 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=10 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 24 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=11 */ psexiMicLevel /* inputID */, 10 /* threshold */, 12 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 22 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=12 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 24 /* actionOffset */, 1 /* actionNum */ },

    /* psMeteorShower */
    { /* operation id=13 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 25 /* actionOffset */, 1 /* actionNum */ },

    /* psWaveHighPitch */
    { /* operation id=0 */ psexiDC /* inputID */, 12 /* threshold */, 1 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=1 */ psexiDC /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=2 */ psexiMicLevel /* inputID */, 1 /* threshold */, 3 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=3 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },

    /* psWaveBass */
    { /* operation id=4 */ psexiDC /* inputID */, 12 /* threshold */, 5 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=5 */ psexiDC /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=6 */ psexiMicLevel /* inputID */, 1 /* threshold */, 7 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=7 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },

    /* psWaveBasic2 */
    { /* operation id=8 */ psexiMicLevel /* inputID */, 1 /* threshold */, 9 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=9 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=10 */ psexiDC /* inputID */, 12 /* threshold */, 11 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=11 */ psexiDC /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },

    /* psWaveBasic1 */
    { /* operation id=12 */ psexiMicLevel /* inputID */, 1 /* threshold */, 13 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=13 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=14 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 3 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=15 */ psexiApp /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 2 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=16 */ psexiDC /* inputID */, 12 /* threshold */, 17 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=17 */ psexiDC /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },

    /* psFirefly */
    { /* operation id=0 */ psexiApp /* inputID */, 1 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 9 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=1 */ psexi300Hz /* inputID */, 40 /* threshold */, 8 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 13 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=2 */ psexiDC /* inputID */, 60 /* threshold */, 3 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=3 */ psexiDC /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 3 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=4 */ psexiMicLevel /* inputID */, 1 /* threshold */, 9 /* elseOpInd */, 1 /* checkOffset */, 1 /* checkNum */, 10 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=5 */ psexiMicLevel /* inputID */, 1 /* threshold */, 9 /* elseOpInd */, 0 /* checkOffset */, 1 /* checkNum */, 11 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=6 */ psexiShake /* inputID */, 1 /* threshold */, 7 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 5 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=7 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=8 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=9 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 4 /* actionOffset */, 1 /* actionNum */ },

    /* psCustomized */
    { /* operation id=0 */ psexi300Hz /* inputID */, 95 /* threshold */, 3 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=1 */ psexiMicLevel /* inputID */, 1 /* threshold */, 2 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=2 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 6 /* actionOffset */, 4 /* actionNum */ },
    { /* operation id=3 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 6 /* actionOffset */, 4 /* actionNum */ },
    { /* operation id=4 */ psexiApp /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },

    /* psCustomizedBounce */

    /* psLightning */
    { /* operation id=5 */ psexiDC /* inputID */, 80 /* threshold */, 6 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 10 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=6 */ psexiDC /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 16 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=7 */ psexiShake /* inputID */, 1 /* threshold */, 8 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 10 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=8 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 16 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=9 */ psexiMicLevel /* inputID */, 1 /* threshold */, 10 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 10 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=10 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 16 /* actionOffset */, 1 /* actionNum */ },

    /* psFire */
    { /* operation id=0 */ psexiMicLevel /* inputID */, 5 /* threshold */, 1 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 2 /* actionOffset */, 4 /* actionNum */ },
    { /* operation id=1 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 6 /* actionOffset */, 4 /* actionNum */ },
    { /* operation id=2 */ psexiDC /* inputID */, 12 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 1 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=3 */ psexi300Hz /* inputID */, 12 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 0 /* actionNum */ },
    { /* operation id=4 */ psexiShake /* inputID */, 120 /* threshold */, 5 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 12 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=5 */ psexiShake /* inputID */, 1 /* threshold */, 6 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 2 /* actionOffset */, 4 /* actionNum */ },
    { /* operation id=6 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 6 /* actionOffset */, 6 /* actionNum */ },
    { /* operation id=7 */ psexiApp /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 14 /* actionOffset */, 1 /* actionNum */ },

    /* psRainbow */
    { /* operation id=0 */ psexiColor /* inputID */, 1 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=1 */ psexiApp /* inputID */, 1 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=2 */ psexi300Hz /* inputID */, 33 /* threshold */, 5 /* elseOpInd */, 0 /* checkOffset */, 1 /* checkNum */, 5 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=3 */ psexi10kHz /* inputID */, 3 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 3 /* checkOffset */, 2 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=4 */ psexi10kHz /* inputID */, 3 /* threshold */, 9 /* elseOpInd */, 3 /* checkOffset */, 2 /* checkNum */, 2 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=5 */ psexi300Hz /* inputID */, 2 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=6 */ psexiMicLevel /* inputID */, 5 /* threshold */, 7 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 9 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=3 */ psexi10kHz /* inputID */, 3 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 3 /* checkOffset */, 2 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=8 */ psexi1kHz /* inputID */, 1 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 1 /* checkOffset */, 2 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=9 */ psexi1kHz /* inputID */, 2 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 1 /* checkOffset */, 2 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=10 */ psexi1kHz /* inputID */, 3 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 1 /* checkOffset */, 2 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },

    /* psHourglass */
    { /* operation id=0 */ psexiMicLevel /* inputID */, 10 /* threshold */, 1 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=1 */ psexiMicLevel /* inputID */, 2 /* threshold */, 2 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 2 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=2 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 4 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=3 */ psexi300Hz /* inputID */, 80 /* threshold */, 4 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=4 */ psexi300Hz /* inputID */, 50 /* threshold */, 5 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 2 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=5 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 4 /* actionOffset */, 2 /* actionNum */ },

    /* psHourglassStack */
    { /* operation id=6 */ psexiApp /* inputID */, 1 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 6 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=7 */ psexiShake /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 7 /* actionOffset */, 2 /* actionNum */ },

    /* psHourglassStackBlink */

    /* psBar */
    { /* operation id=0 */ psexi300Hz /* inputID */, 85 /* threshold */, 1 /* elseOpInd */, 0 /* checkOffset */, 1 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=1 */ psexi300Hz /* inputID */, 22 /* threshold */, 2 /* elseOpInd */, 0 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=2 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 1 /* checkNum */, 4 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=3 */ psexi300Hz /* inputID */, 85 /* threshold */, 4 /* elseOpInd */, 1 /* checkOffset */, 1 /* checkNum */, 5 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=4 */ psexi300Hz /* inputID */, 11 /* threshold */, 5 /* elseOpInd */, 1 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=5 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 1 /* checkOffset */, 1 /* checkNum */, 4 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=6 */ psexi300Hz /* inputID */, 85 /* threshold */, 7 /* elseOpInd */, 2 /* checkOffset */, 1 /* checkNum */, 6 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=7 */ psexi300Hz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 2 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=8 */ psexi1kHz /* inputID */, 85 /* threshold */, 9 /* elseOpInd */, 3 /* checkOffset */, 1 /* checkNum */, 5 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=9 */ psexi1kHz /* inputID */, 11 /* threshold */, 10 /* elseOpInd */, 3 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=10 */ psexi1kHz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 3 /* checkOffset */, 1 /* checkNum */, 4 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=11 */ psexi1kHz /* inputID */, 85 /* threshold */, 12 /* elseOpInd */, 4 /* checkOffset */, 1 /* checkNum */, 6 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=12 */ psexi1kHz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 4 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=13 */ psexi10kHz /* inputID */, 85 /* threshold */, 14 /* elseOpInd */, 5 /* checkOffset */, 1 /* checkNum */, 0 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=14 */ psexi10kHz /* inputID */, 22 /* threshold */, 15 /* elseOpInd */, 5 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 3 /* actionNum */ },
    { /* operation id=15 */ psexi10kHz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 5 /* checkOffset */, 1 /* checkNum */, 4 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=16 */ psexi10kHz /* inputID */, 85 /* threshold */, 17 /* elseOpInd */, 6 /* checkOffset */, 1 /* checkNum */, 5 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=17 */ psexi10kHz /* inputID */, 11 /* threshold */, 18 /* elseOpInd */, 6 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 2 /* actionNum */ },
    { /* operation id=18 */ psexi10kHz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 6 /* checkOffset */, 1 /* checkNum */, 4 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=19 */ psexi10kHz /* inputID */, 85 /* threshold */, 20 /* elseOpInd */, 7 /* checkOffset */, 1 /* checkNum */, 6 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=20 */ psexi10kHz /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 7 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=21 */ psexiMicLevel /* inputID */, 21 /* threshold */, 22 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 6 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=22 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 9 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=7 */ psexi300Hz /* inputID */, 2 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 2 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=12 */ psexi1kHz /* inputID */, 3 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 4 /* checkOffset */, 1 /* checkNum */, 1 /* actionOffset */, 1 /* actionNum */ },

        /* psRave */
    { /* operation id=0 */ psexi300Hz /* inputID */, 1 /* threshold */, SIM_PS_EX_INVALID/* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 0 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=1 */ psexi1kHz /* inputID */, 1 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 4 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=2 */ psexi10kHz /* inputID */, 33 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 9 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=3 */ psexiMicLevel /* inputID */, 0 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 13 /* actionOffset */, 5 /* actionNum */ },
    { /* operation id=4 */ psexiDC /* inputID */, 80 /* threshold */, SIM_PS_EX_INVALID /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 19 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=5 */ psexiMicLevel /* inputID */, 20 /* threshold */, 3 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 19 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=6 */ psexiMicLevel /* inputID */, 5 /* threshold */, 3 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 18 /* actionOffset */, 1 /* actionNum */ },
    { /* operation id=7 */ psexiMicLevel /* inputID */, 45 /* threshold */, 6 /* elseOpInd */, 0 /* checkOffset */, 0 /* checkNum */, 20 /* actionOffset */, 1 /* actionNum */ }
};

/* 6.3 Operation checking */
/**********************************************************************************************
* Array name:  patExCheck
* Description: Pattern external input operation checkings
* Remarks:     Indexed from effect operation record
*
**********************************************************************************************/
LOCAL CONST SIM_CHECK_T patExCheck[] =
{

    /* psFireworkRaiseUp */

    /* psFireworkExplode */

    /* psTraffic */
    { /* check id=0 */  0x0500 /* (0,5) value */, pVelocity /* pID */, sctEqual /* checkType */ },
    { /* check id=1 */  0x0005 /* (5,0) value */, pVelocity /* pID */, sctEqual /* checkType */ },
    { /* check id=2 */  0xFB00 /* (0,-5) value */, pVelocity /* pID */, sctEqual /* checkType */ },
    { /* check id=3 */  0x00FB /* (-5,0) value */, pVelocity /* pID */, sctEqual /* checkType */ },

    /* psExplosion */

    /* psMeteor */

    /* psMeteorShower */

    /* psWaveHighPitch */

    /* psWaveBass */

    /* psWaveBasic2 */

    /* psWaveBasic1 */

    /* psFirefly */
    { /* check id=0 */ 0 /* (0) value */, pLocationY /* pID */, sctLess /* checkType */ },
    { /* check id=1 */ 0 /* (0) value */, pLocationY /* pID */, sctGreater /* checkType */ },

    /* psCustomized */

    /* psCustomizedBounce */

    /* psLightning */

    /* psFire */
    { /* check id=0 */ -20 /* (-20) value */, pLocationY /* pID */, sctGreater /* checkType */ },
    { /* check id=1 */ 0 /* (0) value */, pLocationX /* pID */, sctLess /* checkType */ },

    /* psRainbow */
    { /* check id=0 */ 3 /* (3) value */, pAAmplitude /* pID */, sctLess /* checkType */ },
    { /* check id=1 */ 2 /* (2) value */, pAAmplitude /* pID */, sctGreater /* checkType */ },
    { /* check id=2 */ 6 /* (6) value */, pAAmplitude /* pID */, sctLess /* checkType */ },
    { /* check id=3 */ 5 /* (5) value */, pAAmplitude /* pID */, sctGreater /* checkType */ },
    { /* check id=4 */ 9 /* (9) value */, pAAmplitude /* pID */, sctLess /* checkType */ },

    /* psHourglass */

    /* psHourglassStack */

    /* psHourglassStackBlink */

    /* psBar */
    { /* check id=0 */ 1 /* (1) value */, pAngle /* pID */, sctEqual /* checkType */ },
    { /* check id=1 */ 2 /* (2) value */, pAngle /* pID */, sctEqual /* checkType */ },
    { /* check id=2 */ 3 /* (3) value */, pAngle /* pID */, sctEqual /* checkType */ },
    { /* check id=3 */ 4 /* (4) value */, pAngle /* pID */, sctEqual /* checkType */ },
    { /* check id=4 */ 5 /* (5) value */, pAngle /* pID */, sctEqual /* checkType */ },
    { /* check id=5 */ 6 /* (6) value */, pAngle /* pID */, sctEqual /* checkType */ },
    { /* check id=6 */ 7 /* (7) value */, pAngle /* pID */, sctEqual /* checkType */ },
    { /* check id=7 */ 8 /* (8) value */, pAngle /* pID */, sctEqual /* checkType */ }
};

/* 6.4 Operation action */
/**********************************************************************************************
* Array name:  patExAction
* Description: Pattern external input operation actions
* Remarks:     Indexed from effect operation record
*
**********************************************************************************************/
LOCAL CONST SIM_ACTION_T patExAction[] =
{

    /* psFireworkRaiseUp */
    { /* action id=0 */ 4 /* (4) value */, satDivValProLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */  0xC8C8 /* (-56,-56) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */  0x0848 /* (72,8) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 2 /* (2) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=6 */  0x4848 /* (72,72) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */  0x18C8 /* (-56,24) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 5 /* (5) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */ 25 /* (25) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=10 */  0xC848 /* (72,-56) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=11 */  0xC8C8 /* (-56,-56) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=12 */ 3 /* (3) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=13 */ 0 /* (0) value */, satFireworkApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=14 */ 0 /* (0) value */, satFireworkShake /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psFireworkExplode */
    { /* action id=15 */ 100 /* (100) value */, satAbsValAbsLimit /* actionType */, pSize /* pID */, 3 /* limit */, 0 /* dummy */ },

    /* psTraffic */
    { /* action id=0 */ 4 /* (4) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */  0x0023 /* (35,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */  0x0000 /* (0,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */  0x0027 /* (39,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */  0x0024 /* (36,) value */, satAbsValAbsLimit /* actionType */, psbVelMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */ 6 /* (6) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=6 */  0x0000 /* (0,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */  0x0008 /* (8,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */  0x0024 /* (36,) value */, satAbsValAbsLimit /* actionType */, psbVelMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */  0x0024 /* (36,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=10 */ 33 /* (33) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=11 */  0x0026 /* (38,) value */, satAbsValAbsLimit /* actionType */, psbVelMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=12 */  0x0027 /* (39,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=13 */  0x0009 /* (9,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=14 */  0x0019 /* (25,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=15 */ 33 /* (33) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=16 */  0x0025 /* (37,) value */, satAbsValAbsLimit /* actionType */, psbVelMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=17 */  0x0025 /* (37,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=18 */  0x001A /* (26,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=19 */  0x0023 /* (35,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=20 */  0xFB00 /* (0,-5) value */, satAbsValAbsLimit /* actionType */, pVelocity /* pID */, 8 /* limit */, 0 /* dummy */ },
    { /* action id=21 */ 5 /* (5) value */, satAbsValAbsLimit /* actionType */, pEffectCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=22 */ 0 /* (0) value */, satRelValAbsLimit /* actionType */, pLifespan /* pID */, 8 /* limit */, 0 /* dummy */ },
    { /* action id=23 */  0x0500 /* (0,5) value */, satAbsValAbsLimit /* actionType */, pVelocity /* pID */, 8 /* limit */, 0 /* dummy */ },
    { /* action id=24 */  0x0005 /* (5,0) value */, satAbsValAbsLimit /* actionType */, pVelocity /* pID */, 8 /* limit */, 0 /* dummy */ },
    { /* action id=25 */ 0 /* (0) value */, satRelValAbsLimit /* actionType */, pLifespan /* pID */, 8 /* limit */, 0 /* dummy */ },
    { /* action id=26 */ 5 /* (5) value */, satAbsValAbsLimit /* actionType */, pEffectCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=27 */  0x00FB /* (-5,0) value */, satAbsValAbsLimit /* actionType */, pVelocity /* pID */, 8 /* limit */, 0 /* dummy */ },
    { /* action id=28 */ 2 /* (2) value */, satAbsValAbsLimit /* actionType */, psbTimeMul /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=29 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbTimeMul /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=30 */ 0 /* (0) value */, satTrafficApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psExplosion */
    { /* action id=0 */  0x0006 /* (6,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */  0x0008 /* (8,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */ 10 /* (10) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 10 /* (10) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */ 33 /* (33) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */  0x0003 /* (3,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=6 */  0x0005 /* (5,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */ 20 /* (20) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 20 /* (20) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */ 30 /* (30) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=10 */ 30 /* (30) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=11 */  0x0002 /* (2,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=12 */ 33 /* (33) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=13 */  0x0000 /* (0,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=14 */  0x0008 /* (8,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=15 */ 50 /* (50) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=16 */ 59 /* (59) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=17 */ 3 /* (3) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=18 */ 5 /* (5) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=19 */ 90 /* (90) value */, satAbsValAbsLimit /* actionType */, pSize /* pID */, 6 /* limit */, 0 /* dummy */ },
    { /* action id=20 */ 9 /* (9) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psMeteor */
    { /* action id=21 */ 0 /* (0) value */, satStarApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=22 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbCreateRate /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=23 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=24 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psbCreateRate /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psMeteorShower */
    { /* action id=25 */ 0 /* (0) value */, satMeteorShowerShake /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psWaveHighPitch */
    { /* action id=0 */ 3 /* (3) value */, satAbsValAbsLimit /* actionType */, psbTimeMul /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbTimeMul /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psWaveBass */

    /* psWaveBasic2 */

    /* psWaveBasic1 */
    { /* action id=2 */ 0 /* (0) value */, satWaveApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 0 /* (0) value */, satWaveShake /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psFirefly */
    { /* action id=0 */ 2 /* (2) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */ 3 /* (3) value */, satAbsValAbsLimit /* actionType */, psbTimeMul /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */  0x0003 /* (3,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbTimeMul /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */  0x0007 /* (7,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */ 31 /* (31) value */, satAbsValAbsLimit /* actionType */, psbCreateRate /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=6 */ 150 /* (150) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbCreateRate /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 18 /* (18) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */ 0 /* (0) value */, satFireflyApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=10 */ 3 /* (3) value */, satAbsValAbsLimit /* actionType */, pVelocityY /* pID */, 15 /* limit */, 0 /* dummy */ },
    { /* action id=11 */  0x0003 /* (3,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=12 */ -3 /* (-3) value */, satAbsValAbsLimit /* actionType */, pVelocityY /* pID */, 15 /* limit */, 0 /* dummy */ },
    { /* action id=13 */ 1 /* (1) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=14 */ 15 /* (15) value */, satDivValAbsLimit /* actionType */, psbCreateRate /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psCustomized */
    { /* action id=0 */ 0 /* (0) value */, setCustomizedApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */ 10 /* (10) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */ 15 /* (15) value */, satRelValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 15 /* (15) value */, satAbsValAbsLimit /* actionType */, psiColorMapInd /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */ 10 /* (10) value */, satAbsValAbsLimit /* actionType */, psiColorMapLen /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */ 15 /* (15) value */, satAbsValAbsLimit /* actionType */, psiSeedColorInd /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=6 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psiColorMapInd /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */ 5 /* (5) value */, satAbsValAbsLimit /* actionType */, psiColorMapLen /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psiSeedColorInd /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */ 15 /* (15) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psCustomizedBounce */

    /* psLightning */
    { /* action id=10 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=11 */ 32 /* (32) value */, satAbsValAbsLimit /* actionType */, psbSizeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=12 */ 64 /* (64) value */, satAbsValAbsLimit /* actionType */, psbSizeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=13 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=14 */ 96 /* (96) value */, satAbsValAbsLimit /* actionType */, psbSizeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=15 */ 128 /* (128) value */, satAbsValAbsLimit /* actionType */, psbSizeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=16 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psFire */
    { /* action id=0 */ 9 /* (9) value */, satRelValProLimit /* actionType */, pLocationX /* pID */, 10 /* limit */, 0 /* dummy */ },
    { /* action id=1 */ 0 /* (0) value */, satAbsValProLimit /* actionType */, pLifespan /* pID */, 8 /* limit */, 0 /* dummy */ },
    { /* action id=2 */  0x0010 /* (16,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */  0x0018 /* (24,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */  0x0024 /* (36,) value */, satAbsValAbsLimit /* actionType */, psbVelMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */  0x0027 /* (39,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=6 */  0x0000 /* (0,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */  0x0014 /* (20,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */  0x0023 /* (35,) value */, satAbsValAbsLimit /* actionType */, psbVelMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */  0x0023 /* (35,) value */, satAbsValAbsLimit /* actionType */, psbVelMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=10 */ 30 /* (30) value */, satAbsValAbsLimit /* actionType */, psbCreateRate /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=11 */ 70 /* (70) value */, satAbsValAbsLimit /* actionType */, psbEffectCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=12 */ 40 /* (40) value */, satAbsValAbsLimit /* actionType */, psbEffectCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=13 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psbCreateRate /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=14 */ 0 /* (0) value */, satFireApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psRainbow */
    { /* action id=0 */ 0 /* (0) value */, satCanvasColorPick /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */ 0 /* (0) value */, satCanvasApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */ 4 /* (4) value */, satAbsValAbsLimit /* actionType */, pAVelocity /* pID */, 3 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 20 /* (20) value */, satAbsValAbsLimit /* actionType */, pEffectCnt /* pID */, 3 /* limit */, 0 /* dummy */ },
    { /* action id=4 */ 6 /* (6) value */, satAbsValAbsLimit /* actionType */, pAVelocity /* pID */, 3 /* limit */, 0 /* dummy */ },
    { /* action id=5 */ 20 /* (20) value */, satAbsValAbsLimit /* actionType */, pEffectCnt /* pID */, 3 /* limit */, 0 /* dummy */ },
    { /* action id=6 */ 8 /* (8) value */, satAbsValAbsLimit /* actionType */, pAVelocity /* pID */, 3 /* limit */, 0 /* dummy */ },
    { /* action id=7 */ 90 /* (90) value */, satAbsValAbsLimit /* actionType */, pAngle /* pID */, 3 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, pAVelocity /* pID */, 3 /* limit */, 0 /* dummy */ },
    { /* action id=9 */ 4 /* (4) value */, satAbsValAbsLimit /* actionType */, pAVelocity /* pID */, 9 /* limit */, 0 /* dummy */ },
    { /* action id=10 */ 20 /* (20) value */, satAbsValAbsLimit /* actionType */, pEffectCnt /* pID */, 9 /* limit */, 0 /* dummy */ },
    { /* action id=11 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, pAVelocity /* pID */, 9 /* limit */, 0 /* dummy */ },
    { /* action id=12 */ 90 /* (90) value */, satAbsValAbsLimit /* actionType */, pAngle /* pID */, 9 /* limit */, 0 /* dummy */ },
    { /* action id=13 */ 0 /* (0) value */, satCanvasShake /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psHourglass */
    { /* action id=0 */ 176 /* (176) value */, satAbsValAbsLimit /* actionType */, psbSizeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */ 176 /* (176) value */, satAbsValAbsLimit /* actionType */, psbSizeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */ 80 /* (80) value */, satAbsValAbsLimit /* actionType */, psbSizeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 80 /* (80) value */, satAbsValAbsLimit /* actionType */, psbSizeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psbSizeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, psbSizeMax /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psHourglassStack */
    { /* action id=6 */ 0 /* (0) value */, satHourglassApp /* actionType */, psbId /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, pLifespan /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 1 /* (1) value */, satAbsValAbsLimit /* actionType */, pState /* pID */, 1 /* limit */, 0 /* dummy */ },

    /* psHourglassStackBlink */

    /* psBar */
    { /* action id=0 */ 176 /* (176) value */, satAbsValAbsLimit /* actionType */, pSize /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */ 3 /* (3) value */, satProValAbsLimit /* actionType */, pSize /* pID */, 11 /* limit */, 0 /* dummy */ },
    { /* action id=2 */ -32 /* (-32) value */, satRelValAbsLimit /* actionType */, pSize /* pID */, 11 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ -32 /* (-32) value */, satRelValAbsLimit /* actionType */, pSize /* pID */, 11 /* limit */, 0 /* dummy */ },
    { /* action id=4 */ 0 /* (0) value */, satAbsValAbsLimit /* actionType */, pSize /* pID */, 11 /* limit */, 0 /* dummy */ },
    { /* action id=5 */ 208 /* (208) value */, satAbsValAbsLimit /* actionType */, pSize /* pID */, 11 /* limit */, 0 /* dummy */ },
    { /* action id=6 */ 240 /* (240) value */, satAbsValAbsLimit /* actionType */, pSize /* pID */, 11 /* limit */, 0 /* dummy */ },
    { /* action id=7 */ 58 /* (58) value */, satAbsValAbsLimit /* actionType */, psiBackGround /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 26 /* (26) value */, satAbsValAbsLimit /* actionType */, psiBackGround /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */ 12 /* (12) value */, satProValAbsLimit /* actionType */, pSize /* pID */, 11 /* limit */, 0 /* dummy */ },
        
    /* psRave */
    { /* action id=0 */  0x0006 /* (6,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=1 */  0x0008 /* (8,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=2 */ 52 /* (10) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=3 */ 52 /* (10) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=4 */ 33 /* (33) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=5 */  0x0003 /* (3,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=6 */  0x0005 /* (5,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=7 */ 52 /* (20) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=8 */ 52 /* (20) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=9 */ 52 /* (30) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=10 */ 52 /* (30) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=11 */  0x0002 /* (2,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=12 */ 33 /* (33) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=13 */  0x0000 /* (0,) value */, satAbsValAbsLimit /* actionType */, psbLocMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=14 */  0x0008 /* (8,) value */, satAbsValAbsLimit /* actionType */, psbLocMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=15 */ 52 /* (50) value */, satAbsValAbsLimit /* actionType */, psbLifeMin /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=16 */ 52 /* (59) value */, satAbsValAbsLimit /* actionType */, psbLifeMax /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=17 */ 3 /* (3) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=18 */ 5 /* (5) value */, satDivValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
    { /* action id=19 */ 90 /* (90) value */, satAbsValAbsLimit /* actionType */, pSize /* pID */, 6 /* limit */, 0 /* dummy */ },
    { /* action id=20 */ 9 /* (9) value */, satAbsValAbsLimit /* actionType */, psbMaxCnt /* pID */, 1 /* limit */, 0 /* dummy */ },
};


LOCAL UINT8  simPattern;

LOCAL UINT8  simDelay;
LOCAL UINT16 simEffectCnt;

/* External effect run-time */
LOCAL UINT8  simRTDelay;
LOCAL INT8   simTimeMult[ SIM_PS_NUM ];
LOCAL UINT8  simExEffectInd[ SIM_PS_NUM ];      /* External effect index, one external effect is processed every time tick */
LOCAL UINT16 simExEnableFlag;                   /* Local external input enable flag */

LOCAL GEN_PARA_T simGridData[ SIM_PS_GRID_DATA_NUM ];
LOCAL UINT8      simGridRandomList[ SIM_PS_GRID_DATA_NUM ];

LOCAL SIM_EX_EFFECT_T simExEffect[ SIM_PS_EX_EFFECT_NUM ];
LOCAL SIM_OPERATION_T simExOp[ SIM_PS_EX_OP_NUM ];
LOCAL SIM_CHECK_T     simExCheck[ SIM_PS_EX_CHECK_NUM ];
LOCAL SIM_ACTION_T    simExAction[ SIM_PS_EX_ACTION_NUM ];

LOCAL UINT8 simExActionCnt[ SIM_PS_EX_ACTION_NUM ];
LOCAL GEN_PARA_T randomLoc[ RANDOM_HISTORY ];

LOCAL UINT8 simITarget[ GM_POS_MAX ];           /* Individual target array, specific for Firefly */


/**********************************************************************************************
 Function:    SimulatorInitialize
 Description: Initialize the simulator
 Input:       Nil
 Return:      VOID
 Remarks:
**********************************************************************************************/
VOID SimulatorInitialize(VOID)
{
    simPattern   = PatIdle;       /* Default to PatIdle first */

    simDelay     = 0;
    
    ThemeColorMode = GMThemeColorMode;
    ThemeColor     = GMThemeColor;

    systemCleanUp();
}

/************************************************************************************************
 Function:    Simulator
 Description: DISEngine Simulator module entry point
 Input:       Nil
 Return:      VOID
 Remarks:     Call from regular timer tick
*************************************************************************************************/
VOID Simulator(VOID)
{
    /* Step 1 -- Initialize/load particle system data if necessary */
    if( dataInitializer() )
    {
        /* A pattern is ready, run it */
        /* Step 2 -- Manage particles life and dead */
        emitter();

        /* Step 3 -- Calculate the time step changes of each active particles */
        physicsEngine();
    }
}

/***********************************************************************************************
 Function:    SimRandom
 Description: Generate a random number within the range specified
 Input:       max: upper limit of random number range
              min: lower limit of random number range
 Return:      A random number
 Remarks:
***********************************************************************************************/
UINT16 SimRandom(UINT16 min, UINT16 max)
{
	uint16_t random_num;
	
    if( max == 0 )
        return 0;
    
    if( max == min )
        return min;

    random_num = ADC_Random_Disaffinity(min, max);
	
    //printf("random_num = %d \r\n", random_num);

    return random_num;
}

/*******************************************************************************************
 Function:    SimSqrt(UINT16 x2)
 Description: Get the x2's square root
 Input:       x2: the number to be sqrt
 Return:      square root
 Remarks:     Suppose x = 2 * p + q,so x^2 = 4 * p^2 +4 * q * p + q^2.
              And so x^2 - 4 * p^2 = (4 * p + q) * q,And p only
              is 0 or 1 in binary system.When we know p and x^2,
              we will get x ,if we get q.  Everytime adding 2 bits after caculation
*********************************************************************************************/
UINT8 SimSqrt(UINT16 x2)
{
    UINT16 divisor, root, rem, i;

    if( x2 < 2 )
        return ( UINT8 )x2;

    /* x^2 - 4 * p^2 */
    rem = 0;

    /* x, */
    root = 0;

    /* 4 * p + q */
    divisor = 0;

    /* 8 = 16 / 2;every comparision x2 shift to right 2 bits */
    for( i = 0; i < 8; i++ )
    {
        /* after caculating remainder,root * 2,because in binary system */
        root <<= 1;

        /* x^2 - 4 * p^2 */
        rem = ( ( rem << 2 ) + ( x2 >> 14 ) );

        /* when root add 1 bit,x2 delete 2 bits */
        x2 <<= 2;

        /* 4 * p + q , here it just shift to left 1 bit because it has shifted to left 1 bit upside */
        divisor = ( root << 1 ) + 1;

        if( divisor <= rem )
        {
            rem -= divisor;
            root++;
        }
    }

    return ( UINT8 )( root );
}

/**************************************************************************************************
 Function:     SimMax
 Description:  Calculate Max of two short signed INT's
 Input:           X1, X2 Int nos to be compared
 Return:       the Largest of the two Int
 Remarks:
***************************************************************************************************/
INT16 SimMax(INT16 x1, INT16 x2)
{
    return (x1 >= x2 ? x1: x2);
}

/**************************************************************************************************
 Function:     SimSin
 Description:  Calculate sine of an angle in degree
 Input:           angle : angle to caculate the sine
 Return:       the angle's sine
 Remarks:
***************************************************************************************************/
INT8 SimSin(INT16 angle)
{
    INT16 multiple, weight, i;

    weight = 1;
    /* sine function is uneven function */
    if( angle < 0 )
    {
        weight = -1;
        angle  = -angle;
    }

    multiple = angle / 90;
    angle   -= multiple * 90;
    multiple %= 4;

    /* angle at 2nd or 4th quadrant, multiple = 1 or 3 */
    if( multiple % 2 )      // All need 6 ASM instructions
        angle = 90 - angle; // All need 11 ASM instructions ( all if )

    /* angle at 3rd or 4th quadrant, multiple = 2 or 3  */
    if( multiple >> 1 )    // It needs 4 ASM instructions 
        weight = -weight;  // All need 8 ASM instructions ( all if ),

    i = ( angle / ANGLE_INTERVAL ) + ( ( angle % ANGLE_INTERVAL ) / ( ANGLE_INTERVAL >> 1 ) );
    
    return ( sine[ i ] * weight );
}

/***********************************************************************************************
 Function:    SimDistanceSq
 Description: Calculate the distance between two input locations
 Input:       loc1: first location
              loc2: second location
 Return:      Distance square
 Remarks:
************************************************************************************************/
UINT16 inline SimDistanceSq(PVECTOR_ST loc1, PVECTOR_ST loc2)
{
    INT16  dx, dy;

    dx = loc1.x - loc2.x;
    dy = loc1.y - loc2.y;

    return ( UINT16 )( dx * dx + dy * dy );
}

/**************************************************************************************************
 Function:     SimVectorMag2
 Description:  Calculate the magnitude squre of a vector
 Input:           v1: input vector
 Return:       Magnitude square of the vector
 Remarks:
***************************************************************************************************/
UINT16 inline SimVectorMag2(PVECTOR_ST v1)
{
    INT16 x, y;

    x = v1.x;
    y = v1.y;

    return ( UINT16 )( x * x + y * y );
}

///**************************************************************************************************
// Function:     PauseHourglass
// Description:  Pause the running hourglass and show the pause system
// Input:           VOID
// Return:       VOID
// Remarks:      Special treatment for Hourglass pattern, called from mapPhyDis() in Renderer
//***************************************************************************************************/
//VOID PauseHourglass(VOID)
//{
//    UINT16 i;
//
//    /* Step 1 -- Locate the hourglass stack */
//    for( i = 0; i < SimPMax; i++ )
//    {
//        if( SimP[ i ].systemID == psHourglassStack )
//            break;
//    }
//
//    /* Step 2 -- Set stack size to pause hourglass running */
//    SimP[ i ].size = 0;
//    SimPS[ 3 ].psBase.effectCnt = SimP[ i ].lifespan;       /* Use hourglassPause system effectCnt as buffer for stack life */
//    SimP[ i ].lifespan = SIM_PARTICLE_INFINITE;             /* Set to infinite to avoid being killed */
//
//    /* Step 3 -- Show pause system */
//    SimPS[ 3 ].psBase.psFlag |= SIM_PSB_SHOW;
//}

/**************************************************************************************************
 Function:     RestartHourglass
 Description:  Restart the hourglass and hide the pause system
 Input:           VOID
 Return:       VOID
 Remarks:      Special treatment for Hourglass pattern, called from mapPhyDis() in Renderer
***************************************************************************************************/
VOID RestartHourglass(VOID)
{
    UINT16 i;

    /* Step 1 -- Locate the hourglass stack */
    for( i = 0; i < SimPMax; i++ )
    {
        if( SimP[ i ].systemID == psHourglassStack )
            break;
    }

    /* Step 2 -- Kill the running stack */
    SimP[ i ].lifespan = 0;
    SimP[ i ].state    = 1;

    /* Step 3 -- Locate the hourglass */
    for( i = 0; i < SimPMax; i++ )
    {
        if( SimP[ i ].systemID == psHourglass )
            break;
    }

    /* Step 3 -- Kill the running sand drop */
    SimP[ i ].lifespan = 0;

    /* Step 4 -- Hide pause system */
    SimPS[ 3 ].psBase.psFlag &= ~SIM_PSB_SHOW;
    
}

///**************************************************************************************************
// Function:     ResumeHourglass
// Description:  Resume the running hourglass and hide the pause system
// Input:           VOID
// Return:       VOID
// Remarks:      Special treatment for Hourglass pattern, called from mapPhyDis() in Renderer
//***************************************************************************************************/
//VOID ResumeHourglass(VOID)
//{
//    UINT16 i;
//
//    /* Step 1 -- Locate the hourglass stack */
//    for( i = 0; i < SimPMax; i++ )
//    {
//        if( SimP[ i ].systemID == psHourglassStack )
//            break;
//    }
//
//    /* Step 2 -- Set stack size to pause hourglass running */
//    SimP[ i ].size = 1;                                 /* An arbitrary size > 0 */
//    SimP[ i ].lifespan = SimPS[ 3 ].psBase.effectCnt;   /* Recall lifespan */
//
//    /* Step 3 -- Hide pause system */
//    SimPS[ 3 ].psBase.psFlag &= ~SIM_PSB_SHOW;
//}

/************************************************************************************************
 Function:    systemCleanUp
 Description: Clear all run-time system arrays
 Input:          Nil
 Return:      VOID
 Remarks:
*************************************************************************************************/
LOCAL VOID systemCleanUp(VOID)
{
    UINT16 i;

    for( i = 0; i < SIM_PS_NUM; i++ )
    {
        SimPS[ i ].psBase.id = psInvalid;
        simExEffectInd[ i ]  = SIM_PS_EX_INVALID;
    }

    for( i = 0; i < SIM_PARTICLE_NUM; i++ )
        SimP[ i ].systemID = SIM_PARTICLE_INVALID;

    for( i = 0; i < SIM_PS_EX_EFFECT_NUM; i++ )
        simExEffect[ i ].decisionType = sdtNull;

    simRTDelay = 0;
    
    for( i = 0; i < RANDOM_HISTORY; i++ )
        randomLoc[ i ].scalar = INVALID_LOC;

    for( i = 0; i < SIM_PS_GRID_DATA_NUM; i++ )
    {
        simGridData[ i ].scalar = INVALID_LOC;
        simGridRandomList[ i ]  = INVALID_GD_NUM;
    }

    simEffectCnt = 0;
}

/************************************************************************************************
 Function:    dataInitializer
 Description: Check Simulator State if it is running a pattern.  If not, load a pattern.
              Also call Renderer to load the pattern data.
 Input:          Nil
 Return:      TRUE:  A pattern is ready and need to trigger emitter and physicsEngine
              FALSE: Pattern is idle or in delay count down, skip all in Simulator
 Remarks:
*************************************************************************************************/
LOCAL BOOL dataInitializer(VOID)
{
    PATTERN_INFO_T pInfo;

    //update theme color and color mode
    UpdateThemeColorStatus();
    
    /* Step 1 -- Check pattern broadcast status */
    pInfo = GMPatternInfoRx;
    if( pInfo.id != PatIdle )
    {
        /* Step 1.1 -- A broadcast pattern is received, load it */
        simPattern = pInfo.id;
        
        loadPattern( pInfo.id );
        
        /* Step 1.2 -- Set the pattern characteristics */
        if( pInfo.status != GM_PATTERN_STATUS_NULL )
        {
            switch( pInfo.id )
            {
                case PatFire:
                    setFireLevel( 0, pInfo.status );
                    break;

                case PatHourglass:
                    setHourglassParticle( 1, pInfo.status * 60 );      /* At particle system 1; convert to seconds */
                    break;
            }
        }
        GMPatternInfoTx.status = pInfo.status;          /* Set the pattern status back to broadcast info */
        
        /* Step 1.3 -- Set color */
        if( ( pInfo.pColor[ 0 ].r != 0 ) || ( pInfo.pColor[ 0 ].g != 0 ) || ( pInfo.pColor[ 0 ].b != 0 ) )
        {
            /* It is not at default color, set the prism colors */
            SetPatternBroadcastColor( &pInfo );
        }
        
        /* Step 1.4 -- Set back to GMPattern and reset GMPatternInfoRx */
        GMPattern = pattern_flag;//pInfo.id;
        GMPatternInfoRx.id = PatIdle;
        
        return TRUE;
    }

    /* Step 2 -- Return immediately if at idle state */
    if( ( GMPattern & GM_PATTERN_IDLE_MASK ) == PatIdle )
    {
        /* Pattern is idle, no need to load a new pattern */
        simPattern = PatIdle;

        return FALSE;
    }

    /* Step 3 -- Check if new pattern */
    if( ( simPattern != GMPattern || TRUE == GMPatternReload ) && ( GMPattern <= PatSimMax ) )
    {
        /* Pattern status has been changed or wanna load, load the new one */
        simPattern = GMPattern;

        loadPattern( simPattern );
        
        GMPatternReload = FALSE;
    }
    else
    {
        /* Update simulation conditions */
        if( simDelay == 0 )
        {
            if( simEffectCnt == 0 )
            {
                /* No more delay, trigger Simulator execution */
                updateConditions();
            }
            else
            {
                /* EffectCnt in control, no more external input for the moment */
                --simEffectCnt;
            }

            simDelay = simRTDelay;      /* Reload delay count */
        }
        else
        {
            /* Still in delay count down, no Simulator execution */
            --simDelay;
            return FALSE;
        }
    }

    return TRUE;
}

/************************************************************************************************
 Function:    loadPattern
 Description: Load a pattern data set
  Input:       id: pattern ID
 Return:      VOID
 Remarks:
*************************************************************************************************/
LOCAL VOID loadPattern(UINT16 id)
{
    UINT16  i, j, count;
    UINT16  num, ind;

    /* Step 1 -- Clean up Simulator */
    systemCleanUp();

    /* Step 2 -- Read pattern info */
    simRTDelay = patMeta[ id ].delayCnt;
    SimPMax    = patMeta[ id ].pMax;

    /* Step 3 -- Read meta data and then initialize particle system one by one */
    num = patMeta[ id ].psNum;
    ind = patMeta[ id ].psInd;

    for( i = 0; i < num; i++, ind++ )
    {
        SimPS[ i ].psBase  = patPsb[ ind ];
        SimPS[ i ].psImage = patPsi[ ind ];

        simTimeMult[ i ] = SimPS[ i ].psBase.timeMul;
    }

    /* Step 4 -- Initialize Renderer elements */
    InitRendererDisBuf();
    InitialMapColor( ( COLOR_T * )&( patMeta[ id ].patColor[ 0 ] ), ( PS_COLOR_OP_T *)&( patMeta[ id ].patColorOp[ 0 ] ) );

    /* Copy background and shape, if any */
    for( i = 0; i < num; i++ )
    {
        if( ( SimPS[ i ].psImage.bgFlag & REN_BG_TYPE_MASK ) > bgtBlack )
        {
            /* A background is defined, copy it and update to run-time index */
            SimPS[ i ].psImage.bgFlag = ( SimPS[ i ].psImage.bgFlag & REN_BG_TYPE_MASK ) |
                                        SetBackground( ( VIRDIS_T* )&patBgi[ SimPS[ i ].psImage.bgFlag & REN_BG_IND_MASK ] );
        }

        if( ( SimPS[ i ].psImage.shapeFlag & REN_SHAPE_TYPE_MASK ) >= stImage )
        {
            /* A shape is defined, copy it and update to run-time index */
            j = SimPS[ i ].psImage.shapeFlag & REN_SHAPE_IND_MASK;
            SimPS[ i ].psImage.shapeFlag = ( SimPS[ i ].psImage.shapeFlag & REN_SHAPE_TYPE_MASK ) |
                                           SetShape( ( SHAPE_T * )&patSi[ j ] );

            count = HINIBBLE( ( SimPS[ i ].psImage.shapeFlag & REN_SHAPE_TYPE_MASK ) - stImage );
            while( count > 0 )
            {
                SetShape( ( SHAPE_T * )&patSi[ ++j ] );
                count--;
            }

            SimPS[ i ].psImage.shapeFlag = stImage | ( SimPS[ i ].psImage.shapeFlag & REN_SHAPE_IND_MASK );
        }
    }

    /* Step 5 -- Read grid data array */
    for( i = 0; i < SIM_PS_GRID_DATA_NUM; i++ )
        simGridData[ i ] = patMeta[ id ].patGrid[ i ];

    /* Step 6 -- External effect loading */
    count = 0;
    for( i = 0; i < num; i++ )
    {
        SimPS[ i ].exNum = patMeta[ id ].exNum[ i ];
        SimPS[ i ].exOffset = count;

        if( SimPS[ i ].exNum == 0 )
            simExEffectInd[ i ] = SIM_PS_EX_INVALID;

        else
            simExEffectInd[ i ] = SimPS[ i ].exOffset;

        count += patMeta[ id ].exNum[ i ];
    }

    for( i = 0; i < count; i++ )
        simExEffect[ i ] = patExEffect[ patMeta[ id ].exInd + i ];

    for( i = 0; i < patMeta[ id ].exCheckNum; i++ )
        simExCheck[ i ] = patExCheck[ patMeta[ id ].exCheckInd + i ];

    for( i = 0; i < patMeta[ id ].exActionNum; i++ )
        simExAction[ i ] = patExAction[ patMeta[ id ].exActionInd + i ];

    for( i = 0; i < patMeta[ id ].exOpNum; i++ )
        simExOp[ i ] = patExOp[ patMeta[ id ].exOpInd + i ];

    /* Step 7 -- Read in GM pattern characteristics */
    if( GMPatternStatus[ id ] != GM_PATTERN_STATUS_NULL )
    {
        switch( id )
        {
            case PatFire:
                setFireLevel( 0, GMPatternStatus[ id ] );
                break;

            case PatHourglass:
                setHourglassParticle( 1, GMPatternStatus[ id ] * 60 );      /* At particle system 1; convert to seconds */
                break;
        }
    }

    /* Step 8 -- Prepare broadcast info */
    GMPatternInfoTx.id     = id;
    GMPatternInfoTx.status = GMPatternStatus[ id ];
    GMPatternInfoTx.pColor[ 0 ].r = 0;              /* Set to default color */
    GMPatternInfoTx.pColor[ 0 ].g = 0;
    GMPatternInfoTx.pColor[ 0 ].b = 0;

    GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;

    /* Step 9 -- Initialize pattern immediately for special type */
   // EnableGetDBValues(FALSE);
    switch( id )
    {
        case PatRainbow:
           // EnableGetDBValues(TRUE);
            ResetRainBowStat();
            break;

        case PatBar:
            //EnableGetDBValues(TRUE);
            ResetBarStat();
            loadBar( 0 );
            loadFixedLoc( 1, 1 );
            break;

        case PatWave:
            SetWaveThemeColor();
            GMExInput[ psexiApp ].scalar = 0;
            setWaveLevel();
            loadFixedLoc( 0, num );
            break;

        case PatJet:
            SetJetThemeNormalColor();
            loadEQJet();
            break;

        case PatRave:
            loadRave();
            break;

        case PatCustomized:
            LoadCustomizedTheme();
            break;

        case PatFire:
            if(APP_COLOR == ThemeColorMode)
            {
                colorMap[0] = ThemeColor;
            }
            break;
        default:
            break;
    }

    /* Step 10 -- Enable all external inputs */
    simExEnableFlag = GM_EX_INPUT_ENABLE;
}

LOCAL VOID updateExplosionConditions(UINT16 psInd, BOOL bAudio)
{
    PARTICLE_SYSTEM_ST *psPtr= &SimPS[psInd];

    if(TRUE == bAudio)
    {
        if(GMExInput[psexiDC].scalar >= 36)
        {
            psPtr->psBase.maxCnt  = 36;
            psPtr->psBase.timeMul = 9;
            psPtr->psBase.sizeMin = 32;
            psPtr->psBase.sizeMax = 48;
            psPtr->psBase.createRate = 2;
        }
        else if(GMExInput[psexiDC].scalar >= 12)
        {
            psPtr->psBase.maxCnt  = 36;
            psPtr->psBase.timeMul = 9;
            psPtr->psBase.sizeMin = 8;
            psPtr->psBase.sizeMax = 36;
            psPtr->psBase.createRate = 4;
        }
        else
        {
            psPtr->psBase.createRate = 0;
            if(psPtr->psBase.currentCnt <= 1)
            {
                psPtr->psBase.maxCnt = 8;
                psPtr->psBase.timeMul = 2;
                psPtr->psBase.createRate = 4;
                psPtr->psBase.sizeMin = 8;
                psPtr->psBase.sizeMax = 32;
            }
        }
    }
    else
    {
        psPtr->psBase.maxCnt = 8;
        psPtr->psBase.timeMul = 2;
        psPtr->psBase.createRate = 1;
        psPtr->psBase.sizeMin = 8;
        psPtr->psBase.sizeMax = 32;
    }
}

/************************************************************************************************
 Function:    updateConditions
 Description: Update particles and particle system parameters according to external inputs
 Input:       Nil
 Return:      VOID
 Remarks:
*************************************************************************************************/

LOCAL VOID updateConditions(VOID)
{
    UINT16 i, max, psInd, eInd, opdInd, eCnt;
    INT16  dValue, rValue;
    BOOL   nextEFlag;

    nextEFlag = FALSE;
    for( psInd = 0; psInd < SIM_PS_NUM; psInd++ )
    {
        if(SimPS[ psInd ].psBase.id != psInvalid)
        {
            if(psJet == SimPS[ psInd ].psBase.id)
            {
                continue;
            }
            if(psRave== SimPS[ psInd ].psBase.id)
            {
                continue;
            }
            else if(psExplosion == SimPS[ psInd ].psBase.id)
            {
                updateExplosionConditions(psInd, AudioDet);
                continue;
            }
            else if(psCustomized== SimPS[ psInd ].psBase.id)
            {
                continue;
            }
            else if(psFire == SimPS[ psInd ].psBase.id)
            {
                if(GMExInput[psexi300Hz].scalar >= 12)
                {
                    setFireLevel(psInd, 2);
                }
                else
                {
                    setFireLevel(psInd, 1);
                }
            }
            if( ( SimPS[ psInd ].psBase.id != psInvalid ) && ( simExEffectInd[ psInd ] != SIM_PS_EX_INVALID ) )
            {
                /* It is a valid particle system, process its external effect */
                eCnt = SimPS[ psInd ].exNum;
                eInd = simExEffectInd[ psInd ];
                while ( eCnt > 0 )
                {
                    /* Step 1 -- Determine the operation chain from the effect decision */
                    opdInd = simExEffect[ eInd ].opOffset;
                    dValue = GMExInput[ simExOp[ opdInd ].inputID ].scalar;     /* Determine by scalar value only */

                    max = simExEffect[ eInd ].opOffset + simExEffect[ eInd ].opNum;
                    for( i = opdInd + 1; i < max; i++ )
                    {
                        rValue = GMExInput[ simExOp[ i ].inputID ].scalar;
                        switch( simExEffect[ eInd ].decisionType )
                        {
                            case sdtMax:
                                if( ( rValue > dValue ) || ( dValue == GM_EX_INPUT_INVALID ) )
                                {
                                    dValue = rValue;
                                    opdInd = i;
                                }
                                break;

                            case sdtMin:
                                if( ( rValue < dValue ) || ( dValue == GM_EX_INPUT_INVALID ) )
                                {
                                    dValue = rValue;
                                    opdInd = i;
                                }
                                break;

                            case sdtAlways:
                            default:
                                break;
                        }
                    }

                    if( ( dValue == GM_EX_INPUT_INVALID ) ||
                        ( ( simExEnableFlag & ( 1 << simExOp[ opdInd ].inputID ) ) == FALSE ) || ( simExEffect[ eInd ].opNum == 0 ) )
                    {    
                        /* The input is invalid, no need to go for operations */
                        nextEFlag = FALSE;

                    }
                    else
                    {
                        /* Step 2 -- Determine the operation from the operation chain */
                        i = opdInd;
                        while( dValue < simExOp[ i ].threshold )
                        {
                            if( simExOp[ i ].elseOpInd == SIM_PS_EX_INVALID )
                            {
                                /* There is no more else check, the operation failed */
                                break;
                            }
                            else
                            {
                                /* The checking failed, go to else operation */
                                i = simExOp[ i ].elseOpInd;
                            }
                        }

                        if( dValue >= simExOp[ i ].threshold )
                        {
                            /* Step 3 -- The operation is identified, process the checkings associated and take actions if passed */
                            nextEFlag = inputOperate( dValue, i, psInd );

                        if( ( SimPS[ psInd ].psBase.id == psFireworkRaiseUp ) && nextEFlag )
                        {
                            if( ( simExOp[ i ].inputID == psexiShake ) || ( simExOp[ i ].inputID == psexiApp ) )
                            {
                                if( GMExInput[ psexiShake ].scalar == 0 )
                                    GMExInput[ psexiShake ].scalar = GM_EX_INPUT_INVALID;
                                /* In firework, shake and app share the same effect, no need to go through shake treatment below */
                                break;      /* No need to update effect index so that App would be checked again once after effectCnt */
                            }
                        }
                        else if( ( simExOp[ i ].inputID == psexiShake ) && nextEFlag )
                        {
                            /* Shake is already taken action, start effect count if necessary */
                            if( GMExInput[ psexiShake ].scalar >= 1 )
                            {
                                simEffectCnt = SimPS[ psInd ].psBase.effectCnt;

                                    /* Special case treatment Fire pattern */
                                    if( ( GMExInput[ psexiShake ].scalar == 1 ) && ( SimPS[ psInd ].psBase.id == psFire ) )
                                        GMExInput[ psexiShake ].scalar = fire_shake2;
                                    else
                                        GMExInput[ psexiShake ].scalar = 0;            /* Reset shake action items */
                                }
                                else if( GMExInput[ psexiShake ].scalar == 0 )
                                    GMExInput[ psexiShake ].scalar = GM_EX_INPUT_INVALID;   /* Reset shake input */
                                break;  /* No need to update effect index so that shake would be checked again once after effectCnt */
                            }
                            else
                            {                                                
                                if( ( (simExOp[ i ].inputID == psexi300Hz) ||  (simExOp[ i ].inputID == psexi1kHz) || (simExOp[ i ].inputID == psexi10kHz) ) && nextEFlag )
                                {
                                    simEffectCnt = SimPS[ psInd ].psBase.effectCnt;                                                               
                                }                              
                            }
                        }
                    }

                    /* Step 4 -- Advance to the next external effect */
                    eInd++;
                    if( eInd >= ( SimPS[ psInd ].exOffset + SimPS[ psInd ].exNum ) )
                        eInd = SimPS[ psInd ].exOffset;
    //                eInd = ( eInd - SimPS[ psInd ].exOffset + 1 ) % SimPS[ psInd ].exNum + SimPS[ psInd ].exOffset;
    //                break;
    /* @LM: Seems the below optimization may cause some effects executed at the same cycle and missed */
    /* More investigation later on ... */
                    if( nextEFlag )
                    {
                        /* The effect is taken and leave to the next cycle for the next effect */
                        break;
                    }
                    else
                    {
                        /* The effect is not taken due to conditions not matched, check the next one */
                        eCnt--;
                    }
                }
                simExEffectInd[ psInd ] = eInd;
            }
        }
    }
}

/**************************************************************************************************
 Function:    emitter
 Description: According to current particles' number and the state of simulator,
              kill particles or create new particles
 Input:          Nil
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID emitter(VOID)
{
    UINT16 i;

    /* Step 1 -- Kill any out of scope particles */
    killParticles();

    /* Step 2 -- Create new particles if necessary */
    for( i = 0; i < SIM_PS_NUM; i++ )
    {
        if( SimPS[i].psBase.id != psInvalid )
        {
            /* It is a valid particle system, create particles for it */
            createParticles( i, 0 );
        }
    }
}

/************************************************************************************************
 Function:    killParticles
 Description: Kill particles with lifespan = 0 or renew it to enter another state
              if the particle system defines
 Input:       Nil
 Return:      VOID
 Remarks:
*************************************************************************************************/
LOCAL VOID killParticles(VOID)
{
    PARTICLE_ST           *pPtr;
    PARTICLE_SYSTEM_ST *psPtr;
    PVECTOR_ST         nextLoc, target;
    UINT16             dis, dis2, tInd;

    pPtr = &SimP[0];
    while( pPtr < &SimP[ SimPMax ] )    /* Check all particles */
    {
        if( pPtr -> systemID != SIM_PARTICLE_INVALID )
        {
            /* It is an active particle, check it */
            if( pPtr -> grid == SIM_OUT_SCOPE )
            {
                /* The particle is already out of scope, kill it */
                killOneParticle( pPtr );
            }
            else
            {
                psPtr = &SimPS[ pPtr -> layer ];
                if(( pPtr -> lifespan ) == 0 )
                {
                    if( ( pPtr -> state ) >= ( psPtr -> psBase.maxState ) )
                    {
                        /* Check cycle state flag  */
                        if( psPtr -> psBase.psFlag & SIM_CYCLE_STATE )
                            /* The state should enter next cycle  */
                            enterNextState( pPtr );
                        else
                            /* The particle is already reached the end of its life, kill it */
                            killOneParticle( pPtr );
                    }
                    else
                    {
                        /* The particle reaches the end of a life stage, go to the next stage */
                        enterNextState( pPtr );
                    }
                }
                else if( pPtr -> state == ITARGET_SEEK )
                {
                    /* Individual target seeking */
                    /* Get the target index */
                    for( tInd = 0; tInd < GM_POS_MAX * psPtr -> psBase.seqMax; tInd++ )
                    {
                        if( &SimP[ tInd ] == pPtr )
                            break;
                    }

                    tInd /= psPtr -> psBase.seqMax;
                    target.x = ( REN_PHY_FRAME_W - 1 - simITarget[ tInd ] % REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;
                    target.y = ( REN_PHY_FRAME_H - simITarget[ tInd ] / REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;
                    
                    dis = SimDistanceSq( target, pPtr -> location );
                    pVectorAdd( pPtr -> location, pPtr -> velocity, &nextLoc );
                    dis2 = SimDistanceSq( target, nextLoc );
                    if( dis < dis2 )
                    {
                        /* The particle is already the closest to the target, keep it */
                        pPtr -> velocity.x = 0;
                        pPtr -> velocity.y = 0;
                        pPtr -> state = 0;          /* Convert it back to a normal particle */
                    }
    
                }
                else if( psPtr -> psBase.psFlag & SIM_PSB_SEEK_TARGET )
                {
                    /* Also kill the particle when it reaches the target */
                    dis = SimDistanceSq( psPtr -> psBase.target, pPtr -> location );
                    pVectorAdd( pPtr -> location, pPtr -> velocity, &nextLoc );
                    dis2 = SimDistanceSq( psPtr -> psBase.target, nextLoc );
                    if( dis < dis2 )
                    {
                        /* The particle is already the closest to the target, kill it */
                        killOneParticle( pPtr );
                    }
                }
            }
        }
        pPtr++;
    }
}

/************************************************************************************************
 Function:    killOneParticle
 Description: Kill one particle
 Input:       pPtr: pointer to the particle
 Return:      VOID
 Remarks:
*************************************************************************************************/
LOCAL VOID killOneParticle(PARTICLE_ST *pPtr)
{
    PS_BASE_T   *psbPtr;
    PARTICLE_ST *pNextPtr;

    /* Step 1 -- Update sequence no. of any chained up particles */
    psbPtr = &SimPS[ pPtr -> layer ].psBase;

    if( ( psbPtr -> seqMax ) > 1 )
    {
        pNextPtr = pPtr -> nextPPtr;
        if( pNextPtr != ( PARTICLE_ST * )NULL )
        {
            while( pNextPtr -> nextPPtr != ( PARTICLE_ST * )NULL )
            {
                pNextPtr -> seq--;
                pNextPtr = pNextPtr -> nextPPtr;
            }
            pNextPtr -> seq--;
        }
    }

    /* Step 2 -- Return the particle to the pool */
    pPtr -> systemID = SIM_PARTICLE_INVALID;

    /* Step 3 -- Reduce the particle count of its system */
    psbPtr -> currentCnt--;
}

/************************************************************************************************
 Function:    enterNextState
 Description: Particle enter its next state
 Input:       pPtr: pointer to the particle
 Return:      VOID
 Remarks:
************************************************************************************************/
LOCAL VOID enterNextState(PARTICLE_ST  *pPtr)
{
    PS_BASE_T   *psbPtr;
    PARTICLE_ST *pNextPtr;
    UINT8       tempState, tempSize;
    PVECTOR_ST  tempLoc;
    COLOR_T     tempColor;

    psbPtr = &SimPS[ pPtr -> layer ].psBase;

    /* Step 1 -- Format the particle to the next state */
    tempState = pPtr -> state + 1;      /* Temporarily store the state, location, color and size */
    tempLoc   = pPtr -> location;
    tempColor = pPtr -> color;
    tempSize  = pPtr -> size;

    /* Store the next particle addr */
    pNextPtr = pPtr -> nextPPtr;
    
    /* Renew particle with cycled state */
    if( ( SIM_CYCLE_STATE & psbPtr -> psFlag ) == SIM_CYCLE_STATE )
    {
        if( pPtr -> seq == 1 )
        {
            /* Check state and renew particle parameters according to its state */
            if( pPtr -> state == 0 )
            {
                initParticle( pPtr, ( pPtr -> layer + 1 ));
                pPtr -> state = 1;
                tempColor = pPtr -> color;
            }
            else if( pPtr -> state == 1 )
            {
                initParticle( pPtr, ( pPtr -> layer - 1 ));
                pPtr -> state = 0;
                tempColor = pPtr -> color;
            }
        }
    }
    else
    {
        /* Initialize all other parameters */
        initParticle( pPtr, ( pPtr -> layer + 1 ) );
        pPtr -> state = tempState;       /* Restore state, location, color and size */
    }
    /* Step 2 -- Reduce the particle count of its system */
    psbPtr -> currentCnt--;

    if( ( pPtr -> seqEnd ) > 1 )
    {
        /* Renew all subsequent particles */
        pPtr -> nextPPtr = pNextPtr;
        if( pNextPtr != ( PARTICLE_ST * )NULL )
        {
            do
            {
                if( ( SIM_NO_CUT_TAIL & psbPtr -> psFlag ) == SIM_NO_CUT_TAIL )
                {
                    /* Renew systemID and layer for particles in sequence */
                    pNextPtr -> systemID = pPtr -> systemID;
                    pNextPtr -> layer    = pPtr -> layer;
                    pNextPtr -> velocity = pPtr -> velocity;
                    pNextPtr -> color    = pPtr -> color;
                }
                else
                    pNextPtr -> systemID = SIM_PARTICLE_INVALID;  /* Kill it */
                
                /* Reduce the particle count of its system */
                psbPtr -> currentCnt--;
                SimPS[ pPtr -> layer ].psBase.currentCnt++;
                pNextPtr = pNextPtr -> nextPPtr;
            }while( pNextPtr != ( PARTICLE_ST * )NULL );
        }
    }
    
    /* Renew location, color */
    pPtr -> location = tempLoc;
    pPtr -> color    = tempColor;

    /* Renew size if need */
    if( tempSize > 0 )
        pPtr -> size = tempSize;
}

VOID DecreaseNormalJetRow(UINT16 psInd)
{
    PARTICLE_SYSTEM_ST *psPtr= &SimPS[ psInd ];
    psPtr->psBase.cohesion--;
    if(psPtr->psBase.cohesion >= REN_PHY_FRAME_H)
        psPtr->psBase.cohesion = REN_PHY_FRAME_H - 1;
}

VOID CreateNormalJetThemeParticles(UINT16 psInd)
{
    PARTICLE_ST        *pPtr = NULL;
    PARTICLE_SYSTEM_ST *psPtr;
    
    int         nDist;
    int         nScandParticles = 0;
    BOOL        bPrevCenterClean= TRUE;
    BOOL        bCurLineClean   = TRUE;
    
    //for loop index dec
    int nIdx;
    
    GEN_PARA_T  PrevCenterLoc;
    GEN_PARA_T  CurLineEndLoc;
    GEN_PARA_T  Vel;

    psPtr= &SimPS[psInd];
    PrevCenterLoc = simGridData[(psPtr->psBase.cohesion + 1)%REN_PHY_FRAME_H];
    CurLineEndLoc = simGridData[(psPtr->psBase.cohesion)];
    if(psPtr->psBase.cohesion % 2 != 0)
    {
        Vel = psPtr->psBase.velMin;
        CurLineEndLoc.vector.x -= REN_PHY_FRAME_W * REN_FRAME2VIR_FRAME;
    }
    else
    {
        Vel = psPtr->psBase.velMax;
        CurLineEndLoc.vector.x += REN_PHY_FRAME_W * REN_FRAME2VIR_FRAME;
    }

    //scan for free particle and free location
    for(nIdx = 0; nIdx < SimPMax; nIdx ++)
    {
        if( !bPrevCenterClean || !bCurLineClean || (pPtr && nScandParticles >= psPtr->psBase.currentCnt))
            break;//all task finished, break out

        if(NULL == pPtr && SIM_PARTICLE_INVALID == SimP[nIdx].systemID)
        {//find a free particle
            pPtr = &SimP[nIdx];
        }
        else if(SimP[nIdx].systemID == psPtr->psBase.id)
        {
            if(SimP[nIdx].location.y == PrevCenterLoc.vector.y)
            {
                //check if previous center column have particle
                nDist = SimP[nIdx].location.x - PrevCenterLoc.vector.x;
               if( nDist < 0 )
                   nDist = -nDist;
                if(nDist < REN_FRAME2VIR_FRAME)
                {
                    bPrevCenterClean = FALSE;
                }
            }
            else if(SimP[nIdx].location.y == CurLineEndLoc.vector.y )
            {
                if((Vel.vector.x >= 0 && SimP[nIdx].location.x < CurLineEndLoc.vector.x)
                    || (Vel.vector.x < 0 && SimP[nIdx].location.x > CurLineEndLoc.vector.x))
                {
                    bCurLineClean = FALSE;
                }
            }
            nScandParticles ++;
        }
    }

    if(bPrevCenterClean && bCurLineClean && pPtr)
    {
        initParticle( pPtr, psInd );
        pPtr->location = simGridData[ psPtr->psBase.cohesion ].vector;
        pPtr->velocity = Vel.vector;
        pPtr->color    = colorMap[psPtr->psBase.cohesion];
        DecreaseNormalJetRow(psInd);
    }
}

BOOL CheckEQBandData(PS_EX_INPUT_TYPE_E ExInputIdx, int nThreshold)
{
    if(GMExInput[ExInputIdx].scalar != GM_EX_INPUT_INVALID)
        if(GMExInput[ExInputIdx].scalar >= nThreshold)
            return TRUE;
    
    return FALSE;
}

/************************************************************************************************
 Function:    createParticles
 Description: Create new particles if necessary mainly according to the
 Input:          psInd: index of the particle system;
              pNum:  the number of particles to be created;
 Return:      VOID
 Remarks:
************************************************************************************************/
LOCAL VOID createParticles(UINT16 psInd, UINT16 pNum)
{
    PARTICLE_ST        *pPtr, *pfPtr, *pNextPtr;
    PARTICLE_SYSTEM_ST *psPtr;
    UINT16             minPNum, cPNum, createNum, i;
    BOOL               sFlag;

    /* Step 1 -- Determine the number of particle to be created */
    psPtr = &SimPS[ psInd ];
    if( pNum == 0 )
    {
        /* No particular no. of particle is specified for the creation, determine from the create rate and max count */
        sFlag = FALSE;
        if( ( psPtr -> psBase.psFlag & SIM_PSB_PAT_MAX_CNT ) == SIM_PSB_PAT_MAX_CNT )
        {
            for( i = 0, cPNum = 0; i < SIM_PS_NUM; i++ )        /* Consider all current particles in a pattern */
            {
                if( SimPS[ i ].psBase.id != psInvalid )
                    cPNum += SimPS[ i ].psBase.currentCnt;
            }
        }
        else
            cPNum = psPtr -> psBase.currentCnt;

        if( cPNum >= ( psPtr -> psBase.maxCnt ) )
        {
            /* Already reached the maximum count, no room to create more, quit */
            return;
        }

        createNum = ( psPtr -> psBase.createRate & SIM_CREATE_NUM_MASK );
        if( createNum > ( ( psPtr -> psBase.maxCnt ) - cPNum ) )
        {
            /* Get all rooms left.. */
            pNum = ( psPtr -> psBase.maxCnt ) - cPNum;
        }
        else
        {
            /* Still have room to create at its own rate */
            pNum = createNum;
        }

        /* Further check the necessary no. of particle has to create for sequence particles */
        minPNum = 0;
        if( psPtr -> psBase.seqMax > 1 )
        {
            /* It is a sequence system, check for minimum no. particle has to create */
             pPtr  = &SimP[0];
             while( pPtr < &SimP[ SimPMax ] )
             {
                 if( pPtr -> systemID == psPtr -> psBase.id )
                 {
                    /* It is a particle of the concerned particle system, check its sequence */
                    if( pPtr -> seq == 1 )
                    {
                        /* Only needs to take care sequence heads */
                        pNextPtr = pPtr;
                        while( pNextPtr -> nextPPtr != ( PARTICLE_ST * )NULL )
                            pNextPtr = pNextPtr -> nextPPtr;

                        if( pNextPtr -> seqEnd != SIM_SEQ_END )
                        {
                            /* The sequence is not yet ended, need to add one particle in this cycle */
                            minPNum++;
                        }
                    }
                 }
                 pPtr++;
             }

             if( minPNum > pNum )
                 pNum = minPNum;
        }

        if( ( psPtr -> psBase.createRate & SIM_CREATE_P_MASK ) != 0 )
        {
            /* It is a probablistic creation process */
            /* Determine if necessary to create */
            i = SimRandom( 0, 65535 );      /* Use the maximum range of SimRandom() */
            if( i > ( 65535  / ( ( psPtr -> psBase.createRate & SIM_CREATE_P_MASK ) >> 5 ) ) )
            {
                /* It is not necessary to create now, reduce pNum to zero */
                pNum = 0;
            }
        }

        if( ( psPtr -> psBase.psFlag & SIM_PSB_RAND_CREATE ) == SIM_PSB_RAND_CREATE )
        {

            pNum = SimRandom( minPNum, pNum );  /* Randomize the number of particles to be created */
        }
    }
    else
    {
        /* Adjust requested no. if necessary */
        if( pNum > ( ( psPtr -> psBase.maxCnt ) - ( psPtr -> psBase.currentCnt ) ) )
        {
            /* Requested too many, reduce it */
            pNum = ( psPtr -> psBase.maxCnt ) - ( psPtr -> psBase.currentCnt );
        }

        sFlag = TRUE;
    }

    /* Step 2 -- Create sequence particles, if any */
    pPtr  = &SimP[0];
    if( ( psPtr -> psBase.seqMax ) > 1 )
    {
        pfPtr = &SimP[0];
        while( pPtr < &SimP[ SimPMax ] )
        {
            if( pNum == 0 )
            {
                /* All necessary particles have been created, quite immediately */
                return;
            }

            if( pPtr -> systemID == psPtr -> psBase.id )
            {
                /* It is a particle of the concerned particle system, check its sequence */
                if( pPtr -> nextPPtr != ( PARTICLE_ST * )NULL )
                {
                    pNextPtr = pPtr -> nextPPtr;

                    while( pNextPtr -> nextPPtr != ( PARTICLE_ST * )NULL )
                    {
                        pNextPtr = pNextPtr -> nextPPtr;
                    }
                }

                else
                {
                    /* It is the only particle */
                    pNextPtr = pPtr;
                }

                if( pNextPtr -> seqEnd != SIM_SEQ_END )
                {
                    /* The sequence is not yet ended, add a new one to its end */
                    while( pfPtr < &SimP[ SimPMax ] )
                    {
                        if( pfPtr -> systemID == SIM_PARTICLE_INVALID )
                        {
                            /* It is a free particle, get it */
                            break;
                        }
                        pfPtr++;
                    }

                    if( pfPtr < &SimP[ SimPMax ] )
                    {
                        /* A valid free particle is obtained, use it */
                        *pfPtr = *pNextPtr;
                        pfPtr -> seq = pNextPtr -> seq + 1;
                        SetInitColor( pfPtr, pfPtr -> layer, pfPtr -> color );
                        
                        pfPtr -> nextPPtr = ( PARTICLE_ST* )NULL;
                        if( pfPtr -> seq == pfPtr -> seqEnd )
                        {
                            /* change it to SIM_SEQ_END when its seq no. reaches the maximum */
                            pfPtr -> seqEnd = SIM_SEQ_END;
                        }

                        pNextPtr -> nextPPtr = pfPtr;
                        psPtr -> psBase.currentCnt++;
                        pNum--;
                    }
                    else
                    {
                        /* No more free particle, quit immediately */
                        return;
                    }
                }
                else
                {
                    /* Already reach the end of a sequence, move on to the next one */
                    if( sFlag )
                        pPtr++;     /* Advance pointer to offset decrement later */
                }

                if( sFlag )
                {
                    /* It is a special request of particle creation, make it all for the same sequence first */
                    /* It will go to the same particle head in the next loop then */
                    pPtr--;
                }
            }
            pPtr++;
        }
    }

    /* Step 2.5 -- Create new particles for Cross theme */
    if(psJet == SimPS[ psInd ].psBase.id)
    {
        if(pNum > 0)
            CreateNormalJetThemeParticles(psInd);
        return;
    }

    /* Step 3 -- Create ordinary new particles then */
    pPtr = &SimP[0];
    while( pPtr < &SimP[ SimPMax ] )   /* Check for free particles   */
    {
        if( pNum == 0 )
        {
            /* All necessary particles have been created, quite immediately */
            return;
        }

        if( ( pPtr -> systemID ) == ( SIM_PARTICLE_INVALID ) )
        {
            /* It is a free particle, use it */
            initParticle( pPtr, psInd );
            pNum--;
        }
        pPtr++;
    }
}

/**********************************************************************************************
 Function:    initParticle
 Description: Initialize a particle for the specified particle system
 Input:       pPtr:  pointer to the particle
              psInd: index of the particle system
 Return:      VOID
 Remarks:
***********************************************************************************************/
LOCAL VOID initParticle(PARTICLE_ST *pPtr, UINT16 psInd)
{
    UINT16 i, ind;
    int         nReTryCount     = 0;
    const int   MAX_RETRY_COUNT = 12;
    BOOL        bRandSuccess    = TRUE;
    PARTICLE_SYSTEM_ST *psPtr;
    COLOR_T  color = { 0, 0, 0 };

    psPtr = &SimPS[ psInd ];

    pPtr -> systemID = psPtr -> psBase.id;
    pPtr -> layer    = ( UINT8 )psInd;

    if( psPtr -> psBase.aVelMax == 0 )
        pPtr -> aVelocity = 0;
    else
        pPtr -> aVelocity = ( UINT8 )genRandScalar( psPtr -> psBase.aVelMin, psPtr -> psBase.aVelMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_AVEL ) );

    pPtr -> angle      = ( UINT8 )genRandScalar( psPtr -> psBase.angleMin, psPtr -> psBase.angleMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_ANGLE ) );
    pPtr -> aAmplitude = ( UINT8 )genRandScalar( psPtr -> psBase.aAmpMin, psPtr -> psBase.aAmpMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_AAMP ) );
    pPtr -> size       = ( UINT8 )genRandScalar( psPtr -> psBase.sizeMin, psPtr -> psBase.sizeMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_SIZE ) );
    pPtr -> grow       = ( INT8 )( genRandScalar( ( psPtr -> psBase.growMin + ( REN_SIM_FRAME >> 1 ) ), ( psPtr -> psBase.growMax + ( REN_SIM_FRAME >> 1 ) ), \
                         ( psPtr -> psBase.randSnap & SIM_PSB_RS_GROW ) ) - ( REN_SIM_FRAME >> 1 ) );

    pPtr -> mass       = ( UINT8 )genRandScalar( psPtr -> psBase.massMin, psPtr -> psBase.massMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_MASS ) );
    pPtr -> maxSpeed   = ( UINT8 )genRandScalar( psPtr -> psBase.maxSpeedMin, psPtr -> psBase.maxSpeedMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_MAXSPEED ) );
    pPtr -> maxSteer   = ( UINT8 )genRandScalar( psPtr -> psBase.maxSteerMin, psPtr -> psBase.maxSteerMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_MAXSTEER ) );

    if( psPtr -> psBase.lifeMin == SIM_PARTICLE_INFINITE )
        pPtr -> lifespan = SIM_PARTICLE_INFINITE;
    else
        pPtr -> lifespan = genRandScalar( psPtr -> psBase.lifeMin, psPtr -> psBase.lifeMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_LIFESPAN ) );

    pPtr -> seq        = 1;
    pPtr -> seqEnd     = psPtr -> psBase.seqMax;
    pPtr -> state      = 0;
    pPtr -> effectCnt  = 0;
    pPtr -> nextPPtr   = ( PARTICLE_ST * )NULL;

    /* Avoid duplicate location */
    if( ( psPtr -> psBase.psFlag & SIM_PSB_RAND_SEQ ) && ( psPtr -> psBase.randSnap & SIM_PSB_RS_LOC ) )
    {
        /* Generate random sequence grid location */
        ind = simGridRandomList[ psPtr -> psBase.locMin.scalar ];
        if( ind == INVALID_GD_NUM )
        {
            /* It is an empty sequence, generate it */
            prepareGridDataList( psPtr -> psBase.locMin.scalar, psPtr -> psBase.locMax.scalar, 0 );
            ind = simGridRandomList[ psPtr -> psBase.locMin.scalar ];
        }

        /* Use the existing sequence list */
        pPtr -> location = simGridData[ ind ].vector;

        /* Move the list one location */
        for( i = psPtr -> psBase.locMin.scalar; i < psPtr -> psBase.locMax.scalar; i++ )
            simGridRandomList[ i ] = simGridRandomList[ i + 1 ];

        simGridRandomList[ psPtr -> psBase.locMax.scalar ] = INVALID_GD_NUM;

        /* Generate a new one when the current one is emptied */
        if( simGridRandomList[ psPtr -> psBase.locMin.scalar ] == INVALID_GD_NUM )
            prepareGridDataList( psPtr -> psBase.locMin.scalar, psPtr -> psBase.locMax.scalar, ind );
    }
    else
    {
        genRandVector( psPtr -> psBase.locMin, psPtr -> psBase.locMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_LOC ), ( GEN_PARA_T * )&pPtr -> location  );
        if(psPtr -> psBase.randSnap & SIM_PSB_RS_ALIGN_LOC)
        {
            pPtr->location.x -= ((pPtr->location.x + SIM128_TO_VIR96)%REN_FRAME2VIR_FRAME);
            pPtr->location.y -= ((pPtr->location.y + SIM128_TO_VIR96)%REN_FRAME2VIR_FRAME);
        }
        do{
            bRandSuccess = TRUE;
            for( i = 0; i < RANDOM_HISTORY; i++ )
            {
                if( randomLoc[ i ].scalar != INVALID_LOC )
                {
                    if( ( pPtr -> location.x == randomLoc[ i ].vector.x ) && ( pPtr -> location.y == randomLoc[ i ].vector.y ) )
                    {
                        /* Repeated location, generate again */
                        genRandVector( psPtr -> psBase.locMin, psPtr -> psBase.locMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_LOC ), ( GEN_PARA_T * )&pPtr -> location  );
                        if(psPtr -> psBase.randSnap & SIM_PSB_RS_ALIGN_LOC)
                        {
                            pPtr->location.x -= ((pPtr->location.x + SIM128_TO_VIR96)%REN_FRAME2VIR_FRAME);
                            pPtr->location.y -= ((pPtr->location.y + SIM128_TO_VIR96)%REN_FRAME2VIR_FRAME);
                        }
                        bRandSuccess = FALSE;
                        nReTryCount++;
                    }
                }
            }
        }while(FALSE == bRandSuccess && nReTryCount <= MAX_RETRY_COUNT);
        for( i = 0; i < RANDOM_HISTORY; i++ )       /* Take out the first record */
            randomLoc[ i ] = randomLoc[ i + 1 ];
        randomLoc[ RANDOM_HISTORY - 1 ].vector.x = pPtr -> location.x;
        randomLoc[ RANDOM_HISTORY - 1 ].vector.y = pPtr -> location.y;
    }

    genRandVector( psPtr -> psBase.velMin, psPtr -> psBase.velMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_VEL ), ( GEN_PARA_T * )&pPtr -> velocity );
    genRandVector( psPtr -> psBase.accMin, psPtr -> psBase.accMax, ( psPtr -> psBase.randSnap & SIM_PSB_RS_ACC ), ( GEN_PARA_T * )&pPtr -> acceleration );
    pPtr -> accCnt = SimSqrt( SimVectorMag2( pPtr -> acceleration ) );  /* Get the generated acc count */
    pVectorScaling( pPtr -> acceleration, 1, &pPtr -> acceleration );   /* Scaling acceleration */

    pPtr -> grid = setGrid( pPtr -> location );

    if(psPtr->psBase.id == psExplosion)
    {
        SetExplosionColor( pPtr, psInd, color );
        if(pPtr->lifespan <= 255)
            pPtr->angle = (UINT8)pPtr->lifespan;//used for save init life span
    }
    else
    {
        SetInitColor( pPtr, psInd, color );
    }

    psPtr -> psBase.currentCnt++;
}

/************************************************************************
 Function:    physicsEngine
 Description: Calculate each time step particle behaviours
 Input:       Nil
 Return:      VOID
 Remarks:
************************************************************************/
LOCAL VOID physicsEngine(VOID)
{
    UINT16 psInd;

    for( psInd = 0; psInd < SIM_PS_NUM; psInd++ )
    {
        /* Step 2.1 -- Run for each particle */
        if( SimPS[ psInd ].psBase.id != psInvalid )
        {
            if( simTimeMult[ psInd ] <= 0 )
                simTimeMult[ psInd ]++;
            else
            {
                /* It is a valid particle system, check for any of its particles */
                runParticle( &SimPS[ psInd ] );

                /* Step 2.2 -- Update target */
                if( SimPS[ psInd ].psBase.psFlag & SIM_PSB_TARGET_UPD_MASK )
                {
                    /* Need to update target position */
                    updateTarget( &SimPS[ psInd ] );
                }
                if(SimPS[ psInd ].psBase.id == psExplosion)
                {
                    SimPS[ psInd ].psBase.cohesion += simTimeMult[ psInd ];
                    if(SimPS[ psInd ].psBase.cohesion >= EXPLOSION_MAX_HUE)
                        SimPS[ psInd ].psBase.cohesion -= EXPLOSION_MAX_HUE;
                }
                else if(SimPS[ psInd ].psBase.id >= psWaveHighPitch && 
                        SimPS[ psInd ].psBase.id <= psWaveBasic1)
                {
                    SimPS[ psInd ].psBase.cohesion += simTimeMult[ psInd ];
                    if(SimPS[ psInd ].psBase.cohesion >= 360)
                        SimPS[ psInd ].psBase.cohesion -= 360;
                }

                simTimeMult[ psInd ] = SimPS[ psInd ].psBase.timeMul;
            }
        }
    }
}

/**************************************************************************************************
 Function:    runParticle
 Description: Update current particles' state according to their forces and
              interaction
 Input:          psPtr: pointer to a particle system
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID runParticle(PARTICLE_SYSTEM_ST *psPtr)
{
    PARTICLE_ST *pPtr;

    pPtr  = &SimP[ 0 ];
    while( pPtr < &SimP[ SimPMax ] )
    {
        /* 1st,find the particle system */
        if( pPtr -> systemID == psPtr -> psBase.id )
        {
            if( pPtr -> state == ITARGET_SEEK )
            {
                /* Individual target seeking */
                iTargetSeek( pPtr, psPtr );
            }
            else
            {
                /* Ordinary cases, calculate forces and apply to particle */
                if( ( pVectorNull( psPtr -> psBase.target ) == FALSE ) && ( psPtr -> psBase.psFlag & SIM_PSB_SEEK_TARGET ) )
                {
                    /* There is a target for seeking */
                    /* Consider vertical or horizontal movement only */
                    seekTarget( pPtr, psPtr -> psBase.target );
                }
            }
            updateOneParticle( pPtr, &(psPtr -> psBase) );
        }
        pPtr++;
    }
}

/**************************************************************************************************
 Function:    updateTarget
 Description: Update the target position of a particle system
 Input:          psPtr: pointer to a particle system
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID updateTarget(PARTICLE_SYSTEM_ST *psPtr)
{
    UINT16 i;
    INT16  checkx, checky, lx, ly, tx, ty;

    if( ( pVectorNull( psPtr -> psBase.target ) == FALSE ) || ( psPtr -> psImage.psiFlag & REN_PSI_SHOW_TARGET ) )
    {
        /* There is a valid target, update it */
        tx = ( psPtr -> psBase.target.x + SIM128_TO_VIR96 ) / REN_FRAME2VIR_FRAME;
        ty = ( psPtr -> psBase.target.y + SIM128_TO_VIR96 ) / REN_FRAME2VIR_FRAME;

        checkx = 0xFFFF;
        checky = 0xFFFF;
        for( i = 0; i < SimPMax; i++ )
        {
            if( SimP[ i ].systemID == psPtr -> psBase.id )
            {
                lx = ( SimP[ i ].location.x + SIM128_TO_VIR96 ) / REN_FRAME2VIR_FRAME;
                ly = ( SimP[ i ].location.y + SIM128_TO_VIR96 ) / REN_FRAME2VIR_FRAME;
                switch( psPtr -> psBase.psFlag & SIM_PSB_TARGET_UPD_MASK )
                {
                    case SIM_PSB_TARGET_UPD_MAXX:
                        checky = ty;
                        if( ly == ty )
                        {
                            /* At the same y, check for max x */
                            if( ( lx + 1 > checkx ) || ( checkx == 0xFFFF ) )
                                checkx = lx + 1;
                        }
                        break;

                    case SIM_PSB_TARGET_UPD_MAXY:
                        checkx = tx;
                        if( lx == tx )
                        {
                            /* At the same x, check for max y */
                            if( ( ly + 1 > checky ) || ( checky == 0xFFFF ) )
                                checky = ly + 1;
                        }
                        break;

                    case SIM_PSB_TARGET_UPD_MINX:
                        checky = ty;
                        if( ly == ty )
                        {
                            /* At the same y, check for min x */
                            if( ( lx - 1 < checkx ) || ( checkx == 0xFFFF ) )
                                checkx = lx - 1;
                        }
                        break;

                    case SIM_PSB_TARGET_UPD_MINY:
                        checkx = tx;
                        if( lx == tx )
                        {
                            /* At the same x, check for min y */
                            if( ( ly - 1 < checky ) || ( checky == 0xFFFF ) )
                                checky = ly - 1;
                        }
                        break;

                    default:
                        checkx = tx;
                        checky = ty;
                        break;
                }
            }
        }

        /* Set to target */
        psPtr -> psBase.target.x = checkx * REN_FRAME2VIR_FRAME - SIM128_TO_VIR96;
        psPtr -> psBase.target.y = checky * REN_FRAME2VIR_FRAME - SIM128_TO_VIR96;
    }
}

/**************************************************************************************************
 Function:    iTargetSeek
 Description: Seek the individual target of a particle
 Input:       pPtr : pointer to the particle to be applied force
              psPtr: the system of the particle to be applied forces
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID iTargetSeek(PARTICLE_ST *pPtr, PARTICLE_SYSTEM_ST *psPtr)
{
    PVECTOR_ST target;
    UINT16     tInd;

    /* Step 1 -- Calculate the target index */
    for( tInd = 0; tInd < GM_POS_MAX * psPtr -> psBase.seqMax; tInd++ )
    {
        if( &SimP[ tInd ] == pPtr )
            break;
    }

    /* Step 2 -- Calculate the target location */
    tInd /= psPtr -> psBase.seqMax;
    target.x = ( REN_PHY_FRAME_W - 1 - simITarget[ tInd ] % REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;
    target.y = ( REN_PHY_FRAME_H - simITarget[ tInd ] / REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;

    /* Step 3 -- Calculate the seeking velocity */
    seekTarget( pPtr, target );
}

/**************************************************************************************************
 Function:    seekTarget
 Description: Modify the velocity of a particle to approach a target
 Input:       pPtr:   the particle
              target: the seeking target
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID seekTarget(PARTICLE_ST *pPtr, PVECTOR_ST target)
{
    INT16 dx, dy, vx, vy, vmag;

    vx = pPtr -> velocity.x;
    vy = pPtr -> velocity.y;
    vmag = 0;
    if( vx > 0 )
        vmag = vx;
    else if( vx < 0 )
        vmag = -vx;

    if( vy > vmag )
        vmag = vy;
    else if( -vy > vmag )
        vmag = -vy;

    dx = target.x - pPtr -> location.x;
    dy = target.y - pPtr -> location.y;
    if( ( ( dx < vmag ) && ( dx >= 0 ) ) || ( ( -dx < vmag ) && ( dx <= 0 ) ) )
    {
        if( dy > 0 )
        {
            /* Move it in +y direction */
            pPtr -> velocity.y = vmag;
        }
        else
            pPtr -> velocity.y = -vmag;

        pPtr -> velocity.x = 0;
    }
    else if( ( ( dy < vmag ) && ( dy >= 0 ) ) || ( ( -dy < vmag ) && ( dy <= 0 ) ) )
    {
        if( dx > 0 )
        {
            /* Move it in +x direction */
            pPtr -> velocity.x = vmag;
        }
        else
            pPtr -> velocity.x = -vmag;

        pPtr -> velocity.y = 0;
    }
}

/**************************************************************************************************
 Function:    updateOneParticle
 Description: Update the particle's property
 Input:       pPtr:   pointer, point to the particle to be updated
              psbPtr: pointer, point to the particle system base information which the particle belongs to
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID updateOneParticle(PARTICLE_ST *pPtr, PS_BASE_T *psbPtr)
{
    INT16  sq, sineScalar, size;
    UINT16 timeBase, seq, maxSeq;
    PVECTOR_ST  velocity;
    PARTICLE_ST *endPtr, *prevPtr, *currentPtr;

    if( pPtr -> seq != 1 )
    {
        /* Only update seq = 1 particles, all others would be updated from their corresponding head particles */
        return;
    }

    if( pPtr -> seqEnd > 1 )
    {
        /* It is a sequence particle chain head, update its subsequent particles first */
        /* Step 1 -- Update sequence particles */
        maxSeq = 1;
        /* Start from the end of the sequence */
        endPtr = ( PARTICLE_ST * )NULL;
        while( endPtr != pPtr -> nextPPtr )
        {
            currentPtr = pPtr;
            prevPtr    = pPtr;
            while( currentPtr -> nextPPtr != endPtr )
            {
                prevPtr    = currentPtr;                
                currentPtr = currentPtr -> nextPPtr;
                seq = currentPtr -> seq;
            }
            if( seq > maxSeq )
                maxSeq = seq;

            /* Copy the location and lifespan to the next particle */
//            if( pVectorNull( pPtr -> velocity ) == FALSE )
//            {
                currentPtr -> lifespan = prevPtr -> lifespan;
                
                if( ( psbPtr -> psFlag & SIM_SEQ_LOC_Y )== SIM_SEQ_LOC_Y )
                {
                    currentPtr -> location.y = prevPtr -> location.y;
                    currentPtr -> velocity = pPtr -> velocity;          /* Just to make every particle after head in sequence  have a tail */
                }
                else
                    currentPtr -> location = prevPtr -> location;
//            }
            endPtr = currentPtr;    /* Move up one particle */
        } 

        if( ( psbPtr -> psFlag & SIM_SYMMETRY_LOC ) == SIM_SYMMETRY_LOC )
        {
            if( maxSeq > 2 )
                copySymmetryLocation( pPtr, ( UINT8 )maxSeq );
        }
    }

    /* Step 2 -- Update sequence head or individual particles */
    timeBase = simTimeMult[ pPtr -> layer ];

    /* Step 2.1 -- Update size */
    size = pPtr -> size;
    size += ( ( pPtr -> grow ) * timeBase );
    if( size >= 255)
        size -= 255;
    
    if( size < 0)
        size = 0;
    pPtr -> size = size;        

    /* Step 2.2 -- Update angular amplitude and angle */
    pPtr -> angle += ( ( pPtr -> aVelocity ) * timeBase );
    if( pPtr -> angle >= 180 )      /* Angle is in 2 degrees per unit, hence, 180 is a complete circle */
        pPtr -> angle -= 180;
    
    /* Step 2.3 -- Update location and velocity */
    if( ( HINIBBLE( pPtr -> accCnt ) == 0 ) && ( LONIBBLE( pPtr -> accCnt ) != 0 ) )
    {
        /* It is time to update velocity */
        pVectorAdd( pPtr -> acceleration, pPtr -> velocity, &( pPtr -> velocity ) );
        sq = SimVectorMag2( pPtr -> velocity );

        if( sq > ( pPtr -> maxSpeed ) * ( pPtr -> maxSpeed ) )
        {
            /* Too high the speed, limit it */
            pVectorScaling( pPtr -> velocity, pPtr -> maxSpeed, &( pPtr -> velocity ) );
        }

        pPtr -> accCnt = pPtr -> accCnt + ( pPtr -> accCnt << 4 );  /* Reload counter */
    }
    else if( LONIBBLE( pPtr -> accCnt ) != 0 )
    {
        /* Acceleration is valid, update counter */
        pPtr -> accCnt  -= SIM_ACC_CNT;
    }

    if( psbPtr -> psFlag & SIM_PSB_SHM )
    {
        /* Modulate simple harmonic motion to the particle */
        sineScalar = ( pPtr -> aAmplitude * SimSin( ( INT16 )( pPtr -> angle * 2 ) ) );
        if(TRUE == AudioDet)
        {
            pPtr -> velocity.y = ( INT8 )( sineScalar / 5 );
        }
        else
        {
            pPtr -> velocity.y = ( INT8 )( sineScalar >> 1 );
        }
        pPtr -> location.y = psbPtr -> locMin.vector.y;
    }

    pVectorMult( pPtr -> velocity, timeBase, &velocity );   
    if( ( psbPtr -> psFlag & IN_FRAME_FOREVER )&&( pPtr -> location.y < LOWEST_LED_CENTER ) )
    {
        pPtr -> location.y = LOWEST_LED_CENTER;
        velocity.y = 0;
    }    
    pVectorAdd( pPtr -> location, velocity, &( pPtr -> location ) );

    pPtr -> grid = setGrid( pPtr -> location );

    /* Step 2.4 -- Update life span */
    if( ( pPtr -> lifespan != SIM_PARTICLE_INFINITE ) && ( pPtr -> lifespan > 0 ) )
    {
        if( pPtr -> lifespan > timeBase )
            pPtr -> lifespan -= timeBase;
        else
            pPtr -> lifespan = 0;
    }

    /* Step 2.5 -- Update effectCnt */
    if( pPtr -> effectCnt > 0 )
    {
        if( pPtr -> effectCnt > timeBase )
            pPtr -> effectCnt -= timeBase;
        else
            pPtr -> effectCnt = 0;
    }
}

/************************************************************************
 Function:    setGrid
 Description:    Calculate the grid no. of a location
 Input:        posPtr: a location vector
 Return:    Grid no.
 Remarks:       Grid no. is assigned from the lower left corner, from left to right
                and then bottom up.
                The extreme of coordinates, i.e. +/-128 are marked as out of scope.
                Hence, grid 0 is for (-127, -127) to (-65, -65)
************************************************************************/
LOCAL UINT8 setGrid(PVECTOR_ST posPtr)
{
//    UINT16  gridX, gridY;
    UINT16  x, y;

    x = posPtr.x + ( REN_SIM_FRAME >> 1 );
    y = posPtr.y + ( REN_SIM_FRAME >> 1 );

    if( ( x == 0 ) || ( y == 0 ) || ( x >= REN_SIM_FRAME - 1 ) || ( y >= REN_SIM_FRAME - 1 ) )
    {
        /* The frame of the simulation coordinates is reached, mark it as out of scope */
        return SIM_OUT_SCOPE;
    }

    return 0;   /* If not out of scope, always return zero */

//    gridX = x / SIM_GRID_LEN;
//    gridY = y / SIM_GRID_LEN;
//
//    return ( UINT8 )( gridY * ( REN_SIM_FRAME / SIM_GRID_LEN ) + gridX );
}

/**************************************************************************************************
 Function:    pVectorAdd
 Description: Vector addition
 Input:       v1:   input vector
              v2:   input vector
              rPtr: pointer to the resultant vector
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID pVectorAdd(PVECTOR_ST v1, PVECTOR_ST v2, PVECTOR_ST *rPtr)
{
    INT16 addResult;

    addResult = v1.x + v2.x;
    if( addResult > 127 )
        addResult = 127;
    else if( addResult < -128 )
        addResult = -128;

    rPtr -> x = ( INT8 )addResult;

    addResult = v1.y + v2.y;
    
    if( addResult > 127 )
        addResult = 127;
    else if( addResult < -128 )
        addResult = -128;

    rPtr -> y = ( INT8 )addResult;
}

#if 0
/**************************************************************************************************
 Function:    pVectorSub
 Description: Vector substration
 Input:          v1:   the vector to be substracted
              v2:   the substractive vector
              rPtr: pointer to the resultant vector
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID pVectorSub(PVECTOR_ST v1, PVECTOR_ST v2, PVECTOR_ST *rPtr)
{
    INT16 subResult;

    subResult = v1.x - v2.x;
    if( subResult > 127 )
        subResult = 127;
    else if( subResult < -128 )
        subResult = -128;

    rPtr -> x = ( INT8 )subResult;

    subResult = v1.y - v2.y;
    if( subResult > 127 )
        subResult = 127;
    else if( subResult < -128 )
        subResult = -128;

    rPtr -> y = ( INT8 )subResult;
}
#endif

/**************************************************************************************************
 Function:     pVectorMult
 Description:  Multiplication between a vector and a number
 Input:           v1:   the vector to be multiplied
               num:  the number to multiply
               rPtr: pointer to the resultant vector
 Return:       VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID pVectorMult(PVECTOR_ST v1, INT8 num, PVECTOR_ST *rPtr)
{
    INT16 multResult;

    multResult = v1.x * num;
    
    if( multResult > 127 )
        multResult = 127;
    else if( multResult < -128 )
        multResult = -128;

    rPtr -> x = ( INT8 )multResult;

    multResult = v1.y * num;
    if( multResult > 127 )
        multResult = 127;
    else if( multResult < -128 )
        multResult = -128;

    rPtr -> y = ( INT8 )multResult;
}

#if 0
/**************************************************************************************************
 Function:     pVectorDiv
 Description:  Divide a vector by a number
 Input:           v1:   the vector to be devided
               num:  the number to divide
               rPtr: pointer to the resultant vector
 Return:       VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID pVectorDiv(PVECTOR_ST v1, INT8 num, PVECTOR_ST *rPtr)
{
    rPtr -> x = v1.x / num;
    rPtr -> y = v1.y / num;
}
#endif

/**************************************************************************************************
 Function:      pVectorScaling
 Description:   Normalize a vector and then multiple the specified scaling factor
 Input:            v1:   vector for checking
                num:  scalar for scaling the vector
                rPtr: pointer to the resultant vector
 Return:        VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID pVectorScaling(PVECTOR_ST v1, INT8 num, PVECTOR_ST *rPtr)
{
    INT16 magNum, xNum, yNum;

    xNum = v1.x;
    yNum = v1.y;

    if( xNum == 0 && yNum == 0 )
    {
        /* The vector itself is already zero, nothing to scale */
        *rPtr = v1;
    }
    else
    {
        // Avoid number / SimSqrt( pVectorMag2( a1 ) ) == 0
        magNum = num;
        magNum <<= EIGHT_BITS;
        magNum /= SimSqrt( SimVectorMag2( v1 ) );

        /* For pVector's every member is INT8 number,and   */
        xNum *= magNum;
        yNum *= magNum;

        rPtr -> x = ( INT8 )( xNum >> EIGHT_BITS );
        rPtr -> y = ( INT8 )( yNum >> EIGHT_BITS );
    }
}

/**************************************************************************************************
 Function:     pVectorNull
 Description:  Check if it is a NULL vector, i.e. zero amplitude
 Input:           v1: vector for checking
 Return:       TRUE: it is a null vector; FALSE: it is not a null vector
 Remarks:
***************************************************************************************************/
LOCAL inline BOOL pVectorNull(PVECTOR_ST v1)
{
    if( ( v1.x == 0 ) && ( v1.y == 0 ) )
    {
        return TRUE;
    }

    return FALSE;
}

/**************************************************************************************************
 Function:    inputOperate
 Description: Execute an operation with an input trigger
 Input:          inVal: input value from GMInput[].scalar
              opInd: operation index
              psInd: particle system index
 Return:      TRUE: action has been taken; FALSE: no action has been taken
 Remarks:
***************************************************************************************************/
LOCAL BOOL inputOperate(INT16 inVal, UINT16 opInd, UINT16 psInd)
{
    UINT16 i, maxA, maxC, aInd, pInd;
    BOOL   checkResult, actionFlag, operationFlag;

    operationFlag = FALSE;

    /* Step 1 -- Prepare for action limit count */
    aInd = simExOp[ opInd ].actionOffset;
    maxA = simExOp[ opInd ].actionNum;
    for( i = 0; i < maxA; i++ )
    {
        if( ( simExAction[ aInd + i ].actionType & SIM_ACTION_LIMIT_MASK ) == SIM_ACTION_ABS_LIMIT )
        {
            /* It is an absolute limit, set it directly */
            simExActionCnt[ i ] = simExAction[ aInd + i ].limit;
        }
        else
        {
            /* It is a proportional limit */
            simExActionCnt[ i ] = ( UINT8 )( simExAction[ aInd + i ].limit * inVal );
        }
    }

    /* Step 2 -- Check process */
    maxC = simExOp[ opInd ].checkNum + simExOp[ opInd ].checkOffset;

    /* Check the checkings from the operations */
    for( pInd = 0; pInd < SimPMax; pInd++ )
    {
        if( ( SimP[ pInd ].systemID == SimPS[ psInd ].psBase.id ) || ( SimPS[ psInd ].psBase.currentCnt == 0 ) ) /* The last checking may not be robust enough... */
        {
            checkResult = TRUE;
            for( i = simExOp[ opInd ].checkOffset; i < maxC; i++ )
            {
                checkResult = checkProcess( i, pInd, psInd, inVal );
                if( checkResult == FALSE )
                {
                    /* Can't pass the check, no need to go to the next check */
                    if( SimParaProperty[ simExCheck[ i ].pID ].pGroup == psgP )
                    {
                        /* It is a particle check, move on to the next particle */
                        break;
                    }
                    else
                    {
                        /* It is a system check, no need to further the process */
                        return FALSE;
                    }
                }
            }

            if( SimP[ pInd ].effectCnt > 0 )
            {
                /* Still in an effect valid period, cannot use */
                checkResult = FALSE;
            }

            if( checkResult == TRUE )
            {
                /* Check process passed, take action then */
                /* Step 3 -- Action process */
                actionFlag = FALSE;
                for( i = 0; i < maxA; i++ )
                {
                    if( simExActionCnt[ i ] > 0 )
                    {
                        /* Still within limit, take action */
                        actionProcess( i + aInd, pInd, psInd, inVal );
                        simExActionCnt[ i ]--;
                        actionFlag    = TRUE;
                        operationFlag = TRUE;
                    }
                }

                if( actionFlag == FALSE )
                {
                    /* There is no action taken, due to limit reached, quit it */
                    break;
                }
            }
        } /* if particle valid */
    } /* for  loop */

    return operationFlag;
}

/**************************************************************************************************
 Function:    checkProcess
 Description: Parameter check process
 Input:          cInd:  check process ID
              pInd:  particle index
              psInd: particle system index
              inputValue: external input value
 Return:      TRUE: the check process is passed; FALSE: the check process is failed
 Remarks:
***************************************************************************************************/
LOCAL BOOL checkProcess(UINT16 cInd, UINT16 pInd, UINT16 psInd, INT16 inputValue )
{
    UINT8 *ptr;
    UINT16 paraInd;

    INT32      sData;
    PVECTOR_ST vData;

    paraInd = simExCheck[ cInd ].pID;

    /* Step 1 -- Locate parameter */
    switch( SimParaProperty[ paraInd ].pGroup )
    {
        case psgP:
            ptr = ( UINT8 * )( &SimP[ pInd ] );
            break;

        case psgPsb:
            ptr = ( UINT8 * )( &SimPS[ psInd ].psBase );
            break;

        case psgPsi:
            ptr = ( UINT8 * )( &SimPS[ psInd ].psImage );
            break;
    }
    ptr += SimParaProperty[ paraInd ].offset;

    /* Step 2 -- Read out parameter */
    switch( SimParaProperty[ paraInd ].pType )
    {
        case pstUINT8:
            sData = ( INT32 )( *ptr );
            break;

        case pstINT8:
            sData = ( INT32 )( *( INT8 * )ptr );
            break;

        case pstUINT16:
            sData = ( INT32 )( * ( UINT16 * )ptr );
            break;

        case pstINT16:
            sData = ( INT32 )( *( INT16 * )ptr );
            break;

        case pstLoc2LED:
            sData = ( INT32 )( ( *( INT8 * )ptr + REN_VIR_DIS_SCALE + - LOWEST_LINE ) >> 4 );    /* Convert location to LED index */
            if( sData > 7 )
                return FALSE;
            if( inputValue / simExCheck[ cInd ].value.scalar > sData + 1 )
                return TRUE;
            else 
                return FALSE;
            
        case pstSize2LED:
            sData = ( INT32 )( ( *( UINT8 * )ptr >> 4 ) - 1 ) >> 1;    /* Convert size to LED index ,may need add particle position */
            if( sData >= 6 )
                return FALSE;
            if( inputValue / simExCheck[ cInd ].value.scalar > sData + 2 )  
                return TRUE;
            else 
                return FALSE;
            
        case pstPTR:
            /* No pointer comparision... */
            return FALSE;

        case pstVector:
            vData = *( PVECTOR_ST * )ptr;
            return compareVector( vData, simExCheck[ cInd ].value.vector, simExCheck[ cInd ].checkType );
    }

    return compareScalar( sData, simExCheck[ cInd ].value.scalar, simExCheck[ cInd ].checkType );
}

/**************************************************************************************************
 Function:    actionProcess
 Description: Set parameter action process
 Input:          aInd:  action process ID
              pInd:  particle index
              psInd: particle system index
              iVal:  input value
 Return:      VOID
 Remarks:
***************************************************************************************************/
LOCAL VOID actionProcess(UINT16 aInd, UINT16 pInd, UINT16 psInd, INT16 iVal)
{
    UINT8 *ptr;
    UINT16 paraInd, gInd;

    INT32      sData1, sData2;
    PVECTOR_ST vData1, vData2;

    /* Step 1 -- Special treatment ... */
    switch( simExAction[ aInd ].actionType )
    {
        case satFireworkApp:
//            setFireworkParticles( psInd );
            createSpecialFireworks( psInd, psexiApp );
            return;

        case satTrafficApp:
            setTrafficTarget( psInd );
            return;

        case satStarApp:
            setMeteorParticle( psInd );
                                                        /* Reset the app flag */
            GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
            return;

        case satWaveApp:
            setWaveLevel();
                                                        /* Reset the app flag */
            GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
            return;

        case satFireflyApp:
            setFireflyParticles( psInd );               /* No need to reset app flag */
            return;

        case setCustomizedApp:
 //           GMPatternStatus[ PatCustomized ] = ( UINT8 )GMExInput[ psexiApp ].scalar;
//            GMPatternInfoTx.status = GMPatternStatus[ PatCustomized ];    /* Prepare broadcast info */
                                                        /* Reset the app flag */
 //           GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
            return;

        case satFireApp:
            setFireLevel( psInd, GMExInput[ psexiApp ].scalar );
            GMPatternStatus[ PatFire ] = ( UINT8 )GMExInput[ psexiApp ].scalar;
            GMPatternInfoTx.status = GMPatternStatus[ PatFire ];    /* Prepare broadcast info */
                                                        /* Reset the app flag */
            GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
            return;

        case satCanvasApp:
            setCanvasParticles( psInd );
                                                        /* Reset the app flag */
            GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
            return;

        case satHourglassApp:
            /* Time scale is in minute, convert it to second */
            setHourglassParticle( psInd, GMExInput[ psexiApp ].scalar * 60 );
            GMPatternStatus[ PatHourglass ] = ( UINT8 )GMExInput[ psexiApp ].scalar;
            GMPatternInfoTx.status = GMPatternStatus[ PatHourglass ];    /* Prepare broadcast info */
                                                        /* Reset the app flag */
            GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
            return;

        case satCanvasColorPick:
            GMExInput[ psexiColor ].scalar = FALSE;     /* Reset the color pick flag */
            return;

        case satFireworkShake:
            createSpecialFireworks( psInd, psexiShake );
            return;

        case satMeteorShowerShake:
            setStarAndMeteorShowerParticleSystem( psInd );
            return;

        case satWaveShake:
            setWaveShake();
            return;

        case satCanvasShake:
            if( GMExInput[ psexiShake ].scalar != 0 )
                InterchangeCanvasColor( psInd );
            return;
            
        case satBarShake:
            setBarShake( GMExInput[ psexiShake ].scalar );
            return;    

        case satExInput:
            updateFountainLocation( &SimP[ pInd ], iVal, simExAction[ aInd ].value.scalar, psInd );
            return;                
    }

    /* Step 2 -- Locate the destination */
    paraInd = simExAction[ aInd ].pID;
    switch( SimParaProperty[ paraInd ].pGroup )
    {
        case psgP:
            ptr = ( UINT8 * )( &SimP[ pInd ] );
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

    /* Step 3 -- Read out the current data */
    if( simExAction[ aInd ].actionType & SIM_ACTION_GRID_MASK )
    {
        gInd = simExAction[ aInd ].limit;       /* limit is used as grid data index */
        switch( SimParaProperty[ paraInd ].pType )
        {
            case pstUINT8:
            case pstINT8:
            case pstINT16:
                sData1 = ( INT32 )( simGridData[ gInd ].scalar );
                break;

            case pstUINT16:
                sData1 = ( INT32 )( * ( UINT16 * )&simGridData[ gInd ].scalar );
                break;

            case pstVector:
                vData1 = simGridData[ gInd ].vector;
                break;

            default:
                return;
        }
    }
    else
    {
        switch( SimParaProperty[ paraInd ].pType )
        {
            case pstUINT8:
                sData1 = ( INT32 )( *ptr );
                break;

            case pstINT8:
                sData1 = ( INT32 )( *( INT8 * )ptr );
                break;

            case pstUINT16:
                sData1 = ( INT32 )( * ( UINT16 * )ptr );
                break;

            case pstINT16:
                sData1 = ( INT32 )( *( INT16 * )ptr );
                break;

            case pstPTR:
                /* No pointer comparision... */
                return;

            case pstVector:
                vData1 = *( PVECTOR_ST * )ptr;
                break;

            default:
                return;
        }
    }

    /* Step 4 -- Prepare data and set to destination */
    if( SimParaProperty[ paraInd ].pType == pstVector )
    {
        vData2 = simExAction[ aInd ].value.vector;
        switch( simExAction[ aInd ].actionType & ~SIM_ACTION_LIMIT_MASK )
        {
            case satAbsValAbsLimit:
                break;

            case satProValAbsLimit:
                pVectorMult( vData2, iVal, &vData2 );
                break;

            case satRelValAbsLimit:
                pVectorAdd( vData1, vData2, &vData2 );
                break;

            case satRelProValAbsLimit:
                pVectorMult( vData2, iVal, &vData2 );
                pVectorAdd( vData1, vData2, &vData2 );
                break;

            case satAbsDirAbsLimit:
                pVectorScaling( vData2, SimSqrt( SimVectorMag2( vData1 ) ), &vData2 );
                break;

            default:
                return;
        }

        /* Set to destination */
        if( simExAction[ aInd ].actionType & SIM_ACTION_GRID_MASK )
            simGridData[ gInd ].vector = vData2;
        else
            *( PVECTOR_ST * )ptr = vData2;
    }
    else
    {
        if( SimParaProperty[ paraInd ].pType == pstUINT16 )
            sData2 = ( UINT16 )( simExAction[ aInd ].value.scalar );
        else
            sData2 = simExAction[ aInd ].value.scalar;

        switch( simExAction[ aInd ].actionType & (~SIM_ACTION_LIMIT_MASK) )
        {
            case satAbsValAbsLimit:
            case satAbsValGrid:
                break;

            case satProValAbsLimit:
            case satProValGrid:
                sData2 *= iVal;
                break;

            case satRelValAbsLimit:
            case satRelValGrid:
                sData2 += sData1;
                break;

            case satRelProValAbsLimit:
            case satRelProValGrid:
                sData2 = sData1 + sData2 * iVal;
                break;

            case satDivValAbsLimit:
                sData2 = iVal / sData2;
                break;
                
            case satSetFlag:
                sData2 |= sData1;
                break;

            case satResetFlag:
                sData2 = sData1 & ( ~sData2 );
                break;

            default:
                return;
        }

        /* Set to destination */
        if( simExAction[ aInd ].actionType & SIM_ACTION_GRID_MASK )
        {
            switch( SimParaProperty[ paraInd ].pType )
            {
                case pstUINT8:
                case pstINT8:
                case pstINT16:
                    simGridData[ gInd ].scalar = ( INT16 )sData2;
                    break;

                case pstUINT16:
                    simGridData[ gInd ].scalar = ( INT16 )( sData2 & 0x0000FFFF );
                    break;

                default:
                    break;
            }
        }
        else
        {
            switch( SimParaProperty[ paraInd ].pType )
            {
                case pstUINT8:
                    *ptr = ( UINT8 )sData2;
                    break;

                case pstINT8:
                    *( INT8 * )ptr = ( INT8 )sData2;
                    break;

                case pstUINT16:
                    *( UINT16 * )ptr = ( UINT16 )sData2;
                    break;

                case pstINT16:
                    *( INT16 * )ptr = ( INT16 )sData2;
                    break;
 
                default:
                    break;
            }
        }
    }
}

/**************************************************************************************************
 Function:    compareScalar
 Description: Compare the values of two scalars
 Input:          s1:    first scalar
              s2:    second scalar
              cType: compare type

 Return:      TRUE: s1 "cType" s2 is correct; FALSE: s1 "cType" s2 is not correct
 Remarks:
***************************************************************************************************/
LOCAL BOOL compareScalar(INT32 s1, INT32 s2, UINT16 cType )
{
    switch( cType )
    {
        case sctEqual:
            return ( s1 == s2 );

        case sctNotEqual:
            return ( s1 != s2 );

        case sctGreater:
            return ( s1 > s2 );

        case sctLess:
            return ( s1 < s2 );

        default:
            break;
    }

    return FALSE;
}

/**************************************************************************************************
 Function:    compareVector
 Description: Compare two vectors
 Input:          v1:    first vector
              v2:    second vector
              cType: compare type
 Return:      TRUE: v1 "cType" v2 is correct; FALSE: v1 "cType" v2 is not correct
 Remarks:
***************************************************************************************************/
LOCAL BOOL compareVector(PVECTOR_ST v1, PVECTOR_ST v2, UINT16 cType)
{
    switch( cType )
    {
        case sctEqualDir:
            pVectorScaling( v1, 10, &v1 );  /* Scaling v1 and v2 to the same magnitude */
            pVectorScaling( v2, 10, &v2 );  /* The magnitude is arbitrarily set */
            /* Sharing the same checking as that of sctEqual */

        case sctEqual:
            return ( ( v1.x == v2.x ) && ( v1.y == v2.y ) );

        case sctNotEqual:
            return ( ( v1.x != v2.x ) || ( v1.y != v2.y ) );

        case sctGreater:
            return ( ( v1.x > v2.x ) && ( v1.y > v2.y ) );

        case sctLess:
            return ( ( v1.x < v2.x ) && ( v1.y < v2.y ) );
            
        default:
            break;
    }

    return FALSE;
}

/**********************************************************************************************
 Function:    genRandScalar
 Description: Generate a random scalar, with grid or not check
 Input:       min:  minimum value
              max:  maximum value
              mask: randSnap flag bit mask
 Return:      Result scalar
 Remarks:
***********************************************************************************************/
LOCAL UINT16 genRandScalar(UINT16 min, UINT16 max, UINT16 mask)
{
    UINT16 rd;

    rd = SimRandom( min, max );
    if( mask )
        return simGridData[ rd ].scalar;
    else
        return rd;
}

/**********************************************************************************************
 Function:    genRandVector
 Description: Generate a random vector, with grid or not check
 Input:       min:  minimum vector
              max:  maximum vector
              mask: randSnap flag bit mask
              rPtr: pointer to resultant vector
 Return:      VOID
 Remarks:     min and max would be indexes to simGridData if it is a grid vector
***********************************************************************************************/
LOCAL VOID genRandVector(GEN_PARA_T vMin, GEN_PARA_T vMax, UINT16 mask, GEN_PARA_T *rPtr)
{
    UINT16 ind;

    if( mask )
    {
        ind = SimRandom( vMin.scalar, vMax.scalar );        
        *rPtr = simGridData[ ind ];
    }
    else
    {
        rPtr -> vector.x = ( INT8 )( ( INT16 )( SimRandom( ( vMin.vector.x + ( REN_SIM_FRAME >> 1 ) ), ( vMax.vector.x + ( REN_SIM_FRAME >> 1 ) ) ) ) - ( REN_SIM_FRAME >> 1 ) );
        rPtr -> vector.y = ( INT8 )( ( INT16 )( SimRandom( ( vMin.vector.y + ( REN_SIM_FRAME >> 1 ) ), ( vMax.vector.y + ( REN_SIM_FRAME >> 1 ) ) ) ) - ( REN_SIM_FRAME >> 1 ) );
    }
}

/**********************************************************************************************
 Function:    prepareGridDataList
 Description: Generate a random list of grid data index
 Input:       min:  minimum grid data index
              max:  maximum grid data index
              not1: the index no. cannot be the first one in the list
 Return:      VOID
 Remarks:     this method is recommended to be used to prepare initial particle location for grid data in small number.
              It can avoid duplicate locations.
***********************************************************************************************/
LOCAL VOID prepareGridDataList(UINT16 min, UINT16 max, UINT16 not1)
{
    UINT16 i, ind, tempDat;

    /* Step 1 -- Check data vaility */
    if( min == max )
    {
        /* There is only one data, always the same */
        simGridRandomList[ min ] = min;
        return;
    }

    /* Step 2 -- Fill in indexes sequentially */
    for( i = min; i <= max; i++ )
        simGridRandomList[ i ] = i;

    /* Step 3 -- Generate the rest indexes */
    for( i = min; i <= max; i++ )
    {
        ind = SimRandom( min, max );
        while( ( i == min ) && ( ind == not1 ) )
            ind = SimRandom( min, max );

        /* Exchange the data */
        tempDat = simGridRandomList[ ind ];
        simGridRandomList[ ind ] = simGridRandomList[ i ];
        simGridRandomList[ i ]   = ( UINT8 )tempDat;
    }
}

#if 0
/**********************************************************************************************
 Function:    loadCanvas
 Description: Start Canvas pattern, special treatment
 Input:       psInd: index to the particle system
 Return:      VOID
 Remarks:
***********************************************************************************************/
LOCAL VOID loadCanvas(UINT16 psInd)
{
    UINT16 pCnt, i;

    pCnt = SimPS[ psInd ].psBase.maxCnt;

    /* Step 1 -- Create all necessary particles */
    /* Create the first particle first */
    createParticles( psInd, 1 );

    /* Create subsequent particles */
    if( pCnt > 1 )
        createParticles( psInd, pCnt - 1 );

    /* Step 2 -- Revise particle locations */
    for( i = 0; i < pCnt; i++ )
    {
        if( SimP[ i ].systemID == SimPS[ psInd ].psBase.id )
        {
            /* Revise locations and aAmplitude, for color indexing */
            SimP[ i ].aAmplitude = i;
            SimP[ i ].location   = simGridData[ i ].vector;
        }
    }
}
#endif

/**********************************************************************************************
 Function:    loadBar
 Description: Start Bar pattern, special treatment
 Input:       psInd: index to the particle system
 Return:      VOID
 Remarks:
***********************************************************************************************/
LOCAL VOID loadBar(UINT16 psInd)
{
    UINT16 pCnt, i;
    CONST UINT8 pSeq[] = { 1, 2, 3, 2, 4, 5, 4, 7, 8, 7, 6 };   /* Sync with check conditions */

    pCnt = SimPS[ psInd ].psBase.maxCnt;

    /* Step 1 -- Create all necessary particles */
    createParticles( psInd, pCnt );

    /* Step 2 -- Revise particle locations */
    for( i = 0; i < pCnt; i++ )
    {
        if( SimP[ i ].systemID == SimPS[ psInd ].psBase.id )
        {
            /* Revise locations and aAmplitude, for color indexing */
            SimP[ i ].mass = pSeq[ i ];
            SimP[ i ].location = simGridData[ i ].vector;
        }
    }
}

/**********************************************************************************************
 Function:    loadFixedLoc
 Description: Start Wave pattern, Fountain pattern, special treatment
 Input:       start: particle system index
              num:  the particle system number
 Return:      VOID
 Remarks:
***********************************************************************************************/
LOCAL VOID loadFixedLoc( UINT8 start, UINT16 num )
{
    UINT16 psInd, pCnt, i, j, first, angle;
    
    for( psInd = start; psInd - start < num; psInd++ )
    {
        if( SimPS[ psInd ].psBase.id != psInvalid )
        {
            j = 0;
            pCnt = SimPS[ psInd ].psBase.maxCnt;

            /* Step 1 -- Create all necessary particles */
            createParticles( psInd, 1 );
            createParticles( psInd, ( pCnt - 1 ) );

            /* Step 2 -- Revise particle locations */
            for( i = 0; i < SimPMax; i++ )
            {
                if( SimP[ i ].systemID == SimPS[ psInd ].psBase.id )
                {
                    if( j == 0 )
                        first = i;
                    if( SimPS[ psInd ].psBase.seqMax > 1 )
                        /* Step 2.1 -- Change every valid particle location according to its sequence no. */
                        SimP[ i ].location.x = SimP[ first ].location.x - ( SimP[ i ].seq - 1 ) * REN_FRAME2VIR_FRAME;
                    else
                    {    
                        angle = SimP[ first ].angle - j * SimP[ first ].aVelocity;

                        while( angle < 0 )
                            angle += 180;
                        
                        SimP[ i ].angle      = angle;
                        SimP[ i ].location.x = SimP[ first ].location.x + j * REN_FRAME2VIR_FRAME;
                        SimP[ i ].aVelocity  = SimP[ first ].aVelocity;
                    }
                    j++;
                    if( j == pCnt )
                        break;
                }
            }
        }
     }
}

#if 0
/**********************************************************************************************
 Function:    loadFountain
 Description: Start Fountain pattern, special treatment for EQ effect
 Input:       psInd: index to the particle system
                 num: the number of particle systems to load 
 Return:      VOID
 Remarks:
***********************************************************************************************/
LOCAL VOID loadFountain( UINT16 psInd,UINT16 num )
{
    INT16 loc;
    PARTICLE_ST *pPtr;
    UINT16 pCnt, i, seqCnt, j;
    CONST UINT8 pMass[] = { 1, 2, 3 };   /* Sync with check conditions */
    i = 0;
    
    for(;num > 0; psInd++,num-- )
    {
        pCnt = SimPS[ psInd ].psBase.maxCnt;
        seqCnt = ( UINT16 )SimPS[ psInd ].psBase.maxSteerMin;
        loc = SimPS[ psInd ].psBase.locMin.scalar;
        /* Step 1 -- Create all necessary sequence head */
        createParticles( psInd, seqCnt );
        /* Step 1.1 -- Create rest particles */
        createParticles( psInd, ( pCnt - seqCnt ) );
        
        j = 0;

        /* Step 2 -- Revise particle locations and add the sequence flag that each sequence has its own mass */
        for( ; i < SimPMax; i++ )
        {  
            /* Find head for each sequence */
            if( ( SimP[ i ].systemID == SimPS[ psInd ].psBase.id ) && ( SimP[ i ].seq == 1 ) )
            {
                pPtr = &SimP[ i ];                

                /* Revise location and reset mass to distinguish differnt sequences */
                do
                {                    
                    pPtr -> location = simGridData[ loc ].vector;
                    pPtr -> mass = pMass[ j ];
                    loc++;
                    pPtr = pPtr -> nextPPtr;
                }while( pPtr != ( PARTICLE_ST* )NULL );
                j++;

                if( j == seqCnt )
                    break;
            }            
        }
    }
}
#endif

LOCAL VOID loadRave( void )
{
  if(APP_COLOR != ThemeColorMode)
  {
      tRave_Mode_Info.tRave_Color_buf.r = 255;
      tRave_Mode_Info.tRave_Color_buf.g = 0;
      tRave_Mode_Info.tRave_Color_buf.b = 0;

      //tRave_Mode_Info.bRave_Low_to_Mid_Cnt = 0;
      tRave_Mode_Info.bRave_Breath_Cnt = 104;
  }
  else
  {
    tRave_Mode_Info.tRave_Color_buf = ThemeColor;
  }
  
}
LOCAL VOID loadEQJet( void )
{
  tJet_Mode_PSM_Info.bResetData = TRUE;
  if(APP_COLOR != ThemeColorMode)
  {
      tJet_Mode_PSM_Info.bJET_Color_buf.r = 0;
      tJet_Mode_PSM_Info.bJET_Color_buf.g = 0;
      tJet_Mode_PSM_Info.bJET_Color_buf.b = 255;
  }
  else
  {
    tJet_Mode_PSM_Info.bJET_Color_buf = ThemeColor;
  }
}
VOID LoadCustomizedTheme(VOID)
{
    cus.bcus_col.r = 255;
    cus.bcus_col.g = 0;
    cus.bcus_col.b = 0;

    cus.bcus_col1 = cus.bcus_col;
    cus.bcus_col2 = cus.bcus_col;
    cus.bcus_col3 = cus.bcus_col;

    cus.bcus_timer_cnt = 0;
    cus.bcus_normal_mode_flag = 0;
    cus.bcus_low_mod = 4;
    cus.bcus_mid_mod = 5;
    cus.bcus_high_mod = 6; 

    cus.cus_loc.y = -4;
    cus.cus_loc.x = 5;
    cus.cus_loc1.y = -4;
    cus.cus_loc1.x = 5;

    cus.partical1.x = 80;
    cus.partical1.y = 112;
    cus.partical2.x = 0;
    cus.partical2.y = 176;
    cus.partical3.x = 0;
    cus.partical3.y = 16;
//        cus.bcus_lifecnt1 = 0;
//        cus.bcus_lifecnt2 = 0;
//        cus.bcus_lifecnt3 = 0;
    cus.bcus_lifecnt4 = 0;
    cus.bcus_lifecnt5 = 0;
    cus.bcus_lifecnt6 = 0;
    cus.bcus_lifecnt7 = 0;
    cus.bcus_lifecnt8 = 0;
    cus.bcus_lifecnt9 = 0;
}
///**********************************************************************************************
// Function:    setFireworkParticles
// Description: Set the firework  particle characteristics according to app input
// Input:       psInd: index to particle system
// Return:      VOID
// Remarks:     A special function dedicated to firework app interaction
//***********************************************************************************************/
//LOCAL VOID setFireworkParticles(UINT16 psInd)
//{
//    UINT16     life, lifeMin, lifeMax, maxCnt, i, yVelocity;
//    PVECTOR_ST loc, locMin, locMax, velMax;
//
//    /* Step 1 -- Get the location */
//    i = GMExInput[ psexiApp ].scalar;
//
//    if( i >= GM_POS_MAX )
//    {
//        /* Invalid target position, quit */
//        return;
//    }
//
//    /* Step 2 -- Store the system's meta that will be changed */
//    maxCnt  = SimPS[ psInd ].psBase.maxCnt;
//    lifeMin = SimPS[ psInd ].psBase.lifeMin;
//    lifeMax = SimPS[ psInd ].psBase.lifeMax;
//    locMax  = SimPS[ psInd ].psBase.locMax.vector;
//    locMin  = SimPS[ psInd ].psBase.locMin.vector;
//    velMax  = SimPS[ psInd ].psBase.velMax.vector;
//
//    yVelocity = SimPS[ psInd ].psBase.velMin.vector.y;
//
//    /* Step 3 -- Find the particle's location and result the created location and life */
//    loc.x = ( REN_PHY_FRAME_W - 1 - i % REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;
//    loc.y = locMin.y;
//    SimPS[ psInd ].psBase.locMin.vector = loc;
//    SimPS[ psInd ].psBase.locMax.vector = loc;
//
//    life = ( ( ( REN_PHY_FRAME_H - i / REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72 ) - loc.y ) / yVelocity ;
//    SimPS[ psInd ].psBase.lifeMin = life;
//    SimPS[ psInd ].psBase.lifeMax = life;
//
//    SimPS[ psInd ].psBase.velMax = SimPS[ psInd ].psBase.velMin;
//
//    /* Step 4 -- Create the particle */
//    if( ( SimPS[ psInd ].psBase.currentCnt + SimPS[ psInd+1 ].psBase.currentCnt ) >= maxCnt )
//    {
//        /* Increase the max count to accomodate the added particle */
//        SimPS[ psInd ].psBase.maxCnt = SimPS[ psInd ].psBase.currentCnt + SimPS[ psInd+1 ].psBase.currentCnt + 1;
//    }
//
//     createParticles( psInd, 1 );
//
//     /* Step 5 -- Find the particle and advance the state */
//     for( i = 0; i < SimPMax; i++ )
//     {
//         if( SimP[ i ].systemID == SimPS[ psInd ].psBase.id )
//         {
//             if( ( SimP[ i ].lifespan == life ) && ( ( SimP[ i ].location.x == loc.x ) && ( SimP[ i ].location.y == loc.y ) ) )
//             {
//                 SimP[ i ].state += 1;
//                 break;
//             }
//         }
//     }
//
//     /* Step 6 -- Restore the system parameters */
//     SimPS[ psInd ].psBase.maxCnt  = maxCnt;
//     SimPS[ psInd ].psBase.lifeMin = lifeMin;
//     SimPS[ psInd ].psBase.lifeMax = lifeMax;
//     SimPS[ psInd ].psBase.locMin.vector = locMin;
//     SimPS[ psInd ].psBase.locMax.vector = locMax;
//     SimPS[ psInd ].psBase.velMax.vector = velMax;
//}

/**********************************************************************************************
 Function:    createSpecialFireworks
 Description: create special fireworks for firework shaking
 Input:       psInd: index to particle system
              exIndex: index in GMExInput[]
 Return:      VOID
 Remarks:     A special function dedicated to Firework shaking interaction
***********************************************************************************************/
LOCAL VOID createSpecialFireworks( UINT16 psInd, INT16 exIndex )
{
    PVECTOR_ST zero = { 0, 0 };
    UINT16 pInd, pNum, redNum, secondLife;

    if( GMExInput[ exIndex ].scalar == 1 )
    {
        /* Step 1 -- Create normal particles */
        SimPS[ psInd ].psBase.maxCnt        = 20;

        SimPS[ psInd ].psImage.colorMapLen  = 2 * CIND_SCALE;
        SimPS[ psInd ].psImage.colorMapInd  = 45 * CIND_SCALE;

        GMExInput[ exIndex ].scalar = firework_ExInput2;
    }
    else if( GMExInput[ exIndex ].scalar == firework_ExInput2 )
    {
        /* Step 2 -- Create special particles  */
        pNum = 0;

        /* Clear all particles in the first state so as to control the number */
        for( pInd = 0; pInd < SimPMax; pInd++ )
        {
            if( SimP[ pInd ].systemID == SimPS[ psInd ].psBase.id )
            {
                enterNextState( &SimP[ pInd ] );
            }
        }

        /*  For firework's first system use pattern maximum count */
        SimPS[ psInd ].psBase.maxCnt        = SimPMax;
        SimPS[ psInd ].psBase.createRate    = FIREWORK_SHAKE_CNT;

        /* For breath */
        SimPS[ psInd ].psImage.ucNum        = 1;
        SimPS[ psInd ].psImage.seedColorInd = SimPS[ psInd ].psImage.colorTargetInd * CIND_SCALE;
        SimPS[ psInd ].psImage.psiFlag      = ( 0 | REN_PSI_OLOCK90 | REN_PSI_COLOR_SNAP | csuFixed );

        /* Create the white particle we need  */
        createParticles( psInd, 1 );

        /* Define lifespan according to shake or app input */
        if( exIndex == psexiShake )
            secondLife = SHAKE_LIFE;
        else
            secondLife = APPS_LIFE;

        /* "Create" white particle */
        for( pInd = 0; pInd < SimPMax; pInd++ )
        {
            if( SimP[ pInd ].systemID == SimPS[ psInd ].psBase.id )
            {
                SimP[ pInd ].state    = 2;
                SimP[ pInd ].velocity = zero;
                SimP[ pInd ].lifespan = secondLife;
                SimP[ pInd ].size     = 240;
                SimP[ pInd ].location = simGridData[ SPECIAL_LOC_INDEX ].vector;

                break;
            }
        }

        /* Change the index point to red */
        SimPS[ psInd ].psImage.seedColorInd = 46 * CIND_SCALE;
        
        /* Create the number of particles we need  */
        createParticles( psInd, ( FIREWORK_SHAKE_CNT - 1 ) );

        /* Because break, pInd++ is not done,so need add 1 */
        pInd++;

        /* Red particles' number */
        redNum = 0;

        /* "Create" red particles */
        for( ; pInd < SimPMax; pInd++ )
        {
            if( SimP[ pInd ].systemID == SimPS[ psInd ].psBase.id )
            {
                SimP[ pInd ].state    = 2;
                SimP[ pInd ].velocity = zero;
                SimP[ pInd ].lifespan = secondLife;
                SimP[ pInd ].location = simGridData[ redNum ].vector;

                redNum++;
                if( redNum == FIREWORK_SHAKE_CNT - 1 )
                    break;
            }
        }

        SimPS[ psInd ].psBase.effectCnt  = secondLife;
        SimPS[ psInd ].psBase.maxCnt     = FIREWORK_SHAKE_CNT;
        SimPS[ psInd ].psImage.psiFlag  |= REN_PSI_COLOR_OL;

        GMExInput[ exIndex ].scalar = firework_ExInput3;
    }
    else if( GMExInput[ exIndex ].scalar == firework_ExInput3 )
    {
        /* Step 3 -- Return to the ordinary firework */
        SimPS[ psInd ].psBase.maxCnt     = 3;
        SimPS[ psInd ].psBase.createRate = 10;
        SimPS[ psInd ].psBase.effectCnt  = 25;

        SimPS[ psInd ].psImage.ucNum        = 0;
        SimPS[ psInd ].psImage.colorMapInd  = 0;
        SimPS[ psInd ].psImage.seedColorInd = 0;
        SimPS[ psInd ].psImage.colorMapLen  = 45 * CIND_SCALE;
        SimPS[ psInd ].psImage.psiFlag     &= ~( REN_PSI_COLOR_OL | REN_PSI_CSUM );
        SimPS[ psInd ].psImage.psiFlag     |= csuIncrement;

        GMExInput[ exIndex ].scalar = GM_EX_INPUT_INVALID;
    }

    /* Step 4 -- Set effectCnt to bar all further external input */
    simEffectCnt = SimPS[ psInd ].psBase.effectCnt;
}

/**********************************************************************************************
 Function:    setTrafficTarget
 Description: Set target for seeking from Traffic app
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to Traffic app interaction
***********************************************************************************************/
LOCAL VOID setTrafficTarget(UINT16 psInd)
{
    UINT16 i;

    /* Step 1 -- Get the target location */
    i = GMExInput[ psexiApp ].scalar;

    if( i == traffic_app_end )
    {
        /* The effect is ended */
        SimPS[ psInd ].psBase.psFlag  &= ~SIM_PSB_SEEK_TARGET;
        SimPS[ psInd ].psImage.psiFlag &= ~REN_PSI_SHOW_TARGET;

        GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
        return;
    }

    if( i >= GM_POS_MAX )
    {
        /* Invalid target position, quit */
        return;
    }

    SimPS[ psInd ].psBase.target.x = ( REN_PHY_FRAME_W - 1 - i % REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;
    SimPS[ psInd ].psBase.target.y = ( REN_PHY_FRAME_H - i / REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;

    /* Step 2 -- Set other system parameters */
    SimPS[ psInd ].psBase.psFlag |= SIM_PSB_SEEK_TARGET;
    SimPS[ psInd ].psImage.psiFlag |= REN_PSI_SHOW_TARGET;

    /* Step 3 -- Set effectCnt */
    simEffectCnt = SimPS[ psInd ].psBase.effectCnt;

    GMExInput[ psexiApp ].scalar = traffic_app_end;     /* Set an abritrary value to re-enter */
}

/**********************************************************************************************
 Function:    setMeteorParticle
 Description: Create a particle for Meteor according to app input
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to Star app interaction
***********************************************************************************************/
LOCAL VOID setMeteorParticle(UINT16 psInd)
{
    UINT16 i;
    PVECTOR_ST loc;

    /* Step 1 -- Get the meteor location */
    i = GMExInput[ psexiApp ].scalar;

    if( i >= GM_POS_MAX )
    {
        /* Invalid meteor position, quit */
        return;
    }

    loc.x = ( REN_PHY_FRAME_W - 1 - i % REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 73; /* Minus one more to align the meteor direction */
    loc.y = ( REN_PHY_FRAME_H - i / REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;

    /* Step 2 -- Make room to create the new particle */
    if( SimPS[ psInd ].psBase.maxCnt <= SimPS[ psInd ].psBase.currentCnt )
    {
        /* No room in the moment, make it */
        SimPS[ psInd ].psBase.maxCnt = SimPS[ psInd ].psBase.currentCnt + 1;
    }

    /* Step 3 -- Set the system parameters */
    SimPS[ psInd ].psBase.createRate = 1;
    SimPS[ psInd ].psBase.locMax.vector = loc;
    SimPS[ psInd ].psBase.locMin.vector = loc;
    SimPS[ psInd ].psBase.randSnap &= ~SIM_PSB_RS_LOC;

    /* Step 3 -- Create the particle */
    createParticles( psInd, 1 );

    /* Step 4 -- Shut down the particle creation */
    SimPS[ psInd ].psBase.createRate = 0;
    SimPS[ psInd ].psBase.locMax.scalar = METEOR_MAX_LOC;
    SimPS[ psInd ].psBase.locMin.scalar = METEOR_MIN_LOC;
    SimPS[ psInd ].psBase.randSnap |= SIM_PSB_RS_LOC;
}

/**********************************************************************************************
 Function:    setStarAndMeteorShowerParticleSystem
 Description: Disable star display and show meteor shower
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to Star shake interaction
***********************************************************************************************/
LOCAL VOID setStarAndMeteorShowerParticleSystem(UINT16 psInd)
{
    UINT16 i;

    if( GMExInput[ psexiShake ].scalar == 1 )
    {
        /* Step 1 -- Start a meteor shower */
        SimPS[ psInd ].psBase.createRate = 1;
        SimPS[ psInd ].psBase.psFlag |= SIM_PSB_SHOW;

        for( i = 0; i < SIM_PS_NUM; i++ )
        {
            if( SimPS[ i ].psBase.id == psExplosion )
            {
                /* Disable star */
                SimPS[ i ].psBase.psFlag &= ~SIM_PSB_SHOW;
            }
        }
    }
    else if( GMExInput[ psexiShake ].scalar == 0 )
    {
        /* Step 2 -- End the meteor shower */
        SimPS[ psInd ].psBase.createRate = 0;
        SimPS[ psInd ].psBase.psFlag &= ~SIM_PSB_SHOW;

        for( i = 0; i < SIM_PS_NUM; i++ )
        {
            if( SimPS[ i ].psBase.id == psExplosion )
            {
                /* Enable star */
                SimPS[ i ].psBase.psFlag |= SIM_PSB_SHOW;
            }
        }
    }
}

LOCAL VOID setWaveLevel(VOID)
{
    if( GMExInput[ psexiApp ].scalar == 0 )
    {
        SimPS[ 0 ].psBase.locMin.vector.y = 72;
        SimPS[ 1 ].psBase.locMin.vector.y = 24;
        SimPS[ 2 ].psBase.locMin.vector.y = -8;
        SimPS[ 3 ].psBase.locMin.vector.y = -40;
    }
    
    if( GMExInput[ psexiApp ].scalar == 1 )
    {
        SimPS[ 0 ].psBase.locMin.vector.y = -60;
        SimPS[ 1 ].psBase.locMin.vector.y = 72;
        SimPS[ 2 ].psBase.locMin.vector.y = 24;
        SimPS[ 3 ].psBase.locMin.vector.y = -24;
    }

    if( GMExInput[ psexiApp ].scalar == 2 )
    {
        SimPS[ 0 ].psBase.locMin.vector.y = -60;
        SimPS[ 1 ].psBase.locMin.vector.y = -60;
        SimPS[ 2 ].psBase.locMin.vector.y = 72;
        SimPS[ 3 ].psBase.locMin.vector.y = -8;
    }     
}

/**********************************************************************************************
 Function:    setWaveShake
 Description: Set parameters for wave shake
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to wave shake interaction
***********************************************************************************************/
LOCAL VOID setWaveShake(VOID)
{
    UINT16 psInd;

    /* Step 1 -- Set all particle systems */
    if( GMExInput[ psexiShake ].scalar >= general_shake_set )
    {
        for( psInd = 0; psInd < SIM_PS_NUM; psInd++ )
        {
            if( SimPS[ psInd ].psBase.id != psInvalid )
                SimPS[ psInd ].psBase.timeMul = 2;

            if( SimPS[ psInd ].psBase.id == psWaveHighPitch )
                SimPS[ psInd ].psBase.locMin.vector.y = -60;
            else if( SimPS[ psInd ].psBase.id == psWaveBass )
                SimPS[ psInd ].psBase.locMin.vector.y = -60;
        }
    }
    
    /* Step 2 -- Reset shake effect */
    else if( GMExInput[ psexiShake ].scalar == general_shake_reset )
    {
        for( psInd = 0; psInd < SIM_PS_NUM; psInd++ )
        {
            if( SimPS[ psInd ].psBase.id != psInvalid )
                SimPS[ psInd ].psBase.timeMul = 1;

            if( SimPS[ psInd ].psBase.id == psWaveHighPitch )
                SimPS[ psInd ].psBase.locMin.vector.y = 72;
            else if( SimPS[ psInd ].psBase.id == psWaveBass )
                SimPS[ psInd ].psBase.locMin.vector.y = 24;
        }
    }
}

/**********************************************************************************************
 Function:    setFireflyParticles
 Description: Set Firefly particle system parameters according to app input
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to Firefly app interaction
***********************************************************************************************/
LOCAL VOID setFireflyParticles(UINT16 psInd)
{
    UINT16     tInd, pInd;
    PVECTOR_ST loc;

    /* Step 1 -- Handle effect end cases first */
    if( GMExInput[ psexiApp ].scalar == firefly_app3 )
    {
        /* All particles are at targets, let them die down */
        for( pInd = 0; pInd < SimPS[ psInd ].psBase.currentCnt; pInd++ )
            SimP[ pInd ].lifespan = 35;

        /* Effect end, restore the system */
        SimPS[ psInd ].psBase.maxCnt     = 24;
        SimPS[ psInd ].psBase.createRate = 4;
        SimPS[ psInd ].psBase.lifeMin    = 35;
        SimPS[ psInd ].psBase.lifeMax    = 45;

        simExEnableFlag = GM_EX_INPUT_ENABLE;
        GMExInput[ psexiApp ].scalar = GM_EX_INPUT_INVALID;
        return;
    }
    else if( GMExInput[ psexiApp ].scalar == firefly_app2 )
    {
        /* Check if all particles reach targets */
        for( pInd = 0; pInd < SimPS[ psInd ].psBase.currentCnt; pInd++ )
        {
            if( SimP[ pInd ].state == ITARGET_SEEK )
                break;
        }
        
        if( pInd < SimPS[ psInd ].psBase.currentCnt )
        {
            /* Still with particles not reach target, renew effectCnt and keep waiting */
            simEffectCnt = 20;      /* Give 20 more steps */
        }
        else
        {
            /* All particles reached target, go to next stage */
            GMExInput[ psexiApp ].scalar = firefly_app3;
        }
        return;
    }

    /* Handle the effect from start */
    /* Step 2 -- Stop creating new particles */
    if( SimPS[ psInd ].psBase.currentCnt > 0 )
    {
        /* Stop create anymore */
//        SimPS[ psInd ].psBase.createRate = 0;  
//        return;
        for( pInd = 0; pInd < SimPMax; pInd++ )
            if( SimP[ pInd ].systemID != SIM_PARTICLE_INVALID )
            {
                killOneParticle( &SimP[ pInd ] );
                if( SimPS[ psInd ].psBase.currentCnt == 0 )
                    break;
            }

        /* Disable other external inputs */
        simExEnableFlag = 40;//( 1 << psexiApp );        /* Only leave app input */
    }

    /* Step 3 -- Prepare the system */
    SimPS[ psInd ].psBase.maxCnt     = GM_POS_MAX * SimPS[ psInd ].psBase.seqMax;
    SimPS[ psInd ].psBase.createRate = 1;
    SimPS[ psInd ].psBase.lifeMin    = SIM_PARTICLE_INFINITE;     /* Guarantee enough time */
    SimPS[ psInd ].psBase.lifeMax    = SIM_PARTICLE_INFINITE;

    /* Step 4 -- Create particles */
    for( tInd = 0, pInd = 0; tInd < GM_POS_MAX; tInd++ )
    {
        if( GMPosInfo[ tInd ] != GM_POS_INVALID )
        {
            loc.x = ( REN_PHY_FRAME_W - 1 - tInd % REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;
            loc.y = ( REN_PHY_FRAME_H - tInd / REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;

            /* Create a particle for the position */
            createParticles( psInd, 1 );                /* Create the first sequence particle */

            /* Adjust the velocity so as to point to the target */
            if( SimP[ pInd ].location.x > loc.x )
                SimP[ pInd ].velocity.x = -2;
            else if( SimP[ pInd ].location.x == loc.x )
                SimP[ pInd ].velocity.x = 0;
            else
                SimP[ pInd ].velocity.x = 2;

            if( SimP[ pInd ].location.y > loc.y )
                SimP[ pInd ].velocity.y = -2;
            else if( SimP[ pInd ].location.y == loc.y )
                SimP[ pInd ].velocity.y = 0;
            else
                SimP[ pInd ].velocity.y = 2;

            if( SimPS[ psInd ].psBase.seqMax > 1 )      /* Create sequence subsequent particles, if any */
                createParticles( psInd, SimPS[ psInd ].psBase.seqMax - 1 );

                                                        /* Register the target location */
            simITarget[ pInd / SimPS[ psInd ].psBase.seqMax ] = tInd;
            SimP[ pInd ].state = ITARGET_SEEK;          /* State indiciating individual target seeking */

            pInd += SimPS[ psInd ].psBase.seqMax;

            GMPosInfo[ tInd ] = GM_POS_INVALID;         /* Remove the location */

        }
    }

    /* Step 5 -- Set effectCnt to bar all further external input */
    simEffectCnt = SimPS[ psInd ].psBase.effectCnt;

    SimPS[ psInd ].psBase.createRate = 0;               /* Prevent further particle creation */

    /* Step 6 -- Transit to next stage */
    GMExInput[ psexiApp ].scalar = firefly_app2;
}

/**********************************************************************************************
 Function:    setFireLevel for use with EQ Action
 Description: Set particle system for Fire according to EQ Low Freq values
 Input:       psInd:     index to particle system
              fireLevel: fire level: 0 - 2: small, mid and big
 Return:      VOID
 Remarks:     A special function dedicated to Fire app interaction
***********************************************************************************************/

VOID setEQFireLevel(UINT16 psInd, UINT16 fireLevel)
{
    switch( fireLevel )
    {
        case 0:
            /* Small fire */
            SimPS[ psInd ].psBase.velMax.scalar = 35;
            SimPS[ psInd ].psBase.velMin.scalar = 35;
            SimPS[ psInd ].psBase.lifeMax = 22;//33;
            SimPS[ psInd ].psBase.lifeMin = 11;//22;

            simExCheck[ 0 ].value.scalar  = -40;
            break;

        case 2:
            /* Big fire */
            SimPS[ psInd ].psBase.velMax.scalar = 34;
            SimPS[ psInd ].psBase.velMin.scalar = 34;
            SimPS[ psInd ].psBase.lifeMax = 125;//55;
            SimPS[ psInd ].psBase.lifeMin = 45;
            SimPS[ psInd ].psBase.growMax = 4;
            SimPS[ psInd ].psBase.growMin = 1;
            
            simExCheck[ 0 ].value.scalar  = -10;
            break;

        case 1:
        default:
            /* Medium fire */
            SimPS[ psInd ].psBase.velMax.scalar = 35;
            SimPS[ psInd ].psBase.velMin.scalar = 35;
            SimPS[ psInd ].psBase.lifeMax = 45;
            SimPS[ psInd ].psBase.lifeMin = 35;

            simExCheck[ 0 ].value.scalar  = -20;
            break;
    }
}

/**********************************************************************************************
 Function:    setFireLevel
 Description: Set particle system for Fire according to app input
 Input:       psInd:     index to particle system
              fireLevel: fire level: 0 - 2: small, mid and big
 Return:      VOID
 Remarks:     A special function dedicated to Fire app interaction
***********************************************************************************************/

LOCAL VOID setFireLevel(UINT16 psInd, UINT16 fireLevel)
{
    switch( fireLevel )
    {
        case 0:
            /* Small fire */
            SimPS[ psInd ].psBase.velMax.scalar = 35;
            SimPS[ psInd ].psBase.velMin.scalar = 35;
            SimPS[ psInd ].psBase.lifeMax = 33;
            SimPS[ psInd ].psBase.lifeMin = 22;

            simExCheck[ 0 ].value.scalar  = -30;
            break;

        case 2:
            /* Big fire */
            SimPS[ psInd ].psBase.velMax.scalar = 34;
            SimPS[ psInd ].psBase.velMin.scalar = 34;
            SimPS[ psInd ].psBase.lifeMax = 55;
            SimPS[ psInd ].psBase.lifeMin = 45;
            SimPS[ psInd ].psBase.maxCnt  = 96;
            //SimPS[ psInd ].psBase.timeMul = 2;

            simExCheck[ 0 ].value.scalar  = -10;
            break;

        case 1:
        default:
            /* Medium fire */
            SimPS[ psInd ].psBase.velMax.scalar = 35;
            SimPS[ psInd ].psBase.velMin.scalar = 35;
            SimPS[ psInd ].psBase.lifeMax = 45;
            SimPS[ psInd ].psBase.lifeMin = 5;
            SimPS[ psInd ].psBase.maxCnt  = 96;
            //SimPS[ psInd ].psBase.timeMul = 1;

            simExCheck[ 0 ].value.scalar  = -20;
            break;
    }
}

/**********************************************************************************************
 Function:    setCanvasParticles
 Description: Create particles for Canvas according to app input
 Input:       psInd: index to particle system
 Return:      VOID
 Remarks:     A special function dedicated to Canvas app interaction
***********************************************************************************************/
LOCAL VOID setCanvasParticles(UINT16 psInd)
{
    UINT16 i, pInd;
    PARTICLE_ST *pPtr;

    /* Step 1 -- Set particle system parameters */
    SimPS[ psInd ].psBase.maxCnt = GM_POS_MAX + 9;      /* 9 particles are for the captured color display */
    SimPS[ psInd ].psBase.createRate = 1;
    SimPS[ psInd ].psBase.aAmpMax = 10;     /* Canvas use aAmplitude to assign color, index to 10 is black */
    SimPS[ psInd ].psBase.aAmpMin = 10;

    /* Step 2 -- Remove any previous particles */
    pPtr = &SimP[ 9 ];          /* The first 9 particles are for the canvas */
    while( pPtr < &SimP[ SIM_PARTICLE_NUM ] )
    {
        if( pPtr -> systemID == SimPS[ psInd ].psBase.id )
            killOneParticle( pPtr );
        pPtr++;
    }

    /* Step 3 -- Create the necessary particles */
    for( i = 0, pInd = 9; i < GM_POS_MAX; i++ )
    {
        if( GMPosInfo[ i ] != GM_POS_INVALID )
        {
            /* It is a valid location, create a particle for it */
            createParticles( psInd, 1 );

            /* Adjust the lifespan and location */
            SimP[ pInd ].lifespan = 250;   /* 10 sec, per 40ms time tick */
            SimP[ pInd ].size     = 0;
            SimP[ pInd ].location.x = ( REN_PHY_FRAME_W - 1 - i % REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;
            SimP[ pInd ].location.y = ( REN_PHY_FRAME_H - i / REN_PHY_FRAME_W ) * REN_VIR_DIS_SCALE - 72;

            pInd++;

            /* Remove the point */
            GMPosInfo[ i ] = GM_POS_INVALID;
        }
    }

    /* Step 4 -- Stop further create particle */
    SimPS[ psInd ].psBase.createRate = 0;
}

/**********************************************************************************************
 Function:    setHourglassParticle
 Description: Set the stack particle characteristics according to app input
 Input:       psInd:  index to particle system
              tValue: time value
 Return:      VOID
 Remarks:     A special function dedicated to Hourglass app interaction
***********************************************************************************************/
LOCAL VOID setHourglassParticle(UINT16 psInd, UINT16 tValue)
{
    UINT16 i, life, maxSpeed;
    INT16  tm;

    /* Step 1 -- Kill the current particle */
    for( i = 0; i < SimPMax; i++ )
    {
        if( SimP[ i ].systemID == SimPS[ psInd ].psBase.id )
        {
            killOneParticle( &SimP[ i ] );
            break;
        }
    }

    /* Step 2 -- Calculate the parameters */
    /* It is in second, convert it to minute */
    tm = tValue / 60;
    life  = tm * 1500;          /* 1500 time tick is equivalent to 1 min */
    maxSpeed = tm * 20;         /* 20 steps in 20ms each is equivalent to 1 min */

    tm = tValue - ( tm * 60 );
    while( tm > 0 )
    {
        life  += 375;
        maxSpeed += 5;

        tm -= 15;       /* For every 15 sec, life and maxSpeed are incremented by 375 and 5 respectively */
    }

    /* Step 2 -- Set the new system parameter */
    SimPS[ psInd ].psBase.lifeMax  = life;
    SimPS[ psInd ].psBase.lifeMin  = life;
    SimPS[ psInd ].psBase.maxSpeedMax = ( UINT8 )maxSpeed;
    SimPS[ psInd ].psBase.maxSpeedMin = ( UINT8 )maxSpeed;

    /* Step 3 -- Create a new particle */
    createParticles( psInd, 1 );
}

/**********************************************************************************************
 Function:    setBarShake
 Description: Enable or disable 2 system show 
 Input:       scalar: shake input value
 Return:      VOID
 Remarks:     A special function  for Bar shake interaction
***********************************************************************************************/
LOCAL VOID setBarShake( UINT16 scalar )
{
    if( scalar >= general_shake_set )     // shake effect acts, as other patterns shake value may > 1,so there is not using "=" but using ">="  
    {
        SimPS[ 1 ].psImage.bgFlag = bgtBlack;
        SimPS[ 1 ].psBase.psFlag |= SIM_PSB_SHOW;
        SimPS[ 2 ].psBase.psFlag |= SIM_PSB_SHOW;
        simEffectCnt = SimPS[ 1 ].psBase.effectCnt;
        return;
    }

    if( scalar == general_shake_reset )  // shake effect is gone,reset the parameters
    {
        SimPS[ 1 ].psImage.bgFlag = bgtInvalid;
        SimPS[ 1 ].psBase.psFlag &= ~SIM_PSB_SHOW;
        SimPS[ 2 ].psBase.psFlag &= ~SIM_PSB_SHOW;
        return;
    }    
}

/**********************************************************************************************
 Function:    copySymmetryLocation
 Description: copy one particle location to another particle
 Input:       pPtr: pointor to sequence head particle
 Return:      VOID
 Remarks:     sequence consistute symmetry image
***********************************************************************************************/
LOCAL VOID copySymmetryLocation( PARTICLE_ST *pPtr, UINT8 maxSeq )   
{
    UINT16 copyFirst, i;
    PARTICLE_ST *nextPtr, *symmPtr;

    /* Get the seq-number gap between the head particle and the first copy one in sequence */
    copyFirst = ( maxSeq >> 1 ) + 1 + 1;

    nextPtr = pPtr;
    symmPtr = pPtr;
    /* Copy the location, just for Y direction now */
    for( i = 2; copyFirst <= maxSeq; i++, copyFirst++ )
    {
        while( nextPtr -> seq != copyFirst )
            nextPtr = nextPtr -> nextPPtr;

        while( symmPtr -> seq != i )
            symmPtr = symmPtr -> nextPPtr;

        nextPtr -> location.y = symmPtr -> location.y;
//        nextPtr -> velocity   = symmPtr -> velocity;      // Just for every particle after head in sequence has a tail
//        nextPtr -> location.x = pPtr -> location.x + ( pPtr -> location.x - symmPtr -> location.x );
    }
}

/**********************************************************************************************
 Function:    updateFountainLocation
 Description: update the locations of particles in sequence according inputScalar
 Input:       pPtr: pointor to sequence head particle
              inputScalar: external input value
              aScalar:  simAction[ aInd ].value.scalar
              psInd:  particle system index
 Return:      VOID
 Remarks:     special treat for Fountain EQ input
***********************************************************************************************/
LOCAL VOID updateFountainLocation( PARTICLE_ST *pPtr, INT16 inputScalar, INT16 aScalar, UINT8 psInd ) 
{
    UINT16 maxSeq;
    INT16 targetY;
    PARTICLE_ST* endPtr;

    /* Find the end one and get its sequence */
    endPtr = pPtr;
    if( endPtr != ( PARTICLE_ST* )NULL )
    {
        targetY = inputScalar / aScalar * REN_VIR_DIS_SCALE + LOWEST_LINE;    // -56 is the center of LED at lowest line 
        if( targetY > HIGHEST_LED_CENTER )
            targetY = HIGHEST_LED_CENTER;
        
        if( pPtr -> location.y + REN_VIR_DIS_SCALE > targetY )
            return;
        else
            while( endPtr -> nextPPtr != ( PARTICLE_ST* )NULL )
            {
                endPtr -> location.y = targetY;
                if( targetY > LOWEST_LED_CENTER )
                    targetY -= REN_VIR_DIS_SCALE;                
                if( targetY < LOWEST_LED_CENTER )  
                    targetY = LOWEST_LED_CENTER;
                endPtr = endPtr -> nextPPtr;
            }           
    }
    maxSeq = endPtr -> seq;

    if( maxSeq < 3 )
        return;
    copySymmetryLocation( pPtr, ( UINT8 )maxSeq );
}

LOCAL VOID UpdateThemeColorStatus(VOID)
{   
    if(ThemeColorMode != GMThemeColorMode || 
       (ThemeColor.r != GMThemeColor.r || ThemeColor.g != GMThemeColor.g || ThemeColor.b != GMThemeColor.b))
    {//update theme color and color mode
        ThemeColorMode  = GMThemeColorMode;
        ThemeColor      = GMThemeColor;
    }
}

#if 0
UINT8 DetectLowMidHigh(U)
{
    UINT8 index = 0;

    if(g_lowAvgEQ >= sp_AnaMap[13].dbVal)
    {
       index |= 1;  //low 
    }
  
    if(g_midAvgEQ >= sp_AnaMap[13].dbVal)
    {
       index |= 2; //mid
    }
    
    if(g_highAvgEQ >= sp_AnaMap[12].dbVal)
    {
       index |= 4;  //hight
    }

    if(GMExInput[psexi2000Hz].scalar > sp_AnaMap[12].dbVal)
    {
      index |= 4;  //hight  
    }


    if(index == 0) 
    {
       index = cus.detect_mode;
    }

   return index;
}
#endif

#if 0
UINT8 JetDetectLowMidHigh(U)
{
    UINT8 index = 0;

    if(g_lowAvgEQ >= sp_AnaMap[13].dbVal)
    {
       index |= 1;  //low 
    }
  
    if(g_midAvgEQ >= sp_AnaMap[14].dbVal)
    {
       index |= 2; //mid
    }
    
    if(g_highAvgEQ >= sp_AnaMap[14].dbVal)
    {
       index |= 4;  //hight
    }

   return index;
}
#endif

#if 0
UINT8 RaveDetectLowMidHigh(U)
{
    UINT8 index   = 0;

    if(g_lowAvgEQ >= sp_AnaMap[8].dbVal)
    {
       index |= 1;  //low 
    }
  
    if(g_midAvgEQ >= sp_AnaMap[13].dbVal)
    {
       index |= 2; //mid
    }
    
    if(g_highAvgEQ >= sp_AnaMap[13].dbVal)
    {
       index |= 4;  //hight
    }

    if(GMExInput[psexi2000Hz].scalar > sp_AnaMap[13].dbVal)
    {
      index |= 4;  //hight  
    }


    if(index == 0) 
    {
       index = tRave_Mode_Info.bRave_EXflag;
    }

   return index;
}
#endif

PATTERN_REC_ST *GetPatMeta_Addr(VOID)
{
    return (PATTERN_REC_ST *)(&patMeta[0]);
}

PS_BASE_T *GetPatPsb_Addr(VOID)
{
    return (PS_BASE_T *)(&patPsb[0]);
}

PS_IMAGE_INFO_T *GetPatPsi_Addr(VOID)
{
    return (PS_IMAGE_INFO_T *)(&patPsi[0]);
}

SIM_ACTION_T *GetPatExAction_Addr()
{
    return (SIM_ACTION_T *)(&patExAction[0]);
}

SIM_OPERATION_T *GetPatExOp_Addr()
{
    return (SIM_OPERATION_T *)(&patExOp[0]);
}
