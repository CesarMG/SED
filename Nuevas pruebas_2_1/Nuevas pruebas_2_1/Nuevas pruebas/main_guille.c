#include <lpc17xx.h>
#include <stdio.h>
#include <Math.h>
/*Incluimos el header de la UART 0*/
//#include "uart.h"
/*Incluimos el header del Display LCD*/
#include "lcddriver.h"
#define Fpclk 25e6																//Frecuencia de pclk es de 100/4 Mhz
#define Tpwm 15e-3																//Periodo de la pwm
#define Ttrigger 1.5e-5														//Periodo del trigger para el ultrasonidos
#define PI 3.141516
#define N_muestras 20															//Numero de muestras aptas por DAC
uint8_t modo = 0;																	//Modo de funcionamiento en el que nos encontramos
uint8_t flag_ISP = 0;															//Flag de activación de ISP
uint8_t flag_KEY1 = 0;														//Flag de activación de KEY1
uint8_t flag_KEY2 = 0;														//Flag de activación de KEY2
uint8_t grados1 = 0;															//Posicion en grados en la que se encuentra el servo
uint8_t operacion = 0;														//Operacion de seleccion de grados y de medida en la UART0
uint8_t resolucion = 10;													//Valor de variacion en grados de barrido automatico
uint16_t umbral = 100;														//Distancia de deteccion de objetos
uint8_t flag_TIMER2 = 0;													//Flag activacion de la interrupcion de TIMER 2
uint8_t flag_TIMER1 = 0;													//Flag activacion de la interrupcion de TIMER1
uint8_t flag_TIMER3 = 0;													//Flag activacion de la interrupcion de TIMER1
uint8_t flag_deteccion = 0;															//Variable almacenamiento de las cuentas del ultrasonidos
float distancia = 0;															//Distancia de medida
uint8_t inicio_continuas = 0;											//Una vez que se active mide todo el rato
uint16_t cuentafinal = 0;
float temperatura_ADC;
uint16_t frecuencia = 4000;												//Inicia con umbral de 1metro
uint16_t muestras[N_muestras];										//Array de las distintas muestras
/*Variables para la  UART0*/
char buffer[30];																// Buffer de recepción de 30 caracteres
char *ptr_rx;																		// puntero de recepción
char rx_completa;																// Flag de recepción de cadena que se activa a "1" al recibir la tecla return CR(ASCII=13)
char *ptr_tx;																		// puntero de transmisión
char tx_completa;																// Flag de transmisión de cadena que se activa al transmitir el caracter null (fin de cadena)


/*
					INTERRUPCIONES EXTERNAS DEL MODELO KEY1, KEY 2 Y KEY 3

*/
/*Configuracion de las interrupciones externas*/
void config_interrupciones(){

	LPC_PINCON->PINSEL4 |= (1<<20);								//P2.10 es entrada (ISP)
	LPC_PINCON->PINSEL4 |= (1<<22);								//P2.11 es entrada (KEY1)
	LPC_PINCON->PINSEL4 |= (1<<24);								//P2.12 es entrada (KEY2)
	LPC_SC->EXTMODE |= (1<<0) | (1<<1) | (1<<2); 	//Activacion por flanco
	LPC_SC->EXTPOLAR = 0;													//Activacion por flanco de bajada
	
/*Habilitacion de las interrupciones*/
	NVIC_EnableIRQ(EINT0_IRQn);										//Habilitamos la interrupcion para ISP
	NVIC_EnableIRQ(EINT1_IRQn);										//Habilitamos la interrupcion para KEY1
	NVIC_EnableIRQ(EINT2_IRQn);										//Habilitamos la interrupcion para KEY2
/*Prioridades*/
	NVIC_SetPriority(EINT0_IRQn,1);
	NVIC_SetPriority(EINT1_IRQn,1);
	NVIC_SetPriority(EINT2_IRQn,1);
}
/*Interrupcion de ISP*/
void EINT0_IRQHandler(){
		LPC_SC->EXTINT = (1<<0);											//Borramos flag de activacion
		flag_ISP = 1;																	//Activamos flag
	}
/*Interrupcion de KEY1*/
void EINT1_IRQHandler(){
		LPC_SC->EXTINT = (1<<1);											//Borramos flag de activacion
		flag_KEY1 = 1;																//Activamos flag
	}
/*Interrupcion de KEY1*/
void EINT2_IRQHandler(){
	LPC_SC->EXTINT = (1 << 2);											//Borramos flag de activacion
	flag_KEY2 = 1;																//Activamos flag
}

/*
					MOVIMIENTO DEL SERVO SG90

*/
/*Configuración del servo con PWM*/	
void config_servo(void){
	LPC_SC->PCONP |= (1 << 6);											//Habilitamos la PWM1
	LPC_PINCON->PINSEL3 |= (1<<9);									//Habilitamos la PWM1.2 Port1.20
	LPC_PWM1->MR0 = Fpclk*Tpwm-1;										//Match de la PWM en 15ms
	LPC_PWM1->PCR |= (1<<10);												//Configurado el ENA2 (1.2)
	LPC_PWM1->MCR |= (1<<1);												//Reset en el match
	LPC_PWM1->TCR |= (1<<0) | (1<<3);								//Habilitar cuenta y PWM
}

/*Movimiento del servo*/
void movimiento_servo(uint8_t grados){
	LPC_PWM1->MR2 =(Fpclk*0.3e-3 +Fpclk*1.9e-3*grados/180);
	LPC_PWM1->LER |= (1<<2) | (1<<0);								//Habilitamos Match 0 y PWM1.2
}


/*
				MUESTREO DE 0.5s

*/
/*Configuración del Timer 2 como match para 0.5s muestreo*/
	void config_TIMER1(void){
	LPC_SC->PCONP |= (1<<2);												//Encendemos el TIMER 2
		/*Match 1.0*/
	LPC_TIM1->MCR |= (1<<1) | (1<<0);			//Reset cuando se de el match 0 e interrupcion
	LPC_TIM1->MR0 = Fpclk*0.5-1;										//Interrupcion de match cada 0.5s
	LPC_TIM1->TCR = (1<<0);	
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER1_IRQn,3);
}
void TIMER1_IRQHandler(void){
	LPC_TIM1->IR |= (1<<0);													//Eliminación del flag
	flag_TIMER1 = 1;

}
/*
			ULTRASONIDOS SRF04

*/
/*Configuramos el Timer 2 como match para 10us para el trigger*/
void config_TIMER2(void){
	LPC_SC-> PCONP |= (1<<22);
	/*Match 2.0*/
	LPC_TIM2->MCR |= (1<<0) | (1<<1) |(1<<2); 							//Interrupción, Reset 
	LPC_TIM2->MR0 = (Fpclk*Ttrigger)-1;								//Periodo de Matcha necesario 10us			
	LPC_TIM2->TCR = 0;	
	NVIC_EnableIRQ(TIMER2_IRQn);
}

/*Configuramos el TIMER 3 como capture adquisicion de cuentas*/
void config_TIMER3(void){
	LPC_SC->PCONP |= (1<<23);												//Habilitamos el Timer3
	LPC_PINCON-> PINSEL1 |= (1<<14) | (1<<15);			//CAP3.0 como medida inicial P0.23
	LPC_PINCON-> PINSEL1 |= (1<<16) | (1<<17);			//CAP3.1 como medida final (cuando se recibe el eco) P0.24
	LPC_TIM3-> CCR = (1<<0);												//CAP3.0 como flanco subida
	LPC_TIM3-> CCR |= (1<<4) | (1<<5);							//CAP3.1 como flanco de bajada e interrupción
	LPC_TIM3-> TCR |= (1<<0);												//Habilitación del timer
	NVIC_EnableIRQ(TIMER3_IRQn);	

	}
/*Interrupcion de TIMER1*/
void TIMER2_IRQHandler(void){
	LPC_TIM2->IR |= (1<<1);													//Eliminación del flag interrupcion
	LPC_GPIO3->FIOCLR |= (1<<26);										//Ponemos un 0 en el pin 3.26
	LPC_TIM2->TCR =0;
}



/*Función por la que conseguimos que empiece a funcionar el ultrasonidos*/
void ultrasonidos(void){
		LPC_TIM2->TCR = (1<<0);												//El timer comienza a avanzar
		LPC_GPIO3->FIOSET |= (1<<26);									//Ponemos un 1 en el pin 3.26

}
/*Adquisición de cuentas del eco*/
void TIMER3_IRQHandler(void){
		distancia = (LPC_TIM3->CR1)-(LPC_TIM3->CR0);	//Diferencia de las cuentas del eco
		distancia = distancia*(1/Fpclk)*0.5*331.5*100;//Conversión a distancia
		LPC_TIM3->IR|=(1<<4); 												//Reset captura de CR0
		LPC_TIM3->IR|=(1<<5); 												//Reset captura de CR1
}

/*/////////////////////////////////
			DAC MÓDULO LM386module

*/////////////////////////////////
void config_DAC(void){
	LPC_PINCON->PINSEL1 |= (1<<21); 								//Habilitamos el PORT0.26 como AUT
	LPC_PINCON->PINMODE1 |= (1<<21);								//Deshabilitación del pull-up y pull-down del PORT0.26
	LPC_DAC->DACCTRL = 0; 													//Deshabilitación del modo DMA
}

void config_TIMER0(void){
	LPC_SC->PCONP |= (1<<1);												//Encendemos el TIMER0
	LPC_PINCON->PINSEL7 |= (1<<19) ;								//Habilitamos el PORT3.25                   como Match0.0 
	LPC_TIM0->MCR |= (1<<0) | (1<<1);								//Reset e interrupcion 
	LPC_TIM0->MR0 = (Fpclk/frecuencia/N_muestras)-1;											
	LPC_TIM0-> EMR |= (1<<1);
	LPC_TIM0-> TCR |= (0<<0);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void genera_muestras_DAC(uint16_t muestras_ciclo){
	uint16_t i;
	for(i=0; i<muestras_ciclo; i++){
			muestras[i]=(uint32_t)(511+511*sin(2*3.14159*i/N_muestras))<<6;
				}
}

void TIMER0_IRQHandler(void){
	static uint8_t indice_muestras;
	LPC_TIM0-> IR |= (1<<0);
	LPC_DAC->DACR = muestras[indice_muestras++]<<6;
	indice_muestras &= 0x1F;
}

/*////////////////////////////////
			ADC LM35

*/////////////////////////////////

void config_ADC(void){
	LPC_SC->PCONP |= (1<<12);												//Habilitamos el funcionamiento del ADC
	LPC_PINCON->PINSEL1 |= (1<<14);									//Habilitamos PORT0.23 como AD0.0
	LPC_PINCON->PINMODE1 |= (1<<15);								//Deshabilitamos tanto pull-up como pull-down del PORT0.23
	LPC_ADC->ADCR = (1<<0) | (1<<8) | (1<<21) | (1<<25) | (1<<26); 		//Convierte con Match1.0
	LPC_ADC->ADINTEN = (1<<0);											//Acabar conversion del canal 0 genera interrupcion
	LPC_TIM1->EMR |= (1<<4) | (1<<5);								//Habilitamos como interrupción externa para el ADC
	NVIC_EnableIRQ(ADC_IRQn);												//Habilitamos la interrupcion del ADC
	NVIC_SetPriority(ADC_IRQn,3);
}

void ADC_IRQHandler(void){
	temperatura_ADC = ((LPC_ADC->ADDR0>>4) & 0xFFF)/4095.0*3300/10.0;

}

/*////////////////////////////////
			MAIN

*/////////////////////////////////
int main(){
	LPC_GPIO3->FIODIR |= (1<<26);										//Iniciamos el Port3.26 como salida
	LPC_GPIO3->FIOCLR |= (1<<26);										//Limpiamos el flag de activación poniendolo a 0
	lcdInitDisplay();									
	fillScreen(WHITE);
	config_interrupciones();												//Funcion para configuracion de las interrupciones
	uart0_init(9600);																//Iniciamos la UART0 para que trabaje a una velocidad de 9600Baudios
	ptr_rx=buffer;
	config_servo();
	movimiento_servo(grados1);
	config_TIMER0();
	config_TIMER1();
	config_TIMER2();															//Periodo de muestreo de 0.5s
	config_TIMER3();
	config_DAC();
	config_ADC();
	genera_muestras_DAC(N_muestras);

	while(1){
		switch(modo){
			/*Caso inicial*/ 
			case 0:
				/*Seleccion offline u online*/
				drawString(10,30, "Seleccione un modo:",BLACK, WHITE, MEDIUM);
				drawString(10,50, "Offline(Key1) u Online(Key2)",BLACK, WHITE, MEDIUM);

				if(flag_KEY1 == 1){
					flag_KEY1 = 0;					//Borramos flag de activacion de KEY1
					fillScreen(WHITE);			//Limpiamos la pantalla del LCD		
					flag_ISP = 0;						//Para evitar posibles errores
					modo = 1;								//Seleccionamos el modo offline
			}
				if(flag_KEY2 == 1){
					fillScreen(WHITE);			//Limpiamos la pantalla del LCD	
					flag_KEY2 = 0;					//Borramos flag de activacion de KEY2
					flag_ISP = 0;						//Para evitar posibles errores
					modo = 7;								//Seleccionamos el modo online
			}
					break;
			case 1:
				drawString(10,30, "MODO OFFLINE:",BLACK, WHITE, LARGE);
				drawString(10,60, "Seleccione un modo:",BLACK,WHITE,MEDIUM);
				drawString(10,80, "Manual(Key1), Barrido(Key2)",BLACK,WHITE,MEDIUM);
				drawString(10,100, "o Deteccion(ISP)",BLACK,WHITE,MEDIUM);
				/*Seleccion manual, barrido o deteccion de obstaculos*/
				if(flag_KEY1 == 1){	
					flag_KEY1 = 0;					//Borramos flag de activacion de KEY1
					modo = 2;								//Seleccionamos el modo Manual
					fillScreen(WHITE);			//Limpiamos la pantalla del LCD		
			}
				if(flag_KEY2 == 1){
					flag_KEY2 = 0;					//Borramos flag de activacion de KEY2
					modo = 5;								//Seleccionamos el modo Barrido Automático
					fillScreen(WHITE);			//Limpiamos la pantalla del LCD		
			}
				if(flag_ISP == 1){
					flag_ISP = 0;						//Borramos flag de activacion de ISP
					modo = 6;								//Seleccionamos el modo Deteccion de Obstaculos
					fillScreen(WHITE);			//Limpiamos la pantalla del LCD		
			}
				break;
			case 2:
				drawString(10,30, "MODO OFFLINE-",BLACK, WHITE, LARGE);
				drawString(10,60, "MANUAL", BLACK, WHITE, LARGE);
				drawString(10,90, "Seleccione un modo:",BLACK,WHITE,MEDIUM);
				drawString(10,110, "Botones(Key1) o ",BLACK,WHITE,MEDIUM);
				drawString(10,130, "Continuas(Key2)",BLACK,WHITE,MEDIUM);
				/*Seleccion medidas continuas o botones*/
				if(flag_KEY1 == 1){	
					fillScreen(WHITE);			//Limpiamos la pantalla del LCD		
					flag_KEY1 = 0;					//Borramos flag de activacion de KEY1
					modo = 3;								//Seleccionamos el modo Botones
			}
				if(flag_KEY2 == 1){
					fillScreen(WHITE);			//Limpiamos la pantalla del LCD		
					flag_KEY2 = 0;					//Borramos flag de activacion de KEY2
					modo = 4;								//Seleccionamos el modo Medidas Continuas
			}
				break;
			case 3:
				drawString(10,30, "MODO OFFLINE-",BLACK, WHITE, LARGE);
				drawString(10,60, "MANUAL-BOTONES", BLACK, WHITE, LARGE);
				drawString(10,110, "+10grad(Key1), -10grad(Key2)",BLACK,WHITE,MEDIUM);
				drawString(10,130, "o Medir(ISP)",BLACK,WHITE,MEDIUM);
				/*Modo botones, KEY1 +10º, KEY2 -10º e ISP medir*/
				if(flag_KEY1 == 1){	
					flag_KEY1 = 0;					//Borramos flag de activacion de KEY1
					if(grados1 < 180)
						grados1 += 10;				//+10º en posicion de servo
					movimiento_servo(grados1);
			}
				if(flag_KEY2 == 1){
					flag_KEY2 = 0;					//Borramos flag de activacion de KEY2
					if(grados1 > 0)
						grados1 -= 10;				//-10º en posicion de servo
					movimiento_servo(grados1);
			}
				if(flag_ISP == 1){	
					flag_ISP = 0;						//Borramos flag de activacion de ISP
					ultrasonidos();				//Medida de la distancia
			}
				
				break;
			case 4:
				drawString(10,30, "MODO OFFLINE-",BLACK, WHITE, LARGE);
				drawString(10,60, "MANUAL-", BLACK, WHITE, LARGE);
				drawString(10,90, "CONTINUAS",BLACK, WHITE, LARGE);
				drawString(10,130, "+10grad(Key1), -10grad(Key2)",BLACK,WHITE,MEDIUM);
				drawString(10,150, "o Comenzar a Medir(ISP)",BLACK,WHITE,MEDIUM);
				/*Modo medidas continuas, KEY1 10º, KEY2 -10º y ISP Medir continuamente*/
				if(flag_KEY1 == 1){	
					flag_KEY1 = 0;					//Borramos flag de activacion de KEY1
					if(grados1 < 180)
						grados1 += 10;				//+10º en posicion de servo
					movimiento_servo(grados1);
			}
				if(flag_KEY2 == 1){
					flag_KEY2 = 0;					//Borramos flag de activacion de KEY2
					if(grados1 > 0)
						grados1 -= 10;				//-10º en posicion de servo
					movimiento_servo(grados1);
			}
				if(flag_ISP == 1){	
					flag_ISP = 0;						//Borramos flag de activacion de ISP
					inicio_continuas = 1;					//Medida de la distancia de forma continua
			}
			if(inicio_continuas == 1 && flag_TIMER1==1){
					ultrasonidos();
					flag_TIMER1=0;
			}
				break;
		
			case 5:
				/*Modo Barrido automático*/
				drawString(10,30, "MODO OFFLINE-",BLACK, WHITE, LARGE);
				drawString(10,60, "MANUAL-", BLACK, WHITE, LARGE);
				drawString(10,90, "BARRIDO",BLACK, WHITE, LARGE);
				drawString(10,120, "AUTOMATICO",BLACK, WHITE, LARGE);
				while(modo == 5){
					if(flag_TIMER1 == 1){
						ultrasonidos();
						flag_TIMER2 = 0;
						movimiento_servo(grados1);
						
						if(grados1 <= 180)
							grados1+=10;
						
						if(grados1 > 180)
							grados1 = 0;

						}
					}
				break;
			case 6:
				drawString(10,30, "MODO OFFLINE-",BLACK, WHITE, LARGE);
				drawString(10,60, "MANUAL-", BLACK, WHITE, LARGE);
				drawString(10,90, "DETECCION DE",BLACK, WHITE, LARGE);
				drawString(10,120, "OBSTACULOS",BLACK, WHITE, LARGE);
				drawString(10,160, "+10grad(Key1) o ",BLACK,WHITE,MEDIUM);
				drawString(10,180, "-10grad(Key2)",BLACK,WHITE,MEDIUM);

				/*Deteccion de obstaculos selecciona la posicion KEY1 +10º o KEY2 -10º*/
				if( distancia<=umbral){
					LPC_TIM0->TCR |= (1<<0);
					frecuencia=5000-umbral*10;		//Formula proporcionada de frecuencia de sonido
					LPC_TIM0->MR0 = (Fpclk/frecuencia/N_muestras)-1;
					}
					else{
						LPC_TIM0->TCR = (0<<0);
					}
					
				if(flag_KEY1 == 1){	
					flag_KEY1 = 0;					//Borramos flag de activacion de KEY1
					if(grados1 < 180)
						grados1 += 10;				//+10º en posicion de servo

					movimiento_servo(grados1);
				}
				if(flag_KEY2 == 1){
					flag_KEY2 = 0;					//Borramos flag de activacion de KEY2					
					if(grados1 > 0)
						grados1 -= 10;				//-10º en posicion de servo
					movimiento_servo(grados1);
				}
				if(flag_TIMER1 == 1){
					ultrasonidos();
					flag_TIMER1 = 0;
				}
				break;
			/*Modo online introduce manual, automatico, obstaculos o valores*/
			case 7:
				drawString(10,30, "MODO ONLINE-",BLACK, WHITE, LARGE);
				drawString(10,60, "Es necesario utilizar", BLACK, WHITE, MEDIUM);
				drawString(10,90, "comunicacion por UART",BLACK, WHITE, MEDIUM);
				drawString(10,120, "mediante el TERMITE",BLACK, WHITE, MEDIUM);
				tx_cadena_UART0("Elija un modo de funcionamiento online: manual, automatico, obstaculos o valores \n");
				while(tx_completa == 0);
				tx_completa = 0;
				while(modo == 7){
					if(rx_completa == 1){
						rx_completa = 0;
						if(strcmp(buffer, "manual\r") == 0) modo = 8;												//Seleccionamos el modo Manual
							else if(strcmp(buffer, "automatico\r") == 0) modo = 11;						//Seleccionamos el modo Barrido Automático
								else if(strcmp(buffer, "obstaculos\r") == 0) modo = 12;					//Seleccionamos el modo Deteccion de Obstaculos
									else if(strcmp(buffer, "valores\r") == 0) modo = 13;					//Seleccionamos el modo Cambio de Valores 
										else	{
											tx_cadena_UART0("Comando erroneo \n\r");
											while(tx_completa == 0);
											tx_completa = 0;
										}
									} else {
										/*Representacion de medida de temperatura*/
										}
							}
				break;
			/*Modo manual introduce botones o continuas*/
				case 8: 
					tx_cadena_UART0("Elija un modo manual de funcionamiento: botones o continuas \n");
					while(modo == 8){
						if(rx_completa == 1){
							rx_completa=0;
							if(strcmp(buffer, "botones\r") == 0) modo = 9;										//Seleccionamos el modo Botones
								else if(strcmp(buffer, "continuas\r") == 0) modo = 10;					//Seleccionamos el modo Medidas Continuas
									else	{
										tx_cadena_UART0("Comando erroneo \n");
										while(tx_completa == 0);
										tx_completa = 0;
									}
								} else {
										/*Representacion de medidas de temperatura*/
									}
						}
					break;
			/*Modo botones introduce + (+resolucion), - (-resolucion), S (medir)*/
					case 9:
						tx_cadena_UART0("Introduzca: +, - o S \n");
						while(tx_completa == 0);
						tx_completa = 0;
						while(modo == 9){
							if(rx_completa == 1){
								rx_completa = 0;
								if(strcmp (buffer, "+\r") == 0)	operacion = 1;									//Operacion de suma de la resolucion							
									else if(strcmp (buffer, "-\r") == 0) operacion = 2;
										else	if(strcmp (buffer, "S\r") == 0)	operacion = 3;
										else { 
											tx_cadena_UART0("Comando erroneo\n \r");
											while(tx_completa == 0);
											tx_completa = 0;
											}
								switch(operacion){
									case 1:
										operacion = 0;
										if(grados1 < 180)
											grados1 += 10; 																				//Aumento en grados segun la resolucion seleccionada
										movimiento_servo(grados1);
										break;
									case 2:
										operacion = 0;
										if(grados1 > 0)
											grados1 -= 10;
										movimiento_servo(grados1);
										break;
									case 3:
										operacion = 0;
										ultrasonidos();
										break;
								}
							} else {
									/*Cada 0.5s debemos representar la medida de la temperatura*/
							}
						}
							break;
				/*Modo medidas continuas introduce + (+10º), - (-10º), S (medir continuamente)*/
				case 10:
					tx_cadena_UART0("Introduzca: +, - o S \n");
					while(tx_completa == 0);
					tx_completa = 0;
					while(modo == 10){
						if(rx_completa == 1){
							rx_completa = 0;
							if(strcmp (buffer, "+\r") == 0)	operacion = 1;									//Operacion de suma de la resolucion							
								else if(strcmp (buffer, "-\r") == 0) operacion = 2;
									else	if(strcmp (buffer, "S\r") == 0)	operacion = 3;
										else { 
											tx_cadena_UART0("Comando erroneo\n \r");
											while(tx_completa == 0);
											tx_completa = 0;
											}
								switch(operacion){
									case 1:
										operacion = 0;
										if(grados1 < 180)
											grados1 += 10; 																				//Aumento en grados segun la resolucion seleccionada
											movimiento_servo(grados1);;
										break;
									case 2:
										operacion = 0;
										if(grados1 > 0)
											grados1 -= 10;
											movimiento_servo(grados1);;
										break;
									case 3:
										operacion = 0;
										inicio_continuas=1;
										break;
								}
							} else {
								if(inicio_continuas == 1 && flag_TIMER1 == 1){
										ultrasonidos();
										flag_TIMER1 = 0;
										}
									/*Cada 0.5s debemos representar la medida de la temperatura*/
							}							
						}
					break;
				/*Modo Barrido Automático*/
				case 11:
					/*Medir 0.5s y variacion de 10º*/
					
				while(modo == 11){
					if(flag_TIMER1 == 1){
					ultrasonidos();
					flag_TIMER1 = 0;
					movimiento_servo(grados1);	
					if(grados1 <= 180)
						grados1+=resolucion;
					if(grados1 > 180)
						grados1 = 0;

					}
					}
					break;
				case 12:
					/*DETECCION OBSTACULOS cm*/
					tx_cadena_UART0("Introduzca: + o -  \n");
					while(tx_completa == 0);
					tx_completa = 0;
					while(modo == 12){
						if(rx_completa == 1){
							rx_completa = 0;
							if(strcmp (buffer, "+\r") == 0)	operacion = 1;									//Operacion de suma de la resolucion							
								else if(strcmp (buffer, "-\r") == 0) operacion = 2;
									else { 
										tx_cadena_UART0("Comando erroneo\n \r");
										while(tx_completa == 0);
										tx_completa = 0;
										}
								switch(operacion){
									case 1:
										operacion = 0;
										if(grados1 < 180)
											grados1 += 10; 																				//Aumento en grados segun la resolucion seleccionada
											movimiento_servo(grados1);
										
										break;
									case 2:
										operacion = 0;
										if(grados1 > 0)
											grados1 -= 10;
											movimiento_servo(grados1);

										break;
									}
							} else {
								if(flag_TIMER1 == 1){
										ultrasonidos();
										flag_TIMER1 = 0;
									}
								if( distancia<=umbral){
										LPC_TIM0->TCR |= (1<<0);
										frecuencia=5000-umbral*10;		//Formula proporcionada de frecuencia de sonido
										LPC_TIM0->MR0 = (Fpclk/frecuencia/N_muestras)-1;
										}
										else{
											LPC_TIM0->TCR = (0<<0);
												}
					
								/*Medida de temperatura, distancia y pitido*/
							}
						}
							break;
			/*Modo cambio de valores*/
				case 13:
					tx_cadena_UART0("Introduzca: umbral o resolucion  \n");
					while(tx_completa == 0);
					tx_completa = 0;
					while(modo == 13){
						if(rx_completa == 1){
							rx_completa = 0;
							if(strcmp (buffer, "umbral\r") == 0) modo = 14;
								else if(strcmp (buffer, "resolucion\r") == 0) modo = 15;
									else { 
										tx_cadena_UART0("Comando erroneo\n \r");
										while(tx_completa == 0);
										tx_completa = 0;
										}

							} else {
								/*Medida de temperatura*/
							}
							
						}
							break;
			case 14:
				tx_cadena_UART0("Introduzca un umbral entre 30 y 300 \n");
				while(tx_completa == 0);
				tx_completa = 0;
				while(modo == 14){
					if(rx_completa == 1){
						rx_completa = 0;
						umbral = atoi(buffer);
						if(umbral<30 || umbral>300){
							tx_cadena_UART0("Comando erroneo introduzca uno nuevo\n");
							while(tx_completa == 0);
							tx_completa = 0;
						} else {
							modo = 7;
							}
						} else {
								/*Medida de temperatura*/
							}
	
				}
				break;
			case 15:
				tx_cadena_UART0("Introduzca un resolucion entre 0 y 180 \n");
				while(tx_completa == 0);
				tx_completa = 0;
				while(modo == 15){
					if(rx_completa == 1){
						rx_completa = 0;
						resolucion = atoi(buffer);
						if(resolucion<0 || resolucion>180){
							tx_cadena_UART0("Comando erroneo introduzca uno nuevo\n");
							while(tx_completa == 0);
							tx_completa = 0;
						} else {
							modo = 7;
							}
						} else {
								/*Medida de temperatura*/
							}
	
				}
				break;	
			
			}
		}
					
	}

		

