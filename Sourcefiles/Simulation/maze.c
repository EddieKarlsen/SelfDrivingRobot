#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/configuration.h"
#include "../include/maze.h"
#include "../include/debugger.h"
#include "../include/logger.h"


static int next_maze_id = 1;

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

void carve_random_paths(int **maze, int width, int height, int clear_chance_percent) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            maze[y][x] = (rand() % 100 < clear_chance_percent) ? EMPTY : WALL;
        }
    }
}

int **generate_labyrinthe(LabyrinthType type, int *width, int *height, 
                          Simulationcontext *context) {
    int max_attempts = MAX_ATTEMPTS;
    const char *type_str;
    int clear_percent;
    
    // parameter decides how much empty space thats in the maze
    switch (type) {
        case SIMPLE:
            clear_percent = 70;
            type_str = "SIMPLE";
            break;
        case MEDIUM:
            clear_percent = 60;
            type_str = "MEDIUM";
            break;
        case COMPLEX:
            clear_percent = 25;
            type_str = "COMPLEX";
            break;
        case OPEN:
            clear_percent = 85;
            type_str = "OPEN";
            break;
        case NARROW:
            clear_percent = 30;
            type_str = "NARROW";
            break;
        default:
            clear_percent = 50;
            type_str = "UNKNOWN";
            break;
    }
    
    for (int attempt = 0; attempt < max_attempts; attempt++) {

        int w = DEFAULT_MAZE_WIDTH, h = DEFAULT_MAZE_HEIGHT;
        *width = w;
        *height = h;

        int **maze = create_matrix(h, w);
        if (!maze) {
            printf("Failed to allocate maze memory\n");
            continue;
        }

        // place exterial walls
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                if (x == 0 || y == 0 || x == w - 1 || y == h - 1) {
                    maze[y][x] = BORDER;
                } else {
                    maze[y][x] = EMPTY;
                }
            }
        }
        
        // carves the paths for the maze
        carve_random_paths(maze, w, h, clear_percent);

        context->maze_width = w;
        context->maze_height = h;

        place_goal_on_edge(maze, w, h, context);
        place_start(maze, w, h, context);
        
        maze[context->start_y][context->start_x] = START;
        
        if (is_maze_solvable(maze, w, h, context->start_x, context->start_y, 
                             context->goal_x, context->goal_y)) {
            
            int id = get_next_maze_id();
            save_maze_to_log(id, maze, w, h, type_str, clear_percent, "maze_log.txt");
            return maze;
        }
        
        // if not solvabel free the maze 
        free_matrix(maze, h);
    }
    
    printf("ERROR: Could not generate solvable maze after %d attempts\n", max_attempts);
    return NULL;
}

void place_goal_on_edge(int **maze, int width, int height, Simulationcontext *context) {
    int x, y;
    int side = rand() % 4;
    
    switch (side) {
        case 0: // Top
            x = 1 + rand() % (width - 2);
            y = 0;
            break;
        case 1: // bottom
            x = 1 + rand() % (width - 2);
            y = height - 1;
            break;
        case 2: // left
            x = 0;
            y = 1 + rand() % (height - 2);
            break;
        case 3: // right
            x = width - 1;
            y = 1 + rand() % (height - 2);
            break;
    }
    
    context->goal_x = x;
    context->goal_y = y;
    maze[y][x] = GOAL;
}

void place_start(int **maze, int width, int height, Simulationcontext *context) {
    int x, y;

    do {
        x = 1 + rand() % (width - 2);
        y = 1 + rand() % (height - 2);
    } while ((x == context->goal_x && y == context->goal_y) || maze[y][x] == WALL);

    context->start_x = x;
    context->start_y = y;
    maze[y][x] = START;
}

bool is_maze_solvable(int **maze, int width, int height, int start_x, int start_y, 
                      int goal_x, int goal_y) {
    if (start_x < 0 || start_x >= width || start_y < 0 || start_y >= height ||
        goal_x < 0 || goal_x >= width || goal_y < 0 || goal_y >= height) {
        return false;
    }

    bool visited[height][width];
    memset(visited, 0, sizeof(visited));

    typedef struct { int x, y; } Point;
    Point queue[width * height];
    int front = 0, rear = 0;

    queue[rear++] = (Point){start_x, start_y};
    visited[start_y][start_x] = true;

    int directions[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    while (front < rear) {
        Point p = queue[front++];
        if (p.x == goal_x && p.y == goal_y) {
            return true;  // goal reached
        }

        for (int i=0; i<4; i++) {
            int nx = p.x + directions[i][0];
            int ny = p.y + directions[i][1];

            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (!visited[ny][nx] && maze[ny][nx] != WALL && maze[ny][nx] != BORDER) {
                    visited[ny][nx] = true;
                    queue[rear++] = (Point){nx, ny};
                }
            }
        }
    }

    return false;  // impossible to reache goal
}



void init_maze_id_counter(const char *filename){

        FILE *f = fopen(filename, "r");
        if (!f) return;

        char line[128];
        int id;
        while (fgets(line, sizeof(line), f)) {
            if (sscanf(line, " \"maze_id\": %d", &id) == 1) {
                if (id >= next_maze_id) {
                    next_maze_id = id + 1;
                }
            }
        }
            fclose(f);
    }

int get_next_maze_id() {
    return next_maze_id++;
}