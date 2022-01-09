

/* Includes */
#include "stm32f4xx.h"
#include <stdio.h>
#include "stm32f4xx_dac.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_sdram.h"
#include "module1.h"
#include "module2.h"
#include "module3.h"
#include "module4.h"
#include "module_acc.h"
#include "llibreria.h"


#define FRAMEBUFFER_SIZE			 (LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT) //(4+4+4+4) = 2 bytes jeje?
#define IS42S16400J_SIZE            0x400000
#define TRANS_START_COL				203
#define TRANS_MID_ROW				45
#define LONG_START_ROW				102
#define LONG_MID_COL				44

#define XY_CROW					200
#define XY_CCOL					163


/* Private macro */
/* Private variables */

typedef struct{
	uint16_t col[200];
	uint16_t fil[200];
} AccGraph;

/* Private function prototypes */
/* Private functions */
int mean(int* p){
	int mean = 0;
	for(char i = 0; i<10; i++)
		mean += *(p++);
	return (int)mean/10;
}
/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/


int main(void)
{

	//Variables Initialisation
	static uint8_t numMostres = 0;
	static uint8_t Shift = 0;
	static int acc_long[10];
	static int acc_trans[10];
	static int XY_long[10];
	static int XY_trans[10];
	static uint8_t top = 0;
	static uint8_t XYtop = 0;
	static int mean_x, mean_y;
	static uint16_t XY_y_pos;
	static uint16_t XY_x_pos;
	static uint16_t pcol = 0;
	static uint16_t pfila = 0;

	AccGraph long_values;
	AccGraph trans_values;

	NewData = 0;
	acc_x_g = 0;
	acc_y_g = 0;

	/**
  *  IMPORTANT NOTE!
  *  The symbol VECT_TAB_SRAM needs to be defined when building the project
  *  if code has been located to RAM and interrupts are used.
  *  Otherwise the interrupt table located in flash will be used.
  *  See also the <system_*.c> file and how the SystemInit() function updates
  *  SCB->VTOR register.
  *  E.g.  SCB->VTOR = 0x20000000;
  */

  //P1
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);

  STM_EVAL_LEDOff(LED3);
  STM_EVAL_LEDOff(LED4);
  Module1_Config();
  Module3_Config();
  Module2_Config();
  tics = 0;
  Module4_Config();

  //P2
  Module_Acceleration();

  //P3
  LCD_Init();
  LCD_LayerInit();
  LTDC_Cmd(ENABLE);

  // Print Layer 1 --> Axes and circle
  EsborraPantalla(0x0F, 0x0F, 0x0F);


  /* Infinite loop */
  while (1)
  {
	// Checks whether the TIM3 interrupt has occurred or not
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

		TIM_ClearFlag(TIM3, TIM_IT_Update);
		// Clears the TIM3 interrupt pending bit
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}

	//P3
	if(NewData){
	  //Build up to 10 samples
	  acc_long[top] = acc_x_g;
	  acc_trans[top++] = acc_y_g;
	  top = top%10;
	  if(top == 0){

		  //Mean of 10 samples
		  mean_x = mean(acc_long);
		  mean_y = mean(acc_trans);

		  /*Trans and long graphs*/

		  //Shifting graphs
		  if(Shift){
			  for(uint8_t i = 0; i < numMostres; i++){
				  //SetPixel(long_values.col[i], long_values.fil[i], 0x00, 0x00, 0x00, 0x00);
				  *(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*long_values.fil[i] + long_values.col[i])+1) = 0x00;
				  long_values.col[i] = long_values.col[i+1];
				  SetPixel(long_values.col[i], long_values.fil[i], 0x0F, 0x00, 0x00, 0x0F);

				  //SetPixel(trans_values.col[i], trans_values.fil[i], 0x00, 0x00, 0x00, 0x00);
				  *(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*trans_values.fil[i] + trans_values.col[i])+1) = 0x00;
				  trans_values.fil[i] = trans_values.fil[i+1];
				  SetPixel(trans_values.col[i], trans_values.fil[i], 0x0F, 0x0F, 0x00, 0x00);
			  }

			  //Deletes any data on previous last position
			  *(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*trans_values.fil[numMostres] + trans_values.col[numMostres])+1) = 0x00;
			  *(__IO uint8_t*) (SDRAM_BANK_ADDR + BUFFER_OFFSET + 2*(240*long_values.fil[numMostres] + long_values.col[numMostres])+1) = 0x00;
		  }

		  long_values.col[numMostres] = (int)(mean_x/100) + LONG_MID_COL;
		  long_values.fil[numMostres] = LONG_START_ROW + numMostres;

		  trans_values.col[numMostres] = TRANS_START_COL - numMostres;
		  trans_values.fil[numMostres] = (int)(mean_y/100) + TRANS_MID_ROW;

		  SetPixel(trans_values.col[numMostres], trans_values.fil[numMostres], 0x0F, 0x0F, 0x00, 0x00);
		  SetPixel(long_values.col[numMostres], long_values.fil[numMostres], 0x0F, 0x00, 0x00, 0x0F);

		  /*X-Y Diagram*/
		  XY_trans[XYtop] = mean_y;
		  XY_long[XYtop++] = mean_x;
		  XYtop = XYtop%10;
		  if(XYtop == 0){
			  XY_y_pos = (int)((int)(mean(XY_trans)/100)*3/2) + XY_CCOL;
			  XY_x_pos = (int)((int)(mean(XY_long)/100)*3/2) + XY_CROW;
			  DibuixaBitmap(XY_y_pos, XY_x_pos, pcol, pfila);
			  pcol = XY_y_pos;
			  pfila = XY_x_pos;
		  }

		  //Decides when to shift
		  if(numMostres == 199)
			  Shift = 1;
		  else if(numMostres < 199)
			  numMostres++;

	  }

	  NewData = 0;
	}

  }

}


/*
 * Callback used by stm324xg_eval_i2c_ee.c.
 * Refer to stm324xg_eval_i2c_ee.h for more info.
 */
uint32_t sEE_TIMEOUT_UserCallback(void)
{
  /* TODO, implement your code here */
  while (1)
  {
  }
}
