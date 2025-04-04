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
void handle_sleep();
int get_random_in_range(int range_min, int range_max);


int fd;
int player_id,team_id;
pid_t player;


int main (int argc, char **argv){


    if (argc < 6) {  // Now expects 5 args + pipe FD
        fprintf(stderr, "Usage: %s <config> <fifo> <player_id> <team_id> <pipe_fd>\n", argv[0]);
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

   
   player_id=atoi(argv[3]);
   team_id=atoi(argv[4]);

   int pipe_fd = atoi(argv[5]);
   
    


    // here we generate initial energy and send it to referee 
    send_message(1,generate_energy(player_id));

    
    if (sigset(SIGALRM,handle_sleep) == SIG_ERR){
        perror("NO SLEEP LIKE ME");
        exit(SIGQUIT);
    }
    
    if ( sigset(SIGUSR1, handle_getready) == SIG_ERR ) {
    perror("Sigset can not set SIGUSR1");
    exit(SIGQUIT);
    }
  
    if ( sigset(SIGUSR2, handle_start_game) == SIG_ERR ) {
    perror("Sigset can not set SIGUSR2");
    exit(SIGQUIT);
    }

    signal(SIGINT, handle_sigint);

    char buffer[8];  // Buffer to store the received message
    while (keep_running) {
        ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer - 1));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("player %d in team %d is a %s\n",player_id, team_id+1,buffer);
        }

        //pause();
    }
    close(pipe_fd);
    close(fd);
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
    if (write(fd, &msg, sizeof(Message)) == -1) {
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

void handle_sleep() {
    int sleep_time = get_random_in_range(1, config.max_score);
    
    // Step 1: Block all signals
    sigset_t mask, oldmask;
    sigfillset(&mask);  // Add all signals to the set
    sigprocmask(SIG_BLOCK, &mask, &oldmask);  // Block them
    
    // Step 2: Sleep uninterruptibly for 10 seconds
    unsigned int remaining = 10;
    while (remaining > 0) {
        remaining = sleep(remaining);  // sleep() returns unslept time if interrupted
    }
    
    // Step 3: Restore original signal mask
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

int get_random_in_range(int range_min, int range_max) {
    // Initialize random seed (do this once at program start)
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    
    // Calculate random number within range
    int range = range_max - range_min + 1;
    int random_num = rand() % range + range_min;
    
    return random_num;
}

