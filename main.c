//
//	main.c:
//	Braking module main control code.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>

#include "can.h"

#include "bias.h"
#include "brake.h"
#include "kill.h"
#include "stepper.h"

typedef enum state_t
{
	ms_idle,
	ms_adjusting_bias,
	ms_calibrating_bias
}
state_t;

int 				brake_pressed;	/* is the brake pedal pressed? */
static int 			brake_active;	/* set when pressed, cleared when released */

int 				kill_engaged;	/* is the kill circuit engaged? */
static int 			kill_active;	/* set when engaged, cleared when disengaged */

static state_t 		state;			/* current state of module */

static mob_config_t adjust_mob;		/* adjustment requests */
static mob_config_t calibrate_mob;	/* calibration requests */
static mob_config_t bias_mob;		/* bias position requests */
static mob_config_t status_mob;		/* status announcements */

extern volatile int msg_adj_recv;	/* adjustment message pending */
extern volatile int msg_cal_recv;	/* calibration message pending */

//
//	Configure input and output GPIO pins.
//
//	Port 	Pin		Direction	Function
//	====	===		=========	================================
//	D		1		Input		Left end-of-travel switch
//	D		2		Input		Right end-of-travel switch
//	F		0		Input		Brake pedal switch
//	F		1		Input		Kill circuit
//	G		3		Output		Brake light logic signal

void
io_init (void)
{
	PORTD |= _BV (PD1) | _BV (PD2); /* 1 */
	PORTF |= _BV (PF0) | _BV (PF1);	/* 1 */

	DDRG |= _BV (PG3);
	PORTG |= _BV (PG3);
}

//
//	1.	Use weak internal pull-up resistors.
//

//
//	Initialize the CAN message objects.
//

void
mob_init (void)
{
	adjust_mob.id = 0x200;
	adjust_mob.id_type = id_standard;
	adjust_mob.mask = 0x7ff;
	adjust_mob.rx_callback_ptr = adjust_rx_callback;
	adjust_mob.tx_callback_ptr = 0;

	can_config_mob (0, &adjust_mob);
	can_ready_to_receive (0);

	calibrate_mob.id = 0x210;
	calibrate_mob.id_type = id_standard;
	calibrate_mob.mask = 0x7ff;
	calibrate_mob.rx_callback_ptr = calibrate_rx_callback;
	calibrate_mob.tx_callback_ptr = 0;

	can_config_mob (1, &calibrate_mob);
	can_ready_to_receive (1);

	bias_mob.id = 0x220;
	bias_mob.id_type = id_standard;
	bias_mob.mask = 0x7ff;
	bias_mob.rx_callback_ptr = 0;
	bias_mob.tx_callback_ptr = bias_tx_callback;

	can_config_mob (2, &bias_mob);

	status_mob.id = 0x230;
	status_mob.id_type = id_standard;
	status_mob.mask = 0x7ff;
	status_mob.rx_callback_ptr = 0;
	status_mob.tx_callback_ptr = 0;

	can_config_mob (3, &status_mob);
}

int
main (void)
{
	io_init ();
	stepper_init ();

	can_init (can_baud_1000);
	mob_init ();

	bias_init ();

	sei ();

	while (1)
	{
		brake_pressed = sample_brake_switch ();

		if (brake_pressed)
		{
			turn_on_brake_light ();

			if (!brake_active)
			{
				announce_brake_pressed ();
				brake_active = 1;
			}
		}
		else
		{
			turn_off_brake_light ();

			if (brake_active)
			{
				announce_brake_released ();
				brake_active = 0;
			}
		}

		kill_engaged = sample_kill_circuit ();

		if (kill_engaged && !kill_active)
		{
			announce_kill_engaged ();
			kill_active = 1;
		}
		else if (!kill_engaged && kill_active)
		{
			announce_kill_disengaged ();
			kill_active = 0;
		}

		switch (state)
		{
			int finished;

			case ms_idle:

				if 		(msg_adj_recv)	state = ms_adjusting_bias;
				else if (msg_cal_recv) 	state = ms_calibrating_bias;

				break;

			case ms_adjusting_bias:

				finished = adjust_bias_fsm ();

				if (finished)
					state = ms_idle;

				break;

			case ms_calibrating_bias:

				finished = calibrate_bias_fsm ();

				if (finished)
					state = ms_idle;

				break;
		}
	}

	return 0;
}
