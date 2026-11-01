#include "fnd.h"

#define SEG_A  0x01
#define SEG_B  0x02
#define SEG_C  0x04
#define SEG_D  0x08
#define SEG_E  0x10
#define SEG_F  0x20
#define SEG_G  0x40
#define SEG_DP 0x80

#define FND_BLANK 0x00

static const unsigned char fnd_font[10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66,
	0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

volatile unsigned int fnd_ms_count = 0;
volatile unsigned int fnd_run_sec = 0;
volatile unsigned int fnd_backward_count = 0;
volatile unsigned int fnd_left_count = 0;
volatile unsigned int fnd_right_count = 0;

static volatile unsigned char top_data[4] =
{
	0xff, 0xff, 0xff, 0xff
};

static volatile unsigned char bottom_data[4] =
{
	0xff, 0xff, 0xff, 0xff
};

static volatile unsigned char scan_index = 0;

static motion_t current_motion = MOTION_STOP;
static motion_t last_count_motion = MOTION_STOP;

static unsigned char drive_finished = 0;
static unsigned char result_page = 0;
static unsigned int one_second_count = 0;

static void fnd_all_off(void);
static void fnd_digit_on(unsigned char scan);
static void fnd_set_4digit(volatile unsigned char data[4], unsigned int value);
static void fnd_set_bottom_label(unsigned char label);
static unsigned char motion_seg(motion_t motion);
static unsigned char circle_seg(unsigned char step);
static void fnd_display_running(void);
static void fnd_display_result(void);

ISR(TIMER2_OVF_vect)
{
	TCNT2 = 131;   // 약 0.5ms마다 다음 자리 스캔

	fnd_all_off();

	if (scan_index < 4)
	{
		FND_DATA_PORT = top_data[scan_index];
	}
	else
	{
		FND_DATA_PORT = bottom_data[scan_index - 4];
	}

	fnd_digit_on(scan_index);

	scan_index++;
	if (scan_index >= 8)
	{
		scan_index = 0;
	}

	fnd_ms_count++;

	// 0.5ms * 2000 = 약 1초
	if (!drive_finished && current_motion != MOTION_STOP)
	{
		one_second_count++;
		if (one_second_count >= 2000)
		{
			one_second_count = 0;
			fnd_run_sec++;
		}
	}
}

void init_fnd(void)
{
	MCUCSR |= 1 << JTD;
	MCUCSR |= 1 << JTD;
	
	FND_DATA_DDR = 0xff;

	FND_TOP_DDR |= 0xE0;       // PF5~PF7 출력
	FND_TOP1_DDR |= 0x01; // PE0 출력
	FND_BOTTOM_DDR |= 0xf0;    // PD4~PD7 출력

	fnd_all_off();

	TCCR2 = 0x00;
	TCCR2 |= (1 << CS22);      // 64분주
	TCNT2 = 131;
	TIMSK |= (1 << TOIE2);
}

void fnd_update(void)
{
	if (drive_finished)
	{
		fnd_display_result();
	}
	else
	{
		fnd_display_running();
	}
}

void fnd_set_motion(motion_t motion)
{
	current_motion = motion;

	if (motion != MOTION_STOP && motion != last_count_motion)
	{
		if (motion == MOTION_BACKWARD)
		{
			fnd_backward_count++;
		}
		else if (motion == MOTION_LEFT)
		{
			fnd_left_count++;
		}
		else if (motion == MOTION_RIGHT)
		{
			fnd_right_count++;
		}

		last_count_motion = motion;
	}

	if (motion == MOTION_STOP)
	{
		last_count_motion = MOTION_STOP;
	}
}

void fnd_finish_drive(void)
{
	drive_finished = 1;
	current_motion = MOTION_STOP;
	result_page = 0;
}

unsigned char fnd_next_result_page(void)
{
	if (result_page < 3)
	{
		result_page++;
		return 0;
	}

	return 1;
}

void fnd_reset_drive(void)
{
	drive_finished = 0;
	result_page = 0;

	fnd_run_sec = 0;
	fnd_backward_count = 0;
	fnd_left_count = 0;
	fnd_right_count = 0;
	one_second_count = 0;

	current_motion = MOTION_STOP;
	last_count_motion = MOTION_STOP;
}

unsigned char fnd_is_finished(void)
{
	return drive_finished;
}

static void fnd_all_off(void)
{
	FND_TOP_PORT &= ~0xE0;
	FND_TOP1_PORT &= ~0x01;
	FND_BOTTOM_PORT &= ~0xf0;
}

static void fnd_digit_on(unsigned char scan)
{
	if (scan < 3)
	{
		FND_TOP_PORT |= (1 << (FND_TOP_FIRST + scan));
	}
	else if (scan == 3)
	{
		FND_TOP1_PORT |= (1 << FND_TOP_PIN3 );
	}
	else
	{
		FND_BOTTOM_PORT |= (1 << (FND_BOTTOM_FIRST + (scan - 4)));
	}
}

static void fnd_set_4digit(volatile unsigned char data[4], unsigned int value)
{
	value %= 10000;

	data[0] = ~(fnd_font[value / 1000]);
	data[1] = ~(fnd_font[value / 100 % 10]);
	data[2] = ~(fnd_font[value / 10 % 10]);
	data[3] = ~(fnd_font[value % 10]);
}

static void fnd_set_bottom_label(unsigned char label)
{
	bottom_data[0] = ~(label);
	bottom_data[1] = ~(FND_BLANK);
	bottom_data[2] = ~(FND_BLANK);
	bottom_data[3] = ~(FND_BLANK);
}

static unsigned char motion_seg(motion_t motion)
{
	if (motion == MOTION_FORWARD)
	{
		return ~(SEG_A | SEG_E | SEG_F | SEG_G);  // F
	}
	else if (motion == MOTION_BACKWARD)
	{
		return ~(SEG_C | SEG_D | SEG_E | SEG_F | SEG_G);  // b
	}
	else if (motion == MOTION_LEFT)
	{
		return ~(SEG_D | SEG_E | SEG_F);  // L
	}
	else if (motion == MOTION_RIGHT)
	{
		return ~(SEG_E | SEG_G);  // r
	}

	return ~(SEG_A | SEG_C | SEG_D | SEG_F | SEG_G);  // S
}

static unsigned char circle_seg(unsigned char step)
{
	static const unsigned char circle[6] =
	{
		SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F
	};

	return ~(circle[step % 6]);
}


static void fnd_display_running(void)
{
	unsigned char circle_step = (fnd_ms_count / 120) % 6;

	// 위 4자리: 주행시간
	fnd_set_4digit(top_data, fnd_run_sec);

	// 아래 4자리: 방향 문자 + 바로 옆 서클 애니메이션
	bottom_data[0] = motion_seg(current_motion);
	bottom_data[1] = circle_seg(circle_step);
	bottom_data[2] = circle_seg(circle_step);
	bottom_data[3] = circle_seg(circle_step);
}

static void fnd_display_result(void)
{
	unsigned int value = 0;
	unsigned char label = 0;
	unsigned char sreg;

	if (result_page == 0)
	{
		label = SEG_D | SEG_E | SEG_F | SEG_G;  // t, 주행시간
		value = fnd_run_sec;
	}
	else if (result_page == 1)
	{
		label = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;  // b, 후진 횟수
		value = fnd_backward_count;
	}
	else if (result_page == 2)
	{
		label = SEG_D | SEG_E | SEG_F;  // L, 좌회전 횟수
		value = fnd_left_count;
	}
	else
	{
		label = SEG_E | SEG_G;  // r, 우회전 횟수
		value = fnd_right_count;
	}
	
	// 종료 후: 위 4자리는 값, 아래 첫 자리는 항목 문자
	fnd_set_4digit(top_data, value);
	fnd_set_bottom_label(label);
}
