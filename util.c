/**
 *  A utiliser pour le TP2
 * 
 * @author Aziz Salah
 * @version 2013-03-24
 * correction ==NULL par !=NULL
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include <assert.h>


struct ref_processus lireFichier(const char * fichier){
	FILE * in;
	//int * p;

	if ((in = fopen(fichier,"r"))!=NULL){
		int t;
		int i=0;
		while(!feof(in)){
			i++;
			fscanf(in,"%d",&t);
		//	printf("|%d|",t[i]);
		}
		i--; // annule derniere lecture
		rewind(in);
		int * p = (int *) malloc(sizeof(int)*i);
		int j;
		for(j=0; j<i; j++){
			fscanf(in,"%d",&p[j]);
		}
		struct ref_processus ref_proc;
		ref_proc.references = p;
		ref_proc.nbre_ref = i;
        fclose(in);
		return ref_proc;
 
        //Ajout pour le moment -- verif avec prof
	}else{	
		//printf("errno : %d\n", errno);
		perror("probleme ouverture");
		exit(1);
	}
}

void print_memoire_physique(struct memoire_physique mp){
	int i,j;
	switch (mp.algo){
		case OPTIMAL :
		printf("#Algo optimal [DdeP : %d]\n",mp.nbre_defauts_pages);
		break;
		case HORLOGE : 
		printf("##Algo horloge [DdeP : %d]\n##",mp.nbre_defauts_pages);
		for (i = 0; i < mp.nbre_cadres ; i++){
			printf("%d[%d] ",mp.cadres[i].page,mp.cadres[i].R%2);
		}
		printf("\n");
		break;
		case VIEILLISSEMENT :
		printf("###Algo vieillissement [DdeP : %d] \n###",mp.nbre_defauts_pages);	
		int k;
		for (i = 0; i < mp.nbre_cadres ; i++){
			printf("%d[",mp.cadres[i].page);
			for(j=0,k=128; j<8; j++){ //128 : 10000000 en binaire
				
				printf("%d", (mp.cadres[i].R & k) >> (7-j));//10000000
				k = k/2; 
			}
			printf("] ");
		}
		printf("\n");
			break;
		default :
		puts("Algo inconnu!!"); 
		break;
	}
	puts("");
}

