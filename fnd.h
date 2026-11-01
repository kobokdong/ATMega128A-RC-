#ifndef FND_H_
#define FND_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// 5641BS common anode 기준
// PC0~PC7 : a,b,c,d,e,f,g,dp
#define FND_DATA_DDR   DDRC
#define FND_DATA_PORT  PORTC

// 위 FND : 주행시간 / 결과값
#define FND_TOP_DDR    DDRD
#define FND_TOP_PORT   PORTD
#define FND_TOP_FIRST   5

#define FND_TOP1_DDR DDRD
#define FND_TOP1_PORT PORTD
#define FND_TOP_PIN3    4

// 아래 FND : F, b, S, L, r / 서클 / 조회 항목
#define FND_BOTTOM_DDR   DDRF
#define FND_BOTTOM_PORT  PORTF
#define FND_BOTTOM_FIRST   4

typedef enum
{
	MOTION_STOP = 0,
	MOTION_FORWARD,
	MOTION_BACKWARD,
	MOTION_LEFT,
	MOTION_RIGHT
} motion_t;

void init_fnd(void);
void fnd_update(void);
void fnd_set_motion(motion_t motion);
void fnd_finish_drive(void);
unsigned char fnd_next_result_page(void);
void fnd_reset_drive(void);
unsigned char fnd_is_finished(void);

#endif
