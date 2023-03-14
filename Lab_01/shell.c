#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1000
#define M 100
#define LOG_FILE "/home/osama/Documents/OS_Labs/Lab_01/logging.log"

void setup_environment() {
    char directory[M];
    chdir(getcwd(directory, sizeof(directory)));
}

void log_to_file(char* logon){
    FILE* logging;
    logging = fopen(LOG_FILE, "a");
    if(logging == NULL)
        return;

    time_t t;time(&t);
    fprintf(logging, "At %s", ctime(&t));
    fprintf(logging, "%s\n", logon);
    fclose(logging);
}

int get_command_type(char* command){
    return  strcmp(command, "cd") && 
            strcmp(command, "echo") && 
            strcmp(command, "export");
}

void reap_chile_process(){
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}

void handle_child_termination(){
    reap_chile_process();
    log_to_file("Child Killed Successfully ;)\n");
}

void register_child_signal(void (*handle_child_termination)(int)) {
    signal(SIGCHLD, handle_child_termination);
}

void change_directory(char** commands){

    char directory[M];

    // If we write no path (only 'cd'), then go to the home directory
	if(commands[1] == NULL || !strcmp(commands[1], "~"))
		chdir(getenv("HOME")); 

	else{ 
		if(chdir(commands[1]) == -1)
			printf("%s: no such directory\n", commands[1]);
	}

    if(getcwd(directory, sizeof(directory)) != NULL);
        printf("%s\n", directory);
}

void echo_command(char **commands){
    int i = 0;
    while(commands[++i] != NULL){
        if(commands[i][0] == '\"') 
            commands[i]++;
        if(commands[i][strlen(commands[i]) - 1] == '\"') 
            commands[i][strlen(commands[i]) - 1] = '\0';

        printf("%s ", commands[i]);
    }
    printf("\n");
}

void export_command(char** commands){
    char *key, *value;
    if((value = strchr(commands[1], '=')) != NULL){
        *value = '\0';
        
        key = commands[1];
        value++;
        if(*value == '\"')
            value++;
        commands[1] = value;

        for (int i = 2;commands[i] != NULL;i++){
            char spare[10] = {" "};
            strcat(spare, commands[i]);
            value = strcat(value, spare);
        }

        if(value[strlen(value) - 1] == '\"') 
            value[strlen(value) - 1] = '\0';

        printf("Value: %s\n", value);
        setenv(key, value, 1);
        printf("Key: %s\nValue stored: %s\n", key, getenv(key));
    }
}

void execute_built_in(char** commands){
    if(!strcmp(commands[0], "cd"))
        change_directory(commands);
    else if(!strcmp(commands[0], "echo"))
        echo_command(commands);
    else if(!strcmp(commands[0], "export"))
        export_command(commands);
}

void execute_command(char** commands){
    pid_t child_pid = fork();
    if(child_pid < 0){
        perror("Error creating child :(\n");
        exit(EXIT_FAILURE);
    }

    else if(child_pid == 0){
        if(commands[1] && !strcmp(commands[1], "&"))
            commands[1] = NULL;
        execvp(commands[0], commands);
        printf("Error ocurred");
        exit(EXIT_SUCCESS);
    }

    // else{
    // if it's foreground then wait
    if(!(commands[1] && !strcmp(commands[1], "&")))
        waitpid(child_pid, 0, 0);
    else if(!strcmp(commands[1], "&")) 
        commands[1] = NULL;
    // }
}


int main(){

    register_child_signal(handle_child_termination);

    do{
        char* commands[N] = {};
        char command[100];
    
        printf(">>> ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0';

        int i = 0, f = 0;
        char* word = strtok(command, " ");
        while(word != NULL){
            char *delimiter = strchr(word, '$');
            if(delimiter != NULL){
                delimiter++;
                word = getenv(delimiter);

                char* splitted_word = strtok(word, " ");
                while (splitted_word != NULL){
                    commands[i++] = splitted_word;
                    splitted_word = strtok(NULL, " ");
                }
            }
            else
                commands[i++] = word;
            word = strtok(NULL, " ");
        }

        if(!strcmp(commands[0], "exit"))
            break;

        else if(get_command_type(commands[0]))
            execute_command(commands);
        else 
            execute_built_in(commands);
        
    }   while(1);

    exit(0);
}
