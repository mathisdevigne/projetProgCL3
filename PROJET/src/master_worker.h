#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)
#define WORDER_STOP               -1
#define WORDER_COMPUTE_PRIME       1

typedef struct{
    int readp;
    int writep;
} dataCreateWorker;


dataCreateWorker *createWorker(int n);

#endif
