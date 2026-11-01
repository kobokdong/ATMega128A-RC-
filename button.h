/*
 * button.h
 *
 * Created: 2026-06-10 오전 11:45:01
 *  Author: user
 */ 

#ifndef BUTTON_H_
#define BUTTON_H_

#define F_CPU 16000000UL  //16MHz
#include <avr/io.h>  // PORTA PORTB PORTD... IO관련 reg가 들어 있다. 
#include <util/delay.h>  // _delay_ms _delay_us 등 

#define  BUTTON_DDR  DDRB
#define  BUTTON_PIN  PINB   // PORTB를 읽는 register 5v:1  0v:0 

#define  BUTTON0PIN 4    // PORTB.4, 사진의 manual/auto 전환 버튼
#define  BUTTON1PIN 5
#define  BUTTON2PIN 6
#define  BUTTON3PIN 7

#define  BUTTON0   0
#define  BUTTON1   1
#define  BUTTON2   2
#define  BUTTON3   3
#define  BUTTON_NUMBER  4   // 버튼 갯수 

#define  BUTTON_PRESS  1      // 버튼을 누르면 high (active-high)
#define  BUTTON_RELEASE 0     // 버튼을 뗀 상태 (low)

void init_button(void);
int get_button(int button_num, int button_pin);

#endif
