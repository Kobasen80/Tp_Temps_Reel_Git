// Modele d'application multi-thread
/************************************************************************/
/* L'application cree 2 threads, horloge et observation. Le premier     */
/* �veille le second � l'aide d'un s�maphore, semH. Le second fait      */
/* �voluer l'�tat des Leds A et B                                       */
/************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// inclusion des bibliotheques pour la gestion des threads
#include <pthread.h>

// inclusion de la bibliotheque pour la gestion des timers
#include <time.h>

// inclusion de la bibliotheque pour les semaphores
#include <semaphore.h>

// definition des constantes permettant de controler l'etat des LED
#define LEDA_OFF (long)(1<<19)
#define LEDA_ON (long)(0<<19)
#define LEDB_OFF (long)(1<<19)
#define LEDB_ON (long)(0<<19)

// definition de constantes pour l'application
#define Resolution 5000
#define MaxCount 100

// declaration des semaphores
sem_t semH;

// declaration des tid des taches
pthread_t tid_horloge;
pthread_t tid_observation;

// declaration des attributs des taches
pthread_attr_t attr_horloge;
pthread_attr_t attr_observation;

//definition d'un type pour l'etat
typedef enum {Etat0,Etat1,Etat2,Etat3} Def_State;

// declaration du codes des taches
void code_observation(void)
{
 long i, j;
 int fd1 = 0,fd2 = 0;
 int count;
 Def_State State;


 printf("debut tache observation\n");
 fd1 = open("/dev/gpio1",O_RDWR);
 if (fd1<0) printf("erreur creation fd1\n");
  else printf("ouverture gpio1 OK\n");
 fd2 = open("/dev/gpio2",O_RDWR);
 if (fd2<0) printf("erreur creation fd2\n");
  else printf("ouverture gpio2 OK\n");
 State = Etat0;
 count=MaxCount;
 while(1)
  {
   sem_wait(&semH);
   if (count == MaxCount)
   {
    count = 0;
    switch(State)
     {
	  case Etat0: State = Etat1;
                  i=LEDA_ON;
                  j=LEDB_OFF;
	  break;
	  case Etat1: State = Etat2;
                  i=LEDA_OFF;
                  j=LEDB_ON;
	  break;
	  case Etat2: State = Etat3;
                  i=LEDA_ON;
                  j=LEDB_ON;
	  break;
	  case Etat3: State = Etat0;
                  i=LEDA_OFF;
                  j=LEDB_OFF;
	  break;
	 }
    if (fd1 && fd2)
     {
      write(fd1, &i, sizeof(long));
      write(fd2, &j, sizeof(long));
     }
   }
   else count++;
  }
  printf("fin de la tache observation\n");
}

void code_horloge(void)
{
 printf("debut tache horloge\n");
 while(1)
  {
   usleep(Resolution);
   sem_post(&semH);
  }
  printf("fin de la tache horloge\n");
}

int main(int argc, char** argv)
{
 int res;

 printf("Application Modele multithread -  sept. 2012 - OP\n");
 printf("creation du semaphore semH\n");
 res = sem_init(&semH, 0, 0);
 if (res!=0) printf("erreur creation semH\n");

  // creation de tache horloge
  printf("demarrage de la tache horloge\n");
  // initialisation des attributs de la tache
  pthread_attr_init(&attr_horloge);
  res = pthread_create(&tid_horloge, 
                       &attr_horloge, 
                       (void *)&code_horloge, 
                       NULL);
  if (res != 0)
   printf("erreur demarrage horloge\n");
   else printf("demarrage horloge reussie\n");

  // creation de tache observation
  printf("demarrage de la tache observation\n");
  // initialisation des attributs de la tache
  pthread_attr_init(&attr_observation);
  res = pthread_create(&tid_observation,
                       &attr_observation, 
                       (void *)&code_observation, 
                       NULL);
  if (res != 0)
   printf("erreur demarrage observation\n");
   else printf("demarrage observation reussie\n");

  while(1);
  printf("Attention le main se termine\n");
 return 0;
}
