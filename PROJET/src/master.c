#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ipc.h> 
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin


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
void loop(const int semid)
{
    // boucle infinie :
    while(true){
        // - ouverture des tubes (cf. rq client.c)
        int pctm = open(PIPE_CTM, O_RDONLY);
        assert(pctm != -1);
        int pmtc = open(PIPE_MTC, O_WRONLY);
        assert(pmtc != -1);

        // - attente d'un ordre du client (via le tube nommé)
        int order;
        int retp = read(pctm, &order, sizeof(int));
        assert(ret == sizeof(int));

        switch (order){
        // - si ORDER_STOP
        //       . envoyer ordre de fin au premier worker et attendre sa fin
        //       . envoyer un accusé de réception au client
        case ORDER_STOP :
            break;

        // - si ORDER_COMPUTE_PRIME
        //       . récupérer le nombre N à tester provenant du client
        //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
        //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
        //             on leur envoie tous les nombres entre M+1 et N-1
        //             note : chaque envoie déclenche une réponse des workers
        //       . envoyer N dans le pipeline
        //       . récupérer la réponse
        //       . la transmettre au client
        case ORDER_COMPUTE_PRIME :
            int n;
            retp = read(pctm, &n, sizeof(int));
            assert(ret == sizeof(int));
            break;
            
        // - si ORDER_HOW_MANY_PRIME
        //       . transmettre la réponse au client
        case ORDER_HOW_MANY_PRIME :
            break;

        // - si ORDER_HIGHEST_PRIME
        //       . transmettre la réponse au client
        case ORDER_HIGHEST_PRIME :
            break;


        default:
            break;
        }
        // - fermer les tubes nommés
        retp = close(pctm);
        assert(retp == 0);
        retp = close(pmtc);
        assert(retp == 0);

        // - attendre ordre du client avant de continuer (sémaphore : précédence)
        struct sembuf semb [1];
        semb[0].sem_num = 0;
        semb[0].sem_op = -1;
        semb[0].sem_flg = 0;
        int retop = semop(semstopid, semb, 1);
        assert(retop != -1);

        // - revenir en début de boucle
    }

    // il est important d'ouvrir et fermer les tubes nommés à chaque itération
    // voyez-vous pourquoi ?
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1){
        usage(argv[0], NULL);
    }

    // - création des sémaphores
    int semtid = semget(CLE_SEM_TUBE, 1, IPC_CREAT | 0664);
    assert(semtid != -1);
    int retctl = semctl(semtid, 0, SETVAL, 1);
    assert(retctl != -1);

    int semsid = semget(CLE_SEM_STOP 1, IPC_CREAT | 0664);
    assert(semsid != -1);
    int retctl = semctl(semsid, 0, SETVAL, 0);
    assert(retctl != -1);

    // - création des tubes nommés
    int retmkf = mkfifo(PIPE_MTC, 0644);
    assert(retmkf == 0);
    retmkf = mkfifo(PIPE_CTM, 0644);
    assert(retmkf == 0);

    // - création du premier worker
    int f = fork();
    if(f == 0){
        int ret = exec("worker");
        myassert_func(ret != -1, "Prob exec", argv[0], "main", 0);
    }

    // boucle infinie
    loop(/* paramètres */);

    // destruction des tubes nommés, des sémaphores, ...
    retmkf = unlink(PIPE_CTM);
    assert(retmkf == 0);
    retmkf = unlink(PIPE_MTC);
    assert(retmkf == 0);

    retctl = semctl(semid, -1, IPC_RMID);
    assert(retctl != -1);

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
