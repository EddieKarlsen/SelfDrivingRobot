#ifndef TRACKING_H
#define TRACKING_H


#include "../include/types.h"
#include "../include/configuration.h"
#include "../include/robot.h"


void start_tracking_individual(int id);
void log_movement_step(Individual *ind, Action action, float sensor_readings[5], int step);
void save_movement_data(const char *filename, int id, int generation);
void log_robot_to_file(Individual *ind, const char *filename);

#endif