/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MCP4725.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
volatile uint64_t ch1_rising = 0, ch1_falling = 0, ch1 = 0, pre_ch1 = 0;
volatile uint64_t ch2_rising = 0, ch2_falling = 0, ch2 = 0, pre_ch2 = 0;
volatile uint64_t ch3_rising = 0, ch3_falling = 0, ch3 = 0, pre_ch3 = 0;
volatile uint64_t ch4_rising = 0, ch4_falling = 0, ch4 = 0, pre_ch4 = 0;

int deadband_scale = 0;
int maxpoint = 0;
int midpoint = 0;
int minpoint = 0;
int motor_startup_deadband = 0;

volatile int left_output = 0, right_output = 0;
volatile float ch1_smooth = 0, ch2_smooth = 0;

int left_motor_pwm, right_motor_pwm;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int map(int x, int in_min, int in_max, int out_min, int out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM2) {
		switch (htim->Channel) {
		case HAL_TIM_ACTIVE_CHANNEL_1:
			// Kanal 1 için işlemler
			if ((TIM2->CCER & TIM_CCER_CC1P) == 0) {
				ch1_rising = TIM2->CCR1; // yukselen kenar degerini kaydet
				TIM2->CCER |= TIM_CCER_CC1P; // polariteyi düsen kenar olarak degistir
			} else {
				ch1_falling = TIM2->CCR1;
				pre_ch1 = ch1_falling - ch1_rising; // dusen kenar degerini kaydet ve yukselen kenar degerinden cikar
				if (pre_ch1 < 0) {
					pre_ch1 += 0xFFFF; // eger sonuc negatifse taban tumleme yap
				}
				if (pre_ch1 < 2010 && pre_ch1 > 990) {
					ch1 = pre_ch1;
				}
				TIM2->CCER &= ~TIM_CCER_CC1P; // polariteyi yukselen kenar olarak ayarla
			}
			break;
		case HAL_TIM_ACTIVE_CHANNEL_2:
			// Kanal 2 için işlemler
			if ((TIM2->CCER & TIM_CCER_CC2P) == 0) {
				ch2_rising = TIM2->CCR2; // yukselen kenar degerini kaydet
				TIM2->CCER |= TIM_CCER_CC2P; // polariteyi düsen kenar olarak degistir
			} else {
				ch2_falling = TIM2->CCR2;
				pre_ch2 = ch2_falling - ch2_rising; // dusen kenar degerini kaydet ve yukselen kenar degerinden cikar
				if (pre_ch2 < 0) {
					pre_ch2 += 0xFFFF; // eger sonuc negatifse taban tumleme yap
				}
				if (pre_ch2 < 2010 && pre_ch2 > 990) {
					ch2 = pre_ch2;
				}
				TIM2->CCER &= ~TIM_CCER_CC2P; // polariteyi yukselen kenar olarak ayarla
			}

			break;
		case HAL_TIM_ACTIVE_CHANNEL_3:
			// Kanal 3 için işlemler
			if ((TIM2->CCER & TIM_CCER_CC3P) == 0) {
				ch3_rising = TIM2->CCR3; // yukselen kenar degerini kaydet
				TIM2->CCER |= TIM_CCER_CC3P; // polariteyi düsen kenar olarak degistir
			} else {
				ch3_falling = TIM2->CCR3;
				pre_ch3 = ch3_falling - ch3_rising; // dusen kenar degerini kaydet ve yukselen kenar degerinden cikar
				if (pre_ch3 < 0) {
					pre_ch3 += 0xFFFF; // eger sonuc negatifse taban tumleme yap
				}
				if (pre_ch3 < 2010 && pre_ch3 > 990) {
					ch3 = pre_ch3;
				}
				TIM2->CCER &= ~TIM_CCER_CC3P; // polariteyi yukselen kenar olarak ayarla
			}

			break;
		case HAL_TIM_ACTIVE_CHANNEL_4:
			// Kanal 4 için işlemler
			if ((TIM2->CCER & TIM_CCER_CC4P) == 0) {
				ch4_rising = TIM2->CCR4; // yukselen kenar degerini kaydet
				TIM2->CCER |= TIM_CCER_CC4P; // polariteyi düsen kenar olarak degistir
			} else {
				ch4_falling = TIM2->CCR4;
				pre_ch4 = ch4_falling - ch4_rising; // dusen kenar degerini kaydet ve yukselen kenar degerinden cikar
				if (pre_ch4 < 0) {
					pre_ch4 += 0xFFFF; // eger sonuc negatifse taban tumleme yap
				}
				if (pre_ch4 < 2010 && pre_ch4 > 990) {
					ch4 = pre_ch4;
				}
				TIM2->CCER &= ~TIM_CCER_CC4P; // polariteyi yukselen kenar olarak ayarla
			}

			break;
		default:
			break;
		}
	}
}

MCP4725 LeftMCP4725;
MCP4725 RightMCP4725;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_I2C1_Init();
	MX_I2C2_Init();
	/* USER CODE BEGIN 2 */
	LeftMCP4725 = MCP4725_init(&hi2c1, MCP4725A0_ADDR_A00, 4.57);
	RightMCP4725 = MCP4725_init(&hi2c1, MCP4725A0_ADDR_A01, 4.23);

	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

	if (!MCP4725_isConnected(&LeftMCP4725)
			|| !MCP4725_isConnected(&RightMCP4725)) {
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(500);
	}
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if (ch3 <= 1500) {
			maxpoint = 1900;
			midpoint = 1500;
			minpoint = 1100;
			deadband_scale = 32;
			motor_startup_deadband = 100;
		} else {
			maxpoint = 1900;
			midpoint = 1500;
			minpoint = 1100;
			deadband_scale = 32;
			motor_startup_deadband = 100;
		}

		ch1_smooth -= ch1_smooth / 10.0;
		ch1_smooth += ch1 / 10.0;

		ch2_smooth -= ch2_smooth / 10.0;
		ch2_smooth += ch2 / 10.0;

		left_output = (ch2_smooth + ((ch1_smooth - 1500) * 0.5));
		if (left_output < midpoint - (deadband_scale / 4))
			HAL_GPIO_WritePin(GPIOA, left_motor_direction_Pin, SET);
		else
			HAL_GPIO_WritePin(GPIOA, left_motor_direction_Pin, RESET);

		if (abs(left_output - midpoint) < (deadband_scale / 2))
			left_output = midpoint;  //orta ölübant  (abs() -> mutlak değer)
		if (left_output > maxpoint - deadband_scale)
			left_output = maxpoint;       //max 500
		else if (left_output < minpoint + deadband_scale)
			left_output = minpoint;  //min -500

		right_output = (ch2_smooth - ((ch1_smooth - 1500)*0.5));
		if (right_output < midpoint - (deadband_scale / 4))
			HAL_GPIO_WritePin(GPIOA, right_motor_direction_Pin, SET);
		else
			HAL_GPIO_WritePin(GPIOA, right_motor_direction_Pin, RESET);

		if (abs(right_output - midpoint) < (deadband_scale / 2))
			right_output = midpoint;  //orta ölübant  (abs() -> mutlak değer)
		if (right_output > maxpoint - deadband_scale)
			right_output = maxpoint;       //max 500
		else if (right_output < minpoint + deadband_scale)
			right_output = minpoint;  //min -500

		if (left_output == midpoint) {
			HAL_GPIO_WritePin(GPIOB, left_break_output_Pin, SET);
		} else {
			HAL_GPIO_WritePin(GPIOB, left_break_output_Pin, RESET);
		}
		if (right_output == midpoint) {
			HAL_GPIO_WritePin(GPIOB, right_break_output_Pin, SET);
		} else {
			HAL_GPIO_WritePin(GPIOB, right_break_output_Pin, RESET);
		}

		if (ch4 >= 1500) {
			HAL_GPIO_WritePin(GPIOA, handbrake_Pin, SET);
			HAL_GPIO_WritePin(GPIOB, left_break_output_Pin, SET);
			HAL_GPIO_WritePin(GPIOB, right_break_output_Pin, SET);
		}

		if (left_output >= midpoint)
			left_motor_pwm = map(left_output, midpoint, maxpoint, 0, 1000);
		else
			left_motor_pwm = abs(
					map(left_output, minpoint, midpoint, 0, 1000) - 1000);

		if (right_output >= midpoint)
			right_motor_pwm = map(right_output, midpoint, maxpoint, 0, 1000);
		else
			right_motor_pwm = abs(
					map(right_output, minpoint, midpoint, 0, 1000) - 1000);

		//i2c ile başlatılmış sürücünün bufferından output verilecek
		MCP4725_setValue(&LeftMCP4725,
				(uint16_t) (map(left_motor_pwm, 0, 1000, 800, 2500)),
				MCP4725_FAST_MODE, MCP4725_POWER_DOWN_OFF);
		MCP4725_setValue(&RightMCP4725,
				(uint16_t) (map(right_motor_pwm, 0, 1000, 800, 2500)),
				MCP4725_FAST_MODE, MCP4725_POWER_DOWN_OFF);

		HAL_Delay(20);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void) {

	/* USER CODE BEGIN I2C2_Init 0 */

	/* USER CODE END I2C2_Init 0 */

	/* USER CODE BEGIN I2C2_Init 1 */

	/* USER CODE END I2C2_Init 1 */
	hi2c2.Instance = I2C2;
	hi2c2.Init.ClockSpeed = 100000;
	hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c2.Init.OwnAddress1 = 0;
	hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2 = 0;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C2_Init 2 */

	/* USER CODE END I2C2_Init 2 */

}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 576 - 1;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 1000 - 1;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_IC_InitTypeDef sConfigIC = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 72 - 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 0xFFFF;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_4) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA,
			right_motor_direction_Pin | left_motor_direction_Pin | handbrake_Pin
					| shifter_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, left_break_output_Pin | right_break_output_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : right_motor_direction_Pin left_motor_direction_Pin handbrake_Pin shifter_Pin */
	GPIO_InitStruct.Pin = right_motor_direction_Pin | left_motor_direction_Pin
			| handbrake_Pin | shifter_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : left_break_output_Pin right_break_output_Pin */
	GPIO_InitStruct.Pin = left_break_output_Pin | right_break_output_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
