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
#include <errno.h>
#include <time.h>
int player_pipes[8][2]; // [player][0=read, 1=write]
int player_fell = 0;
int team_id_fall = 0;
int player_id_fall = 0;
int prev_energy = 0;



typedef void (*sighandler_t)(int);
sighandler_t sigset(int sig, sighandler_t disp);

int should_terminate = 1; 

Config config;



void send_data_opengl();
void printConfig(const Config *cfg);
void print_team(const Team *team);
void sort_main_array(int current_energy_team[2][4]);

void handle_alarm_max_time();
void handle_woken();

void load_current_energy(int w_time);

void check_winner();
void check_winner_in_row();
void check_win_tie();

void send_getready_signal();
void send_start_game_signal();

void print_current_array();
void terminate_game();

void send_winner_message(int team);
void send_loser_message(int team);
void send_tie_message();
int get_random_in_range(int range_min, int range_max);
void player_fallen();

void receive_energy(int message_type);

int  first_energy=0;

int max_win_in_row;
int difference_effort=0;/*if pos then team 1 win*/

char fifo_name[8][50];
char fifo_opengl[50];
pid_t player[8];
pid_t opengl_process;
Team team_1;
Team team_2;
int current_energy_team_1[2][4];/*our map to sort the player with largest energy, and there id in team struct */
int current_energy_team_2[2][4];/*to give us good control in teams energy */
int team_id;
int player_id;
int fd[8];
int fd_opengl;
int first_time=0;
int time_is_over=0;

/* Last update */
/* Check end game condition and round update*/
/* put in if stop condiition , after time end ,check how larger ,  */
/* Waht should do*/
int main(int argc, char **argv) {
    if ( sigset(SIGUSR1, handle_woken) == SIG_ERR ) {
        perror("Sigset can not set SIGUSR1");
        exit(SIGQUIT);
        }


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

    for (int i = 0; i < 8; i++) {
        if (pipe(player_pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    
    
    

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
        perror("mkfifo1");
        exit(1);
    }
    }
    
    snprintf(fifo_opengl, sizeof(fifo_opengl),"/tmp/fifo_opengl");
    if (mkfifo(fifo_opengl, 0666) == -1) {
        perror("mkfifo_opengl");
        exit(1);
    }
   
    
    
	
    if ((opengl_process = fork()) == 0){
      execlp("./bin/opengl","./bin/opengl",fifo_opengl , NULL);
      perror("execlp failed for player");
      exit(EXIT_FAILURE);
    }else{
     fd_opengl = open(fifo_opengl,  O_WRONLY);
    if (fd_opengl < 0) {
    perror("open");
    exit(EXIT_FAILURE);
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

      close(player_pipes[i][1]);  // Close unused write end


        // Convert read-FD to string and pass to player
       char fd_str[16];
       snprintf(fd_str, sizeof(fd_str), "%d", player_pipes[i][0]);

       
      // Child process (Player)
      char player_id_str[20];
      char team_id_str[20];

      snprintf(player_id_str, sizeof(player_id_str),"%d", player_id);
      snprintf(team_id_str, sizeof(team_id_str),"%d",team_id);

      execlp("./bin/player","./bin/player", argv[1],fifo_name[i],player_id_str,team_id_str, fd_str, NULL);
      perror("execlp failed for player");
      exit(EXIT_FAILURE);
    } 
    else{
        close(player_pipes[i][0]);  // Close unused read end
    // Parent process (Referee)
    fd[i] = open(fifo_name[i], O_RDONLY);
    if (fd[i] < 0) {
    perror("open");
    exit(EXIT_FAILURE);
    }
    
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
	send_data_opengl();
	printf("|||||||||||||||||||||||||First time|||||||||||||||||||||||\n");
	print_current_array();
	printf("|||||||||||||||||||||||||First time|||||||||||||||||||||||\n");
	
	send_start_game_signal();
	receive_energy(2);
	load_current_energy(1);
	send_data_opengl();
	printf("|||||||||||||||||||||||||Second time|||||||||||||||||||||||\n");
	print_current_array();
	printf("|||||||||||||||||||||||||Second time|||||||||||||||||||||||\n");
	/**********************First Round**********************/
	
       // now for create signals of kill signals that used for start game and getReady 
       // so, we will send get ready signal , player will recive it ,player will send first node( Player data ) after generate
       // its energy , then will parent store player in teams ,from parent will draw easly .
       
 
	/*for get data we suppose to get initial energy at first of program , 
	for update the energy we send energy in last of round so we can be updated with current energy */
    int timer = get_random_in_range(3, (config.max_score/2) + 1);
    int count = 0;
    team_id_fall = get_random_in_range(0,1);
    player_id_fall = get_random_in_range(0,3);
	while(should_terminate){
        if (count == timer){
            player_fallen(team_id_fall,player_id_fall);
            player_fell = 1;
        }
        check_winner();
		send_getready_signal();
		// send function to plot
		send_data_opengl();
		send_start_game_signal();
		receive_energy(2);
		send_data_opengl();
		load_current_energy(1);
		send_data_opengl();
		//check if exceed the thresold of win round 
		// if yes incerement score for team 
		/*for alarm we will check alarm flag also */
		// for case of win some rounds in countinue we will make two variabels counters and check it to value in config file 
		print_current_array();
        sleep(1);
        count++;
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
	    Message msg;

        if (i == (player_id_fall +( team_id_fall * 4)) && player_fell == 1)
            continue;
        
        read(fd[i], &msg, sizeof(Message));
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
        } 
        else if(msg.type == message_type && message_type==2 && msg.type==2){ 
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
                   //total_effort_team_2 = 0;
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
 	
 	if (team_1.score>team_2.score){
            printf("Team 1 Won\n");
            send_status_message(0);
        }
        else if (team_1.score < team_2.score){
            printf("Team 2 Won\n");
            send_status_message(1);
        }
        else{
            printf("Tie\n");
            send_status_message(-1);
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
    if (!should_terminate){
        return;
    }
    // 1. Close all FIFO file descriptors
    for (int i = 0; i < 8; i++) {
        if (close(fd[i]) == -1) {
            perror("Failed to close FIFO");
        }
    }
    
    if (close(fd_opengl) == -1) {
            perror("Failed to close FIFO");
            }

    // 2. Remove (unlink) FIFO files
    for (int i = 0; i < 8; i++) {
        if (unlink(fifo_name[i]) == -1) {
            perror("Failed to unlink FIFO");
        } else {
            printf("Removed FIFO: %s\n", fifo_name[i]);
        }
    }
    if (unlink(fifo_opengl) == -1) {
            perror("Failed to unlink FIFO");
        } else {
            printf("Removed FIFO: %s\n", fifo_opengl);
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
    
    if (kill(opengl_process, SIGTERM) == 0) {
            printf("Killed player with PID %d from team 2.\n", opengl_process);
        } else {
            perror("Failed to kill opengl_process");}
            
    for (int i = 0; i < 8; i++) {
        close(player_pipes[i][1]);  // Close write ends first
    }
    
    should_terminate = 0;
}

void check_winner(){
    if (!should_terminate){
        return;
    }
	if((team_1.score==config.max_score)||(max_win_in_row==team_1.win_counter)){
        printf("Team 1 has won :)\n");
        terminate_game();
    }
	else if((team_2.score==config.max_score)||(max_win_in_row==team_2.win_counter)){
        printf("Team 2 has won :)\n");
        terminate_game();
    }
}
void check_win_tie(){
	if (team_1.score>team_2.score){
        printf("Team 1 has won :)\n");
    }	else if(team_1.score<team_2.score){
        printf("Team 2 has won :)\n");
    }
	else if(team_1.score==team_2.score){
        printf("Tie\n");
    }
}
void send_status_message(int winning_team) {
    const char *winner_message = "Winner"; 
    const char *loser_message = "Loser"; 
    const char *tie_message = "tie";
    if (winning_team == 0){
        for (int i = 0; i < 8; i++) {
            if (i < 4){
                int msg_len = strlen(winner_message);
                ssize_t bytes = write(player_pipes[i][1], winner_message, msg_len);
                if (bytes == -1) {
                    perror("Failed to write status to player pipe");
                } else if (bytes < msg_len) {
                    fprintf(stderr, "Partial write to player %d\n", i);
                }
            }
            else{
                int msg_len = strlen(loser_message);
                ssize_t bytes = write(player_pipes[i][1], loser_message, msg_len);
                if (bytes == -1) {
                    perror("Failed to write status to player pipe");
                } else if (bytes < msg_len) {
                    fprintf(stderr, "Partial write to player %d\n", i);
                }
            }
        }
    }
    else if (winning_team == 1){
        for (int i = 0; i < 8; i++) {
            if (i < 4){
                int msg_len = strlen(loser_message);
                ssize_t bytes = write(player_pipes[i][1], loser_message, msg_len);
                if (bytes == -1) {
                    perror("Failed to write status to player pipe");
                } else if (bytes < msg_len) {
                    fprintf(stderr, "Partial write to player %d\n", i);
                }
            }
            else{
                int msg_len = strlen(winner_message);
                ssize_t bytes = write(player_pipes[i][1], winner_message, msg_len);
                if (bytes == -1) {
                    perror("Failed to write status to player pipe");
                } else if (bytes < msg_len) {
                    fprintf(stderr, "Partial write to player %d\n", i);
                }
            }
        }
    }
    else if (winning_team == -1){
        for (int i = 0; i < 8; i++) {
                int msg_len = strlen(tie_message);
                ssize_t bytes = write(player_pipes[i][1], tie_message, msg_len);
                if (bytes == -1) {
                    perror("Failed to write status to player pipe");
                } else if (bytes < msg_len) {
                    fprintf(stderr, "Partial write to player %d\n", i);
                }
        }
    }
}
void send_winner_message(int winning_team) {
    const char *message = "Winner";  // Example message (includes null terminator)
    int message_length = strlen(message);  // Length without null terminator
    for (int i=0; i < 4; i++){
        int index = (winning_team == 0) ? i : i+4;
        write(player_pipes[index][1], message, message_length);
    }
    }
void send_loser_message(int losing_team){
    const char *message = "Loser";  // Example message (includes null terminator)
    int message_length = strlen(message);  // Length without null terminator
    for (int i=0; i < 4; i++){
        int index = (losing_team == 0) ? i : i+4;
        write(player_pipes[index][1], message, message_length);
    }
}
void send_tie_message(){
    const char *message = "Tie";  // Example message (includes null terminator)
    int message_length = strlen(message);  // Length without null terminator
    for (int i=0; i < 4; i++){
        int index = i;
        write(player_pipes[index][1], message, message_length);
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

void player_fallen(int team_id, int player_id){

    if (team_id == 0){
        printf("player with id %d in team %d has fallen.\n", player_id, team_id+1);
        kill(team_1.players[player_id], SIGALRM);
        int current_player;
        for (int j =0; j < 4; j++){
            if (current_energy_team_1[1][j] == player_id) {
                current_player = j;
                break;
            }
        }
        prev_energy = current_energy_team_1[0][current_player];
        current_energy_team_1[0][current_player] = 0;
    }
    else{
        printf("player with id %d in team %d has fallen.\n", player_id, team_id+1);
        kill(team_2.players[player_id], SIGALRM);
        int current_player;
        for (int j =0; j < 4; j++){
            if (current_energy_team_2[1][j] == player_id) {
                current_player = j;
                break;
            }
        }
        prev_energy = current_energy_team_1[0][current_player];
        current_energy_team_2[0][current_player] = 0;
    }
}
void handle_woken(){
    player_fell = 0;
    if (team_id_fall == 0){
        int current_player;
        for (int j =0; j < 4; j++){
            if (current_energy_team_1[1][j] == player_id_fall) {
                current_player = j;
                break;
            }
        }
        current_energy_team_1[0][current_player] = prev_energy;
    }
    else{
        int current_player;
        for (int j =0; j < 4; j++){
            if (current_energy_team_2[1][j] == player_id_fall) {
                current_player = j;
                break;
            }
        }
        current_energy_team_2[0][current_player] = prev_energy;
    }
}
void send_data_opengl(){
Opengl_Message o_message;

    // Fill current_effort_team_1 with current_energy_team_1 data
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            o_message.current_effort_team_1[i][j] = current_energy_team_1[i][j];
        }
    }

    // Fill current_effort_team_2 with current_energy_team_2 data
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            o_message.current_effort_team_2[i][j] = current_energy_team_2[i][j];
        }
    }

    // Assign scores to the message
    o_message.score_team_1 = team_1.score;
    o_message.score_team_2 = team_2.score;
	if (write(fd_opengl, &o_message, sizeof(o_message)) == -1) {
        perror("write");
        
    }
}


