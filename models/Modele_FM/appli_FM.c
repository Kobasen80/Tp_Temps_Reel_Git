// exemple d'application utilisant les files de messages  et la récupération des erreurs
/***************************************************************************************/
/* L'application cree 3 threads, horloge observation et affichage. Le premier éveille  */
/* le second à l'aide d'un sémaphore, semH. Le second fait évoluer l'état des Leds 1 et*/
/* 2 selon l'etat des BP 1 et 2 et en cas de changement de l'état du BP transmet un    */
/*message à affiche par FileM. Lorsqu'il reçoit un message, affiche l'affiche sur le   */
/* terminal associé à la carte                                                         */
/***************************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// inclusion de la bibliothèque pour le traitement des erreurs et déclaration de la variable globale permettant la récupération des erreurs
#include <errno.h>
extern int errno;

// inclusion des bibliotheques pour la gestion des threads
#include <pthread.h>

// inclusion des bibliotheques pour les files de message
#include <mqueue.h>

// inclusion de la bibliotheque pour la gestion des timers
#include <time.h>

// inclusion de la bibliotheque pour les semaphores
#include <semaphore.h>

// definition des constantes permettant de controler l'etat des LED
#define LED1_OFF (long)(1<<29)
#define LED1_ON (long)(0<<29)
#define LED2_OFF (long)(1<<31)
#define LED2_ON (long)(0<<31)

// definition des constantes permettant de recuperer l'etat des BP
#define BP1MASK (long)(1<<1)
#define BP2MASK (long)(1<<3)

// definition de constantes pour l'ientification de l'Ã©tat des BP
#define Appui 0
#define Relache 1

// definition de constantes pour la FM
#define MsgLength 100
#define NbMess 10

// declaration des semaphores
sem_t semH;

// declaration des files de message
mqd_t FileM;

// declaration des tid des taches
pthread_t tid_horloge;
pthread_t tid_observation;
pthread_t tid_affichage;

// declaration des attributs des taches
pthread_attr_t attr_horloge;
pthread_attr_t attr_observation;
pthread_attr_t attr_affichage;

// declaration du codes des taches
void code_observation(void)
{
 char Message[MsgLength-2];
 int res;
 long k, l;
 long bp1, bp2;
 int fd3 = 0, fd4 = 0, fd6 = 0, fd7 = 0;
 int EtatPrecBP1, EtatBP1, EtatPrecBP2, EtatBP2;


 printf("debut tache observation\n");
 // ouverture du flot pour LED1
 fd3 = open("/dev/gpio3",O_RDWR);
 if (fd3<0) printf("erreur creation fd3\n");
  else printf("ouverture gpio3 OK\n");
 // ouverture du flot pour LED2
 fd4 = open("/dev/gpio4",O_RDWR);
 if (fd4<0) printf("erreur creation fd4\n");
  else printf("ouverture gpio4 OK\n");
 // ouverture du flot pour BP1
 fd6 = open("/dev/gpio6",O_RDWR|O_NONBLOCK);
 if (fd6<0) printf("erreur creation fd6\n");
  else printf("ouverture gpio6 OK\n");
 // ouverture du flot pour BP2
 fd7 = open("/dev/gpio7",O_RDWR|O_NONBLOCK);
 if (fd7<0) printf("erreur creation fd7\n");
  else printf("ouverture gpio7 OK\n");

 // initialisation des variables locales
 EtatBP1 = Relache;
 EtatBP2 = Relache;
 EtatPrecBP1 = Relache;
 EtatPrecBP2 = Relache;

 while(1)
  {
   sem_wait(&semH);
   // gestion de BP1
   read(fd6, &bp1, sizeof(long));
   if (!(bp1 & BP1MASK))
    {
     k = LED1_ON;
     EtatBP1 = Appui;
    }
    else
     {
      k = LED1_OFF;
      EtatBP1 = Relache;
     }
   // gestion de BP2
   read(fd7, &bp2, sizeof(long));
   if (!(bp2 & BP2MASK))
    {
     l = LED2_ON;
     EtatBP2 = Appui;
    }
    else
     {
      l = LED2_OFF;
      EtatBP2 = Relache;
     }
   // affichage de BP1 sur Led1
   if (fd3)
    write(fd3, &k, sizeof(long));
   // affichage de BP2 sur Led2
   if (fd4)
    write(fd4, &l, sizeof(long));
   // emission de messages si l'etat des BP a change
   if (EtatBP1 != EtatPrecBP1)
    {
     EtatPrecBP1 = EtatBP1;
     if (EtatBP1 == Appui) sprintf(Message, "BP1 appuye");
      else sprintf(Message, "BP1 relache");
     res = mq_send(FileM, Message, sizeof(Message), 0);
     if (res == 0) printf("message transmis par observation\n");
      else 
       {
        printf("erreur transmission message par observation avec erreur ");
        switch(errno)
         {
          case EBADF: printf("EBADF");
                      break;
          case EMSGSIZE: printf("EMSGSIZE");
                         break;
          case EAGAIN: printf("EAGAIN");
                       break;
          case EPERM: printf("EPERM");
                      break;
          case EINTR: printf("EINTR");
                      break;
          }
         printf("\n");
        }
    }
   if (EtatBP2 != EtatPrecBP2)
    {
     EtatPrecBP2 = EtatBP2;
     if (EtatBP2 == Appui) sprintf(Message, "BP2 appuye");
	  else sprintf(Message, "BP2 relache");
     res = mq_send(FileM, Message, sizeof(Message), 0);
     if (res == 0) printf("message transmis par observation\n");
      else 
       {
        printf("erreur transmission message par observation avec erreur ");
        switch(errno)
         {
          case EBADF: printf("EBADF");
                      break;
          case EMSGSIZE: printf("EMSGSIZE");
                         break;
          case EAGAIN: printf("EAGAIN");
                       break;
          case EPERM: printf("EPERM");
                      break;
          case EINTR: printf("EINTR");
                      break;
          }
         printf("\n");
        }
    }
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

void code_affichage(void)
{
 struct sigevent FileEveil; // semaphore pour notification par FM
 char message[MsgLength];
 int res;
 struct mq_attr fileattr;

 printf("debut tache affichage\n");
 res = mq_getattr(FileM, &fileattr);
 if (res != 0)
  {
   printf("erreur de lecture des attributs de FileM avec erreur ");
   switch(errno)
    {
     case EBADF: printf("EBADF");
                 break;
     case EMSGSIZE: printf("EMSGSIZE");
                    break;
     case EAGAIN: printf("EAGAIN");
                  break;
     case EPERM: printf("EPERM");
                 break;
     case EINTR: printf("EINTR");
                 break;
    }
   printf("\n");
  }
  else 
   {
    printf("attributs de la file de messages\n flags : %d\n message : %d\nsize : %d\nnbmess : %d\n",
           (int)fileattr.mq_flags,
           (int)fileattr.mq_maxmsg,
           (int)fileattr.mq_msgsize,
           (int)fileattr.mq_curmsgs);
   }
 // preparation du signal pour le lier a la file de message ce qui permet d'effectuer des attentes passives
 FileEveil.sigev_notify = SIGEV_SIGNAL;
 res = mq_notify(FileM, &FileEveil);
 if (res == 0) printf("notify de FileM OK\n");
  else printf("pb lors du notify de FileM\n");

 while(1)
  {
   // attente d'un message
   res = mq_receive(FileM, message, MsgLength, 0);
   if (res < 0)
    {
     printf("erreur reception du message\n");
     switch(errno)
      {
       case EBADF: printf("EBADF");
                   break;
       case EMSGSIZE: printf("EMSGSIZE");
                      break;
       case EAGAIN: printf("EAGAIN");
                    break;
       case EPERM: printf("EPERM");
                   break;
       case EINTR: printf("EINTR");
                   break;
      }
     printf("\n");
    }
    else printf("message recu \"%s\" de taille %d\n", message, res);
   //printf("affiche eveillee\nnb caracteres recus %d\n message : %s\n", res, message);
  }
  printf("fin de la tache affichage\n");
}

int main(int argc, char** argv)
{
 int res;
 struct mq_attr fileattr;
 mode_t mode;

 printf("Application modele FM - sept. 2012 - OP\n");
 printf("creation du semaphore semH\n");
 res = sem_init(&semH, 0, 0);
 if (res!=0) printf("erreur creation semH\n");
  else printf("creation semH Ok\n");

 printf("creation de la file de message\n");
 // definition des attributs nombre maximum de messages dans la FM et taille des messages
 fileattr.mq_maxmsg = NbMess;
 fileattr.mq_msgsize = MsgLength-2;
 FileM = mq_open("/filemess1", (O_RDWR|O_CREAT|O_EXCL), mode, (struct mq_attr *)&fileattr, NULL);
 if (FileM==-1)
  {
   printf("erreur creation FileM avec erreur ");
   switch(errno)
    {
     case ENAMETOOLONG: printf("ENAMETOOLONG");
                        break;
     case EEXIST: printf("EEXIST");
                  break;
     case ENOENT: printf("ENOENT");
                  break;
     case ENOSPC: printf("ENOSPC");
                  break;
     case EPERM: printf("EPERM");
                 break;
     case EINVAL: printf("EINVAL");
                  break;
     case EMFILE: printf("EMFILE");
                  break;
     default: printf("inconnue");
    }
   printf("\n");
  }
  else printf("creation FileM OK\n");
 // lecture des attributs de la file de messages pour verification
 res = mq_getattr(FileM, &fileattr);
 if (res == 0) printf("attributs de la file de messages\n flags : %d\n message : %d\nsize : %d\nnbmess : %d\n",
                      (int)fileattr.mq_flags,
                      (int)fileattr.mq_maxmsg,
                      (int)fileattr.mq_msgsize,
                      (int)fileattr.mq_curmsgs);
  else printf("erreur lecture attribut FileM\n");

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

 // creation de tache affichage
 printf("demarrage de la tache affichage\n");
 // initialisation des attributs de la tache
 pthread_attr_init(&attr_affichage);
 res = pthread_create(&tid_affichage,
                      &attr_affichage, 
                      (void *)&code_affichage, 
                      NULL);
 if (res != 0)
  printf("erreur demarrage affichage\n");
  else printf("creation demarrage reussi\n");
 while(1);
 printf("Attention le main se termine\n");
 return 0;
}
