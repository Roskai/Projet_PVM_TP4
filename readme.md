# Construction parallèle de l'enveloppe convexe haute d'un ensemble de points dans le plan.
- [Construction parallèle de l'enveloppe convexe haute d'un ensemble de points dans le plan.](#construction-parallèle-de-lenveloppe-convexe-haute-dun-ensemble-de-points-dans-le-plan)
	- [Descriptif du Problème : Construction de l'Enveloppe Convexe Haute](#descriptif-du-problème--construction-de-lenveloppe-convexe-haute)
		- [Introduction :](#introduction-)
		- [Enveloppe Convexe :](#enveloppe-convexe-)
		- [Diviser et Conquérir :](#diviser-et-conquérir-)
		- [Objectif de l'Algorithme :](#objectif-de-lalgorithme-)
	- [Présentation des Structures de Données :](#présentation-des-structures-de-données-)
		- [Justification des Choix de Structures pour Optimiser la Performance Parallèle :](#justification-des-choix-de-structures-pour-optimiser-la-performance-parallèle-)


**Auteurs :**
- Jules PROUST
- Angel GEZAT

--- 
## Descriptif du Problème : Construction de l'Enveloppe Convexe Haute

### Introduction :

La construction de l'enveloppe convexe haute conciste à trouver le polygone convexe le plus petit qui englobe un ensemble de points donné dans un plan. L'approche "diviser et conquérir" eva être utiliser pour résoudre efficacement ce problème en le subdivisant en sous-problèmes plus simples.

### Enveloppe Convexe :

L'enveloppe convexe d'un ensemble de points dans le plan est définie comme le plus petit polygone convexe qui contient tous les points de l'ensemble. Un polygone est dit convexe si, pour tout couple de points appartenant à l'ensemble, le segment de droite défini par le couple est entièrement contenu dans le polygone. En d'autres termes, l'enveloppe convexe est la frontière extérieure la plus simple qui englobe tous les points.

### Diviser et Conquérir :

La méthode "diviser et conquérir" est une stratégie algorithmique où un problème est divisé en sous-problèmes plus petits et plus simples qui sont résolus de manière récursive. Ensuite, les solutions partielles sont combinées pour obtenir la solution globale du problème. Dans le contexte de la construction de l'enveloppe convexe haute, cette approche est utilisée pour subdiviser l'ensemble de points en deux sous-ensembles plus petits, calculer l'enveloppe convexe de chaque sous-ensemble, puis fusionner les résultats pour obtenir l'enveloppe convexe haute globale.

### Objectif de l'Algorithme :

L'objectif principal de l'algorithme est de parvenir à une solution efficace pour construire l'enveloppe convexe haute d'un ensemble de points. En utilisant la méthode "diviser et conquérir", la complexité du problème est réduite en résolvant des sous-problèmes plus simples, ce qui permet une solution plus efficace.

Ce chapitre a jeté les bases du problème de construction de l'enveloppe convexe haute, en introduisant les termes clés tels que l'enveloppe convexe et la stratégie "diviser et conquérir". La suite du rapport explorera en détail l'algorithme parallèle utilisé, les structures de données choisies, et fournira une trace d'exécution avec une courbe d'enveloppe convexe si le projet est abouti.


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

### Justification des Choix de Structures pour Optimiser la Performance Parallèle :

- **Liste Chaînée :** L'utilisation d'une liste chaînée pour représenter les points de l'enveloppe convexe permet une manipulation aisée des points, notamment pour les opérations d'ajout en fin de liste. Cela peut être crucial pour les étapes où l'ordre des points est important.

- **Piles :** Les piles (`to_merge` et `to_UH`) sont des structures de données adaptées à la nature récursive de l'algorithme "diviser et conquérir". Elles simplifient la gestion des points à différentes étapes de l'algorithme, facilitant ainsi le traitement parallèle.

