#ifndef MODULE_ACC_H_
#define MODULE_ACC_H_


#include "stm32f4xx.h"
#include <stdio.h>
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stm32f429i_discovery.h"

char NewData;
int acc_x_g;
int acc_y_g;

void DMA2_MemtoMem(void);
void Module_Acceleration(void);

#endif /* MODULE_ACC_H_ */
