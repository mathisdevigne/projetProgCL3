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

    #ifndef NDEBUG
        #include <stdbool.h>
        void myassert_func(bool condition, const char *message, const char *filename,
                           const char *functionName, int line);
       #define myassert(condition, message) myassert_func((condition), (message), __FILE__, __func__, __LINE__)
    #else
       #define myassert(condition, message)
    #endif

#endif
