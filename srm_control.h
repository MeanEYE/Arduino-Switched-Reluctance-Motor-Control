#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Arduino.h"
#include "stdbool.h"

enum SequenceType {
	SEQUENCE_SIMPLE,
	SEQUENCE_OVERLAP
};

// universal configuration structure
typedef struct {
	// pins to set
	char pin1;
	char pin2;
	char pin3;

	// programmed phase
	unsigned char phase1;
	unsigned char phase2;
	unsigned char phase3;
	unsigned char limit;
	unsigned char steps;

	// current positing in chase
	unsigned char position;
	bool running;

	// frequency control
	bool speed_control;
	unsigned long time_step;
	unsigned long next_update;  // timestamp
} Sequence;

// export functions
Sequence *motor_initialize(char pin1, char pin2, char pin3, enum SequenceType type);
void motor_free(Sequence *sequence);
void motor_set_speed_control(Sequence *sequence, bool status);
void motor_set_speed(Sequence *sequence, unsigned int speed);
bool motor_step(Sequence *sequence);
bool motor_step_backward(Sequence *sequence);
void motor_apply(Sequence *sequence);
void motor_release(Sequence *sequence);
void motor_resume(Sequence *sequence);

#ifdef __cplusplus
}
#endif
