/**
 * Switched Reluctance Motor Control
 * Copyright © 2017 Mladen Mijatov. All Rights Reserved.
 *
 * This simple library allows control of switch reluctance motors such
 * as those found in hard disks. It provides varying degrees of precision
 * assuming three phases.
 *
 * Library also supports rudimentary speed control which allows programmers
 * to control motor speed without resorting to usage of `delay`. This is done
 * through setting speed measured in revolutions per minute and calling
 * `motor_step` or `motor_step_backward` frequently. State of pins will be
 * updated only after specific time has passed. Speed control does take into
 * account sequence sizes, so no additional calculation is needed.
 *
 *
 * Please note that:
 *
 * - You should *not* connect motor contacts directly to your micro-controller
 *   as they use much higher current than what your controller can provide and
 *   will most likely result in a damaged hardware. Instead you should come up
 *   with indirect solution such as one with transistors or optocoupler;
 *
 * - You can have multiple different sequences using the same pins. However be
 *   careful when switching between the two while motor is running as you might
 *   cause motor to stall. Ideally you should synchronize when `position == 1`;
 *
 * - Avoid setting `position` manually. It's not a typical counter. Library is
 *   stepping through sequence by shifting a single bit to the left or right;
 *
 * - Calling `motor_release` will stop applying any voltage to configured pins
 *   but phase will continue to cycle. Calling `motor_resume` again will
 *   resume applying voltage to pins. This is useful if you wish to spin up
 *   the motor and let it run on its own for a while.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#include "srm_control.h"


/**
 * Configure pins and create configuration structure.
 */
Sequence *motor_initialize(char pin1, char pin2, char pin3, enum SequenceType type)
{
	Sequence *result = calloc(1, sizeof(Sequence));

	// store sequence data and size
	switch (type) {
		case SEQUENCE_OVERLAP:
			result->phase1 = 0b00110001;
			result->phase2 = 0b00011100;
			result->phase3 = 0b00000111;
			result->steps = 6;
			break;

		case SEQUENCE_SIMPLE:
		default:
			result->phase1 = 0b00000100;
			result->phase2 = 0b00000010;
			result->phase3 = 0b00000001;
			result->steps = 3;
			break;
	}

	// configure pins
	result->pin1 = pin1;
	result->pin2 = pin2;
	result->pin3 = pin3;

	// set initial position
	result->limit = 1 << result->steps;
	result->position = 1;  // we will use bits in char to access phase state
	result->running = true;
	result->speed_control = false;
	motor_set_speed(result, 500);

	// initialize micro-controller pins
	pinMode(pin1, OUTPUT);
	pinMode(pin2, OUTPUT);
	pinMode(pin2, OUTPUT);

	return result;
}

/**
 * Release memory taken by the configuration.
 */
void motor_free(Sequence *sequence)
{
	free(sequence);
}

/**
 * Turn on or off speed control of the motor.
 */
void motor_set_speed_control(Sequence *sequence, bool status)
{
	sequence->speed_control = status;
}

/**
 * Set desired number of revolutions per minute of the motor.
 */
void motor_set_speed(Sequence *sequence, unsigned int speed)
{
	sequence->time_step = (60000000 / speed) / sequence->steps;
	sequence->next_update = micros() + sequence->time_step;
}

/**
 * Check if sequence cycle should be updated.
 */
bool motor_should_update(Sequence *sequence)
{
	bool result;

	// check if we should update cycle
	if (sequence->speed_control) {
		result = micros() >= sequence->next_update;

		if (result)
			sequence->next_update += sequence->time_step;

	} else {
		// speed control is off
		result = true;
	}

	return result;
}

/**
 * Make a single forward step. Precision (angle of rotation) depends on
 * sequence type.
 */
bool motor_step(Sequence *sequence)
{
	// check if cycle can be updated
	if (!motor_should_update(sequence))
		return false;

	// change position
	sequence->position <<= 1;
	if (sequence->position == sequence->limit)
		sequence->position = 1;

	// set pin states
	motor_apply(sequence);

	return true;
}

/** * Make a single backward step. Precision (angle of rotation) depends on
 * sequence type.
 */
bool motor_step_backward(Sequence *sequence)
{
	// check if cycle can be updated
	if (!motor_should_update(sequence))
		return false;

	// change position
	sequence->position >>= 1;
	if (sequence->position < 1)
		sequence->position = sequence->limit >> 1;

	// set pin states
	motor_apply(sequence);

	return true;
}

/**
 * Apply voltage to configured pins.
 */
void motor_apply(Sequence *sequence)
{
	if (!sequence->running)
		return;

	digitalWrite(sequence->pin1, sequence->phase1 & sequence->position ? HIGH : LOW);
	digitalWrite(sequence->pin2, sequence->phase2 & sequence->position ? HIGH : LOW);
	digitalWrite(sequence->pin3, sequence->phase3 & sequence->position ? HIGH : LOW);
}

/**
 * This function will stop applying voltage to select pins and motor
 * will be in free rotation.
 */
void motor_release(Sequence *sequence)
{
	sequence->running = false;
	digitalWrite(sequence->pin1, LOW);
	digitalWrite(sequence->pin2, LOW);
	digitalWrite(sequence->pin3, LOW);
}

/**
 * Resume motor rotation.
 */
void motor_resume(Sequence *sequence)
{
	sequence->running = true;
}
