#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "../include/configuration.h"
#include "../include/robot.h"
#include "../include/maze.h"
#include "../include/chromosome.h"
#include "../include/rotation.h"
#include "../include/logger.h"
#include "../include/types.h"
#include "../include/debugger.h"
#include "../include/tracking.h"


//help functions

static void setup_next_maze_phase(Simulationcontext *context, int target_phase,
                                  LabyrinthType *training_sequence, const char **phase_names, int num_phases,
                                  int *current_phase, int *generations_in_current_maze);

static void initialize_generation(Simulationcontext *context, Individual *population,
                                   Individual *elite, int use_elite,
                                   int generation, int *id_counter);

static void simulate_generation(Simulationcontext *context, Individual *population,
                                 int *total_goals_reached,
                                 MovementLog movement_logs[POP_SIZE][MAX_STEPS],
                                 int movement_counts[POP_SIZE]) ;

static void log_results(Individual *population,
                        int generation, JsonLogger *logger,
                        float *phase_best_fitness, int current_phase,
                        int total_goals_reached,
                        MovementLog movement_logs[POP_SIZE][MAX_STEPS],
                        int movement_counts[POP_SIZE]);

static void evolve_population(Individual *population, Individual *new_population,
                               int generation, int *id_counter);

void simulate_population_batch(Simulationcontext *context, Individual *elite, int use_elite) {
    int id_counter = 0;
    int start_generation = 0;
    int generations = NUM_GENERATIONS;
    Individual new_population[POP_SIZE];
    Individual population[POP_SIZE];
    MovementLog movement_logs[POP_SIZE][MAX_STEPS];
    int movement_counts[POP_SIZE];

    srand(time(NULL));
    // Init counters from previous session
    initialize_counters_from_file("robot_log.json", &start_generation, &id_counter);
    
    // Adjust the number of generations based on where we start
    int remaining_generations = generations - start_generation;
    if (remaining_generations <= 0) {
        printf("All generations already completed! (Target: %d, Last: %d)\n", 
               generations, start_generation - 1);
        return;
    }
    
    printf("Will run %d more generations (from %d to %d)\n", 
           remaining_generations, start_generation, start_generation + remaining_generations - 1);

    JsonLogger *json_logger = init_json_logger("robot_log.json");
    if (!json_logger) {
        printf("Warning: Could not initialize JSON logger\n");
    }

    LabyrinthType training_sequence[] = {OPEN, MEDIUM, COMPLEX, NARROW};
    int num_phases = sizeof(training_sequence) / sizeof(LabyrinthType);
    const char* phase_names[] = {"OPEN (Easy)", "MEDIUM", "COMPLEX", "NARROW (Hard)"};

    int current_phase = -1;
    int generations_in_current_maze = 0;
    float phase_best_fitness[10] = {0};
    int total_goals_reached = 0;
    

    int current_training_phase = -1;
    int generations_in_training_phase = 0;
    const int GENERATIONS_PER_PHASE = PHASES_PER_GENERATION;

    for (int generation = start_generation; generation < start_generation + remaining_generations; generation++) {
        
        int target_training_phase;
        if (generation < num_phases * GENERATIONS_PER_PHASE) {
            target_training_phase = generation / GENERATIONS_PER_PHASE;
        } else {

            if (current_training_phase == -1 || generations_in_training_phase >= GENERATIONS_PER_PHASE) {
                target_training_phase = rand() % num_phases;
                generations_in_training_phase = 0;
                printf("Switching to random phase selection: %s\n", phase_names[target_training_phase]);
            } else {
                target_training_phase = current_training_phase;
            }
        }
        
        // when training phase change show the previous phase information
        if (current_training_phase != target_training_phase) {
            if (current_training_phase != -1) {
                printf("Training phase %d (%s) completed after %d generations. Best fitness: %.2f\n",
                       current_training_phase, 
                       phase_names[current_training_phase], 
                       generations_in_training_phase,
                       phase_best_fitness[current_training_phase]);
            }
            current_training_phase = target_training_phase;
            generations_in_training_phase = 0;
            printf("Starting training phase %d (%s) at generation %d\n", 
                   current_training_phase, phase_names[current_training_phase], generation);
        }
        generations_in_training_phase++;
        
        int target_phase = target_training_phase;
        
        init_maze_id_counter("maze_log.txt");
        setup_next_maze_phase(context, target_phase, training_sequence, phase_names, num_phases, &current_phase,
                              &generations_in_current_maze);
        if (json_logger) {
        const char* maze_type_name = phase_names[current_phase];
        log_generation_start(json_logger, generation, maze_type_name, context);
        }   
        generations_in_current_maze++;
        initialize_generation(context, population, elite, use_elite, generation, &id_counter);

        simulate_generation(context, population, &total_goals_reached, movement_logs, movement_counts);

        log_results(population, generation, json_logger,
                    phase_best_fitness, current_phase, total_goals_reached,
                    movement_logs, movement_counts);
        
        if (generation < start_generation + remaining_generations - 1) {
            evolve_population(population, new_population, generation, &id_counter);
        }
        
        // show traning information every 25:th generation
        if ((generation + 1) % GENERATIONS_PER_PHASE == 0) {
            printf("\n=== TRAINING PROGRESS SUMMARY ===\n");
            printf("Completed %d generations in phase %d (%s)\n", 
                   GENERATIONS_PER_PHASE, current_training_phase, phase_names[current_training_phase]);
            printf("Phase best fitness: %.2f\n", phase_best_fitness[current_training_phase]);
            printf("Total goals reached so far: %d\n", total_goals_reached);
            printf("===================================\n\n");
        }
    }

    if (json_logger) {
        close_json_logger(json_logger);
    }

    if (context->maze) {
        free_matrix(context->maze, context->maze_height);
    }
    
    printf("\n=== FINAL TRAINING SUMMARY ===\n");
    printf("Simulation completed! Final generation: %d\n", start_generation + remaining_generations - 1);
    for (int i = 0; i < num_phases; i++) {
        printf("Phase %d (%s): Best fitness %.2f\n", i, phase_names[i], phase_best_fitness[i]);
    }
    printf("Total goals reached: %d\n", total_goals_reached);
    printf("==============================\n");
}

//Help functions 

static void read_sensors(Individual *ind, Simulationcontext *ctx, float readings[5]) {
    for (int s = 0; s < 5; s++) {
        readings[s] = simulate_ultrasonic(ind, s, ctx);
    }
}

static void log_step(MovementLog *log, const Individual *ind, Action action,
                     const float readings[5], int step) {
    log->x = ind->robot.x;
    log->y = ind->robot.y;
    log->angle = ind->robot.angle;
    log->step = step;
    log->action = action;
    for (int s = 0; s < 5; s++) {
        log->sensor_readings[s] = readings[s];
    }
}

static void update_individual(Simulationcontext *ctx, Individual *ind, int step,
                               MovementLog *movement_log, int *movement_count,
                               int *total_goals_reached) {
    float readings[5];
    read_sensors(ind, ctx, readings);

    Action action = decide_action(ind, ctx);
    log_step(&movement_log[(*movement_count)++], ind, action, readings, step);

    bool success = execute_action(ind, action, ctx);
    if (reached_goal(&ind->robot, ctx)) {
        ind->reached_goal = true;
        (*total_goals_reached)++;
    }

    ind->steps_taken++;

    if (!success || ind->reached_goal || step >= MAX_STEPS - 1) {
        ind->fitness = calculate_fitness(ind, ind->steps_taken, ctx);
        if (ind->fitness <= 0) ind->fitness = 1.0f;
        ind->active = 0;
    }
}

static void setup_next_maze_phase(Simulationcontext *context, int target_phase,
                                  LabyrinthType *training_sequence, const char **phase_names, int num_phases,
                                  int *current_phase, int *generations_in_current_maze) {
    
    // logic for generating new mazes when a new training phase starts
    if (*current_phase != target_phase || context->maze == NULL) {
        if (context->maze) {
            free_matrix(context->maze, context->maze_height);
        }
        
        *current_phase = target_phase;
        LabyrinthType maze_type = training_sequence[*current_phase % num_phases];
        *generations_in_current_maze = 0;

        context->maze = generate_labyrinthe(maze_type, &context->maze_width, 
                                            &context->maze_height, context);
        if (!context->maze) {
            printf("ERROR: Failed to generate maze for phase %d (%s)\n", 
                   *current_phase, phase_names[*current_phase]);
            return;
        }
    }
}

static void initialize_generation(Simulationcontext *context, Individual *population,
                                   Individual *elite, int use_elite,
                                   int generation, int *id_counter) {
    for (int i = 0; i < POP_SIZE; i++) {
        if (generation == 0 && i == 0 && use_elite && elite != NULL) {
            population[i] = *elite;
            initialize_robot(&population[i].robot, (float)context->start_x, (float)context->start_y);
            printf("Using elite individual as starter\n");
        } else {
            initialize_robot(&population[i].robot, (float)context->start_x, (float)context->start_y);
            initialize_chromosome(&population[i].chromosome);
        }
        population[i].id = (*id_counter)++;
        population[i].generation = generation;
        population[i].reached_goal = false;
        population[i].collision_count = 0;
        population[i].active = 1;
        population[i].fitness = 0;
        population[i].steps_taken = 0;
        population[i].is_best = 0;
    }
}

static void simulate_generation(Simulationcontext *context, Individual *population,
                                 int *total_goals_reached,
                                 MovementLog movement_logs[POP_SIZE][MAX_STEPS],
                                 int movement_counts[POP_SIZE]) 
{
    // Init movment log
    for (int i = 0; i < POP_SIZE; i++) {
        movement_counts[i] = 0;
    }

    int active_count = POP_SIZE;
    for (int step = 0; step < MAX_STEPS && active_count > 0; step++) {
        for (int i = 0; i < POP_SIZE; i++) {
            if (!population[i].active) continue;

            update_individual(context, &population[i], step,
                              movement_logs[i], &movement_counts[i],
                              total_goals_reached);

            if (!population[i].active) {
                active_count--;
            }
        }
    }
}

static void log_results(Individual *population,
                        int generation, JsonLogger *logger,
                        float *phase_best_fitness, int current_phase,
                        int total_goals_reached,
                        MovementLog movement_logs[POP_SIZE][MAX_STEPS],
                        int movement_counts[POP_SIZE]) 
{
    int best_index = find_best_index(population);
    float best_fitness = population[best_index].fitness;
    population[best_index].is_best = 1;

    if (best_fitness > phase_best_fitness[current_phase]) {
        phase_best_fitness[current_phase] = best_fitness;
    }

    if (logger) {
        for (int i = 0; i < POP_SIZE; i++) {
            log_individual_complete(logger, &population[i],
                                    movement_logs[i], movement_counts[i]);
        }
    }

    int reached = 0;
    float avg_fitness = 0;
    for (int i = 0; i < POP_SIZE; i++) {
        if (population[i].reached_goal) reached++;
        avg_fitness += population[i].fitness;
    }
    avg_fitness /= POP_SIZE;

    if (logger) {
        log_generation_end(logger, reached, avg_fitness,
                           best_fitness, population[best_index].id);
    }

    printf("Generation %d results: Reached %d/%d (Total: %d), Best: %.2f, Avg: %.2f\n",
           generation, reached, POP_SIZE, total_goals_reached,
           best_fitness, avg_fitness);
}

static void evolve_population(Individual *population, Individual *new_population,
                               int generation, int *id_counter) {
    elite_selection(population, new_population, generation, id_counter);
    for (int i = 0; i < POP_SIZE; i++) {
        population[i] = new_population[i];
    }
}