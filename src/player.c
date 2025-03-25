#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>

#include "../include/config.h"


Config config;
int NUM_PLAYERS_PER_TEAM=4;

void handle_signal(int sig);
int generate_energy(int player_id);

player_state* player_init(team_id team, int player_num, player_config config);
void player_process(team_id team, int player_num, player_config config);

int fd;

int main (int argc, char **argv){

     if (argc < 2) {
        fprintf(stderr, "Usage: %s <fifo_name>\n", argv[0]);
        return EXIT_FAILURE;
    }
   if (load_config(argv[1], &config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
    }

    


     fd = open(argv[2], O_WRONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
	
    
    write(fd, message, strlen(message));

    close(fd);
    return 0;
}

int generate_energy(int player_id){
    srand(time(NULL) + player_id);
    int energy = config.initial_energy_min[player_id] + 
                 (rand() % (config.initial_energy_max[player_id] - config.initial_energy_min[player_id] + 1));

    return energy;
}

















void handle_signal(int sig){

}

player_state* player_init(team_id team, int player_num, player_config config) {
    player_state* player = malloc(sizeof(player_state));
    if (!player) {
        perror("Failed to allocate player state");
        exit(EXIT_FAILURE);
    }

    // Set basic info
    player->team = team;
    player->player_num = player_num;
    player->config = config;

    // Initialize random seed
    srand(time(NULL) ^ (getpid() << 16));

    // Set initial random energy
    player->energy = config.min_energy +
                    (rand() % (config.max_energy - config.min_energy + 1));

    // Set random energy decrease rate
    player->decrease_rate = config.min_decrease +
                           (rand() % (config.max_decrease - config.min_decrease + 1));

    // Initial position (will be updated when game starts)
    player->position = 0;

    // Set up signal handlers
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction SIGUSR1");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction SIGUSR2");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(EXIT_FAILURE);
    }

    // Create pipe for communication with referee
    if (pipe(player->pipe_fd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Notify referee that initialization is complete
    // (Implementation depends on your communication protocol)
    send_init_ack();

    return player;
}

void player_process(team_id team, int player_num, player_config config) {
    printf("Player %d of team %d starting...\n", player_num, team);

    // Initialize player state
    player_state* player = player_init(team, player_num, config);

    // Main game loop would go here
    // ...

    // Cleanup (would be called on termination)
    free(player);
}
void printConfig(const Config *cfg) {
    printf("\n=== Config Values ===\n");
    printf("Max Score: %d\n", cfg->max_score);
    printf("Max Time: %d\n", cfg->max_time);
}

