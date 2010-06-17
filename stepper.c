//
//	stepper.c
//
//	Michael Jean <michael.jean@shaw.ca>
//

#include <avr/io.h>
#include <util/delay.h>

#include "stepper.h"

void
stepper_init (void)
{
	DDRB |= _BV (STEPPER_ENABLE) | _BV (STEPPER_SLEEP) | _BV (STEPPER_MS2) |
			_BV (STEPPER_MS1) | _BV (STEPPER_MS1) | _BV (STEPPER_DIR) |
			_BV (STEPPER_RESET)	| _BV (STEPPER_STEP);

	PORTB = _BV (STEPPER_RESET) | _BV (STEPPER_SLEEP);
	_delay_ms (STEPPER_SLEEP_DELAY);
}

void
stepper_step (stepper_dir_t direction)
{
	if (direction == dir_forward)
	{
		PORTB &= ~_BV (STEPPER_DIR);
	}
	else if (direction == dir_reverse)
	{
		PORTB |= _BV (STEPPER_DIR);
	}
	_delay_us (STEPPER_DIR_DELAY);

	PORTB |= _BV (STEPPER_STEP);
	_delay_us (STEPPER_STEP_DELAY);

	PORTB &= ~_BV (STEPPER_STEP);
	_delay_us (STEPPER_STEP_DELAY);
}
