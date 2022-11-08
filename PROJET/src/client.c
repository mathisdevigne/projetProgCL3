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

void my_semop(int semid, int sem_op){
    struct sembuf semb [1];
    sembt[0].sem_num = 0;
    sembt[0].sem_op = sem_op;
    sembt[0].sem_flg = 0;

    int retop = semop(semid, semb, 1);
    assert(retop != -1);
}

struct dataThread{
    int *tab;
    int n;
    sem_t *sem;
}

void * codeThread(void * arg){
    int *data = (struct dataThread *) arg;


    return NULL;
}

void sieveOfEratosthenes(int n){
    //Init
    bool tab = malloc(sizeof(bool) * n-2);
    for(int i = 0; i < n-2; i++){
        tab[i] = true;
    }
    int nbThreads = ((int) sqrt(n)) - 1;
    sem_t sem;
    ret = sem_init(&sem, 0, 0);

    //Data init
    for(int i = 0; i < nbThreads; i++){
        struct dataThread datas;
        datas.n = i + 2;
        datas.tab = tab;
        datas.sem = &sem;
    }

    //Threads
    pthread_t threadTab = malloc(sizeof(pthread_t) * nbThreads);
    for(int i = 0; i < nbThreads; i++){
        int ret = pthread_create(&(threadTab[i]), NULL, codeThread, &curN);
        assert(ret == 0);
    }
}
/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);
    printf("%d\n", order); // pour éviter le warning

    // order peut valoir 5 valeurs (cf. master_client.h) :
    //      - ORDER_COMPUTE_PRIME_LOCAL
    //      - ORDER_STOP
    //      - ORDER_COMPUTE_PRIME
    //      - ORDER_HOW_MANY_PRIME
    //      - ORDER_HIGHEST_PRIME
    //
    // si c'est ORDER_COMPUTE_PRIME_LOCAL
    //    alors c'est un code complètement à part multi-thread
    if(order == ORDER_COMPUTE_PRIME_LOCAL){
        
    }
    // sinon
    else{
        //    - entrer en section critique :
        //           . pour empêcher que 2 clients communiquent simultanément
        //           . le mutex est déjà créé par le master
        int semtid = semget(CLE_SEM_TUBE, 1, 0);
        assert(semtid != -1);

        my_semop(semtid, -1);

        int semsid = semget(CLE_SEM_STOP, 1, 0);
        assert(semsid != -1);


        //    - ouvrir les tubes nommés (ils sont déjà créés par le master)
        //           . les ouvertures sont bloquantes, il faut s'assurer que
        //             le master ouvre les tubes dans le même ordre
        int pctm = open(PIPE_CTM, O_WRONLY);
        assert(pctm != -1);
        int pmtc = open(PIPE_MTC, O_RDONLY);
        assert(pmtc != -1);

        //    - envoyer l'ordre et les données éventuelles au master
        int retw = write(pctm, &order, sizeof(int));
        assert(retw == sizeof(int));
        if(order == ORDER_COMPUTE_PRIME){
            int secArg = atoi(argv[2]);
            retw = write(pctm, &secArg, sizeof(int));
            assert(retw == sizeof(int));
        }

        //    - attendre la réponse sur le second tube
        int mRep;
        int retr = read(pmtc, &mRep, sizeof(int));
        assert(retr == sizeof(int));
        printf("Reponse : %d\n", mRep);

        //    - sortir de la section critique
        my_semop(semtid, 1);

        //    - libérer les ressources (fermeture des tubes, ...)
        retp = close(pctm);
        assert(retp == 0);
        retp = close(pmtc);
        assert(retp == 0);

        //    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
        my_semop(semsid, 1);
        
        // Une fois que le master a envoyé la réponse au client, il se bloque sur un sémaphore ; le dernier point permet donc au master de continuer
    }
    
    // N'hésitez pas à faire des fonctions annexes ; si la fonction main
    // ne dépassait pas une trentaine de lignes, ce serait bien.
    
    return EXIT_SUCCESS;
}
