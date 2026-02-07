#ifndef ODRIVE_CAN_DRIVER_H
#define ODRIVE_CAN_DRIVER_H

#include "stm32f4xx_hal.h"  // Change to your STM32 series (f1xx, f4xx, f7xx, h7xx, etc.)
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * ODrive Axis States
 * ============================================================================ */
typedef enum {
    AXIS_STATE_UNDEFINED = 0,
    AXIS_STATE_IDLE = 1,
    AXIS_STATE_STARTUP_SEQUENCE = 2,
    AXIS_STATE_FULL_CALIBRATION = 3,
    AXIS_STATE_MOTOR_CALIBRATION = 4,
    AXIS_STATE_ENCODER_INDEX_SEARCH = 6,
    AXIS_STATE_ENCODER_OFFSET_CALIBRATION = 7,
    AXIS_STATE_CLOSED_LOOP_CONTROL = 8,
    AXIS_STATE_LOCKIN_SPIN = 9,
    AXIS_STATE_ENCODER_DIR_FIND = 10,
    AXIS_STATE_HOMING = 11,
} ODriveAxisState;

/* ============================================================================
 * ODrive Control Modes
 * ============================================================================ */
typedef enum {
    CONTROL_MODE_VOLTAGE = 0,
    CONTROL_MODE_TORQUE = 1,
    CONTROL_MODE_VELOCITY = 2,
    CONTROL_MODE_POSITION = 3,
} ODriveControlMode;

/* ============================================================================
 * ODrive Input Modes
 * ============================================================================ */
typedef enum {
    INPUT_MODE_INACTIVE = 0,
    INPUT_MODE_PASSTHROUGH = 1,
    INPUT_MODE_VEL_RAMP = 2,
    INPUT_MODE_POS_FILTER = 3,
    INPUT_MODE_TRAP_TRAJ = 5,
    INPUT_MODE_TORQUE_RAMP = 6,
    INPUT_MODE_MIRROR = 7,
} ODriveInputMode;

/* ============================================================================
 * Feedback Structure
 * ============================================================================ */
typedef struct {
    float pos;  // Position in turns/rotations
    float vel;  // Velocity in turns/second
} feedback;

/* ============================================================================
 * Exoskeleton Joint Definitions (Example - Modify for your design)
 * ============================================================================ */
#define LEFT_HIP_ODRIVE      0
#define LEFT_KNEE_ODRIVE     1
#define LEFT_ANKLE_ODRIVE    2
#define RIGHT_HIP_ODRIVE     3
#define RIGHT_KNEE_ODRIVE    4
#define RIGHT_ANKLE_ODRIVE   5


void odrive_can_init(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef send_msg(uint8_t node_id, uint8_t cmd_id, uint8_t *data, uint8_t length, uint32_t timeout_ms);
bool receive_msg(CAN_RxHeaderTypeDef *rx_header, uint8_t *rx_data, uint32_t timeout_ms);
void set_pos(uint8_t node_id, float pos, float vel, float torq, 
             float vel_scale, float torq_scale, uint32_t timeout_ms);
void set_pos_no_scale(uint8_t node_id, float pos, float vel, float torq, uint32_t timeout_ms);
void set_vel(uint8_t node_id, float vel, float torq, uint32_t timeout_ms);
void set_torque(uint8_t node_id, float torq, uint32_t timeout_ms);
feedback get_encoder_estimates(uint8_t node_id, uint32_t timeout_ms);
float get_torque_estimate(uint8_t node_id, uint32_t timeout_ms);
bool get_bus_voltage_current(uint8_t node_id, float *voltage, float *current, uint32_t timeout_ms);
uint32_t get_motor_error(uint8_t node_id, uint32_t timeout_ms);
uint32_t get_encoder_error(uint8_t node_id, uint32_t timeout_ms);
void set_axis_state(uint8_t node_id, uint32_t state, uint32_t timeout_ms);
void set_controller_mode(uint8_t node_id, uint32_t control_mode, uint32_t input_mode, uint32_t timeout_ms);
void set_limits(uint8_t node_id, float vel_limit, float current_limit, uint32_t timeout_ms);
void set_pos_gain(uint8_t node_id, float pos_gain, uint32_t timeout_ms);
void set_vel_gains(uint8_t node_id, float vel_gain, float vel_integrator_gain, uint32_t timeout_ms);
void estop(uint8_t node_id, uint32_t timeout_ms);
void clear_errors(uint8_t node_id, uint32_t timeout_ms);
void reboot_odrive(uint8_t node_id, uint32_t timeout_ms);
void set_absolute_position(uint8_t node_id, float position, uint32_t timeout_ms);
void set_traj_vel_limit(uint8_t node_id, float traj_vel_limit, uint32_t timeout_ms);
void set_traj_accel_limits(uint8_t node_id, float accel_limit, float decel_limit, uint32_t timeout_ms);
void set_traj_inertia(uint8_t node_id, float inertia, uint32_t timeout_ms);
bool get_torques(uint8_t node_id, float *torque_target, float *torque_estimate, uint32_t timeout_ms);
bool get_powers(uint8_t node_id, float *electrical_power, float *mechanical_power, uint32_t timeout_ms);
bool get_temperature(uint8_t node_id, float *fet_temp, float *motor_temp, uint32_t timeout_ms);

#endif // ODRIVE_CAN_DRIVER_H