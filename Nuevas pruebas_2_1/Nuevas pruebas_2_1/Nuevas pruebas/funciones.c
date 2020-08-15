#include "funciones.h"

	///////////////////////////
	/////   DECLARACION   /////
	/////			  DE				/////
	/////    FUNCIONES  	/////
	///////////////////////////

void set_servo(uint8_t grados)		// Mueve el servo
{
    LPC_PWM1->MR2	 = (Fpclk*0.65e-3 + Fpclk*2e-3*grados/180); // Tiempo en alto
		LPC_PWM1->LER	|= (1<<2)|(1<<0);														// Load Register
	
}//set_servo

void generar_pulso_alto(void)    // Genera un pulso de 10 us para el trigger
{
		LPC_TIM2->TCR     = (1<<0); 		// EMPIEZA A CONTAR
		LPC_GPIO1->FIOSET = (1U<<31); 	// P1.31 a 1 (high)
	
}	//generar_pulso_alto

void genera_muestras(uint16_t muestras_ciclo)	// Genera el seno
{
		uint16_t i;
		for(i=0; i<muestras_ciclo; i++)
		{
			muestras[i]=(uint32_t)1023*(0.5+0.5*sin(2*PI*i/muestras_ciclo));		//El DAC es de 10 bits
		}
} //genera_muestras

float leer_Temperatura_I2C()    // Lee la temperatura del sensor de I2C
{
		//int temp=0;								// Variable auxiliar
		I2CSendAddr(0x48, 0);       // Inicio de la comunicación
    I2CSendByte(0xEE);          // Inicio de la conversión continua
    I2CSendStop();              // Fin de la comunicación	
		
		I2CSendAddr(0x48, 0);				// Direccion para el DS1621: 000 y modo escritura
		I2CSendByte(0xAA);					// Leer temperatura
		I2CSendAddr(0x48, 1);				// Modo lectura
		
		temp = I2CGetByte(0);
		I2CSendStop();
	
		if(I2CGetByte(1)==0)
		{
			temperaturaI2C = (float)temp;		   // Si el bit7=0, el valor es directo
		}
		else
		{
			temperaturaI2C = (float)temp+0.5;  // Si el bit7=1, es necesario sumar 0.5º
		}

		I2CSendStop();										   // Termina la comunicacion

		return temperaturaI2C;		
}// Leer_temperatura_I2C
	
void mostrar_resultados()			// Muestra los resultados por pantalla
{

//		GUI_Text(30 ,145, "A",CYAN,BLACK);
//		sprintf(linea1, "B",    grados);
//		drawString(30, 145, "A", WHITE, BLACK, MEDIUM);
//		drawString(70, 165, linea1, WHITE, BLACK, MEDIUM);
	
		sprintf(linea1, "%d grados          ",    grados);
		//sprintf(linea2, "%.2f cm"  ,    distancia);
		sprintf(linea3, "%3.2f grados        ", temperatura_global);
		sprintf(linea4, "%3.2f grados         ", temperaturaI2C);

		drawString(30, 125, "Medidas realizadas:", WHITE, BLACK, MEDIUM);
		
		drawString(30, 145, "Angulo del servo:", WHITE, BLACK, MEDIUM);
		drawString(70, 165, linea1, WHITE, BLACK, MEDIUM);
		
		//drawString(30, 185, "Distancia:", WHITE, BLACK, MEDIUM);
		//drawString(70, 205, linea2, WHITE, BLACK, MEDIUM);
		
		drawString(30, 225, "Temperatura LM35:", WHITE, BLACK, MEDIUM);
		drawString(70, 245, linea3 , WHITE, BLACK, MEDIUM);
		
		drawString(30, 265, "Temperatura I2C: ", WHITE, BLACK, MEDIUM);
		drawString(70, 285, linea4, WHITE, BLACK, MEDIUM);
		
} // Mostrar_resultados

void mostrar_medidas_uart()			//COLOCAR EN EL IRQ DEL TIMER 0 (del que interrumpe cada 0.5 seg)- ADC_HANDLER
{
//		sprintf(buffer_temp_glob, "\n Temperatura LM35 : %3.2f grados        ", temperatura_global);
//		tx_cadena_UART0(buffer_temp_glob);
//		while(tx_completa==0);
//		tx_completa=0;					//Se transmite la cadena de buffer	
//		sprintf(buffer_temp_I2C, "\n Temperatura I2C : %3.2f grados        ", temperaturaI2C);
//		tx_cadena_UART0(buffer_temp_I2C);
//		while(tx_completa==0);
//		tx_completa=0;					//Se transmite la cadena de buffer				
}//mostrar_medidas_uart
