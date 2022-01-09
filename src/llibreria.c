
#include "llibreria.h"

#define IS42S16400J_SIZE	0x400000
#define BUFFER_OFFSET		0x50000

uint16_t color = 0x0000;

uint16_t getColor(uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval){
	color = alfa;
	color = color<<4&0xFFF0;
	color |= Rval;
	color = color<<4&0xFFF0;
	color |= Gval;
	color = color<<4&0xFFF0;
	color |= Bval;
	return color;
}

RetSt SetPixel (uint16_t col, uint16_t fila, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval){
	//Writes on Layer2
	//Color value from parameters will be on 4LSB, rest 0s
	color = getColor(alfa, Rval, Gval, Bval);
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*fila + col)) = color;

	if((uint16_t)GetPixel(col, fila) == color)
		return OK;
	else return NO_OK;
}


uint32_t GetPixel (uint16_t col, uint16_t fila){
	//Reads from Layer2
	uint32_t color32 = 0x00000000;
	color32 = *(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*fila + col));
	if(color32 != 0)
		color32 |= 0xFFFF0000;
	return color32;
}


RetSt DibuixaLiniaHoritzontal (uint16_t col_inici, uint16_t col_fi, uint16_t fila, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval){
	//Writes on Layer1
	//Checks parameters correctness
	if ((0 > col_inici) || (239 < col_inici) || (0 > col_fi) || (239 < col_fi) || col_inici == col_fi)
		return NO_OK;

	color = getColor(alfa, Rval, Gval, Bval);

	if(col_inici < col_fi){
		for(char i = col_inici; i <= col_fi; i++){
			*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*fila + i)) = color;
		}
	}else if(col_inici > col_fi){
		//Inverse loop (from great to minor)
		for(char i = col_inici; i >= col_fi; i--){
			*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*fila + i)) = color;
		}
	}
	return OK;

}



RetSt DibuixaLiniaVertical (uint16_t col, uint16_t fila_inici, uint16_t fila_fi, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval){
	//Writes on Layer1
	if ((0 > fila_inici) || (319 < fila_inici) || (0 > fila_fi) || (319 < fila_fi) || fila_inici == fila_fi)
			return NO_OK;

	color = getColor(alfa, Rval, Gval, Bval);

	if(fila_inici < fila_fi){
		for(uint16_t i = fila_inici; i <= fila_fi; i++){
			*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*i + col)) = color;
		}
	}else if(fila_inici > fila_fi){
		//Inverse loop (from great to minor)
		for(uint16_t i = fila_inici; i >= fila_fi; i--){
			*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*i + col)) = color;
		}
	}
	return OK;
}



RetSt DibuixaCircumferencia (uint16_t ccol, uint16_t cfila, uint16_t radi, uint8_t alfa, uint8_t Rval, uint8_t Gval, uint8_t Bval){
	//Writes on Layer1
	if((ccol - radi) < 0 || (ccol + radi) > 239 || (cfila - radi) < 0 || (cfila + radi) > 319)
		return NO_OK;

	color = getColor(alfa, Rval, Gval, Bval);

	int x, y, d;
	x = 0;
	y = radi;
	d = 3 - (radi << 1);

	while(x <= y){

		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(y + cfila) + (x + ccol))) = color;
		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(y + cfila) + (-x + ccol))) = color;
		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(-y + cfila) + (x + ccol))) = color;
		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(-y + cfila) + (-x + ccol))) = color;
		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(x + cfila) + (y + ccol))) = color;
		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(x + cfila) + (-y + ccol))) = color;
		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(-x + cfila) + (y + ccol))) = color;
		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*(240*(-x + cfila) + (-y + ccol))) = color;

		if(d<0)
			d += (x<<2) + 6;
		else{
			d += ((x-y)<<2) + 10;
			y--;
		}
	x++;
	}
	return OK;
}

/**
 * pcol, pfila, previous bitmap position
 */
RetSt DibuixaBitmap (uint16_t col, uint16_t fila, uint16_t pcol, uint16_t pfila){
	//Writes on Layer2
	//Checks parameters correctness
	if ((0 > col) || (239 < col) || (0 > fila) || (319 < fila))
		return NO_OK;

	if(pcol != 0 && pfila != 0){
		//Deletes previous bitmap
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila-1) + (pcol-1))+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila-1) + pcol)+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila-1) + (pcol+1))+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila) + (pcol-1))+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila) + pcol)+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila) + (pcol+1))+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila+1) + (pcol-1))+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila+1) + pcol)+1) = 0x00;
		*(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(pfila+1) + (pcol+1))+1) = 0x00;

	}
	color = 0xF0F0;	//Green

	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila-1) + (col-1))) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila-1) + col)) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila-1) + (col+1))) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila) + (col-1))) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila) + col)) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila) + (col+1))) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila+1) + (col-1))) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila+1) + col)) = color;
	*(__IO uint16_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*(fila+1) + (col+1))) = color;

	return OK;
}


RetSt EsborraPantalla (uint8_t Rval, uint8_t Gval, uint8_t Bval){
	//Overwrites all data on SDRAM
	color = getColor(0x00, Rval, Gval, Bval);
    for (uint32_t counter = 0x00; counter < IS42S16400J_SIZE; counter++)
    {
    	if(counter >= BUFFER_OFFSET)
    		*(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*counter) = color&0x0FFF;
    	else *(__IO uint16_t*) (SDRAM_BANK_ADDR + 2*counter) = color;

    }

    DibuixaLiniaHoritzontal(3, 204, 4, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaHoritzontal(3, 204, 45, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaHoritzontal(3, 204, 86, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaHoritzontal(3, 85, 101, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaHoritzontal(3, 85, 302, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaHoritzontal(103, 223, 200, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaVertical (3, 4, 86, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaVertical (204, 4, 86, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaVertical (3, 101, 302, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaVertical (44, 101, 302, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaVertical (85, 101, 302, 0x0F, 0x00, 0x00, 0x00);
    DibuixaLiniaVertical (163, 140, 260, 0x0F, 0x00, 0x00, 0x00);

    DibuixaCircumferencia(163, 200, 15, 0x0F, 0x00, 0x00, 0x00);
    DibuixaCircumferencia(163, 200, 30, 0x0F, 0x00, 0x00, 0x00);
    DibuixaCircumferencia(163, 200, 45, 0x0F, 0x00, 0x00, 0x00);
    DibuixaCircumferencia(163, 200, 60, 0x0F, 0x00, 0x00, 0x00);

    return OK;
}
