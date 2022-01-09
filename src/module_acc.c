
#include <module_acc.h>

#define NUMDATA 16
__IO uint16_t MemtoMemValues[NUMDATA];
__IO uint16_t PeriphtoMemValues[NUMDATA];


//Utilitzar independent mode - Scan (Multichannel) amb trigger un timer de 50us

/**
 * DMA PeriphtoMem transfer completed
 */
void DMA2_Stream0_IRQHandler(void) {
	if(DMA_GetITStatus(DMA2_Stream0,DMA_IT_TCIF0  )!=RESET) {
		ADC_Cmd(ADC1, DISABLE);
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0 );
		//DMA_Cmd(DMA2_Stream0,DISABLE);
		//DMA_Cmd(DMA2_Stream1,ENABLE);

	}
}


/**
 * DMA Block transfer MemtoMem
 */
void DMA2_MemtoMem(void) {
	DMA_Cmd(DMA2_Stream1,ENABLE);
	DMA_SetCurrDataCounter(DMA2_Stream1, NUMDATA);
	ADC_Cmd(ADC1, ENABLE);
}


/**
 * DMA MemtoMem transfer completed
 */
void DMA2_Stream1_IRQHandler(void) {
	if( DMA_GetITStatus(DMA2_Stream1,DMA_IT_TCIF1)!=RESET ) {
		//Acceleration calculation
		uint16_t acc_x = 0;
		uint16_t acc_y = 0;


		for(int i = 0; i < NUMDATA; i++){
			if(i%2 == 0){
				acc_x = acc_x + MemtoMemValues[i];
			}else{
				acc_y = acc_y + MemtoMemValues[i];
			}
		}

		//3 float resolution
		acc_x = (acc_x/8)*3000/4096;
		acc_y = (acc_y/8)*3000/4096;


		//Line equation y=mx+n : m = 1/4, n=+1
		//We want x -> 'g'
		if(acc_x >= 2000) acc_x_g = 4000;
		else acc_x_g = acc_x*4-4000;
		if(acc_y >= 2000) acc_y_g = 4000;
		else acc_y_g = acc_y*4-4000;

		NewData = 1;	//Podem llegir mostra per P3
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
		DMA_Cmd(DMA2_Stream1,DISABLE);
	}
}

//void TIM2_IRQHandler(){
//	uint16_t tim = TIM_GetCounter(TIM2);
//}

void TIM2_config(void){
	//Timer de 50us

	//90MHz
  /* TIM2 clock enable*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 4499;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  //TIM2 initialise
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  //Enable TIM2 interrupt
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  //Start TIM2
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
  TIM_Cmd(TIM2, ENABLE);

//  NVIC_InitTypeDef NVIC_InitStruct;
//  NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
//  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStruct);
}


void DMA_CPU_config(void){
	DMA_InitTypeDef  DMA_InitStructure;
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) PeriphtoMemValues;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) MemtoMemValues;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
	DMA_InitStructure.DMA_BufferSize = (uint32_t) NUMDATA;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
	DMA_ITConfig(DMA2_Stream1,DMA_IT_TC,ENABLE);

	NVIC_InitTypeDef nvicStructure;
 	nvicStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
 	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
 	nvicStructure.NVIC_IRQChannelSubPriority = 2;
 	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
 	NVIC_Init(&nvicStructure);
}

void ADC1_DMA_config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* Configure ADC1 Channel11 and 13 pin as analog input ******************************/
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&PeriphtoMemValues;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)NUMDATA;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC,ENABLE);
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  NVIC_InitTypeDef nvicStructure;
  nvicStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  nvicStructure.NVIC_IRQChannelPreemptionPriority = 1;
  nvicStructure.NVIC_IRQChannelSubPriority = 1;
  nvicStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicStructure);




  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC1 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel10&11 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_15Cycles); //PC1
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 2, ADC_SampleTime_15Cycles); //PC3

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);


  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  ADC_SoftwareStartConv(ADC1);

}

void Module_Acceleration(void){
	TIM2_config();
	ADC1_DMA_config();
	DMA_CPU_config();
}
