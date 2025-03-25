#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../include/config.h"

Config config;

void printConfig(const Config *cfg);
void handle_alarm_max_time();
char fifo_name[50];
pid_t player;
int fd;
Team team_1;
Team team_2;

int main(int argc, char **argv) {


    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return EXIT_FAILURE;
    }


	/* Read  Configuration file */    
    if (load_config(argv[1], &config) != 0) {
        fprintf(stderr, "Failed to load config file.\n");
        return EXIT_FAILURE;
    }
	/* Read  Configuration file */ 
    

    // Set up alarm for maximum time
    signal(SIGALRM, handle_alarm_max_time);
    printf("Referee: Maximum time set to %d seconds\n", config.max_time);
    alarm(config.max_time);



    /* FIFO Creation */
    snprintf(fifo_name, sizeof(fifo_name),"/tmp/fifo%d", getpid());
    if (mkfifo(fifo_name, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }
    /* FIFO Creation */

    
	/*Create procesess*/
	// here we will creat while loop and send to player team id ,player id [0-3],pid_t i think isnt nessecery where it 
	// can fetch it by getPid() , then player send its energy by fifo the team node ready with initial energy .
    if ((player = fork()) == 0) {
       
        // Child process (Player)
        execlp("./bin/player","./bin/player", argv[1],fifo_name, NULL);
        perror("execlp failed for player");
        exit(EXIT_FAILURE);
    } 
        // Parent process (Referee)
        fd = open(fifo_name, O_RDONLY);
        if (fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        /*Create procesess*/
        
        
        
       // now for create signals of kill signals that used for start game and getReady 
       // so, we will send get ready signal , player will recive it ,player will send first node( Player data ) after generate
       // its energy , then will parent store player in teams ,from parent will draw easly .
       
       
       

       /*
        // Read message Example from player
        char buffer[100];
        int n = read(fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';  // Null-terminate
            printf("Referee received: %s\n", buffer);
        }

        close(fd);
        unlink(fifo_name);  // Clean up FIFO
	*/    

    return 0;
}

void handle_alarm_max_time() {
    printf("Referee: Maximum time reached\n");
}

void printConfig(const Config *cfg) {
    printf("\n=== Config Values ===\n");
    printf("Max Score: %d\n", cfg->max_score);
    printf("Max Time: %d\n", cfg->max_time);
}

