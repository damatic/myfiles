/*
#################################################################
#																#	
#		Description: Linux ljuska u c programskom jeziku		#
#		Author: Danijel Matic									#
#		Unversity: Sveucilisni odjel za strucne studije			#
#																#
#																#
#																#
#################################################################
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "builtin.h"
#include "pipe.h"

#define BUFFER_LENGTH 1024 // velicina buffera
#define BUFFER_SIZE_FOR_NAMES 50
#define ARGUMENT_SIZE 50

typedef struct init{
	char program_path[PATH_MAX];
	char hostname_file[PATH_MAX];
	char hostname[BUFFER_SIZE_FOR_NAMES];
	char username[BUFFER_SIZE_FOR_NAMES];
	char line[BUFFER_LENGTH];
	char* argv1[BUFFER_SIZE_FOR_NAMES];
	char* argv2[BUFFER_SIZE_FOR_NAMES];
	char* token;
	int child_status;
	unsigned argc1;
	unsigned argc2;
	size_t length;
	char line_builtin[BUFFER_LENGTH];
	unsigned count_pipe;
	char temp_buffer[BUFFER_LENGTH];
	pid_t pid;
	
}init;

init* init_shell()
{
	init* data;
	
	data = (init*)malloc(sizeof(init));
	
	strcpy(data->program_path, "/home/matic/shell/commands/");  // korisnik postaviti svoju putanju!!!!
	strcpy(data->hostname_file, "/home/matic/shell/hostname_test");
	strcpy(data->hostname, "slash");
	strcpy(data->username, "matic");
	data->line[0] = '\0';
	
	return data;
}

void cleaning(init* data)
{
	free(data);
	data = NULL;
	
	init_shell(data);
}


int main()
{
	init* data;
	
	while(1){
		//printf("START\n");
		while(1){
			data = init_shell();
			init_hostname(data->hostname, data->hostname_file);
			//strcpy(program_path, path);
		
			printf("%s@%s >> ", data->username, name(data->hostname_file));
			
			if (fgets(data->line, BUFFER_LENGTH, stdin) == 0){ // u slucaju da je ^D End-Of-File
				printf("exit\n");
				return 1;
			}
			
			strcpy(data->temp_buffer, data->line);
			if (strtok(data->temp_buffer, " \t\n") == NULL) { // prazna komanda
				printf("empty command!!!\n");
				break;
			}
			
			history_shell(data->line);
			
			data->length = strlen(data->line);
			if (data->line[data->length - 1] == '\n'){ // brise oznaku za novi red
				data->line[data->length - 1] = '\0';
			}
			
			strcpy(data->line_builtin, data->line); // posebno za builtin programe, da ne dira izvorni string
			if(check_builtin(data->line_builtin) == 1){
				cleaning(data);
				break;
			}
			
			data->token = strtok(data->line, " \n\t()<>&;");
			while(data->token != NULL && data->argc1 < 256 && data->argc2 < 256){
				if(strcmp(data->token, "|") == 0){
					data->count_pipe = 1;
				}else{
					if(data->count_pipe == 0){
						data->argv1[data->argc1] = data->token;
						data->argc1++;
					}
					if(data->count_pipe == 1){
						data->argv2[data->argc2] = data->token;
						data->argc2++;
					}
				}
				data->token = strtok(NULL, " \n\t()<>&;"); // " \n\t()<>|&;" znakovi koje "ignorira"
			}

			if(data->count_pipe > 0){
				data->argv1[data->argc1] = NULL;
				data->argv2[data->argc2] = NULL;

				execArgsPiped(data->argv1, data->argv2);
				cleaning(data);
				break;
			}
			
			data->argv1[data->argc1] = NULL; // potrebno zbog execvp, da se zna gdje je kraj niza argumenata
			strcat(data->program_path, data->argv1[0]); // spajanje putanje gdje se nalaze programi i samog imena programa
			printf("%s %s\n", data->program_path, data->argv1[0]);
			if((data->pid = fork()) == -1){
				cleaning(data);
				break;
			}
			
			if(data->pid == 0){ // child process
				if (execvp(data->program_path, data->argv1) == -1){
						printf("ERRNO: %s\n", strerror(errno));
						exit(1);
				}
			}else{ // parrent process
			// cekanje da dijete proces zavrsi sa dodatnim informacijama, pomocu MACRO-a provjera
				wait(NULL);  // wait(&child_status); 
				//printf("Child exited\n");
				cleaning(data);
			}
			cleaning(data);
		}
	}
	return 0;
}





















