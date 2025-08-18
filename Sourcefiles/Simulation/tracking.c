#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/tracking.h"
#include "../include/types.h"
#include "../include/configuration.h"
#include "../include/robot.h"

typedef struct {
    int id;
    int generation;
    int count;
    MovementLog logs[MAX_STEPS];
} IndividualTracking;

static IndividualTracking current;
static int header_written = 0;

void start_tracking_individual(int id) {
    current.id = id;
    current.count = 0;
}

void log_movement_step(Individual *ind, Action action, float sensor_readings[5], int step) {
    if (current.count >= MAX_STEPS) return;

    MovementLog *log = &current.logs[current.count++];
    log->x = ind->robot.x;
    log->y = ind->robot.y;
    log->angle = ind->robot.angle;
    log->step = step;
    log->action = action;
    for (int i = 0; i < 5; i++) {
        log->sensor_readings[i] = sensor_readings[i];
    }
}

void save_movement_data(const char *filename, int id, int generation) {
    FILE *f = fopen(filename, "a");
    if (!f) return;

    if (!header_written) {
        fprintf(f, "# Movement Log Format:\n");
        fprintf(f, "# Individual ID, Generation\n");
        fprintf(f, "# Step, X_Position, Y_Position, Angle_Radians, Action_Code, Sensor_Front, Sensor_Left, Sensor_Right, Sensor_Front_Left, Sensor_Front_Right\n");
        fprintf(f, "# Action codes: 0=FORWARD, 1=TURN_LEFT_45, 2=TURN_RIGHT_45, 3=BACKWARD\n");
        fprintf(f, "# Sensor values are distances in maze units\n");
        fprintf(f, "#\n");
        header_written = 1;
    }

    fprintf(f, "Individual %d, Generation %d\n", id, generation);
    for (int i = 0; i < current.count; i++) {
        MovementLog *m = &current.logs[i];
        fprintf(f, "%d,%.2f,%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                m->step, m->x, m->y, m->angle, m->action,
                m->sensor_readings[0], m->sensor_readings[1], m->sensor_readings[2],
                m->sensor_readings[3], m->sensor_readings[4]);
    }
    fprintf(f, "\n");
    fclose(f);
}

void log_robot_to_file(Individual *ind, const char *filename) {
    static int robot_header_written = 0;
    FILE *f = fopen(filename, "a");
    if (!f) return;

    if (!robot_header_written) {
        fprintf(f, "# Robot Log Format:\n");
        fprintf(f, "# ID, Generation, Fitness, Reached_Goal(1/0), Steps_Taken, Collision_Count, Final_Angle\n");
        fprintf(f, "#\n");
        robot_header_written = 1;
    }

    fprintf(f, "%d,%d,%.2f,%d,%d,%d,%.2f\n",
            ind->id, ind->generation, ind->fitness, ind->reached_goal,
            ind->steps_taken, ind->collision_count, ind->robot.angle);

    fclose(f);
}