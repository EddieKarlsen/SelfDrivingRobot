#ifndef ROBOT_H
#define ROBOT_H

#include "types.h"
#include <math.h>
#include "configuration.h"
#include <stdbool.h>


typedef struct {
    float x, y;
    float width, height;
    float angle;
    int orientation;
} Robot;

typedef struct {
    float x, y;
    float angle;
    float range;
} Sensor;

extern Sensor sensors[5];



typedef struct {
    Robot robot;
    Chromosome chromosome; 
    int active;
    float fitness;
    int steps_taken;
    int generation;
    int is_best;
    int id;
} Individual;

// Sensorarray 
Sensor sensors[5] = {
    {0, 0, 0, 50},             // Fram (ultraljud)
    {0, 0, -M_PI/2, 50},       // Vänster (ultraljud)
    {0, 0, M_PI/2, 50},        // Höger (ultraljud)
    {0, 0, -M_PI/4, 30},       // Fram-vänster (IR)
    {0, 0, M_PI/4, 30}         // Fram-höger (IR)
};

bool execute_action(Robot *robot, Action action, int **maze);
bool reached_goal(Robot *robot);
bool check_collision(Robot *robot, float x, float y, float angle, int **maze);
float simulate_ultrasonic(Robot *robot, int sensor_id, int **maze, int width, int height);

void initialize_robot(Robot *robot, float start_x, float start_y);
void update_orientation(Robot *robot);
void get_direction_vector(Robot *robot, float *dx, float *dy);
void print_robot_status(Robot *robot);

extern int maze_width, maze_height;

#endif
