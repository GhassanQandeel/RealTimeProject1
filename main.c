#include <stdio.h>
#include "include/config.h"
void printConfig(const Config *cfg) {
    printf("\n=== Config Values ===\n");
    printf("Max Score: %d\n", cfg->max_score);
    printf("Max Time: %d\n", cfg->max_time);

    printf("Initial Energy Max: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", cfg->initial_energy_max[i]);
    }
    printf("\n");

    printf("Initial Energy Min: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", cfg->initial_energy_min[i]);
    }
    printf("\n");

    printf("Rate of Decrease Min: %d\n", cfg->rate_of_decrease_min);
    printf("Rate of Decrease Max: %d\n", cfg->rate_of_decrease_max);
    printf("Rejoin Time Min: %d\n", cfg->re_join_time_min);
    printf("Rejoin Time Max: %d\n", cfg->re_join_time_max);
    printf("Win Threshold: %d\n", cfg->win_threshold);
    printf("=====================\n");
}
int main(void) {
    Config config;

    int x = load_config("../config.txt", &config);
    printConfig(&config);


    return 0;
}
