/*
 * module2.c
 */

#include "module2.h"

GPIO_InitTypeDef GPIO_InitStruct;
EXTI_InitTypeDef EXTI_InitStruct;
NVIC_InitTypeDef NVIC_InitStruct;
//uint16_t tics = 0;
uint32_t speed;
uint32_t data2;
uint8_t  rG;
uint16_t value;

//Si TIM_GetCounter anterior es mes gran que el nou, zeroX --
//Flag overflow timer?


//Utils
uint16_t getSpeed(uint16_t pre_t, uint16_t post_t) {
/*
	Abans de cridar aquesta funció:
	if (!zeroX)
		speedX = 0;
	else
		speedX = getSpeed(pre_tX, post_tX);
*/
	if (post_t < pre_t)
		speed = (uint32_t) 0x0000FFFF-pre_t + post_t;
	else
		speed = (uint32_t) post_t - pre_t;
	//speed = (10^6 * 100) / (dif_timer * 32 * 2)
	//		2*10^-6 segons (2us)	-> Ttimer
	//		32						-> Fórmula
	//		100						-> velocitat rotació multiplicada per 100
	speed = 100000000 / (speed<<6);
	return (uint16_t) speed;
}

uint16_t getDAC_Data(uint16_t speedR, uint16_t speedL) {
	data2 = 0x00000800;
	if (speedR == speedL) {
		return (uint16_t) data2;
	}
	if (speedR > speedL) {
		rG = 1;
		value = (100*speedR)/speedL - 100;
	}
	else {
		rG = 0;
		value = (100*speedL)/speedR - 100;
	}
	if (value < 30)
		return (uint16_t) data2;
	//Roda dreta amb major velocitat
	if (rG) {
		//Velocitat mínima?
		if (speedL < 555)
			return (uint16_t) data2;
		//(1.5*4095)/(0.7*3) = 2925
		data2 = 2925*(value-30);
		if (data2 > 0x7FF*100)
			return 0xFFF;
		return (0x800*100 + data2)/100;
	}
	//Roda esquerra amb major velocitat
	else {
		if (speedR < 555)
			return (uint16_t) data2;
		//(1.5*4095)/(0.7*3) = 2925
		data2 = 2925*(value-30);
		if (data2 > 0x800*100)
			return 0;
		return (0x800*100 - data2)/100;
	}
}

//tics = TIM_GetCounter(TIM3)

//RSIs
/*void TIM3_IRQHandler(void)
{
	// Checks whether the TIM2 interrupt has occurred or not
	if (TIM_GetITStatus(TIM3, TIM_IT_Update))
	{
		if (tics == 0xFFFF)
		{
			if (zeroR > 0)
				zeroR--;
			if (zeroL > 0)
				zeroL--;
		}
		tics++;

		//STM_EVAL_LEDToggle(LED4);
		TIM_ClearFlag(TIM3, TIM_IT_Update);
		// Clears the TIM4 interrupt pending bit
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}

}*/

/* Seguiment temps Roda dreta */
void EXTI4_IRQHandler(void)
{
	//Roda dreta (PB4)
	if (EXTI_GetFlagStatus(EXTI_Line4))
	{
		pre_tR = post_tR;
		post_tR = tics;
		//post_tR = TIM_GetCounter(TIM3);
		zeroR = 2;
		EXTI_ClearFlag(EXTI_Line4);
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

/* Seguiment temps Roda esquerra */
void EXTI15_10_IRQHandler(void)
{
	//Roda esquerre (PC11)
	if (EXTI_GetFlagStatus(EXTI_Line11))
	{
		pre_tL = post_tL;
		post_tL = tics;
		//post_tL = TIM_GetCounter(TIM3);
		zeroL = 2;
		EXTI_ClearFlag(EXTI_Line11);
		EXTI_ClearITPendingBit(EXTI_Line11);
	}
}

//Configurations

/* Interrupció per flanc de baixada del PWM roda dreta */
void EXTILine4_Config(void)
{
	//Roda dreta (PB4)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

	EXTI_InitStruct.EXTI_Line = EXTI_Line4;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

/* Interrupció per flanc de baixada del PWM roda esquerra */
void EXTILine11_Config(void)
{
	//Roda esquerre (PC11)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource11);

	EXTI_InitStruct.EXTI_Line = EXTI_Line11;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

/* Configuració timer en free-running */
    /*timer_tick_frequency = Timer_default_frequency / (prescaller_set + 1)

    In our case, we want a max frequency for timer, so we set prescaller to 0
    And our timer will have tick frequency

    timer_tick_frequency = 84000000 / (0 + 1) = 84000000 */

void TIM3_Config(void) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// Update Event (Hz) = 90MHz / ((179 + 1) * (0 + 1)) = 500kHz
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 0;	//179
	TIM_TimeBaseInitStruct.TIM_Period = 179;	//0xFFFF
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

	// TIM4 initialize
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	// Enable TIM4 interrupt
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	// Start TIM4
	TIM_Cmd(TIM3, ENABLE);
/*
	// Nested vectored interrupt settings
	// TIM4 interrupt is most important (PreemptionPriority and
	// SubPriority = 0)
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
*/
}

void Module2_Config(void){
	EXTILine4_Config();
	EXTILine11_Config();
	TIM3_Config();
}

/* Ús de Input Capture */
/*
void EXTI9_5_IRQHandler(void)
{
	//Roda esquerre (PC8)
	if (EXTI_GetFlagStatus(EXTI_Line8))
	{
		pre_tL = post_tL;
		pre_tL = tics;
		zeroL = 2;

		STM_EVAL_LEDOn(LED3);
		EXTI_ClearFlag(EXTI_Line8);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}

void EXTILine8_Config(void)
{
	//Roda esquerre (PC8)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);

	EXTI_InitStruct.EXTI_Line = EXTI_Line8;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void TIM3Ch_Config(void)
{

	To use the Timer in Input Capture mode, the following steps are mandatory:

      (#) Enable TIM clock using RCC_APBxPeriphClockCmd(RCC_APBxPeriph_TIMx, ENABLE)
          function

      (#) Configure the TIM pins by configuring the corresponding GPIO pins

      (#) Configure the Time base unit as described in the first part of this driver,
          if needed, else the Timer will run with the default configuration:
        (++) Autoreload value = 0xFFFF
        (++) Prescaler value = 0x0000
        (++) Counter mode = Up counting
        (++) Clock Division = TIM_CKD_DIV1

      (#) Fill the TIM_ICInitStruct with the desired parameters including:
        (++) TIM Channel: TIM_Channel
        (++) TIM Input Capture polarity: TIM_ICPolarity
        (++) TIM Input Capture selection: TIM_ICSelection
        (++) TIM Input Capture Prescaler: TIM_ICPrescaler
        (++) TIM Input CApture filter value: TIM_ICFilter

      (#) Call TIM_ICInit(TIMx, &TIM_ICInitStruct) to configure the desired channel
          with the corresponding configuration and to measure only frequency
          or duty cycle of the input signal, or, Call TIM_PWMIConfig(TIMx, &TIM_ICInitStruct)
          to configure the desired channels with the corresponding configuration
          and to measure the frequency and the duty cycle of the input signal

      (#) Enable the NVIC or the DMA to read the measured frequency.

      (#) Enable the corresponding interrupt (or DMA request) to read the Captured
          value, using the function TIM_ITConfig(TIMx, TIM_IT_CCx)
          (or TIM_DMA_Cmd(TIMx, TIM_DMA_CCx))

      (#) Call the TIM_Cmd(ENABLE) function to enable the TIM counter.

      (#) Use TIM_GetCapturex(TIMx); to read the captured value.

      -@- All other functions can be used separately to modify, if needed,
          a specific feature of the Timer.

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	//TIM_ICStructInit(&TIM_ICInitStruct);

	TIM_ICInitTypeDef TIM_ICInitStruct;

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICFilter = 0x00;
	TIM_ICInit(TIM3, &TIM_ICInitStruct);
	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);

	TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICFilter = 0x00;
	TIM_ICInit(TIM3, &TIM_ICInitStruct);
	TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);

	TIM_Cmd(TIM3, ENABLE);

//	TIM_GetCapture1(TIM3); 			//Per llegirvalor (uint32_t)

}*/

