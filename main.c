#include <stdio.h>
#include <stdlib.h>
#include "salle.h"

#define MAX_SALLES 100

int main() {
    Room *salles[MAX_SALLES];
    int nb_salles = 0;
    int choix;

    do {
        printf("\n=== The Binding of Briatte - CRUD ===\n");
        printf("1. Creer une salle\n"); 
        printf("2. Afficher une salle\n");
        printf("3. Afficher toutes les salles\n");
        printf("4. Modifier une salle\n");
        printf("5. Supprimer une salle\n");
        printf("6. Sauvegarder (jeu.rtbob)\n");
        printf("7. Charger (jeu.rtbob)\n");
        printf("8. Quitter\n");
        printf("Choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1: {
                if (nb_salles >= MAX_SALLES) {
                    printf("Nombre maximum de salles atteint.\n");
                    break;
                }
                int hauteur, largeur;
                do {
                    printf("Hauteur (impair >= 9) : ");
                    scanf("%d", &hauteur);
                } while (hauteur < 9 || hauteur % 2 == 0);
                do {
                    printf("Largeur (impair >= 15) : ");
                    scanf("%d", &largeur);
                } while (largeur < 15 || largeur % 2 == 0);

                int id = nb_salles + 1;
                salles[nb_salles] = creer_salle(hauteur, largeur, id);
                nb_salles++;
                printf("Salle %d creee.\n", id);
                break;
            }
            case 2: {
                if (nb_salles == 0) { printf("Aucune salle.\n"); break; }
                int id;
                printf("ID de la salle : ");
                scanf("%d", &id);
                int trouve = 0;
                for (int i = 0; i < nb_salles; i++) {
                    if (salles[i]->id == id) {
                        afficher_salle(salles[i]);
                        trouve = 1;
                        break;
                    }
                }
                if (!trouve) printf("Salle %d introuvable.\n", id);
                break;
            }
            case 3: {
                if (nb_salles == 0) { printf("Aucune salle.\n"); break; }
                for (int i = 0; i < nb_salles; i++) {
                    afficher_salle(salles[i]);
                }
                break;
            }
            case 4: {
                if (nb_salles == 0) { printf("Aucune salle.\n"); break; }
                int id;
                printf("ID de la salle a modifier : ");
                scanf("%d", &id);
                int trouve = 0;
                for (int i = 0; i < nb_salles; i++) {
                    if (salles[i]->id == id) {
                        modifier_salle(salles[i]);
                        trouve = 1;
                        break;
                    }
                }
                if (!trouve) printf("Salle %d introuvable.\n", id);
                break;
            }
            case 5: {
                if (nb_salles == 0) { printf("Aucune salle.\n"); break; }
                int id;
                printf("ID de la salle a supprimer : ");
                scanf("%d", &id);
                supprimer_salle(salles, &nb_salles, id);
                break;
            }
            case 6: {
                if (nb_salles == 0) { printf("Aucune salle.\n"); break; }
                sauvegarder_salles(salles, nb_salles);
                break;
            }
            case 7: {
                int nb_charges = 0;
                Room **charges = charger_salles(&nb_charges);
                if (charges) {
                    for (int i = 0; i < nb_charges; i++) {
                        if (nb_salles < MAX_SALLES) {
                            salles[nb_salles++] = charges[i];
                        }
                    }
                    free(charges);
                }
                break;
            }
            case 8:
                printf("Au revoir !\n");
                break;
            default:
                printf("Choix invalide.\n");
        }
    } while (choix != 8);

    for (int i = 0; i < nb_salles; i++) {
        liberer_salle(salles[i]);
    }

    return 0;
}