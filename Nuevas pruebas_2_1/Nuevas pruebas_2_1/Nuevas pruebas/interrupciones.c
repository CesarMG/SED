#include "interrupciones.h"
#define Fpclk 25e6

	//////////////////////////////
	/////   INTERRUPCIONES   /////
	//////////////////////////////

void EINT0_IRQHandler() 	 // KEY ISP (P2.10)
{
	LPC_SC->EXTINT |= (1<<0);		 // Borramos flag interrupcion
	FLAG_ISP        = 1;				 // Activamos flag del boton
	
	if ((mode==111)||(mode==13))
		generar_pulso_alto();

}// EINT0_IRQHandler

void EINT1_IRQHandler()	   // KEY 1 (P2.11)
{
	LPC_SC->EXTINT |= (1<<1);		// Borramos flag interrupción
	FLAG_KEY1       = 1;				// Activamos flag del boton
	
	if(((mode==111)||(mode==112)||(mode==13))&&(grados>0))
			set_servo(grados-=10);//se mueve -10º
	
}//EINT1_IRQHandler

void EINT2_IRQHandler()	//key2 (P2.12)
{
	LPC_SC->EXTINT |= (1<<2);		// Borramos flag de la interrupcion
	FLAG_KEY2 = 1;						  // Activamos flag del boton
	
	if(((mode==111)||(mode==112)||(mode==13))&&(grados<180))
			set_servo(grados+=10);//se mueve +10º
}//EINT2_IRQHandler

void TIMER0_IRQHandler(void)	//TIMER0 (cada 0,5 seg)
{
	LPC_TIM0->IR|=(1<<0);	//limpiamos flag MR0
	
	FLAG_TIMER0 = 1;
	nevera_abierta = 1;
	
	prueba=leer_Temperatura_I2C();
	LPC_ADC->ADCR|=(1<<24);									//Que empiece la conversion ahora

	if((mode==112)&&(FLAG_ISP==1))
	{
		generar_pulso_alto();
	}
	if((mode==12)&&(grados<=190))
	{
		generar_pulso_alto();
		set_servo(grados+=10);
		if(grados==190)
		{
			set_servo(grados=0);		
		}
	}
	if((mode==212)&&(disparo==1))
		generar_pulso_alto();
	
	if((mode==22)&&(grados<=190))
	{
		generar_pulso_alto();
		set_servo(grados+=10);
		if(grados==190)
		{
			set_servo(grados=0);		
		}
	}

	if((mode==13)||(mode==23))	//modo deteccion obstaculos
	{
		generar_pulso_alto();
		
		if(distancia <= umbral)
		{
			frecuencia    = 5000-umbral*10;								  		//Aplico la formula dada
			if (DAC_ON == 0)
			{
				LPC_TIM1->MR0 = (Fpclk/frecuencia/N_muestras)-1;		//Saco la frecuencia por el Match
				LPC_TIM1->TCR = (1<<0)|(0<<1);															// Habilitamos TIMER1		
				DAC_ON = 1;
			}
		}
		else if (DAC_ON == 1)
		{
			LPC_TIM1->TCR = (0<<0)|(1<<1);			// Deshabilitamos time1
			DAC_ON = 0;
		}
		
		//mostrar_resultados();
	}
	
//	if((mode==23)&&(disparo==1))
//	{
//		generar_pulso_alto();
//	}
	

	//mostrar_resultados();
}//TIMER0_IRQHandler

void TIMER1_IRQHandler(void)			//TIMER del DAC
{
		static uint8_t indice_muestra;
		FLAG_TIMER1 = 1;
	
		LPC_TIM1->IR|=(1<<0);														//Borro el flag de la interrupcion
		LPC_DAC->DACR=muestras[indice_muestra++]<<6;		//Desplazamient bit6...bit15 para colocarlos bien
		indice_muestra &= 0x1F;
}

void TIMER2_IRQHandler(void)			//TIMER pulso alto
{																	    // Tras los 10us del pulso en High, queremos que vuelva a 0
		FLAG_TIMER2 = 1;
	
		LPC_TIM2->IR|= (1<<1);						// Borrar el flag de interrupción del TIMER0
		LPC_GPIO1->FIOCLR =(1U<<31); 			// Limpiar el pin
		LPC_TIM2->TCR = 0; 								// Apagar Timer 2
}//TIMER2_IRQHandler

void TIMER3_IRQHandler(void) 			//TIMER para el Capture			
{
		float dif_pulsos; 	
		FLAG_TIMER3 = 1;
	
		LPC_TIM3->IR|=(1<<4); 						// Reset captura de CR0
		LPC_TIM3->IR|=(1<<5); 						// Reset captura de CR1
		dif_pulsos=((LPC_TIM3->CR1)-(LPC_TIM3->CR0)); 		// Calcular  diferencia del ancho de pulso
		distancia=dif_pulsos*0.017+0.00003*temperatura_global; 			// Formula distancia final + incremento velocidad del sonido con temperatura
		
		if(((mode==111)||(mode==112)||(mode==12))&&((distancia>30)&&(distancia<300)))
		{
			sprintf(linea2, "%.2f cm        "  ,    distancia);
			
			drawString(30, 185, "\n Distancia:", WHITE, BLACK, MEDIUM);
			drawString(70, 205, linea2, WHITE, BLACK, MEDIUM);
		}
		else if((mode==13)||(mode==211)||(mode==212)||(mode==22))
		{
			sprintf(linea1, "%d grados          ",    grados);
			sprintf(linea2, "%.2f cm"  ,    distancia);
			sprintf(linea3, "%3.2f grados        ", temperatura_global);
			sprintf(linea4, "%3.2f grados         ", temperaturaI2C);

			drawString(30, 125, "Medidas realizadas:", WHITE, BLACK, MEDIUM);
			
			drawString(30, 145, "Angulo del servo:", WHITE, BLACK, MEDIUM);
			drawString(70, 165, linea1, WHITE, BLACK, MEDIUM);
			
			drawString(30, 185, "Distancia:", WHITE, BLACK, MEDIUM);
			drawString(70, 205, linea2, WHITE, BLACK, MEDIUM);
			
			drawString(30, 225, "Temperatura LM35:", WHITE, BLACK, MEDIUM);
			drawString(70, 245, linea3 , WHITE, BLACK, MEDIUM);
			
			drawString(30, 265, "Temperatura I2C: ", WHITE, BLACK, MEDIUM);
			drawString(70, 285, linea4, WHITE, BLACK, MEDIUM);
		}
//		else
//		{
//				sprintf(buffer_dist, "\n Distancia : %.2f cm        ", distancia);
//				tx_cadena_UART0(buffer_dist);
//				while(tx_completa==0);
//				tx_completa=0;					//Se transmite la cadena de buffer
//			
//		}

}//TIMER3_IRQHandler	

void ADC_IRQHandler(void)
{
	float temperatura;
	
	temperatura=((LPC_ADC->ADDR2>>4)&0xFFF)/4095.0*3300/10.0;
	
	if(temperatura_global != temperatura)	//si la nueva temperatura es diferente que la anterior
		temperatura_global=temperatura;
	
	
	if((mode==111)||(mode==112)||(mode==12))
		mostrar_resultados();
//	else if((mode==211)||(mode==212)||(mode==22)||(mode==23))
//		mostrar_medidas_uart();
}


//void SysTickHandler()
//{
//	aux_time++;
//	if(aux_time>1000)
//	{
//		if(mode==13)	
//			mostrar_resultados();
//		aux_time=0;
//		
//	}
//}