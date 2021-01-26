#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(){
    char v[10] = "vamshi";
    char ** d = (char**)malloc(10*sizeof(char*));
    d[3] = v;
    printf("lol %s\n", d[3]);
    printf("%s\n",v);
    char * x  =  v+3;
    printf("%s\n", x);
    char s[100] = "   hello vamshi   ";
    printf("%s\n",strstrip(s));
    return 0;
}

           /* for(int i = inp + 1; i < outp; i++){
                command_data->input_file[i-(inp + 1)] = command_data->pipe_command_arr[0][i];
            }
            command_data->input_file[outp - inp - 1] = '\0';
            for(int i = outp + 1; i < strlen(command_data->pipe_command_arr[0]); i++){
                command_data->output_file[i - outp - 1] = command_data->pipe_command_arr[0][i];
            }
            command_data->output_file[strlen(command_data->pipe_command_arr[0])-]*/