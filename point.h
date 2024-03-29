/*
 * upper.h
 * SR 		13/04/94
 * LL 		14/10/97
 *
 *  Definition du programme upper
 */

#define AFFICHE 0 // 1 si affichage de la progression souhaité, 0 sinon

/**
 * Messages PVM utilisables
*/
#define MSG_RESULT 0
#define MSG_UH 1
#define MSG_MERGE 2
#define MSG_END 3

#define XMAX (1024)
#define YMAX (1024)


/*
 * structure associee a chaque point
 */
typedef struct st_point point;
struct st_point
{
    int x, y;
    point *next; /* liste chainee des points de l'enveloppe */
};

/*
 * dans point.c
 * utilitaire de calcul pour le TAD point
 */

/*
 * Affiche une chaine de point dans la console
*/
extern void point_printf();

/*
 * (des)allocation
 */
extern point *point_alloc();

/*
 * desalloue une liste de points
 */
extern void point_free();

/*
 * genere une liste de points, avec ligne si solid == 1
 * generation au format jgraph
 */
extern void point_print();

/*
 * au format gnuplot, dans 2 fichiers.
 * appele ensuite gnuplot
 */
extern void point_print_gnuplot();

/*
   Construit un ensemble de points d'abscisse comprise
   entre 0 et XMAX et d'ordonnee entre 0 et YMAX.
   Tous les points ont des abscisses et des ordonnees
   differentes. Les points sont trie's par ordre croissant
   des abscisses.
 */
extern point *point_random();

/*
   Calcule l'enveloppe convexe haute de 4 points au plus.
   Les points de l'enveloppe sont lies par une liste
   Les autres sont elimines de la liste.
 */
extern point *point_UH();

/*
 * nombre de points dans une liste
 */
extern int point_nb();

/*
 * partitionne une liste en deux.
 * retourne le milieu de la liste
 */
extern point *point_part();

/*
 * calcule l'enveloppe convexe commune de
 * deux enveloppes convexes
 */
extern point *point_merge_UH();

/*
 * calcul recursif d'enveloppe
 * convexe par bissection
 */
extern point *upper_hull();