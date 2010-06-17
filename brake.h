//
//	brake.h:
//	Brake light and switch related functions for the brake module.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#ifndef _BRAKE_H
#define _BRAKE_H

#define BRAKE_SWITCH_ON_THRESHOLD 	2
#define	BRAKE_SWITCH_OFF_THRESHOLD	-2

//
//	Sample the brake switch. Implement a hysteresis debouncing feature.
//	Return 1 if the switch is held, or 0 if it is released.
//

int
sample_brake_switch (void);

//
//	Turn off the brake light. Pull the brake light module logic signal low.
//

void
turn_off_brake_light (void);

//
//	Turn on the brake light. Pull the brake light module logic signal high.
//

void
turn_on_brake_light (void);

//
//	Announce the brake pedal has been pressed over the network.
//

void
announce_brake_pressed (void);

//
//	Announce the brake pedal has been released over the network.
//

void
announce_brake_released (void);

#endif
