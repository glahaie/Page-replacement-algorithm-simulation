/* mem_sim.c
 * Par Guillaume Lahaie
 * LAHG04077707
 * Dernière modification: 29 mars 2013
 *
 * Déclaration de threads pour les fonctions de simulation de
 * remplacement de pages
 *
 * Pour le moment, je teste 1 à 1 les fonctions, donc on n'a pas
 * encore d'utilisation de threads.
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
} Arguments;  //vue qu'un seul arg  pour threads


//appel_algo: fonction appelée lors de la creation du thread. Selon les
//arguments fournis au thread, elle appelle la fonction de simulation
//de remplacement de page de l'algorithme demande.
void* appel_algo(void *);
    

int main(int argc, char *argv[]) {

   if(argc != 4) {
       fprintf(stderr, "Utilisation: ./%s <nombres_de_cadres> <cycle> <fichier_sequence_referenrce\n", argv[0]);
       exit(1);
   }

   pthread_t tid[NB_THREADS];    //tid des threads
   void* retour[NB_THREADS];     //pointeur retourné par les threads
   Arguments arg[NB_THREADS];    //Arguments pour pthread_create. J'ai besoin d'un
                                 //tableau pour que les threads executent les 
                                 //algorithmes differents.
   int cadres, cycle, i;
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

   //Maintenant on est prêt à tester le fichier
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
    
    //Pour le moment j'ai les deux facons de faire les joins: premiere, naive,
    //qui les fait dans l'ordre, la second le fait lorsqu'un thread a termine.
    //La seconde cause des problemes pour la verif avec valgrind, donc, je ne
    //l'utilise pas pour le moment.

    for(i = 0; i < NB_THREADS; i++) {
        if(pthread_join(tid[i], &retour[i])) {
            fprintf(stderr, "Erreur sur join #%d\n", i);
        }
    }

/*
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
*/

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


       
