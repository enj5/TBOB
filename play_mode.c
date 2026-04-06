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

static void render_room(const Room *room, int width, int height)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            printf("%c ", room->grid[y][x]);
        }
        printf("\n");
    }
}

static void fire_projectile(Room *room, int width, int height, int start_x, int start_y, int dx, int dy)
{
    int x = start_x + dx;
    int y = start_y + dy;
    const char *direction = (dx == 1 ? "droite" : dx == -1 ? "gauche" : dy == -1 ? "haut" : "bas");

    while (x >= 0 && x < width && y >= 0 && y < height) {
        char original = room->grid[y][x];
        if (original != ' ') {
            if (original == 'R') {
                room->grid[y][x] = ' ';
                printf("Projectile a detruit un rocher en (%d,%d).\n", x, y);
            } else if (original == 'W' || original == 'D') {
                printf("Projectile a frappe un obstacle '%c' en (%d,%d).\n", original, x, y);
            } else {
                printf("Projectile a touche '%c' en (%d,%d).\n", original, x, y);
                room->grid[y][x] = ' ';
            }
            return;
        }

        room->grid[y][x] = '*';
        system("cls");
        render_room(room, width, height);
        Sleep(120);
        room->grid[y][x] = original;

        x += dx;
        y += dy;
    }

    printf("Projectile tire vers %s sans rien toucher.\n", direction);
}

int play_mode(void)
{
    printf("=== TBOB Playable Prototype ===\n\n");
    printf("Contrôles : z/s/q/d ou w/s/a/d, x pour quitter.\n\n");

    int height = 0, width = 0;
    do {
        printf("Entrez la hauteur impaire des salles (9-19) : ");
        if (scanf("%d", &height) != 1) { height = 0; continue; }
    } while (height < 9 || height > 19 || (height % 2 == 0));

    do {
        printf("Entrez la largeur impaire des salles (9-19) : ");
        if (scanf("%d", &width) != 1) { width = 0; continue; }
    } while (width < 9 || width > 19 || (width % 2 == 0));

    Room rooms[14];
    int nS;
    int i = 0;


    create_spawner_room_custom(&rooms[0], 0, height, width);

/*
    FILE* fich = fopen("salles.rtbob", "r"); // Ouverture du fichier en lecture 
            //Chargement du contenu dans le tableau des salles
        if(fich != NULL){
           
            fscanf(fich, "{%d}\n", &nS); //Lecture nbre de salles
            for(i = 1; i < nS+1; i++){
                ReadInFile(&rooms[i], fich); //Lecture de chaque salle
            }


            fclose(fich);
        }
*/

    for (int i = 1; i <= 10; ++i) {
        create_normal_room_custom(&rooms[i], i, height, width);
    }
    create_item_room_custom(&rooms[11], 11, height, width, 'I');
    create_boss_room_custom(&rooms[12], 12, height, width);
    create_item_room_custom(&rooms[13], 13, height, width, 'H');
    

    typedef struct { int x, y; } Coord;
    Coord pool[29];
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

    bool playing = true;
    while (playing) {
        system("cls");
        printf("Salle actuelle : %d / 13\n", current_room);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                printf("%c ", rooms[current_room].grid[y][x]);
            }
            printf("\n");
        }

        afficher_minimap(layout, current_room);

        printf("\nMouvement : z/w/up, s/down, q/a/gauche, d/droite\n");
        printf("Tir : fleches directionnelles (haut/bas/gauche/droite), x pour quitter\n");
        int key = getch();
        if (key == 'x' || key == 'X') break;

        int dx = 0, dy = 0;
        bool shooting = false;

        if (key == 0 || key == 224) {
            int arrow = getch();
            shooting = true;
            if (arrow == 72) dy = -1;
            else if (arrow == 80) dy = 1;
            else if (arrow == 75) dx = -1;
            else if (arrow == 77) dx = 1;
            else shooting = false;
        } else if (key == 'z' || key == 'w') {
            dy = -1;
        } else if (key == 's') {
            dy = 1;
        } else if (key == 'q' || key == 'a') {
            dx = -1;
        } else if (key == 'd') {
            dx = 1;
        } else {
            continue;
        }

        if (shooting) {
            if (dx == 0 && dy == 0) continue;
            fire_projectile(&rooms[current_room], width, height, player_x, player_y, dx, dy);
            continue;
        }

        int nx = player_x + dx;
        int ny = player_y + dy;
        if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;

        char cible = rooms[current_room].grid[ny][nx];
        if (cible == 'W' || cible == 'R') continue;

        if (cible == 'D') {
            int dir = -1;
            int midW = width / 2;
            int midH = height / 2;

            if (ny == 0 && nx == midW) dir = 0;
            else if (nx == width - 1 && ny == midH) dir = 1;
            else if (ny == height - 1 && nx == midW) dir = 2;
            else if (nx == 0 && ny == midH) dir = 3;

            if (dir == -1) continue;

            int next_room = adjacency[current_room][dir];
            if (next_room == -1) continue;

            rooms[current_room].grid[player_y][player_x] = 'D';
            current_room = next_room;
            if (dir == 0) { player_y = height - 2; player_x = midW; }
            else if (dir == 1) { player_y = midH;       player_x = 1; }
            else if (dir == 2) { player_y = 1;          player_x = midW; }
            else if (dir == 3) { player_y = midH;       player_x = width - 2; }

            rooms[current_room].grid[player_y][player_x] = 'P';
            continue;
        }

        rooms[current_room].grid[player_y][player_x] = ' ';
        player_x = nx;
        player_y = ny;
        rooms[current_room].grid[player_y][player_x] = 'P';
    }

    for (int i = 0; i < 14; ++i) freeR(&rooms[i]);

    printf("Fin du jeu.\n");
    return 0;
}
