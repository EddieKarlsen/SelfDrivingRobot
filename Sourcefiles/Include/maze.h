#ifndef MAZE_H
#define MAZE_H
#include <stdbool.h>
#include "types.h"

typedef struct {
    int width, height;
    int **grid;  // 0 = fri väg, 1 = vägg
    int start_x, start_y;
    int goal_x, goal_y;
} Maze;

typedef enum {
    SIMPLE,
    MEDIUM,
    COMPLEX,
    OPEN,
    NARROW
} LabyrinthType;

// Funktionsdeklarationer
int **create_matrix(int rows, int cols);
void free_matrix(int **matrix, int rows);
int **generate_labyrinthe(LabyrinthType type, int *width, int *height, 
                          Simulationcontext *context);
void place_goal_on_edge(int **maze, int width, int height, Simulationcontext *context);
void place_start(int **maze, int width, int height, Simulationcontext *context);
bool is_maze_solvable(int **maze, int width, int height, int start_x, int start_y, 
                      int goal_x, int goal_y);
void init_maze_id_counter(const char *filename);
int get_next_maze_id();
#endif