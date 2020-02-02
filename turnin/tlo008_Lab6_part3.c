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
enum counts {startState,init,waitPress,inc,dec,reset} count;
unsigned char op = 0x00;

void tick(){
 switch(count){ 			//Transitions
	case startState:		//Initial transition
	    count = init;
	    PORTB = 0x07;
	break;
	
	case init:
	  if(op==0x03){
	    count = reset; break;
	  }else if(op==0x01){	//PA0 button pressed
	    count = inc; break;
	  }else if(op==0x02){ //PA0 button pressed
	    count = dec; break;
	  }else{
	    count = init; break;
	  }

	case inc:
            count = waitPress;
	    TimerSet(1000);
	    TimerOn();
	break;

	case dec:
            count = waitPress;
	    TimerSet(1000);
            TimerOn();
	break;

	case waitPress:
          if(op==0x03){
	    count = reset; break;
	  }else if(op==0x01){
	    count = inc; break;
	  }else if(op==0x02){   
            count = dec; break;
	  }else{
            count = init; break;
          }	
	
	case reset:
	if((op==0x01)||(op==0x02)){      //button pressed
            count = reset; break;
          }else{
            count = init; break;
          }

	default:
	break;
 }
 switch(count){
	case startState:
	break;	

	case init:
	break;

	case inc:
	 if(PORTB<0x09){
	   PORTB = PORTB + 0x01; break;
	 }else{
	   PORTB = 0x09; break;
	 }

	case dec:
         if(PORTB>0x00){
           PORTB = PORTB - 0x01; break;
         }else{
           PORTB = 0x00; break;
         }
	
	case waitPress:
	break;

	case reset:
	   PORTB = 0x00; break;

	default:
	break;
 }
}


int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; // Set port B to output
	PORTB = 0x00; // Init port B to 0s
	count = startState;
	while(1) {
	// User code (i.e. synchSM calls)
	TimerSet(100);
        TimerOn();
	op = ~PINA&0x03;
	tick();
	while (!TimerFlag); // Wait 1 sec
	TimerFlag = 0;
// Note: For the above a better style would use a synchSM with TickSM()
// This example just illustrates the use of the ISR and flag
	}
}
