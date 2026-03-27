#ifndef SALLE_H
#define SALLE_H

#include <stdlib.h>
#include <stdio.h>

typedef struct Room {
    int hauteur;
    int largeur;
    int id;
    char **grille;
} Room;

// gestion salle
Room*  creer_salle(int hauteur, int largeur, int id);
void   afficher_salle(Room *salle);
void   liberer_salle(Room *salle);

// CRUD
void   modifier_salle(Room *salle);
void   supprimer_salle(Room **salles, int *nb_salles, int id);

// fichier .rtbob
void   sauvegarder_salles(Room **salles, int nb_salles);
Room** charger_salles(int *nb_salles);

#endif