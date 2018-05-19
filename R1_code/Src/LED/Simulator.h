/***************************************************************************************************************************
* Copyright (C)  TaiSing Industrial Company Ltd.(Shenzhen).
* Project Name:  Pulse II
* File name:     Simulator.h
* Description:   Particle system simulator   

* Author: Terry Li    	                         Date: 2015-March-01
***************************************************************************************************************************/

/**************************** Edit History *********************************************************************************
* DATE              NAME                 DESCRIPTION
* 2015-Mar-01       Terry Li             Created
* 2015-Mar-22       LMCheong             Modified
* 2015-Mar-27       Terry Li             Add the section about that particle systems to enter 2nd state
* 2015-Jul-25       James Lin            Make this revision for MP releae any other change must add new revision history
***************************************************************************************************************************/

#ifndef SIMULATOR_H
#define SIMULATOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "Rendering.h"

/**************************************************Type declarations ******************************************/
#define SIM_TIMETICK            5      /* Time interval between two simulations = TimerSys x SIM_TIMETICK  */
#define SIM_PS_NUM              4      /* Total no. of particle system in Simulator                        */
#define SIM_PS_EX_EFFECT_NUM    16     /* Total no. of external effects                                    */
#define SIM_PS_EX_OP_NUM        30     /* Total no. of external operations                                 */
#define SIM_PS_EX_CHECK_NUM     10     /* Total no. of external operation checkings                        */
#define SIM_PS_EX_ACTION_NUM    32     /* Total no. of external operation actions                          */
#define SIM_PS_GRID_DATA_NUM    40     /* Total no. of grid data                                           */

#define SIM_PARTICLE_NUM        108    /* Total no. of particle in the simulator                           */
#define SIM_PARTICLE_INVALID    0xFF   /* Particle invalid flag                                            */
#define SIM_PARTICLE_INFINITE   0xFFFF /* Indicate the particle will live forever                          */
#define SIM_TEXTURE_INVALID     0xFF   /* Texture invalid flag                                             */
#define SIM_PS_EX_INVALID       0xFF   /* External effect invalid flag                                     */

extern const UINT16 EXPLOSION_HUE_FACTOR;
extern const UINT16 EXPLOSION_MAX_HUE;

/* Particle system id                 */
typedef enum PARTICLE_SYSTEM_ID
{
    psFireworkRaiseUp     = 0,
    psFireworkExplode     = 1,
//    psFireworkHeart       = 2,
    psTraffic             = 2,
    psExplosion           = 3,
    psMeteor              = 4,
    psMeteorShower        = 5,
    psWaveHighPitch       = 6,
//    psWaveAlto            = 8,
    psWaveBass            = 7,
    psWaveBasic2          = 8,
    psWaveBasic1          = 9,
    psFirefly             = 10,
    psCustomized          = 11,
    psCustomizedBounce    = 12,
    psLightning           = 13,
    psFire                = 14,
    psRainbow             = 15,
    psHourglass           = 16,
    psHourglassStack      = 17,
    psHourglassStackBlink = 18,
    psBar                 = 19,
    psRave                = 20, //whan add
    psJet                 = 21,
            
    psInvalid          = 0xFF
}PARTICLE_SYSTEM_ID_E;

/* External effect */
/* Check type */
typedef enum SIM_CHECK_TYPE
{
    sctEqual      = 0,              /* = */
    sctNotEqual   = 1,              /* != */
    sctGreater    = 2,              /* > */
    sctLess       = 3,              /* < */
    sctEqualDir   = 4               /* Equal direction, for vector only */
}SIM_CHECK_TYPE_E;

/* Check structure */
typedef struct SIM_CHECK
{
    GEN_PARA_T value;               /* Value for comparison */
    UINT8      pID;                 /* The parameter the checking operates on */
    UINT8      checkType;           /* The type of checking */
}SIM_CHECK_T;

/* Action type */
typedef enum SIM_ACTION_TYPE
{
    satAbsValAbsLimit    = 0x00,    /* Assign value to the parameter for limit instances */
    satProValAbsLimit    = 0x01,    /* Assign (value * input) to the parameter for limit instances */
    satRelValAbsLimit    = 0x02,    /* Add value to the parameter for limit instances */
    satRelProValAbsLimit = 0x03,    /* Add (value * input) to the parameter for limit instances */
    satAbsDirAbsLimit    = 0x04,    /* Set value's direction to the parameter for limit instances, for vector only */
    satDivValAbsLimit    = 0x05,    /* Set (input / value) to the parameter for limit instances */

    satAbsValProLimit    = 0x10,    /* Assign value to the parameter for (limit * input) instances */
    satProValProLimit    = 0x11,    /* Assign (value * input) to the parameter for (limit * input) instances */
    satRelValProLimit    = 0x12,    /* Add value to the parameter for (limit * input) instances */
    satRelProValProLimit = 0x13,    /* Add (value * input) to the parameter for (limit * input) instances */
    satAbsDirProLimit    = 0x14,    /* Set value's direction to the parameter for (limit * input) instances, for vector only */
    satDivValProLimit    = 0x15,    /* Set (input / value) to the parameter for (limit * input) instances */
    satExchValLimit      = 0x16,    /* Exchange vector' 2 values of two direction for (vector) instances */

    satSetFlag           = 0x21,    /* OR value to the parameter */
    satResetFlag         = 0x22,    /* AND ~value to the parameter */

    satAbsValGrid        = 0x40,    /* Assign value to simGridData[limit] which is associated with pID */
    satProValGrid        = 0x41,    /* Assign (value * input) to simGridData[limit] which is associated with pID */
    satRelValGrid        = 0x42,    /* Add value to simGridData[limit] which is associated with pID */
    satRelProValGrid     = 0x43,    /* Add (value * input) to simGridData[limit] which is associated with pID */

    satFireworkApp       = 0xE0,    /* Specific action dedicated to traffic app interaction */
    satTrafficApp        = 0xE1,    /* Specific action dedicated to traffic app interaction */
    satStarApp           = 0xE2,    /* Specific action dedicated to star app interaction */
    satWaveApp           = 0xE3,    /* Specific action dedicated to wave app interaction */
    satFireflyApp        = 0xE4,    /* Specific action dedicated to firefly app interaction */
    setCustomizedApp           = 0xE5,    /* Specific action dedicated to Customized app interaction */
    satFireApp           = 0xE6,    /* Specific action dedicated to fire app interaction */
    satCanvasApp         = 0xE7,    /* Specific action dedicated to canvas app interaction */
    satHourglassApp      = 0xE8,    /* Specific action dedicated to hourglas app interaction */
    satCanvasColorPick   = 0xE9,    /* Specific action dedicated to canvas color pick interaction */
    satFireworkShake     = 0xEA,    /* Specific action dedicated to firework shake interaction */
    satMeteorShowerShake = 0xEB,    /* Specific action dedicated to meteor shower shake interaction */
    satWaveShake         = 0xEC,    /* Specific action dedicated to wave pattern shake interaction */
    satCanvasShake       = 0xED,    /* Specific action dedicated to canvas shake interaction */
    satBarShake          = 0xEE,
    satExInput           = 0xEF,

    satInvalidAction     = 0xFF
}SIM_ACTION_TYPE_E;

#define SIM_ACTION_LIMIT_MASK   0x10
#define SIM_ACTION_PRO_LIMIT    0x10
#define SIM_ACTION_ABS_LIMIT    0x00
#define SIM_ACTION_GRID_MASK    0x40

/* Action structure */
typedef struct SIM_ACTION
{
    GEN_PARA_T value;               /* Value set to para */
    UINT8      actionType;          /* The type of action */
    UINT8      pID;                 /* The parameter the action operates on */
    UINT8      limit;               /* The maximum no. of operations on the parameter */
                                    /* It is valid only when operates on particles of a particle system */
                                    /* Also serve as index to simGridData[] if it is an action on grid data */

    UINT8 dummy;                    /* Dummy byte for alignment */
}SIM_ACTION_T;

/* Operation structure */
/* An operation is defined as 2 steps: */
/* 1. Complete and fulfill of a set of checking operations */
/* 2. Execute a set of actions */
typedef struct SIM_OPERATION
{
    UINT8 inputID;                  /* ID of the external input defined for the operation */
    UINT8 threshold;                /* A value check against the input parameter so as to trigger the operation */
    UINT8 elseOpInd;                /* Index to the next operation check if this check failed */

    /* Checking operations, the overall result of checking is "AND" of all individual checkings */
    UINT8 checkOffset;              /* The checking offset in the check array */
    UINT8 checkNum;                 /* No. of checking operations */

    /* Actions would be taken only when the overall checking result is TRUE */
    UINT8 actionOffset;             /* Action offset in the action array */
    UINT8 actionNum;                /* No. of actions*/
}SIM_OPERATION_T;

/* Decision type */
typedef enum SIM_DECISION_TYPE
{
    sdtMax      = 0,                /* Maximum */
    sdtMin      = 1,                /* Minimum */
    sdtAlways   = 2,                /* Always pass decision, should only assoicate with only one operation chain */

    sdtNull     = 3                 /* Null decision, means the effect is invalid */
}SIM_DECISION_TYPE_E;

/* External effect structure */
typedef struct SIM_EX_EFFECT
{
    UINT8 decisionType;             /* Type of decision for the selection of an operation */
    UINT8 opOffset;                 /* Offset of the operation for the effect in operation array */
    UINT8 opNum;                    /* No. of operation to be considered according to decision type */
}SIM_EX_EFFECT_T;

/* Pattern parameter set */
typedef struct SIM_PAT_PARA
{
    GEN_PARA_T value;               /* Value set to para */
    UINT8      pID;                 /* The parameter id in the system */
    UINT8      sInd;                /* Index of the particle system using this parameter */
}SIM_PAT_PARA_T;

/* Parameter set, one for each pattern, indexed by pattern ID */
typedef struct SIM_PAT_PARA_SET
{
    UINT8 offset;                   /* Offset of the parameter set in the set array */
    UINT8 num;                      /* No. of parameter in the parameter set */
}SIM_PAT_PARA_SET_T;

/* Force structure */
typedef struct FORCE
{
    PVECTOR_ST  location;
    UINT16      lifespan;
    INT8        field;
    INT8        fieldRate;          // Rate of change of field with respect to time, d(field)/dt            
    UINT16      frozenCnt;          // Duration of temporarily disable the force
    struct FORCE*   nextFPtr;

}FORCE_ST;

/* Wind structure */
typedef struct WIND
{
    PVECTOR_ST  direction;
    UINT16      lifespan;
    INT8        field;
    INT8        fieldRate;          // Rate of change of field with respect to time, d(field)/dt
    UINT16      frozenCnt;          // Duration of temporarily disable the wind
    struct WIND*    nextWPtr;

}WIND_ST;

typedef union FORCE_WIND
{
    FORCE_ST force;
    WIND_ST  wind;
}FORCE_WIND_ST;

/* Particle system structure */
typedef struct particleSystemBase
{
    UINT8  id;                      // ID of the particle system
    UINT8  seqMax;                  // Maximum no. of particle in a particle sequence
    UINT8  createRate;              // No. of particle can be generated in one timetick.It equals to 0 if the system only creates particles in a bunch or from any external trigger.
    UINT8  maxCnt;                  // Maximum no. of living particle
    UINT8  currentCnt;              // current no. of living particle

    UINT8  maxState;                // The highest possible state number defined for the system	
    INT8   timeMul;                 // Shift in time scale in no. of time tick for calculation.
                                    // Negative values are delay while positive advanced

    UINT8  angleMin;                // Minimum possible angle for a newly generated particle
    UINT8  angleMax;                // Maxmum possible angle for a newly generated particle
    UINT8  aVelMin;                 // Minimum possible angular velocity for a newly generated particle
    UINT8  aVelMax;                 // Maxmum possible angular velocity for a newly generated particle	
    UINT8  aAmpMin;                 // Minimum possible angular amplitude for a newly generated particle
    UINT8  aAmpMax;                 // Maxmum possible angular amplitude for a newly generated particle

    UINT8  sizeMin;                 // Minimum possible size for a newly generated particle
    UINT8  sizeMax;                 // Maxmum possible size for a newly generated particle
    UINT8  growMin;                 
    UINT8  growMax;

    UINT8  massMin;
    UINT8  massMax;

    UINT8  maxSpeedMin;
    UINT8  maxSpeedMax;
    UINT8  maxSteerMin;             // Minimum possible maximum steering force for a newly generated particle	
    UINT8  maxSteerMax;             // Maxmum possible maximum steering force for a newly generated particle

    INT8   sizeAspect;              // Aspect ratio of the particle.  > 0 is the height/width; <0 is width/height
    UINT16 separation;              // Distance for two particles must keep apart	
    UINT16 cohesion;                // Maximum distance between two particles that seeking effect is still valid	

    UINT16 lifeMin;
    UINT16 lifeMax;
    UINT16 effectCnt;               // Shake effect time, decrement for each timetick; clear target and divider when it reaches 0

    PVECTOR_ST target;              // Location of any target to be seeked by the particles in the system
    GEN_PARA_T locMin;
    GEN_PARA_T locMax;
    GEN_PARA_T velMin;
    GEN_PARA_T velMax;
    GEN_PARA_T accMin;
    GEN_PARA_T accMax;

    WIND_ST*  windPtr;
    FORCE_ST* forcePtr;

    UINT16 psFlag;                  // Enable/disable system features
    UINT16 randSnap;                // Random variable snap flags
}PS_BASE_T;

/* ps flag definitions */
#define SIM_PSB_SHOW            0x0001      /* Render the particle system or not */
//#define SIM_PSB_ALIGNMENT       0x0002      /* Enable/disable alignment */
#define SIM_PSB_SEPARATION      0x0004      /* Enable/disable separation */
#define SIM_PSB_COHESION        0x0008      /* Enable/disable cohesion */
#define SIM_PSB_RAND_CREATE     0x0010      /* Create random no. of particles */
#define SIM_PSB_PAT_MAX_CNT     0x0020      /* Maximum count including all particle system in the pattern */
#define SIM_PSB_SHM             0x0040      /* Simple harmonic motion */
#define SIM_PSB_SEEK_TARGET     0x0080      /* Seek target */
#define SIM_SEQ_LOC_Y           0x1000      /* Just copy location Y */
#define SIM_CYCLE_STATE         0x2000      /* Cycle the state */
#define SIM_NO_CUT_TAIL         0x4000      /* Don't cut particles after the head in sequence */
#define SIM_SYMMETRY_LOC        0x8000      /* Symmtry x location sequence */
#define IN_FRAME_FOREVER        0x0002      /* In the frame forever */

#define SIM_PSB_TARGET_UPD_MASK 0x0700      /* Target update method mask */
#define SIM_PSB_TARGET_UPD_FIX  0x0000      /* No target position update */
#define SIM_PSB_TARGET_UPD_MAXX 0x0100      /* Above max X among particles */
#define SIM_PSB_TARGET_UPD_MAXY 0x0200      /* Above max Y among particles */
#define SIM_PSB_TARGET_UPD_MINX 0x0300      /* Below min X among particles */
#define SIM_PSB_TARGET_UPD_MINY 0x0400      /* Below min Y among particles */

#define SIM_PSB_RAND_SEQ        0x0800      /* Random sequence in location generation, for grid loc only */

/* randSnap flag definitions */
#define SIM_PSB_RS_ANGLE        0x0001      /* Angle, angle */
#define SIM_PSB_RS_AVEL         0x0002      /* Angular velocity, aVel */
#define SIM_PSB_RS_AAMP         0x0004      /* Angular amplitude, aAmp */
#define SIM_PSB_RS_SIZE         0x0008      /* Size, size */
#define SIM_PSB_RS_GROW         0x0010      /* Grow, grow */
#define SIM_PSB_RS_MASS         0x0020      /* Mass, mass */
#define SIM_PSB_RS_MAXSPEED     0x0040      /* Maximum speed, maxSpeed */
#define SIM_PSB_RS_MAXSTEER     0x0080      /* Maximum steer, maxSteer */
#define SIM_PSB_RS_LIFESPAN     0x0100      /* Lifespan, lifespan */
#define SIM_PSB_RS_LOC          0x0200      /* Location, loc */
#define SIM_PSB_RS_VEL          0x0400      /* Velocity, vel */
#define SIM_PSB_RS_ACC          0x0800      /* Acceleration, acc */
#define SIM_PSB_RS_ALIGN_LOC    0x1000      /*rand in aligned location*/

/* Size aspect ratio definition */
#define SIM_SA_X_INFINITE       -1          /* Effectively with only height, and same for all x values */
#define SIM_SA_Y_INFINITE       0           /* Same x value for all y */

/* Complete particle system */
typedef struct PARTICLE_SYSTEM
{
    PS_BASE_T       psBase;         /* Particle system base information */
    PS_IMAGE_INFO_T psImage;        /* Image information */
    UINT8           exOffset;       /* Offset of external effects on the exEffect array */
    UINT8           exNum;          /* No. of external effects */

    UINT16          dummy;          /* Dummy byte added for alignment */
}PARTICLE_SYSTEM_ST;

/* Memory storage record for a pattern */
typedef struct PATTERN_REC
{
    /* Basic particle system and image information definitions */
    UINT8   psInd;                  /* Index of the first particle system (of the pattern) at patMeta[] */
    UINT8   psNum;                  /* No. of particle systems defined in a pattern */

    UINT8   delayCnt;               /* Time delay for the pattern */

    /* External effect definitions */
    UINT8   exInd;                  /* Index of external effect parameters at patExEffect[] */
    UINT8   exNum[ SIM_PS_NUM ];    /* No. of external effect parameters */
    UINT8   exOpInd;                /* Index of external effect operation at patExOp[] */
    UINT8   exOpNum;                /* No. of external effect operations */
    UINT8   exCheckInd;             /* Index of external operation checking at patExCheck[] */
    UINT8   exCheckNum;             /* No. of external operation checkings */
    UINT8   exActionInd;            /* Index of external operation action at patExAction[] */
    UINT8   exActionNum;            /* No. of external operation actions */

    /* Pattern color map array */
    COLOR_T patColor[ REN_COLOR_ARRAY_LEN ];

    /* Color operations */
    PS_COLOR_OP_T patColorOp[ REN_COLOR_OPERATION_NUM ];

    UINT8 pMax;                    /* Maximum no. of particle for the pattern */

    /* Grid data array */
    GEN_PARA_T patGrid[ SIM_PS_GRID_DATA_NUM ];
}PATTERN_REC_ST;

/* Particle system parameter groups */
typedef enum PS_GROUP
{
    psgP   = 0,                     /* Particle */
    psgPsb = 1,                     /* Particle system base */
    psgPsi = 2                      /* Particle system image information */
                   
}PS_GROUP_E;

/* Particle system parameter type */
typedef enum PS_PARA_TYPE
{
    pstUINT8  = 0,
    pstUINT16,
    pstINT8,
    pstINT16,
    pstPTR,
    pstVector,
    pstVirDis,
    pstColor,
    pstLoc2LED,                     /* Particle location convert to LED index */
    pstSize2LED                      /* Particle size to LED number */
}PS_PARA_TYPE_E;

/* Particle system parameter properties */
typedef struct PS_PARA_PROPERTY
{
    UINT8 pGroup;
    UINT8 offset;                   /* In no. of byte */
    UINT8 pType;
}PS_PARA_PROPERTY_T;

/* Particle and particle system parameter enumeration */
typedef enum SIM_PARA
{
    pSystemID          = 0x00,      /* Particle: systemID */
    pLayer             = 0x01,      /* Particle: layer */
    pAngle             = 0x02,      /* Particle: angle */
    pAVelocity         = 0x03,      /* Particle: aVelocity */
    pGrid              = 0x04,      /* Particle: grid */
    pAAmplitude        = 0x05,      /* Particle: aAmplitude */

    pSize              = 0x06,      /* Particle: size */
    pGrow              = 0x07,      /* Particle: grow */
    pMass              = 0x08,      /* Particle: mass */
    pMaxSpeed          = 0x09,      /* Particle: maxSpeed */
    pMaxSteer          = 0x0A,      /* Particle: maxSteer */
    pSeq               = 0x0B,      /* Particle: seq */

    pState             = 0x0C,      /* Particle: state */
    pTexture           = 0x0D,      /* Particle: texture */
    pLifespan          = 0x0E,      /* Particle: lifespan */
    pEffectCnt         = 0x0F,      /* Particle: effectCnt */
    pLocation          = 0x10,      /* Particle: location */
    pVelocity          = 0x11,      /* Particle: velocity */
    pAcceleration      = 0x12,      /* Particle: acceleration */
    pColor             = 0x13,      /* Particle: color */
    pSeqEnd            = 0x14,      /* Particle: seqEnd */

    psbId              = 0x15,      /* Particle system base: id */
    psbSeqMax          = 0x16,      /* Particle system base: seqMax */
    psbCreateRate      = 0x17,      /* Particle system base: createRate */
    psbMaxCnt          = 0x18,      /* Particle system base: maxCnt */
    psbCurrentCnt      = 0x19,      /* Particle system base: currentCnt */
    psbMaxState        = 0x1A,      /* Particle system base: maxState */

    psbTimeMul         = 0x1B,      /* Particle system base: timeMul */
    psbAngleMin        = 0x1C,      /* Particle system base: angleMin */
    psbAngleMax        = 0x1D,      /* Particle system base: angleMax */
    psbAVelMin         = 0x1E,      /* Particle system base: aVelMin */
    psbAVelMax         = 0x1F,      /* Particle system base: aVelMax */
    psbAAmpMin         = 0x20,      /* Particle system base: aAmpMin */

    psbAAmpMax         = 0x21,      /* Particle system base: aAmpMax */
    psbSizeMin         = 0x22,      /* Particle system base: sizeMin */
    psbSizeMax         = 0x23,      /* Particle system base: sizeMax */
    psbGrowMin         = 0x24,      /* Particle system base: growMin */
    psbGrowMax         = 0x25,      /* Particle system base: growMax */
    psbMassMin         = 0x26,      /* Particle system base: massMin */

    psbMassMax         = 0x27,      /* Particle system base: massMax */
    psbMaxSpeedMin     = 0x28,      /* Particle system base: maxSpeedMin */
    psbMaxSpeedMax     = 0x29,      /* Particle system base: maxSpeedMax */
    psbMaxSteerMin     = 0x2A,      /* Particle system base: maxSteerMin */
    psbMaxSteerMax     = 0x2B,      /* Particle system base: maxSteerMax */
    psbSizeAspect      = 0x2C,      /* Particle system base: sizeAspect */

    psbSeparation      = 0x2D,      /* Particle system base: separation */
    psbCohesion        = 0x2E,      /* Particle system base: cohesion */
    psbLifeMin         = 0x2F,      /* Particle system base: lifeMin */
    psbLifeMax         = 0x30,      /* Particle system base: lifeMax */
    psbEffectCnt       = 0x31,      /* Particle system base: effectCnt */

    psbTarget          = 0x32,      /* Particle system base: target */
    psbLocMin          = 0x33,      /* Particle system base: locMin */
    psbLocMax          = 0x34,      /* Particle system base: locMax */
    psbVelMin          = 0x35,      /* Particle system base: velMin */
    psbVelMax          = 0x36,      /* Particle system base: velMax */
    psbAccMin          = 0x37,      /* Particle system base: accMin */
    psbAccMax          = 0x38,      /* Particle system base: accMax */

    psbWind            = 0x39,      /* Particle system base: windPtr */
    psbForce           = 0x3A,      /* Particle system base: forcePtr */

    psbPsFlag          = 0x3B,      /* Particle system base: psFlag */
    psbRandSnap        = 0x3C,      /* Particle system base: randSnap */

    psiPsiFlag         = 0x3D,      /* Particle system image info: psiFlag */
    psiSeedColorInd    = 0x3E,      /* Particle system image info: seedColorInd */
    psiInitColorInd    = 0x3F,      /* Particle system image info: initColorInd */
    psiIcNum           = 0x40,      /* Particle system image info: icNum */
    psiUpdateColorInd  = 0x41,      /* Particle system image info: updateColorInd */
    psiUcNum           = 0x42,      /* Particle system image info: ucNum */
    psiSColorMethod    = 0x43,      /* Particle system image info: sColorMethod */
    psiColorMapInd     = 0x44,      /* Particle system image info: colorMapInd */
    psiColorMapLen     = 0x45,      /* Particle system image info: colorMapLen */
    psiColorTargetInd  = 0x46,      /* Particle system image info: colorTargetInd */
    psiShape           = 0x47,      /* Particle system image info: shape */
    psiBackGround      = 0x48,      /* Particle system image info: background */

    pLocationX         = 0x49,      /* Particle: location.x */
    pLocationY         = 0x4A,      /* Particle: location.y */
    pVelocityX         = 0x4B,      /* Particle: pVelocityX */
    pVelocityY         = 0x4C,      /* Particle: pVelocityY */
    pLEDLocY           = 0x4D,      /* Particle: pLEDLocY  */
    pLEDSize           = 0x4E,      /* Particle: pLEDSize  */

    sParaInvalid = 0xFF
}SIM_PARA_E;

/************************************************* Global variable declarations *******************************/
GLOBAL PARTICLE_SYSTEM_ST SimPS[ SIM_PS_NUM ];
GLOBAL PARTICLE_ST        SimP [ SIM_PARTICLE_NUM ];

GLOBAL UINT8              SimPMax;/* Maximum no. of particle for the running pattern */

GLOBAL CONST PS_PARA_PROPERTY_T SimParaProperty[];

GLOBAL BOOL               AudioDet;
GLOBAL THEME_COLOR_MODE   ThemeColorMode;
GLOBAL COLOR_T            ThemeColor;
/************************************************* Functional prototypes **************************************/
/************************************************************************
 Function:    SimulatorInitialize
 Description: Initialize the simulator
 Input:       Nil
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL VOID SimulatorInitialize(VOID);

/************************************************************************
 Function:    Simulator
 Description: DISEngine Simulator module entry point
 Input:       Nil
 Return:      VOID
 Remarks:
************************************************************************/
GLOBAL VOID Simulator(VOID);

/************************************************************************
 Function:    SimRandom   
 Description: Generate a random number within the range specified
 Input:       max: upper limit of random number range
              min: lower limit of random number range
 Return:      A random number
 Remarks:
************************************************************************/
GLOBAL UINT16 SimRandom(UINT16 min, UINT16 max);

/************************************************************************
 Function:    SimSqrt(UINT16 x2)
 Description: Get the x2's square root
 Input:       x2: the number to be sqrt
 Return:      square root
 Remarks:
************************************************************************/
GLOBAL UINT8 SimSqrt(UINT16 x2);

/**************************************************************************************************
 Function:     SimMax
 Description:  Calculate Max of two short signed INT's
 Input:	       X1, X2 Int nos to be compared
 Return:       the Largest of the two Int
 Remarks:
***************************************************************************************************/
GLOBAL INT16 SimMax(INT16 x1, INT16 x2);

/**************************************************************************************************
 Function:     SimSin
 Description:  Calculate sine of an angle in degree
 Input:	       angle : angle to caculate the sine
 Return:       the angle's sine
 Remarks:
***************************************************************************************************/
GLOBAL INT8 SimSin(INT16 angle);

/************************************************************************
 Function:    SimDistanceSq
 Description: Calculate the distance between two input locations
 Input:       loc1: first location
              loc2: second location
 Return:      Distance square
 Remarks:
************************************************************************/
GLOBAL UINT16 inline SimDistanceSq(PVECTOR_ST loc1, PVECTOR_ST loc2);

/**************************************************************************************************
 Function:     SimVectorMag2
 Description:  Calculate the magnitude squre of a vector
 Input:	       v1: input vector
 Return:       Magnitude square of the vector
 Remarks:
***************************************************************************************************/
GLOBAL UINT16 inline SimVectorMag2(PVECTOR_ST v1);

///**************************************************************************************************
// Function:     PauseHourglass
// Description:  Pause the running hourglass and show the pause system
// Input:	       VOID
// Return:       VOID
// Remarks:      Special treatment for Hourglass pattern, called from mapPhyDis() in Renderer
//***************************************************************************************************/
//GLOBAL VOID PauseHourglass(VOID);

/**************************************************************************************************
 Function:     RestartHourglass
 Description:  Restart the hourglass and hide the pause system
 Input:	       VOID
 Return:       VOID
 Remarks:      Special treatment for Hourglass pattern, called from mapPhyDis() in Renderer
***************************************************************************************************/
GLOBAL VOID RestartHourglass(VOID);

///**************************************************************************************************
// Function:     ResumeHourglass
// Description:  Resume the running hourglass and hide the pause system
// Input:	       VOID
// Return:       VOID
// Remarks:      Special treatment for Hourglass pattern, called from mapPhyDis() in Renderer
//***************************************************************************************************/
//GLOBAL VOID ResumeHourglass(VOID);

GLOBAL VOID setEQWaveLevelLarge(VOID);
GLOBAL VOID setEQWaveLevelMedium(VOID);
GLOBAL VOID setEQWaveLevelSmall(VOID);
GLOBAL VOID setEQFireLevel(UINT16 psInd, UINT16 fireLevel);

GLOBAL PATTERN_REC_ST *GetPatMeta_Addr(VOID);
GLOBAL PS_BASE_T *GetPatPsb_Addr(VOID);
GLOBAL PS_IMAGE_INFO_T *GetPatPsi_Addr(VOID);

GLOBAL SIM_ACTION_T *GetPatExAction_Addr(VOID);
GLOBAL SIM_OPERATION_T *GetPatExOp_Addr(VOID);


#endif /* SIMULATOR_H */

