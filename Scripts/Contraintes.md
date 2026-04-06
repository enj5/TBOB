# Idées

# CRUD des pièces du donjon
>_create, read, update, delete_


## A - Pour la création et le remplissage des salles

**_L'UTILISATEUR ENTRE, MODIFIE, CREER, ETC... TOUT CE QU'IL VEUT. SEULEMENT QUAND IL A FINI, ON STOCKE DANS LE FICHIER !!!!_**

**_A chaque modif, on regénre le fichier avec toutes les salles, à défaut de le faire dynamique comme l'IA_**

**_Stocker les salles dans un tableau de structure de donnée dont les champs seront le numéro de l'étage (ou num de la salle), la taille (ligne et colonne) et le tableau de la salle (pointeur)_**

- Imposer nb_ligne et nb_colonne impair
- nb_ligne < nb_colonne
- On propose à l'utilisateur d'entrer nb_ligne et nb_colonne
- On lui propose de définir les obstacles à l'intérieur de l'espace
  - Il devra entrer les indices, ainsi que le type d'obstacle (G, H) 
  - Il pourra entrer un obstacle autant de fois qu'il veut
    Jusqu'une certaine limite qu'on va fixer peut-être
  - On restraint les indices possibles utilisable pour placer les obstacles
>Autre idée de création : faire un peut comme dans le while(running) de Miguel où à chaque fois que l'utilisateur fait un déplacement, ça regénère la salle et affiche la ouvelle position. On peut appliquer ça à la modification des salles et au choix de la position pour placer les obstacles. Mod-Op: On lui demande si il veut ajouter ou supp. un obstacle, si il veut ajouter, il choisi G ou H, et il pourra voir en temps réel l'obstacle et le déplacer à gauche, à droite, en haut ou en bas. Même chose pour la suppression, un s qui se balade et sur sa position finale, il remplace par un vide.
- On peut également stocker une salle par défaut. L'utilisateur pourra la modifier, la conserver ou la supprimer.

**_Comment gérer le stockage efficace des salles ??_**

## Construction du code de la création des salles

- Découpage en fonctions : 
  - Création (fonction)
  - Modification (procédure)
  - Affichage (procédure)
  - Suppression (procédure) : chercher comment gérer ça