#include "ai_platform.h"
#include "network.h"
#include "network_data.h"
#include <string.h>

#include "ai_inference.hpp"

static ai_handle network = AI_HANDLE_NULL;

// Activations buffer — must be 4-byte aligned
AI_ALIGNED(4)
static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

static ai_float in_data[AI_NETWORK_IN_1_SIZE];
static ai_float out_data[AI_NETWORK_OUT_1_SIZE];

void ai_init(void)
{
    ai_error err;

    // create network instance with activations buffer
    const ai_handle acts[] = { activations };
    err = ai_network_create_and_init(&network, acts, NULL);

    if (err.type != AI_ERROR_NONE)
    {
        // handle init failure. loop here so debugger catches it.
        while (1);
    }
}

int ai_run(const PolicyInputs *inputs, PolicyOutputs *outputs)
{
    if (inputs == NULL || outputs == NULL)
        return -1;

    // pack struct fields into flat input buffer
    in_data[0] = inputs->left_pos;
    in_data[1] = inputs->left_vel;
    in_data[2] = inputs->right_pos;
    in_data[3] = inputs->right_vel;

    // set up AI buffer descriptors
    ai_buffer ai_input[AI_NETWORK_IN_NUM]   = AI_NETWORK_IN;
    ai_buffer ai_output[AI_NETWORK_OUT_NUM] = AI_NETWORK_OUT;

    ai_input[0].data  = AI_HANDLE_PTR(in_data);
    ai_output[0].data = AI_HANDLE_PTR(out_data);

    // run inference
    ai_i32 n_batch = ai_network_run(network, ai_input, ai_output);

    if (n_batch != 1)
    {
        // optionally read the error
        // ai_error err = ai_network_get_error(network);
        return -1;
    }

    // unpack output buffer into struct
    outputs->left_torque  = out_data[0];
    outputs->right_torque = out_data[1];

    return 0;
}
