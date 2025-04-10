#ifndef CONFIG_H
#define CONFIG_H
#include<sys/types.h>

enum {
    READY_TO_PLAY,
    JUMP_START_ID,
    PULL_START_ID,
    JUMP_END_ID,
    PULL_END_ID,
    ENERGY_ID
};

typedef struct {
    int max_score;
    int max_time;
    int initial_energy_max[4];
    int initial_energy_min[4];
    int rate_of_decrease_min;
    int rate_of_decrease_max;
    int re_join_time_min;
    int re_join_time_max;
    int win_threshold;
} Config;

typedef struct {
    int team_id;
    int score;
    int win_counter;
    int player_id[4];
    int initial_energy[4];
    pid_t players[4];
} Team;


typedef struct {
    int type;
    int player_id;
    pid_t player_pid;
    int team_id;
    char content[20];
} Message;

typedef struct {
    int current_effort_team_1[2][4];
    int current_effort_team_2[2][4];
    int score_team_1;
    int score_team_2;
} Opengl_Message;



int load_config(const char *filename, Config *config);

#endif
