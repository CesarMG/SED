#include <LPC17xx.H>
#include "lcddriver.h"
#include <stdio.h>
#include <Math.H>
#include "uart.h"
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
void  mostrar_resultados_DAC(void);

#endif

extern uint16_t muestras[N_muestras];

extern uint8_t grados;
extern float   distancia;
extern float   temperatura_global;
extern float   temperaturaI2C;
extern int 	   temp;
extern uint8_t mode;


