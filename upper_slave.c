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

/**
 * Envoie d'un résultat de calcul par l'esclave au maitre :
 *   - tid : Id du maitre
 *   - pts : Chaine de point du résultat
 *   - id : Identifiant de la chaine pour que le maitre reconnaisse
 */
void slave_send_point(int tid, point *pts, int id)
{
    int tabSize = point_nb(pts);
	int *tabX = malloc(tabSize * sizeof(int));
	int *tabY = malloc(tabSize * sizeof(int));
    
    int i;
    point *cur ;
    for (cur = pts, i=0 ; i<tabSize && cur != NULL; cur = cur->next, i++) {
        tabX[i] = cur->x;
        tabY[i] = cur->y;
    }
    
    pvm_initsend(PvmDataDefault);
    pvm_pkint(&id, 1,1);
    pvm_pkint(&tabSize, 1,1);
    pvm_pkint(tabX, tabSize, 1);
    pvm_pkint(tabY, tabSize, 1);

    pvm_send(tid, MSG_RESULT);

	free(tabX);
	free(tabY);

	point_free(pts);
}

/**
 * Réception du calcul du maitre par l'esclave :
 *   - tid : Id du maitre
 */
int slave_receive_point(int tid)
{
	int bufid, bytes[1], msgtag[1], sender[1];
	#if AFFICHE
	printf("En attente d'un message du maitre...\n");
	#endif
	bufid = pvm_recv(tid, -1);
	pvm_bufinfo(bufid, bytes, msgtag, sender);

	#if AFFICHE
	printf("Message recu : ");
	if (msgtag[0] == MSG_END) printf("FIN.\n");
	#endif

	if (msgtag[0] == MSG_END) return 0;

	int merge = (msgtag[0] == MSG_MERGE);

	#if AFFICHE
	if (merge) printf("FUSIONNE.\n");
	else printf("CALCUL L'UH.\n");
	#endif

	int id;
	pvm_upkint(&id, 1, 1);

	point *pts;
	{
		int tabSize;
		pvm_upkint(&tabSize, 1, 1);
		int *tabX = malloc(tabSize * sizeof(int));
		int *tabY = malloc(tabSize * sizeof(int));
		pvm_upkint(tabX, tabSize, 1);
		pvm_upkint(tabY, tabSize, 1);

		pts = point_alloc();
		point *actual = pts;
		actual->x = tabX[0];
		actual->y = tabY[0];

		for (int i=1; i<tabSize; i++) {
			point *prec = actual;
			actual = point_alloc();
			actual->x = tabX[i];
			actual->y = tabY[i];
			prec->next = actual;
		}

		free(tabX);
		free(tabY);

		#if AFFICHE
		printf("Chaine 1 : ");
		point_printf(pts);
		#endif
	}


	if (!merge) point_UH(pts);
	else
	{
		point* pts2;

		int tabSize;
		pvm_upkint(&tabSize, 1, 1);
		int *tabX = malloc(tabSize * sizeof(int));
		int *tabY = malloc(tabSize * sizeof(int));
		pvm_upkint(tabX, tabSize, 1);
		pvm_upkint(tabY, tabSize, 1);

		pts2 = point_alloc();
		point *actual = pts2;
		actual->x = tabX[0];
		actual->y = tabY[0];

		for (int i=1; i<tabSize; i++) {
			point *prec = actual;
			actual = point_alloc();
			actual->x = tabX[i];
			actual->y = tabY[i];
			prec->next = actual;
		}

		free(tabX);
		free(tabY);

		#if AFFICHE
		printf("Chaine 2 : ");
		point_printf(pts2);
		#endif

		point_merge_UH(pts, pts2);
	}

	#if AFFICHE
	printf("Renvoie au maitre de : ");
	point_printf(pts);
	#endif

	slave_send_point(tid, pts, id);

	return 1;
}

/**
 * Main de l'esclave
*/
int main(void)
{
	while (slave_receive_point(pvm_parent())); // Tant qu'on a pas de message FIN : On reçoit
	#if AFFICHE
	printf("Arrêt !\n");
	#endif
	exit(EXIT_SUCCESS);
}
