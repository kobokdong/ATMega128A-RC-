#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define TRIG_DDR   DDRA
#define TRIG_PORT  PORTA
#define TRIG_LEFT  0    // PA0
#define TRIG_FRONT 1    // PA1
#define TRIG_RIGHT 2    // PA2

#define ECHO_DDR    DDRE
#define ECHO_PINREG PINE
#define ECHO_LEFT   4   // PE4
#define ECHO_FRONT  5   // PE5
#define ECHO_RIGHT  6   // PE6

extern volatile int left_ultrasonic_distance;
extern volatile int front_ultrasonic_distance;
extern volatile int right_ultrasonic_distance;

void init_ultrasonic(void);
void ultrasonic_processing(void);
void ultrasonic_auto(void);

#endif
