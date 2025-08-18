#ifndef ROBOT_H
#define ROBOT_H

#include "types.h"
#include <math.h>
#include "configuration.h"

// Funktionsdeklarationer
bool execute_action(Individual *individual, Action action, Simulationcontext *context);
bool reached_goal(Robot *robot, Simulationcontext *context);
bool check_collision(Robot *robot, float x, float y, float angle, Simulationcontext *context);
float simulate_ultrasonic(Individual *individual, int sensor_id, Simulationcontext *context);

void initialize_robot(Robot *robot, float start_x, float start_y);
void update_orientation(Robot *robot);
void get_direction_vector(Robot *robot, float *dx, float *dy);
void print_robot_status(Robot *robot);

#endif