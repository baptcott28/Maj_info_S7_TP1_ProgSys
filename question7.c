#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>


#define BUFFER_SIZE 64
#define nom_fichier_size 20

int main(){


	const char welcome_msg[]="Bienvenue sur le shell de l'ENSEA\nPour quitter, tapez 'exit'\n";
	const char bye_msg[]="Bye bye...\n";
	const char exit_txt[]="exit";
	char acceuil_msg[]="enseash >> ";
	char error_cmd[]="commande non reconnue\n";

	char exit_zero[BUFFER_SIZE];
	char exit_signaled[BUFFER_SIZE];
	char nom_fichier[nom_fichier_size];

	struct timespec time1;
	struct timespec time2;

	char buffer_input[BUFFER_SIZE];
	int ret;
	pid_t pid ;
	int status;
	int delta_time;
	int file_descriptor;
	
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
  		char *tableau[10]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};							// Pour un premier code, on part du principe qu'il n'y a pas plus de 9 args dans la ligne de commande...
  		char *p = strtok(buffer_input, separateur);
  		int i=0;

  		while(p != NULL)																				// tant qu'il y a des caracteres dans buffer_input...
  		{
    		tableau[i]=malloc(strlen(p)*sizeof(char));													// ...on alloue la memoire necessaire à la case du tableau,...
    		strcpy(tableau[i],p);
    		strcat(tableau[i],"\0");																	// ... et on y copie le parametre.
    		p = strtok(NULL, separateur);
    		i++;
  		}

  		// fin de la construction du tableau d'arguemnts de la ligne de commande


  		// debut du procédé pere/fils

		pid=fork ();

		if(pid!=0){
			//father code
			wait(&status);
			
		}else{
			//children code
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&time1);												// on recupere le temps du debut de l'execution du fils

			// Ouverture d'in fichier si il y a une redirection dans la commande
			for(int i=1; i<10;i++){																		// on s'autorise à commencer à 1 car la commande ne commencera jamais par ">"

				if(tableau[i]==NULL){																	// Dès qu'une case == NULL => on n'a pas trouvé de chevron (sinon on serai sorti avec le breaf du if l90)...
					break;																				// ...donc on sort de la boucle for car strcmp(NULL,">") = seg fault
				}	
																										
				if(strcmp(tableau[i],">")==0){															// redirection dans la commande ?

					strcat(nom_fichier,tableau[i+1]);
					file_descriptor=open(nom_fichier,O_CREAT|O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);			// ouverture du fichier, le crée si il n'existe pas et accorde les droits RWX pour user et groupe et X pour other
					printf("fichier ouvert : %d\n", file_descriptor);
					dup2(STDOUT_FILENO,file_descriptor);												// on change le descripteur de fichier pour que le résultat de exedvp soit retourné dans le fichier voulu et pas dans le shell
					break;
				}
			}
			
			execvp(tableau[0],tableau);																	// le fils execute la commande contenue dans tableau

			printf("tableau 0 : %s\n",tableau[0]);														// sinon il affiche quelques elements du tableau pour que l'utilisateur controle le decoupage de sa commande 
  			printf("tableau 1 : %s\n",tableau[1]);
  			printf("tableau 2 : %s\n",tableau[2]);

			write(STDOUT_FILENO,error_cmd,strlen(error_cmd));											// on ecrit "erreur commande" si execvp n'a rien reconnu
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
