/**
 * A utiliser pour le TP2
 * 
 * @author Aziz Salah
 * @version 2013-03-20
 */
//Ne pas modifier ce fichier

#ifndef UTIL 
#define UTIL
#define OPTIMAL 0
#define HORLOGE 1
#define VIEILLISSEMENT 2

struct ref_processus {
	int * references;
	int nbre_ref;
};



struct cadre {
	int  page;
	char R;
};

struct memoire_physique {
	int algo;
	struct cadre * cadres;
	int nbre_cadres;
	int nbre_defauts_pages;
};

//Fournies
struct ref_processus lireFichier( const char * );
void print_memoire_physique( struct memoire_physique );


// A faire
struct memoire_physique * algo_optimal( struct ref_processus  *, int );
struct memoire_physique * algo_horloge( struct ref_processus *, int );
struct memoire_physique * algo_vieillissement( struct ref_processus *, int, int );

#endif
