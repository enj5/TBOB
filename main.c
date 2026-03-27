#include "salle.h"
#include "structs.h"
#include "items.h"
#include "monsters.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <conio.h> // Nécessaire pour getch() sur Windows

/*
#include <termios.h> // Remplace conio.h pour Mac/Linux
#include <unistd.h> // Fonction read() pour getch() sur Mac/Linux

int getch_mac() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

*/

static void clear_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static void trim_newline(char *s)
{
    if (!s) return;
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n')
        s[len - 1] = '\0';
}

static void prompt_item(Item *item)
{
    if (!item)
        return;

    printf("Nom (peut contenir des espaces) : ");
    if (fgets(item->name, sizeof(item->name), stdin)) {
        trim_newline(item->name);
    }

    printf("hpMax (float) : ");
    scanf("%f", &item->hpMax);

    printf("shield (int) : ");
    scanf("%d", &item->shield);

    printf("dmg (float) : ");
    scanf("%f", &item->dmg);

    printf("ps (0/1) : ");
    scanf("%d", (int *)&item->ps);

    printf("ss (0/1) : ");
    scanf("%d", (int *)&item->ss);

    printf("flight (0/1) : ");
    scanf("%d", (int *)&item->flight);

    clear_stdin();
}

static void run_item_menu(void)
{
    bool running = true;
    while (running) {
        printf("\n=== CRUD d'objets (items) ===\n");
        printf("1) Creer item\n");
        printf("2) Lister les items\n");
        printf("3) Modifier item\n");
        printf("4) Supprimer item\n");
        printf("0) Retour\n");
        printf("Choix : ");

        int choix = -1;
        if (scanf("%d", &choix) != 1) {
            clear_stdin();
            continue;
        }
        clear_stdin();

        switch (choix) {
            case 1: {
                Item newItem = {0};
                prompt_item(&newItem);
                if (append_item_to_file(ITEMS_FILE_NAME, &newItem))
                    printf("Item ajoute\n");
                else
                    printf("Erreur a l\'ecriture\n");
            } break;
            case 2:
                print_all_items(ITEMS_FILE_NAME);
                break;
            case 3: {
                print_all_items(ITEMS_FILE_NAME);
                printf("Index de l\'item a modifier: ");
                size_t idx;
                if (scanf("%zu", &idx) != 1) {
                    clear_stdin();
                    break;
                }
                clear_stdin();
                Item updated = {0};
                prompt_item(&updated);
                if (update_item_in_file(ITEMS_FILE_NAME, idx, &updated))
                    printf("Item mis à jour\n");
                else
                    printf("Erreur : indice invalide ou mise a jour impossible\n");
            } break;
            case 4: {
                print_all_items(ITEMS_FILE_NAME);
                printf("Index de l\'item a supprimer: ");
                size_t idx;
                if (scanf("%zu", &idx) != 1) {
                    clear_stdin();
                    break;
                }
                clear_stdin();
                if (delete_item_in_file(ITEMS_FILE_NAME, idx))
                    printf("Item supprime\n");
                else
                    printf("Erreur : indice invalide ou suppression impossible\n");
            } break;
            case 0:
                running = false;
                break;
            default:
                printf("Choix invalide.\n");
                break;
        }
    }
}

static void prompt_entity(Entity *entity)
{
    if (!entity)
        return;

    printf("Nom (peut contenir des espaces) : ");
    if (fgets(entity->name, sizeof(entity->name), stdin)) {
        trim_newline(entity->name);
    }

    printf("Vie : ");
    scanf("%f", &entity->hpMax);
/*
    printf("dmg (float) : ");
    scanf("%f", &entity->dmg);

    printf("x (int) : ");
    scanf("%d", &entity->x);

    printf("y (int) : ");
    scanf("%d", &entity->y);
*/
    printf("Peut tirer? (0/1) : ");
    scanf("%d", (int *)&entity->shoot);

    printf("Tir spectral? (0/1) : ");
    scanf("%d", (int *)&entity->ss);

    printf("Peut voler? (0/1) : ");
    scanf("%d", (int *)&entity->flight);

    clear_stdin();
}

static void run_monster_menu(void)
{
    bool running = true;
    while (running) {
        printf("\n=== CRUD de monstres ===\n");
        printf("1) Creer monstre\n");
        printf("2) Lister les monstres\n");
        printf("3) Modifier monstre\n");
        printf("4) Supprimer monstre\n");
        printf("0) Retour\n");
        printf("Choix : ");

        int choix = -1;
        if (scanf("%d", &choix) != 1) {
            clear_stdin();
            continue;
        }
        clear_stdin();

        switch (choix) {
            case 1: {
                Entity newEntity = {0};
                prompt_entity(&newEntity);
                if (append_entity_to_file(MONSTERS_FILE_NAME, &newEntity))
                    printf("Monstre ajouté\n");
                else
                    printf("Erreur à l'écriture\n");
            } break;
            case 2:
                print_all_entities(MONSTERS_FILE_NAME);
                break;
            case 3: {
                print_all_entities(MONSTERS_FILE_NAME);
                printf("Index du monstre à modifier : ");
                size_t idx;
                if (scanf("%zu", &idx) != 1) {
                    clear_stdin();
                    break;
                }
                clear_stdin();
                Entity updated = {0};
                prompt_entity(&updated);
                if (update_entity_in_file(MONSTERS_FILE_NAME, idx, &updated))
                    printf("Monstre mis à jour\n");
                else
                    printf("Erreur : indice invalide ou mise à jour impossible\n");
            } break;
            case 4: {
                print_all_entities(MONSTERS_FILE_NAME);
                printf("Index du monstre à supprimer : ");
                size_t idx;
                if (scanf("%zu", &idx) != 1) {
                    clear_stdin();
                    break;
                }
                clear_stdin();
                if (delete_entity_in_file(MONSTERS_FILE_NAME, idx))
                    printf("Monstre supprimé\n");
                else
                    printf("Erreur : indice invalide ou suppression impossible\n");
            } break;
            case 0:
                running = false;
                break;
            default:
                printf("Choix invalide.\n");
                break;
        }
    }
}

int old_main(int argc, char const *argv[])
{
    
    //Variables génériques
    int i, j, n, m;
    char choix;
    Room tab_Room[14];

    //Title
    printf("CRUD des salles du donjon\n\n");


    //Prédéfinition des salles

    Room spawner, itRoom, bRoom, itRoomB; //Salles spéciales
    Room R1, R2, R3, R4, R5, R6, R7, R8, R9, R10; //10 salles
    //tab_Room[14] = {spawner, itRoom, bRoom, itRoomB, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10}; 
    tab_Room[0] = R1;   tab_Room[5] = R6;   tab_Room[10] = spawner;
    tab_Room[1] = R2;   tab_Room[6] = R7;   tab_Room[11] = itRoom;
    tab_Room[2] = R3;   tab_Room[7] = R8;   tab_Room[12] = bRoom;
    tab_Room[3] = R4;   tab_Room[8] = R9;   tab_Room[13] = itRoomB;
    tab_Room[4] = R5;   tab_Room[9] = R10;
    
    
    //Création des salles par défaut

        //Choix de la taille par l'user
    
    printf("Entrez n (hauteur) : ");
    scanf("%d", &n);
    printf("Entrez m (largeur) : ");
    scanf("%d", &m);
    printf("\n");

    for(i = 0; i < 14; i++)
        set_RoomId(&tab_Room[i], i+1); //Affectation des id
    

            //Salles modifiables par l'user
    creer(&tab_Room[0], n, m);
    show(&tab_Room[0]);


    bool running = true;

    char choice;
    printf("Obstacle a ajouter : G | R ? : ");
    scanf(" %c", &choice);  
    //tab_Room[0].grid[tab_Room[0].height/2][tab_Room[0].width/2] = choice;
    
    int X = tab_Room[0].width/2;
    int Y = tab_Room[0].height/2;

    int nextX = X;
    int nextY = Y;

    while (running) {

        
        
        //int nextX , nextY; 
        // int X = tab_Room[0].width/2;
        // int Y = tab_Room[0].height/2;

        // nextX = tab_Room[0].width/2;
        // nextY = tab_Room[0].height/2;

        tab_Room[0].grid[Y][X] = choice;

        printf("\n");
        show(&tab_Room[0]);
        
        
        printf("\n");
        printf("\n");
        //scanf(" %c", &touche); 
        int touche = getch();

            if (touche == 'z' || touche == 'w') 
                nextY--;
            if (touche == 's') 
                nextY++;
            if (touche == 'q' || touche == 'a') 
                nextX--;
            if (touche == 'd') 
                nextX++;
            if (touche == 'x') 
                running = false;
        

        // Collision : on ne bouge que si la case n'est pas un Mur ou un Rocher
        if (tab_Room[0].grid[nextY][nextX] != 'W' && tab_Room[0].grid[nextY][nextX] != 'R') {
            X = nextX;
            Y = nextY;
        }
        
    }





     /*     
    creer(&tab_Room[0], n, m);
    show(&tab_Room[0]);
    printf("\n");
    creer(&tab_Room[2], n, m);
    show(&tab_Room[2]);
    printf("\n");
    creer(&tab_Room[4], n, m);
    show(&tab_Room[4]);
    printf("\n");
    creer(&tab_Room[7], n, m);
    show(&tab_Room[7]);
    printf("\n");
    */
    
    printf("\n");
   
    






    // for(i = 0; i < 10; i++)
    //     show(&tab_Room[i]);




    //Choix de l'action
    // printf("Que souhaitez vous faire?\n"); 
    // do{
    //     printf("Tapez c(Create); r(read), u(update) ou d(delete) : ");
    //     scanf(" %c", &choix);
    // }while(choix != 'c' &&  choix != 'r' && choix != 'u' && choix != 'd');

    /*
    //Exécution selon le choix de l'user
    switch(choix){
        case 'c' :
            //creer(n, m);
            break;
        case 'r' :
            //show(..., n, m); //Mettre plutôt en paramètre le numéro de la salle correspondante
            break;
        case 'u' :
            //modif(..., n, m); //Idem ici
            break;
        case 'd' :
            //suppr(...) //Idem pour l'indice correspondant dans le tableau de la strucure
            break;
        default :
            //...
            break;

    }

    */

    run_item_menu();
    run_monster_menu();

     printf("=== TBOB Multi-Floor Generator ===\n\n");
    
    // Load room templates once
    RoomTemplate *templates = NULL;
    size_t template_count = 0;
    
    if (!load_room_templates("rooms.rtbob", &templates, &template_count)) {
        fprintf(stderr, "Failed to load room templates!\n");
        return 1;
    }
    
    printf("Loaded %zu room templates\n\n", template_count);
    
    // Generate multiple floors
    for (int floor = 0; floor < 3; ++floor) {
        printf("\n========== GENERATING FLOOR %d ==========\n", floor);
        
        Room *rooms = NULL;
        size_t room_count = 0;
        
        // Generate this floor
        if (!generate_floor(floor, templates, template_count,
                           "monstres.mtbob", "items.itbob",
                           &rooms, &room_count)) {
            fprintf(stderr, "Error generating floor %d!\n", floor);
            continue;
        }
        
        printf("Generated %zu rooms for floor %d\n", room_count, floor);
        
        // TODO: Process rooms here
        // For example:
        // - Save to file
        // - Display room summary
        // - Initialize game state with these rooms
        // - Setup spawn points, treasure locations, etc.
        
        for (size_t i = 0; i < room_count; ++i) {
            const char *type;
            switch (i) {
                case 0: type = "SPAWNER"; break;
                case 11: type = "ITEM_ROOM"; break;
                case 12: type = "BOSS_ROOM"; break;
                case 13: type = "BONUS_ITEM"; break;
                default: type = "NORMAL";
            }
            printf("  Room %zu (%s): %dx%d\n", i, type, rooms[i].height, rooms[i].width);
        }
        
        // Free rooms for this floor
        free_rooms(rooms, room_count);
    }
    
    // Free templates
    free_room_templates(templates, template_count);
    
    printf("\n========== GENERATION COMPLETE ==========\n");

        printf("=== FLOOR GENERATION TEST ===\n\n");

    // Test floor generation
    if (generate_and_display_floor(0, "rooms.rtbob", "monstres.mtbob", "items.itbob")) {
        printf("\nFloor 0 generated successfully!\n");
    } else {
        fprintf(stderr, "Failed to generate floor 0\n");
        return 1;
    }

    printf("\n=== TEST COMPLETE ===\n");

    return 0;
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

int main(int argc, char const *argv[])
{
    printf("=== TBOB Playable Prototype ===\n\n");
    printf("Contrôles : z/s/q/d ou w/s/a/d, x pour quitter.\n\n");

    int height = 0, width = 0;
    do {
        printf("Entrez la hauteur impaire des salles (9-19) : ");
        if (scanf("%d", &height) != 1) { clear_stdin(); height = 0; continue; }
        clear_stdin();
    } while (height < 9 || height > 19 || (height % 2 == 0));

    do {
        printf("Entrez la largeur impaire des salles (9-19) : ");
        if (scanf("%d", &width) != 1) { clear_stdin(); width = 0; continue; }
        clear_stdin();
    } while (width < 9 || width > 19 || (width % 2 == 0));

    // Création des 14 salles en mémoire avec les dimensions partagées
    Room rooms[14];
    create_spawner_room_custom(&rooms[0], 0, height, width);
    for (int i = 1; i <= 10; ++i) {
        create_normal_room_custom(&rooms[i], i, height, width);
    }
    create_item_room_custom(&rooms[11], 11, height, width, 'I');
    create_boss_room_custom(&rooms[12], 12, height, width);
    create_item_room_custom(&rooms[13], 13, height, width, 'H');

    // Positionnement aléatoire contigu sur grille 5x6 avec spawn central
    typedef struct { int x, y; } Coord;
    Coord pool[29];
    int idx = 0;
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 5; ++x) {
            if (x == 2 && y == 2) continue; // position centrale réservée au spawn
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
    // Spawn en position centrale fixe
    room_position[0] = (Coord){2, 2};
    layout[2][2] = 0;

    bool used[6][5] = {0};
    used[2][2] = true;

    for (int i = 1; i < 14; ++i) {
        // Construire la liste de candidats adjacents déjà connectés
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
            // Aucun choix connecté, tomber en fallback sur une position libre
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

    int adjacency[14][4]; // N E S W
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
    // joueur deja place dans spawn

    bool playing = true;
    while (playing) {
        system("cls");
        printf("Salle actuelle : %d / 13\n", current_room);

        // Affiche la salle actuelle avec la minicarte a cote
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                printf("%c ", rooms[current_room].grid[y][x]);
            }
            if (y == 0) printf("   5x6 Minicarte:");
            if (y < 6) {
                int yy = y;
                printf("   ");
                for (int xx = 0; xx < 5; ++xx) {
                    if (layout[yy][xx] == -1) printf(" . ");
                    else if (layout[yy][xx] == current_room) printf(" P ");
                    else printf(" * ");
                }
            }
            printf("\n");
        }

        printf("\nMouvement : z/w/up, s/down, q/a/gauche, d/droite, x/quitter\n");
        int key = getch();
        if (key == 'x' || key == 'X') break;

        int dx = 0, dy = 0;
        if (key == 'z' || key == 'w') dy = -1;
        else if (key == 's') dy = 1;
        else if (key == 'q' || key == 'a') dx = -1;
        else if (key == 'd') dx = 1;
        else continue;

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

            if (dir == -1) continue; // puerta inválida, no navegar

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
