#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/configuration.h"
#include "../include/robot.h"
#include "../include/maze.h"
#include "../include/chromosome.h"
#include "../include/rotation.h"
#include "../include/logger.h"
#define POP_SIZE 50
const int p = 50;

// Sensorarray
Sensor sensors[5] = {
    {0, 0, 0, 50},             // Fram (ultraljud)
    {0, 0, -M_PI/2, 50},       // Vänster (ultraljud)
    {0, 0, M_PI/2, 50},        // Höger (ultraljud)
    {0, 0, -M_PI/4, 30},       // Fram-vänster (IR)
    {0, 0, M_PI/4, 30}         // Fram-höger (IR)
};

// Globala maze-dimensioner (används i check_collision och sensorfunktioner)
int maze_width = 50;
int maze_height = 50;

// Globala målkoordinater (för fitness)
int goal_x;
int goal_y;

// Simulerar och loggar en population
void simulate_population_batch(Individual population[POP_SIZE], int **maze, int width, int height, int generation, int *id_counter) {
    srand(time(NULL));
    int active_count = 5;
    int max_steps = 1000;

    // Initiera varje individ
    for(int i = 0; i < POP_SIZE; i++) {
        initialize_robot(&population[i].robot, 1.0f, 1.0f);
        initialize_chromosome(&population[i].chromosome);
        population[i].active = 1;
        population[i].fitness = 0;
        population[i].steps_taken = 0;
        population[i].id = (*id_counter)++;
        population[i].generation = generation;
        population[i].is_best = 0;
    }

    // Simulera individer
    for(int step = 0; step < max_steps && active_count > 0; step++) {
        for(int i = 0; i < POP_SIZE; i++) {
            if(!population[i].active) continue;

            Action action = decide_action(&population[i], maze);
            bool success = execute_action(&population[i].robot, action, maze);
            population[i].steps_taken++;

            if(!success || reached_goal(&population[i].robot) || step >= max_steps) {
                population[i].fitness = calculate_fitness(&population[i], step);
                population[i].active = 0;
                active_count--;
            }
        }
    }

    // Hitta bästa individ
    float best_fitness = population[0].fitness;
    int best_index = 0;
    for (int i = 1; i < POP_SIZE; i++) {
        if (population[i].fitness > best_fitness) {
            best_fitness = population[i].fitness;
            best_index = i;
        }
    }
    population[best_index].is_best = 1;
    // här borde mutaion och crossover steget läggas in
   

    // Generera unik loggfil för denna generation
    char log_filename[64];
    sprintf(log_filename, "robot_gen_%03d.csv", generation);

    // Logga alla individer
    for (int i = 0; i < POP_SIZE; i++) {
        log_robot_to_file(&population[i], log_filename);
    }

    // Logga maze
    log_maze_to_file(maze, width, height, "maze_log.txt");

    // Debug-output
    int reached = 0;
    for (int i = 0; i < POP_SIZE; i++) {
        if (reached_goal(&population[i].robot)) reached++;
    }
    printf("Generation %d: %d av individerna nådde målet\n", generation, reached);

}

// Huvudprogram
int main() {
    int id_counter = 0;
    int generations = 50;
    Individual new_population[POP_SIZE];
    Individual elite;
    int use_elite = load_best_individual_from_file(&elite, "robot_gen_049.csv");


    for (int generation = 0; generation < generations; generation++) {
        int **maze = generate_labyrinth(COMPLEX, &maze_width, &maze_height);//första värdet bestämer typen
        set_random_goal_on_edge(maze, maze_width, maze_height, &goal_x, &goal_y);
        maze[goal_y][goal_x] = -1;
        
        Individual population[POP_SIZE];

        // Initiera population
        for (int i = 0; i < p; i++) {
            if (generation == 0 && i == 0 && use_elite) {
                population[i] = elite;
                population[i].id = id_counter++;
                population[i].generation = 0;
                initialize_robot(&population[i].robot, 1.0f, 1.0f);
            } else {
                initialize_robot(&population[i].robot, 1.0f, 1.0f);
                initialize_chromosome(&population[i].chromosome);
                population[i].id = id_counter++;
                population[i].generation = generation;
            }
        }

        simulate_population_batch(population, maze, maze_width, maze_height, generation, &id_counter);

        elite_selection(population, new_population, generation, &id_counter);
        for (int i = 0; i < POP_SIZE; i++) {
        population[i] = new_population[i];
        }

        free_matrix(maze, maze_height);
    }

    return 0;
}