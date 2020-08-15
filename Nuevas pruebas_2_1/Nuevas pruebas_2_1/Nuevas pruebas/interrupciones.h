#include <LPC17xx.H>
#include "lcddriver.h"
#define N_muestras 32 // muestras del DAC (antes eran 20)

#ifndef _interrupciones

#define _interrupciones
#include <stdio.h>
#include "uart.h"
void  generar_pulso_alto(void);
void  set_servo(uint8_t grados);
float leer_Temperatura_I2C(void);
void  mostrar_resultados(void);
void 	mostrar_medidas_uart(void);
void  mostrar_resultados_DAC(void);


///////// NUEVO MUNDO ////////
extern uint8_t FLAG_EINT0_PULSO;
extern uint8_t FLAG_EINT_SERVO;
extern uint8_t FLAG_AUTO;
extern uint8_t FLAG_DETEC;
extern uint8_t FLAG_ONLINE;
extern uint8_t FLAG_OFFLINE;

//////////////////////////////
 
extern uint8_t FLAG_ISP;	
extern uint8_t FLAG_KEY1;
extern uint8_t FLAG_KEY2;
extern uint8_t FLAG_ADC;
extern uint8_t FLAG_TIMER0; // Flag timer 0
extern uint8_t FLAG_TIMER1; // Flag timer 1
extern uint8_t FLAG_TIMER2; // Flag timer 2
extern uint8_t FLAG_TIMER3; // Flag timer 3
extern uint8_t mode;			 	// variable de modo activo
extern uint8_t modo_selec;	// flag de seleccion de modo terminado
extern uint8_t FLAG_DISPARO_CONTINUO; // Flag 
extern uint8_t grados;
extern uint8_t pulsos;
extern int umbral;
extern float frecuencia;
extern float distancia;
extern float temperatura_global;

extern uint16_t muestras[N_muestras];
extern float temp_I2C;							//la uso?
extern char disparo;

extern uint8_t DAC_ON ;


#endif
