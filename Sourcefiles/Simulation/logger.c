#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h> 
#include <sys/stat.h>

#include "../include/types.h"
#include "../include/logger.h"
#include "../include/chromosome.h"
#include "../include/configuration.h"
#include "../include/robot.h"

static void truncate_file(FILE *f, long pos) {
    fflush(f);
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(f));
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER li;
        li.QuadPart = pos;
        SetFilePointerEx(hFile, li, NULL, FILE_BEGIN);
        SetEndOfFile(hFile);
    }
}

JsonLogger* init_json_logger(const char *filename) {
    JsonLogger *logger = malloc(sizeof(JsonLogger));
    if (!logger) return NULL;

    FILE *f = fopen(filename, "rb+");
    if (f) {
        // File exists → append mode
        fseek(f, 0, SEEK_END);
        long size = ftell(f);

        // Find the last ']' in the generations list
        long pos = size - 1;
        int ch;
        while (pos > 0) {
            fseek(f, pos, SEEK_SET);
            ch = fgetc(f);
            if (ch == ']') break;
            pos--;
        }

        // Truncate after ']' and add comma for new entries
        truncate_file(f, pos);
        fseek(f, 0, SEEK_END);
        fputs(",\n", f);

        logger->file = f;
        logger->first_entry = false;
        logger->first_generation = false; // Initialize this flag
        printf("Appending to existing JSON log: %s\n", filename);

    } else {
        // New file
        f = fopen(filename, "wb");
        if (!f) {
            free(logger);
            return NULL;
        }
        fprintf(f, "{\n");
        fprintf(f, "  \"simulation_info\": {\n");
        fprintf(f, "    \"version\": \"1.0\",\n");
        fprintf(f, "    \"created\": \"2024\"\n");
        fprintf(f, "  },\n");
        fprintf(f, "  \"generations\": [\n");

        logger->file = f;
        logger->first_entry = true;
        logger->first_generation = true; // Initialize this flag
        printf("Created new JSON log: %s\n", filename);
    }

    return logger;
}

void close_json_logger(JsonLogger *logger) {
    if (!logger || !logger->file) return;
    fputs("\n  ]\n}\n", logger->file);
    fclose(logger->file);
    free(logger);
}

void log_generation_start(JsonLogger *logger, int generation, const char *maze_type,
                         Simulationcontext *context) {
    if (!logger || !logger->file) return;
    
    // Add comma before new generation (except for first)
    if (!logger->first_generation) {
        fprintf(logger->file, ",\n");
    }
    logger->first_generation = false;
    
    fprintf(logger->file, "    {\n");
    fprintf(logger->file, "      \"generation\": %d,\n", generation);
    fprintf(logger->file, "      \"maze_info\": {\n");
    fprintf(logger->file, "        \"type\": \"%s\",\n", maze_type);
    fprintf(logger->file, "        \"width\": %d,\n", context->maze_width);
    fprintf(logger->file, "        \"height\": %d,\n", context->maze_height);
    fprintf(logger->file, "        \"start\": [%d, %d],\n", context->start_x, context->start_y);
    fprintf(logger->file, "        \"goal\": [%d, %d],\n", context->goal_x, context->goal_y);
    fprintf(logger->file, "        \"layout\": ");
    write_maze(logger->file, context->maze, context->maze_width, context->maze_height);
    fprintf(logger->file, "\n      },\n");
    fprintf(logger->file, "      \"individuals\": [\n");
    
    logger->generation_count = 0;
    fflush(logger->file);
}

void log_individual_complete(JsonLogger *logger, Individual *ind,
                           MovementLog *movements, int movement_count) {
    if (!logger || !logger->file) return;
    
    // Add comma before individual (except first)
    if (logger->generation_count > 0) {
        fprintf(logger->file, ",\n");
    }
    logger->generation_count++;
    
    // Calculate distance to goal (you might want to implement this properly)
    float distance_to_goal = 0.0f; // Placeholder
    
    fprintf(logger->file, "        {\n");
    fprintf(logger->file, "          \"id\": %d,\n", ind->id);
    fprintf(logger->file, "          \"fitness\": %.3f,\n", ind->fitness);
    fprintf(logger->file, "          \"steps_taken\": %d,\n", ind->steps_taken);
    fprintf(logger->file, "          \"reached_goal\": %s,\n", ind->reached_goal ? "true" : "false");
    fprintf(logger->file, "          \"is_best\": %s,\n", ind->is_best ? "true" : "false");
    fprintf(logger->file, "          \"collision_count\": %d,\n", ind->collision_count);
    fprintf(logger->file, "          \"final_position\": {\n");
    fprintf(logger->file, "            \"x\": %.3f,\n", ind->robot.x);
    fprintf(logger->file, "            \"y\": %.3f,\n", ind->robot.y);
    fprintf(logger->file, "            \"angle\": %.3f,\n", ind->robot.angle);
    fprintf(logger->file, "            \"distance_to_goal\": %.3f\n", distance_to_goal);
    fprintf(logger->file, "          },\n");
    fprintf(logger->file, "          \"chromosome\": ");
    write_chromosome(logger->file, &ind->chromosome);
    
    if (movements && movement_count > 0) {
        fprintf(logger->file, ",\n          \"movements\": ");
        write_movements(logger->file, movements, movement_count);
    }
    
    fprintf(logger->file, "\n        }");
    fflush(logger->file);
}

void log_generation_end(JsonLogger *logger, int goals_reached, float avg_fitness,
                        float best_fitness, int best_individual_id) {
    if (!logger || !logger->file) return;

    fprintf(logger->file, "\n      ],\n");
    fprintf(logger->file, "      \"generation_stats\": {\n");
    fprintf(logger->file, "        \"goals_reached\": %d,\n", goals_reached);
    fprintf(logger->file, "        \"avg_fitness\": %.3f,\n", avg_fitness);
    fprintf(logger->file, "        \"best_fitness\": %.3f,\n", best_fitness);
    fprintf(logger->file, "        \"best_individual_id\": %d\n", best_individual_id);
    fprintf(logger->file, "      }\n");
    fprintf(logger->file, "    }");  // NOTE: No trailing comma here!
    
    fflush(logger->file);
}

void write_maze(FILE *f, int **maze, int width, int height) {
    fprintf(f, "[\n");
    for (int y = 0; y < height; y++) {
        fprintf(f, "  [");
        for (int x = 0; x < width; x++) {
            char cell;
            switch (maze[y][x]) {
                case WALL:   cell = '#'; break;
                case BORDER: cell = 'B'; break;
                case START:  cell = 'S'; break;
                case GOAL:   cell = 'G'; break;
                case EMPTY:  cell = 'O'; break;
                default:     cell = '?'; break;
            }
            fprintf(f, "\"%c\"", cell);
            if (x < width - 1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        if (y < height - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    fprintf(f, "]\n");
}

void write_chromosome(FILE *file, Chromosome *chr) {
    fprintf(file, "{\n");
    fprintf(file, "            \"sensor_weights\": [");
    for (int i = 0; i < 5; i++) {
        fprintf(file, "%.3f", chr->sensor_weights[i]);
        if (i < 4) fprintf(file, ", ");
    }
    fprintf(file, "],\n");
    
    fprintf(file, "            \"distance_thresholds\": [");
    for (int i = 0; i < 3; i++) {
        fprintf(file, "%.3f", chr->distance_thresholds[i]);
        if (i < 2) fprintf(file, ", ");
    }
    fprintf(file, "],\n");
    
    fprintf(file, "            \"action_priorities\": [");
    for (int i = 0; i < 4; i++) {
        fprintf(file, "%.3f", chr->action_priorities[i]);
        if (i < 3) fprintf(file, ", ");
    }
    fprintf(file, "],\n");
    
    fprintf(file, "            \"turn_aggressiveness\": %.3f,\n", chr->turn_aggressiveness);
    fprintf(file, "            \"collision_avoidance\": %.3f\n", chr->collision_avoidance);
    fprintf(file, "          }");
}

void write_movements(FILE *file, MovementLog *movements, int count) {
    const char* action_names[] = {"FORWARD", "TURN_LEFT_45", "TURN_RIGHT_45", "BACKWARD"};
    
    fprintf(file, "[\n");
    for (int i = 0; i < count; i++) {
        MovementLog *mov = &movements[i];
        const char* action_name = (mov->action >= 0 && mov->action <= 3) ? 
                                 action_names[mov->action] : "UNKNOWN";
        
        fprintf(file, "            {\n");
        fprintf(file, "              \"step\": %d,\n", mov->step);
        fprintf(file, "              \"position\": [%.3f, %.3f],\n", mov->x, mov->y);
        fprintf(file, "              \"angle\": %.3f,\n", mov->angle);
        fprintf(file, "              \"action\": \"%s\",\n", action_name);
        fprintf(file, "              \"sensors\": [%.2f, %.2f, %.2f, %.2f, %.2f]\n", 
                mov->sensor_readings[0], mov->sensor_readings[1], mov->sensor_readings[2],
                mov->sensor_readings[3], mov->sensor_readings[4]);
        fprintf(file, "            }");
        
        if (i < count - 1) fprintf(file, ",");
        fprintf(file, "\n");
    }
    fprintf(file, "          ]");
}

// Fix for proper JSON generation function
int fix_json_file(const char *filename) {
    printf("Trying to fix JSON structure in %s...\n", filename);
    
    FILE *original = fopen(filename, "r");
    if (!original) {
        printf("Could not open original file\n");
        return 0;
    }
    
    char backup_name[256];
    snprintf(backup_name, sizeof(backup_name), "%s.backup", filename);
    
    FILE *backup = fopen(backup_name, "w");
    if (!backup) {
        fclose(original);
        printf("Could not create backup file\n");
        return 0;
    }
    
    // Copy original to backup
    char ch;
    while ((ch = fgetc(original)) != EOF) {
        fputc(ch, backup);
    }
    fclose(backup);
    fclose(original);
    
    printf("Backup created as %s\n", backup_name);
    printf("You need to manually fix JSON structure or start with a new file\n");
    
    return 1;
}

// Rest of the functions remain the same...
Individual* load_best_individual_from_file(const char *filename, int *found) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        *found = 0;
        return NULL;
    }
    
    Individual *best = malloc(sizeof(Individual));
    if (!best) {
        fclose(file);
        *found = 0;
        return NULL;
    }
    
    float best_fitness = -1e9;
    *found = 0;
    
    char line[2048];
    Individual current;
    int parsing_individual = 0;
    int parsing_chromosome = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "\"id\":")) {
            parsing_individual = 1;
            memset(&current, 0, sizeof(Individual));
            
            char *id_pos = strstr(line, "\"id\":");
            if (id_pos) {
                sscanf(id_pos, "\"id\": %d", &current.id);
            }
        }
        
        if (parsing_individual) {
            if (strstr(line, "\"fitness\":")) {
                char *fitness_pos = strstr(line, "\"fitness\":");
                if (fitness_pos) {
                    sscanf(fitness_pos, "\"fitness\": %f", &current.fitness);
                }
            }
            
            if (strstr(line, "\"steps_taken\":")) {
                char *steps_pos = strstr(line, "\"steps_taken\":");
                if (steps_pos) {
                    sscanf(steps_pos, "\"steps_taken\": %d", &current.steps_taken);
                }
            }
            
            if (strstr(line, "\"reached_goal\":")) {
                char *goal_pos = strstr(line, "\"reached_goal\":");
                if (goal_pos) {
                    if (strstr(goal_pos, "true")) {
                        current.reached_goal = true;
                    } else {
                        current.reached_goal = false;
                    }
                }
            }
            
            if (strstr(line, "\"collision_count\":")) {
                char *coll_pos = strstr(line, "\"collision_count\":");
                if (coll_pos) {
                    sscanf(coll_pos, "\"collision_count\": %d", &current.collision_count);
                }
            }
            
            if (strstr(line, "\"x\":") && strstr(line, "final_position")) {
                char *x_pos = strstr(line, "\"x\":");
                if (x_pos) {
                    sscanf(x_pos, "\"x\": %f", &current.robot.x);
                }
            }
            
            if (strstr(line, "\"y\":") && strstr(line, "final_position")) {
                char *y_pos = strstr(line, "\"y\":");
                if (y_pos) {
                    sscanf(y_pos, "\"y\": %f", &current.robot.y);
                }
            }
            
            if (strstr(line, "\"angle\":") && strstr(line, "final_position")) {
                char *angle_pos = strstr(line, "\"angle\":");
                if (angle_pos) {
                    sscanf(angle_pos, "\"angle\": %f", &current.robot.angle);
                }
            }
            
            if (strstr(line, "\"chromosome\":")) {
                parsing_chromosome = 1;
            }
            
            if (parsing_chromosome) {
                if (strstr(line, "\"sensor_weights\":")) {
                    if (fgets(line, sizeof(line), file)) {
                        char *start = strchr(line, '[');
                        if (start) {
                            sscanf(start, "[%f, %f, %f, %f, %f]",
                                   &current.chromosome.sensor_weights[0],
                                   &current.chromosome.sensor_weights[1],
                                   &current.chromosome.sensor_weights[2],
                                   &current.chromosome.sensor_weights[3],
                                   &current.chromosome.sensor_weights[4]);
                        }
                    }
                }
                
                if (strstr(line, "\"distance_thresholds\":")) {
                    if (fgets(line, sizeof(line), file)) {
                        char *start = strchr(line, '[');
                        if (start) {
                            sscanf(start, "[%f, %f, %f]",
                                   &current.chromosome.distance_thresholds[0],
                                   &current.chromosome.distance_thresholds[1],
                                   &current.chromosome.distance_thresholds[2]);
                        }
                    }
                }
                
                if (strstr(line, "\"action_priorities\":")) {
                    if (fgets(line, sizeof(line), file)) {
                        char *start = strchr(line, '[');
                        if (start) {
                            sscanf(start, "[%f, %f, %f, %f]",
                                   &current.chromosome.action_priorities[0],
                                   &current.chromosome.action_priorities[1],
                                   &current.chromosome.action_priorities[2],
                                   &current.chromosome.action_priorities[3]);
                        }
                    }
                }
                
                if (strstr(line, "\"turn_aggressiveness\":")) {
                    char *aggr_pos = strstr(line, "\"turn_aggressiveness\":");
                    if (aggr_pos) {
                        sscanf(aggr_pos, "\"turn_aggressiveness\": %f", &current.chromosome.turn_aggressiveness);
                    }
                }
                
                if (strstr(line, "\"collision_avoidance\":")) {
                    char *avoid_pos = strstr(line, "\"collision_avoidance\":");
                    if (avoid_pos) {
                        sscanf(avoid_pos, "\"collision_avoidance\": %f", &current.chromosome.collision_avoidance);
                    }
                }
            }
            
            if (strstr(line, "}") && !strstr(line, "\"")) {
                parsing_individual = 0;
                parsing_chromosome = 0;
                
                if (current.fitness > best_fitness) {
                    *best = current;
                    best_fitness = current.fitness;
                    *found = 1;
                }
            }
        }
    }
    
    fclose(file);
    
    if (*found) {
        printf("Loaded best individual: ID=%d, Fitness=%.3f, Steps=%d, Goal=%s\n",
               best->id, best->fitness, best->steps_taken, 
               best->reached_goal ? "YES" : "NO");
        return best;
    } else {
        free(best);
        return NULL;
    }
}

int load_best_individual_from_file_wrapper(Individual *dest, const char *filename) {
    int found = 0;
    Individual *loaded = load_best_individual_from_file(filename, &found);
    if (found && loaded) {
        *dest = *loaded;
        free(loaded);
        return 1;
    }
    return 0;
}

void save_maze_to_log(int maze_id, int **maze, int width, int height,
                      const char *maze_type, int clear_percent,
                      const char *filename) {
    FILE *f = fopen(filename, "a");
    if (!f) return;

    fprintf(f, "{\n");
    fprintf(f, "  \"maze_id\": %d,\n", maze_id);
    fprintf(f, "  \"maze_type\": \"%s\",\n", maze_type);
    fprintf(f, "  \"width\": %d,\n", width);
    fprintf(f, "  \"height\": %d,\n", height);
    fprintf(f, "  \"clear_percent\": %d,\n", clear_percent);
    fprintf(f, "  \"grid\": [\n");

    for (int y = 0; y < height; y++) {
        fprintf(f, "    \"");
        for (int x = 0; x < width; x++) {
            char cell;
            switch (maze[y][x]) {
                case WALL:   cell = '#'; break;
                case BORDER: cell = 'B'; break;
                case START:  cell = 'S'; break;
                case GOAL:   cell = 'G'; break;
                case EMPTY:  cell = 'O'; break;
                default:     cell = '?'; break;
            }
            fputc(cell, f);
        }
        fprintf(f, "\"%s\n", (y < height - 1) ? "," : "");
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");

    fclose(f);
}

int get_last_generation_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    int last_generation = -1;
    char line[1024];
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "\"generation\":")) {
            int gen;
            if (sscanf(line, " \"generation\": %d", &gen) == 1) {
                if (gen > last_generation) {
                    last_generation = gen;
                }
            }
        }
    }
    
    fclose(file);
    
    if (last_generation >= 0) {
        printf("Found previous data, last generation was: %d\n", last_generation);
    } else {
        printf("No valid generation data found in file\n");
    }
    
    return last_generation;
}

int get_last_individual_id_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    
    int last_id = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "\"id\":")) {
            int id;
            if (sscanf(line, " \"id\": %d", &id) == 1) {
                if (id > last_id) {
                    last_id = id;
                }
            }
        }
    }
    
    fclose(file);
    
    printf("Found highest individual ID: %d, next ID will be: %d\n", last_id, last_id + 1);
    return last_id + 1;
}

void initialize_counters_from_file(const char *filename, int *start_generation, int *id_counter) {
    int last_gen = get_last_generation_from_file(filename);
    int next_id = get_last_individual_id_from_file(filename);
    
    if (last_gen >= 0) {
        *start_generation = last_gen + 1;
        printf("Continuing from generation %d\n", *start_generation);
    } else {
        *start_generation = 0;
        printf("Starting fresh simulation from generation 0\n");
    }
    
    *id_counter = next_id;
    printf("Individual ID counter set to: %d\n", *id_counter);
}