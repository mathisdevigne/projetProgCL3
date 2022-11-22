#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h> 
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>	
#include <unistd.h>
#include <sys/wait.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/
static int nFirstWorker = 2;

// on peut ici définir une structure stockant tout ce dont le master
// a besoin

typedef struct{
    int semsid;
    int semtid;

    int pReadClient;
    int pWriteClient;

    int pReadWorker;
    int pWriteWorker;

    int maxN;
    int howManyPrime;
    int highestPrime;
} dataM;

/************************************************************************
 * Mes fonctions & utils
 ************************************************************************/

void initMaster(dataM *data){
    // - création des sémaphores
    data->semtid = mysemget(CLE_SEM_TUBE, 1, IPC_CREAT | 0664); //Sem named pipe
    data->semsid = mysemget(CLE_SEM_STOP, 1, IPC_CREAT | 0664); //Sem stop

    // Init sémaphores
    mysemctlwithval(data->semtid, 0, SETVAL, 1);
    mysemctlwithval(data->semsid, 0, SETVAL, 0);

    // - création des tubes nommés
    mymkfifo(PIPE_MTC, 0644);
    mymkfifo(PIPE_CTM, 0644);
}

void createFirstWorker(dataM *data){
    // - création du premier worker     
    dataCreateWorker *dCW = createWorker(nFirstWorker);
    data->pReadWorker = dCW->readp;
    data->pWriteWorker = dCW->writep;
    free(dCW);

    bool temp;
    myread(data->pReadWorker, &temp, sizeof(bool)); //read the return of the worker 2
}

void closeMaster(dataM data){
    // destruction des tubes nommés, des sémaphores, ...
    myunlink(PIPE_CTM);
    myunlink(PIPE_MTC);

    mysemctlnoval(data.semsid, -1, IPC_RMID);
    mysemctlnoval(data.semtid, -1, IPC_RMID);
}

void orderStop(dataM data){
    int order = WORDER_STOP;
    mywrite(data.pWriteWorker, &order, sizeof(int));    //envoyer ordre de fin au premier worker et attendre sa fin
    wait(NULL);
    order = ORDER_STOP;
    mywrite(data.pWriteClient, &order, sizeof(int));    //envoyer un accusé de réception au client
}

void orderCompute(dataM *data){
    int n = 0;
    myread(data->pReadClient, &n, sizeof(int));  //récupérer le nombre N à tester provenant du client

    

    bool isPrime;
    if (n >3581){
        printf("Too high,  %d > 3581, not supported yet\n", n);
        isPrime = false;
    }
    else{
        int order = WORDER_COMPUTE_PRIME;
                
        if(n > data->maxN){
            for(int i = data->maxN+1; i < n; i++){   //il faut connaître le plus nombre (data.maxN) déjà enovoyé aux workers on leur envoie tous les nombres entre eux
                mywrite(data->pWriteWorker, &order, sizeof(int));
                int i2 = i;
                mywrite(data->pWriteWorker, &i2, sizeof(int)); 
                myread(data->pReadWorker, &isPrime, sizeof(bool)); //note : chaque envoie déclenche une réponse des workers
            }
            data->maxN = n;
        }
        
        mywrite(data->pWriteWorker, &order, sizeof(int)); //write order to worker
        mywrite(data->pWriteWorker, &n, sizeof(int)); //write n to worker

        myread(data->pReadWorker, &isPrime, sizeof(bool)); //read if it's prime
                
        if(isPrime){
            if(n > data->highestPrime){
                data->highestPrime = n;
            }
            data->howManyPrime++;
        }
    }

    mywrite(data->pWriteClient, &isPrime, sizeof(bool)); //write to client the answer
}

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(dataM data) // nom de variable : pipe read/write master/client/worker to master/client/worker
{
    data.maxN = nFirstWorker;
    data.howManyPrime = 0;
    data.highestPrime = 0;
    int order;
    bool stop = false;
    
    while(!stop){   // boucle infinie :
        data.pWriteClient = myopen(PIPE_MTC, O_WRONLY); // - ouverture des tubes (cf. rq client.c)
        data.pReadClient = myopen(PIPE_CTM, O_RDONLY);

        myread(data.pReadClient, &order, sizeof(int));  // - attente d'un ordre du client (via le tube nommé)

        switch (order){
        case ORDER_STOP :      // - si ORDER_STOP, envoyer ordre de fin au premier worker et attendre sa fin, envoyer un accusé de réception au client
            orderStop(data);
            stop = true;
            break;
        
        case ORDER_COMPUTE_PRIME :
            orderCompute(&data); 
            break;

        // - si ORDER_HOW_MANY_PRIME, transmettre la réponse au client
        case ORDER_HOW_MANY_PRIME :
            mywrite(data.pWriteClient, &(data.howManyPrime), sizeof(int));
            break;

        // - si ORDER_HIGHEST_PRIME, transmettre la réponse au client
        case ORDER_HIGHEST_PRIME :
            mywrite(data.pWriteClient, &(data.highestPrime), sizeof(int));
            break;

        default:
            break;
        }
        myclose(data.pReadClient);  // - fermer les tubes nommés
        myclose(data.pWriteClient);
        
        mysemop(data.semsid, -1);    // - attendre ordre du client avant de continuer (sémaphore : précédence)

        
    }   // - revenir en début de boucle
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1){
        usage(argv[0], NULL);
    }
    
    dataM data;

    initMaster(&data);

    createFirstWorker(&data);

    // boucle infinie
    loop(data);

    closeMaster(data);

    printf("Master stopped\n");

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
