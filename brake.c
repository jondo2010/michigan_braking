//
//	brake.c:
//	Brake light and switch related functions for the brake module.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#include <avr/io.h>

#include "can.h"
#include "brake.h"

int
sample_brake_switch (void)
{
	static int hyst_count = 0;
	static int pressed = 0;

	int	sample;

	sample = (PINF & _BV (PF0));
	hyst_count += sample ? -1 : 1;

	if (hyst_count >= BRAKE_SWITCH_ON_THRESHOLD)
	{
		pressed = 1;
		hyst_count = BRAKE_SWITCH_ON_THRESHOLD;
	}
	else if (hyst_count <= BRAKE_SWITCH_OFF_THRESHOLD)
	{
		pressed = 0;
		hyst_count = BRAKE_SWITCH_OFF_THRESHOLD;
	}

	return pressed;
}

void
turn_off_brake_light (void)
{
	PORTG |= _BV (PG3);
}

void
turn_on_brake_light (void)
{
	PORTG &= ~_BV (PG3);
}

void
announce_brake_pressed (void)
{
	uint8_t data[2] = {0x20, 0x01};

	can_load_data (3, data, 2);
	can_ready_to_send (3);
}

void
announce_brake_released (void)
{
	uint8_t data[2] = {0x20, 0x00};

	can_load_data (3, data, 2);
	can_ready_to_send (3);
}
