#ifndef _U_TYPE_DEF_H_
#define _U_TYPE_DEF_H_

/* Processor & Compiler independent, size specific definitions */
// To Do:  We need to verify the sizes on each compiler.  These
//         may be compiler specific, we should either move them
//         to "compiler.h" or #ifdef them for compiler type.
typedef signed int          INT32;
typedef signed char         INT8;
typedef signed short int    INT16;
typedef signed long long    INT64;

typedef unsigned int        UINT32;
typedef unsigned char       UINT8;
typedef unsigned short int  UINT16;
typedef unsigned long long  UINT64;


#define    UCHAR	        UINT8  /* unsigned 8-bit */
#define    CHAR             char


typedef void                VOID;

#define SIZE_T               UINT32

#ifndef uint16_t
#define uint16_t UINT16
#define uint8_t  UINT8
#define uint32_t UINT32
#endif

#ifndef int8_t
#define int8_t   INT8
#define int16_t  INT16
#define int32_t  INT32
#endif
#ifndef int8
#define int8   INT8
#define int16  INT16
#define int32  INT32
#endif

#ifndef NULL
#define NULL (0)
#endif

#ifndef u8
#define u8  UINT8
#define u16 UINT16
#define u32 UINT32
#endif

#ifndef uint8
#define uint8  UINT8
#define uint16 UINT16
#define uint32 UINT32
#endif

#ifndef HANDLE_T
#define HANDLE_T INT32
#endif

#define HIGH		(1)
#define LOW			(0)

#ifdef __cplusplus
typedef bool bool_t;
#else
typedef enum
{
	false   = 0,
	true    = !(false)
} BOOL;

#define bool BOOL
#endif // __cplusplus


#ifndef ON
#define ON  TRUE
#define OFF FALSE
#endif

#ifndef TRUE
#define TRUE true
#define FALSE false
#endif


#ifndef MSIZE_T
#define MSIZE_T  UINT32
#endif 

#ifndef SUCCESS
#define SUCCESS true
#define FAILURE false
#endif

#ifndef ERROR	
#define OK true
#define ERROR false
#endif

#ifndef CONST
#define CONST const
#endif

typedef void (*NO_INPUT_NO_RETURN_CALLBACK)(void);
typedef void (*U8_INPUT_NO_RETURN_CALLBACK)(uint8 u8Data);
typedef void (*U32_INPUT_NO_RETURN_CALLBACK)(uint32 u8Data);

#define RESULT_TRUE  1
#define RESULT_OK  1
#define RESULT_FALSE  0
#define RESULT_ERROR  0

#define MCU_FLASH_SIZE      512 //K   

#ifndef MCU_FLASH_SIZE
#define MCU_FLASH_SIZE      128
#endif

#ifndef LOCAL
#define LOCAL  static
#endif

#define VOID    void
#define CONST   const
#define CHAR    char
#define GLOBAL  extern      /* Function attributes */

#define MSB( n )  ( ( UINT8 )( ( n & 0xFF00 ) >> 8 ) )           /* Most significant byte of a 16-bit word */
#define LSB( n )  ( ( UINT8 )( n & 0x00FF ) )                /* Least significant byte of a 16-bit word */

#define HINIBBLE(n) ( n >> 4 )              /* Upper 4-bit of a 8-bit byte */
#define LONIBBLE(n) ( n & 0x0F )            /* Lower 4-bit of a 8-bit byte */


#define SUB_MSG_ID(Module_id, sub_msg_id)  Module_id<<8 | sub_msg_id
#define GET_MSG_ID(SBU_MSG)                SBU_MSG>>8


typedef struct	bit_struct
{
	unsigned char	bit0:1;			/* MSB */
	unsigned char	bit1:1;
	unsigned char	bit2:1;
	unsigned char	bit3:1;
	unsigned char	bit4:1;
	unsigned char	bit5:1;
	unsigned char	bit6:1;
	unsigned char	bit7:1;			

	unsigned char	bit8:1;			
	unsigned char	bit9:1;
	unsigned char	bit10:1;
	unsigned char	bit11:1;
	unsigned char	bit12:1;
	unsigned char	bit13:1;
	unsigned char	bit14:1;
	unsigned char	bit15:1;	    /* LSB */
}Bit_Struct;

#endif