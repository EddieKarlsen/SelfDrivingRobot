#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

void init_robot_log_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "id,generation,fitness,steps_taken,reached_goal,is_best,");
    fprintf(f, "w1,w2,w3,w4,w5,t1,t2,t3,a1,a2,a3,a4,turn_aggr,collision_avoid\n");

    fclose(f);
}

#endif