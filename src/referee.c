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
void print_team(const Team *team);
void handle_alarm_max_time();
void load_current_energy();
void sort_main_array(int current_energy_team[2][4]);
void send_getready_signal();
void receive_energy(int message_type);
int  first_energy=0;
char fifo_name[8][50];
pid_t player[8];
Team team_1;
Team team_2;
int current_energy_team_1[2][4];/*our map to sort the player with largest energy, and there id in team struct */
int current_energy_team_2[2][4];/*to give us good control in teams energy */
int team_id;
int player_id;
int fd[8];

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


    for(int  i = 0 ; i<8 ; i++ ){
    /* FIFO Creation */


        if(i  <= 3 ){
            team_id = 0;
            player_id=i;
        }else{
            team_id=1;
            player_id=i-4;
        }

    snprintf(fifo_name[i], sizeof(fifo_name),"/tmp/fifo_team_%d_player_%d",team_id, player_id);
    if (mkfifo(fifo_name[i], 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }
    }
    /* FIFO Creation */

    
	/*Create procesess*/
	// here we will creat while loop and send to player team id ,player id [0-3],pid_t i think isnt nessecery where it 
	// can fetch it by getPid() , then player send its energy by fifo the team node ready with initial energy .


    for(int  i = 0 ; i<8 ; i++ ){
    if ((player [i]= fork()) == 0)
    {
	


      if(i  <= 3 ){
        team_id = 0;
        player_id=i;
      }else{
        team_id=1;
        player_id=i-4;
      }

       
        // Child process (Player)
      char player_id_str[20];
      char team_id_str[20];

      snprintf(player_id_str, sizeof(player_id_str),"%d", player_id);
      snprintf(team_id_str, sizeof(team_id_str),"%d",team_id);

      execlp("./bin/player","./bin/player", argv[1],fifo_name[i],player_id_str,team_id_str, NULL);
      perror("execlp failed for player");
      exit(EXIT_FAILURE);
    } 
        // Parent process (Referee)
        fd[i] = open(fifo_name[i], O_RDONLY);
        if (fd[i] < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        /*Create procesess*/
    }
        
        
       // now for create signals of kill signals that used for start game and getReady 
       // so, we will send get ready signal , player will recive it ,player will send first node( Player data ) after generate
       // its energy , then will parent store player in teams ,from parent will draw easly .
       
 
       
 
	printf("\nFinal Team Data:\n");
	print_team(&team_1);
	print_team(&team_2);
	/* Recive initial energy */
	
	
	/*for get data we suppose to get initial energy at first of program , for update the energy we send energy in last of round so we can be updated with current energy */
	send_getready_signal();
	/*now we sorted players in program and plot it in sorted way */
	load_current_energy();
	
	

    return 0;
}
/*27/3/2025 Revivsion */
/*here we sent get ready signal , and have initial energy */
/*So we go to bulid alingment code in opengl */

void sort_main_array(int current_energy_team[2][4]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3 - i; j++) {
            if (current_energy_team[0][j] > current_energy_team[0][j + 1]) {
                // Swap energy values
                int temp_energy = current_energy_team[0][j];
                current_energy_team[0][j] = current_energy_team[0][j + 1];
                current_energy_team[0][j + 1] = temp_energy;

                // Swap corresponding player IDs
                int temp_id = current_energy_team[1][j];
                current_energy_team[1][j] = current_energy_team[1][j + 1];
                current_energy_team[1][j + 1] = temp_id;
            }
        }
    }
}
void receive_energy(int message_type){
   /* Recive initial energy */
    Message msg;
    int received_count = 0;  // Track received messages
    int total_effort_team_1=0;
    int total_effort_team_2=0;
    while (received_count < 8) {
         for (int i = 0; i < 8; i++) {
              int n = read(fd[i], &msg, sizeof(Message));
              if (n > 0) {
              	 if(msg.type == message_type){	//Initial energy 
            		if(msg.team_id == 0){
            		team_1.team_id = msg.team_id ;
            		team_1.player_id[msg.player_id]=msg.player_id;
            		team_1.players[msg.player_id]=msg.player_pid;
            		team_1.initial_energy[msg.player_id]=atoi(msg.content);
            		}else{
            		team_2.team_id = msg.team_id ;
            		team_2.player_id[msg.player_id]=msg.player_id;
            		team_2.players[msg.player_id]=msg.player_pid;
            		team_2.initial_energy[msg.player_id]=atoi(msg.content);
            	
            	}
            }
            else if(msg.type == message_type){
            	if(msg.team_id == 0){
            		int current_player_team_1;
            		int current_effort_team_1;
            		int current_effort_with_wieght_team_1;
            		for (int i = 0; i < 4; i++) 
           			if(current_energy_team_1[1][i]==msg.player_id)
           				current_player_team_1=i;
           						 
           	       	current_effort_team_1=atoi(msg.content);
           	       	current_effort_with_wieght_team_1=current_effort_team_1*(current_player_team_1+1);//effort depend on position 1.2.3.4 (i+1)
            		current_energy_team_1[0][current_player_team_1]=current_energy_team_1[0][current_player_team_1]-current_effort_with_wieght_team_1;
            		//Here we will check case of energy go to zero ,  soon
            		total_effort_team_1=total_effort_team_1+current_effort_with_wieght_team_1;

            		}else{
            	
            		int current_player_team_2;
            		int current_effort_team_2;
            		int current_effort_with_wieght_team_2;
            		for (int i = 0; i < 4; i++) 
           			if(current_energy_team_2[1][i]==msg.player_id)
           				current_player_team_2=i;
           						 
           	       	current_effort_team_2=atoi(msg.content);
           	       	current_effort_with_wieght_team_2=current_effort_team_2*(current_player_team_2+1);//effort depend on position 1.2.3.4 (i+1)
            		current_energy_team_2[0][current_player_team_2]=current_energy_team_2[0][current_player_team_2] - current_effort_with_wieght_team_2;
            		//Here we will check case of energy go to zero ,  soon
            		total_effort_team_2=total_effort_team_2+current_effort_with_wieght_team_2;
            	
            	}
            
            }
        received_count++;  // Increment count of received messages
        }
    }
}

}

void load_current_energy() {
    /* If first_energy is 0, initialize energy from team structures */
    if (first_energy == 0) {
        first_energy = 1; 
       for (int i = 0; i < 4; i++) {
            current_energy_team_1[0][i] = team_1.initial_energy[i]; // Store energy
            current_energy_team_1[1][i] = team_1.player_id[i];       // Store player ID
        }
        for (int i = 0; i < 4; i++) {
            current_energy_team_2[0][i] = team_2.initial_energy[i]; // Store energy
            current_energy_team_2[1][i] = team_2.player_id[i];       // Store player ID
        }
        
        
        sort_main_array(current_energy_team_1);
        sort_main_array(current_energy_team_2);
        
        printf("Sorted Energy Levels:\n");
        printf("Team 1:\n");
        for (int i = 0; i < 4; i++) {
            printf("Player %d  - Energy: %d\n", current_energy_team_1[1][i], current_energy_team_1[0][i]);
        }
        printf("Team 2:\n");
        for (int i = 0; i < 4; i++) {
            printf("Player %d - Energy: %d\n", current_energy_team_2[1][i], current_energy_team_2[0][i]);
        }
    } else {
    
    /*Here we will recieve effort that players do and save score */
    
        
    }
}
void send_getready_signal(){

/*Here we will kill all SIGUSR1 for all child to know getready message or signal */   
    	for(int  i = 0 ; i<4 ; i++ ){
    	
    	 	if (kill(team_1.players[i], SIGUSR1) == 0) {
        		printf("Send signal to process %d From team 1.\n",team_1.players[i]);
    		} else {
        		perror("kill failed for team 1");
    		}
    		if (kill(team_2.players[i], SIGUSR1) == 0) {
        		printf("Send signal to process %d From team 2.\n", team_2.players[i]);
    		} else {
        		perror("kill failed for team 2");
    		}
    		
    	    }

}


void handle_alarm_max_time() {
    printf("Referee: Maximum time reached\n");
}

void printConfig(const Config *cfg) {
    printf("\n=== Config Values ===\n");
    printf("Max Score: %d\n", cfg->max_score);
    printf("Max Time: %d\n", cfg->max_time);
}

void print_team(const Team *team) {
    printf("\n=== Team %d ===\n", team->team_id);
    printf("Score: %d\n", team->score);
    for (int i = 0; i < 4; i++) {
        printf("Player %d:\n", team->player_id[i]);
        printf("  Initial Energy: %d\n", team->initial_energy[i]);
        printf("  PID: %d\n", team->players[i]);
    }
}


