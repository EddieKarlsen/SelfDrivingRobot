#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "types.h"
#include "robot.h"
#include <stdio.h>

// Skriv ut grundläggande status för en individ
void debug_print_individual(const Individual *ind, int step);
void debug_print_maze(int **maze, int width, int height);

#endif