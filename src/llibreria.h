

#ifndef LLIBRERIA_H_
#define LLIBRERIA_H_

//#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_sdram.h"

typedef enum {
	NO_OK = 0,
	OK = !NO_OK
} RetSt;

RetSt SetPixel (uint16_t col, uint16_t fila, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval);
uint32_t GetPixel (uint16_t col, uint16_t fila);
RetSt DibuixaLiniaHoritzontal (uint16_t col_inici, uint16_t col_fi, uint16_t fila, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval);
RetSt DibuixaLiniaVertical (uint16_t col, uint16_t fila_inici, uint16_t fila_fi, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval);
RetSt DibuixaCircumferencia (uint16_t ccol, uint16_t cfila, uint16_t radi, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval);
RetSt DibuixaBitmap (uint16_t col, uint16_t fila, uint16_t pcol, uint16_t pfila);
RetSt EsborraPantalla (uint8_t Rval, uint8_t Gval, uint8_t Bval);


//http://www.lucadavidian.com/2017/10/02/stm32-using-the-ltdc-display-controller/

#endif /* LLIBRERIA_H_ */
