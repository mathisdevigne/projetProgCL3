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

#include "master_worker.h"
//Devigne Mathis
/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...

typedef struct{
    int p;
    int pWriteMaster;
    int pReadMaster;
} dataW;

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char *argv[], dataW *data)
{
    if (argc != 4)
    {
        printf("%d\n", argc);
        usage(argv[0], "Nombre d'arguments incorrect");
    }
    // remplir la structure
    data->p = atoi(argv[1]);
    data->pReadMaster = atoi(argv[2]);
    data->pWriteMaster = atoi(argv[3]);
}

/************************************************************************
 * Mes fonctions & utils
 ************************************************************************/

void closeWorker(dataW data){
    myclose(data.pReadMaster);
    myclose(data.pWriteMaster);
}

void orderStop(const dataCreateWorker *dCW)
{
    int order = WORDER_STOP;
    if (dCW != NULL)
    { // Si il y a un worker suivant on lui envoie l'ordre et on attend qu'il s'arrete
        mywrite(dCW->writep, &order, sizeof(int));
        wait(NULL);
    }
}

void orderCompute(dataW data, dataCreateWorker **dCWp)
{
    int n, order = WORDER_COMPUTE_PRIME;
    bool bRet;

    myread(data.pReadMaster, &n, sizeof(int));
    if (data.p == n)
    { // Si n est égale ou est divisible alors on renvoie vrai ou faux
        bRet = true;
    }
    else if (n % data.p == 0)
    {
        bRet = false;
    }
    else
    { // Sinon on envoie au worker suivant
        if (*dCWp == NULL)
        {
            *dCWp = createWorker(n);
            // fprintf(stderr, "%d creer le worker %d\n", data.p, n);
        }
        else
        {
            mywrite((*dCWp)->writep, &order, sizeof(int));
            mywrite((*dCWp)->writep, &n, sizeof(int));
        }
        myread((*dCWp)->readp, &bRet, sizeof(bool)); // Retour du worker suivant
    }

    mywrite(data.pWriteMaster, &bRet, sizeof(bool)); // On envoie au master la réponse
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(dataW data)
{
    int order;
    bool stop = false;
    dataCreateWorker *dCW = NULL;
    // fprintf(stderr,"worker %d créé\n", data.p);

    // boucle infinie :
    while (!stop)
    {
        myread(data.pReadMaster, &order, sizeof(int)); // attendre l'arrivée d'un nombre à tester

        switch (order)
        {
        case WORDER_STOP: //    si ordre d'arrêt, si il y a un worker suivant, transmettre l'ordre et attendre sa fin, sortir de la boucle
            orderStop(dCW);
            stop = true;
            break;

        case WORDER_COMPUTE_PRIME: //    sinon c'est un nombre à tester
            orderCompute(data, &dCW);
            break;

        default:
            break;
        }
    }
    free(dCW);
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char *argv[])
{
    dataW data;
    parseArgs(argc, argv, &data);

    // Si on est créé c'est qu'on est un nombre premier, envoyer au master un message positif pour dire que le nombre testé est bien premier
    bool b = true;
    mywrite(data.pWriteMaster, &b, sizeof(bool));

    loop(data);

    closeWorker(data);// libérer les ressources : fermeture des files descriptors par exemple

    fprintf(stderr, "worker %d stoped\n", data.p);

    return EXIT_SUCCESS;
}
