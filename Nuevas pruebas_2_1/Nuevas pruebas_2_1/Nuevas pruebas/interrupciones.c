#include "interrupciones.h"
#define Fpclk 25e6

	//////////////////////////////
	/////   INTERRUPCIONES   /////
	//////////////////////////////

void EINT0_IRQHandler() 	 		// KEY ISP (P2.10)
{
	LPC_SC->EXTINT |= (1<<0);		// Borramos flag interrupcion
	FLAG_ISP        = 1;				// Activamos flag del boton
	
	if(FLAG_EINT0_PULSO)
		generar_pulso_alto();     // Enviamos pulso del trigger

}// EINT0_IRQHandler

void EINT1_IRQHandler()	   		// KEY 1 (P2.11)
{
	LPC_SC->EXTINT |= (1<<1);		// Borramos flag interrupción
	FLAG_KEY1       = 1;				// Activamos flag del boton
	
	if(FLAG_EINT_SERVO&&(grados>0))
			set_servo(grados-=10);	//se mueve -10º
}//EINT1_IRQHandler

void EINT2_IRQHandler()	//key2 (P2.12)
{
	LPC_SC->EXTINT |= (1<<2);		// Borramos flag de la interrupcion
	FLAG_KEY2 = 1;						  // Activamos flag del boton
	
	if(FLAG_EINT_SERVO&&(grados<180))
			set_servo(grados+=10);//se mueve +10º
}//EINT2_IRQHandler

void TIMER0_IRQHandler(void)	//TIMER0 (cada 0,5 seg)
{
	LPC_TIM0->IR|=(1<<0);								//limpiamos flag MR0
	
	FLAG_TIMER0 = 1;
	
	temp_I2C=leer_Temperatura_I2C();
	LPC_ADC->ADCR|=(1<<24);							//Que empiece la conversion ahora

	
	if(FLAG_DISPARO_CONTINUO==1)// Si es modo continuo
		generar_pulso_alto();     // Enviamos pulso del trigger
	
	else if((FLAG_AUTO==1)&&(grados<=180))
	{
		generar_pulso_alto();     // Enviamos pulso del trigger
		if(grados == 180)					// Si hemos llegado al final
			set_servo(grados = 0);	// Volver a 0
		
		else
			set_servo(grados += 10);// Mover servo
	}
	
	else if(FLAG_DETEC == 1)	//modo deteccion obstaculos
	{
		generar_pulso_alto();    // Enviamos pulso del trigger
		
		if(distancia <= umbral)  // Si esta dentro del umbral
		{
			frecuencia = 5000-umbral*10;								  		//Aplico la formula dada
			if (DAC_ON == 0)			 // si no esta sonando
			{
				LPC_TIM1->MR0 = (Fpclk/frecuencia/N_muestras)-1;		//Saco la frecuencia por el Match
				LPC_TIM1->TCR = (1<<0)|(0<<1);											// Habilitamos TIMER1		
				DAC_ON = 1;																					// Sonando
			}
		}
		else if (DAC_ON == 1) //Si estaba sonando y fuera del umbral
		{
			LPC_TIM1->TCR = (0<<0)|(1<<1);			// Deshabilitamos timer1
			DAC_ON = 0;     										// Deja de sonar							
		}
		if(FLAG_TIMER0 && FLAG_OFFLINE && (FLAG_DETEC))
		{
			FLAG_TIMER0 = 0;
			mostrar_resultados_DAC();		
		}
		
	}

}//TIMER0_IRQHandler

void TIMER1_IRQHandler(void)												// TIMER del DAC
{
		static uint8_t indice_muestra;
		FLAG_TIMER1 = 1;
	
		LPC_TIM1->IR|=(1<<0);														// Borro el flag de la interrupcion
		LPC_DAC->DACR=muestras[indice_muestra++]<<6;		// Desplazamient bit6...bit15 para colocarlos bien
		indice_muestra &= 0x1F;
	
}

void TIMER2_IRQHandler(void)					// TIMER pulso alto
{																	    // Tras los 10us del pulso en High, queremos que vuelva a 0
		FLAG_TIMER2 = 1;
	
		LPC_TIM2->IR|= (1<<1);						// Borrar el flag de interrupción del TIMER0
		LPC_GPIO1->FIOCLR =(1U<<31); 			// Limpiar el pin
		LPC_TIM2->TCR = 0; 								// Apagar Timer 2
}//TIMER2_IRQHandler

void TIMER3_IRQHandler(void) 													//TIMER para el Capture			
{
		float dif_pulsos; 	
		FLAG_TIMER3 = 1;
	
		LPC_TIM3->IR|=(1<<4); 														// Reset captura de CR0
		LPC_TIM3->IR|=(1<<5); 														// Reset captura de CR1
		dif_pulsos=((LPC_TIM3->CR1)-(LPC_TIM3->CR0)); 		// Calcular  diferencia del ancho de pulso
		distancia=dif_pulsos*0.017+0.00003*temperatura_global; 			// Formula distancia final + incremento velocidad del sonido con temperatura

}		//TIMER3_IRQHandler	

void ADC_IRQHandler(void)
{
	float temperatura;
	if(FLAG_TIMER0 && FLAG_OFFLINE && (!FLAG_DETEC))
	{
		FLAG_TIMER0 = 0;
		mostrar_resultados();
		//FLAG_ADC= 1;
	}
	
	else if(FLAG_TIMER0 && FLAG_ONLINE) //&& (FLAG_DETEC)
	{
		FLAG_TIMER0 = 0;
		mostrar_medidas_uart();
	}
	temperatura=((LPC_ADC->ADDR2>>4)&0xFFF)/4095.0*3300/10.0;
	
	if(temperatura_global != temperatura)	//si la nueva temperatura es diferente que la anterior
		temperatura_global=temperatura;
	
}
