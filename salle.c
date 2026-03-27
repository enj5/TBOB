#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "salle.h"

#define NOM_FICHIER "jeu.rtbob"

Room* creer_salle(int hauteur, int largeur, int id) {
    Room *salle = malloc(sizeof(*salle));
    salle->hauteur = hauteur;
    salle->largeur = largeur;
    salle->id = id;

    salle->grille = malloc(hauteur * sizeof(*salle->grille));
    for (int i = 0; i < hauteur; i++) {
        salle->grille[i] = malloc(largeur * sizeof(*salle->grille[i]));
    }

    // remplir : intérieur vide et murs extérieurs
    for (int i = 0; i < hauteur; i++) {
        for (int j = 0; j < largeur; j++) {
            if (i == 0 || i == hauteur-1 || j == 0 || j == largeur-1)
                salle->grille[i][j] = 'W';
            else
                salle->grille[i][j] = ' ';
        }
    }

    // portes D au milieu de chaque côté
    salle->grille[0][largeur/2]         = 'D';
    salle->grille[hauteur-1][largeur/2] = 'D';
    salle->grille[hauteur/2][0]         = 'D';
    salle->grille[hauteur/2][largeur-1] = 'D';

    // Coin haut-gauche
    salle->grille[1][2] = 'G';
    salle->grille[2][1] = 'G';
    salle->grille[2][2] = 'G';

    // Coin haut-droit
    salle->grille[1][largeur-3] = 'G';
    salle->grille[2][largeur-2] = 'G';
    salle->grille[2][largeur-3] = 'G';

    // Coin bas-gauche
    salle->grille[hauteur-3][1] = 'G';
    salle->grille[hauteur-3][2] = 'G';
    salle->grille[hauteur-2][2] = 'G';

    // Coin bas-droit
    salle->grille[hauteur-3][largeur-2] = 'G';
    salle->grille[hauteur-3][largeur-3] = 'G';
    salle->grille[hauteur-2][largeur-3] = 'G';

    srand(time(NULL));

    // surface intérieure
    int surface = (hauteur - 2) * (largeur - 2);
    int nb_R = surface / 20;
    int nb_S = surface / 25;

    // placement R et S
    char items[] = {'R', 'S'};
    int counts[] = {nb_R, nb_S};

    for (int t = 0; t < 2; t++) {
        int placed = 0;
        int attempts = 0;
        while (placed < counts[t] && attempts < 10000) {
            int i = 1 + rand() % (hauteur - 2);
            int j = 1 + rand() % (largeur - 2);
            if (salle->grille[i][j] == ' '
                && !(i == 1         && j == largeur/2)
                && !(i == hauteur-2 && j == largeur/2)
                && !(i == hauteur/2 && j == 1)
                && !(i == hauteur/2 && j == largeur-2))
            {
                salle->grille[i][j] = items[t];
                placed++;
            }
            attempts++;
        }
    }

    // placement H dans les creux des coins
    int coins_H[4][2] = {
        {1,         1},
        {1,         largeur-2},
        {hauteur-2, 1},
        {hauteur-2, largeur-2}
    };

    for (int k = 3; k > 0; k--) {
        int r = rand() % (k + 1);
        int tmp0 = coins_H[k][0]; coins_H[k][0] = coins_H[r][0]; coins_H[r][0] = tmp0;
        int tmp1 = coins_H[k][1]; coins_H[k][1] = coins_H[r][1]; coins_H[r][1] = tmp1;
    }

    int nb_H_coins = 1 + rand() % 4;
    for (int k = 0; k < nb_H_coins; k++) {
        salle->grille[coins_H[k][0]][coins_H[k][1]] = 'H';
    }

    return salle;
}

void afficher_salle(Room *salle) {
    printf("\nSalle %d (%d x %d)\n", salle->id, salle->hauteur, salle->largeur);
    for (int i = 0; i < salle->hauteur; i++) {
        for (int j = 0; j < salle->largeur; j++) {
            printf("%c ", salle->grille[i][j]);
        }
        printf("\n");
    }
}

void liberer_salle(Room *salle) {
    for (int i = 0; i < salle->hauteur; i++) {
        free(salle->grille[i]);
    }
    free(salle->grille);
    free(salle);
}

void modifier_salle(Room *salle) {
    int ligne, colonne;
    char valeur;

    afficher_salle(salle);

    printf("\nElements modifiables : R, S (espace pour vider)\n");
    printf("Ligne (1 a %d) : ", salle->hauteur - 2);
    scanf("%d", &ligne);
    printf("Colonne (1 a %d) : ", salle->largeur - 2);
    scanf("%d", &colonne);

    if (salle->grille[ligne][colonne] == 'W' || salle->grille[ligne][colonne] == 'D') {
        printf("Impossible de modifier un mur (W) ou une porte (D) !\n");
        return;
    }

    printf("Valeur (R, S, espace pour vider) : ");
    scanf(" %c", &valeur);

    if (valeur != 'R' && valeur != 'S' && valeur != ' ') {
        printf("Valeur invalide !\n");
        return;
    }

    salle->grille[ligne][colonne] = valeur;
    printf("Case (%d, %d) modifiee avec succes.\n", ligne, colonne);
}

void supprimer_salle(Room **salles, int *nb_salles, int id) {
    int idx = -1;
    for (int i = 0; i < *nb_salles; i++) {
        if (salles[i]->id == id) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("Salle %d introuvable.\n", id);
        return;
    }

    liberer_salle(salles[idx]);

    for (int i = idx; i < *nb_salles - 1; i++) {
        salles[i] = salles[i + 1];
    }
    (*nb_salles)--;
    printf("Salle %d supprimee.\n", id);
}

void sauvegarder_salles(Room **salles, int nb_salles) {
    FILE *f = fopen(NOM_FICHIER, "w");
    if (!f) {
        printf("Erreur : impossible d'ouvrir %s\n", NOM_FICHIER);
        return;
    }

    fprintf(f, "{%d}\n", nb_salles);

    for (int s = 0; s < nb_salles; s++) {
        Room *salle = salles[s];
        fprintf(f, "[%d|%d] %d\n", salle->hauteur, salle->largeur, salle->id);
        for (int i = 0; i < salle->hauteur; i++) {
            for (int j = 0; j < salle->largeur; j++) {
                if (j > 0) fprintf(f, " ");
                fprintf(f, "%c", salle->grille[i][j]);
            }
            fprintf(f, "\n");
        }
    }

    fclose(f);
    printf("Salles sauvegardees dans %s\n", NOM_FICHIER);
}

Room** charger_salles(int *nb_salles) {
    FILE *f = fopen(NOM_FICHIER, "r");
    if (!f) {
        printf("Erreur : impossible d'ouvrir %s\n", NOM_FICHIER);
        return NULL;
    }

    fscanf(f, "{%d}\n", nb_salles);

    Room **salles = malloc(*nb_salles * sizeof(Room *));

    for (int s = 0; s < *nb_salles; s++) {
        int hauteur, largeur, id;
        fscanf(f, "[%d|%d] %d\n", &hauteur, &largeur, &id);

        Room *salle = malloc(sizeof(Room));
        salle->hauteur = hauteur;
        salle->largeur = largeur;
        salle->id = id;

        salle->grille = malloc(hauteur * sizeof(char *));
        for (int i = 0; i < hauteur; i++) {
            salle->grille[i] = malloc(largeur * sizeof(char));
            for (int j = 0; j < largeur; j++) {
                char c;
                fscanf(f, " %c", &c);
                salle->grille[i][j] = c;
            }
        }
        salles[s] = salle;
    }

    fclose(f);
    printf("%d salle(s) chargee(s) depuis %s\n", *nb_salles, NOM_FICHIER);
    return salles;
}