#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "myassert.h"

#include "master_client.h"

// fonctions éventuelles internes au fichier

// fonctions éventuelles proposées dans le .h
void my_semop(int semid, int sem_op){
    struct sembuf semb[1];
    semb[0].sem_num = 0;
    semb[0].sem_op = sem_op;
    semb[0].sem_flg = 0;

    int retop = semop(semid, semb, 1);
    assert(retop != -1);
}
