#include <stdbool.h>
#include "../include/configuration.h"
#include "../include/robot.h"

float simulate_ultrasonic(Robot *robot, int sensor_id, int **maze, int width, int height) {
    float start_x = robot->x;
    float start_y = robot->y;
    float angle = robot->angle + sensors[sensor_id].angle;
    
    // Stega längs strålen tills vi träffar en vägg
    for(float dist = 0; dist < sensors[sensor_id].range; dist += 0.1) {
        int check_x = (int)(start_x + cos(angle) * dist);
        int check_y = (int)(start_y + sin(angle) * dist);
        
        if(check_x < 0 || check_x >= width || check_y < 0 || check_y >= height) {
            return dist; // Träffade kant
        }
        
        if(maze[check_y][check_x] == 1) {
            return dist; // Träffade vägg
        }
    }
    
    return sensors[sensor_id].range; // Inget hinder inom räckvidd
}

bool can_move_to(Robot *robot, int new_x, int new_y, int **maze, int width, int height) {
    // Kontrollera båda rutorna som roboten skulle uppta
    if(robot->orientation == 0 || robot->orientation == 2) {  // Horisontell
        return (maze[new_y][new_x] == 0 && maze[new_y][new_x + 1] == 0);
    } else {  // Vertikal
        return (maze[new_y][new_x] == 0 && maze[new_y + 1][new_x] == 0);
    }
}

Action directions[8][2] = {
    {1,0}, {1,-1}, {0,-1}, {-1,-1},   // 0°, 45°, 90°, 135°
    {-1,0}, {-1,1}, {0,1}, {1,1}      // 180°, 225°, 270°, 315°
};