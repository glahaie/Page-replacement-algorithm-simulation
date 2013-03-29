/* algo.c
 * Par Guillaume Lahaie
 * LAHG04077707
 * Derniere modification: 29 mars 2013
 *
 * Implémentation des fonction simulant les différents algorithmes
 * de remplacement de pages. Pour les trois algorithmes, je
 * considère que la mémoire est vide en commencant.
 */

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define MAX_R 256           //Valeur maximale de R + 1

//contient: vérifie si la page est contenue dans un des cadres. Si elle est 
//contenue, on retourne la position dans le tableau, sinon on retourne -1.
int contient(struct memoire_physique*, int);

//initiliaser_memoire: alloue de facon dynamique une struct memoire_physique
//en ensuite initialise les valeurs dans cette structure.
struct memoire_physique* initialiser_memoire(int cadres, int algo);


//trouver_proch_occ: trouve la prochaine occurence d'une page dans les
//prochaines demandes. la valeur retournée représente la distance relative
//par rapport à la page traitée présentement. Si la page n'est plus demandée,
//on retourne -1.
int trouver_proch_occ(struct ref_processus*, int, int);


//algo_horloge: simule le remplacement des pages selon l'algorithme de 
//l'horloge.
struct memoire_physique* algo_horloge(struct ref_processus *pages, int cadres) {

    assert(cadres > 0 && "Valeur de cadres invalide");

    int courant = 0,        //position du prochain cadre vide, seulement pour
                            //les premiers cadres.
        i,
        retour;             //position dans les cadres d'une page, si elle est
                            //présente

    struct memoire_physique* mem = initialiser_memoire(cadres, HORLOGE);

    //On traite les demandes de pages
    for(i = 0; i < pages->nbre_ref; ++i) {

        //On vérifie si la page est déjà dans un cadre
        if((retour = contient(mem, pages->references[i])) >= 0 ) {
            mem->cadres[retour].R = '1';
        } else {
            //défaut de page
            
            //On trouve la position de la victime. Soit un cadre vide, 
            //soit le premier cadre ou R == '0'
            while(mem->cadres[courant].page >= 0 && 
                    mem->cadres[courant].R != '0') 
            {
                mem->cadres[courant].R = '0';
                courant = (courant+1)%mem->nbre_cadres;
            }

            //Maintenant on a la position ou changer la valeur
            mem->nbre_defauts_pages++;
            mem->cadres[courant].page = pages->references[i];
            mem->cadres[courant].R = '1';
            courant = (courant +1)%mem->nbre_cadres;
        }

    }

    return mem;
}

//algo_optimal: simule le remplacement des pages selon l'algorithme optimal.
//Pour ce faire, lors d'un remplacement de pages on regarde dans la suite
//des pages demandées pour trouver le meilleur remplacement, celui qui, soit
//ne revient, soit qui revient le plus tard possible.
struct memoire_physique * algo_optimal(struct ref_processus* pages, int cadres) {

    assert(cadres > 0 && "Valeur de cadres invalide");
    struct memoire_physique* mem = initialiser_memoire(cadres, OPTIMAL);

    int courant = 0,            //indique la position du prochain cadre libre,
                                //s'il y en a un
        max_proche = -1,        //distance et position maximale de la prochaine
        pos_max,                //demande de page
        prochOcc, i, j;

    //Maintenant on traite
    for(i = 0; i < pages->nbre_ref; ++i) {

        //Debut: on regarde si la page n'est pas présente. Si elle est
        //présente, rien à faire.
        if(contient(mem, pages->references[i]) < 0) {

            mem->nbre_defauts_pages++;
            
            //A-t-on un cadre libre?
            if(courant < mem->nbre_cadres) {
                mem->cadres[courant].page = pages->references[i];
                courant++;
            } else {
                //On trouve le remplacement optimal

                max_proche = -1;
                for(j = 0; j < mem->nbre_cadres; ++j) {
                    //On trouve la prochaine occurence de cette page
                    prochOcc = trouver_proch_occ(pages, i, mem->cadres[j].page);
                    
                    if(prochOcc < 0) {
                        pos_max= j;
                        break;
                    }
                    if(max_proche < prochOcc) {
                        max_proche = prochOcc;
                        pos_max = j;
                    }      
                }

                //On a maintenant l'endroit pour ecraser, on fait le changement
                mem->cadres[pos_max].page = pages->references[i];
            }
        }
    } //fin for

    return mem;
}


//Pour le moment j'utilise la division et l'addition pour mettre à jour
//les R, toutefois, il faudrait changer pour des operations bits à bits
//pour etre un peu plus efficace et etre plus proche de ce qui se passe
//vraiment
struct memoire_physique * algo_vieillissement(struct ref_processus* pages, 
        int cadres, int cycle) 
{

    assert(cadres > 0 && "Valeur de cadres invalide");

    struct memoire_physique* mem = initialiser_memoire(cadres, VIEILLISSEMENT);

    int courant = 0,        //position du prochain cadre libre, s'il y en a un
        min,                //min: plus petite valeur de R
        pos,                //position de min
        i, j,
        retour;             //
    int pos_cycle= 0;
    unsigned char temp;     //permet de facilement mettre a jour la valeur
                            //de vieillissement d'un cadre

    char *maj = (char*)malloc(cadres*sizeof(char));
    if(!maj) {
        fprintf(stderr, "Erreur lors d'une allocation de memoire dynamique.\n");
        exit(1);
    }
    for(i = 0; i < cadres; ++i) {
        maj[i] = 0;
    }

    for(i = 0; i < pages->nbre_ref; ++i) {

        //debut: on regarde si la page est dans un des cadres
        if((retour = contient(mem, pages->references[i])) >= 0) {
            maj[retour] = 1;
        }
        ++pos_cycle;

        //si on a defaut de page ou on a un clock tick, on met à jour
        //les R
        if(retour < 0 || pos_cycle >= cycle) {
            for(j = 0; j < cadres; j++) {
                temp = mem->cadres[j].R;
                temp = temp >> 1; //shift a droite d'une position
                if(maj[j] != 0) {
                    temp += 128; //0x10000000
                }
                mem->cadres[j].R = temp;
                maj[j] = 0;
            }
            pos_cycle = 0;
        }

        //Maintenant, on regarde s'il y a defaut de page.
        if(retour < 0) {
            //Debut: on regarde si un cadre ne contient pas de pages
            mem->nbre_defauts_pages++;
            if (courant < cadres) {
                    mem->cadres[courant].page = pages->references[i];
                    mem->cadres[courant].R = 128;
                    courant++;
            } else {
                min = MAX_R;  //Nom de var à changer
                //Si on a pas de cadres libres, on choisit la victime
                for(j = 0; j < cadres; j++) {
                    temp = mem->cadres[j].R;
     
                    if (temp < min) {
                        min = temp;
                        pos = j;
                        if(min <= 0) 
                            break;
                    }
                }
                //Maintenant on remplace
                mem->cadres[pos].page = pages->references[i];
                mem->cadres[pos].R = 0x80;
            } //fin else
        } //fin if
    } //fin for exterieur
    free(maj);
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

struct memoire_physique* initialiser_memoire(int cadres, int algo) {

    int i;
    struct memoire_physique* mem = 
        (struct memoire_physique*)malloc(sizeof(struct memoire_physique));
    if(!mem) {
        fprintf(stderr, "Erreur d'allocation de memoire dynamique.\n");
        exit(1);
    }
    mem->algo = algo;
    mem->nbre_cadres = cadres;
    mem->nbre_defauts_pages = 0;
    mem->cadres = (struct cadre*)malloc(cadres*sizeof(struct cadre));
    if(!mem->cadres) {
        fprintf(stderr, "Erreur d'allocation de memoire dynamique.\n");
        exit(1);
    }

    for(i = 0; i < cadres; ++i) {
        mem->cadres[i].page = -1;
        mem->cadres[i].R = 0;
    }
    return mem;
}


int trouver_proch_occ(struct ref_processus* pages, int pos_debut, int page) {
    int i = pos_debut, total = 0, trouve = 0;

    while(i < pages->nbre_ref) {
        total++;
        if(pages->references[i] == page) {
            trouve = 1;
            break;
        }
        i++;
    }
    return trouve?total:-1;
}
    
