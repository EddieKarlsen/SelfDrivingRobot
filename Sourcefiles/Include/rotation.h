#ifndef ROTATION_H
#define ROTATION_H

#include"../include/types.h"

// Funktionsdeklarationer
// rotate_point(local_corners[i][0], local_corners[i][1], angle, &world_x, &world_y);
void rotate_point(float px, float py, float angle, float *rx, float *ry);
void get_robot_corners(Robot *robot, float corners[4][2]); 
#endif