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

static int load_monsters(const char *filename, Entity monsters[], int max_monsters) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    int count;
    fscanf(f, "{%d}\n", &count);
    for (int i = 0; i < count && i < max_monsters; ++i) {
        fscanf(f, "---\n");
        fscanf(f, "name=%[^\n]\n", monsters[i].name);
        fscanf(f, "hpMax=%f\n", &monsters[i].hpMax);
        monsters[i].shoot = 0;
        monsters[i].ss = 0;
        monsters[i].flight = 0;
        monsters[i].dmg = 10.0f; // valeur par défaut
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "---")) break;
            if (strstr(line, "shoot=1")) monsters[i].shoot = 1;
            else if (strstr(line, "ss=1")) monsters[i].ss = 1;
            else if (strstr(line, "flight=1")) monsters[i].flight = 1;
        }
    }
    fclose(f);
    return count;
}

static void afficher_minimap(int layout[6][5], int current_room)
{
    printf("\nMinicarte (grille 5x6) :\n\n");
    for (int y = 0; y < 6; ++y) {
        printf("\n");
        for (int x = 0; x < 5; ++x) {
            if (layout[y][x] == -1) {
                printf(" . ");
            } else if (layout[y][x] == current_room) {
                printf(" P ");
            } else {
                printf(" * ");
            }
        }
        printf("\n");
    }
}

static void render_room(const Room *room, int width, int height, Projectile projectiles[], int num_projectiles)
{
    // Affiche la salle et superpose les projectiles actifs.
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            char c = room->grid[y][x];
            bool has_projectile = false;
            for (int p = 0; p < num_projectiles; ++p) {
                if (projectiles[p].active && projectiles[p].x == x && projectiles[p].y == y) {
                    has_projectile = true;
                    break;
                }
            }
            if (has_projectile) {
                printf("* "); // projectile visible sur la carte
            } else {
                printf("%c ", c);
            }
        }
        printf("\n");
    }
}

static void update_projectiles(Room *room, int width, int height, Projectile projectiles[], int *num_projectiles)
{
    for (int p = 0; p < *num_projectiles; ++p) {
        if (!projectiles[p].active) continue;
        int nx = projectiles[p].x + projectiles[p].dx;
        int ny = projectiles[p].y + projectiles[p].dy;
        if (nx < 0 || nx >= width || ny < 0 || ny >= height) {
            projectiles[p].active = false; // sort de la carte
            continue;
        }
        char cible = room->grid[ny][nx];
        if (cible != ' ') {
            if (cible == 'R') {
                room->grid[ny][nx] = ' ';
                printf("Projectile a detruit un rocher en (%d,%d).\n", nx, ny);
            } else if (cible == 'W' || cible == 'D') {
                printf("Projectile a frappe un obstacle '%c' en (%d,%d).\n", cible, nx, ny);
            } else {
                printf("Projectile a touche '%c' en (%d,%d).\n", cible, nx, ny);
                room->grid[ny][nx] = ' ';
            }
            projectiles[p].active = false; // impact et fin du projectile
            continue;
        }
        projectiles[p].x = nx;
        projectiles[p].y = ny;
    }
    // Compresse le tableau de projectiles en supprimant les inactifs
    int active_count = 0;
    for (int p = 0; p < *num_projectiles; ++p) {
        if (projectiles[p].active) {
            projectiles[active_count++] = projectiles[p];
        }
    }
    *num_projectiles = active_count;
}

static int sign_int(int value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

static bool can_monster_enter(char tile)
{
    return tile == ' ';
}

static bool update_monsters(Room *room, int width, int height, int player_x, int player_y)
{
    char original[20][20];
    char next_grid[20][20];
    bool any_moved = false;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            original[y][x] = room->grid[y][x];
            next_grid[y][x] = room->grid[y][x];
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (original[y][x] != 'M')
                continue;

            int dx = sign_int(player_x - x);
            int dy = sign_int(player_y - y);
            int nx = x;
            int ny = y;
            bool moved = false;

            int dist_x = abs(player_x - x);
            int dist_y = abs(player_y - y);

            if (dist_x >= dist_y && dx != 0) {
                int tx = x + dx;
                if (tx >= 0 && tx < width && can_monster_enter(original[y][tx]) && next_grid[y][tx] == ' ') {
                    nx = tx;
                    moved = true;
                }
            }

            if (!moved && dy != 0) {
                int ty = y + dy;
                if (ty >= 0 && ty < height && can_monster_enter(original[ty][x]) && next_grid[ty][x] == ' ') {
                    ny = ty;
                    moved = true;
                }
            }

            if (!moved && dist_x < dist_y && dx != 0) {
                int tx = x + dx;
                if (tx >= 0 && tx < width && can_monster_enter(original[y][tx]) && next_grid[y][tx] == ' ') {
                    nx = tx;
                    moved = true;
                }
            }

            if (!moved || (nx == x && ny == y))
                continue;

            if (next_grid[ny][nx] == 'P') {
                continue;
            }

            next_grid[y][x] = ' ';
            next_grid[ny][nx] = 'M';
            any_moved = true;
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            room->grid[y][x] = next_grid[y][x];
        }
    }

    return any_moved;
}

int play_mode(void)
{
    printf("=== TBOB Playable Prototype ===\n\n");
    printf("Contrôles : z/s/q/d ou w/s/a/d, x pour quitter.\n\n");

    int height = 9, width = 15;

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


    create_spawner_room_custom(&rooms[0], 0, height, width);


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
        create_normal_room_custom(&rooms[i], i, height, width);
    }

*/

    create_item_room_custom(&rooms[11], 11, height, width, 'I');
    create_boss_room_custom(&rooms[12], 12, height, width);
    create_item_room_custom(&rooms[13], 13, height, width, 'H');
    

    Entity monster_types[10];
    int num_monster_types = load_monsters("monstres.mtbob", monster_types, 10);
    bool visited[14] = {false};
    visited[0] = true; // salle spawner déjà visitée

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
        configure_room_doors(&rooms[i],
                             adjacency[i][0] != -1,
                             adjacency[i][1] != -1,
                             adjacency[i][2] != -1,
                             adjacency[i][3] != -1);
    }

    int current_room = 0;
    int player_x = width / 2;
    int player_y = height / 2;

    Projectile projectiles[MAX_PROJECTILES];
    int num_projectiles = 0;

    bool playing = true;
    bool needs_render = true; // contrôle du rendu pour éviter le scintillement
    while (playing) {
        bool has_input = false;
        // Gestion de l'entrée sans bloquer l'exécution
        int key = -1;
        if (kbhit()) {
            key = getch();
            if (key == 0 || key == 224) {
                key = getch(); // touche spéciale, par exemple flèche
            }
            has_input = true;
        }

        if (key == 'x' || key == 'X') {
            playing = false; // quitter le mode de jeu
            continue;
        }

        int dx = 0, dy = 0;
        bool shooting = false;

        if (key == 72) { shooting = true; dy = -1; } // up arrow
        else if (key == 80) { shooting = true; dy = 1; } // down
        else if (key == 75) { shooting = true; dx = -1; } // left
        else if (key == 77) { shooting = true; dx = 1; } // right
        else if (key == 'z' || key == 'w') dy = -1;
        else if (key == 's') dy = 1;
        else if (key == 'q' || key == 'a') dx = -1;
        else if (key == 'd') dx = 1;

        bool moved = false;
        if (shooting && num_projectiles < MAX_PROJECTILES) {
            // Tir non bloquant : on ajoute le projectile à la liste et on continue
            projectiles[num_projectiles].x = player_x + dx;
            projectiles[num_projectiles].y = player_y + dy;
            projectiles[num_projectiles].dx = dx;
            projectiles[num_projectiles].dy = dy;
            projectiles[num_projectiles].active = true;
            num_projectiles++;
            needs_render = true;
        } else if (!shooting && (dx != 0 || dy != 0)) {
            // Déplacement du joueur
            int nx = player_x + dx;
            int ny = player_y + dy;
            if (nx >= 0 && ny >= 0 && nx < width && ny < height) {
                char cible = rooms[current_room].grid[ny][nx];
                if (cible != 'W' && cible != 'R') {
                    if (cible == 'D') {
                        int dir = -1;
                        int midW = width / 2;
                        int midH = height / 2;

                        if (ny == 0 && nx == midW) dir = 0;
                        else if (nx == width - 1 && ny == midH) dir = 1;
                        else if (ny == height - 1 && nx == midW) dir = 2;
                        else if (nx == 0 && ny == midH) dir = 3;

                        if (dir != -1) {
                            int next_room = adjacency[current_room][dir];
                            if (next_room != -1) {
                                rooms[current_room].grid[player_y][player_x] = 'D';
                                current_room = next_room;
                                if (dir == 0) { player_y = height - 2; player_x = midW; }
                                else if (dir == 1) { player_y = midH; player_x = 1; }
                                else if (dir == 2) { player_y = 1; player_x = midW; }
                                else if (dir == 3) { player_y = midH; player_x = width - 2; }
                                rooms[current_room].grid[player_y][player_x] = 'P';
                                moved = true;
                                // Spawn de monstres si première visite d'une salle normale
                                if (!visited[current_room] && current_room >= 1 && current_room <= 10) {
                                    
                                    visited[current_room] = true;
                                    int num_monsters = rand() % 6 + 1;
                                    int num_types = rand() % 2 + 1;
                                    int selected_types[2];
                                    for (int t = 0; t < num_types; ++t) {
                                        selected_types[t] = rand() % num_monster_types;
                                    }
                                    for (int m = 0; m < num_monsters; ++m) {
                                        int type = selected_types[rand() % num_types];
                                        // Trouver une position aléatoire valide
                                        int attempts = 0;
                                        bool placed = false;
                                        while (attempts < 100 && !placed) {
                                            int x = rand() % width;
                                            int y = rand() % height;
                                            if (rooms[current_room].grid[y][x] == ' ') {
                                                rooms[current_room].grid[y][x] = 'M';
                                                placed = true;
                                            }
                                            attempts++;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        rooms[current_room].grid[player_y][player_x] = ' ';
                        player_x = nx;
                        player_y = ny;
                        rooms[current_room].grid[player_y][player_x] = 'P';
                        moved = true;
                    }
                }
            }
        }

        // Met à jour la position de tous les projectiles actifs
        int prev_num = num_projectiles;
        update_projectiles(&rooms[current_room], width, height, projectiles, &num_projectiles);

        // Déplace les monstres vers le joueur dans la salle actuelle
        bool monsters_moved = update_monsters(&rooms[current_room], width, height, player_x, player_y);

        if (num_projectiles != prev_num || moved || monsters_moved || has_input) {
            needs_render = true; // redessine seulement lorsqu'il y a un changement
        }

        // Rendu conditionnel pour éviter le scintillement constant
        if (needs_render) {
            system("cls");
            printf("Salle actuelle : %d / 13\n", current_room);
            render_room(&rooms[current_room], width, height, projectiles, num_projectiles);
            afficher_minimap(layout, current_room);
            printf("\nMouvement : z/w/up, s/down, q/a/gauche, d/droite\n");
            printf("Tir : fleches directionnelles, x pour quitter\n");
            needs_render = false;
        }

        Sleep(100); // ~10 FPS
    }

    for (int i = 0; i < 14; ++i) freeR(&rooms[i]);

    printf("Fin du jeu.\n");
    return 0;
}
