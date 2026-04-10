#include "play_mode.h"
#include "crud1_salle.h"
#include "salle.h"
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

// Projectile en vol : position et direction
typedef struct {
    int x, y;      // coordonnées actuelles du projectile
    int dx, dy;    // direction du mouvement (par exemple, (1,0) droite)
    bool active;   // actif si il continue de voler
} Projectile;

#define MAX_PROJECTILES 10

static int charger_monstres(const char *filename, Entity monsters[], int max_monsters)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        return 0;
    }
    int count;
    fscanf(f, "{%d}\n", &count);
    for (int i = 0; i < count && i < max_monsters; ++i)
    {
        fscanf(f, "---\n");
        fscanf(f, "name=%[^\n]\n", monsters[i].name);
        fscanf(f, "hpMax=%f\n", &monsters[i].hpMax);
        monsters[i].shoot = 0;
        monsters[i].ss = 0;
        monsters[i].flight = 0;
        monsters[i].dmg = 10.0f; // valeur par défaut
        char line[256];
        while (fgets(line, sizeof(line), f))
        {
            if (strstr(line, "---"))
            {
                break;
            }
            if (strstr(line, "shoot=1"))
            {
                monsters[i].shoot = 1;
            }
            else if (strstr(line, "ss=1"))
            {
                monsters[i].ss = 1;
            }
            else if (strstr(line, "flight=1"))
            {
                monsters[i].flight = 1;
            }
        }
    }
    fclose(f);
    return count;
}

static void afficher_minimap(int layout[6][5], int salle_actuelle)
{
    printf("\nMinicarte (grille 5x6) :\n\n");
    for (int y = 0; y < 6; ++y)
    {
        printf("\n");
        for (int x = 0; x < 5; ++x)
        {
            if (layout[y][x] == -1)
            {
                printf(" . ");
            }
            else if (layout[y][x] == salle_actuelle)
            {
                printf(" P ");
            }
            else
            {
                printf(" * ");
            }
        }
        printf("\n");
    }
}

static void rendre_salle(const Room *room, int largeur, int hauteur, Projectile projectiles[], int nombre_projectiles)
{
    // Affiche la salle et superpose les projectiles actifs.
    for (int y = 0; y < hauteur; ++y)
    {
        for (int x = 0; x < largeur; ++x)
        {
            char c = room->grid[y][x];
            bool has_projectile = false;
            for (int p = 0; p < nombre_projectiles; ++p)
            {
                if (projectiles[p].active && projectiles[p].x == x && projectiles[p].y == y)
                {
                    has_projectile = true;
                    break;
                }
            }
            if (has_projectile)
            {
                printf("* "); // projectile visible sur la carte
            }
            else
            {
                printf("%c ", c);
            }
        }
        printf("\n");
    }
}

static bool mettre_a_jour_projectiles(Room *room, int largeur, int hauteur, Projectile projectiles[], int *nombre_projectiles)
{
    bool any_moved = false;
    for (int p = 0; p < *nombre_projectiles; ++p)
    {
        if (!projectiles[p].active)
        {
            continue;
        }
        int nx = projectiles[p].x + projectiles[p].dx;
        int ny = projectiles[p].y + projectiles[p].dy;
        if (nx < 0 || nx >= largeur || ny < 0 || ny >= hauteur)
        {
            projectiles[p].active = false; // sort de la carte
            continue;
        }
        char cible = room->grid[ny][nx];
        if (cible != ' ')
        {
            if (cible == 'R')
            {
                room->grid[ny][nx] = ' ';
                printf("Projectile a detruit un rocher en (%d,%d).\n", nx, ny);
            }
            else if (cible == 'W' || cible == 'D')
            {
                printf("Projectile a frappe un obstacle '%c' en (%d,%d).\n", cible, nx, ny);
            }
            else
            {
                printf("Projectile a touche '%c' en (%d,%d).\n", cible, nx, ny);
                room->grid[ny][nx] = ' ';
            }
            projectiles[p].active = false; // impact et fin du projectile
            continue;
        }
        projectiles[p].x = nx;
        projectiles[p].y = ny;
        any_moved = true;
    }
    // Compresse le tableau de projectiles en supprimant les inactifs
    int active_count = 0;
    for (int p = 0; p < *nombre_projectiles; ++p)
    {
        if (projectiles[p].active)
        {
            projectiles[active_count++] = projectiles[p];
        }
    }
    *nombre_projectiles = active_count;
    return any_moved;
}

static int signe_entier(int value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

static bool peut_monstre_entrer(char tile)
{
    return tile == ' ';
}

static bool appliquer_attaques_monstres(const Room *room, int largeur, int hauteur, int joueur_x, int joueur_y, int *pv_joueur, int *delai_attaque_monstre)
{
    const int damage = 1;
    bool attacked = false;
    const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    (*delai_attaque_monstre)++;

    if (*delai_attaque_monstre >= 10)
    {
        for (int i = 0; i < 4; ++i)
        {
            int nx = joueur_x + dirs[i][0];
            int ny = joueur_y + dirs[i][1];
            if (nx < 0 || nx >= largeur || ny < 0 || ny >= hauteur)
            {
                continue;
            }
            if (room->grid[ny][nx] == 'M')
            {
                *pv_joueur -= damage;
                attacked = true;
                *delai_attaque_monstre = 0;
                break;
            }
        }
    }

    if (*pv_joueur < 0)
    {
        *pv_joueur = 0;
    }

    return attacked;
}

static bool mettre_a_jour_monstres(Room *room, int largeur, int hauteur, int joueur_x, int joueur_y, bool bouger)
{
    if (!bouger)
    {
        return false;
    }

    char original[20][20];
    char next_grid[20][20];
    bool any_moved = false;

    for (int y = 0; y < hauteur; ++y)
    {
        for (int x = 0; x < largeur; ++x)
        {
            original[y][x] = room->grid[y][x];
            next_grid[y][x] = room->grid[y][x];
        }
    }

    for (int y = 0; y < hauteur; ++y)
    {
        for (int x = 0; x < largeur; ++x)
        {
            if (original[y][x] != 'M')
            {
                continue;
            }

            int dx = signe_entier(joueur_x - x);
            int dy = signe_entier(joueur_y - y);
            int nx = x;
            int ny = y;
            bool moved = false;

            int dist_x = abs(joueur_x - x);
            int dist_y = abs(joueur_y - y);

            if (dist_x >= dist_y && dx != 0)
            {
                int tx = x + dx;
                if (tx >= 0 && tx < largeur && peut_monstre_entrer(original[y][tx]) && next_grid[y][tx] == ' ')
                {
                    nx = tx;
                    moved = true;
                }
            }

            if (!moved && dy != 0)
            {
                int ty = y + dy;
                if (ty >= 0 && ty < hauteur && peut_monstre_entrer(original[ty][x]) && next_grid[ty][x] == ' ')
                {
                    ny = ty;
                    moved = true;
                }
            }

            if (!moved && dist_x < dist_y && dx != 0)
            {
                int tx = x + dx;
                if (tx >= 0 && tx < largeur && peut_monstre_entrer(original[y][tx]) && next_grid[y][tx] == ' ')
                {
                    nx = tx;
                    moved = true;
                }
            }

            if (!moved || (nx == x && ny == y))
            {
                continue;
            }

            if (next_grid[ny][nx] == 'P')
            {
                continue;
            }

            next_grid[y][x] = ' ';
            next_grid[ny][nx] = 'M';
            any_moved = true;
        }
    }

    for (int y = 0; y < hauteur; ++y)
    {
        for (int x = 0; x < largeur; ++x)
        {
            room->grid[y][x] = next_grid[y][x];
        }
    }

    return any_moved;
}

int mode_jeu(void)
{
    printf("=== TBOB Playable Prototype ===\n\n");
    printf("Contrôles : z/s/q/d ou w/s/a/d, x pour quitter.\n\n");

    int hauteur = 9, largeur = 15;

    /*
    do {
        printf("Entrez la hauteur impaire des salles (9-19) : ");
        if (scanf("%d", &height) != 1) { height = 0; continue; }
    } while (height < 9 || height > 19 || (height % 2 == 0));

    do {
        printf("Entrez la largeur impaire des salles (9-19) : ");
        if (scanf("%d", &width) != 1) { width = 0; continue; }
    } while (width < 9 || width > 19 || (width % 2 == 0));
    */


    Room rooms[14];
    int nS;
    int i = 0;


    creer_salle_spawn_personnalisee(&rooms[0], 0, hauteur, largeur);


    FILE* fich = fopen("salles.rtbob", "r"); // Ouverture du fichier en lecture 
            //Chargement du contenu dans le tableau des salles
        if(fich != NULL){
           
            fscanf(fich, "{%d}\n", &nS); //Lecture nbre de salles
            for(i = 1; i < nS+1; i++){
                ReadInFile(&rooms[i], fich); //Lecture de chaque salle
            }


            fclose(fich);
        }

/*
    for (int i = 1; i <= 10; ++i) {
        creer_salle_normale_personnalisee(&rooms[i], i, hauteur, largeur);
    }

*/

    creer_salle_objet_personnalisee(&rooms[11], 11, hauteur, largeur, 'I');
    creer_salle_boss_personnalisee(&rooms[12], 12, hauteur, largeur);
    creer_salle_objet_personnalisee(&rooms[13], 13, hauteur, largeur, 'H');
    

    Entity monster_types[10];
    int num_monster_types = charger_monstres("monstres.mtbob", monster_types, 10);
    bool visitee[14] = {false};
    visitee[0] = true; // salle spawner déjà visitée

    typedef struct { int x, y; } Coord;
    Coord pool[29];

    Entity Lenina;
    Entity jogger;

    jogger.dmg = 1.0f;
    jogger.hpMax = 5.0f;

    int idx = 0;
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 5; ++x) {
            if (x == 2 && y == 2) continue;
            pool[idx++] = (Coord){x, y};
        }
    }

    srand((unsigned int)time(NULL));
    for (int i = 28; i > 0; --i) {
        int j = rand() % (i + 1);
        Coord tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }

    int layout[6][5];
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 5; ++x)
            layout[y][x] = -1;

    Coord room_position[14];
    room_position[0] = (Coord){2, 2};
    layout[2][2] = 0;

    bool used[6][5] = {0};
    used[2][2] = true;

    for (int i = 1; i < 14; ++i) {
        Coord candidates[29];
        int cand_count = 0;

        for (int j = 0; j < 29; ++j) {
            int px = pool[j].x;
            int py = pool[j].y;
            if (used[py][px]) continue;

            bool has_neighbor = false;
            if (py > 0 && used[py-1][px]) has_neighbor = true;
            if (px < 4 && used[py][px+1]) has_neighbor = true;
            if (py < 5 && used[py+1][px]) has_neighbor = true;
            if (px > 0 && used[py][px-1]) has_neighbor = true;

            if (has_neighbor) {
                candidates[cand_count++] = pool[j];
            }
        }

        Coord chosen;
        if (cand_count > 0) {
            chosen = candidates[rand() % cand_count];
        } else {
            Coord freepos[29];
            int free_count = 0;
            for (int j = 0; j < 29; ++j) {
                int px = pool[j].x;
                int py = pool[j].y;
                if (!used[py][px]) freepos[free_count++] = pool[j];
            }
            if (free_count > 0)
                chosen = freepos[rand() % free_count];
            else
                chosen = (Coord){0,0};
        }

        room_position[i] = chosen;
        layout[chosen.y][chosen.x] = i;
        used[chosen.y][chosen.x] = true;
    }

    int adjacency[14][4];
    for (int i = 0; i < 14; ++i) {
        int x = room_position[i].x;
        int y = room_position[i].y;
        adjacency[i][0] = (y > 0 && layout[y-1][x] != -1) ? layout[y-1][x] : -1;
        adjacency[i][1] = (x < 4 && layout[y][x+1] != -1) ? layout[y][x+1] : -1;
        adjacency[i][2] = (y < 5 && layout[y+1][x] != -1) ? layout[y+1][x] : -1;
        adjacency[i][3] = (x > 0 && layout[y][x-1] != -1) ? layout[y][x-1] : -1;
        configurer_portes_salle(&rooms[i],
                             adjacency[i][0] != -1,
                             adjacency[i][1] != -1,
                             adjacency[i][2] != -1,
                             adjacency[i][3] != -1);
    }

    int salle_actuelle = 0;
    int joueur_x = largeur / 2;
    int joueur_y = hauteur / 2;
    int pv_joueur = 10;
    int tick_monstre = 0;
    int delai_attaque_monstre = 0;

    Projectile projectiles[MAX_PROJECTILES];
    int nombre_projectiles = 0;

    bool en_jeu = true;
    bool besoin_rendu = true; // contrôle du rendu pour éviter le scintillement
    while (en_jeu)
    {
        bool a_entree = false;
        // Gestion de l'entrée sans bloquer l'exécution
        int touche = -1;
        if (kbhit())
        {
            touche = getch();
            if (touche == 0 || touche == 224)
            {
                touche = getch(); // touche spéciale, par exemple flèche
            }
            a_entree = true;
        }

        if (touche == 'x' || touche == 'X')
        {
            en_jeu = false; // quitter le mode de jeu
            continue;
        }

        int dx = 0, dy = 0;
        bool tir = false;

        if (touche == 72)
        {
            tir = true;
            dy = -1;
        } // up arrow
        else if (touche == 80)
        {
            tir = true;
            dy = 1;
        } // down
        else if (touche == 75)
        {
            tir = true;
            dx = -1;
        } // left
        else if (touche == 77)
        {
            tir = true;
            dx = 1;
        } // right
        else if (touche == 'z' || touche == 'w')
        {
            dy = -1;
        }
        else if (touche == 's')
        {
            dy = 1;
        }
        else if (touche == 'q' || touche == 'a')
        {
            dx = -1;
        }
        else if (touche == 'd')
        {
            dx = 1;
        }

        bool deplace = false;
        if (tir && nombre_projectiles < MAX_PROJECTILES)
        {
            // Tir non bloquant : on ajoute le projectile à la liste et on continue
            projectiles[nombre_projectiles].x = joueur_x;
            projectiles[nombre_projectiles].y = joueur_y;
            projectiles[nombre_projectiles].dx = dx;
            projectiles[nombre_projectiles].dy = dy;
            projectiles[nombre_projectiles].active = true;
            nombre_projectiles++;
            besoin_rendu = true;
        }
        else if (!tir && (dx != 0 || dy != 0))
        {
            // Déplacement du joueur
            int nx = joueur_x + dx;
            int ny = joueur_y + dy;
            if (nx >= 0 && ny >= 0 && nx < largeur && ny < hauteur)
            {
                char cible = rooms[salle_actuelle].grid[ny][nx];
                if (cible != 'W' && cible != 'R')
                {
                    if (cible == 'D')
                    {
                        int direction = -1;
                        int milieu_largeur = largeur / 2;
                        int milieu_hauteur = hauteur / 2;

                        if (ny == 0 && nx == milieu_largeur)
                        {
                            direction = 0;
                        }
                        else if (nx == largeur - 1 && ny == milieu_hauteur)
                        {
                            direction = 1;
                        }
                        else if (ny == hauteur - 1 && nx == milieu_largeur)
                        {
                            direction = 2;
                        }
                        else if (nx == 0 && ny == milieu_hauteur)
                        {
                            direction = 3;
                        }

                        if (direction != -1)
                        {
                            int salle_suivante = adjacency[salle_actuelle][direction];
                            if (salle_suivante != -1)
                            {
                                rooms[salle_actuelle].grid[joueur_y][joueur_x] = 'D';
                                salle_actuelle = salle_suivante;
                                delai_attaque_monstre = 0;
                                if (direction == 0)
                                {
                                    joueur_y = hauteur - 2;
                                    joueur_x = milieu_largeur;
                                }
                                else if (direction == 1)
                                {
                                    joueur_y = milieu_hauteur;
                                    joueur_x = 1;
                                }
                                else if (direction == 2)
                                {
                                    joueur_y = 1;
                                    joueur_x = milieu_largeur;
                                }
                                else if (direction == 3)
                                {
                                    joueur_y = milieu_hauteur;
                                    joueur_x = largeur - 2;
                                }
                                rooms[salle_actuelle].grid[joueur_y][joueur_x] = 'P';
                                deplace = true;
                                // Spawn de monstres si première visite d'une salle normale
                                if (!visitee[salle_actuelle] && salle_actuelle >= 1 && salle_actuelle <= 10)
                                {
                                    
                                    visitee[salle_actuelle] = true;
                                    int nombre_monstres = rand() % 6 + 1;
                                    int nombre_types = rand() % 2 + 1;
                                    int types_selectionnes[2];
                                    for (int t = 0; t < nombre_types; ++t)
                                    {
                                        types_selectionnes[t] = rand() % num_monster_types;
                                    }
                                    for (int m = 0; m < nombre_monstres; ++m)
                                    {
                                        int type = types_selectionnes[rand() % nombre_types];
                                        // Trouver une position aléatoire valide
                                        int tentatives = 0;
                                        bool place = false;
                                        while (tentatives < 100 && !place)
                                        {
                                            int x = rand() % largeur;
                                            int y = rand() % hauteur;
                                            if (rooms[salle_actuelle].grid[y][x] == ' ')
                                            {
                                                rooms[salle_actuelle].grid[y][x] = 'M';
                                                place = true;
                                            }
                                            tentatives++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        rooms[salle_actuelle].grid[joueur_y][joueur_x] = ' ';
                        joueur_x = nx;
                        joueur_y = ny;
                        rooms[salle_actuelle].grid[joueur_y][joueur_x] = 'P';
                        deplace = true;
                    }
                }
            }
        }

        // Met à jour la position de tous les projectiles actifs
        int nombre_precedent = nombre_projectiles;
        bool projectiles_deplaces = mettre_a_jour_projectiles(&rooms[salle_actuelle], largeur, hauteur, projectiles, &nombre_projectiles);

        bool attaque = appliquer_attaques_monstres(&rooms[salle_actuelle], largeur, hauteur, joueur_x, joueur_y, &pv_joueur, &delai_attaque_monstre);
        bool bouger = (tick_monstre % 5) == 0;
        bool monstres_deplaces = mettre_a_jour_monstres(&rooms[salle_actuelle], largeur, hauteur, joueur_x, joueur_y, bouger);
        tick_monstre++;

        if (pv_joueur <= 0)
        {
            en_jeu = false;
            printf("\n¡Has sido derrotado por los monstruos!\n");
        }

        if (nombre_projectiles != nombre_precedent || deplace || monstres_deplaces || attaque || projectiles_deplaces || a_entree)
        {
            besoin_rendu = true; // redessine seulement lorsqu'il y a un changement
        }

        // Rendu conditionnel pour éviter le scintillement constant
        if (besoin_rendu)
        {
            system("cls");
            printf("Salle actuelle : %d / 13\n", salle_actuelle);
            printf("HP joueur : %d\n", pv_joueur);
            rendre_salle(&rooms[salle_actuelle], largeur, hauteur, projectiles, nombre_projectiles);
            afficher_minimap(layout, salle_actuelle);
            printf("\nMouvement : z/w/up, s/down, q/a/gauche, d/droite\n");
            printf("Tir : fleches directionnelles, x pour quitter\n");
            besoin_rendu = false;
        }

        Sleep(100); // ~10 FPS
    }

    for (int i = 0; i < 14; ++i) freeR(&rooms[i]);

    printf("Fin du jeu.\n");
    return 0;
}
