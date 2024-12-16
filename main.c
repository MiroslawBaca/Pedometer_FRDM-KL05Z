#include "uart.h" 
#include "i2c.h"
#include <math.h>


void SysTick_Handler(void);
void bus_scan (void);
void expander_looptest (uint8_t address);

static char temp[99];
static uint8_t arrayXYZ[6];


int main (void) { 
	
	
	UART_Init(9600);												/* inicjalizacja UART */ 
	I2C_Init();															/* inicjalizacja I2C na potrzeby komunikacji z sensorem*/
	

	int liczba_krokow = 0;
	int liczba_krokow_bieg = 0;
	

	double accelerationVectorOld=1.0;
	
	UART_Println("Rozpoczynam pomiar za 2 s ...");
	DELAY(300);
	UART_Println("   X       Y       Z");
	DELAY(2000);
	while(1) {
		
		//DELAY(1000);
		
		for(int xjd=1000; xjd>=1; xjd--){
			I2C_ReadRegBlock(0x1D, 0x01, 16, arrayXYZ);  //zczytywanie z sensora
		
			//wysylka UART
			double x_=((double)((int16_t)((arrayXYZ[0]<<8)|arrayXYZ[1])>>2)/4096);
			double y_=((double)((int16_t)((arrayXYZ[2]<<8)|arrayXYZ[3])>>2)/4096);
			double z_=((double)((int16_t)((arrayXYZ[4]<<8)|arrayXYZ[5])>>2)/4096);//czulosc domyslnie 4096 jednostek / g (przyspieszenie ziemskie)
			sprintf(temp,"%1.4f  %1.4f  %1.4f", x_, y_, z_);
			UART_Println(temp);


				
			double accelerationVector = sqrt(x_*x_+y_*y_+z_*z_);
			
			if (accelerationVectorOld-accelerationVector>=0.7){
				liczba_krokow_bieg++;
			} else if (accelerationVectorOld-accelerationVector>=0.2){
				liczba_krokow++;
			}
			
			accelerationVectorOld=accelerationVector;

			
			DELAY(100); 
			
		}
		
		__WFI();	
	} 
}
