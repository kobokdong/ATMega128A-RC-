/*
 * 06.DCMOTOR_PWM_CONTROL
 *
 * Created: 2026-06-15 오전 11:36:45
 * Author : user
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>    // sei 등 함수 
#include <stdio.h>

#include "def.h"
#include "button.h"
#include "ultrasonic.h"
#include "fnd.h"

extern void init_timer1_pwm(void);
extern void init_motor_driver(void);
extern void init_button(void);
extern void init_uart1(void);
extern volatile uint8_t bt_data;
extern void forward(int speed);
extern void backward(int speed);
extern void turn_left(int speed);
extern void turn_right(int speed);
extern void stop(void);

void manual_mode(void);
void auto_mode(void);
void mode_button_check(void);
void init_timer0(void);

int func_state=MANUAL_MODE;

void manual_mode(void)
{
	switch(bt_data) {
		case 'F':
		case 'f':
			forward(500);   // 4us x 500 = 0.002sec(2ms)
			break;
		case 'B':
		case 'b':
			backward(500);	
			break;
		case 'L':
		case 'l':
			turn_left(700);  // 4us x 700 = 0.0028sec 
			break;
		case 'R':
		case 'r':
			turn_right(700);
			break;
		case 'S':
		case 's':
			stop();
			fnd_finish_drive();
			func_state = MANUAL_MODE;
			bt_data = 0;
			break;
		default:
			break;
	}
}

void auto_mode(void)
{
	ultrasonic_auto();
}

void mode_button_check(void)
{
	if (get_button(BUTTON0, BUTTON0PIN))
	{
		if (fnd_is_finished())
		{
			if (fnd_next_result_page())
			{
				fnd_reset_drive();
				func_state = MANUAL_MODE;
				bt_data = 0;
				stop();
			}
			return;
		}

		stop();
		bt_data = 0;

		if (func_state == MANUAL_MODE)
		{
			func_state = AUTO_MODE;
		}
		else
		{
			fnd_finish_drive();
			func_state = MANUAL_MODE;
		}
	}
}

int main(void)
{
   init_uart1();
   init_button();
   init_motor_driver();
   init_timer1_pwm();
   init_ultrasonic();
   init_fnd();
  
   sei();   // 전역(대문) interrupt 허용 
   
    while (1) 
    {
		mode_button_check();
		if (func_state == MANUAL_MODE)
		{
			manual_mode();
		}
		else
		{
			auto_mode();
		}
		fnd_update();
    }
}

