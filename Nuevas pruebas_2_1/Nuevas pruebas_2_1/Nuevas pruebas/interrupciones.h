#include <LPC17xx.H>
#include "lcddriver.h"
#define N_muestras 32 // muestras del DAC (antes eran 20)

#ifndef _interrupciones

#define _interrupciones
#include <stdio.h>
void  generar_pulso_alto(void);
void  set_servo(uint8_t grados);
float leer_Temperatura_I2C(void);
void  mostrar_resultados(void);
void 	mostrar_medidas_uart(void);



extern char nevera_abierta; 
extern uint8_t FLAG_ISP;	
extern uint8_t FLAG_KEY1;
extern uint8_t FLAG_KEY2;
extern uint8_t FLAG_TIMER0;  // Flag timer 0
extern uint8_t FLAG_TIMER1;  // Flag timer 1
extern uint8_t FLAG_TIMER2;  // Flag timer 2
extern uint8_t FLAG_TIMER3;  // Flag timer 3

extern uint8_t mode;				// variable de modo activo
extern uint8_t modo_selec;	// flag de seleccion de modo terminado
extern uint8_t grados;
extern uint8_t pulsos;
extern float distancia;
extern float temperatura_global;

extern uint16_t muestras[N_muestras];

extern uint8_t mostrar;					//la uso?
extern float prueba;							//la uso?
extern int G;
extern int GA;


extern char linea1[100];
extern char linea2[100];
extern char linea3[100];
extern char linea4[100];

extern char buffer[200];
extern char buffer_dist[100];
extern char buffer_temp_glob[100];
extern char buffer_temp_I2C[100];
extern char buffer_grad[100];
extern char *ptr_rx; // Puntero de recepción
extern volatile int rx_completa; // Flag de recepción de cadena que se activa a "1" al recibir la tecla return CR(ASCII=13)
extern char *ptr_tx; // puntero de transmisión
extern char tx_completa; // Flag de transmisión de cadena que se activa al transmitir el caracter null (fin de cadena)	

extern char disparo;
extern char aux;
extern char aux3;

extern int umbral;
extern float frecuencia;
extern uint8_t DAC_ON ;
extern int aux_time;
extern float temperaturaI2C;

#endif
