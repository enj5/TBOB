#include "play_mode.h"
#include "crud1_salle.h"
#include "salle.h"
#include "rooms.h"
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

typedef struct { int x, y; } Coord;

typedef struct {
    bool active;
    int etage;
    int hp;
    int x;
    int y;
    bool can_move;
    bool can_shoot;
    int shoot_type; // 1 = directional, 2 = cross
    int shoot_timer;
    int shoot_interval;
} Boss;

typedef struct {
    bool active;
    int hp;
} MonsterCell;

static Boss boss;
static MonsterCell monster_grid[14][20][20];

static int signe_entier(int value);

static bool is_monster_char(char c)
{
    return c == 'M' || c == 'N' || c == 'C' || c == 'c' || c == 'm';
}

static int hp_for_monster_char(char c)
{
    switch (c) {
        case 'M': return 20;
        case 'N': return 30;
        case 'C': return 25;
        case 'c': return 15;
        case 'm': return 15;
        default: return 20;
    }
}

static void clear_monster_grid(void)
{
    for (int r = 0; r < 14; ++r) {
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                monster_grid[r][y][x].active = false;
                monster_grid[r][y][x].hp = 0;
            }
        }
    }
}

static void initialize_monster_grid(Room *rooms, size_t room_count)
{
    clear_monster_grid();
    for (size_t r = 0; r < room_count && r < 14; ++r) {
        for (int y = 0; y < rooms[r].height; ++y) {
            for (int x = 0; x < rooms[r].width; ++x) {
                char c = rooms[r].grid[y][x];
                if (is_monster_char(c)) {
                    monster_grid[r][y][x].active = true;
                    monster_grid[r][y][x].hp = hp_for_monster_char(c);
                }
            }
        }
    }
}

static void move_monster_cell(int room_index, int old_x, int old_y, int new_x, int new_y)
{
    monster_grid[room_index][new_y][new_x] = monster_grid[room_index][old_y][old_x];
    monster_grid[room_index][old_y][old_x].active = false;
    monster_grid[room_index][old_y][old_x].hp = 0;
}

static void defeat_monster_at(int room_index, int x, int y)
{
    monster_grid[room_index][y][x].active = false;
    monster_grid[room_index][y][x].hp = 0;
}

static int count_monsters_in_room(const Room *room, int room_index)
{
    int count = 0;
    for (int y = 0; y < room->height; ++y) {
        for (int x = 0; x < room->width; ++x) {
            if (monster_grid[room_index][y][x].active) {
                count++;
            }
        }
    }
    return count;
}

#define MAX_PROJECTILES 10

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
            else if (layout[y][x] == 12)
            {
                printf("%c ", (unsigned char)178);
            }
            else if (layout[y][x] == 11 || layout[y][x] == 13)
            {
                printf("%c ", (unsigned char)170);
            }
            else
            {
                printf(" * ");
            }
        }
        printf("\n");
    }
}

static void clear_previous_boss_marker(Room *room)
{
    for (int y = 0; y < room->height; ++y)
    {
        for (int x = 0; x < room->width; ++x)
        {
            if (room->grid[y][x] == 'B')
            {
                room->grid[y][x] = ' ';
            }
        }
    }
}

static void initialiser_boss(Room *room, int floor_index)
{
    boss.active = true;
    boss.etage = floor_index + 1;
    boss.shoot_timer = 0;
    boss.shoot_interval = 15;
    boss.can_move = false;
    boss.can_shoot = true;

    clear_previous_boss_marker(room);

    int midX = room->width / 2;
    int midY = room->height / 2;

    if (floor_index == 0)
    {
        boss.hp = 100;
        boss.can_move = true;
        boss.shoot_type = 1;
        boss.x = midX;
        boss.y = midY;
    }
    else if (floor_index == 1)
    {
        boss.hp = 300;
        boss.can_move = false;
        boss.shoot_type = 1;
        boss.x = midX;
        boss.y = 1;
    }
    else
    {
        boss.hp = 450;
        boss.can_move = false;
        boss.shoot_type = 2;
        boss.x = midX;
        boss.y = midY;
    }

    room->grid[boss.y][boss.x] = 'B';
}

static bool boss_hit(Room *room, int x, int y, int damage)
{
    if (!boss.active)
        return false;
    if (boss.x != x || boss.y != y)
        return false;
    if (room->grid[y][x] != 'B')
        return false;

    boss.hp -= damage;
    printf("Projectile a touche le boss de l'etage %d (%d HP restants).\n", boss.etage, boss.hp > 0 ? boss.hp : 0);
    if (boss.hp <= 0)
    {
        boss.active = false;
        room->grid[y][x] = 'I';
        printf("Le boss est mort et laisse tomber un I.\n");
    }
    return true;
}

static bool add_projectile(Projectile projectiles[], int *nombre_projectiles, int x, int y, int dx, int dy)
{
    if (*nombre_projectiles >= MAX_PROJECTILES)
        return false;

    projectiles[*nombre_projectiles].x = x;
    projectiles[*nombre_projectiles].y = y;
    projectiles[*nombre_projectiles].dx = dx;
    projectiles[*nombre_projectiles].dy = dy;
    projectiles[*nombre_projectiles].active = true;
    (*nombre_projectiles)++;
    return true;
}

static void boss_shoot(int joueur_x, int joueur_y, Projectile projectiles[], int *nombre_projectiles)
{
    if (!boss.active || !boss.can_shoot)
        return;

    boss.shoot_timer++;
    if (boss.shoot_timer < boss.shoot_interval)
        return;

    boss.shoot_timer = 0;

    if (boss.shoot_type == 1)
    {
        int dx = signe_entier(joueur_x - boss.x);
        int dy = signe_entier(joueur_y - boss.y);

        if (dx == 0 && dy == 0)
            return;

        add_projectile(projectiles, nombre_projectiles, boss.x, boss.y, dx, dy);
        printf("Le boss tire un projectile vers le joueur.\n");
    }
    else if (boss.shoot_type == 2)
    {
        add_projectile(projectiles, nombre_projectiles, boss.x, boss.y, 1, 0);
        add_projectile(projectiles, nombre_projectiles, boss.x, boss.y, -1, 0);
        add_projectile(projectiles, nombre_projectiles, boss.x, boss.y, 0, 1);
        add_projectile(projectiles, nombre_projectiles, boss.x, boss.y, 0, -1);
        printf("Athina tire 4 projectiles en croix.\n");
    }
}

static void boss_move(int joueur_x, int joueur_y, Room *room)
{
    if (!boss.active || !boss.can_move)
        return;

    int dx = signe_entier(joueur_x - boss.x);
    int dy = signe_entier(joueur_y - boss.y);
    int nx = boss.x;
    int ny = boss.y;
    bool moved = false;

    int dist_x = abs(joueur_x - boss.x);
    int dist_y = abs(joueur_y - boss.y);

    if (dist_x >= dist_y && dx != 0)
    {
        if (boss.x + dx >= 0 && boss.x + dx < room->width && boss.y >= 0 && boss.y < room->height && room->grid[boss.y][boss.x + dx] == ' ')
        {
            nx = boss.x + dx;
            moved = true;
        }
    }
    if (!moved && dy != 0)
    {
        if (boss.y + dy >= 0 && boss.y + dy < room->height && boss.x >= 0 && boss.x < room->width && room->grid[boss.y + dy][boss.x] == ' ')
        {
            ny = boss.y + dy;
            moved = true;
        }
    }
    if (!moved && dist_x < dist_y && dx != 0)
    {
        if (boss.x + dx >= 0 && boss.x + dx < room->width && room->grid[boss.y][boss.x + dx] == ' ')
        {
            nx = boss.x + dx;
            moved = true;
        }
    }

    if (moved)
    {
        room->grid[boss.y][boss.x] = ' ';
        boss.x = nx;
        boss.y = ny;
        room->grid[boss.y][boss.x] = 'B';
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

static bool mettre_a_jour_projectiles(Room *room, int room_index, int largeur, int hauteur, Projectile projectiles[], int *nombre_projectiles)
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
                printf("Projectile a frappe un rocher en (%d,%d).\n", nx, ny);
            }
            else if (cible == 'W' || cible == 'D' || cible == 'G' || cible == 'S' || cible == 'H' || cible == 'I')
            {
                printf("Projectile a frappe un obstacle '%c' en (%d,%d).\n", cible, nx, ny);
            }
            else if (is_monster_char(cible))
            {
                MonsterCell *cell = &monster_grid[room_index][ny][nx];
                if (cell->active)
                {
                    cell->hp -= 10;
                    if (cell->hp <= 0)
                    {
                        printf("Projectile a tue un monstre en (%d,%d).\n", nx, ny);
                        room->grid[ny][nx] = ' ';
                        defeat_monster_at(room_index, nx, ny);
                    }
                    else
                    {
                        printf("Projectile a blesse un monstre en (%d,%d) (%d HP restants).\n", nx, ny, cell->hp);
                    }
                }
                else
                {
                    printf("Projectile a touche un monstre inconnu en (%d,%d).\n", nx, ny);
                    room->grid[ny][nx] = ' ';
                }
            }
            else if (cible == 'B')
            {
                if (!boss_hit(room, nx, ny, 10))
                {
                    printf("Projectile a touche un boss inconnu en (%d,%d).\n", nx, ny);
                }
            }
            else
            {
                printf("Projectile a frappe '%c' en (%d,%d).\n", cible, nx, ny);
            }
            projectiles[p].active = false; // impact et fin du projectile
            continue;
        }
        projectiles[p].x = nx;
        projectiles[p].y = ny;
        any_moved = true;
    }
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

static bool initialiser_plan_et_portes(Room *rooms, size_t room_count,
                                      int layout[6][5],
                                      Coord room_position[14],
                                      int adjacency[14][4],
                                      bool visitee[14])
{
    if (room_count != 14 || !rooms) {
        return false;
    }

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

    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 5; ++x)
            layout[y][x] = -1;

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

    for (int i = 0; i < 14; ++i) {
        visitee[i] = false;
    }
    visitee[0] = true;

    return true;
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
            if (is_monster_char(room->grid[ny][nx]))
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

static bool mettre_a_jour_monstres(Room *room, int room_index, int largeur, int hauteur, int joueur_x, int joueur_y, bool bouger)
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
            if (!is_monster_char(original[y][x]))
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
            next_grid[ny][nx] = original[y][x];
            move_monster_cell(room_index, x, y, nx, ny);
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
    Room *rooms = NULL;
    size_t room_count = 0;
    int current_floor = 0;
    const int max_floors = 3;

    if (!generate_floor(current_floor, NULL, 0, "monstres.mtbob", "items.itbob", &rooms, &room_count)) {
        fprintf(stderr, "Erreur : impossible de generer le sol %d\n", current_floor);
        return 1;
    }
    initialize_monster_grid(rooms, room_count);
    if (room_count != 14) {
        fprintf(stderr, "Erreur : nombre de salles generees incorrect (%zu)\n", room_count);
        if (rooms) liberer_salles(rooms, room_count);
        return 1;
    }

    bool visitee[14] = {false};


    int layout[6][5];
    Coord room_position[14];
    int adjacency[14][4];

    if (!initialiser_plan_et_portes(rooms, room_count,
                                    layout, room_position, adjacency, visitee)) {
        fprintf(stderr, "Erreur : impossible de configurer le plan du sol %d\n", current_floor);
        liberer_salles(rooms, room_count);
        return 1;
    }

    initialiser_boss(&rooms[12], current_floor);

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
        bool boss_item_ramasse = false;
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
                                    int deja = count_monsters_in_room(&rooms[salle_actuelle], salle_actuelle);
                                    int max_add = 6 - deja;
                                    if (max_add > 0)
                                    {
                                        int nombre_monstres = rand() % max_add + 1;
                                        int nombre_types = rand() % 2 + 1;
                                        char type_markers[2] = { 'M', 'N' };

                                        for (int m = 0; m < nombre_monstres; ++m)
                                        {
                                            int tentatives = 0;
                                            bool place = false;
                                            while (tentatives < 100 && !place)
                                            {
                                                int x = rand() % largeur;
                                                int y = rand() % hauteur;
                                                if (rooms[salle_actuelle].grid[y][x] == ' ')
                                                {
                                                    char marker = type_markers[rand() % nombre_types];
                                                    rooms[salle_actuelle].grid[y][x] = marker;
                                                    monster_grid[salle_actuelle][y][x].active = true;
                                                    monster_grid[salle_actuelle][y][x].hp = hp_for_monster_char(marker);
                                                    place = true;
                                                }
                                                tentatives++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (cible == 'I' && salle_actuelle == 12 && !boss.active)
                        {
                            boss_item_ramasse = true;
                        }
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
        bool projectiles_deplaces = mettre_a_jour_projectiles(&rooms[salle_actuelle], salle_actuelle, largeur, hauteur, projectiles, &nombre_projectiles);

        bool attaque = appliquer_attaques_monstres(&rooms[salle_actuelle], largeur, hauteur, joueur_x, joueur_y, &pv_joueur, &delai_attaque_monstre);
        bool bouger = (tick_monstre % 5) == 0;
        bool monstres_deplaces = mettre_a_jour_monstres(&rooms[salle_actuelle], salle_actuelle, largeur, hauteur, joueur_x, joueur_y, bouger);
        tick_monstre++;

        if (salle_actuelle == 12)
        {
            boss_move(joueur_x, joueur_y, &rooms[salle_actuelle]);
            boss_shoot(joueur_x, joueur_y, projectiles, &nombre_projectiles);
        }

        if (boss_item_ramasse)
        {
            if (current_floor + 1 < max_floors)
            {
                printf("\nBoss tue et I collecte ! Chargement de l'etage %d...\n", current_floor + 2);
                liberer_salles(rooms, room_count);
                rooms = NULL;
                room_count = 0;
                current_floor++;

                if (!generate_floor(current_floor, NULL, 0, "monstres.mtbob", "items.itbob", &rooms, &room_count) || room_count != 14)
                {
                    fprintf(stderr, "Erreur : impossible de generer le sol %d\n", current_floor);
                    en_jeu = false;
                    break;
                }
                initialize_monster_grid(rooms, room_count);

                if (!initialiser_plan_et_portes(rooms, room_count,
                                                layout, room_position, adjacency, visitee))
                {
                    fprintf(stderr, "Erreur : impossible de configurer le plan du sol %d\n", current_floor);
                    liberer_salles(rooms, room_count);
                    en_jeu = false;
                    break;
                }

                initialiser_boss(&rooms[12], current_floor);

                salle_actuelle = 0;
                joueur_x = largeur / 2;
                joueur_y = hauteur / 2;
                delai_attaque_monstre = 0;
                tick_monstre = 0;
                nombre_projectiles = 0;
                besoin_rendu = true;
                continue;
            }
            else
            {
                printf("\nFelicitation ! Vous avez termine tous les etages.\n");
                en_jeu = false;
            }
        }

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
            printf("Etage : %d / %d\n", current_floor + 1, max_floors);
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

    liberer_salles(rooms, room_count);

    printf("Fin du jeu.\n");
    return 0;
}
