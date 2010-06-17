//
//	bias.h:
//	Bias adjustment and calibration functions for the braking module.
//
//	Michael Jean <michael.jean@shaw.ca>
//

#ifndef _BIAS_H
#define _BIAS_H

#define	MAX_BIAS 		70		/* front-to-rear distribution at front-max eot (percent) */
#define MIN_BIAS		30		/* front-to-rear distribution at front-min eot (percent) */

#define STEP_DELAY		50.0	/* delay between step adjustments (ms) */
#define REVERSE_DELAY	1000.0	/* delay between reversing calibration direction (ms) */

typedef enum adj_state_t
{
	as_start,
	as_adjusting,
	as_finished
}
adj_state_t;

typedef enum cal_state_t
{
	cs_start,
	cs_finding_left_eot,
	cs_finding_right_eot,
	cs_rebiasing,
	cs_finished
}
cal_state_t;

//
//	Called when an adjustment request is received over the network.
//

void
adjust_rx_callback (uint8_t mob_index, uint32_t id, packet_type_t type);

//
//	Called after a current bias request is replied to.
//

void
bias_tx_callback (uint8_t mob_index);

//
//	Called when a callibration request is received over the network.
//

void
calibrate_rx_callback (uint8_t mob_index, uint32_t id, packet_type_t type);

//
//	Arm the bias remote reply message object with the correct value.
//

void
arm_bias_reply (void);

//
//	Initialize the bias subsystem.
//

void
bias_init (void);

//
//	Execute another pass of the bias adjustment finite-state-machine.
//	Returns 1 if the routine is finished, or 0 if not.
//

int
adjust_bias_fsm (void);

//
//	Execute another pass of the bias calibration finite-state-machine.
//	Returns 1 if the routine is finished, or 0 if not.
//

int
calibrate_bias_fsm (void);

//
//	Sample the left end-of-travel switch. Do not implement hysteresis.
//	Return 1 if the switch is closed, or 0 if it is open.
//

int
sample_left_eot_switch (void);

//
//	Sample the right end-of-travel switch. Do not implement hysteresis.
//	Return 1 if the switch is closed, or 0 if it is open.
//

int
sample_right_eot_switch (void);

//
//	Announce that the adjustment procedure was successful over the network.
//

void
announce_adjustment_success (void);

//
//	Announce that the calibration procedure was successful over the network.
//

void
announce_calibration_success (void);

#endif
