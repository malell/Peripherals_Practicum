/*
 * module2.h
 *
 *  Created on: 11 Apr 2021
 *      Author: legar
 */

#ifndef MODULE2_H_
#define MODULE2_H_

#include "stm32f4xx.h"
#include <stdio.h>
#include "stm32f4xx_dac.h"
#include "stm32f429i_discovery.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"

uint8_t zeroR, zeroL;
uint16_t pre_tR, post_tR, pre_tL, post_tL;
uint16_t speedR, speedL;
uint16_t flag;
uint16_t tics;

void Module2_Config(void);
uint16_t getSpeed(uint16_t pre_t, uint16_t post_t);
uint16_t getDAC_Data(uint16_t speedR, uint16_t speedL);

#endif /* MODULE2_H_ */
