#include "module4.h"

uint16_t data = 2048;

/* Configurem el DAC i habilitem el canal 2 per treure Vc*/
void Module4_Config(void){

	//Iniciar GPIO PA5
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Enable GPIOA clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	//PA5 - DAC_OUT2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	DAC_InitTypeDef DACInitStruct;

	DACInitStruct.DAC_Trigger = DAC_Trigger_None;	//DAC_Trigger_T4_TRGO, DAC_Trigger_Ext_IT9, DAC_Trigger_Software, DAC_Trigger_None
	DACInitStruct.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DACInitStruct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DACInitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;


	DAC_Init(DAC_Channel_2, &DACInitStruct);
	//vvvvvv Amb aquesta instruccio activarem o desactivarem DAC en funcio de velocitat
	//Sempre estara activat, cal inicialitzar variable velocitat abans
	DAC_Cmd(DAC_Channel_2, ENABLE);

	DAC_SetChannel2Data(DAC_Align_12b_R, data); //ficar variable 0<=velocitat<=4095

}
