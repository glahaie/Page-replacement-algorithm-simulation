/* mem_sim.c
 * Par Guillaume Lahaie
 * LAHG04077707
 * Dernière modification: 26 mars 2013
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

int main(int argc, char *argv[]) {

   if(argc != 4) {
       fprintf(stderr, "Utilisation: ./%s <nombres_de_cadres> <cycle> <fichier_sequence_referenrce\n", argv[0]);
       exit(1);
   }

   int cadres, cycle, i;
   char *endPtr;
   FILE *fichier;
   struct ref_processus proc;  
   struct memoire_physique * mem;
   cadres = strtol(argv[1], &endPtr, 10);
   printf("cadres = %d\n", cadres);
   if(cadres <= 0 || isspace(*endPtr)) {
       fprintf(stderr, "Erreur: nombre de cadres invalide.\n");
       exit(1);
   }

   cycle = strtol(argv[2], &endPtr, 10);
   if(cycle <= 0 || isspace(*endPtr)) {
       fprintf(stderr, "Erreur: nombre de cycle invalide.\n");
       exit(1);
   }

    
   //Maintenant on essaie d'ouvrir le fichier

   fichier = fopen(argv[3], "r");
   if(fichier == NULL) {
       fprintf(stderr, "Erreur lors de l'ouverture du fichier.\n");
       exit(1);
   }
   fclose(fichier);

   //Maintenant on est prêt à tester le fichier
    proc = lireFichier(argv[3]);

    
    //verif de proc
    printf("proc =\n");
    for(i = 0; i < proc.nbre_ref; ++i) {
        printf("%d ", proc.references[i]);
    }
    printf("\n");

    mem = algo_optimal(&proc, cadres);

    print_memoire_physique(*mem);

    return 0;
}


       
