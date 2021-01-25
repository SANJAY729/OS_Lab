#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 

#define BUFF_SIZE 1024
#define MAX_PIPE 100
#define MAXLIST 100
#define MAX_FILENAME_LENGTH 15

struct parsed_command{
    char* input_file;
    char* output_file;
    char** pipe_command_arr;
    int pipes;
};

char* read_command(){
    char *buffer = (char*)malloc(sizeof(char) * BUFF_SIZE);
    buffer  = readline("\n$topper$");
    if(strlen(buffer) != 0)
        add_history(buffer);
    return buffer;
}

struct parsed_command* parse_command(char* command){
    int n = strlen(command);
    struct parsed_command* command_data = (struct parsed_command*)malloc(sizeof(struct parsed_command));
    command_data->input_file = (char*)malloc(MAX_FILENAME_LENGTH * sizeof(char));
    command_data->output_file = (char*)malloc(MAX_FILENAME_LENGTH* sizeof(char));
    command_data->pipe_command_arr = (char**)malloc(MAX_PIPE * sizeof(char*));
    command_data->pipes = 0;
    for (int i = 0; i < MAX_PIPE; i++){
        command_data->pipe_command_arr[i] = strsep(&command, "|");
        if(command_data->pipe_command_arr[i] == NULL)
            break;
        command_data->pipes++;
    }
    if(command_data->pipes == 1){ // no pipe
        int inp = - 1, outp = -1;
        for (int i = 0; i < strlen(command_data->pipe_command_arr[0]); i++){
            if(command_data->pipe_command_arr[0][i] == '<')
                inp = i;
            if(command_data->pipe_command_arr[0][i] == '>')
                outp = i;
        }
        if(inp != -1 && outp != -1 && outp > inp){
            strncpy(command_data->input_file,command_data->pipe_command_arr[0] + inp + 1,outp - 1 - inp);
            command_data->input_file[outp - 1 - inp] = '\0';
            strncpy(command_data->output_file,command_data->pipe_command_arr[0] + outp + 1,strlen(command_data->pipe_command_arr[0]) - 1 - outp);
            command_data->output_file[strlen(command_data->pipe_command_arr[0]) - 1 - outp] = '\0';
            command_data->pipe_command_arr[0][inp] = '\0';
        }
        if(inp == -1 && outp != -1){
            strncpy(command_data->output_file,command_data->pipe_command_arr[0] + outp + 1,strlen(command_data->pipe_command_arr[0]) - 1 - outp);
            command_data->output_file[strlen(command_data->pipe_command_arr[0]) - 1 - outp] = '\0';
            command_data->input_file = "\0";
            command_data->pipe_command_arr[0][outp] = '\0';
        }
        if(inp != -1 && outp == -1){
            strncpy(command_data->input_file,command_data->pipe_command_arr[0] + inp + 1,strlen(command_data->pipe_command_arr[0]) - 1 - inp);
            command_data->input_file[strlen(command_data->pipe_command_arr[0]) - 1 - inp] = '\0';
            command_data->output_file = "\0";
            command_data->pipe_command_arr[0][inp] = '\0';
        }
        if(inp == -1 && outp == -1){
            command_data->input_file = "\0";
            command_data->output_file = "\0";
        }
    }
    else{ // pipe is present
        int inp = - 1, outp = -1;
        for (int i = 0; i < strlen(command_data->pipe_command_arr[0]); i++)
            if(command_data->pipe_command_arr[0][i] == '<')
                inp = i;
        for (int i = 0; i < strlen(command_data->pipe_command_arr[command_data->pipes - 1]); i++)
            if(command_data->pipe_command_arr[command_data->pipes - 1][i] == '>')
                outp = i;
        if(inp != -1){
            strncpy(command_data->input_file,command_data->pipe_command_arr[0] + inp + 1,strlen(command_data->pipe_command_arr[0]) - 1 - inp);
            command_data->input_file[strlen(command_data->pipe_command_arr[0]) - 1 - inp] = '\0';
            command_data->pipe_command_arr[0][inp] = '\0';
        }
        else
            command_data->input_file = "\0";
        if(outp != -1){
            strncpy(command_data->output_file,command_data->pipe_command_arr[command_data->pipes - 1] + outp + 1,strlen(command_data->pipe_command_arr[command_data->pipes - 1]) - 1 - outp);
            command_data->output_file[strlen(command_data->pipe_command_arr[command_data->pipes - 1]) - 1 - outp] = '\0';
            command_data->pipe_command_arr[command_data->pipes - 1][outp] = '\0';
        }
        else
            command_data->output_file = "\0";
    }
    return command_data;
}
void parseSpace(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < MAXLIST; i++) { 
		parsed[i] = strsep(&str, " "); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 
void execArgs(char* str) 
{
    char *parsed[MAXLIST];
    parseSpace(str, parsed);
	// Forking a child 
	pid_t pid = fork(); 

	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return; 
	} else if (pid == 0) { 
		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command \'%s\'",parsed[0]); 
		} 
		exit(0); 
	} else { 
		// waiting for child to terminate 
		wait(NULL); 
		return; 
	} 
} 

int main(){
    char *command;
    struct parsed_command* command_data;
    printf("\033[H\033[J");
    while(1){
        command = read_command();
        if(strlen(command) == 0)
            continue;
        command_data = parse_command(command);
        printf("PARSED DETAILS\n");
        printf("input thingy is $%s$\n", command_data->input_file);
        printf("output thingy is $%s$\n", command_data->output_file);
        printf("number of pipes is $%d$\n", command_data->pipes);
        for(int i = 0; i < command_data->pipes; i++)
            printf("pipe $%d$ is $%s$\n", i, command_data->pipe_command_arr[i]);
        if(command_data->pipes==1){
            execArgs(command_data->pipe_command_arr[0]);
        }
    }
    return 0;
}