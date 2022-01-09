/*
 * module3.c
 */

#include "module3.h"

#define MAXSPEED	6
#define	DIF_SPEED	5

// Matriu de valors de període
const uint16_t relativeSpeed[DIF_SPEED][MAXSPEED] = {{0, 2037, 578, 449, 202, 74},
								{0, 1629, 462, 359, 161, 59},
								{0, 1509, 428, 332, 149, 55},
								{0, 1131, 320, 249, 112, 40},
								{0, 925, 262, 204, 91, 33}};

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCStruct;

/* Interrupció USER BUTTON*/
void EXTI0_IRQHandler(void)
{

	STM_EVAL_LEDOn(LED4);
	static uint16_t speedup = 1;
	static uint16_t upndown = 1;
	static uint16_t we = 0;
	static uint16_t wd = 0;
	static uint16_t speed = 0;
	static uint16_t wenwd = 1;

	if(EXTI_GetITStatus(EXTI_Line0) != RESET){

		//Comprovar estat de la GPIO auxiliar
		if(!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)){

			//Incrementem velocitat cotxe
			if(speed == MAXSPEED-1)
				speedup = 0;
			else if(speed == 0)
				speedup = 1;
			if(speedup) speed++;
			else speed--;

		}else if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)){

			//Canviem relacio rotacio rodes we = x*wd (PF6 = PE5)
			if(wenwd){
				if(we == DIF_SPEED-1)
					upndown = 0;
				else if(we == 0){
					upndown = 1;
				}
				if(upndown) we++;
				else we--;
				if(we == 0) wenwd = 0;
			}else{
				if(wd == DIF_SPEED-1)
					upndown = 0;
				else if(wd == 0)
					upndown = 1;
				if(upndown) wd++;
				else wd--;
				if(wd == 0) wenwd = 1;
			}
		}

		/* Si velocitat és zero, posem el duty-cycle absolut, sino 50%*/
		if(speed == 0){
			//pulse_length = ((TIM_Period + 1) * 50) / 100 - 1 = x
			TIM_OCStruct.TIM_Pulse = ((relativeSpeed[we][speed] + 1) * 100) / 100 -1; //x
			TIM_OC1Init(TIM9, &TIM_OCStruct);
			TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);

			TIM_OCStruct.TIM_Pulse = ((relativeSpeed[wd][speed] + 1) * 100) / 100 -1;
			TIM_OC1Init(TIM10, &TIM_OCStruct);
			TIM_OC1PreloadConfig(TIM10, TIM_OCPreload_Enable);
		}else {

			TIM_SetAutoreload(TIM9, relativeSpeed[we][speed]);

			TIM_SetAutoreload(TIM10, relativeSpeed[wd][speed]);

			//pulse_length = ((TIM_Period + 1) * 50) / 100 - 1 = x
			TIM_OCStruct.TIM_Pulse = ((relativeSpeed[we][speed] + 1) * 50) / 100 -1; //x
			TIM_OC1Init(TIM9, &TIM_OCStruct);
			TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);

			TIM_OCStruct.TIM_Pulse = ((relativeSpeed[wd][speed] + 1) * 50) / 100 -1;
			TIM_OC1Init(TIM10, &TIM_OCStruct);
			TIM_OC1PreloadConfig(TIM10, TIM_OCPreload_Enable);
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line0);

	STM_EVAL_LEDOff(LED4);
}

void Module3_Config(void)
{
  /*EXTI_InitTypeDef   EXTI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  // Enable GPIOA clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  // Enable SYSCFG clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  // Configure PA0 pin as input floating
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // Connect EXTI Line0 to PA0 pin
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

  // Configure EXTI Line0
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  // Enable and set EXTI Line0 Interrupt to the lowest priority
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);*/

	//Configurem el polsador del usuari
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

	//Configurem el GPIOs
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Enable GPIOC clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//PC8 GPIOaux
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOC, &GPIO_InitStructure);



	/************/
	/*	PWMs	*/
	/************/

	/* Inicialitzem pins de sortida */
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Clock for GPIOs */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //PE5
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); //PF6

	/* Alternating functions for pins */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_TIM9);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource6, GPIO_AF_TIM10);

	/* Set pins */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOF, &GPIO_InitStruct);


	//Necessitem dos timers ja que els periodes de cada PWM poden ser diferents

	/* Configurem el TIM9 pels PWM */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = relativeSpeed[0][1];
	TIM_TimeBaseStructure.TIM_Prescaler = 1999;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	//TIM9 initialise
	TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);
	//Start TIM9
	TIM_Cmd(TIM9, ENABLE);

	/* Configurem el TIM10 pels PWM */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = relativeSpeed[0][1];
	TIM_TimeBaseStructure.TIM_Prescaler = 1999;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	//TIM10 initialise
	TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStructure);
	//Start TIM10
	TIM_Cmd(TIM10, ENABLE);


	/* Inicialitzem els PWM en els Channel 1 de cada Timer*/
	// TIM9_CH1 -> PE5 (we)  ||  TIM10_CH1 -> PF6 (wd)

	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_High;

	//pulse_length = ((TIM_Period + 1) * 50) / 100 - 1 = x
	TIM_OCStruct.TIM_Pulse = 0; //x
	TIM_OC1Init(TIM9, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);

	TIM_OCStruct.TIM_Pulse = 0; //x
	TIM_OC1Init(TIM10, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM10, TIM_OCPreload_Enable);


}
