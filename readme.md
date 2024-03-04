# Construction parallèle de l'enveloppe convexe haute d'un ensemble de points dans le plan.
- [Construction parallèle de l'enveloppe convexe haute d'un ensemble de points dans le plan.](#construction-parallèle-de-lenveloppe-convexe-haute-dun-ensemble-de-points-dans-le-plan)
	- [Descriptif du Problème : Construction de l'Enveloppe Convexe Haute](#descriptif-du-problème--construction-de-lenveloppe-convexe-haute)
	- [Présentation des Structures de Données :](#présentation-des-structures-de-données-)



**Auteurs :**
- Jules PROUST
- Angel GEZAT

--- 

## Descriptif du Problème : Construction de l'Enveloppe Convexe Haute

L'objectif de ce projet est de développer un algorithme parallèle pour construire l'enveloppe convexe haute d'un ensemble de points dans le plan. L'enveloppe convexe d'un ensemble de points est le plus petit polygone convexe qui englobe tous les points. Un polygone est considéré comme convexe si le segment de droite défini par chaque paire de points dans l'ensemble est entièrement contenu à l'intérieur du polygone.

L'algorithme utilisé est de type "diviser et conquérir", une approche qui réduit la taille du problème jusqu'à ce qu'une solution puisse être calculée de manière simple. L'idée principale est de diviser l'ensemble de points en deux sous-ensembles, calculer l'enveloppe convexe haute pour chaque sous-ensemble, puis fusionner les résultats.

## Présentation des Structures de Données :

1. **Structure point :**
    - **Description :** La structure `point` représente un point dans le plan, caractérisé par ses coordonnées (x, y).
    - **Attributs :**
        - `int x, y` : Coordonnées du point.
        - `point *next` : Pointeur vers le point suivant dans la liste chaînée des points de l'enveloppe convexe.

2. **Variable statique `to_merge` :**
    - **Description :** La variable statique `to_merge` est une pile de pointeurs vers des points. Elle est utilisée pour stocker les points à fusionner pendant l'algorithme.
    - **Justification :** L'utilisation d'une pile est pertinente ici, car elle suit la logique de la méthode "diviser et conquérir". Elle permet de gérer de manière efficace les points à fusionner à différentes étapes de l'algorithme.

3. **Variable statique `to_UH` :**
    - **Description :** La variable statique `to_UH` est une pile de pointeurs vers des points. Elle est utilisée pour stocker les points à calculer lors de la construction de l'enveloppe convexe haute.
    - **Justification :** De manière similaire à `to_merge`, l'utilisation d'une pile pour `to_UH` est appropriée pour gérer les points à calculer lors de l'algorithme. Elle facilite l'organisation et la récupération des points nécessaires à chaque étape.

**Justification des Choix de Structures pour Optimiser la Performance Parallèle :**

- **Liste Chaînée :** L'utilisation d'une liste chaînée pour représenter les points de l'enveloppe convexe permet une manipulation aisée des points, notamment pour les opérations d'ajout en fin de liste. Cela peut être crucial pour les étapes où l'ordre des points est important.

- **Piles :** Les piles (`to_merge` et `to_UH`) sont des structures de données adaptées à la nature récursive de l'algorithme "diviser et conquérir". Elles simplifient la gestion des points à différentes étapes de l'algorithme, facilitant ainsi le traitement parallèle.

En résumé, les structures de données choisies ont été sélectionnées en fonction des exigences spécifiques de l'algorithme, en privilégiant la simplicité d'accès et de manipulation pour optimiser les performances parallèles. La combinaison d'une liste chaînée pour représenter les points et de piles pour gérer les opérations récursives contribue à une implémentation efficace de l'algorithme de construction de l'enveloppe convexe haute.