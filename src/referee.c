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

volatile sig_atomic_t should_terminate = 1; 

Config config;

void printConfig(const Config *cfg);
void print_team(const Team *team);
void handle_alarm_max_time();
void load_current_energy(int w_time);
void sort_main_array(int current_energy_team[2][4]);
void send_getready_signal();
void send_start_game_signal();
void receive_energy(int message_type);
void print_current_array();
void terminate_game();
void check_winner();
void check_winner_in_row();
void check_win_tie();
void send_winner_message(int team);
void send_loser_message(int team);
int  first_energy=0;

int max_win_in_row;
int difference_effort=0;/*if pos then team 1 win*/

char fifo_name[8][50];
pid_t player[8];
Team team_1;
Team team_2;
int current_energy_team_1[2][4];/*our map to sort the player with largest energy, and there id in team struct */
int current_energy_team_2[2][4];/*to give us good control in teams energy */
int team_id;
int player_id;
int fd[8];
int first_time=0;
int time_is_over=0;
/* Last update */
/* Check end game condition and round update*/
/* put in if stop condiition , after time end ,check how larger ,  */
/* Waht should do*/
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
    max_win_in_row=(config.max_score/2)+1;
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
        
  	receive_energy(1);
 
	printf("\nFinal Team Data:\n");
	print_team(&team_1);
	print_team(&team_2);
	/* Recive initial energy */
	
	
	/*for get data we suppose to get initial energy at first of program , 
	for update the energy we send energy in last of round so we can be updated with current energy */
	
	//send_getready_signal();
	/*now we sorted players in program and plot it in sorted way */
	
	/**********************First Round**********************/
	
	
	load_current_energy(0);
	printf("|||||||||||||||||||||||||First time|||||||||||||||||||||||\n");
	print_current_array();
	printf("|||||||||||||||||||||||||First time|||||||||||||||||||||||\n");
	
	send_start_game_signal();
	receive_energy(2);
	load_current_energy(1);
	printf("|||||||||||||||||||||||||Second time|||||||||||||||||||||||\n");
	print_current_array();
	printf("|||||||||||||||||||||||||Second time|||||||||||||||||||||||\n");
	/**********************First Round**********************/
	
       // now for create signals of kill signals that used for start game and getReady 
       // so, we will send get ready signal , player will recive it ,player will send first node( Player data ) after generate
       // its energy , then will parent store player in teams ,from parent will draw easly .
       
 
	/*for get data we suppose to get initial energy at first of program , 
	for update the energy we send energy in last of round so we can be updated with current energy */
	
	
	
	
	while(should_terminate){
        sleep(1);
        check_winner();
		send_getready_signal();
		// send function to plot
		send_start_game_signal();
		receive_energy(2);
		load_current_energy(1);
		//check if exceed the thresold of win round 
		// if yes incerement score for team 
		/*for alarm we will check alarm flag also */
		// for case of win some rounds in countinue we will make two variabels counters and check it to value in config file 
		print_current_array();

	}
    return 0;
    /*#################### Main ################################*/
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
void receive_energy(int message_type) {
    if (!should_terminate){
        return;
    }
    int total_effort_team_1 = 0;
    int total_effort_team_2 = 0;

    for (int i = 0; i < 8; i++) {
        int n;
	Message msg;

	while ((n = read(fd[i], &msg, sizeof(Message))) <= 0) {
    	if (n == -1) {
        	perror("Error reading from FIFO");
    	}
    	sleep(1);  // Wait and retry
	}

        if (msg.type == message_type && message_type==1 && msg.type==1) {  // Initial energy received
            if (msg.team_id == 0) {
                team_1.team_id = msg.team_id;
                team_1.player_id[msg.player_id] = msg.player_id;
                team_1.players[msg.player_id] = msg.player_pid;
                team_1.initial_energy[msg.player_id] = atoi(msg.content);
                team_1.score=0;
                team_1.win_counter=0;
            } else {
                team_2.team_id = msg.team_id;
                team_2.player_id[msg.player_id] = msg.player_id;
                team_2.players[msg.player_id] = msg.player_pid;
                team_2.initial_energy[msg.player_id] = atoi(msg.content);
            	team_2.score=0;
            	team_2.win_counter=0;
            }
        } else if(msg.type == message_type && message_type==2 && msg.type==2){ 
        	 // Update energy based on effort
            
            
            int current_player;
            int current_effort;
            int current_effort_with_weight;

            if (msg.team_id == 0) {  // Team 1
                for (int j = 0; j < 4; j++) {
                    if (current_energy_team_1[1][j] == msg.player_id) {
                        current_player = j;
                        break;
                    }
                }

                current_effort = atoi(msg.content);
                
                current_effort_with_weight = current_effort * (current_player + 1);
                current_energy_team_1[0][current_player] -= current_effort_with_weight;
                total_effort_team_1 += current_effort_with_weight;
		printf("current_effort = %d ,current_effort_with_weight = %d  total_effort_team_1= %d (player_id =%d),team_id = %d , player in sorted array =%d\n",current_effort,current_effort_with_weight,total_effort_team_1,msg.player_id,msg.team_id,current_player);
            } else {  // Team 2
                for (int j = 0; j < 4; j++) {
                    if (current_energy_team_2[1][j] == msg.player_id) {
                        current_player = j;
                        break;
                    }
                }

                current_effort = atoi(msg.content);
                current_effort_with_weight = current_effort * (current_player + 1);
                current_energy_team_2[0][current_player] -= current_effort_with_weight;
                total_effort_team_2 += current_effort_with_weight;
                printf("current_effort = %d,current_effort_with_weight = %d  total_effort_team_2= %d (player_id =%d),team_id = %d , player in sorted array =%d\n",current_effort,current_effort_with_weight,total_effort_team_2,msg.player_id,msg.team_id,current_player);
            }
        }
    }
    difference_effort=(total_effort_team_1-total_effort_team_2);
    if (difference_effort > 0){
        team_1.score+=1;
        team_1.win_counter++;
        team_2.win_counter=0;
    }
    else if (difference_effort < 0){
        team_2.score+=1;
        team_2.win_counter++;
        team_1.win_counter=0;
    }
    else if(total_effort_team_1 >= config.win_threshold){
        team_1.score+=1;
        team_1.win_counter++;
        team_2.win_counter=0;
    }
    else if(total_effort_team_2 >= config.win_threshold){
        team_2.score+=1;
        team_2.win_counter++;
        team_1.win_counter=0;
 	}
    difference_effort=0;
    // If the difference_effort == 0 do nothing
}


void load_current_energy(int w_time) {
    if (!should_terminate){
        return;
    }
    /* If first_energy is 0, initialize energy from team structures */
    if (w_time == 0) {
        first_energy = 1; 
       for (int i = 0; i < 4; i++) {
            current_energy_team_1[0][i] = team_1.initial_energy[i]; // Store energy
            current_energy_team_1[1][i] = team_1.player_id[i];      // Store player ID
        }
        for (int i = 0; i < 4; i++) {
            current_energy_team_2[0][i] = team_2.initial_energy[i]; // Store energy
            current_energy_team_2[1][i] = team_2.player_id[i];      // Store player ID
        }
        
        
        sort_main_array(current_energy_team_1);
        sort_main_array(current_energy_team_2);
        
            
    } else if(w_time == 1) {
    	sort_main_array(current_energy_team_1);
        sort_main_array(current_energy_team_2);
        
    }
}
void send_getready_signal(){
	//printf("////////////////SIGUSR1///////////////////\n");

/*Here we will kill all SIGUSR1 for all child to know getready message or signal */   
    	for(int  i = 0 ; i<4 ; i++ ){
    	 	if (kill(team_1.players[i], SIGUSR1) == 0) {
        		//printf("Send signal to process %d From team 1.\n",team_1.players[i]);
    		} else {
        		perror("kill failed for team 1");
    		}
    		}
    		for(int  i = 0 ; i<4 ; i++ ){
    		if (kill(team_2.players[i], SIGUSR1) == 0) {
        		//printf("Send signal to process %d From team 2.\n", team_2.players[i]);
    		} else {
        		perror("kill failed for team 2");
    		}
    		}
    		
    	    
    	   // printf("////////////////SIGUSR1///////////////////\n");

}
void send_start_game_signal(){

/*Here we will kill all SIGUSR2 for all child to know start game message or signal */   
	printf("////////////////SIGUSR2///////////////////\n");        		
    	for(int  i = 0 ; i<4 ; i++ ){
    	 	if (kill(team_1.players[i], SIGUSR2) == 0) {
        		//printf("Send signal to process %d From team 1.\n",team_1.players[i]);
    		} else {
        		perror("kill failed for team 1");
    		}
    		}
    		for(int  i = 0 ; i<4 ; i++ ){
    		if (kill(team_2.players[i], SIGUSR2) == 0) {
        		//printf("Send signal to process %d From team 2.\n", team_2.players[i]);
    		} else {
        		perror("kill failed for team 2");
    		}
    		}
    		
    	    
    	printf("////////////////SIGUSR2///////////////////\n");	    

}


void handle_alarm_max_time() {
    printf("Referee: Maximum time reached\n");
    check_win_tie();
    terminate_game();
}

void terminate_game(){
    // 1. Close all FIFO file descriptors
    for (int i = 0; i < 8; i++) {
        if (close(fd[i]) == -1) {
            perror("Failed to close FIFO");
        }
    }

    // 2. Remove (unlink) FIFO files
    for (int i = 0; i < 8; i++) {
        if (unlink(fifo_name[i]) == -1) {
            perror("Failed to unlink FIFO");
        } else {
            printf("Removed FIFO: %s\n", fifo_name[i]);
        }
    }
    
    // 3. Kill all player processes
    for (int i = 0; i < 4; i++) {
        if (kill(team_1.players[i], SIGTERM) == 0) {
            printf("Killed player with PID %d from team 1.\n", team_1.players[i]);
        } else {
            perror("Failed to kill player from team 1");
        }
    }
    
    for (int i = 0; i < 4; i++) {
        if (kill(team_2.players[i], SIGTERM) == 0) {
            printf("Killed player with PID %d from team 2.\n", team_2.players[i]);
        } else {
            perror("Failed to kill player from team 2");
        }
    }
    should_terminate = 0;
}

void check_winner(){
	if((team_1.score==config.max_score)||(max_win_in_row==team_1.win_counter)){
        printf("Team 1 has won :)\n");
        //send_winner_message(0);
        //send_loser_message(1);
        terminate_game();
    }
	else if((team_2.score==config.max_score)||(max_win_in_row==team_2.win_counter)){
        printf("Team 2 has won :)\n");
       // send_winner_message(1);
       // send_loser_message(0);
        terminate_game();
    }
}
void check_win_tie(){
	if (team_1.score>team_2.score){
        printf("Team 1 has won :)\n");
        //send_winner_message(0);
        //send_loser_message(1);
    }	else if(team_1.score<team_2.score){
        printf("Team 2 has won :)\n");
      //  send_winner_message(1);
      //  send_loser_message(0);
    }
	else if(team_1.score==team_2.score){
        printf("Tie\n");
        // Send message
    }
}
void send_winner_message(int winning_team) {
    Message msg;
    msg.type = 3;  // Winner message type
    msg.team_id = winning_team;
    snprintf(msg.content, sizeof(msg.content), "Your team won!");

    if (winning_team == 0) {
        // Team 0 players (fd[0]-fd[3])
        for (int i =0; i<4; i++){
            msg.player_id = i;
            msg.player_pid = team_1.players[i];
            if (write(fd[i], &msg, sizeof(Message)) == -1) perror("Failed to send to player");
        }
    }
    else {
        // Team 1 players (fd[4]-fd[7])
        for (int i =0; i<4; i++){
            msg.player_id = i;
            msg.player_pid = team_1.players[i];
            if (write(fd[i+4], &msg, sizeof(Message)) == -1) perror("Failed to send to player");
        }
    }
}
void send_loser_message(int losing_team){
    Message msg;
    msg.type = 4;  // Loser message type
    msg.team_id = losing_team;
    snprintf(msg.content, sizeof(msg.content), "Your team lost.");

    if (losing_team == 0) {
        // Team 0 players (fd[0]-fd[3])
        for (int i =0; i<4; i++){
            msg.player_id = i;
            msg.player_pid = team_1.players[i];
            if (write(fd[i], &msg, sizeof(Message)) == -1) perror("Failed to send to player");
        }
    }
    else {
        // Team 1 players (fd[4]-fd[7])
        for (int i =0; i<4; i++){
            msg.player_id = i;
            msg.player_pid = team_1.players[i];
            if (write(fd[i+4], &msg, sizeof(Message)) == -1) perror("Failed to send to player");
        }
    }
}
void send_tie_message(){
    Message msg;
    msg.type = 5;  // tie message type
    snprintf(msg.content, sizeof(msg.content), "Tie.");
    for (int i =0; i<8; i++){
        if (i < 4){
            msg.player_id = i;
            msg.player_pid = team_1.players[i];
            if (write(fd[i], &msg, sizeof(Message)) == -1) perror("Failed to send to player");
        }
        else{
            msg.player_id = i-4;
            msg.player_pid = team_2.players[i-4];
            if (write(fd[i], &msg, sizeof(Message)) == -1) perror("Failed to send to player");
        }
    }
}
void print_current_array(){
    if (!should_terminate){
        return;
    }
 	printf("Sorted Energy Levels:\n");
        printf("Team 1:\n");
        for (int i = 0; i < 4; i++) 
            printf("Player %d  - Energy: %d\n", current_energy_team_1[1][i], current_energy_team_1[0][i]);
        printf("Score for team 1: %d\n", team_1.score);
        printf("Team 2:\n");
        for (int i = 0; i < 4; i++) 
            printf("Player %d - Energy: %d\n", current_energy_team_2[1][i], current_energy_team_2[0][i]);
        printf("Score for team 2: %d\n",team_2.score);             
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


