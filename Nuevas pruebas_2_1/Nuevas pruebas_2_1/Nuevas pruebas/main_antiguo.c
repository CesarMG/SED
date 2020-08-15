#include <LPC17xx.H>
#include "lcddriver.h"
#include <stdio.h>

#include "configuraciones.h"
#include "funciones.h"
#include "interrupciones.h"

#define F_cpu 100e6		// Defecto Keil (xtal=12Mhz)
#define Fpclk 25e6		// Fcpu/4 (defecto después del reset)
#define Tpwm 15e-3		// Perido de la señal PWM (15ms)
#define N_muestras 32 // muestras del DAC (antes eran 20)
#define PI 3.141592

uint8_t mode=0;				// variable de modo activo
uint8_t modo_selec=0;	// flag de seleccion de modo terminado
uint8_t grados=0;
uint8_t pulsos=0;
float distancia=0;

float umbral=100;
float frecuencia=4000;
uint16_t muestras[N_muestras];

float temperatura_global;
float temperaturaI2C=0;
float prueba=0;							//la uso?
int 	temp=0;
float temp_ADC_nueva=0;			//la uso?
float temp_I2C_nueva=0;			//la uso?

uint8_t mostrar=0;					//la uso?
char linea1[100];
char linea2[100];
char linea3[100];
char linea4[100];
//char linea_prueba[10];

int main()
{
	lcdInitDisplay();				
  fillScreen(BLACK);

	config_pines();
	config_externas();
	config_pwm1();
	config_ADC();
	config_DAC();
	config_TIMER0();
	config_TIMER1();
	config_TIMER2();
	config_TIMER3();
	genera_muestras(N_muestras);
	config_DS1621();
	
	//Encabezado
	//fillRect(85, 20, 90, 20, WHITE); //x0, y0, ancho, alto, color
	drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
	drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
	drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
	drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
	
	drawString(30, 125, "Elija modo:", WHITE, BLACK, MEDIUM);
	drawString(30, 155, "1) Offline -> KEY 1", WHITE, BLACK, MEDIUM);
	drawString(30, 185, "2) Online -> KEY 2", WHITE, BLACK, MEDIUM);
	
	while(mode==0);		//espera para seleccion de modo

	switch(mode)
		{
			case 1:		//modo offline
				fillScreen(BLACK);
				drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
				drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
				drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
				drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
			
				drawString(30, 125, "Elija submodo:", WHITE, BLACK, MEDIUM);
				drawString(10, 155, "1) Manual:", WHITE, BLACK, MEDIUM);
				drawString(50, 175, "-> KEY1", WHITE, BLACK, MEDIUM);
				drawString(10, 195, "2) Barrido automatico:", WHITE, BLACK, MEDIUM);
				drawString(50, 215, "-> KEY2", WHITE, BLACK, MEDIUM);
				drawString(10, 235, "3) Deteccion de obstaculos:", WHITE, BLACK, MEDIUM);	
				drawString(50, 255, "-> ISP", WHITE, BLACK, MEDIUM);
			
				while(mode==1);		//espera para sel. submodo
				switch(mode)
				{
					case 11:	//modo manual
							fillScreen(BLACK);
							drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
							drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
							drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
							drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
					
							drawString(30, 125, "Elija una opcion:", WHITE, BLACK, MEDIUM);
							drawString(30, 155, "1) Disparo unico:", WHITE, BLACK, MEDIUM);
							drawString(70, 175, "-> KEY 1", WHITE, BLACK, MEDIUM);
							drawString(30, 195, "2) Disparos continuos:", WHITE, BLACK, MEDIUM);
							drawString(70, 215, "-> KEY 2", WHITE, BLACK, MEDIUM);
					
						while(mode==11);					//espera para sel. subsubmodo
						set_servo(grados=90);			//llevamos el servo a la posicion 90 (la mitad)
					
						if(mode==111)							//modo medida manual
							LPC_TIM0->TCR|=(1<<0);	//habilitamos timer 0
		
							fillScreen(BLACK);

							drawString(30, 10, "Elija una opcion:", WHITE, BLACK, SMALL);
						  drawString(30, 20, "1) Servo: -10 grados", WHITE, BLACK, SMALL);
							drawString(70, 30, "-> KEY 1", WHITE, BLACK, SMALL);
							drawString(30, 40, "2) Servo: +10 grados", WHITE, BLACK, SMALL);
							drawString(70, 50, "-> KEY 2", WHITE, BLACK, SMALL);
							drawString(30, 60, "3) Disparo ultrasonidos:", WHITE, BLACK, SMALL);	
							drawString(70, 70, "-> ISP", WHITE, BLACK, SMALL);
							
							//ACTUALIZAR LOS RESULTADOS EN PANTALLA SOLO SI SON DISTINTOS
							/*while(1)
							{
								if(temp_ADC_nueva!=temperatura_global || temp_I2C_nueva!=temperaturaI2C)
								{
									temp_ADC_nueva=temperatura_global;
									temp_I2C_nueva=temperaturaI2C;
									mostrar_resultados();
								}
							}*/
							//mostrar_resultados();
						
						if(mode==112)							//modo medidas continuas
							LPC_TIM0->TCR|=(1<<0);	//habilitamos timer 0
					break;
					case 12:	//modo automatico
						set_servo(0);							//llevamos el servo a la posicion 0
						LPC_TIM0->TCR|=(1<<0);	  //habilitamos timer 0
					
						fillScreen(BLACK);
						drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
						drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
						drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
						drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
					break;
					case 13:	//modo deteccion obstaculos
						set_servo(grados=90);		  //llevamos el servo a la posicion 90 (la mitad)
			  		LPC_TIM0->TCR|=(1<<0);	  //habilitamos timer 0 
						 
						fillScreen(BLACK);
						drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
						drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
						drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
						drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);					
					break;
				}
			break;
			case 2:		//modo online
			break;
		}
	while(1)
	{
			if(mostrar==1)
				{
					mostrar=0;
					//mostrar_resultados();
				}
				
				if(distancia<=umbral)
				{
					LPC_TIM1->TCR=(1<<0);														//Activo el TIMER
					frecuencia=5000-umbral*10;								  			//Aplico la formula dada
					LPC_TIM1->MR0=(Fpclk/frecuencia/N_muestras)-1;		//Saco la frecuencia por el Match 
					//if(mode==13)
				}
				else
					LPC_TIM1->TCR=(0<<0);															//Reseteo el TIMER

//	  drawString(30, 300, "sonoro", RED, WHITE, MEDIUM);

//		if(s!=0)
//		{ 
//			g++;
//			s = 0;
//		}
//		if(g==16)
//			g=0;
	}
}//main
