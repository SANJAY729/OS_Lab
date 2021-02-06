#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 

#define BUFF_SIZE 1024
#define MAX_PIPE 100
#define MAX_LIST 100
#define MAX_FILENAME_LENGTH 15

struct parsed_command{
    char* input_file;
    char* output_file;
    char** pipe_command_arr;
    int pipes;
    int background;
    int append;
};

void initialize(){
    printf("\033[H\033[J");
    printf("################################################################################\n");
    printf("#                               COMMAND SYNTAX                                 #\n");
    printf("# To write to output file                                                      #\n");
    printf("#              cmd1<input_file|cmd2|cmd3|......|cmdk>output_file               #\n");
    printf("# To append to output file                                                     #\n");
    printf("#              cmd1<input_file|cmd2|cmd3|......|cmdk>>output_file              #\n");
    printf("#Absence of input/output file(s) will result in stdin/stdout being used        #\n");
    printf("################################################################################\n");
}
void printDir() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf("\n~%s$", cwd); 
} 
char* read_command(){
    char *buffer = (char*)malloc(sizeof(char) * BUFF_SIZE);
    printDir();
    buffer  = readline(" ");
    if(strlen(buffer) != 0)
        add_history(buffer);
    return buffer;
}

void trim(char *str){
    if (strlen(str) != 0){
        int i, begin = 0, end = strlen(str) - 1;
        while (isspace((unsigned char) str[begin]))
            begin++;
        while ((end >= begin) && isspace((unsigned char) str[end]))
            end--;
        for (i = begin; i <= end; i++)
            str[i - begin] = str[i];
        str[i - begin] = '\0';
    }
}

struct parsed_command* parse_command(char* command){
    struct parsed_command* command_data = (struct parsed_command*)malloc(sizeof(struct parsed_command));
    command_data->input_file = (char*)malloc(MAX_FILENAME_LENGTH * sizeof(char));
    command_data->output_file = (char*)malloc(MAX_FILENAME_LENGTH* sizeof(char));
    command_data->pipe_command_arr = (char**)malloc(MAX_PIPE * sizeof(char*));
    command_data->pipes = 0;
    command_data->background = 0;
    command_data->append = 0;
    if(command[strlen(command) - 1] == '&'){
        command_data->background = 1;
        command[strlen(command) - 1] = '\0';
    }
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
    trim(command_data->input_file); trim(command_data->output_file);
    if(command_data->pipes == 1 && command_data->input_file[strlen(command_data->input_file) - 1] == '>'){
        command_data->append = 1;
        command_data->input_file[strlen(command_data->input_file) - 1] = '\0';
        trim(command_data->input_file);
    }
    if(command_data->pipes > 1 && command_data->pipe_command_arr[command_data->pipes - 1][strlen(command_data->pipe_command_arr[command_data->pipes - 1]) - 1] == '>'){
        command_data->append = 1;
        command_data->pipe_command_arr[command_data->pipes - 1][strlen(command_data->pipe_command_arr[command_data->pipes - 1]) - 1] = '\0';
    }
    if(command_data->pipes == 1 && strlen(command_data->input_file) == 0 && command_data->pipe_command_arr[0][strlen(command_data->pipe_command_arr[0]) - 1] == '>'){
        command_data->append = 1;
        command_data->pipe_command_arr[0][strlen(command_data->pipe_command_arr[0]) - 1] = '\0';
    }
    return command_data;
}

void getArgs(char* str, char** Args){  
	for (int i = 0; i < MAX_LIST; i++){ 
		Args[i] = strsep(&str, " "); 
		if(Args[i] == NULL) 
			break; 
		if(strlen(Args[i]) == 0) 
			i--; 
	} 
}

void execArgs(char* str,int bg){
    char *Args[MAX_LIST];
    getArgs(str, Args);
	// Forking a child 
	pid_t pid = fork(); 

	if(pid == -1){ 
		printf("\nFailed forking child..."); 
		return; 
	} 
    else if(pid == 0){
		if (execvp(Args[0], Args) < 0)
			printf("\nCould not execute command \'%s\'",Args[0]);
		exit(0); 
	}
    if(bg==0)
        waitpid(pid,NULL,0);
    else
        printf("Process created with PID: %d\n",pid);
} 

void execArgs_input(char* str, char* input_file, int bg){
    int inp,saved_stdin = dup(0);
    inp = open(input_file, O_RDONLY);
    if (inp < 0){
        printf("Input File could not be opened");
        return;
    }
    dup2(inp,0);
    close(inp);
    execArgs(str,bg);
    dup2(saved_stdin,0);
}

void execArgs_output(char* str, char* output_file, int bg, int append){
    int outp,saved_stdout = dup(1);
    if (append == 1){
        outp = open(output_file, O_WRONLY | O_APPEND);
        if (outp < 0){
            printf("Output File cold not be opened");
            return;
        }
    }
    else 
        outp = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    dup2(outp,1);
    close(outp);
    execArgs(str,bg);
    dup2(saved_stdout,1);
}

void execArgs_input_output(char* str, char* input_file, char* output_file, int bg, int append){
    int inp,outp,saved_stdin = dup(0),saved_stdout = dup(1);
    inp = open(input_file, O_RDONLY);
    if ( inp < 0){
        printf("Input File could not be opened");
        return;
    }
    if (append == 1){
        outp = open(output_file, O_WRONLY | O_APPEND);
        if (outp < 0){
            printf("Output File cold not be opened");
            return;
        }
    }
    else 
        outp = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);;
    dup2(inp,0); dup2(outp,1);
    close(inp); close(outp);
    execArgs(str,bg);
    dup2(saved_stdout,1);
    dup2(saved_stdin,0);
}

void execArgs_pipe(char** pipes, int num_pipes,char* input_file, char* output_file,int bg,int append){
	int fd[2];
	pid_t pid;
	int i = 0, fdinp = 0, fdout = 1;				/* Backup */
    if (strlen(input_file)!=0){
        fdinp=open(input_file, O_RDONLY);
        if (fdinp < 0){
            printf("Input File could not be opened");
            return;
        }
    }
    if (strlen(output_file)!=0){
        if(append){
            fdout = open(output_file, O_WRONLY | O_APPEND);
        }
        else
            fdout = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        if(fdout < 0){
            printf("Output File could not be opened");
            return;
        }
    }
	while (i < num_pipes){
		pipe(fd);				/* Sharing bidiflow */
		if ((pid = fork()) == -1) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			dup2(fdinp, 0);
			if (i!=num_pipes-1)
				dup2(fd[1], 1);
            else
                dup2(fdout,1);
			close(fd[0]);
			char *Args[MAX_LIST];
            getArgs(pipes[i], Args);
            if (execvp(Args[0], Args) < 0)
			    printf("\nCould not execute command \'%s\'",Args[0]);
			exit(1);
		}
		else {
			waitpid(pid,NULL,0); 		/* Collect childs */
            //wait(NULL);
			close(fd[1]);
			fdinp = fd[0];
			i++;
		}
	}
}

int main(){
    char *command;
    struct parsed_command* command_data;
    initialize();
    while(1){
        command = read_command();
        if(strlen(command) == 0)
            continue;
        command_data = parse_command(command);
        /*
        printf("PARSED DETAILS\n");
        printf("input thingy is $%s$\n", command_data->input_file);
        printf("output thingy is $%s$\n", command_data->output_file);
        printf("To be run in background? $%d$\n", command_data->background);
        printf("Output Append? $%d$\n", command_data->append);
        printf("number of pipes is $%d$\n", command_data->pipes);
        for(int i = 0; i < command_data->pipes; i++)
            printf("pipe $%d$ is $%s$\n", i, command_data->pipe_command_arr[i]);
        printf("################################################################################\n");
        */
        if(command_data->pipes == 1 && strlen(command_data->input_file) == 0 && strlen(command_data->output_file) == 0) 
            execArgs(command_data->pipe_command_arr[0],command_data->background);
        else if(command_data->pipes == 1 && strlen(command_data->input_file) != 0 && strlen(command_data->output_file) == 0)
            execArgs_input(command_data->pipe_command_arr[0],command_data->input_file,command_data->background);
        else if(command_data->pipes == 1 && strlen(command_data->input_file) == 0 && strlen(command_data->output_file) != 0)
            execArgs_output(command_data->pipe_command_arr[0],command_data->output_file,command_data->background,command_data->append);
        else if(command_data->pipes == 1 && strlen(command_data->input_file) != 0 && strlen(command_data->output_file) != 0)
            execArgs_input_output(command_data->pipe_command_arr[0],command_data->input_file,command_data->output_file,command_data->background,command_data->append);
        else
            execArgs_pipe(command_data->pipe_command_arr,command_data->pipes,command_data->input_file,command_data->output_file,command_data->background, command_data->append);
        free(command_data);
    }
    return 0;
}