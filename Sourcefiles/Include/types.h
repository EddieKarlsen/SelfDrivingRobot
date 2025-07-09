#ifndef TYPES_H
#define TYPES_H

typedef struct Chromosome Chromosome;
typedef struct Individual Individual;
typedef struct Robot Robot;

typedef enum {
    FORWARD,
    TURN_LEFT_45,
    TURN_RIGHT_45,
    BACKWARD
} Action;

#endif