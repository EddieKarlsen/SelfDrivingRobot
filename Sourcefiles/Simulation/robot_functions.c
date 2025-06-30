#include <math.h>
#include <stdbool.h>
#include "../include/configuration.h"
#include "../include/robot.h"
#include "../include/maze.h"

// Konstanter för robotens rörelser
#define MOVE_DISTANCE 1.0f
#define TURN_ANGLE_45 (M_PI / 4.0f)
#define GOAL_THRESHOLD 2.0f

bool execute_action(Robot *robot, Action action, int **maze) {
    // Spara nuvarande position för att kunna återställa vid kollision
    float old_x = robot->x;
    float old_y = robot->y;
    float old_angle = robot->angle;
    
    switch(action) {
        case FORWARD: {
            // Beräkna ny position
            float new_x = robot->x + cos(robot->angle) * MOVE_DISTANCE;
            float new_y = robot->y + sin(robot->angle) * MOVE_DISTANCE;
            
            // Kontrollera kollision innan flytt
            if (check_collision(robot, new_x, new_y, robot->angle, maze)) {
                return false; // Kollision - rör dig inte
            }
            
            robot->x = new_x;
            robot->y = new_y;
            break;
        }
        
        case TURN_LEFT_45:
            robot->angle -= TURN_ANGLE_45;
            // Normalisera vinkel till [0, 2π]
            while(robot->angle < 0) robot->angle += 2 * M_PI;
            break;
            
        case TURN_RIGHT_45:
            robot->angle += TURN_ANGLE_45;
            // Normalisera vinkel till [0, 2π]
            while(robot->angle >= 2 * M_PI) robot->angle -= 2 * M_PI;
            break;
            
        case BACKWARD: {
            // Flytta bakåt
            float new_x = robot->x - cos(robot->angle) * MOVE_DISTANCE;
            float new_y = robot->y - sin(robot->angle) * MOVE_DISTANCE;
            
            if (check_collision(robot, new_x, new_y, robot->angle, maze)) {
                return false; // Kollision - rör dig inte
            }
            
            robot->x = new_x;
            robot->y = new_y;
            break;
        }
        
        default:
            return false; // Okänd aktion
    }
    
    return true; // Framgångsrik rörelse
}

bool check_collision(Robot *robot, float x, float y, float angle, int **maze) {
    // Robotens hörn i lokala koordinater (relativt centrum)
    float local_corners[4][2] = {
        {-robot->width/2, -robot->height/2},  // Bakre vänster
        {robot->width/2, -robot->height/2},   // Bakre höger  
        {robot->width/2, robot->height/2},    // Främre höger
        {-robot->width/2, robot->height/2}    // Främre vänster
    };
    
    // Transformera till världskoordinater och kontrollera kollision
    for(int i = 0; i < 4; i++) {
        float world_x, world_y;
        rotate_point(local_corners[i][0], local_corners[i][1], angle, &world_x, &world_y);
        world_x += x;
        world_y += y;
        
        // Konvertera till rutnätkoordinater
        int grid_x = (int)floor(world_x);
        int grid_y = (int)floor(world_y);
        
        // Kontrollera gränser
        if(grid_x < 0 || grid_x >= maze_width || grid_y < 0 || grid_y >= maze_height) {
            return true; // Kollision med kanter
        }
        
        // Kontrollera vägg
        if(maze[grid_y][grid_x] == 1) {
            return true; // Kollision med vägg
        }
    }
    
    return false; // Ingen kollision
}

bool reached_goal(Robot *robot) {
    // Antag att målet är vid position (45, 45) som i din fitness-funktion
    float goal_x = 45.0f;
    float goal_y = 45.0f;
    
    // Beräkna euklidiskt avstånd till mål
    float distance = sqrt(pow(robot->x - goal_x, 2) + pow(robot->y - goal_y, 2));
    
    return distance <= GOAL_THRESHOLD;
}

// Hjälpfunktion för att initiera robot
void initialize_robot(Robot *robot, float start_x, float start_y) {
    robot->x = start_x;
    robot->y = start_y;
    robot->angle = 0.0f; // Peka åt höger initialt
    robot->width = 2.0f;  // Robotens bredd
    robot->height = 2.0f; // Robotens höjd
    robot->orientation = 0; // Höger
}

// Hjälpfunktion för att uppdatera orientation baserat på angle
void update_orientation(Robot *robot) {
    // Konvertera från radianer till diskreta riktningar
    float degrees = robot->angle * 180.0f / M_PI;
    
    // Normalisera till 0-360
    while(degrees < 0) degrees += 360;
    while(degrees >= 360) degrees -= 360;
    
    // Bestäm närmaste riktning (0=höger, 1=upp, 2=vänster, 3=ner)
    if(degrees < 45 || degrees >= 315) {
        robot->orientation = 0; // Höger
    } else if(degrees < 135) {
        robot->orientation = 1; // Upp  
    } else if(degrees < 225) {
        robot->orientation = 2; // Vänster
    } else {
        robot->orientation = 3; // Ner
    }
}

// Hjälpfunktion för att få robotens nuvarande riktningsvektor
void get_direction_vector(Robot *robot, float *dx, float *dy) {
    *dx = cos(robot->angle);
    *dy = sin(robot->angle);
}

// Debug-funktion för att skriva ut robotstatus
void print_robot_status(Robot *robot) {
    printf("Robot: pos(%.2f, %.2f), angle=%.2f rad (%.1f°), orient=%d\n",
           robot->x, robot->y, robot->angle, 
           robot->angle * 180.0f / M_PI, robot->orientation);
}