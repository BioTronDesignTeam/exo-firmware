#ifndef AI_INFERENCE_H
#define AI_INFERENCE_H

#include <stdint.h>

struct PolicyInputs {
	float left_pos;
	float left_vel;
	float right_pos;
	float right_vel;
};

struct PolicyOutputs {
	float left_torque;
	float right_torque;
};

// call once at startup
void ai_init(void);

// fill inputs, run inference, read outputs
// returns 0 on success, -1 on error
int ai_run(const PolicyInputs *inputs, PolicyOutputs *outputs);

#endif
