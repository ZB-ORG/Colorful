#ifndef __AP_AUTOTEST_CHANNEL_TEST_H
#define __AP_AUTOTEST_CHANNEL_TEST_H

#define ADC_SAMPLE_RATE             (16)

#define POWER_LOW_THRESHOLD         (0x1000)

#define MAX_SAMPLE_COUNT            (4)

//#define PRINT_CHANNEL_DATA

#ifdef DEBUG_WRITE_CHANNEL_DATA
extern void write_temp_file(uint8 file_index, uint8 *write_buffer, uint32 write_len);
#endif

#endif
