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

void handle_getready();
int generate_energy(int id);


int fd;
int player_id,team_id;
pid_t player;



int main (int argc, char **argv){


   if (argc < 5) {
        fprintf(stderr, "Usage: %s <fifo_name>\n", argv[0]);
        return EXIT_FAILURE;
   }
   if (load_config(argv[1], &config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
   }

   player_id=atoi(argv[3]);
   team_id=atoi(argv[4]);
   
    fd = open(argv[2], O_WRONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    


    int energy = generate_energy(player_id);
    printf("Player %d (Team %d) initial energy: %d\n", player_id, team_id, energy);

    // Create message structure
    Message msg;
    msg.type = 1; // Type 1: Energy message
    msg.player_id = player_id;
    msg.team_id = team_id;
    msg.player_pid = getpid();
    snprintf(msg.content, sizeof(msg.content), "%d", energy);

    // Send message through FIFO
    if (write(fd, &msg, sizeof(Message)) == -1) {
        perror("write");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Initial energy sent successfully.\n");
    
    
    signal(SIGUSR1,handle_getready());
    
    
    

    close(fd);
    return 0;

}


int generate_energy(int id) {
    srand(time(NULL) ^ getpid());  // Use getpid() to introduce more entropy
    int energy = config.initial_energy_min[id] +
                 (rand() % (config.initial_energy_max[id] - config.initial_energy_min[id] + 1));
    return energy;
}



void handle_getready(){


}

void printConfig(const Config *cfg) {
    printf("\n=== Config Values ===\n");
    printf("Max Score: %d\n", cfg->max_score);
    printf("Max Time: %d\n", cfg->max_time);
}

