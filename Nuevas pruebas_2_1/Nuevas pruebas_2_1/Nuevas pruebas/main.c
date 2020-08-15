#include <LPC17xx.H>
#include "lcddriver.h"
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>


#include "configuraciones.h"
#include "funciones.h"
#include "interrupciones.h"

#define F_cpu 100e6				// Defecto Keil (xtal=12Mhz)
#define Fpclk 25e6				// Fcpu/4 (defecto después del reset)
#define Tpwm 15e-3				// Perido de la señal PWM (15ms)
#define N_muestras 32 		// muestras del DAC (antes eran 20)
#define PI 3.141592

uint8_t FLAG_ISP   = 0;		// Flag boton ISP
uint8_t FLAG_KEY1  = 0;		// Flag boton KEY1
uint8_t FLAG_KEY2  = 0;		// Flag boton KEY2

uint8_t FLAG_TIMER0 = 0;  // Flag timer 0
uint8_t FLAG_TIMER1 = 0;  // Flag timer 1
uint8_t FLAG_TIMER2 = 0;  // Flag timer 2
uint8_t FLAG_TIMER3 = 0;  // Flag timer 3

uint8_t FLAG_init_servo = 0;  // Flag servo inicializado
uint8_t FLAG_init_timer = 0;  // Flag timer inicializado
uint8_t FLAG_menu_1     = 0;  // Flag menu printeado
uint8_t DAC_ON = 0;						// Flag activacion DAC

char nevera_abierta = 0;   

uint8_t mode		   = 0;  // Variable de modo activo 
uint8_t grados     = 0;	 // Variable grados girados [0º - 180º]
uint8_t pulsos     = 0;	 // Variable pulsos servo [0 - 18]
float distancia    = 0;  // Variable distancia medida (cm)

int umbral     = 100;  // Variable distancia de deteccion (cm)
float frecuencia = 4000; // Variable frecuencia seno (Hz)
uint16_t muestras[N_muestras];  // Array que contiene al seno 

float temperatura_global; 	
float temperaturaI2C=0;
float prueba=0;							//la uso?
int 	temp=0;
float temp_ADC_nueva=0;			//la uso?
float temp_I2C_nueva=0;			//la uso?
int G = 0;
int GA = 0;

uint8_t mostrar=0;					//la uso?
char linea1[100];
char linea2[100];
char linea3[100];
char linea4[100];

char buffer[200]; // Buffer de recepción de 200 caracteres
char buffer_dist[100];
char buffer_temp_glob[100];
char buffer_temp_I2C[100];
char buffer_grad[100];
char buffer_umbral[100];
char *ptr_rx; // Puntero de recepción
volatile int rx_completa; // Flag de recepción de cadena que se activa a "1" al recibir la tecla return CR(ASCII=13)
char *ptr_tx; // puntero de transmisión
char tx_completa; // Flag de transmisión de cadena que se activa al transmitir el caracter null (fin de cadena)	
char aux = 0;
char aux2 = 0;
char aux3 = 0;
char aux4 = 0;
int aux5 = 0;
int aux_time=0;
char disparo=0;
int i = 0;

int main()
{
	lcdInitDisplay();				// Inicializamos display
  fillScreen(BLACK);			// Pintamos la pantalla de negro

	config_pines();
	config_externas();
	config_pwm1();
	config_ADC();
	config_DAC();
	config_TIMER0();
	config_TIMER1();
	config_TIMER2();
	config_TIMER3();
	genera_muestras(N_muestras);	// Generamos el seno
	config_DS1621();
	
	ptr_rx=buffer;
	uart0_init(9600);
	
//	SysTick->LOAD=49000;
//	SysTick->VAL=0;
//	SysTick->CTRL=0x07;
	
	
while(1)
{
//			if(nevera_abierta)
//			{
//				mostrar_resultados();
//				nevera_abierta = 0;
//			}
	switch(mode)
	{
		case 0:   //Inicio
			if(FLAG_menu_1 == 0)
			{
				FLAG_menu_1 = 1;	// Evitamos que ponga continuamente el menu
				//Encabezado
				//fillRect(85, 20, 90, 20, WHITE); //x0, y0, ancho, alto, color
				drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
				drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
				drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
				drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
				
				drawString(30, 125, "Elija modo:", WHITE, BLACK, MEDIUM);
				drawString(30, 155, " 1) Offline -> KEY 1", WHITE, BLACK, MEDIUM);
				drawString(30, 185, " 2) Online  -> KEY 2", WHITE, BLACK, MEDIUM);
			}
			if(FLAG_KEY1 == 1)	// Se ha pulsado KEY1 (offline)
			{
				FLAG_KEY1   = 0;			// Borramos flag de KEY1	
				FLAG_ISP    = 0;			// Borramos flag de ISP por si acaso
				FLAG_menu_1 = 0;			// Permitimos que aparezca el siguiente menu
				mode        = 1;			// Modo offline seleccionado
			}
			if(FLAG_KEY2 == 1)  		// Se ha pulsado KEY2 (online)
			{
				FLAG_KEY2   = 0;			// Borramos flag de KEY2
				FLAG_ISP    = 0;		  // Borramos flag de ISP por si acaso
				FLAG_menu_1 = 0; 			// Permitimos que aparezca el siguiente menu 
				mode        = 2;	  	// Modo online seleccionado
			}
		break;
		
		case 1:		// Modo offline
			if(FLAG_menu_1 == 0)
			{
				FLAG_menu_1 = 1;
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
			}
			if(FLAG_KEY1 == 1)	// Se ha pulsado KEY1 (manual)
			{
				FLAG_KEY1   = 0;			// Borramos flag de KEY1	
				mode        = 11;			// Modo manual seleccionado
				FLAG_menu_1 = 0; 			// Permitimos que aparezca el siguiente menu 
			}
			if(FLAG_KEY2 == 1)  // Se ha pulsado KEY2 (automatico)
			{
				FLAG_KEY2   = 0;			// Borramos flag de KEY2
				mode        = 12;	  	// Modo automatico seleccionado
				FLAG_menu_1 = 0; 			// Permitimos que aparezca el siguiente menu 
			}
			if(FLAG_ISP == 1)   // Se ha pulsado ISP (deteccion)
			{
				FLAG_ISP    = 0;			// Borramos flag de KEY2
				mode        = 13;	  	// Modo deteccion seleccionado
				FLAG_menu_1 = 0; 			// Permitimos que aparezca el siguiente menu 
			}
		break;
		
		case 11:	// Modo manual (offline)
			if(FLAG_menu_1 == 0)
			{
				FLAG_menu_1 = 1; 	
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
			}
			if(FLAG_init_servo == 0)
			{
				set_servo(grados = 90);	 // Inicializamos servo a la posicion 90º
				FLAG_init_servo = 1;		 // Servo inicializado
			}
			
			if(FLAG_KEY1 == 1)	// Se ha pulsado KEY1 (único)
			{
				FLAG_KEY1   = 0;			  // Borramos flag de KEY1	
				mode        = 111;			// Modo disparo unico seleccionado
				FLAG_menu_1 = 0; 		  	// Permitimos que aparezca el siguiente menu 
			}
			
			if(FLAG_KEY2 == 1)  // Se ha pulsado KEY2 (continuo)
			{
				FLAG_KEY2   = 0;			  // Borramos flag de KEY2
				mode        = 112;	  	// Modo disparos continuos seleccionado
				FLAG_menu_1 = 0; 			  // Permitimos que aparezca el siguiente menu 
			}
		break;
		
		case 111:	// Modo manual - medida unica (offline)
			if(FLAG_menu_1 == 0)
			{
				FLAG_menu_1 = 1;
				fillScreen(BLACK);

				drawString(30, 10, "Elija una opcion:", WHITE, BLACK, SMALL);
				drawString(30, 20, "1) Servo: -10 grados", WHITE, BLACK, SMALL);
				drawString(70, 30, "-> KEY 1", WHITE, BLACK, SMALL);
				drawString(30, 40, "2) Servo: +10 grados", WHITE, BLACK, SMALL);
				drawString(70, 50, "-> KEY 2", WHITE, BLACK, SMALL);
				drawString(30, 60, "3) Disparo ultrasonidos:", WHITE, BLACK, SMALL);	
				drawString(70, 70, "-> ISP", WHITE, BLACK, SMALL);
			}
		  if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);			// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;    // Enable Timer 3
				FLAG_init_timer  =  1;					// Timer 0 inicializado
			}
			
//			if((FLAG_KEY1 == 1)&&(grados > 0))  // Se mueve -10º
//			{
//				set_servo(grados -= 10);    // Se mueve -10º
//				FLAG_KEY1 = 0;			 	      // Borramos flag de KEY1
//			}
//			
//			if((FLAG_KEY2 == 1)&&(grados < 180))  // Se mueve +10º
//			{
//				set_servo(grados += 10);    // Se mueve +10º
//				FLAG_KEY2 = 0;			 	      // Borramos flag de KEY2
//			}
//			
//			if(FLAG_ISP == 1)  // Dispara
//			{
//				generar_pulso_alto();			// Trigger al ultrasonidos	
//				FLAG_ISP = 0;			 	      // Borramos flag de ISP
//			}
//			if(FLAG_TIMER0 == 1)
//			{
//				//leer_Temperatura_I2C();			// Leemos temperatura del sensor I2C
//				//LPC_ADC->ADCR |= (1<<24);	  // Empezar conversion ADC
//				//mostrar_resultados();				// Mostrar resultados por pantalla
//				FLAG_TIMER0    =  0;    			// Borramos flag del timer 0
//			}
			
		break;
		
		case 112: // Modo manual - medidas continuas (offline)
			if(FLAG_menu_1==0)
			{
				FLAG_menu_1 = 1;
				
				fillScreen(BLACK);
			
				drawString(30, 10, "Elija una opcion:", WHITE, BLACK, SMALL);
				drawString(30, 20, "1) Servo: -10 grados", WHITE, BLACK, SMALL);
				drawString(70, 30, "-> KEY 1", WHITE, BLACK, SMALL);
				drawString(30, 40, "2) Servo: +10 grados", WHITE, BLACK, SMALL);
				drawString(70, 50, "-> KEY 2", WHITE, BLACK, SMALL);
				drawString(30, 60, "3) Disparo ultrasonidos:", WHITE, BLACK, SMALL);	
				drawString(70, 70, "-> ISP", WHITE, BLACK, SMALL);
			}
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);			// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;    // Enable Timer 3
				FLAG_init_timer  =  1;					// Timer 0 inicializado
				set_servo(grados=0);
			}
			if((FLAG_KEY1 == 1)&&(grados > 0))  // Se mueve -10º
			{
				FLAG_KEY1 = 0;			 	      // Borramos flag de KEY1
				set_servo(grados -= 10);    // Se mueve -10º
			}
			if((FLAG_KEY2 == 1)&&(grados < 180))  // Se mueve +10º
			{
				FLAG_KEY2 = 0;			 	      // Borramos flag de KEY2
				set_servo(grados += 10);    // Se mueve +10º
			}
			if(FLAG_TIMER0 == 1)
			{
				//generar_pulso_alto();				// Activamos ultrasonidos
				//leer_Temperatura_I2C();			// Leemos temperatura del sensor I2C
				//LPC_ADC->ADCR |= (1<<24);	  // Empezar conversion ADC
				//mostrar_resultados();				// Mostrar resultados por pantalla
				FLAG_TIMER0    =  0;    			// Borramos flag del timer 0
			}
		break;
		
		case 12:	// Modo Automatico (offline)
			if(FLAG_menu_1==0)
			{
				FLAG_menu_1 = 1;
		
				fillScreen(BLACK);
				drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
				drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
				drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
				drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
			}
			if(FLAG_init_servo == 0)
			{
				set_servo(grados = 0);	 // Inicializamos servo a la posicion 0º
				FLAG_init_servo = 1;		 // Servo inicializado
			}
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);				// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;      // Enable Timer	3		
				FLAG_init_timer  =  1;						// Timer 0 inicializado
			}
			if(FLAG_TIMER0 == 1)
			{
				//generar_pulso_alto();				// Activamos ultrasonidos
				//leer_Temperatura_I2C();			// Leemos temperatura del sensor I2C
				//LPC_ADC->ADCR |= (1<<24);	  // Empezar conversion ADC
				
//				if(grados < 180)						// Movemos el servo
//					set_servo(grados += 10);	
//					
//				else 												// Cuando llega al final vuelve
//					set_servo(grados = 0);		// Servo en posicion 0
				
				//mostrar_resultados();				// Mostrar resultados por pantalla
				FLAG_TIMER0 = 0;    			  	// Borramos flag del timer 0
			}
		break;

		case 13:	// Modo Deteccion obstaculos (offline)
			if(FLAG_menu_1==0)
			{
				FLAG_menu_1 = 1;
				fillScreen(BLACK);
				drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
				drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
				drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
				drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
			}
			
//			if(distancia <= umbral)
//			{
//				frecuencia    = 5000-umbral*10;								  		//Aplico la formula dada
//				if (DAC_ON == 0)
//				{
//					LPC_TIM1->MR0 = (Fpclk/frecuencia/N_muestras)-1;		//Saco la frecuencia por el Match
//					LPC_TIM1->TCR = (1<<0)|(0<<1);															// Habilitamos TIMER1		
//					DAC_ON = 1;
//				}
//			}
//			else if (DAC_ON == 1)
//			{
//			  LPC_TIM1->TCR = (0<<0)|(1<<1);			// Deshabilitamos time1
//			  DAC_ON = 0;
//			}
			if(FLAG_init_servo == 0)
			{
				set_servo(grados = 90);	 // Inicializamos servo a la posicion 90º
				FLAG_init_servo = 1;		 // Servo inicializado
			}
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);				// Habilitamos timer 0
				LPC_TIM3->TCR    = 0x01;          // Enable Timer 3
				FLAG_init_timer  =  1;						// Timer 0 inicializado
			}			
			if((FLAG_KEY1 == 1)&&(grados > 0))  // Se mueve -10º
			{
				set_servo(grados -= 10);    // Se mueve -10º
				FLAG_KEY1 = 0;			 	      // Borramos flag de KEY1
			}			
			if((FLAG_KEY2 == 1)&&(grados < 180))  // Se mueve +10º
			{
				set_servo(grados += 10);    // Se mueve +10º
				FLAG_KEY2 = 0;			 	      // Borramos flag de KEY2
			}		
			if(FLAG_TIMER0 == 1)
			{
				FLAG_TIMER0 = 0;    			  	// Borramos flag del timer 0
			}			
	
			break;
					
		case 2:		// Modo online
			fillScreen(BLACK);
			do{
			    tx_cadena_UART0("Introduce: \n 1. Modo manual \n 2. Modo automatico \n 3. Modo deteccion de obstaculos \n\r");
		      while(rx_completa==0); // Espera a que la recepcion del mensaje acabe con un ENTER (CR)
		      rx_completa=0;				// Borrar flag
					if(strcmp (buffer, "1\r") == 0)
					{
            mode = 21;
            aux=1;
          } 
					else if(strcmp (buffer, "2\r") == 0) 
					{
            mode = 22;
            aux=1;
          } 
					else if(strcmp (buffer, "3\r") == 0) 
					{
            mode = 23;
            aux=1;
          } 
					else 
					{
			        tx_cadena_UART0("\n\rOpcion seleccionada incorrecta \n\r");
			    }			    
		   } while (aux==0);
			aux=0;
//					tx_cadena_UART0("Introduce: \n 1. Modo manual \n 2. Modo automatico \n 3. Modo deteccion de obstaculos \n\r");
//					while(rx_completa==0);
//					rx_completa=0;
//					tx_cadena_UART0("HE SALIDO \n\r");
//				while(aux==0)
//				{
//					if(rx_completa==0)
//					rx_completa=0;
//					
//					if(strcmp(buffer,"1\r")==0)
//					{
//						mode=21;
//						aux=1;
//					}
//					else if(strcmp(buffer,"2\r")==0)
//					{
//						mode=22;
//						aux=1;
//					}
//					else if(strcmp(buffer,"3\r")==0)
//					{
//						mode=23;
//						aux=1;
//					}
//					else
//					{
//						sprintf(buffer, "Opcion seleccionada incorrecta \n\r");
//						tx_cadena_UART0(buffer);
//						while(tx_completa==0);
//						tx_completa=0;					//Se transmite la cadena de buffer	
//					}
//				}
//			do
//			{
//        //Input display message
//			  tx_cadena_UART0("Introduce: \n 1. Modo manual \n 2. Modo automatico \n 3. Modo deteccion de obstaculos \n\r");
//				if(rx_completa)
//				{
//					rx_completa=0;
//					if(strcmp (buffer, "1\r")==0)
//						mode = 21;
//					
//					else if(strcmp (buffer, "2\r")==0)
//						mode = 22;
//					
//					else if(strcmp (buffer, "3\r")==0)
//						mode = 23;
//					else
//					{
//						sprintf(buffer, "Opcion seleccionada incorrecta \n\r");
//						tx_cadena_UART0(buffer);
//						while(tx_completa==0);
//						tx_completa=0;					//Se transmite la cadena de buffer	
//					}
//				}
//			}
//			while ((mode!=21)||(mode!=22)||(mode!=23));
//			
//				while(rx_completa==0); // Espera introducir una cadena de caracteres terminada con CR (0x0D)
//				rx_completa=0;				// Borrar flag
//	
//						//Selection of the mode depending on the input
//						if(strcmp (buffer, "1\r") == 0) 
//						mode=21;
//						
//						else if(strcmp (buffer, "2\r") == 0) 
//						mode=22;
//						
//						else if(strcmp (buffer, "3\r") == 0) 
//						mode=23;
//						
//						else 
//						{
//							sprintf(buffer, "Opcion seleccionada incorrecta \n\r");
//							tx_cadena_UART0(buffer);
//							while(tx_completa==0);
//							tx_completa=0;					//Se transmite la cadena de buffer	
//							
////									for(contador=0; contador<500000;contador++); //Retardo, s?lo con el prop?sito de dar tiempo a que la UART termine de enviar el anterior mensaje de error
//						}
//		    } 
//			  while ((mode!=21)||(mode!=22)||(mode!=23));

	
		break;
			
		case 21:
			do
			{
				tx_cadena_UART0("Introduce: \n 1. Disparo unico \n 2. Disparos continuos \n\r");
				while(rx_completa==0);
				rx_completa=0;				// Borrar flag
				if(strcmp (buffer, "1\r") == 0) 
				{
					mode = 211;
					aux=1;
				} 
				else if(strcmp (buffer, "2\r") == 0) 
				{
					mode = 212;
					aux=1;
				}  
				else 
				{
					tx_cadena_UART0("\n\rOpcion seleccionada incorrecta\n\r");
				}			
		  } while (aux==0);
			aux=0;
			if(FLAG_init_servo == 0)
			{
				set_servo(grados = 90);	 // Inicializamos servo a la posicion 90º
				FLAG_init_servo = 1;		 // Servo inicializado
			}
		break;
		
		case 211:	// Modo manual - medidas manuales (online)
			do
			{
				if(aux2 == 0)
				{
					tx_cadena_UART0("Introduce: \n 1. Mover el servo a la derecha -> + \n 2. Mover el servo hacia la izquierda -> - \n 3. Disparar -> S \n\r");
					while(tx_completa==0);
					tx_completa=0;					//Se transmite la cadena de buffer	
					aux2=1;
				}
				while(rx_completa==0);
				rx_completa=0;				// Borrar flag
				if(strcmp (buffer, "+\r") == 0) 
				{
					if(grados<190)
						set_servo(grados+=10);
//					sprintf(buffer_grad, "\n Angulo del servo : %d grados          ", grados );
//					tx_cadena_UART0(buffer_grad);
//					while(tx_completa==0);
//					tx_completa=0;					//Se transmite la cadena de buffer	
					aux=1;
				} 
				else if(strcmp (buffer, "-\r") == 0) 
				{
					if(grados<190)
						set_servo(grados-=10);
//					sprintf(buffer_grad, "\n Angulo del servo : %d grados          ", grados );
//					tx_cadena_UART0(buffer_grad);
//					while(tx_completa==0);
//					tx_completa=0;					//Se transmite la cadena de buffer	
					aux=1;
				}  
				else if(strcmp (buffer, "S\r") == 0)
				{
					generar_pulso_alto();
				}		
				else 
				{
					tx_cadena_UART0("\n\rOpcion seleccionada incorrecta\n\r");
				}			

			} while (aux==0);
			aux=0;
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);			// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;    // Enable Timer 3
				FLAG_init_timer  =  1;					// Timer 0 inicializado
			}

		break;
		
		case 212: // Modo manual - medidas continuas (online)
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);			// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;    // Enable Timer 3
				FLAG_init_timer  =  1;					// Timer 0 inicializado
				set_servo(grados=0);
			}
			do
			{
				if(aux2 == 0)
				{
					tx_cadena_UART0("Introduce: \n 1. Mover el servo a la derecha -> + \n 2. Mover el servo hacia la izquierda -> - \n 3. Disparar -> S \n\r");
					aux2=1;
				}
				while(rx_completa==0);
				rx_completa=0;				// Borrar flag
				if(strcmp (buffer, "+\r") == 0) 
				{
					if(grados<190)
						set_servo(grados+=10);
//					sprintf(buffer_grad, "\n Angulo del servo : %d grados          ", grados );
//					tx_cadena_UART0(buffer_grad);
//					while(tx_completa==0);
//					tx_completa=0;					//Se transmite la cadena de buffer	
					//aux3 = 1;
						aux=1;
				}
				else if(strcmp (buffer, "-\r") == 0) 
				{
					if(grados>0)
						set_servo(grados-=10);
//					sprintf(buffer_grad, "\n Angulo del servo : %d grados          ", grados );
//					tx_cadena_UART0(buffer_grad);
//					while(tx_completa==0);
//					tx_completa=0;					//Se transmite la cadena de buffer	
					//aux3 = 1;
						aux=1;
				}  
				else if(strcmp (buffer, "S\r") == 0)
				{
					disparo = 1;
					aux = 1;
				}		
				else 
				{
					tx_cadena_UART0("\n\rOpcion seleccionada incorrecta\n\r");
					aux=1;
				}
				//aux3 = 0;				
		  } while (aux==0);
			aux=0;

		break;
		
		case 22:	// Modo Automatico (online)
			if(FLAG_init_servo == 0)
			{
				set_servo(grados = 0);	 // Inicializamos servo a la posicion 90º
				FLAG_init_servo = 1;		 // Servo inicializado
			}
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);				// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;      // Enable Timer	3		
				FLAG_init_timer  =  1;						// Timer 0 inicializado
			}
		break;

		case 23:	// Modo Deteccion obstaculos (online)

			do
			{
				if(FLAG_init_servo == 0)
				{
					set_servo(grados = 90);	 // Inicializamos servo a la posicion 90º
					FLAG_init_servo = 1;		 // Servo inicializado
				}
				if(FLAG_init_timer == 0)
				{
					LPC_TIM0->TCR   |= (1<<0);				// Habilitamos timer 0
					LPC_TIM3->TCR    = 0x01;          // Enable Timer 3
					FLAG_init_timer  =  1;						// Timer 0 inicializado
				}
				
				if(aux2 == 0)
				{
					tx_cadena_UART0("Introduce: \n 1. Mover el servo a la derecha -> + \n 2. Mover el servo hacia la izquierda -> - \n 3. Disparar -> S \n 4. Establecer nuevo umbral -> U \n\r");
					while(tx_completa==0);
					//tx_completa=0;					//Se transmite la cadena de buffer	
					//aux2=1;
				}	
				while(rx_completa==0);
				//rx_completa=0;				// Borrar flag
				if(strcmp (buffer, "+\r") == 0) 
				{
					if(grados<180)
						set_servo(grados+=10);						
					aux=1;
				} 
				else if(strcmp (buffer, "-\r") == 0) 
				{
					if(grados>0)
						set_servo(grados-=10);
					aux=1;		
				}  
		
				else if(strcmp (buffer, "U\r") == 0)
				{
					tx_cadena_UART0("Introduzca el nuevo umbral, comprendido entre 0 y 300: \n\r");	
					while(tx_completa==0);
					tx_completa=0;					//Se transmite la cadena de buffer
					while(aux4==0)
					{
						if(rx_completa)
						{
							rx_completa=0;
							aux5=atoi(buffer);
							if((umbral<=300)&&(umbral>=30))
							{	
								umbral=aux5;
								sprintf(buffer_umbral, "Nuevo umbral seleccionado: %d \n\r", umbral);
								tx_cadena_UART0(buffer_umbral);
								while(tx_completa==0);
								tx_completa=0;					//Se transmite la cadena de buffer			
								aux4 = 1;
							}		
							else
							{
								umbral=umbral;
								sprintf(buffer_umbral, "Valor fuera de rango \n\r");
								tx_cadena_UART0(buffer_umbral);
								while(tx_completa==0);
								tx_completa=0;					//Se transmite la cadena de buffer
								aux4 = 1;
							}
						}
					}
					mostrar_resultados();
					aux=1;
				}
				else 
				{
					tx_cadena_UART0("\n\rOpcion seleccionada incorrecta\n\r");
					aux=1;
				}		
			
			}while (aux==0);
			aux=0;		
		break;
	
		}
	}
}

