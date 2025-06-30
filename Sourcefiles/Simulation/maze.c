#include <stdlib.h>
#include <time.h>
#include "../include/configuration.h"
#include "../include/maze.h"

int **create_matrix(int rows, int cols) {
    int **matrix = malloc(rows * sizeof(int*));
    for(int i = 0; i < rows; i++) {
        matrix[i] = malloc(cols * sizeof(int));
    }
    return matrix;
}

void free_matrix(int **matrix, int rows) {
    for(int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Enklare "fylld med väggar" + slumputskärning
void carve_random_paths(int **maze, int width, int height, int clear_chance_percent) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            maze[y][x] = (rand() % 100 < clear_chance_percent) ? 0 : 1;
        }
    }
}

// Generera labyrint baserat på typ
int **generate_labyrinth(LabyrinthType type, int *width, int *height) {
    srand(time(NULL));

    int w, h, clear_percent;

    switch (type) {
        case SIMPLE:
            w = h = 5;
            clear_percent = 70;
            break;
        case MEDIUM:
            w = h = 8;
            clear_percent = 60;
            break;
        case COMPLEX:
            w = h = 12;
            clear_percent = 40;
            break;
        case OPEN:
            w = h = 10;
            clear_percent = 85;
            break;
        case NARROW:
            w = h = 10;
            clear_percent = 30;
            break;
        default:
            w = h = 8;
            clear_percent = 50;
            break;
    }

    *width = w;
    *height = h;

    int **maze = create_matrix(h, w);

    // Sätt yttre väggar
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            maze[y][x] = (x == 0 || y == 0 || x == w - 1 || y == h - 1) ? 1 : 0;
        }
    }

    // Slumpmässigt skära ut vägar
    carve_random_paths(maze, w, h, clear_percent);

    // Start och mål öppna kanske borde sätta start o mål till andra värden så bet vissas tyddligare vart den börja och vart målet är
    maze[1][1] = 0;
    maze[h - 2][w - 2] = 0;

    return maze;
}

void set_random_goal_on_edge(int **maze, int width, int height, int *gx, int *gy) {
    int x, y;
    int side;
    do {
        side = rand() % 4;
        switch (side) {
            case 0: // Topp
                x = rand() % width;
                y = 0;
                break;
            case 1: // Botten
                x = rand() % width;
                y = height - 1;
                break;
            case 2: // Vänster
                x = 0;
                y = rand() % height;
                break;
            case 3: // Höger
                x = width - 1;
                y = rand() % height;
                break;
        }
    } while (maze[y][x] == 1);  // Upprepa tills det är en fri plats

    *gx = x;
    *gy = y;
}

