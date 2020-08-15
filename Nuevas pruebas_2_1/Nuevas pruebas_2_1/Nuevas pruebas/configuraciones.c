#include "configuraciones.h"

/////////////////////////////
///// CONFIGURACIONES   /////
/////////////////////////////



void config_prioridades()
{
	NVIC_SetPriorityGrouping(0);
	NVIC_SetPriority(ADC_IRQn,   5);
	NVIC_SetPriority(UART0_IRQn, 0);
	NVIC_SetPriority(EINT0_IRQn, 0);		// Asignamos prioridades a las interrupciones
	NVIC_SetPriority(EINT1_IRQn, 0);
	NVIC_SetPriority(EINT2_IRQn, 0);
	NVIC_SetPriority(TIMER0_IRQn,0);
	NVIC_SetPriority(TIMER1_IRQn,2);    // Establecer prioridades
	NVIC_SetPriority(TIMER2_IRQn,4); 	  // Establecer prioridades
	NVIC_SetPriority(TIMER3_IRQn,1); 	  // Establecer prioridad
}

void config_pines()
{
	/*
		Configuración del trigger
	*/
	LPC_GPIO1->FIODIR |= (1U<<31); 		// pin 1.31 -> Declarar como salida el pin del pulso
	LPC_GPIO1->FIOCLR |= (1U<<31);		// pin 1.31 -> Aseguramos un nivel bajo
}
void config_externas()
{	
	/*
		EINT 0 (P2.10), EINT 1 (P2.11), EINT 2(P2.12)
	*/
	LPC_PINCON->PINSEL4 |= (1<<20)|(1<<22)|(1<<24);	// configuramos pines para EINT
	LPC_SC->EXTMODE |= (1<<0)|(1<<1)|(1<<2);				// Activacion por flanco
	LPC_SC->EXTPOLAR = 0;														// de bajada
	
	NVIC_EnableIRQ(EINT0_IRQn);							// Habilitamos las interrupciones
	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(EINT2_IRQn);
	
}//config_externas

void config_pwm1(void)				  				
{
	/*
		PWM para mover el servo
	*/
	LPC_PINCON->PINSEL3 |= (2<<8); 				// P1.20 salida PWM (PWM1.2)
	
	LPC_SC->PCONP |= (1<<6);							// Alimentamos PWM 1
	LPC_PWM1->MR0	 = Fpclk*Tpwm-1;		
	LPC_PWM1->PCR	|= (1<<10); 						// configurado el ENA2 (1.2)
	LPC_PWM1->MCR	|= (1<<1);							// Reset on Match
	LPC_PWM1->TCR	|= (1<<0)|(1<<3);				// Start
	
}

void config_DAC(void)										
{
	/*
		Configuración del DAC
	*/
	LPC_PINCON->PINSEL1  |= (2<<20);		// AOUT en el pin P0.26
	LPC_PINCON->PINMODE1 |= (2<<20);		// Pull-up-pull-down deshabilitado
	LPC_DAC->DACCTRL      = 0;					// Deshabilita el modo DMA
	//LPC_SC->PCLKSEL0|=(0x00<<22);			  // El DAC trabaja como maximo a 1MHz (CCLK/4)
}

void config_ADC(void)
{
	/*
		config_ADC
	*/
	LPC_SC->PCONP					|= (1<<12);			// Habilitar ADC
	LPC_PINCON->PINSEL1		 = (1<<18); 		// Habilitamos el AD0.2 en el P0.25
	LPC_PINCON->PINMODE1   = (2<<18);		  // Quitar resistencias de pull-up y pull-down
	
	LPC_ADC->ADCR    |= (1<<2)|						// Seleccionamos el canal del ADC (AD0.2)
								      (1<<8)|						// Dividimos entre (1+1 = 2) para que CLKAD0 = 12,5<13MHz
								      (1<<21);	 		    // ADC operativo
	LPC_ADC->ADINTEN |= (1<<2);	   				// Acabar conversion canal 2 genera interrupcion
	
	NVIC_EnableIRQ(ADC_IRQn);				      // Habilito la interrupcion del ADC
	NVIC_SetPriority(ADC_IRQn, 2);

}

void config_DS1621()
{
		//LPC_PINCON->PINSEL0 |= (3<<0)|(3<<2); //P0.0 SDA1 y P0.1 SCL1
		//LPC_PINCON->PINMODE0 |= (2<<0)|(2<<2);
		
		I2CSendAddr(0x48, 0);		// Direccion del dispositivo: 000
		I2CSendByte(0xAC);			// Se accede al registro de configuracion
		I2CSendByte(0x02);			// Se elige el modo de conversion continua (1SHOT=0) y la polaridad activa a valor alto
		I2CSendStop();					// Termino la comunicacion con un STOP
		
		I2Cdelay();							// Retardo de 10s para asegurar la configuración con guardado en EEprom interna
		
		I2CSendAddr(0x48, 0);   // Inicio de la comunicación
		I2CSendByte(0xEE);			// Se inicia la conversion
		I2CSendStop();					// Termino la comunicacion con un STOP
	
}//config_DS1621

void config_TIMER0()      // TIMER0 (interrumpe cada 0,5 seg) 
{
	LPC_SC->PCONP	|=(1<<1);			     // Alimentamos TIMER0
	LPC_TIM0->PR	 =0;							 // Prescaler a 1
	LPC_TIM0->MCR |=(3<<0);				   // Interrumpe y resetea MR0
	LPC_TIM0->MR0=(Fpclk*0.5-1);     // Interrumpe cada 0,5 seg
	
	NVIC_EnableIRQ(TIMER0_IRQn);	   // Habilito la interrupcion del TIMER0
	
}//config_TIMER0

void config_TIMER1(void)   // TIMER DAC
{
	LPC_SC->PCONP |= (1<<2);						// Habilitar Timer 1

	LPC_TIM1->PR   = 0;				  		    // Prescaler=1
	LPC_TIM1->MCR |= (1<<0)|(1<<1);   	// Interrupcion y reset para MR0 
	LPC_TIM1->EMR  = (1<<1);						// Cuando ocurra el Match 1.0, que no haga nada en el pin
	LPC_TIM1->TCR  = (0<<0);						// Power On TIMER1->Desactivado
	
	
	NVIC_EnableIRQ(TIMER1_IRQn);		    // Habilitar interrupcion
	
}//config_TIMER1

void config_TIMER2(void)   // Timer Pulso alto (trigger)
{
	LPC_SC->PCONP |= (1<<22); 					// Alimenta TIMER0
	LPC_TIM2->MCR  = 0x07; 						  // con 7 activo los 3 bits: STOP, Interrupción, Reset
	LPC_TIM2->MR0  = (Fpclk*2e-5)-1; 		// 20micro segundos
	LPC_TIM2->TCR  = 0;			  				  // Apagar TIMER

	
	NVIC_EnableIRQ(TIMER2_IRQn); 			  // Habilitar interrupción
	
}//config_TIMER2

void config_TIMER3()		  // TIMER3 (capture)
{
	LPC_SC->PCONP       |= (1<<23); 		// Alimentamos el TIMER3
	LPC_PINCON->PINSEL1 |= (3<<14);			// Selecciono CAP3.0	P0.23
	LPC_PINCON->PINSEL1 |= (3<<16);			// Selecciono CAP3.1 P0.24
	LPC_TIM3->PR				 = 24;
	LPC_TIM3->CCR 			 = (1<<0)|      // CAP3.0 flanco de subida
												 (1<<4)|      // CAP3.1 flanco bajada 
												 (1<<5);      // con interrupción
	
	NVIC_EnableIRQ(TIMER3_IRQn);			  // Habilitar interrupción    
	
}//Config_TIMER3
