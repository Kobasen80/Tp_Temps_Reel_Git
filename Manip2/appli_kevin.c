// Application prise en main périphérique
/************************************************************************/
// 14/09 : Allumage/extinction des leds A et B OK
// 14/09 : Allumage/extinction des leds 1 et 2
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

#define LED1_OFF (long)(1<<29)
#define LED1_ON (long)(0<<29)
#define LED2_OFF (long)(1<<31)
#define LED2_ON (long)(0<<31)

// definition de constantes pour l'application
#define Resolution 5000
#define MaxCount 100

// declaration des semaphores
sem_t semH;

// declaration des tid des taches
pthread_t tid_horloge;


// declaration des attributs des taches
pthread_attr_t attr_horloge;



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
// variable pour led
 long i, j, k, l;
 int fd1 = 0,fd2 = 0, fd3=0,  fd4=0;

 int res;

 printf("Application Modele multithread -  sept. 2012 - OP\n");
 printf("creation du semaphore semH\n");
 res = sem_init(&semH, 0, 0);
 if (res!=0) printf("erreur creation semH\n");

 

  // initialisation des attributs de la tache
  pthread_attr_init(&attr_horloge);
  res = pthread_create(&tid_horloge, 
                       &attr_horloge, 
                       (void *)&code_horloge, 
                       NULL);
  if (res != 0)
   printf("erreur demarrage horloge\n");
   else printf("demarrage horloge reussie\n");

 fd1 = open("/dev/gpio1",O_RDWR);
 fd2 = open("/dev/gpio2",O_RDWR);
 fd3 = open("/dev/gpio3",O_RDWR);
 fd4 = open("/dev/gpio4",O_RDWR);

  i=LEDA_OFF;
  write(fd1, &i, sizeof(long));

  j=LEDB_ON;
  write(fd2, &j, sizeof(long));

 k=LED1_ON;
  write(fd3, &j, sizeof(long));

 l=LED2_ON;
  write(fd4, &j, sizeof(long));

  while(1);
  printf("Attention le main se termine\n");

 return 0;
}
