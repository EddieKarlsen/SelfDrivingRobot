#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "../include/configuration.h"
#include "../include/chromosome.h"
#include "../include/maze.h"
#include "../include/robot.h"

extern int goal_x, goal_y; 

// Slumptalsgenerator
float random_float(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

void initialize_chromosome(Chromosome *chr) {
    // Initialisera sensorviktningar (0.0 - 1.0)
    for(int i = 0; i < 5; i++) {
        chr->sensor_weights[i] = random_float(0.0, 1.0);
    }
    
    // Initialisera tröskelvärden (stigande ordning)
    chr->distance_thresholds[0] = random_float(5.0, 15.0);   // Nära
    chr->distance_thresholds[1] = random_float(15.0, 30.0);  // Mellan  
    chr->distance_thresholds[2] = random_float(30.0, 50.0);  // Långt
    
    // Aktionsprioritet
    for(int i = 0; i < 4; i++) {
        chr->action_priorities[i] = random_float(0.0, 1.0);
    }
    
    chr->turn_aggressiveness = random_float(0.1, 2.0);
    chr->collision_avoidance = random_float(0.5, 2.0);
}

void mutate_chromosome(Chromosome *chr, float mutation_rate) {
    // Mutera sensorviktningar
    for(int i = 0; i < 5; i++) {
        if(random_float(0, 1) < mutation_rate) {
            chr->sensor_weights[i] += random_float(-0.1, 0.1);
            // Klämma till [0,1]
            if(chr->sensor_weights[i] < 0) chr->sensor_weights[i] = 0;
            if(chr->sensor_weights[i] > 1) chr->sensor_weights[i] = 1;
        }
    }
    
    // Mutera tröskelvärden
    for(int i = 0; i < 3; i++) {
        if(random_float(0, 1) < mutation_rate) {
            chr->distance_thresholds[i] += random_float(-2.0, 2.0);
            if(chr->distance_thresholds[i] < 1.0) chr->distance_thresholds[i] = 1.0;
        }
    }
    
    // Mutera aktionsprioritet
    for(int i = 0; i < 4; i++) {
        if(random_float(0, 1) < mutation_rate) {
            chr->action_priorities[i] += random_float(-0.1, 0.1);
            if(chr->action_priorities[i] < 0) chr->action_priorities[i] = 0;
            if(chr->action_priorities[i] > 1) chr->action_priorities[i] = 1;
        }
    }
    
    // Mutera övriga parametrar
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
    // Uniform crossover
    for(int i = 0; i < 5; i++) {
        if(rand() % 2) {
            child1->sensor_weights[i] = parent1->sensor_weights[i];
            child2->sensor_weights[i] = parent2->sensor_weights[i];
        } else {
            child1->sensor_weights[i] = parent2->sensor_weights[i];
            child2->sensor_weights[i] = parent1->sensor_weights[i];
        }
    }
    
    // Korsning för tröskelvärden och prioriteter på samma sätt
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
}

void elite_selection(Individual old_pop[POP_SIZE], Individual new_pop[POP_SIZE], int generation, int *id_counter) {
    // 1. Hitta två bästa
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

    // 2. Kopiera bästa till nya populationen
    new_pop[0] = old_pop[best1];
    new_pop[1] = old_pop[best2];
    new_pop[0].is_best = 1;
    new_pop[1].is_best = 0;

    // 3. Generera resten
    for (int i = 2; i < POP_SIZE; i += 2) {
        Individual *parent1 = &old_pop[rand() % POP_SIZE];
        Individual *parent2 = &old_pop[rand() % POP_SIZE];

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

        if (i + 1 < POP_SIZE) {
            initialize_robot(&new_pop[i + 1].robot, 1.0f, 1.0f);
            new_pop[i + 1].chromosome = c2;
            new_pop[i + 1].active = 1;
            new_pop[i + 1].id = (*id_counter)++;
            new_pop[i + 1].generation = generation + 1;
            new_pop[i + 1].is_best = 0;
        }
    }
}

Action decide_action(Individual *individual, int **maze) {
    Robot *robot = &individual->robot;
    Chromosome *chr = &individual->chromosome;
    
    // Läs av alla sensorer
    float sensor_readings[5];
    for(int i = 0; i < 5; i++) {
        sensor_readings[i] = simulate_ultrasonic(robot, i, maze, 50, 50); // Anpassa storlek
    }
    
    // Beräkna viktade poäng för varje möjlig aktion
    float action_scores[5] = {0}; // FORWARD, TURN_LEFT_45, TURN_RIGHT_45, BACKWARD
    
    // Framåt: Prioritera om vägen framåt är fri
    if(sensor_readings[0] > chr->distance_thresholds[1]) { // Fram sensor
        action_scores[0] = chr->action_priorities[0] * chr->sensor_weights[0];
    }
    
    // Vänstersvärg: Bra om vänster sida är fri
    if(sensor_readings[1] > chr->distance_thresholds[0]) { // Vänster sensor
        action_scores[1] = chr->action_priorities[1] * chr->sensor_weights[1];
    }
    
    // Högersvärg: Bra om höger sida är fri  
    if(sensor_readings[2] > chr->distance_thresholds[0]) { // Höger sensor
        action_scores[2] = chr->action_priorities[2] * chr->sensor_weights[2];
    }
    
    // Kollisionsundvikande: Minska poäng om sensorer detekterar nära hinder
    for(int i = 0; i < 5; i++) {
        if(sensor_readings[i] < chr->distance_thresholds[0]) {
            // Kraftigt straff för framåt om hinder framför
            if(i == 0) action_scores[0] *= 0.1;
            // Straff för svängar om hinder på sidorna
            if(i == 1) action_scores[1] *= 0.5;
            if(i == 2) action_scores[2] *= 0.5;
        }
    }
    
    // Hitta bästa aktion
    int best_action = 0;
    float best_score = action_scores[0];
    
    for(int i = 1; i < 4; i++) {
        if(action_scores[i] > best_score) {
            best_score = action_scores[i];
            best_action = i;
        }
    }
    
    return (Action)best_action;
}

float calculate_fitness(Individual *individual, int steps_taken) {
    Robot *robot = &individual->robot;
    
    // Fitness-funktion från rapporten: f = (α·T + β·E + γ·D + δ·C) - (ε·B)
    float alpha = 1.0, beta = 0.5, gamma = 0.3, delta = 2.0, epsilon = 10.0;
    
    float time_penalty = alpha * steps_taken;
    float energy_penalty = beta * steps_taken * 0.1; // Approximation
    
    // Avstånd till mål (enkel euklidisk)
    float distance_to_goal = sqrt(pow(robot->x - goal_x, 2) + pow(robot->y - goal_y, 2));
    float distance_penalty = gamma * distance_to_goal;
    
    // Bonus om målet nås
    float goal_bonus = 0;
    if(distance_to_goal < 2.0) {
        goal_bonus = epsilon * 100;
    }
    
    // Högre fitness = bättre (negativ kostnad)
    return goal_bonus - (time_penalty + energy_penalty + distance_penalty);
}

void print_chromosome(Chromosome *chr) {
    printf("Sensor weights: ");
    for(int i = 0; i < 5; i++) {
        printf("%.2f ", chr->sensor_weights[i]);
    }
    printf("\nThresholds: %.1f %.1f %.1f\n", 
           chr->distance_thresholds[0], 
           chr->distance_thresholds[1], 
           chr->distance_thresholds[2]);
}

