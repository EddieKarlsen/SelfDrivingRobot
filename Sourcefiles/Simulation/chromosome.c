#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include "../include/configuration.h"
#include "../include/chromosome.h"
#include "../include/maze.h"
#include "../include/robot.h"

float random_float(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

int find_best_index(Individual population[POP_SIZE]){
    float best_fitness = population[0].fitness;
    int best_index = 0;
    for (int i = 1; i < POP_SIZE; i++) {
        if (population[i].fitness > best_fitness) {
            best_fitness = population[i].fitness;
            best_index = i;
        }
    }
    return best_index;
}

void initialize_chromosome(Chromosome *chr) {
    for(int i = 0; i < 5; i++) {
        chr->sensor_weights[i] = random_float(0.0, 1.0);
    }
    chr->distance_thresholds[0] = random_float(5.0, 15.0);   // Nära
    chr->distance_thresholds[1] = random_float(15.0, 30.0);  // Mellan  
    chr->distance_thresholds[2] = random_float(30.0, 50.0);  // Långt
    
    for(int i = 0; i < 4; i++) {
        chr->action_priorities[i] = random_float(0.0, 1.0);
    }
    
    chr->turn_aggressiveness = random_float(0.1, 2.0);
    chr->collision_avoidance = random_float(0.5, 2.0);
}

void mutate_chromosome(Chromosome *chr, float mutation_rate) {
    for(int i = 0; i < 5; i++) {
        if(random_float(0, 1) < mutation_rate) {
            chr->sensor_weights[i] += random_float(-0.1, 0.1);
            if(chr->sensor_weights[i] < 0) chr->sensor_weights[i] = 0;
            if(chr->sensor_weights[i] > 1) chr->sensor_weights[i] = 1;
        }
    }
    
    for(int i = 0; i < 3; i++) {
        if(random_float(0, 1) < mutation_rate) {
            chr->distance_thresholds[i] += random_float(-2.0, 2.0);
            if(chr->distance_thresholds[i] < 1.0) chr->distance_thresholds[i] = 1.0;
        }
    }
    
    for(int i = 0; i < 4; i++) {
        if(random_float(0, 1) < mutation_rate) {
            chr->action_priorities[i] += random_float(-0.1, 0.1);
            if(chr->action_priorities[i] < 0) chr->action_priorities[i] = 0;
            if(chr->action_priorities[i] > 1) chr->action_priorities[i] = 1;
        }
    }
    
    if(random_float(0, 1) < mutation_rate) {
        chr->turn_aggressiveness += random_float(-0.2, 0.2);
        if(chr->turn_aggressiveness < 0.1) chr->turn_aggressiveness = 0.1;
    }
    
    if(random_float(0, 1) < mutation_rate) {
        chr->collision_avoidance += random_float(-0.2, 0.2);
        if(chr->collision_avoidance < 0.1) chr->collision_avoidance = 0.1;
    }
}

void crossover_chromosomes(Chromosome *parent1, Chromosome *parent2, 
                          Chromosome *child1, Chromosome *child2) {
    for(int i = 0; i < 5; i++) {
        if(rand() % 2) {
            child1->sensor_weights[i] = parent1->sensor_weights[i];
            child2->sensor_weights[i] = parent2->sensor_weights[i];
        } else {
            child1->sensor_weights[i] = parent2->sensor_weights[i];
            child2->sensor_weights[i] = parent1->sensor_weights[i];
        }
    }
    
    for(int i = 0; i < 3; i++) {
        if(rand() % 2) {
            child1->distance_thresholds[i] = parent1->distance_thresholds[i];
            child2->distance_thresholds[i] = parent2->distance_thresholds[i];
        } else {
            child1->distance_thresholds[i] = parent2->distance_thresholds[i];
            child2->distance_thresholds[i] = parent1->distance_thresholds[i];
        }
    }
    
    for(int i = 0; i < 4; i++) {
        if(rand() % 2) {
            child1->action_priorities[i] = parent1->action_priorities[i];
            child2->action_priorities[i] = parent2->action_priorities[i];
        } else {
            child1->action_priorities[i] = parent2->action_priorities[i];
            child2->action_priorities[i] = parent1->action_priorities[i];
        }
    }
    
    if(rand() % 2) {
        child1->turn_aggressiveness = parent1->turn_aggressiveness;
        child2->turn_aggressiveness = parent2->turn_aggressiveness;
        child1->collision_avoidance = parent1->collision_avoidance;
        child2->collision_avoidance = parent2->collision_avoidance;
    } else {
        child1->turn_aggressiveness = parent2->turn_aggressiveness;
        child2->turn_aggressiveness = parent1->turn_aggressiveness;
        child1->collision_avoidance = parent2->collision_avoidance;
        child2->collision_avoidance = parent1->collision_avoidance;
    }
}

void elite_selection(Individual old_pop[POP_SIZE], Individual new_pop[POP_SIZE], 
                     int generation, int *id_counter) {
    int best1 = 0, best2 = 1;
    if (old_pop[1].fitness > old_pop[0].fitness) {
        best1 = 1; best2 = 0;
    }
    for (int i = 2; i < POP_SIZE; i++) {
        if (old_pop[i].fitness > old_pop[best1].fitness) {
            best2 = best1;
            best1 = i;
        } else if (old_pop[i].fitness > old_pop[best2].fitness) {
            best2 = i;
        }
    }

    new_pop[0] = old_pop[best1];
    new_pop[1] = old_pop[best2];
    new_pop[0].is_best = 1;
    new_pop[1].is_best = 0;
    
    new_pop[0].reached_goal = false;
    new_pop[1].reached_goal = false;
    new_pop[0].collision_count = 0;
    new_pop[1].collision_count = 0;

    for (int i = 2; i < POP_SIZE; i += 2) {
        Individual *parent1 = &old_pop[best1];
        Individual *parent2 = &old_pop[best2];

        Chromosome c1, c2;
        crossover_chromosomes(&parent1->chromosome, &parent2->chromosome, &c1, &c2);
        mutate_chromosome(&c1, 0.1f);
        mutate_chromosome(&c2, 0.1f);

        initialize_robot(&new_pop[i].robot, 1.0f, 1.0f);
        new_pop[i].chromosome = c1;
        new_pop[i].active = 1;
        new_pop[i].id = (*id_counter)++;
        new_pop[i].generation = generation + 1;
        new_pop[i].is_best = 0;
        new_pop[i].reached_goal = false;
        new_pop[i].collision_count = 0;
        new_pop[i].fitness = 0;
        new_pop[i].steps_taken = 0;

        if (i + 1 < POP_SIZE) {
            initialize_robot(&new_pop[i + 1].robot, 1.0f, 1.0f);
            new_pop[i + 1].chromosome = c2;
            new_pop[i + 1].active = 1;
            new_pop[i + 1].id = (*id_counter)++;
            new_pop[i + 1].generation = generation + 1;
            new_pop[i + 1].is_best = 0;
            new_pop[i + 1].reached_goal = false;
            new_pop[i + 1].collision_count = 0;
            new_pop[i + 1].fitness = 0;
            new_pop[i + 1].steps_taken = 0;
        }
    }
}
// works on a point system which is based on the sensors and the chromosomes of the indiviudal 
Action decide_action(Individual *individual, Simulationcontext *context) {
    Chromosome *chr = &individual->chromosome;
    float sensor_readings[5];

    for (int i = 0; i < 5; i++) {
       sensor_readings[i] = simulate_ultrasonic(individual, i, context);
    }

    float action_scores[4] = {0}; // 0: FORWARD, 1: LEFT, 2: RIGHT, 3: BACKWARD


    if (sensor_readings[0] > chr->distance_thresholds[1])
        action_scores[0] = chr->action_priorities[0] * chr->sensor_weights[0];

    if (sensor_readings[1] > chr->distance_thresholds[0])
        action_scores[1] = chr->action_priorities[1] * chr->sensor_weights[1];

    if (sensor_readings[2] > chr->distance_thresholds[0])
        action_scores[2] = chr->action_priorities[2] * chr->sensor_weights[2];

    action_scores[3] = chr->action_priorities[3] * 0.3f;

  
    if (sensor_readings[3] > chr->distance_thresholds[0]) {
        action_scores[1] += 0.5f * chr->sensor_weights[3];
    }

    if (sensor_readings[4] > chr->distance_thresholds[0]) {
        action_scores[2] += 0.5f * chr->sensor_weights[4];
    }

    for (int i = 0; i < 5; i++) {
        if (sensor_readings[i] < chr->distance_thresholds[0]) {
            float penalty = chr->collision_avoidance;

            if (i == 0) action_scores[0] /= penalty;
            if (i == 1 || i == 3) action_scores[1] /= penalty;
            if (i == 2 || i == 4) action_scores[2] /= penalty;
        }
    }

    action_scores[1] *= chr->turn_aggressiveness;
    action_scores[2] *= chr->turn_aggressiveness;

    bool all_zero = true;
    for (int i = 0; i < 4; i++) {
        if (action_scores[i] > 0.01f) {
            all_zero = false;
            break;
        }
    }
    if (all_zero) return BACKWARD;

    int best_action = 0;
    float best_score = action_scores[0];
    for (int i = 1; i < 4; i++) {
        if (action_scores[i] > best_score) {
            best_score = action_scores[i];
            best_action = i;
        }
    }

    return (Action)best_action;
}

float calculate_fitness(Individual *individual, int steps_taken, Simulationcontext *context) {
    Robot *robot = &individual->robot;
    float base_line = BASE_LINE_FITNESS;
    // weights for the penalties and bonus
    float alpha = 1.0, beta = 0.5, gamma = 0.3, delta = 2.0, epsilon = 10.0;
    
    float time_penalty = alpha * steps_taken;
    float energy_penalty = beta * steps_taken * 0.1; // Approximation will change when i start with robot
    

    float distance_to_goal = sqrt(pow(robot->x - context->goal_x, 2) + 
                                  pow(robot->y - context->goal_y, 2));
    float distance_penalty = gamma * distance_to_goal;
    
    float number_of_collisons = individual->collision_count;
    float collison_penalty = delta * number_of_collisons;
    
    float goal_bonus = 0;
    if(individual->reached_goal) { 
        goal_bonus = epsilon * 100;
    }
    
    // Higher fitness is better 
    float fitness = base_line + goal_bonus - (time_penalty + energy_penalty + distance_penalty + collison_penalty);
    if (fitness <= 0) fitness = 1.0f;
    return fitness;
}