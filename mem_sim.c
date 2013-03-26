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

int main(int argc, char *argv[]) {

   if(argc != 4) {
       fprintf(stderr, "Utilisation: ./%s <nombres_de_cadres> <cycle> <fichier_sequence_referenrce\n", argv[0]);
       exit(1);
   }

   int cadres, cycle;
   char *endPtr;
   FILE *fichier;
   struct ref_processus proc;  
   struct memoire_physique * mem;
   cadres = strtol(argv[1], &endPtr, 10);
   if(cadres <= 0 || endPtr != NULL) {
       fprintf(stderr, "Erreur: nombre de cadres invalide.\n");
       exit(1);
   }

   cycle = strtol(argv[2], &endPtr, 10);
   if(cycle <= 0 || endPtr != NULL) {
       fprintf(stderr, "Erreur: nombre de cycle invalide.\n");
       exit(1);
   }

    
   //Maintenant on essaie d'ouvrir le fichier

   fichier = fopen(argv[3]);
   if(fichier == NULL) {
       fprintf("Erreur lors de l'ouverture du fichier.\n");
       exit(1);
   }

   //Maintenant on est prêt à tester le fichier
    proc = lireFichier(fichier);

    fclose(fichier);

    
    mem = algo_horloge(&proc, cadres);

    print_memoire_physique(*mem);

    return 0;
}


       
