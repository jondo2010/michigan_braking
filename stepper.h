//
//	stepper.h
//	Driver for the A3967 stepper motor driver IC.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#ifndef STEPPER_H_
#define STEPPER_H_

#include <inttypes.h>

#define STEPPER_ENABLE 		PB1		/* active low */
#define STEPPER_SLEEP		PB2		/* active low */
#define STEPPER_MS2 		PB3		/* should be fixed zero */
#define STEPPER_MS1			PB4		/* should be fixed zero */
#define STEPPER_DIR			PB5		/* 0 = forward, 1 = reverse */
#define STEPPER_RESET		PB6		/* active low */
#define STEPPER_STEP		PB7		/* steps on low-high transition */

#define	STEPPER_SLEEP_DELAY	10.0	/* 1 */
#define	STEPPER_STEP_DELAY	10.0	/* 2 */
#define STEPPER_DIR_DELAY	10.0	/* 3 */

//
//	1.	The datasheet calls for 1.0 ms of wake-up recovery time, so we add
//		a safety margin of two because the delay.h calls aren't perfect and
//		it's not like we are in a hurry.
//
//	2.	The datasheet calls for 1.0 us of delay between changing the step
//		signal, e.g., 1.0 us up, 1.0 us down. We a safety margin of two like
//		we did for the sleep delay.
//
//	3.	The datasheet calls for 200 ns of delay between changing the direction
//		signal. We use a safety margin of five, since we can't really delay by
//		nanoseconds.
//
//	N.B. These are all units of microseconds.
//

typedef enum stepper_dir_t
{
	dir_forward,
	dir_reverse
}
stepper_dir_t;

void
stepper_init (void);

void
stepper_step
(
	stepper_dir_t 	direction
);

#endif
