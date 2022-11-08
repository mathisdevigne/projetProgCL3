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

#include "myassert.h"

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
