#ifndef STRUCTS_H

#define STRUCTS_H

#include <stdbool.h>

//Structure des salles
typedef struct{
    int id;
    int height; // Hauteur de la pièce
    int width;  // Largeur de la pièce
    char** grid; // Matrice de caractères : W (wall), R (rock), G (gap), D (door), S (spike), H (item)
}Room;

// 2. Structure des Objets
typedef struct {
    char name[50];
    float hpMax;   // Points de vie maximum
    int shield;    // Bouclier (Points de vie bleus)
    float dmg;     // Dégâts de l'entité
    bool ps;       // Tir perçant (Pass à travers les ennemis)
    bool ss;       // Tir spectral (Pass à travers les rochers)
    bool flight;   // Vol (Passer par-dessus les trous/gaps)
} Item;

// 3. Structure d'Entité : Joueur ou Monstre
typedef struct {
    char name[50]; // Nom de l'entité
    float hpMax;   // Points de vie maximum
    float dmg;     // Dégâts infligés
    int x, y;      // Coordonnées de position dans la salle
    bool shoot;    // Capacité à tirer des projectiles
    bool ss;       // Tir spectral (Pass à travers les rochers)
    bool flight;   // Vol (Passer par-dessus les trous/gaps)
} Entity;

#endif /* STRUCTS_H */

//