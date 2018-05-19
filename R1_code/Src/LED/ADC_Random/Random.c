#include "stm32f0xx_hal.h"
#include "..\..\..\Inc\sys_common.h"

extern ADC_HandleTypeDef hadc;

uint16_t ADC_Random_Disaffinity(uint16_t min, uint16_t max)
{
	static uint32_t old_random_num = 0xff;
	uint32_t random_num1, random_num2;
	
	while(1)
	{
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, 50);
		if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc), HAL_ADC_STATE_REG_EOC))
		{
			random_num1 = HAL_ADC_GetValue(&hadc);
		}
		random_num2 = ( ( uint16_t )( random_num1 % ( max - min + 1 ) ) + min );
		
		if (random_num2 != old_random_num)
		{
			old_random_num = random_num2;
			break;
		}
	}
	
	return random_num2;
}

uint16_t ADC_Random_Complete(uint16_t min, uint16_t max)
{
	uint32_t random_num;

	HAL_ADC_Start(&hadc);
	HAL_ADC_PollForConversion(&hadc, 50);
	if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc), HAL_ADC_STATE_REG_EOC))
	{
		random_num = HAL_ADC_GetValue(&hadc);
	}
	
	return ( ( uint16_t )( random_num % ( max - min + 1 ) ) + min );
}

#if 0
uint16_t ADC_Random_Disaffinity_16bit(uint16_t min, uint16_t max)
{
	uint8_t i;
	static uint32_t old_random_num = 0xff;
	uint32_t random_num1, random_num2;
	
	while(1)
	{
		for (i=0; i<4; i++)
		{
			HAL_ADC_Start(&hadc);
			HAL_ADC_PollForConversion(&hadc, 50);
			if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc), HAL_ADC_STATE_REG_EOC))
			{
				random_num1 |= (HAL_ADC_GetValue(&hadc)%16)<<(4*i);
			}
		}
		
		random_num2 = ( ( uint16_t )( random_num1 % ( max - min + 1 ) ) + min );
		
		if (random_num2 != old_random_num)
		{
			old_random_num = random_num2;
			break;
		}
	}
	
	return random_num2;
}

uint16_t ADC_Random_Complete_16bit(uint16_t min, uint16_t max)
{
	uint8_t i;
	uint32_t random_num = 0;

	for (i=0; i<4; i++)
	{
		HAL_ADC_Start(&hadc);
		HAL_ADC_PollForConversion(&hadc, 50);
		if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc), HAL_ADC_STATE_REG_EOC))
		{
			random_num |= (HAL_ADC_GetValue(&hadc)%16)<<(4*i);
		}
	}
	
	return ( ( uint16_t )( random_num % ( max - min + 1 ) ) + min );
}
#endif

