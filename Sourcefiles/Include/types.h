#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef struct Chromosome Chromosome;
typedef struct Individual Individual;
typedef struct Robot Robot;

typedef struct {
    float x, y;
    float angle;
    float range;
} Sensor;

typedef struct Chromosome{
    float sensor_weights[5];
    float distance_thresholds[3]; // t1-t3: nära, mellan, långt

    // Aktionsprioritet för olika situationer
    float action_priorities[4];   // a1-a4: prioritet för olika riktningar
    float turn_aggressiveness;    // Hur aggressiv roboten är med svängar
    float collision_avoidance;    // Viktning för kollisionsundvikande
} Chromosome;

typedef struct Robot{
    float x, y;
    float width, height;
    float angle;
    int orientation;
} Robot;

typedef struct Individual{
    Robot robot;
    Chromosome chromosome; 
    int active;
    float fitness;
    int steps_taken;
    int generation;
    int is_best;
    int id;
    int collision_count;
    bool reached_goal;
} Individual;

typedef struct {
    float x, y;
    float angle;
    int step;
    int action;
    float sensor_readings[5];
} MovementLog;

typedef struct {
    int start_x, start_y;
    int goal_x, goal_y;
    int maze_width, maze_height;
    int **maze;
    Sensor sensors[5];
} Simulationcontext;

typedef enum {
    FORWARD,
    TURN_LEFT_45,
    TURN_RIGHT_45,
    BACKWARD
} Action;

#endif