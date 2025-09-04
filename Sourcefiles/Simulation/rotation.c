#include <math.h>
#include "../include/rotation.h"
#include "../include/robot.h"

void rotate_point(float px, float py, float angle, float *rx, float *ry) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    *rx = px * cos_a - py * sin_a;
    *ry = px * sin_a + py * cos_a;
}

void get_robot_corners(Robot *robot, float corners[4][2]) {
    // Local cordinates for robots corner (relative to its centrum)
    float local[4][2] = {{-1,-0.5}, {1,-0.5}, {1,0.5}, {-1,0.5}};
    
    for(int i = 0; i < 4; i++) {
        rotate_point(local[i][0], local[i][1], robot->angle, 
                    &corners[i][0], &corners[i][1]);
        corners[i][0] += robot->x;
        corners[i][1] += robot->y;
    }
}