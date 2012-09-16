// exemple d'application utilisant les BPs et les LEDs
/************************************************************************/
/* L'application cree 2 threads, horloge et observation. Le premier     */
/* éveille le second à l'aide d'un sémaphore, semH. Le second fait      */
/* évoluer l'état des Leds A et B selon le nombre (pair ou impair)      */
/* d'appui sur le bouton Start et recopie l'etat des BP 1 et 2          */
/* respectivement sur les Led 1 et 2.                                   */
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

// definition des constantes permettant de recuperer l'etat des BP
#define STARTMASK (long)(1<<5)
#define BP1MASK (long)(1<<1)
#define BP2MASK (long)(1<<3)

// definition de constantes pour l'application
#define Duree_Etat 100

// declaration des semaphores
sem_t semH;

// declaration des tid des taches
pthread_t tid_horloge;
pthread_t tid_observation;

// declaration des attributs des taches
pthread_attr_t attr_horloge;
pthread_attr_t attr_observation;


// declaration du codes des taches
void code_observation(void)
{
 int t;
 long i, j, k, l;
 long start, bp1, bp2;
 int fd1 = 0,fd2 = 0, fd3 = 0, fd4 = 0, fd5 = 0, fd6 = 0, fd7 = 0;
 int EtatPrecBPStart, EtatStart;
 int valeur;


 printf("debut tache observation\n");
 // ouverture du flot pour LEDA
 fd1 = open("/dev/gpio1",O_RDWR);
 if (fd1<0) printf("erreur creation fd1\n");
  else printf("ouverture gpio1 OK\n");
 // ouverture du flot pour LEDB
 fd2 = open("/dev/gpio2",O_RDWR);
 if (fd2<0) printf("erreur creation fd2\n");
  else printf("ouverture gpio2 OK\n");
 // ouverture du flot pour LED1
 fd3 = open("/dev/gpio3",O_RDWR);
 if (fd3<0) printf("erreur creation fd3\n");
  else printf("ouverture gpio3 OK\n");
 // ouverture du flot pour LED2
 fd4 = open("/dev/gpio4",O_RDWR);
 if (fd4<0) printf("erreur creation fd4\n");
  else printf("ouverture gpio4 OK\n");
 // ouverture du flot pour Start
 fd5 = open("/dev/gpio5",O_RDWR|O_NONBLOCK); // ajouter O_NONBLOCK pour rendre le read non bloquant
 if (fd5<0) printf("erreur creation fd5\n");
  else printf("ouverture gpio5 OK\n");
 // ouverture du flot pour BP1
 fd6 = open("/dev/gpio6",O_RDWR|O_NONBLOCK); // ajouter O_NONBLOCK pour rendre le read non bloquant
 if (fd6<0) printf("erreur creation fd6\n");
  else printf("ouverture gpio6 OK\n");
 // ouverture du flot pour BP2
 fd7 = open("/dev/gpio7",O_RDWR|O_NONBLOCK); // ajouter O_NONBLOCK pour rendre le read non bloquant
 if (fd7<0) printf("erreur creation fd7\n");
  else printf("ouverture gpio7 OK\n");
 EtatPrecBPStart = 0;
 EtatStart = 0;
 valeur = 0;
 t=0;
 while(1)
  {
   sem_wait(&semH);
   // lecture de l'etat de Start
   read(fd5, &start, sizeof(long));
   if (!(start & STARTMASK))
    {
     if (EtatPrecBPStart == 0)
      {
       EtatPrecBPStart = 1;
       EtatStart = !EtatStart;
      }
    }
    else
     {
     if (EtatPrecBPStart == 1)
      {
       EtatPrecBPStart = 0;
      }
     }
   if (fd1 && fd2 && EtatStart)
    {
     // la gestion des LEDA et LEDB est fonction de l'etat du bouton Start
     if (t >= Duree_Etat )
      {
       t = 0;
       switch (valeur)
        {
         case 0: i=LEDA_OFF;
                 j=LEDB_OFF;
                 break;
         case 1: i=LEDA_ON;
                 j=LEDB_OFF;
                 break;
         case 2: i=LEDA_OFF;
                 j=LEDB_ON;
                 break;
         case 3: i=LEDA_ON;
                 j=LEDB_ON;
                 break;
        }
       write(fd1, &i, sizeof(long));
       write(fd2, &j, sizeof(long));
       valeur = (valeur +1)%4;
      }
      else t++;
    }
   // gestion de BP1
   read(fd6, &bp1, sizeof(long));
   if (!(bp1 & BP1MASK))
    k = LED1_ON;
    else k = LED1_OFF;
   if (fd3)
    write(fd3, &k, sizeof(long));
   // gestion de BP2
   read(fd7, &bp2, sizeof(long));
   if (!(bp2 & BP2MASK))
    l = LED2_ON;
    else l = LED2_OFF;
   if (fd4)
    write(fd4, &l, sizeof(long));
  }
  printf("fin de la tache observation\n");
}

void code_horloge(void)
{
 printf("debut tache horloge\n");
 while(1)
  {
   usleep(5000);
   sem_post(&semH);
  }
  printf("fin de la tache horloge\n");
}

int main(int argc, char** argv)
{
 int res;


 printf("Application Modele pour BPs et Leds - sept. 2012 - OP\n");
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
  else printf("demarrage horloge reussi\n");

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
  else printf("demarrage observation reussi\n");
 while(1);
 printf("Attention le main se termine\n");
 return 0;
}
