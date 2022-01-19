#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>


#define BUFFER_SIZE 64

int main(){


	const char welcome_msg[]="Bienvenue sur le shell de l'ENSEA\nPour quitter, tapez 'exit'\n";
	const char bye_msg[]="Bye bye...\n";
	const char exit_txt[]="exit";
	char acceuil_msg[]="enseash >> ";
	char error_cmd[]="commande non reconnue\n";

	char exit_zero[BUFFER_SIZE];
	char exit_signaled[BUFFER_SIZE];

	struct timespec time1;
	struct timespec time2;

	char buffer_input[BUFFER_SIZE];
	int ret;
	pid_t pid ;
	int status;
	int delta_time;
	
	write(STDOUT_FILENO,welcome_msg,strlen(welcome_msg));												// Note : STDOUT_FILENO : sortie fichier

	while(1){
		write(STDOUT_FILENO,acceuil_msg,strlen(acceuil_msg));

		ret=read(STDIN_FILENO,buffer_input,BUFFER_SIZE);												

		if(strncmp(buffer_input,exit_txt,strlen(exit_txt))==0||(ret==0)){								//on assure la terminaison avec la commande exit ou ctrl+D
			write(STDOUT_FILENO,bye_msg,strlen(bye_msg));
			break;
		}
		
		buffer_input[ret-1]=0;																			//On remplace le dernier caractere dans buffer_input pour que le execlp fonctionne

		// fabrication du tableau d'arguments de la ligne de commande

		char separateur[] = " ";
  		char *tableau[10];																				// Pour un premier code, on part du principe qu'il n'y a pas plus de 9 args dans la ligne de commande...
  		char *p = strtok(buffer_input, separateur);
  		int i=0;

  		while(p != NULL)																				// tant qu'il y a des caracteres dans buffer_input...
  		{
    		tableau[i]=malloc(strlen(p)*sizeof(char));													// ...on alloue la memoire necessaire à la case du tableau,...
    		strcpy(tableau[i],p);																		// ... et on y copie le parametre.
    		p = strtok(NULL, separateur);
    		i++;
  		}
  		tableau[i]=NULL;																				// il faut quer le dernier élément du tableau soit NULL pour que execv fonctionne

  		// fin de la construction du tableau d'arguemnts de la ligne de commande


  		// debut du procédé pere/fils

		pid=fork ();

		if(pid!=0){
			//father code
			wait(&status);
			
		}else{
			//children code
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&time1);												// on recupere le temps du debut de l'execution du fils

			//execlp(buffer_input,buffer_input,(char *)NULL);
			execvp(tableau[0],tableau);
			printf("tableau 0 : %s\n",tableau[0]);
  			printf("tableau 1 : %s\n",tableau[1]);
  			printf("tableau 2 : %s\n",tableau[2]);
			write(STDOUT_FILENO,error_cmd,strlen(error_cmd));											// on ecrit "erreur commande" si execlp n'a rien reconnu
			exit(EXIT_FAILURE);

		}
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&time2);													// on recupere le temps de fin d'execution du fils et on fait la soustraction des deux temps

		delta_time=((time2.tv_nsec)-(time1.tv_nsec));													// Note : time.nsec ou &time->nsec ATTENTION aux pointeurs (calcul du temps d'exec)


		if(WIFEXITED(status)){																			// Terminaison normale du fils
			sprintf(exit_zero,"[Exit : 0] | %d ns]\n",delta_time);
			write(STDOUT_FILENO,exit_zero,strlen(exit_zero));											
		}

		if(WIFSIGNALED(status)){																		// Terminaison du fils sur un signal
			int child_exit_status = WTERMSIG(status);													// on récupere le numéro du signal qui l'a terminé...
			sprintf(exit_signaled,"[signal : %d | %d ns]\n", child_exit_status,delta_time);							
			write(STDOUT_FILENO,exit_signaled,strlen(exit_signaled));									// ...et on l'affiche
		}
	}
}
