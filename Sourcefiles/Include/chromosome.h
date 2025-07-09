#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include "types.h"
extern int goal_x, goal_y;

typedef enum {
    FORWARD,
    TURN_LEFT_45,   
    TURN_RIGHT_45,
    BACKWARD
} Action;

typedef struct {
    float sensor_weights[5];
    float distance_thresholds[3]; // t1-t3: nära, mellan, långt

    // Aktionsprioritet för olika situationer
    float action_priorities[4];   // a1-a4: prioritet för olika riktningar
    float turn_aggressiveness;    // Hur aggressiv roboten är med svängar
    float collision_avoidance;    // Viktning för kollisionsundvikande
} Chromosome;

void initialize_chromosome(Chromosome *chr);
void mutate_chromosome(Chromosome *chr, float mutation_rate);
void crossover_chromosomes(Chromosome *parent1, Chromosome *parent2, 
                          Chromosome *child1, Chromosome *child2);
Chromosome copy_chromosome(Chromosome *src);
void print_chromosome(Chromosome *chr);
Action decide_action(Individual *individual, int **maze);
float calculate_fitness(Individual *individual, int steps_taken);

#endif