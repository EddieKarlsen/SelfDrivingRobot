#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include "../include/types.h"
#include "configuration.h"

// Funktionsdeklarationer
void initialize_chromosome(Chromosome *chr);
void mutate_chromosome(Chromosome *chr, float mutation_rate);
void crossover_chromosomes(Chromosome *parent1, Chromosome *parent2, 
                          Chromosome *child1, Chromosome *child2);
void elite_selection(Individual old_pop[POP_SIZE], Individual new_pop[POP_SIZE], 
                     int generation, int *id_counter);
Individual* tournament_select(Individual population[]);

Action decide_action(Individual *individual, Simulationcontext *context);
float calculate_fitness(Individual *individual, int steps_taken, Simulationcontext *context);
int find_best_index(Individual pop[POP_SIZE]);

// Hjälpfunktioner
float random_float(float min, float max);

#endif