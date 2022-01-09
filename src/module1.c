/*
 * module1.c
 */

#include "module1.h"
uint16_t interrupt_counter = 0;
uint16_t dataDAC;

/* Canvia l'estat del LED i calcula velocitat assignant-la al DAC */
void TIM4_IRQHandler(){

	//Led Toggle
	interrupt_counter++;
	if(interrupt_counter == 200){
		interrupt_counter = 0;

		STM_EVAL_LEDToggle(LED3);
	}

	//Càlcul velocitat roda dreta
	if(!zeroR)
	//if (speed < 3)
			speedR = 0;
	//else if (speed >= 3 || wd < 2)
			//speedR = 0;
	else
		speedR = getSpeed(pre_tR, post_tR);
	//Càlcul velocitat roda esquerre
	if (!zeroL)
	//if(speed < 3)
		speedL = 0;
	//else if (speed >= 3 || we < 2)
		//speedL = 0;
	else
		speedL = getSpeed(pre_tL, post_tL);
	//Actualització valor analògic
	dataDAC = getDAC_Data(speedR, speedL);
	DAC_SetChannel2Data(DAC_Align_12b_R, dataDAC);


	//MemToMem transfer
	DMA2_MemtoMem();

	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}


/* Configuració TIM4 (1ms) */
void Module1_Config(void)
{

	//90MHz
  /* TIM4 clock enable - Module 1*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 89;
  TIM_TimeBaseStructure.TIM_Prescaler = 999;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  //TIM4 initialise
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  //Enable TIM4 interrupt
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  //Start TIM4
  TIM_Cmd(TIM4, ENABLE);

  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

}


