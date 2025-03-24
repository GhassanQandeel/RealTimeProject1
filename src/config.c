//
// Created by ghassan on 3/22/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"

int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open configuration file");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        char key[50];
        int value;
        if (sscanf(line, "%49[^=]=%d", key, &value) == 2) {
            if (strcmp(key, "max_score") == 0) config->max_score = value;
            else if (strcmp(key, "max_time") == 0) config->max_time = value;
            else if (strcmp(key, "initial_energy_max_a") == 0) config->initial_energy_max[0] = value;
            else if (strcmp(key, "initial_energy_min_a") == 0) config->initial_energy_min[0] = value;
            else if (strcmp(key, "initial_energy_max_b") == 0) config->initial_energy_max[1] = value;
            else if (strcmp(key, "initial_energy_min_b") == 0) config->initial_energy_min[1] = value;
            else if (strcmp(key, "initial_energy_max_c") == 0) config->initial_energy_max[2] = value;
            else if (strcmp(key, "initial_energy_min_c") == 0) config->initial_energy_min[2] = value;
            else if (strcmp(key, "initial_energy_max_d") == 0) config->initial_energy_max[3] = value;
            else if (strcmp(key, "initial_energy_min_d") == 0) config->initial_energy_min[3] = value;
            else if (strcmp(key, "rate_of_decrease_min") == 0) config->rate_of_decrease_min = value;
            else if (strcmp(key, "rate_of_decrease_max") == 0) config->rate_of_decrease_max = value;
            else if (strcmp(key, "re_join_time_min") == 0) config->re_join_time_min = value;
            else if (strcmp(key, "re_join_time_max") == 0) config->re_join_time_max = value;
            else if (strcmp(key, "win_threshold") == 0) config->win_threshold = value;

        }
    }
    fclose(file);
    return 0;
}