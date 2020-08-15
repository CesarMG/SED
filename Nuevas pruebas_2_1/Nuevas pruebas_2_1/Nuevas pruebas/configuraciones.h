#include <LPC17xx.H>

#define Fpclk 25e6		// Fcpu/4 (defecto después del reset)
#define Tpwm 15e-3		// Perido de la señal PWM (15ms)

// Macros: para no incluir varias veces las mismas funciones hacemos lo siguiente: 

#ifndef _configuraciones

#define _configuraciones

void config_pines(void);
void config_externas(void);
void config_pwm1(void);
void config_DAC(void);
void config_ADC(void);
void config_DS1621(void);
void config_TIMER0(void);
void config_TIMER1(void);
void config_TIMER2(void);
void config_TIMER3(void);

#endif
