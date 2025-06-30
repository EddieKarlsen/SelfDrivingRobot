#include <string.h>
#include <stdio.h>
#include "../include/logger.h"
#include "../include/chromosome.h"
#include "../include/configuration.h"


void log_robot_to_file(Individual *ind, const char *filename) {
    FILE *f = fopen(filename, "a");
    if (!f) return;

    fprintf(f, "%d,%d,%.2f,%d,%d,%d,", 
        ind->id, 
        ind->generation,
        ind->fitness, 
        ind->steps_taken, 
        reached_goal(&ind->robot) ? 1 : 0,
        ind->is_best
    );

    for (int i = 0; i < 5; i++) fprintf(f, "%.2f,", ind->chromosome.sensor_weights[i]);
    for (int i = 0; i < 3; i++) fprintf(f, "%.2f,", ind->chromosome.distance_thresholds[i]);
    for (int i = 0; i < 4; i++) fprintf(f, "%.2f,", ind->chromosome.action_priorities[i]);

    fprintf(f, "%.2f,%.2f\n", 
        ind->chromosome.turn_aggressiveness, 
        ind->chromosome.collision_avoidance
    );

    fclose(f);
}

void log_maze_to_file(int **maze, int width, int height, const char *filename) {
    FILE *f = fopen(filename, "a");
    if (!f) return;

    fprintf(f, "Maze %dx%d\n", width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fprintf(f, "%d", maze[y][x]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
    fclose(f);
}

int load_best_individual_from_file(Individual *ind, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;

    char line[512];
    fgets(line, sizeof(line), f);

    float best_fitness = -1e9;
    Individual best;

    while (fgets(line, sizeof(line), f)) {
        Individual temp;
        Chromosome *chr = &temp.chromosome;
        int reached, is_best;

        sscanf(line, "%d,%d,%f,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
               &temp.id,
               &temp.generation,
               &temp.fitness,
               &temp.steps_taken,
               &reached,
               &is_best,
               &chr->sensor_weights[0],
               &chr->sensor_weights[1],
               &chr->sensor_weights[2],
               &chr->sensor_weights[3],
               &chr->sensor_weights[4],
               &chr->distance_thresholds[0],
               &chr->distance_thresholds[1],
               &chr->distance_thresholds[2],
               &chr->action_priorities[0],
               &chr->action_priorities[1],
               &chr->action_priorities[2],
               &chr->action_priorities[3],
               &chr->turn_aggressiveness,
               &chr->collision_avoidance);

        if (temp.fitness > best_fitness) {
            best = temp;
            best_fitness = temp.fitness;
        }
    }

    fclose(f);
    *ind = best;
    return 1;
}