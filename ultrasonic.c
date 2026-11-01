#include "ultrasonic.h"

#define FRONT_SAFE_CM 15
#define SIDE_SAFE_CM  15
#define SIDE_CRASH_CM 10
#define AUTO_SPEED    300
#define TURN_SPEED    450
#define MAX_ECHO_US   30000UL

volatile int left_ultrasonic_distance = 0;
volatile int front_ultrasonic_distance = 0;
volatile int right_ultrasonic_distance = 0;

extern void forward(int speed);
extern void backward(int speed);
extern void turn_left(int speed);
extern void turn_right(int speed);
extern void stop(void);

static int ultrasonic_read_cm(unsigned char trig_pin, unsigned char echo_pin);

void init_ultrasonic(void)
{
	TRIG_DDR |= (1 << TRIG_LEFT) | (1 << TRIG_FRONT) | (1 << TRIG_RIGHT);
	TRIG_PORT &= ~((1 << TRIG_LEFT) | (1 << TRIG_FRONT) | (1 << TRIG_RIGHT));

	ECHO_DDR &= ~((1 << ECHO_LEFT) | (1 << ECHO_FRONT) | (1 << ECHO_RIGHT));
}

static int ultrasonic_read_cm(unsigned char trig_pin, unsigned char echo_pin)
{
	unsigned long echo_time = 0;
	unsigned long timeout = 0;

	TRIG_PORT &= ~(1 << trig_pin); //초음파 트리거 초기화
	_delay_us(2);
	TRIG_PORT |= (1 << trig_pin); //트리거 high
	_delay_us(10);
	TRIG_PORT &= ~(1 << trig_pin); //트리거 Low

	while (!(ECHO_PINREG & (1 << echo_pin))) //에코 신호를 기다림
	{
		_delay_us(2);
		timeout += 2;
		if (timeout >= MAX_ECHO_US)
		{
			return 400; //지정한 시간 내에 돌아오지 않으면 400값을 리턴
		}
	}

	while (ECHO_PINREG & (1 << echo_pin)) //에코 신호가 high
	{
		_delay_us(2);
		echo_time += 2;
		if (echo_time >= MAX_ECHO_US)
		{
			return 400; // 지정한 시간보다 오래 걸렸으면 400리턴
		}
	}

	return (int)(echo_time / 58); //정상이면 시간 값을 리턴
}

void ultrasonic_processing(void) // 계산한 거리를 변수에 저장
{
	left_ultrasonic_distance = ultrasonic_read_cm(TRIG_LEFT, ECHO_LEFT);
	_delay_ms(10);
	front_ultrasonic_distance = ultrasonic_read_cm(TRIG_FRONT, ECHO_FRONT);
	_delay_ms(10);
	right_ultrasonic_distance = ultrasonic_read_cm(TRIG_RIGHT, ECHO_RIGHT);
	_delay_ms(10);
}

void ultrasonic_auto(void)
{
	// 1. 달리는 중에도 양옆 상황을 알아야 하므로 센서는 3개 다 읽어줍니다.
	front_ultrasonic_distance = ultrasonic_read_cm(TRIG_FRONT, ECHO_FRONT);
	_delay_ms(40); // 딜레이를 약간 줄여서 반응속도 향상

	if (front_ultrasonic_distance > FRONT_SAFE_CM)
	{
		forward(AUTO_SPEED);
		return;

	}
	
	stop();
	_delay_ms(80);
	
	left_ultrasonic_distance = ultrasonic_read_cm(TRIG_LEFT, ECHO_LEFT);
	_delay_ms(40);
	right_ultrasonic_distance = ultrasonic_read_cm(TRIG_RIGHT, ECHO_RIGHT);
	_delay_ms(40);
	
	// 2. 긴급 회피: 앞이 뚫려있어도 옆이 부딪힐 것 같으면 즉시 방향 틀기
	if (left_ultrasonic_distance < SIDE_CRASH_CM)
	{
		turn_right(TURN_SPEED); // 측면 회피는 딜레이를 짧게
		_delay_ms(350);
		return;
	}
	else if (right_ultrasonic_distance < SIDE_CRASH_CM)
	{
		turn_left(TURN_SPEED);
		_delay_ms(350);
		return;
	}

	// 앞 + 왼쪽 막힘 (<= 로 수정하여 정확히 10일 때의 버그 방지)
	if (left_ultrasonic_distance <= SIDE_SAFE_CM && right_ultrasonic_distance > SIDE_SAFE_CM)
	{
		turn_right(TURN_SPEED);
		_delay_ms(350);
		return;
	}

	// 앞 + 오른쪽 막힘
	if (right_ultrasonic_distance <= SIDE_SAFE_CM && left_ultrasonic_distance > SIDE_SAFE_CM)
	{
		turn_left(TURN_SPEED);
		_delay_ms(400);
		return;
	}

	// 앞 + 좌우 모두 막힘: 후진 후 넓은 쪽으로 회전
	if (left_ultrasonic_distance <= SIDE_SAFE_CM && right_ultrasonic_distance <= SIDE_SAFE_CM)
	{
		backward(AUTO_SPEED);
		_delay_ms(350);

		stop();
		_delay_ms(80);
		
		if (left_ultrasonic_distance > right_ultrasonic_distance)
		{
			turn_left(TURN_SPEED);
			_delay_ms(400);
			return;
		}
		else
		{
			turn_right(TURN_SPEED);
			_delay_ms(400);
			return;
		}
	}

	// 위 모든 조건에 해당하지 않으면 (좌우 모두 꽤 넓음), 더 넓은 쪽으로 회전
	if (left_ultrasonic_distance > right_ultrasonic_distance)
	{
		turn_left(TURN_SPEED);
		_delay_ms(250);
		return;
	}
	else
	{
		turn_right(TURN_SPEED);
		_delay_ms(250);
		return;
	}
}