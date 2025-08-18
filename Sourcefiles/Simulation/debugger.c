#include "../include/debugger.h"
#include <math.h>
#include"../include/maze.h"
#include <stdio.h>

void debug_print_individual(const Individual *ind, int step) {
    printf("Individ %d | Gen %d | Steg %d | Aktiv: %d | Fitness: %.2f\n",
          ind->id, ind->generation, step, ind->active, ind->fitness);

    printf("    Pos: (%.1f, %.1f) | Vinkel: %.2f | Koll: %d | Mål: %s\n",
        ind->robot.x, ind->robot.y, ind->robot.angle,
        ind->collision_count,
        ind->reached_goal ? "JA" : "nej");

    printf("    Kromosom:\n");
    printf("      Sensorvikter: ");
    for (int i = 0; i < 5; i++) printf("%.2f ", ind->chromosome.sensor_weights[i]);
    printf("\n");

    printf("      Avståndströsklar: ");
    for (int i = 0; i < 3; i++) printf("%.2f ", ind->chromosome.distance_thresholds[i]);
    printf("\n");

    printf("      Prioritet (a1–a4): ");
    for (int i = 0; i < 4; i++) printf("%.2f ", ind->chromosome.action_priorities[i]);
    printf("\n");

    printf("      Svängaggressivitet: %.2f | Kollisionsundvik: %.2f\n",
        ind->chromosome.turn_aggressiveness,
        ind->chromosome.collision_avoidance);

    printf("------------------------------------------------------------\n");
}

void debug_print_maze(int **maze, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            switch (maze[y][x]) {
                case BORDER: printf("B"); break; 
                case WALL:   printf("#"); break;  
                case EMPTY:  printf("O"); break;
                case GOAL:   printf("G"); break;
                case START:  printf("S"); break;
                default:     printf("?"); break;
            }
        }
        printf("\n");
    }
    printf("\n");
}