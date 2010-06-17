//
//	kill.c:
//	Kill circuit related functions for the brake module.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#include <avr/io.h>
#include <util/delay.h>

#include "can.h"
#include "kill.h"

int
sample_kill_circuit (void)
{
	static int hyst_count = 0;
	static int engaged = 0;

	int	sample;

	sample = (PINF & _BV (PF1));
	hyst_count += sample ? -1 : 1;

	if (hyst_count >= KILL_CIRCUIT_ON_THRESHOLD)
	{
		engaged = 1;
		hyst_count = KILL_CIRCUIT_ON_THRESHOLD;
	}
	else if (hyst_count <= KILL_CIRCUIT_OFF_THRESHOLD)
	{
		engaged = 0;
		hyst_count = KILL_CIRCUIT_OFF_THRESHOLD;
	}

	return engaged;
}

void
announce_kill_engaged (void)
{
	uint8_t data[2] = {0x10, 0x01};

	can_load_data (3, data, 2);
	can_ready_to_send (3);
}

void
announce_kill_disengaged (void)
{
	uint8_t data[2] = {0x10, 0x00};

	can_load_data (3, data, 2);
	can_ready_to_send (3);
}
