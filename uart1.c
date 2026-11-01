/*
 * uart1.c
 *
 * Created: 2026-06-22 오전 10:38:34
 *  Author: user
 */ 

#include "uart1.h"

extern void UART0_transmit(uint8_t data);

volatile uint8_t bt_data;

void init_uart1(void);
void UART1_transmit(uint8_t data);

// P278 표12-3
// BT로부터 1byte가 들어 오면 자동적으로 이곳으로 진입 한다.
ISR(USART1_RX_vect)
{

	bt_data = UDR1;  // UDR1의 내용이 data에 복사된후 UDR1의 내용은 빈 상태로 된다.
	UART1_transmit(bt_data);   // BT로 부터 들어온 byte를 확인 하기 위해서 comport로 출력 한다. 

}
/*
1. 전송속도 : 9600bps
2. start/stop 설정
3. RX(수신) : interrupt 로 설정
*/
void init_uart1(void)
{
	// 1. 전송속도 : 9600bps
	UBRR1H = 0x00;
	UBRR1L = 207;   //9600BPS 표8-9
	UCSR1A |= 1 << U2X1;  // 2배속 설정 (sampling 8)
	// UART0 를 송신.수신이 다 가능하고 RX INT가 가능 하도록 설정 한다.
	UCSR1B |= 1 << RXEN1 | 1 << TXEN1 | 1 << RXCIE1;
}

// UART1 로 1byte를 전송 하는 함수
void UART1_transmit(uint8_t data)
{
	while ( !(UCSR1A & 1 << UDRE1))   // data가 송신중이면 송신이 끝날때 까지 기다림
		;     // No operation
	UDR1 = data;   // HW 전송 register에 data를 송신 한다.
}

