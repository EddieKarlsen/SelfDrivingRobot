#ifndef CONFIGURATION_H
#define CONFIGURATION_H

//aproximativt värde för pi skilljer sig från det riktiga värdet 0.0000000000000000000841 %
#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

//GA configuration
#define POP_SIZE 50
#define NUM_GENERATIONS 500
#define MAX_STEPS 1000
#define MUTATION_RATE 0.2f
#define BASE_LINE_FITNESS 100

//Maze configuration
#define DEFAULT_MAZE_WIDTH 25
#define DEFAULT_MAZE_HEIGHT 25
#define ROBOT_WIDTH 2.0f
#define ROBOT_HEIGHT 2.0f
#define MAX_ATTEMPTS 1000

// Maze configuration över hur loggningen ser ut
#define WALL '#'         
#define EMPTY 'O'        
#define BORDER 'B'        
#define GOAL  'G'         
#define START 'S'       


//Fitness configuration
#define FITNESS_ALPHA   1.0f
#define FITNESS_BETA    0.5f
#define FITNESS_GAMMA   0.3f
#define FITNESS_DELTA   2.0f
#define FITNESS_EPSILON 10.0f
#endif