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

static int pb_max; /* Nombre de problème maximum dans to_UH et to_merge */

static int slave_queue[NB_SLAVE];
static int slave_queue_nb = 0;
int depile_slave()
{
	if (slave_queue_nb > 0)
		return slave_queue[--slave_queue_nb];
	else
		return -1;
}
void empile_slave(int tid)
{
	slave_queue[slave_queue_nb++] = tid;
}


static point **to_UH; /* La pile de points à calculer */
static int to_UH_nb = 0; /* l'index de tete de pile */
point *get_and_remove_to_UH(int pos)
{
	point *p = to_UH[pos];
	to_UH[pos] = NULL;
	to_UH_nb--;
	return p;
}
void add_to_UH(int pos, point *pts)
{
	if(to_UH[pos] == NULL) to_UH_nb++;
	to_UH[pos] = pts;
}
int first_to_UH()
{
	for (size_t i = 0; i < pb_max; i++)
		if (to_UH[i] != NULL) return i;
}

static point **to_merge; /* La pile des points à fusionner */
static int to_merge_nb = 0; /* l'index de tete de pile */

static int to_merge_merged_nb = 0; /* l'index de tete de pile */
static int *to_merge_status; /* Du statut des points à fusionner (0 = en calcul , 1 = fusionné, 2 = Prêt à être fusionné ) */
#define MERGING 0
#define MERGED 1
#define READY_TO_MERGE 2
static point **to_merge; /* Le tableau des points à fusionner */
point *get_and_remove_to_merge(int pos)
{
	point *p = to_merge[pos];
	to_merge[pos] = NULL;
	to_merge_nb--;
	to_merge_status[pos] = MERGING;
	return p;
}
void add_to_merge(int pos, point *pts)
{
	if(to_merge[pos] == NULL) to_merge_nb++;
	to_merge[pos] = pts;
	to_merge_status[pos] = READY_TO_MERGE;
}
void set_merged(int pos) {
	get_and_remove_to_merge(pos);
	to_merge_status[pos] = MERGED;
	to_merge_merged_nb++;
}
void get_points_to_merge(int *ppos1, int *ppos2) {
	int pos1 = -1;
	int pos2 = -1;
	for (size_t i = 0; i < pb_max; i++)
	{
		if (to_merge_status[i] == READY_TO_MERGE) {
			if (pos1 == -1) {
				pos1 = i;
			} else {
				pos2 = i;
				break;
			}
		} else if (to_merge_status[i] == MERGING) {
			pos1 = -1;
		}
	}
	ppos1 = &pos1;
	ppos2 = &pos2;
}
int solved() {
	return (to_merge_nb == 1 && to_merge_merged_nb == pb_max-1);
}


void master_send_point(int tid, point *pts, point *pts2, int id)
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
    pvm_pkint(&id, 1,1);
    pvm_pkint(&tabSize, 1,1);
    pvm_pkint(tabX, tabSize, 1);
    pvm_pkint(tabY, tabSize, 1);

	if (pts2 != NULL)
	{
		int tabSize2 = point_nb(pts2);
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

void master_receive_point()
{
	int bufid, bytes[1], msgtag[1], sender[1];

	bufid = pvm_recv(-1, MSG_RESULT);
	pvm_bufinfo(bufid, bytes, msgtag, sender);

	int id;
	pvm_upkint(&id, 1, 1);
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

	#if AFFICHE
	printf("Reception par %d de : ", sender[0]);
	point_printf(first);
	#endif

	add_to_merge(id, first);
	empile_slave(sender[0]);
}

/*
 * Calcul de l'enveloppe convexe
 */
point *upper_hull(point *points)
{
	point *pts = points;
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

	// Allocations
	#if AFFICHE
	printf("Division en groupe de 4 max :\n");
	#endif
	int nb_points = point_nb(pts);
	pb_max = ((nb_points / 4) + (nb_points%4 == 0 ? 0 : 1));
	to_UH = (point **)malloc(pb_max * sizeof(point *));
	to_merge = (point **)malloc(pb_max * sizeof(point *));
	to_merge_status = (int *)malloc(pb_max * sizeof(int));
	for (size_t i = 0; i < pb_max; i++)
	{
		to_UH[i] = NULL;
		to_merge[i] = NULL;
		to_merge_status[i] = MERGING;
	}
	

	// Division en sous-ensembles de 4 points maximum
	int actual_id = 0;
	point* prec = NULL;
	while (pts != NULL) 
	{
		if (prec != NULL) prec->next = NULL;
		add_to_UH(actual_id++, pts);
		for (size_t j=0; j < 4; j++)
		{
			if (pts != NULL)
			{
				prec = pts;
				pts = pts->next;
			}
		}
	}

	#if AFFICHE
	for (size_t i = 0; i < pb_max; i++)
	{
		printf("   - ");
		point_printf(to_UH[i]);
	}
	#endif

	// Apparition des esclaves
	pvm_spawn(EPATH "/upper_slave", (char**)0, 0, "", NB_SLAVE, slave_queue);
	slave_queue_nb = NB_SLAVE;

	// Calcul de l'UH
	/* Envoi à chaque esclave */
	for (size_t i=0; i<pb_max && i<NB_SLAVE; i++) { // Envoi d'un sous-ensemble à chaque esclave (dans la mesure des sous-ensembles disponibles)
		#if AFFICHE
		int sl = depile_slave();
		printf("Envoie à l'esclave %d de : ", sl);
		empile_slave(sl);
		point_printf(to_UH[i]);
		#endif
		master_send_point(depile_slave(), get_and_remove_to_UH(i), NULL, i);
	}

	#if AFFICHE
	printf("Calcul d'UH : \n");
	printf("Nombre d'UH à calculer restant : %d\n", to_UH_nb);
	#endif
	while (to_UH_nb>0) // S'il reste des sous-ensembles à l'UH non-calculé
	{
		int sender;
		#if AFFICHE
		printf("En attente d'un retour d'esclave...\n");
		#endif
		master_receive_point(); // On reçoit l'ensemble de point et l'ajoute au tableau des ensembles à fusionner
		
		int id = first_to_UH();

		#if AFFICHE
		int sl = depile_slave();
		printf("Envoie à l'esclave %d de : ", sl);
		empile_slave(sl);
		point_printf(to_UH[id]);
		#endif

		master_send_point(depile_slave(), get_and_remove_to_UH(id), NULL, id); // On donne à l'esclave ayant répondu un autre ensemble pour lequel calculer l'UH
	}

	#if AFFICHE
	printf("Fusion : \n");
	printf("Nombre de fusion à effectuer restant : %d\n", to_merge_nb);
	#endif
	while (!solved() && slave_queue_nb != NB_SLAVE) // Lorsque tout les UH de sous-ensemble ont été calculé
	{
		#if AFFICHE
		printf("En attente d'un retour d'esclave...\n");
		#endif

		master_receive_point(); // On reçoit l'ensemble de point et l'ajoute au tableau des ensembles à fusionner

		#if AFFICHE
		printf("Nombre d'ensembles à fusionner restant : %d\n", to_merge_nb);
		printf("Ensembles à fusionner restant : \n");
		for (size_t i = 0; i < pb_max; i++)
		{
			printf("   %d - ",i);
			if (to_merge_status[i] == READY_TO_MERGE)
				point_printf(to_merge[i]);
			else if (to_merge_status[i] == MERGED)
				printf("MERGED\n");
			else
				printf("MERGING\n");
		}
		#endif

		int pos1 = -1;
		int pos2 = -1;
		for (size_t i = 0; i < pb_max; i++)
		{
			if (to_merge_status[i] == READY_TO_MERGE) {
				if (pos1 == -1) {
					pos1 = i;
				} else {
					pos2 = i;
					break;
				}
			} else if (to_merge_status[i] == MERGING) {
				pos1 = -1;
			}
		}
		while (pos1 != -1 && pos2 != -1 && slave_queue_nb > 0)
		{
			#if AFFICHE
			int sl = depile_slave();
			printf("Fusion par %d de :\n", sl);
			empile_slave(sl);
			printf("   %d - ",pos1);
			point_printf(to_merge[pos1]);
			printf("   %d - ",pos2);
			point_printf(to_merge[pos2]);
			#endif

			master_send_point(depile_slave(),
				get_and_remove_to_merge(pos1),
				get_and_remove_to_merge(pos2),
				pos1);
			set_merged(pos2);

			pos1 = -1;
			pos2 = -1;
			for (size_t i = 0; i < pb_max; i++)
			{
				if (to_merge_status[i] == READY_TO_MERGE) {
					if (pos1 == -1) {
						pos1 = i;
					} else {
						pos2 = i;
						break;
					}
				} else if (to_merge_status[i] == MERGING) {
					pos1 = -1;
				}
			}
		}
	}

	for (size_t i = 0; i < pb_max; i++)
	{
		if (to_merge_status[i] == READY_TO_MERGE) {
			pts = to_merge[i];
			break;
		}
	}
	
	
	pvm_mcast(slave_queue, NB_SLAVE, MSG_END); // Envoi du message de fin aux esclaves
	pvm_exit();

	// Libération des piles et tableaux
	free(to_UH);
	to_UH = NULL;
	free(to_merge);
	to_merge = NULL;
	free(to_merge_status);
	to_merge_status = NULL;

	#if AFFICHE
	printf("Résultat : ");
	point_printf(pts);
	#endif

	return pts;
}

/*
 * upper <nb points>
 * exemple :
 * % upper 200 
 * % evince upper_hull.pdf
 */
int main(int argc, char **argv)
{
	#if AFFICHE
	pvm_catchout(stdout);
	#endif
	
	point *pts;
	if (argc != 2) {
		fprintf(stderr, "usage: %s <nb points>\n", *argv);
		exit(-1);
	}
	pts = point_random(atoi(argv[1]));
	point_print_gnuplot(pts, 0); /* affiche l'ensemble des points */

	#if AFFICHE
	printf("PTS before : ");
	point_printf(pts);
	#endif

	pts = upper_hull(pts);

	#if AFFICHE
	printf("PTS after : ");
	point_printf(pts);
	#endif

	point_print_gnuplot(pts, 1); /* affiche l'ensemble des points restant, i.e
					l'enveloppe, en reliant les points */
	point_free(pts);
}