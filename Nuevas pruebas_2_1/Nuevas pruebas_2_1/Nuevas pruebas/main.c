#include <LPC17xx.H>
#include "lcddriver.h"
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <string.h>

#include "configuraciones.h"
#include "funciones.h"
#include "interrupciones.h"

#define F_cpu 100e6				// Defecto Keil (xtal=12Mhz)
#define Fpclk 25e6				// Fcpu/4 (defecto después del reset)
#define Tpwm 15e-3				// Perido de la señal PWM (15ms)
#define N_muestras 32 		// muestras del DAC (antes eran 20)
#define PI 3.141592

////////////// ZONA PARA DEFINES ////////////
#define TEST     		 		9
#define INICIO   		 		0
#define OFFLINE  		 	  1
#define OFFLINE_M 		 11
#define OFFLINE_A 		 12
#define OFFLINE_D 		 13
#define OFFLINE_M_U  	111
#define OFFLINE_M_C  	112
#define ONLINE   		    2
#define ONLINE_M  		 21
#define ONLINE_A  		 22
#define ONLINE_D  		 23
#define ONLINE_M_U  	211
#define ONLINE_M_C  	212

////////////// ZONA PARA FLAGS //////////////
uint8_t FLAG_ISP   	= 0;		// Flag boton ISP
uint8_t FLAG_KEY1  	= 0;		// Flag boton KEY1
uint8_t FLAG_KEY2  	= 0;		// Flag boton KEY2
uint8_t FLAG_TIMER0 = 0;  // Flag timer 0
uint8_t FLAG_TIMER1 = 0;  // Flag timer 1
uint8_t FLAG_TIMER2 = 0;  // Flag timer 2
uint8_t FLAG_TIMER3 = 0;  // Flag timer 3
uint8_t FLAG_EINT0_PULSO;
uint8_t FLAG_EINT_SERVO;
uint8_t FLAG_AUTO;
uint8_t FLAG_DETEC;
uint8_t FLAG_ONLINE;
uint8_t FLAG_OFFLINE;
uint8_t DAC_ON = 0;						// Flag activacion DAC
uint8_t FLAG_MENU = 0;
uint8_t FLAG_init_timer = 0;
uint8_t FLAG_DISPARO_CONTINUO = 0;
//////////// ZONA PARA VARIABLES GLOBALES//////////

uint8_t mode		  = 0;  // Variable de modo activo 
uint8_t grados    = 0;	 // Variable grados girados [0º - 180º]
uint8_t pulsos    = 0;	 // Variable pulsos servo [0 - 18]
float distancia   = 0;  // Variable distancia medida (cm)
float temperatura_global; 	
float temperaturaI2C=0;
int		temp				= 0;
float temp_I2C		= 30;
int 	umbral     	= 100;  // Variable distancia de deteccion (cm)
float frecuencia 	= 4000; // Variable frecuencia seno (Hz)
uint16_t muestras[N_muestras];  // Array que contiene al seno 





char buffer[30] ; // Buffer de recepción de 200 caracteres

char *ptr_rx; // Puntero de recepción
char rx_completa; // Flag de recepción de cadena que se activa a "1" al recibir la tecla return CR(ASCII=13)
char *ptr_tx; // puntero de transmisión
char tx_completa; // Flag de transmisión de cadena que se activa al transmitir el caracter null (fin de cadena)	
char aux = 0;
//char aux2 = 0;
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
	config_prioridades();
	ptr_rx=buffer;
	uart0_init(9600);
		
	
	while(1)
	{	
		switch(mode)
		{
			case INICIO:
				if(FLAG_MENU == 0)
				{
					FLAG_MENU	= 1;	// Evitamos que ponga continuamente el menu
					//Encabezado

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
					FLAG_MENU = 0;			// Permitimos que aparezca el siguiente menu
					mode        = 1;			// Modo offline seleccionado
				}
				if(FLAG_KEY2 == 1)  		// Se ha pulsado KEY2 (online)
				{
					FLAG_KEY2   = 0;			// Borramos flag de KEY2
					FLAG_ISP    = 0;		  // Borramos flag de ISP por si acaso
					FLAG_MENU = 0; 			// Permitimos que aparezca el siguiente menu 
					mode        = 2;	  	// Modo online seleccionado
				}
			break;


			case OFFLINE:
				if(FLAG_MENU == 0)
				{
					FLAG_MENU = 1;
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
					FLAG_MENU = 0; 			// Permitimos que aparezca el siguiente menu 
				}
				if(FLAG_KEY2 == 1)  // Se ha pulsado KEY2 (automatico)
				{
					FLAG_KEY2   = 0;			// Borramos flag de KEY2
					mode        = 12;	  	// Modo automatico seleccionado
					FLAG_MENU = 0; 			// Permitimos que aparezca el siguiente menu 
				}
				if(FLAG_ISP == 1)   // Se ha pulsado ISP (deteccion)
				{
					FLAG_ISP    = 0;			// Borramos flag de KEY2
					mode        = 13;	  	// Modo deteccion seleccionado
					FLAG_MENU 	= 0; 			// Permitimos que aparezca el siguiente menu 
				}	

			break;
			case OFFLINE_M:	// Modo manual (offline)
					
					if(FLAG_MENU == 0)
					{
						FLAG_MENU = 1; 	
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
						set_servo(grados = 90);	 // Inicializamos servo a la posicion 90º
					}
					
					if(FLAG_KEY1 == 1)	// Se ha pulsado KEY1 (único)
					{
						FLAG_KEY1   = 0;			  		// Borramos flag de KEY1	
						mode        = OFFLINE_M_U;			// Modo disparo unico seleccionado
						FLAG_MENU = 0; 		  				// Permitimos que aparezca el siguiente menu 
					}
					
					if(FLAG_KEY2 == 1)  // Se ha pulsado KEY2 (continuo)
					{
						FLAG_KEY2   = 0;			  		// Borramos flag de KEY2
						mode        = OFFLINE_M_C;	// Modo disparos continuos seleccionado
						FLAG_MENU = 0; 			  			// Permitimos que aparezca el siguiente menu 
					}
			break;
			
			case OFFLINE_M_U:	// Modo manual - medida unica (offline)
				if(FLAG_MENU == 0)
				{
					FLAG_MENU = 1;
					fillScreen(BLACK);

					drawString(30, 10, "Elija una opcion:", WHITE, BLACK, SMALL);
					drawString(30, 20, "1) Servo: -10 grados", WHITE, BLACK, SMALL);
					drawString(70, 30, "-> KEY 1", WHITE, BLACK, SMALL);
					drawString(30, 40, "2) Servo: +10 grados", WHITE, BLACK, SMALL);
					drawString(70, 50, "-> KEY 2", WHITE, BLACK, SMALL);
					drawString(30, 60, "3) Disparo ultrasonidos:", WHITE, BLACK, SMALL);	
					drawString(70, 70, "-> ISP", WHITE, BLACK, SMALL);
					FLAG_EINT0_PULSO = 1;
					FLAG_EINT_SERVO  = 1;
					FLAG_OFFLINE = 1;
				}
				if(FLAG_init_timer == 0)
				{
					LPC_TIM0->TCR   |= (1<<0);			// Habilitamos timer 0
					LPC_TIM3->TCR        = 0x01;    // Enable Timer 3
					FLAG_init_timer  =  1;					// Timer 0 inicializado
				}
				
				
			
				
			break;
			
			case OFFLINE_M_C: // Modo manual - medidas continuas (offline)
				if(FLAG_MENU ==0)
				{
					FLAG_MENU = 1;
					fillScreen(BLACK);
					drawString(30, 10, "Elija una opcion:", WHITE, BLACK, SMALL);
					drawString(30, 20, "1) Servo: -10 grados", WHITE, BLACK, SMALL);
					drawString(70, 30, "-> KEY 1", WHITE, BLACK, SMALL);
					drawString(30, 40, "2) Servo: +10 grados", WHITE, BLACK, SMALL);
					drawString(70, 50, "-> KEY 2", WHITE, BLACK, SMALL);
					drawString(30, 60, "3) Disparo ultrasonidos:", WHITE, BLACK, SMALL);	
					drawString(70, 70, "-> ISP", WHITE, BLACK, SMALL);
					FLAG_EINT_SERVO  = 1;
					FLAG_OFFLINE = 1;
				}
				if(FLAG_init_timer == 0)
				{
					LPC_TIM0->TCR   |= (1<<0);			// Habilitamos timer 0
					LPC_TIM3->TCR        = 0x01;    // Enable Timer 3
					FLAG_init_timer  =  1;					// Timer 0 inicializado
				}
				
				if(FLAG_ISP)
					FLAG_DISPARO_CONTINUO = 1;
				
			break;
			
			case OFFLINE_A:	// Modo Automatico (offline)
				if(FLAG_MENU==0)
				{
					FLAG_MENU = 1;
			
					fillScreen(BLACK);
					drawString(100,22, "UAH-SED", BLACK, WHITE, MEDIUM);
					drawString(20, 52, "PROYECTO SONAR ULTRASONICO", WHITE, BLACK, MEDIUM);
					drawString(20, 72, "Ana Belen Bartolome", WHITE, BLACK, MEDIUM);
					drawString(20, 92, "Cesar Murciego", WHITE, BLACK, MEDIUM);
					set_servo(grados = 0);
					FLAG_OFFLINE = 1;
					FLAG_AUTO 	 = 1;
				}
				if(FLAG_init_timer == 0)
				{
					LPC_TIM0->TCR   |= (1<<0);				// Habilitamos timer 0
					LPC_TIM3->TCR        = 0x01;      // Enable Timer	3		
					FLAG_init_timer  =  1;						// Timer 0 inicializado
				}
			break;	
					
				
			case ONLINE:
				fillScreen(BLACK);
				do
				{
					if(FLAG_MENU ==0)
					{
						tx_cadena_UART0("Introduce: \n 1. Modo manual \n 2. Modo automatico \n 3. Modo deteccion de obstaculos \n\r");
						while(tx_completa==0); // Espera a que la recepcion del mensaje acabe con un ENTER (CR)
						FLAG_MENU = 1;
					}	
					if(rx_completa)
						{
						if(strcmp (buffer, "1\r") == 0)
								mode = ONLINE_M;
						 
						else if(strcmp (buffer, "2\r") == 0) 
							mode = ONLINE_A;
	
						
						else if(strcmp (buffer, "3\r") == 0) 
							mode = ONLINE_D;
						
						else 
							tx_cadena_UART0("\n\rOpcion seleccionada incorrecta \n\r");
						rx_completa = 0;
					}
				}while (mode == ONLINE);	
				FLAG_MENU = 0;
				FLAG_ONLINE = 1;
			break;

			
			
			case ONLINE_M:
				
				set_servo(grados = 90);	 // Inicializamos servo a la posicion 90º
			
				do
				{
					if(FLAG_MENU ==0)
					{
						tx_cadena_UART0("Introduce: \n 1. Disparo unico \n 2. Disparos continuos \n\r");
						while(tx_completa==0); // Espera a que la recepcion del mensaje acabe con un ENTER (CR)
						FLAG_MENU = 1;
					}	
					if(rx_completa == 1)
					{
						
						if(strcmp (buffer, "1\r") == 0) 
							mode = ONLINE_M_U;
						else if(strcmp (buffer, "2\r") == 0) 
							mode = ONLINE_M_C;
						else 
							tx_cadena_UART0("\n\rOpcion seleccionada incorrecta\n\r");
						rx_completa = 0;
					}
								
				} while (mode == ONLINE_M);
				FLAG_MENU = 0;
			break;
				
			case ONLINE_M_U:	// Modo manual - medidas manuales (online)
								
				if(FLAG_init_timer == 0)
				{
					LPC_TIM0->TCR   |= (1<<0);	// Habilitamos timer 0
					LPC_TIM3->TCR    =  0x01;   // Enable Timer 3
					FLAG_init_timer  =  1;			// Timer 0 inicializado
				}
				
				if(FLAG_MENU == 0)
				{
					tx_cadena_UART0("Introduce: \n 1. Mover el servo a la derecha -> + \n 2. Mover el servo hacia la izquierda -> - \n 3. Disparar -> S \n\r");
					while(tx_completa == 0);
					FLAG_MENU = 1;
				}
				
				if(rx_completa == 1)
				{
				
					if(strcmp (buffer, "+\r") == 0) 
					{
						if(grados<180)
							set_servo(grados+=10);
					} 
					else if(strcmp (buffer, "-\r") == 0) 
					{
						if(grados<180)
							set_servo(grados-=10);
					}  
					else if(strcmp (buffer, "S\r") == 0)
						generar_pulso_alto();

					else 
						tx_cadena_UART0("\n\rOpcion seleccionada incorrecta\n\r");
					rx_completa = 0;
				}
		break;
			
		case ONLINE_M_C: // Modo manual - medidas continuas (online)
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);			// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;    // Enable Timer 3
				FLAG_init_timer  =  1;					// Timer 0 inicializado
			}
			
			if(FLAG_MENU == 0)
			{
				tx_cadena_UART0("Introduce: \n 1. Mover el servo a la derecha -> + \n 2. Mover el servo hacia la izquierda -> - \n 3. Disparar -> S \n\r");
				while(tx_completa == 0);
				FLAG_MENU=1;
			}
				
				
			if(rx_completa == 1)
			{
				if(strcmp (buffer, "+\r") == 0) 
				{
					if(grados<180)
						set_servo(grados+=10);

				}
				else if(strcmp (buffer, "-\r") == 0) 
				{
					if(grados>0)
						set_servo(grados-=10);
				}  
				else if(strcmp (buffer, "S\r") == 0)
				{
					FLAG_DISPARO_CONTINUO = 1;
				}		
				else 
				{
					tx_cadena_UART0("\n\rOpcion seleccionada incorrecta\n\r");
				}

			}
			
		break;
			
		case ONLINE_A:	// Modo Automatico (online)
			
			if(FLAG_init_timer == 0)
			{
				LPC_TIM0->TCR   |= (1<<0);				// Habilitamos timer 0
				LPC_TIM3->TCR        = 0x01;      // Enable Timer	3		
				FLAG_init_timer  =  1;						// Timer 0 inicializado
				set_servo(grados = 0);
				FLAG_ONLINE = 1;
				FLAG_AUTO 	 = 1;
			}
		break;

		
			
			
		case TEST:
			set_servo(10);
		break;
			
		}
	}
}
