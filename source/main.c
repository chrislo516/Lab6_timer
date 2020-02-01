/*      Author: TSZHIN LO CHRIS
 *  Partner(s) Name: VIK
 *      Lab Section: 23
 *      Assignment: Lab 6  Exercise 2
 *      Exercise Description: [optional - include for your own benefit]
 *
 *      I acknowledge all content contained herein, excluding template or example
 *      code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. Cprogrammer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.

unsigned long _avr_timer_M = 1; // Start count from here, down to 0.Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
void TimerOn() {

// AVR timer/counter controller register TCCR1

	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;

// TimerISR will be called every _avr_timer_cntcurr milliseconds

//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}



void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}



void TimerISR() {
	TimerFlag = 1;
}


// In our approach, the C programmer does not touch this ISR, butrather TimerISR()

ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
  _avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
  if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
	TimerISR(); // Call the ISR that the user uses
	_avr_timer_cntcurr = _avr_timer_M;
  }
}



// Set TimerISR() to tick every M ms

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum LED_States {Init, One, Two, Three, wait, reset} LED_State;

void Loop(){
   switch(LED_State) {

	case Init:
	LED_State = One;
	break;

	case One:
	if((~PINA&0x01)==0x01){
	 LED_State = wait;
	 break;
	}else 
	 LED_State = Two;
	break;

	case Two:
	if((~PINA&0x01)==0x01){
	 LED_State = wait;
	 break;
	}else
	 LED_State = Three;
	break;

	case Three:
	if((~PINA&0x01)==0x01){
         LED_State = wait;
	 break;
	}else
	 LED_State = One;
	break;

	case wait:
	if((~PINA&0x01)==0x01){
	 LED_State = wait;
	 break; 
	}else
	   LED_State = reset;
	break;
	
	case reset:
	 if((~PINA&0x01)==0x01){
	   LED_State = One;
	   break;
	 }else
	   LED_State = reset;
	 break;

	default:
	LED_State = Init;
	break;

   }

   switch(LED_State) {

	case Init:
	PORTB = 0x00;
	break;

	case One:
	PORTB = 0x01;
	break;

	case Two:
	PORTB = 0x02;
	break;

	case Three:
	PORTB = 0x04;
	break;
	
	case wait:
        break;

        case reset:
        break;

	default:
	break;

   } 
}



int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; // Set port B to output
	PORTB = 0x00; // Init port B to 0s
	LED_State = Init;
	TimerSet(300);
	TimerOn();	
	while(1) {
	// User code (i.e. synchSM calls)
	Loop();
	while (!TimerFlag); // Wait 1 sec
	TimerFlag = 0;
	}
	return 1;
}
