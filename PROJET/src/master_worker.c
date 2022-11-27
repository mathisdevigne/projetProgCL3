#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


#include "master_worker.h"
#include "myassert.h"
//Devigne Mathis

// fonctions éventuelles internes au fichier
char **prepArgWorker(int readp, int writep, int n){

    int readpSize = snprintf(NULL, 0, "%d", readp); //Init size
    int writepSize = snprintf(NULL, 0, "%d", writep);
    int nSize = snprintf(NULL, 0, "%d", n);

    char * argTemp0 = malloc(readpSize*sizeof(char));
    char * argTemp1 = malloc(writepSize*sizeof(char));
    char * nTemp = malloc(nSize*sizeof(char));
    sprintf(argTemp0, "%d", readp);
    sprintf(argTemp1, "%d", writep);
    sprintf(nTemp, "%d", n);

    char **ret = malloc(5* sizeof(char *));
    ret[0] = "worker.c";
    ret[1] = nTemp;
    ret[2] = argTemp0;
    ret[3] = argTemp1;
    ret[4] = NULL;
    return ret;
}

// fonctions éventuelles proposées dans le .h

dataCreateWorker *createWorker(int n){
    // - Création tubes anonymes
    int pwtm[2], pmtw[2]; //Tube anonyme worker-master (ici master = celui qui créer le worker)
    mypipe(pwtm);
    mypipe(pmtw);

    int f = fork();
    if(f == 0){
        myclose(pwtm[0]);
        myclose(pmtw[1]);

        char **arg = prepArgWorker(pmtw[0], pwtm[1], n);

        myexecv("worker", arg);
        return NULL;
    }
    else{
        close(pwtm[1]);
        close(pmtw[0]);
        dataCreateWorker *dCW = malloc(sizeof(dataCreateWorker));
        dCW->readp = pwtm[0];
        dCW->writep = pmtw[1];
        return dCW;
    }
}
