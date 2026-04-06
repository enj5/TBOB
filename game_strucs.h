#include <stdbool.h>

// 1. Structure de la Salle (Personne A)
typedef struct {
    int id;
    int height; // Par défaut 9 (Hauteur de la pièce)
    int width;  // Par défaut 15 (Largeur de la pièce)
    char grid[50][50]; // Matrice de caractères : W (wall), R (rock), G (gap), D (door), S (spike), H (item)
} Room;

// 2. Structure des Statistiques et Objets (Personne B)
typedef struct {
    char name[50];
    float hpMax;   // Points de vie maximum
    int shield;    // Bouclier (Points de vie bleus)
    float dmg;     // Dégâts de l'entité
    bool ps;       // Tir perçant (Pass à travers les ennemis)
    bool ss;       // Tir spectral (Pass à travers les rochers)
    bool flight;   // Vol (Passer par-dessus les trous/gaps)
} Item;

// 3. Structure d'Entité : Joueur ou Monstre (Personne B et C)
typedef struct {
    float hp;      // Points de vie actuels
    float dmg;     // Dégâts infligés
    int x, y;      // Coordonnées de position dans la salle
    bool can_shoot; // Capacité à tirer des projectiles
    bool flight;   // Vol (Passer par-dessus les trous/gaps)
    // ... autres champs d'état (ex: vitesse de déplacement)
} Entity;