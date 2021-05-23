/*
 * timer0.cpp
 *
 * Created: 02.08.2020 13:54:32
 *  Author: F-NET-ADMIN
 */ 

#include "timer0.h"

procedure TCNT0_COMPARE_A = nullptr;

ISR(TIMER0_COMPA_vect){
	if (TCNT0_COMPARE_A) TCNT0_COMPARE_A();
}

procedure TCNT1_COMPARE_A = nullptr;

ISR(TIMER1_COMPA_vect){
	if (TCNT1_COMPARE_A) TCNT1_COMPARE_A();
}



