//
//	kill.h:
//	Kill circuit related functions for the brake module.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#ifndef _KILL_H
#define _KILL_H

#define KILL_CIRCUIT_ON_THRESHOLD 	2
#define	KILL_CIRCUIT_OFF_THRESHOLD	-2

//
//	Sample the kill circuit. Implement a hysteresis debouncing feature.
//	Return 1 if the circuit is engaged, or 0 if it is disengaged.
//

int
sample_kill_circuit (void);

//
//	Announce the kill circuit has been engaged over the network.
//

void
announce_kill_engaged (void);

//
//	Announce the kill circuit has been disengaged over the network.
//

void
announce_kill_disengaged (void);

#endif
