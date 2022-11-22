#ifdef HAVE_CONFIG
#include "config.h"
#endif

/*****************************************************************************
 * auteur : Gilles Subrenat
 *
 * fichier : myassert.c
 *
 * note :
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "myassert.h"

void myexecv(char *name, char **arg){
    int ret = execv(name, arg);
    myassert(ret != -1, "Prob exec");
}

void mypipe(int t[]){
    int ret = pipe(t);
    myassert(ret == 0, "Creating pipe");
}

void myunlink(const char *p){
    int ret = unlink(p);
    myassert(ret == 0, "Wrong unlink");

}

void mysemctlwithval(const int semid, const int semno, const int cmd, const int val){ //Pour init
    int ret = semctl(semid, semno, cmd, val);
    myassert(ret != -1, "semctl");
}

void mysemctlnoval(const int semid, const int semno, const int cmd){ //Pour destroy
    int ret = semctl(semid, semno, cmd);
    myassert(ret != -1, "semctl");
}

int mysemget(const int key, const int nb, const int order){
    int semid = semget(key, nb, order);
    myassert(semid != -1, "Creating semaphore");
    return semid;
}

void mymkfifo(const char *name, const int n){
    int retmkf = mkfifo(name, n);
    myassert(retmkf == 0, "Creating pipe");
}

int myopen(const char * name, const int order){
    int namedPipe = open(name, order);
    myassert(namedPipe != -1, "Opening tube");
    return namedPipe;
}

void myclose(const int p){
    int ret = close(p);
    myassert(ret == 0, "Closing tube");
}

void mywrite(const int fd, const void *buf, const size_t size){
    int ret = write(fd, buf, size);
    myassert((size_t)ret == size, "Wrong size");
}

void myread(const int fd, void *buf, const size_t size){
    int ret = read(fd, buf, size);
    myassert((size_t)ret == size, "Wrong size");
}

void myassert_func(bool condition, const char *message, const char *fileName,
                   const char *functionName, int line)
{
    if (! condition)
    {
        fprintf(stderr, "/---------------------------\n");
        fprintf(stderr, "| Erreur détectée !\n");
        fprintf(stderr, "|       fichier  : %s\n", fileName);
        fprintf(stderr, "|       ligne    : %d\n", line);
        fprintf(stderr, "|       fonction : %s\n", functionName);
        fprintf(stderr, "|       pid      : %d\n", getpid());
        fprintf(stderr, "|    Message :\n");
        fprintf(stderr, "|       -> %s\n", message);
        fprintf(stderr, "|    Message systeme:\n");
        fprintf(stderr, "|       -> ");
        perror("");
        fprintf(stderr, "|    On stoppe le programme\n");
        fprintf(stderr, "\\---------------------------\n");
        exit(EXIT_FAILURE);
    }
}
