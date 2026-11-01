/*
 * pwm.c
 *
 * Created: 2026-06-18 오후 2:23:02
 *  Author: user
 */ 

#include "pwm.h"
#include "fnd.h"

extern int get_button(int button_num, int button_pin);
static char direction;

void init_timer3_pwm(void);
void init_motor_driver(void);
void dcmotor_pwm_control_main(void);

// 16bit 1번 timer/counter를 사용
/*
   PWM 출력 신호
   ============
   PB5 : OC1A   : 왼쪽 바퀴
   PB6 : OC1B :  오른쪽 바퀴 
   BTN0 : auto/manual mode 설정 
   
   방향 설정
   =========
   1. 왼쪽 바퀴 
      PORTF0 -- IN1  ( DC motor Driver) 
	  PORTF1 -- IN2
   2. 오른쪽 바퀴
      PORTF2 -- IN3  ( DC motor Driver)
      PORTF3 -- IN4
	  
	  IN1/IN3     IN2/IN4
	  =======    ========
	     0           1  : 역회전
		 1           0  : 정회전
		 1           1  : stop  
*/

#define MOTOR_PWM_DDR  DDRB
#define MOTOR_LEFT_PORT_DDR  5    // OC1A
#define MOTOR_RIGHT_PORT_DDR  6   // OC1B

#define MOTOR_DIRECTION_PORT  PORTF
#define MOTOR_DIRECTION_PORT_DDR  DDRF

void init_motor_driver(void);
void init_timer1_pwm(void);

void forward(int speed);
void backward(int speed);
void turn_left(int speed);
void turn_right(int speed);
void stop(void);

void init_timer1_pwm(void)
{
	//---- 분주비 설정 ----------
	// 분주비 64
	// 16000000Hz / 64 --> 250000Hz (250KHz)
	// T=1/f  1/250000Hz  --> 0.000004sec ---> 4us
	// 250000Hz에서 256개 펄스를 count하면 소요 시간: 1.02ms
	//              127                           : 0.5ms
	//              0x3ff(1023)  ---> 4ms
	// TCNT3 : 0~255(0x00ff) 까지 count한 후 0으로 다시 돌아 간다.
	TCCR1B |= 1 << CS11 | 1 << CS10;   // 분주비 64
	// OCR3C : 50 인경우 Duty (HIGH)가 몇 % 인가 ?
	// Duty Cycle = (OCR3C / TOP) x 100 = 50 / 255 x 100 = 19.61%
	
	// 모드 14 :  고속 PWM 모드 사용 하겠다. timer1 (P327 표14-5)
	TCCR1A |= 1 << WGM11;     // TOP --> ICR1으로 설정 
	TCCR1B |= 1 << WGM13 | 1 << WGM12;
	
	// 비반전 모드 top : ICR1 비교일치 값 (PWM) 지정 OCR1A OCR1B P350 표15-7  
	// 비교 일치 발생시 OCR1, OCR1B의 출력핀은 LOW로 바뀌고 BOTTOM에서 HIGH로 바뀐다. 
	TCCR1A |= 1 << COM1A1;  
	TCCR1A |= 1 << COM1B1;  
	
	ICR1 = 0x3ff;  // 1023 * 4us ==> 4ms TOP 값 

	
}

/*
   방향 설정
   =========
   1. 왼쪽 바퀴
   PORTF0 -- IN1  ( DC motor Driver)
   PORTF1 -- IN2
   2. 오른쪽 바퀴
   PORTF2 -- IN3  ( DC motor Driver)
   PORTF3 -- IN4
   
   IN1/IN3     IN2/IN4
   =======    ========
   0           1  : 역회전
   1           0  : 정회전
   1           1  : stop
*/
void init_motor_driver(void)
{
	// 1. 출력 모드로 설정
	MOTOR_PWM_DDR &= ~(1 << 5 | 1 << 6);   // 0 으로 초기화 하고 시작 하자 
	MOTOR_PWM_DDR |= 1 << 5 | 1 << 6;
	MOTOR_DIRECTION_PORT_DDR &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DIRECTION_PORT_DDR |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;   // 출력 모드로 설정
	
	// 2. 모터를 전진 모드로 
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);  // 0 으로 초기화 하고 시작 
	MOTOR_DIRECTION_PORT |=  1 << 2 | 1 << 0;   // 모터를 전진 모드로 IN4IN3IN2IN1
	                                            //                    0  1 0  1
}

void forward(int speed)
{
	fnd_set_motion(MOTION_FORWARD);

	//  모터를 전진 모드로
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);  // 0 으로 초기화 하고 시작
	MOTOR_DIRECTION_PORT |=  1 << 2 | 1 << 0;   // 모터를 전진 모드로 IN4IN3IN2IN1
							//                    0  1 0  1	
	OCR1A=OCR1B=speed;   // OCR1A: PWM left OCR1B: PWM right
}

void backward(int speed)
{
	fnd_set_motion(MOTION_BACKWARD);

	//  모터를 후진 모드로
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);  // 0 으로 초기화 하고 시작
	MOTOR_DIRECTION_PORT |=  1 << 3 | 1 << 1;   // 모터를 후진 모드로 IN4IN3IN2IN1
	                                            //                    1 0  1  0
	OCR1A=OCR1B=speed;   // OCR1A: PWM left OCR1B: PWM right
}

void turn_left(int speed)
{
	fnd_set_motion(MOTION_LEFT);

	//  모터를 전진 모드로
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);  // 0 으로 초기화 하고 시작
	MOTOR_DIRECTION_PORT |=  1 << 2 | 1 << 0;   // 모터를 전진 모드로 IN4IN3IN2IN1
	//                    0  1 0  1
	OCR1A=0;
	OCR1B=speed;   // OCR1A: PWM left OCR1B: PWM right
	direction = 'l';
}

void turn_right(int speed)
{
	fnd_set_motion(MOTION_RIGHT);

	//  모터를 전진 모드로
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);  // 0 으로 초기화 하고 시작
	MOTOR_DIRECTION_PORT |=  1 << 2 | 1 << 0;   // 모터를 전진 모드로 IN4IN3IN2IN1
	//                    0  1 0  1
	OCR1A=speed;
	OCR1B=0;   // OCR1A: PWM left OCR1B: PWM right
	direction = 'r';
}

void stop(void)
{
	fnd_set_motion(MOTION_STOP);

	//  모터를 stop 모드로
	MOTOR_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);  // 0 으로 초기화 하고 시작
	MOTOR_DIRECTION_PORT |=  1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;   // 모터를 stop 모드로 IN4IN3IN2IN1
	//                    0  1 0  1
	OCR1A=0;
	OCR1B=0;   // OCR1A: PWM left OCR1B: PWM right
}
