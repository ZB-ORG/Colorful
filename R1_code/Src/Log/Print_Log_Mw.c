//#include "..\..\Inc\sys_common.h"

#include "stm32f0xx_hal.h"
#include <stdarg.h>

#define KEILC
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

extern UART_HandleTypeDef huart1;

extern void UART1_SendData (uint8_t Data);


PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART1 and Loop until the end of transmission */
    HAL_UART_Transmit (&huart1, (uint8_t*) &ch, 1, 500);

    return ch;
}

#if 0

CONST unsigned char Hex[] = "0123456789ABCDEF";

char Print_char (char c)
{
    UART1_SendData (c);
    return true;
}


void Do_Print ( CONST char* fmt, va_list ap )
{
    char  ch;
    char  i;
    long  value;
    char   fl_zero;
    //char   fl_num;
    unsigned char  fl_len;
    unsigned char  cnt;
    unsigned short  mask=1;   // 1word

#ifdef KEILC
    char* ptr;
#endif

    while (1)
    {

        //----- Find Formatter % -----

        switch ( ch = *fmt++ )
        {
            case 0:
                return;

            case '%':
                if ( *fmt != '%' )
                {
                    break;
                }

                fmt++;

            default:
                Print_char ( ch );
                continue;
        }

        //----- Get Count -------------

        fl_zero = 0;
        //fl_num = 0;
        cnt = 0;

        ch = *fmt++;

        if ( ch=='0' )
        {
            fl_zero = 1;
            ch = *fmt++;
            cnt = ch - '0';
            ch = *fmt++;
        }
        else if ( ch>='0' && ch<='9' )
        {
            cnt = ch - '0';
            ch = *fmt++;
        }

        //----- Get char(B) / int / long(L) ----------------

        fl_len = 2;

        switch (ch)
        {
            case 'l':
            case 'L':
                ch = *fmt++;
                fl_len = 4;
                break;

            case 'b':
            case 'B':
                ch = *fmt++;
                fl_len = 1;
                break;
        }

        //----- Get Type Discriptor -----

        switch ( ch )
        {

            case 'd':
            case 'u':

                switch (fl_len)
                {
                    case 1:
                        if ( ch=='d' )
                        {
                            value = (char) va_arg ( ap, char );
                        }
                        else
                        {
                            value = (unsigned char) va_arg ( ap, unsigned char );
                        }

                        break;

                    case 2:
                        if ( ch=='d' )
                        {
                            value = (int) va_arg ( ap,  int );
                        }
                        else
                        {
                            value = (unsigned short) va_arg ( ap, unsigned short );
                        }

                        break;

                    case 4:
                        if ( ch=='d' )
                        {
                            value = (long) va_arg ( ap, long );
                        }
                        else
                        {
                            value = (unsigned long) va_arg ( ap, unsigned long );
                        }

                        break;
                }

                if ( value<0 )
                {
                    Print_char ('-');
                    value = value* (-1);
                }

                if (cnt==0)
                {
                    if ( value==0 )
                    {
                        Print_char ('0');
                        continue;
                    }

                    for (cnt=0, mask=1; cnt<10; cnt++)
                    {
                        if ( (value/mask) ==0 )
                        {
                            break;
                        }

                        mask = mask*10;
                    }
                }

                for (i=0, mask=1; i<cnt-1; i++)
                {
                    mask = mask*10;
                }

                while (1)
                {
                    ch = (value / mask) + '0';

                    if ( ch=='0' && fl_zero==0 && mask!=1 )
                    {
                        ch=' ';
                    }
                    else
                    {
                        fl_zero = 1;
                    }

                    Print_char (ch);

                    value = value % (mask);
                    mask = mask / 10;

                    if ( mask==0 )
                    {
                        break;
                    }
                }

                continue;

            case 'x':
            case 'X':

                switch (fl_len)
                {
                    case 1:
                        value = (unsigned char) va_arg ( ap, unsigned char );
                        break;

                    case 2:
                        value = (unsigned short) va_arg ( ap, unsigned short );
                        break;

                    case 4:
                        value = (unsigned long) va_arg ( ap, unsigned long );
                        break;
                }

                if (cnt==0)
                {
                    cnt = fl_len*2;
                }

                for (i=0; i<cnt; i++)
                {
                    Print_char ( Hex[ (value >> (cnt-i-1) *4) & 0x000f] );
                }

                continue;

            case 's':

#ifdef TASKINGC

                value = (unsigned short) va_arg ( ap, unsigned short );

                while (* (char*) value!='\0')
                {
                    Print_char (* (char*) value++);
                }

                continue;

#elif defined KEILC

                ptr = (char*) va_arg ( ap, char* );

                while (*ptr!='\0')
                {
                    Print_char (*ptr++);
                }

                continue;

#endif


            case 'c':
                value = va_arg ( ap, int );
                Print_char ( (unsigned char) value);
                continue;

            default:
                value = (unsigned short) va_arg ( ap, int );
                continue;
        }
    }
}
#endif

//===========================================================================//
//                                                                           //
//===========================================================================//



void  dbg_level_printf (const char* fmt, ...  )
{

    va_list ap;

    va_start (ap, fmt);
    //Do_Print( fmt, ap );
    vprintf ( fmt, ap );
    va_end ( ap );

}








