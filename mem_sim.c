/* mem_sim.c
 * Par Guillaume Lahaie
 * LAHG04077707
 * Dernière modification: 7 avril 2013
 *
 * Ce programme simule différents algorithmes de remplacement de pages. Les
 * trois algorithmes simulés sont l'algorithme de remplacement optimal, 
 * l'algorithme de remplacement horloge et l'algorithme de remplacement de
 * vieillissement. 
 *
 * Pour lancer le programme, il faut spécifier le nombre de cadres et
 * le cycle pour l'algorithme de vieillissement. Il faut aussi fournir
 * un nom de fichier qui contient l'appel des pages.
 *
 * usage: ./executable <cadres> <cycle> <nom_fichier>
 *
 * Pour la simulation des algorithmes, le programme crée trois threads qui
 * lancent la simulation d'un algorithme chaque. Afin de traiter la fin de
 * chaque thread dans l'ordre qu'ils se terminent, j'utilise la solution du
 * proposé au lab 8. Toutefois, après certains tests avec valgrind, il semble
 * que cela peut causer certaines difficultés. Valgrind dénote une fuite de 
 * mémoire malgré qu'une vérification du code permet de voir que chaque
 * allocation de mémoire dynamique est libérée avant la fin du programme.
 *
 * Un cadre vide est représenté par le nombre -1. Les valeurs de R pour 
 * l'algorithme de l'horloge sont représentés par les entiers 0 et 1.
 * 
 * Le programme vérifie d'abord que les arguments fournies lors du
 * lancement du programme sont valides. Il lit ensuite le fichier
 * passé en argument et insère les données du fichier dans la
 * struct ref processus. Les trois simulations sont ensuites
 * appelées, chacun à partir d'un thread différent, et le programme
 * attend ensuite la fin de l'éxécution des threads. Il affiche ensuite
 * le résultat des différentes simulations.
 *
 * À noter, dans le fichier util.c fourni pour effectuer le TP, le
 * fichier lu dans la fonction lireFichier n'est jamais fermé avec
 * fclose avant la fin de la fonction, ce qui crée une fuite de mémoire.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include <ctype.h>
#include <pthread.h>

#define NB_THREADS 3
#define EN_COURS -1
#define FINI 1
#define TRAITE 0

int global_etats[NB_THREADS];

typedef struct {
    struct ref_processus *proc;
    int cadres;
    int cycle;
    int algo;
    int i;
} Arguments;


//appel_algo: fonction appelée lors de la creation du thread. Selon les
//arguments fournis au thread, elle appelle la fonction de simulation
//de remplacement de page de l'algorithme demande.
void* appel_algo(void *);
    

int main(int argc, char *argv[]) {

   if(argc != 4) {
       fprintf(stderr, "Utilisation: %s <cadres> <cycle> \
               <fichier_sequence_reference\n", argv[0]);
       exit(1);
   }

   pthread_t tid[NB_THREADS];    //tid des threads
   void* retour[NB_THREADS];     //pointeur retourné par les threads
   Arguments arg[NB_THREADS];    //Arguments pour pthread_create. J'ai besoin d'un
                                 //tableau pour que les threads executent les 
                                 //algorithmes differents.
   int cadres, cycle, i,j;
   char *endPtr;
   struct ref_processus proc;  
   struct memoire_physique * mem;

   //Verification des arguments
   cadres = strtol(argv[1], &endPtr, 10);
   if(cadres <= 0 || isspace(*endPtr)) {
       fprintf(stderr, "Erreur: nombre de cadres invalide.\n");
       exit(1);
   }
   cycle = strtol(argv[2], &endPtr, 10);
   if(cycle <= 0 || isspace(*endPtr)) {
       fprintf(stderr, "Erreur: nombre de cycle invalide.\n");
       exit(1);
   }

    //Lecture du fichier
    proc = lireFichier(argv[3]);
 
    //Creation des threads
    for(i = 0; i < NB_THREADS; ++i) {
        arg[i].proc = &proc;
        arg[i].cycle = cycle;
        arg[i].cadres = cadres;
        arg[i].i = i;
        switch(i) {
            case 0: arg[i].algo = OPTIMAL;
                    break;
            case 1: arg[i].algo = HORLOGE;
                    break;
            case 2: arg[i].algo = VIEILLISSEMENT;
                    break;
            default:
                    fprintf(stderr, "Erreur dans le nombre d'algo.\n");
                    exit(1);
        }

        //maintenant, on devrait etre pret à creer les threads
        if(pthread_create(&tid[i], NULL, appel_algo, (void*) &arg[i])) {
            fprintf(stderr, "Erreur lors de la création du thread, i = %d\n", i);
            exit(1);
        }

    }
    
    
    //On fait les joins lorsque les threads sont terminés.
    i = 0;
    j= 0;
    while(j < NB_THREADS) {
        if(global_etats[i] == FINI) {
            if(!pthread_join(tid[i], &retour[i])) {
                global_etats[i] = TRAITE;
            } else {
                fprintf(stderr, "Erreur lors d'un join.\n");
            }
            ++j;

        }
        i = (i + 1)%NB_THREADS;
    }


    //Maintenant, on affiche les resultats
    for(i = 0; i < NB_THREADS; i++) {
        mem = (struct memoire_physique *)retour[i];
        print_memoire_physique(*mem);
        free(mem->cadres);
        free(mem);
    }

    free(proc.references);
    return 0;
}


void* appel_algo(void *p) {
    Arguments* arg = (Arguments*)p;
    struct memoire_physique* result;
    global_etats[arg->i] = EN_COURS;

    switch(arg->algo) {
        case OPTIMAL:   result = algo_optimal(arg->proc, arg->cadres);
                        break;
        case HORLOGE:   result = algo_horloge(arg->proc, arg->cadres);
                        break;
        case VIEILLISSEMENT:   
            result = algo_vieillissement(arg->proc, arg->cadres, arg->cycle);
            break;
        default: fprintf(stderr, "Erreur lors de appel_algo: algorithme inconnu.\n");
                 exit(1);
    }
    global_etats[arg->i] = FINI;
    pthread_exit ((void *)result);
}


       
