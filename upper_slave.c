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

void slave_send_point(int tid, point *pts)
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
    pvm_pkint(&tabSize, 1,1);
    pvm_pkint(tabX, tabSize, 1);
    pvm_pkint(tabY, tabSize, 1);

    pvm_send(tid, MSG_RESULT);

	free(tabX);
	free(tabY);

	point_free(pts);
}

int slave_receive_point(int tid)
{
	int bufid, bytes[1], msgtag[1], sender[1];

	printf("En attente d'un message du maitre...\n");
	bufid = pvm_recv(tid, -1);
	printf("Message recu : ");
	pvm_bufinfo(bufid, bytes, msgtag, sender);

	if (msgtag[0] == MSG_END) printf("FIN.\n");
	if (msgtag[0] == MSG_END) return 0;

	int merge = (msgtag[0] == MSG_MERGE);
	if (merge) printf("FUSIONNE.\n");
	else printf("CALCUL L'UH.\n");

	int tabSize;
	pvm_upkint(&tabSize, 1, 1);
	int *tabX = malloc(tabSize * sizeof(int));
	int *tabY = malloc(tabSize * sizeof(int));
	pvm_upkint(tabX, tabSize, 1);
	pvm_upkint(tabY, tabSize, 1);

	point *pts = point_alloc();
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

	printf("Chaine 1 : ");
	printf_point(pts);

	if (!merge)
	{
		point_UH(pts);
	}
	else
	{
		pvm_upkint(&tabSize, 1, 1);
		tabX = malloc(tabSize * sizeof(int));
		tabY = malloc(tabSize * sizeof(int));
		pvm_upkint(tabX, tabSize, 1);
		pvm_upkint(tabY, tabSize, 1);

		point *pts2 = point_alloc();
		actual = pts2;
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

		printf("Chaine 2 : ");
		printf_point(pts2);

		point_merge_UH(pts, pts2);

		point_free(pts2);
	}

	printf("Send : ");
	printf_point(pts);

	slave_send_point(tid, pts);
	
	printf("Renvoyé au maître.\n");

	return 1;
}

int main(void)
{
	while (slave_receive_point(pvm_parent()));
	exit(EXIT_SUCCESS);
}
