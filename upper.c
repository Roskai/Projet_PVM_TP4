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
#include "pvm3.h"

#define NB_SLAVE 5
#define SLAVE "upper_slave"	/* nom du programme esclave */

static point **to_merge; /* La pile des points à fusionner */
static int to_merge_nb; /* l'index de tete de pile */
point *depile_to_merge()
{
	if (to_merge_nb > 0)
		return to_merge[--to_merge_nb];
	else
		return NULL;
}
void empile_to_merge(point *pt)
{
	to_merge[to_merge_nb++] = pt;
}

static point **to_UH; /* La pile de points à calculer */
static int to_UH_nb; /* l'index de tete de pile */
point *depile_to_UH()
{
	if (to_UH_nb > 0)
		return to_UH[--to_UH_nb];
	else
		return NULL;
}
void empile_to_UH(point *pt)
{
	to_UH[to_UH_nb++] = pt;
}

void master_send_point(int tid, point *pts, point *pts2)
{
    int tabSize = point_nb(pts);
	int *tabX = malloc(tabSize * sizeof(int));
	int *tabY = malloc(tabSize * sizeof(int));
    
    int i;
    point *cur;
    for (cur = pts, i=0 ; i<tabSize && cur != NULL; cur = cur->next, i++) {
        tabX[i] = cur->x;
        tabY[i] = cur->y;
    }
    
    pvm_initsend(PvmDataDefault);
    pvm_pkint(&tabSize, 1,1);
    pvm_pkint(tabX, tabSize, 1);
    pvm_pkint(tabY, tabSize, 1);

	if (pts2 != NULL)
	{
		int tabSize2 = point_nb(pts);
		int *tabX2 = malloc(tabSize2 * sizeof(int));
		int *tabY2 = malloc(tabSize2 * sizeof(int));
		for (cur = pts2, i=0 ; i<tabSize2 && cur != NULL; cur = cur->next, i++) {
			tabX2[i] = cur->x;
			tabY2[i] = cur->y;
		}
		pvm_pkint(&tabSize2, 1,1);
		pvm_pkint(tabX2, tabSize2, 1);
		pvm_pkint(tabY2, tabSize2, 1);
	}

    pvm_send(tid, (pts2 != NULL) ? MSG_MERGE : MSG_UH);

	point_free(pts);
	point_free(pts2);
}

point *master_receive_point(int *sender)
{
	int bufid, bytes[1], msgtag[1];

	bufid = pvm_recv(-1, MSG_RESULT);
	pvm_bufinfo(bufid, bytes, msgtag, sender);

	int tabSize;
	pvm_upkint(&tabSize, 1, 1);
	int *tabX = malloc(tabSize * sizeof(int));
	int *tabY = malloc(tabSize * sizeof(int));
	pvm_upkint(tabX, tabSize, 1);
	pvm_upkint(tabY, tabSize, 1);

	point *first = point_alloc();
	point *actual = first;
	actual->x = tabX[0];
	actual->y = tabY[0];


	for (int i=1; i<tabSize; i++) {
		point *prec = actual;
		actual = point_alloc();
		actual->x = tabX[i];
		actual->y = tabY[i];
		prec->next = actual;
	}
	return first;
}

/*
 * Calcul de l'enveloppe convexe
 */
void upper_hull(point *pts)
{
    /** TODO : bouger cette explication
     * Un seul père :
     * Génère plein de fils et distribue les tâches (potentiellement par une pile)
     * voir MSSrtCorrige
     * 
     * Le père se charge de la séparation en sous-ensembles
	 * 
	 * upper.c : maitre | upper_slave.c : esclave
	 * 
	 * Procédure maitre :
	 * 	- Divise en sous-ensembles de 4 points ou moins
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

	// Allocation des piles
	printf("Division en groupe de 4 max :\n");
	int nb_points = point_nb(pts);
	to_UH = (point **)malloc(((nb_points / 4) + 1) * sizeof(point *));
	to_merge = (point **)malloc(((nb_points / 4) + 1) * sizeof(point *));

	// Division en sous-ensembles de 4 points maximum
	point* prec = NULL;
	while (pts != NULL) 
	{
		if (prec != NULL) prec->next = NULL;
		empile_to_UH(pts);
		for (size_t j=0; j < 4; j++)
		{
			if (pts != NULL)
			{
				prec = pts;
				pts = pts->next;
			}
		}
	}
	
	point *prf;
	while ((prf = depile_to_UH()))
	{
		empile_to_merge(prf);
		printf("   - ");
		printf_point(prf);
	}
	while ((prf = depile_to_merge()))
	{
		empile_to_UH(prf);
		// printf("  ..");
		// printf_point(prf);
	}

	// Calcul de l'UH
	int tids[NB_SLAVE]; /* tids fils */
	int sender; /* tid du dernier envoyeur */

	pvm_spawn(EPATH "/upper_slave", (char**)0, 0, "", NB_SLAVE, tids);

	int inactive_slave = NB_SLAVE; /* nombre d'esclaves inactifs (pour quitter la boucle) */

	/* envoi à chaque esclave */
	for (size_t i=0; to_UH_nb>0 && i<NB_SLAVE; i++) { // Envoi d'un sous-ensemble à chaque esclave (dans la mesure des sous-ensembles disponibles)
		printf("Envoie à l'esclave %d de : ", tids[i]);
		point *tempp = depile_to_UH();
		printf_point(tempp);
		empile_to_UH(tempp);
		master_send_point(tids[i], depile_to_UH(), NULL);
		inactive_slave--;
	}
		
	printf("Calcul d'UH : \n");
	printf("Nombre d'UH à calculer restant : %d\n", to_UH_nb);
	while (to_UH_nb>0) // S'il reste des sous-ensembles à l'UH non-calculé
	{
		printf("En attente d'un retour d'esclave...\n");
		empile_to_merge(master_receive_point(&sender)); // On reçoit l'ensemble de point et l'empile dans la pile des ensembles à fusionner
	
		printf("Reception par %d de : ", sender);
		point *tempp = depile_to_merge();
		printf_point(tempp);
		empile_to_merge(tempp);
		
		printf("Envoie à l'esclave %d de : ", sender);
		tempp = depile_to_UH();
		printf_point(tempp);
		empile_to_UH(tempp);

		master_send_point(sender, depile_to_UH(), NULL); // On donne à l'esclave ayant répondu un autre ensemble pour lequel calculer l'UH
	}

	printf("Fusion : \n");
	printf("Nombre de fusion à effectuer restant : %d\n", to_merge_nb);

	while (1) // Lorsque tout les UH de sous-ensemble ont été calculé
	{
		printf("En attente d'un retour d'esclave...\n");
		empile_to_merge(master_receive_point(&sender)); // On reçoit l'ensemble de point et l'empile dans la pile des ensembles à fusionner
		
		printf("Reception par %d de : ", sender);
		point *tempp = depile_to_merge();
		printf_point(tempp);
		empile_to_merge(tempp);

		printf("Nombre d'ensembles à fusionner restant : %d\n", to_merge_nb);
		printf("Ensembles à fusionner restant : \n");
		while ((prf = depile_to_merge()))
		{
			empile_to_UH(prf);
			printf("   - ");
			printf_point(prf);
		}
		while ((prf = depile_to_UH()))
		{
			empile_to_merge(prf);
			// printf("   - ");
			// printf_point(prf);
		}

		if (to_merge_nb == 1) { // S'il reste un seul ensemble de points à fusionner
			inactive_slave++; // On déclare l'esclave inactif
			printf("Rien à faire : l'esclave %d s'endort\n", sender);
			printf("Esclaves endormies : %d\n", inactive_slave);
			if (inactive_slave == NB_SLAVE) break; // Si tout les esclaves sont inactifs et qu'il ne reste qu'un ensemble : fin
			continue; // Si on attend la réponse d'autre esclave : on réitère
		}

		printf("Fusion par %d\n", sender);
		master_send_point(sender, depile_to_merge(), depile_to_merge()); // On envoie les deux ensembles à fusionner
	}

	pts = depile_to_merge(); // On récupère le résultat à afficher
	
	pvm_mcast(tids, NB_SLAVE, MSG_END); // Envoi du message de fin aux esclaves
	pvm_exit();

	// Libération des piles
	free(to_UH);
	to_UH = NULL;
	free(to_merge);
	to_merge = NULL;

	printf("Résultat : ");
	printf_point(pts);
}

/*
 * upper <nb points>
 * exemple :
 * % upper 200 
 * % evince upper_hull.pdf
 */
int main(int argc, char **argv)
{
	pvm_catchout(stdout);
	point *pts;
	if (argc != 2) {
		fprintf(stderr, "usage: %s <nb points>\n", *argv);
		printf("-2\n");
		exit(-1);
	}
	pts = point_random(atoi(argv[1]));
	point_print_gnuplot(pts, 0); /* affiche l'ensemble des points */
	printf("PTS before : ");
	printf_point(pts);
	upper_hull(pts);
	printf("PTS after : ");
	printf_point(pts);
	point_print_gnuplot(pts, 1); /* affiche l'ensemble des points restant, i.e
					l'enveloppe, en reliant les points */
	point_free(pts);
}