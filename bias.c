//
//	bias.c:
//	Bias adjustment and calibration functions for the braking module.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "can.h"

#include "bias.h"
#include "stepper.h"

static uint16_t		step_range; 				/* number of steps to travel between EOT switches */
static uint16_t 	EEMEM ee_step_range = 100;	/* eeprom backup of stepper range */

static uint16_t 	step_pos;					/* actual stepper position */
static uint16_t		EEMEM ee_step_pos = 0;		/* eeprom backup of stepper position */

static uint8_t		bias_actual;						/* actual front bias */
static uint8_t		EEMEM ee_bias_actual = MIN_BIAS;	/* eeprom backup of front bias */

static uint16_t 	step_pos_req;		/* currently requested stepper position */
volatile uint8_t 	bias_req;			/* currently requested front bias */

volatile int 		msg_adj_recv;		/* adjustment request received */
volatile int 		msg_cal_recv;		/* calibration requested received */

static adj_state_t 	adj_state;			/* current adjustment state */
static cal_state_t 	cal_state;			/* current calibration state */

extern int 			brake_pressed;		/* is brake pedal pressed? */
extern int 			kill_engaged;		/* is kill circuit engaged? */

void
adjust_rx_callback (uint8_t mob_index, uint32_t id, packet_type_t type)
{
	if (!msg_adj_recv)
	{
		msg_adj_recv = 1;
		can_read_data (mob_index, (uint8_t *)&bias_req, 1);
	}

	can_ready_to_receive (mob_index);
}

void
bias_tx_callback (uint8_t mob_index)
{
	arm_bias_reply ();
}

void
calibrate_rx_callback (uint8_t mob_index, uint32_t id, packet_type_t type)
{
	msg_cal_recv = 1;
	can_ready_to_receive (mob_index);
}

void
arm_bias_reply (void)
{
	can_load_data (2, &bias_actual, 1);
	can_reply_valid (2);
}

void
bias_init (void)
{
	step_pos = eeprom_read_word (&ee_step_pos);
	step_range = eeprom_read_word (&ee_step_range);
	bias_actual = eeprom_read_byte (&ee_bias_actual);

	arm_bias_reply ();
}

int
adjust_bias_fsm (void)
{
	int 	finished = 0;
	int		left_eot_pushed = 0;
	int		right_eot_pushed = 0;
	double	travel_pct = 0;

	switch (adj_state)
	{
		case as_start:

			if 		(bias_req > MAX_BIAS) 	bias_req = MAX_BIAS;
			else if (bias_req < MIN_BIAS) 	bias_req = MIN_BIAS;

			travel_pct = (double)(bias_req - MIN_BIAS) / (MAX_BIAS - MIN_BIAS);
			step_pos_req = travel_pct * step_range;

			adj_state = as_adjusting;
			break;

		case as_adjusting:

			left_eot_pushed = sample_left_eot_switch ();
			right_eot_pushed = sample_right_eot_switch ();

			if (!brake_pressed && !kill_engaged)
			{
				if (step_pos < step_pos_req)
				{
					stepper_step (dir_forward);
					_delay_ms (STEP_DELAY);

					if ((++step_pos >= step_pos_req) || right_eot_pushed)
						adj_state = as_finished;
				}
				else if (step_pos > step_pos_req)
				{
					stepper_step (dir_reverse);
					_delay_ms (STEP_DELAY);

					if ((--step_pos <= step_pos_req) || left_eot_pushed)
						adj_state = as_finished;
				}
				else
				{
					adj_state = as_finished;
				}
			}

			break;

		case as_finished:

			bias_actual = bias_req;
			arm_bias_reply ();

			eeprom_write_word (&ee_step_pos, step_pos);
			eeprom_write_byte (&ee_bias_actual, bias_actual);

			announce_adjustment_success ();

			finished = 1;
			msg_adj_recv = 0;

			adj_state = as_start;
			break;
	}

	return finished;
}

int
calibrate_bias_fsm (void)
{
	int 	finished = 0;
	int		left_eot_pushed = 0;
	int		right_eot_pushed = 0;
	double 	travel_pct = 0;

	switch (cal_state)
	{
		case cs_start:

			step_range = 0;

			cal_state = cs_finding_left_eot;
			break;

		case cs_finding_left_eot:

			left_eot_pushed = sample_left_eot_switch ();

			if (!left_eot_pushed && !brake_pressed && !kill_engaged)
			{
				stepper_step (dir_reverse);
				_delay_ms (STEP_DELAY);
			}
			else if (left_eot_pushed)
			{
				step_pos = 0;
				_delay_ms (REVERSE_DELAY);

				cal_state = cs_finding_right_eot;
			}

			break;

		case cs_finding_right_eot:

			right_eot_pushed = sample_right_eot_switch ();

			if (!right_eot_pushed && !brake_pressed && !kill_engaged)
			{
				stepper_step (dir_forward);
				_delay_ms (STEP_DELAY);

				step_pos++;
				step_range++;
			}
			else if (right_eot_pushed)
			{
				travel_pct = (double)(bias_actual - MIN_BIAS) / (MAX_BIAS - MIN_BIAS);
				step_pos_req = travel_pct * step_range;

				_delay_ms (REVERSE_DELAY);

				cal_state = cs_rebiasing;
			}

			break;

		case cs_rebiasing:

			if ((step_pos > step_pos_req) && !brake_pressed && !kill_engaged)
			{
				stepper_step (dir_reverse);
				_delay_ms (STEP_DELAY);

				step_pos--;
			}
			else if (step_pos == step_pos_req)
			{
				cal_state = cs_finished;
			}

			break;

		case cs_finished:

			eeprom_write_word (&ee_step_pos, step_pos);
			eeprom_write_word (&ee_step_range, step_range);

			announce_calibration_success ();

			finished = 1;
			msg_cal_recv = 0;

			cal_state = cs_start;
			break;
	}

	return finished;
}

int
sample_left_eot_switch (void)
{
	int pressed;

	pressed = (PIND & _BV (PD1)) ? 1 : 0;
	return pressed;
}

int
sample_right_eot_switch (void)
{
	int pressed;

	pressed = (PIND & _BV (PD2)) ? 1 : 0;
	return pressed;
}

void
announce_adjustment_success (void)
{
	uint8_t data[2] = {0x30, bias_actual};

	can_load_data (3, data, 2);
	can_ready_to_send (3);
}

void
announce_calibration_success (void)
{
	uint8_t data = 0x40;

	can_load_data (3, &data, 1);
	can_ready_to_send (3);
}
