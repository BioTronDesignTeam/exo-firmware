
bool get_version(uint8_t node_id, version_t *version, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    if (send_msg(node_id, 0x00, NULL, 0, timeout_ms) != HAL_OK) {
        return false;
    }
    
    if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
        return false;
    }
    
    if (version) {
        version->hw_version_major = rx_data[0];     //Copies the hardware version fields from the first three bytes.
        version->hw_version_minor = rx_data[1];
        version->hw_version_variant = rx_data[2];
        version->fw_version_major = rx_data[3];
        version->fw_version_minor = rx_data[4];   //Copies the firmware version fields from the next four bytes.
        version->fw_version_revision = rx_data[5];
        version->fw_version_unreleased = rx_data[6];
        version->protocol_version = rx_data[7];
    }
    
    return true;
}

bool heartbeat(uint8_t *rx_data, heartbeat_t *heartbeat) {
    if (heartbeat == NULL || rx_data == NULL) {
        return false;
    }
    
    memcpy(&heartbeat->axis_error, &rx_data[0], 4);
    heartbeat->axis_state = rx_data[4];
    heartbeat->procedure_result = rx_data[5];
    heartbeat->trajectory_done_flag = rx_data[6];
    
    return true;
}

void estop(uint8_t node_id, uint32_t timeout_ms) {
    send_msg(node_id, 0x02, NULL, 0, timeout_ms);
}

bool get_error(uint8_t node_id, error_t *error, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef rx_header; //new variable to store received message
    uint8_t rx_data[8]; //creates 8 byte buffer
    
    if (send_msg(node_id, 0x03, NULL, 0, timeout_ms) != HAL_OK) { //sends failure command to the device
        return false; //returns false if snending the request fails
    }
    
    if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
        return false; 
    }
    
    if (error) {
        memcpy(&error->active_errors, &rx_data[0], 4); //copies the active errors
        memcpy(&error->disarm_reason, &rx_data[4], 4);
    }
    
    return true;
}

void rx_sdo(uint8_t node_id, uint16_t endpoint_id, uint8_t *data, uint8_t length, uint32_t timeout_ms) {
    uint8_t tx_data[8];
    tx_data[0] = endpoint_id & 0xFF;
    tx_data[1] = (endpoint_id >> 8) & 0xFF;
    tx_data[2] = 0;  // Reserved
    tx_data[3] = 0;  // Reserved
    
    if (length > 0 && data != NULL) {
        memcpy(&tx_data[4], data, (length > 4) ? 4 : length);
    }
    
    send_msg(node_id, 0x04, tx_data, 8, timeout_ms);
}

bool tx_sdo(uint8_t node_id, uint16_t endpoint_id, uint32_t timeout_ms) {
    uint8_t tx_data[4];
    tx_data[0] = endpoint_id & 0xFF;
    tx_data[1] = (endpoint_id >> 8) & 0xFF;
    tx_data[2] = 0;  // Reserved
    tx_data[3] = 0;  // Reserved
    
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    if (send_msg(node_id, 0x05, tx_data, 4, timeout_ms) != HAL_OK) {
        return false;
    }
    
    if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
        return false;
    }
    
    return true;
}

void set_axis_state(uint8_t node_id, float axis_requested_state, uint32_t timeout_ms) {
    uint8_t data[4]; //create an array of 4 bytes
    memcpy(&data[0], &axis_requested_state, 4);
    
    send_msg(node_id, 0x07, data, 4, timeout_ms);
}

getencoder estimates

void set_controller_mode(uint8_t node_id, float control_mode, float input_mode, uint32_t timeout_ms) {
    uint8_t data[8]; 
    memcpy(&data[0], &control_mode, 4);   
    memcpy(&data[4], &input_mode, 4);  
    
    send_msg(node_id, 0x0b, data, 8, timeout_ms); //sends CAN message
}


void set_pos(uint8_t node_id, float pos, float vel, float torq, 
             float vel_scale, float torq_scale, uint32_t timeout_ms) {

    uint8_t data[8]; //CAN messages are 8 bytes maximum. This creates an array called data

    memcpy(&data[0], &pos, 4); //Copies the 4 bytes of pos (float) into the CAN

    int16_t vel_ff = (int16_t)(vel * vel_scale); //send velocity (int16) by multiplying by a scale 
    data[4] = vel_ff & 0xFF;  //takes the lowest 8 bits and sotres it, since CAN messages only sends bytes
    data[5] = (vel_ff >> 8) & 0xFF; //takes the highest 8 bits and stores it

    int16_t torq_ff = (int16_t)(torq * torq_scale); //sends torque as int16
    data[6] = torq_ff & 0xFF;  //takes loweest 8 bit and stores it
    data[7] = (torq_ff >> 8) & 0xFF; //takes highest 8 bits and stores it

    send_msg(node_id, 0x0C, data, 8, timeout_ms); //sends CAN message of the data array to node using hte id 0x0c. containing 8 bytes of data
}

//


void set_vel(uint8_t node_id, float vel, float torq, uint32_t timeout_ms) {
    uint8_t data[8]; //creates array of data 8 
    memcpy(&data[0], &vel, 4);   //copies the 4 bytes of the float 
    memcpy(&data[4], &torq, 4);  //copies the 4 bytes of the torque
    
    send_msg(node_id, 0x0D, data, 8, timeout_ms); //sends CAN message
}

void set_torque(uint8_t node_id, float torq, uint32_t timeout_ms) {
    uint8_t data[4]; //create an array of 4 bytes
    memcpy(&data[0], &torq, 4);
    
    send_msg(node_id, 0x0E, data, 4, timeout_ms);
}

void set_limits(uint8_t node_id, float vel_limit, float current_limit, uint32_t timeout_ms) {
    uint8_t data[8]; //create an array of 8 bytes because theres two floats to send 
    memcpy(&data[0], &vel_limit, 4); //send the velocity limit of byte 4
    memcpy(&data[4], &current_limit, 4); //sends current limit of byte 4
    
    send_msg(node_id, 0x0F, data, 8, timeout_ms);
}

void set_traj_vel_limit(uint8_t node_id, float traj_vel_limit, uint32_t timeout_ms) {
    uint8_t data[4];
    memcpy(&data[0], &traj_vel_limit, 4);

    send_msg(node_id, 0x11, data, 4, timeout_ms)
}

void set_traj_accel_limits(uint8_t node_id, float traj_accel_limit, float traj_decel_limit, uint32_t timeout_ms){
    uint8_t data[8];
    memcpy(&data[0], &traj_accel_limit, 4);
    memcpy(&data[4], &traj_decel_limit, 4);

    send_msg(node_id, 0x12, data, 8, timeout_ms);
}

void set_traj_inertia(uint8_t node_id, float traj_inertia, uint32_t timeout_ms) {
    uint8_t data[4];
    memcpy(&data[0], &traj_inertia, 4);

    send_msg(node_id, 0x13, data, 4, timeout_ms);
}

bool get_iq(uint8_t node_id, float *iq_setpoint, float *iq_measured, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    if (send_msg(node_id, 0x14, NULL, 0, timeout_ms) != HAL_OK) {
        return false;
    }
    
    if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
        return false;
    }
    
    if (iq_setpoint) {
        memcpy(iq_setpoint, &rx_data[0], 4);
    }
    if (iq_measured) {
        memcpy(iq_measured, &rx_data[4], 4);
    }
    
    return true;
}

bool get_temperature(uint8_t node_id, float *fet_temp, float *motor_temp, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    if (send_msg(node_id, 0x15, NULL, 0, timeout_ms) != HAL_OK) {
        return false;
    }
    
    if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
        return false;
    }
    
    if (fet_temp) {
        memcpy(fet_temp, &rx_data[0], 4);
    }
    if (motor_temp) {
        memcpy(motor_temp, &rx_data[4], 4);
    }
    
    return true;
}
void reboot_odrive(uint8_t node_id, uint32_t timeout_ms) {
    send_msg(node_id, 0x16, NULL, 0, timeout_ms);
}
bool get_bus_voltage_current(uint8_t node_id, float *voltage, float *current, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    if (send_msg(node_id, 0x17, NULL, 0, timeout_ms) != HAL_OK) {
        return false;
    }
    
    if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
        return false;
    }
    
    if (voltage) {
        memcpy(voltage, &rx_data[0], 4);
    }
    if (current) {
        memcpy(current, &rx_data[4], 4);
    }
    
    return true;
}
void clear_errors(uint8_t node_id, uint32_t timeout_ms) {
    send_msg(node_id, 0x18, NULL, 0, timeout_ms);
}

void set_absolute_position(uint8_t node_id, float position, uint32_t timeout_ms) {
    uint8_t data[4];
    memcpy(&data[0], &position, 4);

    send_msg(node_id, 0x19, data, 4, timeout_ms);
}

void set_position_gain(uint8_t node_id, float pos_gain, uint32_t timeout_ms) {
    uint8_t data[4];
    memcpy(&data[0], &pos_gain, 4);

    send_msg(node_id, 0x1a, data, 4, timeout_ms);
}

void set_velocity_gains(uint8_t node_id, float vel_gain, float vel_integrator_gain, uint32_t timeout_ms){
    uint8_t data[8];
    memcpy(&data[0], &vel_gain, 4);
    memcpy(&data[4], &vel_integrator_gain, 4);

    send_msg(node_id, 0x1b, data, 8, timeout_ms);
}

bool get_torques(uint8_t node_id, float *torque_target, float *torque_estimate, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef rx_header; //declares variable that requests torque data
    uint8_t rx_data[8]; //create 8 byte array to store CAN information that we are getting
    if (send_msg(node_id, 0x1C, NULL, 0, timeout_ms) != HAL_OK) {
    return false; //returns failure message
}

if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
    return false;
}

if (torque_target) { //if torque target is not null
    memcpy(torque_target, &rx_data[0], 4); //copies the torque motor is trying to apply into bytes 0-3
}
if (torque_estimate) {
    memcpy(torque_estimate, &rx_data[4], 4); //copies torque motor is producing into bytes 4-7
}

return true; //returns successful after recieving data
}

bool get_powers(uint8_t node_id, float *electrical_power, float *mechanical_power, uint32_t timeout_ms) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    if (send_msg(node_id, 0x1D, NULL, 0, timeout_ms) != HAL_OK) {
        return false;
    }
    
    if (!receive_msg(&rx_header, rx_data, timeout_ms)) {
        return false;
    }
    
    if (electrical_power) {
        memcpy(electrical_power, &rx_data[0], 4);
    }
    if (mechanical_power) {
        memcpy(mechanical_power, &rx_data[4], 4);
    }
    
    return true;
}

void enter_dfu_mode(uint8_t node_id, uint32_t timeout_ms) {
    send_msg(node_id, 0x1F, NULL, 0, timeout_ms);
}