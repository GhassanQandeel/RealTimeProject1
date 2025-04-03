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

typedef void (*sighandler_t)(int);
sighandler_t sigset(int sig, sighandler_t disp);

Config config;
int NUM_PLAYERS_PER_TEAM=4;


volatile sig_atomic_t keep_running = 1;  // Global flag to control the loop

void handle_sigint(int sig);
void handle_getready();
void handle_start_game();
void send_message(int message_type,int content);
int generate_energy(int id);


int fd_read, fd_write;
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
   
    fd_write = open(argv[2], O_WRONLY);
    if (fd_write == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

     // Open FIFO for reading (from referee)
     fd_read = open(argv[2], O_RDONLY | O_NONBLOCK);
     if (fd_read == -1) {
         perror("open read FIFO");
         close(fd_write);
         return EXIT_FAILURE;
     }
   
   player_id=atoi(argv[3]);
   team_id=atoi(argv[4]);
   
    


    // here we generate initial energy and send it to referee 
    send_message(1,generate_energy(player_id));

    
    
    
    if ( sigset(SIGUSR1, handle_getready) == SIG_ERR ) {
    perror("Sigset can not set SIGUSR1");
    exit(SIGQUIT);
    }
  
    if ( sigset(SIGUSR2, handle_start_game) == SIG_ERR ) {
    perror("Sigset can not set SIGUSR2");
    exit(SIGQUIT);
    }

    signal(SIGINT, handle_sigint);
    
    while (keep_running) {
        pause();
        // Message msg;
        // int n = read(fd_read, &msg, sizeof(Message));
        
        // if (n > 0) {
        //     if (msg.type == 3) { // Winner message
        //         printf("Player %d: We won the game!\n", player_id);
        //         keep_running = 0;
        //     } 
        //     else if (msg.type == 4) { // Loser message
        //         printf("Player %d: We lost the game.\n", player_id);
        //         keep_running = 0;
        //     }
        // }
    }
    close(fd_read);
    close(fd_write);
    return 0;
}


int generate_energy(int id) {
    srand(time(NULL) ^ getpid());  
    int energy = config.initial_energy_min[id] +
                 (rand() % (config.initial_energy_max[id] - config.initial_energy_min[id] + 1));
    return energy;
}
void send_message(int message_type,int content){
	// Create message structure
    Message msg;
    msg.type = message_type ; 
    msg.player_id = player_id;
    msg.team_id = team_id;
    msg.player_pid = getpid();
    snprintf(msg.content, sizeof(msg.content), "%d", content);

    // Send message through FIFO
    if (write(fd_write, &msg, sizeof(Message)) == -1) {
        perror("write");
        
    }
}



void handle_getready(){
	/* For get ready signal with our implemintion is just for alignment the players in openGl */
		//printf("SIGUSR1 %d \n",getpid());

}
void handle_start_game(){
	// when start game we will do the following 
	/*1- make effort */
	/*2- compare to range of decrease*/
	srand(time(NULL) ^ getpid());  
    	int effort = config.rate_of_decrease_min + (rand() % (config.rate_of_decrease_max - config.rate_of_decrease_min + 1));
        
        printf("player %d palyer_id %d team_id %d make effort = %d\n",getpid(),player_id,team_id,effort);         
	
	/*3- send the effort */
   	send_message(2,effort);
	
}

void printConfig(const Config *cfg) {
    printf("\n=== Config Values ===\n");
    printf("Max Score: %d\n", cfg->max_score);
    printf("Max Time: %d\n", cfg->max_time);
}

void handle_sigint(int sig) {
    keep_running = 0;  // Set flag to exit the loop
}

