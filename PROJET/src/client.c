#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>	
#include <unistd.h>
#include "myassert.h"

#include "master_client.h"

// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<nombre>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombres premiers en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit être >= 2");
    }       
    
    return order;
}


/************************************************************************
 * Mes fonctions & utils
 ************************************************************************/


typedef struct{
    int order;
    int n;

    int semtid;
    int semsid;

    int pReadMaster;
    int pWriteMaster;
} dataC;

void initClient(dataC *data){
    //    - entrer en section critique :
    data->semtid = mysemget(CLE_SEM_TUBE, 1, 0); //semaphore named pipe
    mysemop(data->semtid, -1);

    data->semsid = mysemget(CLE_SEM_STOP, 1, 0); //semaphore release master

    //    - ouvrir les tubes nommés (ils sont déjà créés par le master), les ouvertures sont bloquantes, il faut s'assurer que le master ouvre les tubes dans le même ordre
    data->pReadMaster = myopen(PIPE_MTC, O_RDONLY);
    data->pWriteMaster = myopen(PIPE_CTM, O_WRONLY);
}


void sendOrderAndData(dataC data){
    mywrite(data.pWriteMaster, &data.order, sizeof(int)); //    - envoyer l'ordre et les données éventuelles au master

    if(data.order == ORDER_COMPUTE_PRIME){ //Send a 2nd arg
        mywrite(data.pWriteMaster, &(data.n), sizeof(int));
    }
}

void receiveAnswer(dataC data){
    if(data.order == ORDER_COMPUTE_PRIME){ //Receive a bool
        bool mRep;
        myread(data.pReadMaster, &mRep, sizeof(bool));
        printf("Reponse : %d %s premier\n", data.n, mRep ? "est" : "n'est pas");
    }
    else{ // Receive an int
        int mRep;
        myread(data.pReadMaster, &mRep, sizeof(int));
        if(data.order == ORDER_STOP){
            myassert(mRep == ORDER_STOP, "bad return");
        }
        else{
            printf("Reponse : %d\n", mRep);
        }
    }
}

void closeClient(dataC data){
    mysemop(data.semtid, 1); //    - sortir de la section critique
        
    myclose(data.pWriteMaster); //    - libérer les ressources (fermeture des tubes, ...)
    myclose(data.pReadMaster);
        
    mysemop(data.semsid, 1); //    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
}

typedef struct{
    bool *tab;
    int i;
    int n;
    pthread_mutex_t *mutex;
} dataThread;

void * codeThread(void * arg){
    dataThread *data = (dataThread *) arg;

    pthread_mutex_lock(data->mutex);
    for(int j = 2 * data->i; j <= data->n; j+=data->i){
        (data->tab)[j-2] = false;
    }
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

void sieveOfEratosthenes(int n){
    //Init
    bool *tab = malloc(sizeof(bool) * n-1);
    for(int i = 0; i < n-1; i++){
        tab[i] = true;
    }
    int nbThreads = ((int) sqrt(n)) - 1;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    //Data init
    dataThread *datas = malloc(sizeof(dataThread) * n-2);
    for(int i = 0; i < n-1; i++){
        datas[i].i = i + 2;
        datas[i].n = n;
        datas[i].tab = tab;
        datas[i].mutex = &mutex;
    }

    //Threads
    pthread_t *threadTab = malloc(sizeof(pthread_t) * nbThreads);
    for(int i = 0; i < nbThreads; i++){
        int ret = pthread_create(&(threadTab[i]), NULL, codeThread, datas + i);
        assert(ret == 0);
    }

    for(int i = 0; i < nbThreads; i++){
        int ret = pthread_join(threadTab[i], NULL);
        assert(ret == 0);
    }


    //print
    for(int i = 0; i < n-1; i++){
        printf("%d : %s   ", i+2, tab[i] ? "true" : "false");
        if(i%10 == 9){
            printf("\n");
        }
    }

    //Del data
    pthread_mutex_destroy(&mutex);
    free(datas);
    free(tab);
}
/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    dataC data;
    data.n = 0;
    data.order = parseArgs(argc, argv, &(data.n));  // order peut valoir 5 valeurs (cf. master_client.h) : - ORDER_COMPUTE_PRIME_LOCAL - ORDER_STOP - ORDER_COMPUTE_PRIME - ORDER_HOW_MANY_PRIME - ORDER_HIGHEST_PRIME
    printf("Order number : %d\n", data.order); // pour éviter le warning

    if(data.order == ORDER_COMPUTE_PRIME_LOCAL){ // si c'est ORDER_COMPUTE_PRIME_LOCAL alors c'est un code complètement à part multi-thread
        sieveOfEratosthenes(data.n);
    }
    else{   //sinon
        initClient(&data);
        
        sendOrderAndData(data);

        receiveAnswer(data);

        closeClient(data);
    }
    
    // N'hésitez pas à faire des fonctions annexes ; si la fonction main ne dépassait pas une trentaine de lignes, ce serait bien.
    printf("Client stopped\n");

    return EXIT_SUCCESS;
}
