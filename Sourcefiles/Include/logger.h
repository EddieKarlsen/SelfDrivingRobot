#ifndef LOGGER_H
#define LOGGER_H

#include "types.h"
#include <stdio.h>

typedef struct {
    FILE *file;
    int first_entry;
    bool first_generation; 
    int generation_count;
} JsonLogger;

// Huvudfunktioner
JsonLogger* init_json_logger(const char *filename);
void close_json_logger(JsonLogger *logger);

// Loggning
void log_generation_start(JsonLogger *logger, int generation, const char *maze_type, 
                         Simulationcontext *context);
void log_individual_complete(JsonLogger *logger, Individual *ind, 
                           MovementLog *movements, int movement_count);
void log_generation_end(JsonLogger *logger, int goals_reached, float avg_fitness, 
                       float best_fitness, int best_individual_id);
void save_maze_to_log(int maze_id, int **maze, int width, int height,
                      const char *maze_type, int clear_percent,
                      const char *filename);

// Hjälpfunktioner
void write_maze(FILE *file, int **maze, int width, int height);
void write_chromosome(FILE *file, Chromosome *chr);
void write_movements(FILE *file, MovementLog *movements, int count);

// Läsning
Individual* load_best_individual_from_file(const char *filename, int *found);
int load_best_individual_from_file_wrapper(Individual *dest, const char *filename);

// NYA FUNKTIONER för att läsa generation och ID counters
int get_last_generation_from_file(const char *filename);
int get_last_individual_id_from_file(const char *filename);
void initialize_counters_from_file(const char *filename, int *start_generation, int *id_counter);

#endif