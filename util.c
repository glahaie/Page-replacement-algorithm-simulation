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

//Contient vérifie si la page est contenue dans les cadres. Si elle est contenue,
//on retourne la position dans le tableau, sinon on retourne -1.
int contient(struct memoire_physique*, int);

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
		return ref_proc;
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

//*****************************************************************************
//Debut de l'implémentation des fonctions simulants les algorithmes de 
//développement

struct memoire_physique * algo_horloge(struct ref_processus *pages, int cadres) {

    //On initialise le tableau de cadres

    assert(cadres > 0 && "Valeur de cadres invalide");
    struct memoire_physique* mem = (struct memoire_physique*)malloc(sizeof(struct memoire_physique));
    if(!mem)
        printf("Probleme\n");
        //
    mem->algo = HORLOGE;

    mem->nbre_cadres = cadres;
    mem->nbre_defauts_pages = 0;
    mem->cadres = (struct cadre*)malloc(cadres*sizeof(struct cadre));
    if(!mem->cadres)
        //Erreur de malloc
        printf("stuff\n");

    int courant = 0;
    int i;
    int retour;

    //On initialise les cadres à -1, pour savoir s'ils sont vides ou non au
    //début;
    for(i = 0; i < cadres; ++i) {
        mem->cadres[i].page = -1;
        mem->cadres[i].R = '0';
    }

    //Maintenant on traite les demandes de pages
    for(i = 0; i < pages->nbre_ref; ++i) {
        //cas facile: est-ce que le cadre est vide
        //
        //On doit vérifier tout d'abord si le cadre contient la page

        if((retour = contient(mem, pages->references[i])) >= 0 ) {
            printf("page deja presente\n");

            mem->cadres[retour].R = '1';
            continue;
        }            

        //On regarde maintenant si on a un cadre vide ou encore on cherche
        //le cadre a remplacer
        printf("avant while: courant = %d, page = %d\n", courant, mem->cadres[courant].page);
        while(mem->cadres[courant].page >= 0 && mem->cadres[courant].R != '0') {
            printf("courant = %d, R = %c: on change a zero\n", courant, mem->cadres[courant].R);
           mem->cadres[courant].R = '0';
           courant = (courant+1)%mem->nbre_cadres;
           printf("apres, courant = %d\n", courant);
        }

        //Maintenant on a la position ou faire le defaut de page: on change la valeur
        mem->nbre_defauts_pages++;
        mem->cadres[courant].page = pages->references[i];
        mem->cadres[courant].R = '1';
        courant = (courant +1)%mem->nbre_cadres;

    }

    return mem;
}


int contient (struct memoire_physique * mem, int valeur) {
    //Comme le tableau n'est pas classé, on parcours tous les éléments
    int i;
    for(i = 0; i < mem->nbre_cadres; ++i) {
        if(mem->cadres[i].page == valeur)
            return i;
    }
    return -1;
}


struct memoire_physique * algo_optimal(struct ref_processus* pages, int cadres) {

    //On répete bcp d'initilisation au début, p-e qu'on peut éviter.
    //
    printf("appel de algo_optimal\n");
    assert(cadres > 0 && "Valeur de cadres invalide");
    struct memoire_physique* mem = (struct memoire_physique*)malloc(sizeof(struct memoire_physique));
    if(!mem)
        printf("Probleme\n");
        //
    mem->algo = OPTIMAL;

    mem->nbre_cadres = cadres;
    mem->nbre_defauts_pages = 0;
    mem->cadres = (struct cadre*)malloc(cadres*sizeof(struct cadre));
    if(!mem->cadres)
        //Erreur de malloc
        printf("stuff\n");

    int courant = 0, max_proche = -1, pos, j, k, temp, trouve;
    int i;
    int retour;

    //On initialise les cadres à -1, pour savoir s'ils sont vides ou non au
    //début;
    for(i = 0; i < cadres; ++i) {
        mem->cadres[i].page = -1;
        mem->cadres[i].R = '0';
    }
    courant = 0;
    //Maintenant on traite
    printf("debut boucle\n");
    for(i = 0; i < pages->nbre_ref; ++i) {
        printf("i = %d\n", i);

        //Debut: on regarde si la page est déjà dans le cadre
        if((retour = contient(mem, pages->references[i])) >= 0) {
            printf("Page deja en memoire, on ne fait rien.\n");
            continue;
        }
        printf("page pas en memoire\n");

        //On regarde maintenant si on a un cadre de libre
        //
        //On pourrait avoir un bool ici pour éviter de refaire la
        //boucle une fois que tous les cadres sont occupés.
        while(mem->cadres[courant].page >=  0 && courant < pages->nbre_ref) {
            courant++;
        }
        if(courant < pages->nbre_ref) {
            printf("Cadre libre #%d - on l'occupe\n", courant);
            mem->cadres[courant].page = pages->references[i];
            mem->nbre_defauts_pages++;
            continue;
        }

        max_proche = -1;
        trouve = 0;
        //Si on est rendu ici, il faut trouver le max dans les cadres
        for(j = 0; j < mem->nbre_cadres; ++j) {
            //On trouve la prochaine occurence de cette page
            temp = 0;
            for(k = i + 1; k < pages->nbre_ref; k++) {
                temp++;
                if(mem->cadres[j].page == pages->references[k]) {
                    trouve = 1;
                    break;
                }
            }

            //On regarde ici si on a plus grand que le max
            if(!trouve) {
                //Le cadre ne revient plus: pas besoin de chercher un autre
                pos = j;
                break;
            }
            if(max_proche < temp) {
                max_proche = temp;
                pos = j;
            }
        }

        //On a maintenant l'endroit pour enlever: on met la page dans le cadre
        mem->cadres[pos].page = pages->references[i];
        mem->nbre_defauts_pages++;
    } //fin for

    return mem;
}



