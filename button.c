/*
 * button.c
 *
 * Created: 2026-06-10 오후 1:25:20
 *  Author: user
 */ 

#include "button.h"

void init_button(void);
int get_button(int button_num, int button_pin);

// 버튼 초기화 방향설정(입력) 
void init_button(void)
{
	BUTTON_DDR &= ~(1 << BUTTON0PIN);
	// BUTTON_DDR &= 0xf0;
	// BUTTON_DDR &= 0x0f
}

// 예) BUTTON0 3
//    button을 눌렀다 떼면 : 1을 리턴
//          이 idle 상태 : 0을 리턴 

int get_button(int button_num, int button_pin)
{
	static unsigned char button_status[BUTTON_NUMBER] =
	{
		BUTTON_RELEASE,BUTTON_RELEASE,BUTTON_RELEASE,BUTTON_RELEASE
	};
	// static 지역변수에 static을 선언하면 함수를 빠져나와 다시 들어가도 이전값유지 
	int current_state;
	
	// 1. button을 읽는다. 
	current_state = BUTTON_PIN & (1 << button_pin);
	// 2. 버튼 상태 check 
	if (current_state  && button_status[button_num] == BUTTON_RELEASE)  // 버튼이 처음 눌려진 상태 
	{
		_delay_ms(15);  // noise가 지나가기를 기다린다. 
		button_status[button_num] =BUTTON_PRESS;  
		return 0;     // 아직은 완전히 눌렀다 뗀 상태가 아니다. 
	}
	else if (button_status[button_num] == BUTTON_PRESS && current_state == BUTTON_RELEASE)
	{  // 버튼이 이전에 눌려진 상태였으며 지금은 떼어진 상태 
		_delay_ms(15);  // noise가 지나가기를 기다린다. 
		button_status[button_num] = BUTTON_RELEASE;   // 다음 버튼을 체크 하기 위하여 초기화

		return 1;   // 완전히 1번 눌렀다 뗀 상태로 인정 한다. 
	}
	
	return 0;   // 버튼이 open 상태 이다. 
}
