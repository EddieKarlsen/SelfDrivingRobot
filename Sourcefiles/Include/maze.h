#ifndef MAZE_H
#define MAZE_H


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

int **create_matrix(int rows, int cols);
void free_matrix(int **matrix, int rows);
int **generate_labyrinth(LabyrinthType type, int *width, int *height);
extern int goal_x, goal_y;
#endif