// charger_salle.c

#include <stdio.h>
#include "game_strucs.h"

Room charger_salle_depuis_fichier(const char* nomFichier) {
    Room salle;
    FILE* fichier = fopen(nomFichier, "r");
    
    if (fichier == NULL) {
        printf("Erreur: Impossible d'ouvrir le fichier %s\n", nomFichier);
        return salle; 
    }

    // Lire la matrice de la salle (9 lignes, 15 colonnes)
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 15; j++) {
            fscanf(fichier, " %c", &salle.grid[i][j]); 
        }
    }

    fclose(fichier);
    return salle;
}