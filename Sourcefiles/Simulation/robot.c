#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "../include/configuration.h"
#include "../include/robot.h"
#include "../include/rotation.h"
#include "../include/types.h"
#include "../include/maze.h"

#define MOVE_DISTANCE 1.0f
#define TURN_ANGLE_45 (M_PI / 4.0f)
#define GOAL_THRESHOLD 2.0f

// Initiate robot whith standardvalues
void initialize_robot(Robot *robot, float start_x, float start_y) {
    robot->x = start_x;
    robot->y = start_y;
    robot->angle = 0.0f;        // points towards the right
    robot->width = 2.0f;
    robot->height = 2.0f;
    robot->orientation = 0;
}

void update_orientation(Robot *robot) {
    float normalized_angle = fmod(robot->angle + 2 * M_PI, 2 * M_PI);
    robot->orientation = (int)((normalized_angle + M_PI / 8) / (M_PI / 4)) % 8;
}

void get_direction_vector(Robot *robot, float *dx, float *dy) {
    *dx = cos(robot->angle);
    *dy = sin(robot->angle);
}

bool reached_goal(Robot *robot, Simulationcontext *context) {
    float distance = sqrtf(powf(robot->x - context->goal_x, 2) + 
                           powf(robot->y - context->goal_y, 2));
    return distance <= GOAL_THRESHOLD;
}

bool check_collision(Robot *robot, float x, float y, float angle, Simulationcontext *context) {
    float local_corners[4][2] = {
        {-robot->width / 2, -robot->height / 2},
        { robot->width / 2, -robot->height / 2},
        { robot->width / 2,  robot->height / 2},
        {-robot->width / 2,  robot->height / 2}
    };

    for (int i = 0; i < 4; i++) {
        float world_x, world_y;
        rotate_point(local_corners[i][0], local_corners[i][1], angle, &world_x, &world_y);
        world_x += x;
        world_y += y;

        int grid_x = (int)floor(world_x);
        int grid_y = (int)floor(world_y);

        if (grid_x < 0 || grid_x >= context->maze_width || 
            grid_y < 0 || grid_y >= context->maze_height)
            return true;

        if (context->maze[grid_y][grid_x] == WALL || context->maze[grid_y][grid_x] == BORDER)
            return true;
    }

    return false;
}

bool execute_action(Individual *individual, Action action, Simulationcontext *context) {
    Robot *robot = &individual->robot;

    switch (action) {
        case FORWARD: {
            float new_x = robot->x + cos(robot->angle) * MOVE_DISTANCE;
            float new_y = robot->y + sin(robot->angle) * MOVE_DISTANCE;

            if (check_collision(robot, new_x, new_y, robot->angle, context)) {
                individual->collision_count++;
                return false;
            }

            robot->x = new_x;
            robot->y = new_y;
            break;
        }

        case BACKWARD: {
            float new_x = robot->x - cos(robot->angle) * MOVE_DISTANCE;
            float new_y = robot->y - sin(robot->angle) * MOVE_DISTANCE;

            if (check_collision(robot, new_x, new_y, robot->angle, context)) {
                individual->collision_count++;
                return false;
            }

            robot->x = new_x;
            robot->y = new_y;
            break;
        }

        case TURN_LEFT_45:
            robot->angle -= TURN_ANGLE_45;
            while (robot->angle < 0) robot->angle += 2 * M_PI;
            break;

        case TURN_RIGHT_45:
            robot->angle += TURN_ANGLE_45;
            while (robot->angle >= 2 * M_PI) robot->angle -= 2 * M_PI;
            break;

        default:
            return false;
    }

    update_orientation(robot);
    return true;
}


float simulate_ultrasonic(Individual *individual, int sensor_id, Simulationcontext *context) {
    float start_x = individual->robot.x;
    float start_y = individual->robot.y;
    float angle = individual->robot.angle + context->sensors[sensor_id].angle;

    for (float dist = 0; dist < context->sensors[sensor_id].range; dist += 0.5f) {
        int check_x = (int)(start_x + cos(angle) * dist);
        int check_y = (int)(start_y + sin(angle) * dist);

        if (check_x < 0 || check_x >= context->maze_width || 
            check_y < 0 || check_y >= context->maze_height)
            return dist;

        if (context->maze[check_y][check_x] == WALL || context->maze[check_y][check_x] == BORDER)
            return dist;
    }

    return context->sensors[sensor_id].range;
}

void print_robot_status(Robot *robot) {
    printf("Robot: (%.1f, %.1f), angle=%.2f rad, orientation=%d\n",
           robot->x, robot->y, robot->angle, robot->orientation);
}