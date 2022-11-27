#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE
//Devigne Mathis

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)
#define MON_FICHIER "master_client.h"
#define SEM_TUBE_ID 1
#define SEM_STOP_ID 2
#define CLE_SEM_TUBE ftok(MON_FICHIER,SEM_TUBE_ID)
#define CLE_SEM_STOP ftok(MON_FICHIER,SEM_STOP_ID)

#define PIPE_CTM "pipe_client_to_master"
#define PIPE_MTC "pipe_master_to_client"

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation
void mysemop(int semid, int sem_op); //id du semaphore et de l'operation a lui appliquer (marche seulement avec les semaphore simple)

#endif
