# Construction parallèle de l'enveloppe convexe haute d'un ensemble de points dans le plan.

**Auteurs :**
- Jules PROUST
- Angel GEZAT

--- 

## Descriptif du Problème : Construction de l'Enveloppe Convexe Haute

L'objectif de ce projet est de développer un algorithme parallèle pour construire l'enveloppe convexe haute d'un ensemble de points dans le plan. L'enveloppe convexe d'un ensemble de points est le plus petit polygone convexe qui englobe tous les points. Un polygone est considéré comme convexe si le segment de droite défini par chaque paire de points dans l'ensemble est entièrement contenu à l'intérieur du polygone.

L'algorithme utilisé est de type "diviser et conquérir", une approche qui réduit la taille du problème jusqu'à ce qu'une solution puisse être calculée de manière simple. L'idée principale est de diviser l'ensemble de points en deux sous-ensembles, calculer l'enveloppe convexe haute pour chaque sous-ensemble, puis fusionner les résultats.

