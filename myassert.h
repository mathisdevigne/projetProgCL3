/*****************************************************************************
 * auteur : Gilles Subrenat
 *
 * fichier : myassert.h
 *
 * note :
 *     Utiliser uniquement la macro myassert
 *        arg1 : booleen : si false, une erreur est déclenchée et le
 *                         programme s'arrête
 *        arg2 : string  : message à afficher en cas d'erreur
 *     note : définir la macro NDEBUG désactive le myassert
 *
 * exemple d'appel :
 *   void f(int n)
 *   {
 *       myassert(n > 0, "n doit être positif");
 *       ...
 *   }
 *   si NDEBUG est défini
 *       il ne se passe rien et la fonction s'exécute sans erreur ou non
 *       selon la valeur de n
 *   sinon si n est positif
 *       il ne se passe rien et la fonction s'exécute sans erreur
 *   sinon
 *       le programme s'arrête avec un message d'erreur complet
 *****************************************************************************/

#ifndef MYASSERT_H
#define MYASSERT_H
//Devigne Mathis

void myexecv(char *name, char **arg);
void mypipe(int t[]);
void myunlink(const char *p);
void mysemctlwithval(const int semid, const int semno, const int cmd, const int val);
void mysemctlnoval(const int semid, const int semno, const int cmd);
int mysemget(const int key, const int nb, const int order);
void mymkfifo(const char *name, const int n);
int myopen(const char * name, const int order);
void myclose(const int p);
void myread(const int fd, void *buf, const size_t size);
void mywrite(const int fd, const void *buf, const size_t size);

    #ifndef NDEBUG
        #include <stdbool.h>
        void myassert_func(bool condition, const char *message, const char *filename,
                           const char *functionName, int line);
       #define myassert(condition, message) myassert_func((condition), (message), __FILE__, __func__, __LINE__)
    #else
       #define myassert(condition, message)
    #endif

#endif
