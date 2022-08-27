
/*
* f_interrupt.h
*
* Created: 28.08.2018 21:36:28
* Author: Maximilian Starke
FPS:
		all atomic classes are ready.
		please test them. please test memory consumption of different atomic solutions
		please check descriptions
		write the cimota classes !!!
*/


#ifndef F_INTERRUPT_H_
#define F_INTERRUPT_H_

#include <avr/interrupt.h>

namespace fsl {
	
	namespace hw {

		
		/* A standard_atomic is a more advanced variant of a simple_atomic. You may see the description of simple_atomic to get familiar with the concept.
		A standard_atomic can on the one hand disable interrupts and save the MCUs interrupt configuration register SREG in a copy
		and on the other hand restore the MCUs interrupt configuration register from the copy saved.
		A standard_atomic (in difference to simple_atomic) may block interrupts and unblock them again as often as desired.
		*/
		class standard_atomic {
			
			bool _active;
			uint8_t sreg;
			
			/* copy interrupt register and deactivate interrupts */
			inline void store(){ sreg = SREG; cli(); }
			/* restore the register copy to register iff active() */
			inline void restore(){ if (_active) SREG = sreg; }
			
			public:
			
			/* create standard_atomic and immediately go into critical section / go into active state */
			inline standard_atomic() : _active(true), sreg(SREG) { cli(); }
			/* create standard_atomic, go into critical section / active state iff active. */
			inline standard_atomic(bool active) : _active(active) { if (active) store(); }
			/* leave critical section / active state if in this state */
			inline ~standard_atomic() { restore(); }
			
			standard_atomic(const standard_atomic&) = delete;
			standard_atomic& operator = (const standard_atomic&) = delete;
			
			/* take state of obj and let obj become an inactive standard_atomic */
			inline standard_atomic(standard_atomic&& obj) : _active(obj._active), sreg(obj.sreg) { obj._active = false; }
			/* leave the own critical section if necessary, i.e. if active, then take state of obj and let obj become an inactive standard_atomic */
			inline standard_atomic& operator = (standard_atomic&& obj) { restore(); _active = obj._active; sreg = obj.sreg; obj._active = false; return *this; }
			
			/* go into critical section (iff not already in) */
			inline void open(){ if (!_active) { store(); _active = true; } }
			/* go into critical section (iff not already in) */
			inline void p(){ open(); }
			/* leave critical section (iff not already inactive) */
			inline void close(){ restore(); _active = false; }
			/* leave critical section (iff not already inactive) */
			inline void v(){ close(); }
			
			/* returns true iff standard_atomic is in its critical section */
			inline bool active(){ return _active; }
		};

		/* If you need an atomic section, i.e. a snipped of code that is guaranteed to not be preempted from any interrupt, you may use an simple_atomic object.
		The only things you can do with an simple_atomic is to create it and let it become destroyed.
		On creation the simple_atomic turns off interrupts and saves the MCUs configuration before turning off.
		On destruction the simple_atomic restores the MCUs configuration about interrupts, i.e. re-enables the interrupts if it has really disabled them on creation.
		*/
		class simple_atomic {
			
			uint8_t sreg;
			
			public:
			
			/* create a simple_atomic, copy the global interrupt enable flag and clear the flag by using copy of whole SREG */
			inline simple_atomic() : sreg(SREG) { cli(); }
			/* restore the saved SREG copy to restore the global interrupt enable of before */
			inline ~simple_atomic() { SREG = sreg; }
			
			simple_atomic(const simple_atomic&) = delete;
			simple_atomic& operator = (const simple_atomic&) = delete;
			
			inline simple_atomic(simple_atomic&& obj) = delete;
			inline simple_atomic& operator = (simple_atomic&& obj) = delete;
			
		};
		
		/* deprecated since it has size 1 byte, but you could also just write cli() and sei() */
		class stupid_atomic {
			// << put it into an extra header and add a:
			//#warning Using this class is deprecated
			// which appears as soon as you include this header.
			public:
			inline stupid_atomic() { cli(); }
			inline ~stupid_atomic() { sei(); }
		};

		/* anti-atomic */
		// << variant to turn on interrupts for some section inside a section where interrupts are disabled (perhaps by an atomic.)
		class lite_cimota {
			// standard with one_byte solution;
		};
		
		class standard_cimota {
			
		};
		
		class simple_cimota {
			
		};
		
	}
}


#endif /* F_INTERRUPT_H_ */