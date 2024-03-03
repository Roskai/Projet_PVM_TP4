/* TP algorithmique parallele
 * maitrise
 * LL
 * 13/03/2018
 * utilise gnuplot au lieu de jgraph
 */

/*
 * upper.c
 *
 * programme principal en sequentiel
 */

#include "stdio.h"
#include "stdlib.h"
#include "point.h"

#define P 5
#define SLAVE "upper_slave"	/* nom du programme esclave */

#define PB 50 // TODO mieux refaire

static point *Q[PB]; /* La pile de points */
static int Q_nb; /* l'index de tete de pile */

/*
 * empile ou depile dans la 
 * pile globale Q un probleme
 */
point *depile()
{
	if (Q_nb > 0)
		return Q[--Q_nb];
	else
		return NULL;
}
void empile(point *pb)
{
	Q[Q_nb++] = pb;
}

/*
 * calcul recursif d'enveloppe
 * convexe par bissection
 */
void upper_hull(point *pts)
{
	point *upper, *pts2;
    /**
     * Un seul père :
     * Génère plein de fils et distribue les tâches (potentiellement par une pile)
     * voir MSSrtCorrige
     * 
     * Le père se charge de la séparation en sous-ensembles
	 * 
	 * upper.c : maitre | upper_slave.c : esclave
	 * 
	 * Procédure maitre :
	 * 	- Divise en sous-ensemble de 4 points ou moins
	 * 	- Envoie chaque sous-ensemble pour calcul à un esclave
	 * 	- Reçoit les résultats des esclaves et les empiles
	 *  	-> Si il y a deux chaines de points dans la pile : on les dépiles et les envoie à un esclave pour merge
	 * 
	 * Procédure esclave :
	 * 	- Reçoit un sous-ensemble et vérifie s'il doit en recevoir un second
	 * 		-> Définition de la nature du calcul : calcul d'UH ou merge
	 * 	- Si nature = merge : reçoit un second sous-ensemble
	 * 	- Fait son calcul et envoie le résultat au maitre
    */

	// upper = point_UH(pts); /* retourne 0 si plus de 4 points */
	// if (!upper) {
	// 	pts2 = point_part(pts);
	// 	upper_hull(pts);
	// 	upper_hull(pts2);
	// 	point_merge_UH(pts, pts2);
	// }

	int i = 0;
	point* prec = NULL;
	while (pts != NULL) 
	{
		if (prec != NULL) prec->next = NULL;
		empile(pts);
		for (size_t i = 0; i < 4; i++)
		{
			if (pts != NULL)
			{
				prec = pts;
				pts = pts->next;
			}
		}
	}

	point *p = depile();
	while (p != NULL)
	{
		printf("%d\n",point_nb(p));
		p = depile();
	}

	//pvm_spawn(EPATH "/"+SLAVE, (char**)0, 0, "", P, tids);
}

/*
 * upper <nb points>
 * exemple :
 * % upper 200 
 * % evince upper_hull.pdf
 */

int main(int argc, char **argv)
{
	point *pts;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <nb points>\n", *argv);
		exit(-1);
	}
	pts = point_random(atoi(argv[1]));
	point_print_gnuplot(pts, 0); /* affiche l'ensemble des points */
	upper_hull(pts);
	point_print_gnuplot(pts, 1); /* affiche l'ensemble des points restant, i.e
					l'enveloppe, en reliant les points */
	point_free(pts); 
}

