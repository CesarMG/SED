#include <LPC17xx.H>
#include "lcddriver.h"
#include <stdio.h>
#include <Math.H>

#define Fpclk			 25e6		// Fcpu/4 (defecto después del reset)
#define N_muestras 32     // muestras del DAC (antes eran 20)
#define PI				 3.141592

#ifndef _funciones

#define _funciones

void  set_servo(uint8_t grados);
void  generar_pulso_alto(void);
void  genera_muestras(uint16_t muestras_ciclo);
float leer_Temperatura_I2C(void);
void  mostrar_resultados(void);
void 	mostrar_medidas_uart(void);

#endif

extern uint16_t muestras[N_muestras];

extern uint8_t grados;
extern float   distancia;
extern float   temperatura_global;
extern float   temperaturaI2C;
extern int 	   temp;
extern uint8_t mode;

extern char linea1[100];
extern char linea2[100];
extern char linea3[100];
extern char linea4[100];

extern char buffer[200]; // Buffer de recepción de 200 caracteres
extern char buffer_dist[100];
extern char buffer_temp_glob[100];
extern char buffer_temp_I2C[100];
extern char *ptr_rx; // Puntero de recepción
extern volatile int rx_completa; // Flag de recepción de cadena que se activa a "1" al recibir la tecla return CR(ASCII=13)
extern char *ptr_tx; // puntero de transmisión
extern char tx_completa; // Flag de transmisión de cadena que se activa al transmitir el caracter null (fin de cadena)	

