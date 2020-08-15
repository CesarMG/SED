#include "interrupciones.h"

	//////////////////////////////
	/////   INTERRUPCIONES   /////
	//////////////////////////////

void EINT0_IRQHandler() 	 //key ISP (P2.10)
{
	LPC_SC->EXTINT |= (1<<0);		     //Borramos flag
	
	if((modo_selec==0)&&(mode==1)) 	 //estamos seleccionando modo
	{
		mode = 13;										 //modo deteccion elegido
		modo_selec = 1; 							 //se finaliza la seleccion de modo
	}
	else if ((mode==111)||(mode==13))
		generar_pulso_alto();
	
}// EINT0_IRQHandler

void EINT1_IRQHandler()	   //key1 (P2.11)
{
	LPC_SC->EXTINT |= (1<<1);		//Borramos flag
	
	if(modo_selec==0)		      	//estamos seleccionando modo
	{
		switch(mode)
		{
			case 0:						//no se ha elegido nada aun
				mode = 1;				//modo Offline elegido
				break;
			case 1:						//se ha elegido ya modo offline
				mode = 11;			//modo manual elegido
				break;
			case 11:					//se ha elegido offline y manual
				mode = 111;			//modo botones elegido
				modo_selec = 1; //se finaliza la seleccion de modo
				break;
		}	
	}
	else if((mode==111)||(mode==112)||(mode==13))
		if(grados>0)
			set_servo(grados-=10); //se mueve -10º
	
}//EINT1_IRQHandler

void EINT2_IRQHandler()	//key2 (P2.12)
{
	LPC_SC->EXTINT |= (1<<2);		//Borramos flag
	
	if(modo_selec==0)					//estamos seleccionando modo
	{
		switch(mode)
		{
			case 0:		  				//no se ha elegido nada aun
				mode = 2;					//modo Online elegido
				break;
			case 1:			  			//se ha elegido ya modo offline
				mode = 12;				//modo automatico elegido
				modo_selec = 1;		//se finaliza la seleccion de modo
				break;
			case 11:						//se ha elegido offline y manual
				mode = 112;				//modo medidas continuas elegido
				modo_selec = 1;		//se finaliza la seleccion de modo
				break;
		}
	}
	else if((mode==111)||(mode==112)||(mode==13))
		if(grados<180)	
			set_servo(grados+=10);//se mueve +10º
}//EINT2_IRQHandler

void TIMER0_IRQHandler(void)	//TIMER0 (cada 0,5 seg)
{
	LPC_TIM0->IR|=(1<<0);	//limpiamos flag MR0
	
	mostrar=1;
	
	prueba=leer_Temperatura_I2C();
	LPC_ADC->ADCR|=(1<<24);									//Que empiece la conversion ahora
	
	if(mode==12)	//modo barrido automatico activado
	{	
		generar_pulso_alto();
		if(pulsos<18)	
		{
			set_servo(grados+=10);	
			pulsos+=1;
		}
		else //cuando llega al final vuelve
		{
			set_servo(grados=0);	
			pulsos=0;
		}
	}
	if(mode==13)	//modo deteccion obstaculos
	{
		generar_pulso_alto();
	}
	
	//mostrar_resultados();

}//TIMER0_IRQHandler

void TIMER1_IRQHandler(void)			//TIMER del DAC
{
		static uint8_t indice_muestra;

	
		LPC_TIM1->IR|=(1<<0);														//Borro el flag de la interrupcion
		LPC_DAC->DACR=muestras[indice_muestra++]<<6;		//Desplazamient bit6...bit15 para colocarlos bien
//		if(indice_muestra==N_muestras)									//Resetear el indice
//		{
//			indice_muestra=0;
//		}
		indice_muestra &= 0x1F;
//		if(s==0)
//			s = 1;
}

void TIMER2_IRQHandler(void)			//TIMER pulso alto
{																	    //Tras los 10us del pulso en High, queremos que vuelva a 0
		LPC_TIM2->IR|= (1<<1);						//Borrar el flag de interrupción del TIMER0
		LPC_GPIO1->FIOCLR =(1U<<31); 			//Limpiar el pin
		LPC_TIM2->TCR = 0; 								// Apagar timer2
}//TIMER2_IRQHandler

void TIMER3_IRQHandler(void) 			//TIMER para el Capture			
{
		float dif_pulsos; 
		LPC_TIM3->IR|=(1<<4); 						//Reset captura de CR0
		LPC_TIM3->IR|=(1<<5); 						//Reset captura de CR1
		dif_pulsos=((LPC_TIM3->CR1)-(LPC_TIM3->CR0)); 		//Calcular  diferencia del ancho de pulso
		distancia=dif_pulsos*0.017; 			// Formula distancia final
}//TIMER3_IRQHandler	

void ADC_IRQHandler(void)
{
	float temperatura;
	
	temperatura=((LPC_ADC->ADDR2>>4)&0xFFF)/4095.0*3300/10.0;
	//temperatura_global=temperatura;
	
		if(temperatura_global != temperatura) {			//si la nueva temperatura es diferente que la anterior
		temperatura_global=temperatura;
		//flag_temp = 1;										//se muestra por pantalla (lo hace el main)
	}
	
	mostrar_resultados();
	//NVIC_DisableIRQ(ADC_IRQn);				//Habilitamos interrupcion
}