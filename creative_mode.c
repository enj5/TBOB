#include "creative_mode.h"
#include "items.h"
#include "monsters.h"
#include "rooms.h"
#include "crud1_salle.h"
#include "structs.h"
#include "crud1_salle.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

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

    printf("Peut tirer? (0/1) : ");
    scanf("%d", (int *)&entity->shoot);

    printf("Tir spectral? (0/1) : ");
    scanf("%d", (int *)&entity->ss);

    printf("Peut voler? (0/1) : ");
    scanf("%d", (int *)&entity->flight);

    clear_stdin();
}


void run_room_menu(void)
{

    //Variables génériques
    int i, nS;
    char menu, keep, choix;
    Room tab_Room[26]; //Tableau des salles
    Room spawner, itRoom, bRoom, itRoomB; //Salles spéciales
    

    //Boucle unique des actions du menu
    do
    {
        system("cls");

        //Title
        printf("\n--- CRUD des pieces du donjon---\n\n");
        
        //Affichage du menu
        printf("Menu :\n");
        printf("(10 salles par defaut sont existantes)\n");
        printf("(Tapez le numero correspondant a l'action de votre choix)\n\n");
        printf("1- Afficher les salles\n");
        printf("2- Creer une nouvelle salle\n"); //Juste créer une salle vide nxm avec un id = nS+1. Les ajouts d'obstacles se feront dans le 3.Modifier
        printf("3- Modifier une salle\n");
        printf("4- Supprimer une salle\n");
        printf("5- Quitter\n");

        //Choix de l'option
        do
        {
            printf("\nEntrer votre choix (1|2|3|4|5) : ");
            scanf(" %c", &menu);
        } while(menu != '1' && menu != '2' && menu != '3' && menu != '4' && menu != '5');
        printf("\n\n");

        //Chargement initial du fichier dans le tableau des salles
        FILE* fich = fopen("salles.rtbob", "r"); // Ouverture du fichier en lecture 
            //Chargement du contenu dans le tableau des salles
        if(fich != NULL){
            fscanf(fich, "{%d}\n", &nS); //Lecture nbre de salles
            for(i = 0; i < nS; i++){
                ReadInFile(&tab_Room[i], fich); //Lecture de chaque salle
            }
            //fclose(fich);
        }
    
        switch(menu)
        {
            case '1':
                //FILE* fich = fopen("salles.rtbob", "r"); // Ouverture du fichier en lecture
                printf("\n");
                printf("{%d}\n", nS);
                for(i = 0; i < nS; i++){
                    printf("[%d|%d]%d\n", tab_Room[i].height, tab_Room[i].width, tab_Room[i].id);
                    show(tab_Room[i]); //Affichage de la salle dans la console
                }
                //fclose(fich);
                //Liberation de l'espace alloue
                for(i = 0; i < nS; i++)
                    freeR(&tab_Room[i]);
                
                break;
            case '2':
                char keep_create;
                Room New_Room;
                do
                {
                    system("cls");
                    nS = nS + 1;
                    creer(&tab_Room[nS-1], nS, 9, 15); //Creation d'une salle 9x15 vide
                    fclose(fich);
                    fich = fopen("salles.rtbob", "w+");   
                    fprintf(fich, "{%d}\n", nS);
                    for(i = 0; i < nS; i++){    
                        WriteInFile(&tab_Room[i], fich);
                    }
                    fclose(fich);
                    printf("\nSALLE AJOUTEE AVEC SUCCES!");
                    printf("\nAjouter une nouvelle salle ?");
                    do
                    {
                        printf("(o/n) : ");
                        scanf(" %c", &keep_create);
                    } while (keep_create != 'o' && keep_create != 'n');
                } while (keep_create == 'o');
                
                break;
            case '3':
                char on, keep_up;
                int num;
                //MODIFICATION DES OBSTACLES
                printf("\n");
                do
                {
                    system("cls");
                    printf("Entrer le numero de la salle");
                    do{
                        printf("\n[1, %d] : ", nS); //Restriction au nombre de salle existant
                        scanf(" %d", &num);
                        /*
                        while(scanf(" %d", &num) == 0){
                            printf("\n[1, %d] : ", nS);
                            while(getchar() != '\n') //vider le buffer
                                printf("\n[1, %d] : ", nS);
                        }
                        */  //TENTATIVE D'IMPOSER L'ENTRER D'UNE VALEUR ENTIERE! A REVOIR
                    }while(num < 1 || num > nS);

                    //Modification de la salle
                    modif(&tab_Room[num-1], num);
                    
                    //Reecriture dans le fichier
                    //rewind(fich); //Retour au début du fichier
                    fclose(fich);//Fermeture pour vider le fichier avant de reecrire 
                    fich = fopen("salles.rtbob", "w+");   
                    fprintf(fich, "{%d}\n", nS);
                    for(i = 0; i < nS; i++){    
                        WriteInFile(&tab_Room[i], fich);
                    }
                    fclose(fich); 
                    do
                    {
                        printf("\nModifier une autre salle? (o/n) : ");
                        scanf(" %c", &keep_up);
                    } while (keep_up != 'o' && keep_up != 'n');    
                } while (keep_up == 'o');
                break;
            case '4':
                int del;
                char keep_del;
                fclose(fich);
                do
                {
                    system("cls");
                    //Cas Aucune salle dans le fichier
                    if(nS == 0){
                        printf("Aucune salles existante!"); 
                        keep_del = 'n';
                    }
                    //Il y a au moins une salle
                    else{
                        printf("Entrez le numero de la salle a supprimer");
                        //Conditionnement des valeurs d'entree
                        do
                        {
                            printf("[1, %d] : ", nS);
                            scanf(" %d", &del);
                        } while (del < 1 || del > nS); //Pas frocément optimal - à revoir
                        
                        DelInFile(del, nS); //Suppression et reecriture dans le fichier
                        printf("Voulez-vous supprimer une autre salle?\n");
                        do
                        {
                            printf("(o/n) : ");
                            scanf(" %c", &keep_del);
                        } while (keep_del != 'o' && keep_del != 'n');
                    }
                } while (keep_del == 'o');
                
                
                break;
            case '5':
                system("cls");
                printf("\nVous allez quitter le menu.\n");
                break;
            default:
                printf("ERREUR DE SAISIE!");
                break;
        }
        fclose(fich);
        do
        {
            printf("\nRetourner au Menu --> Tapez R | Quitter --> Tapez Q : ");
            scanf(" %c", &keep);
        } while (keep != 'R' && keep != 'Q');
    } while (keep == 'R');
    
    
    for(i = 0; i < nS; i++)
        freeR(&tab_Room[i]);

}


static void run_item_menu(void)
{
    bool running = true;
    while (running) {
        printf("\n=== CRUD d'objets ===\n");
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
                    printf("Erreur a l'ecriture\n");
            } break;
            case 2:
                print_all_items(ITEMS_FILE_NAME);
                break;
            case 3: {
                print_all_items(ITEMS_FILE_NAME);
                printf("Index de l'item a modifier: ");
                size_t idx;
                if (scanf("%zu", &idx) != 1) {
                    clear_stdin();
                    break;
                }
                clear_stdin();
                Item updated = {0};
                prompt_item(&updated);
                if (update_item_in_file(ITEMS_FILE_NAME, idx, &updated))
                    printf("Item mis a jour\n");
                else
                    printf("Erreur : indice invalide ou mise a jour impossible\n");
            } break;
            case 4: {
                print_all_items(ITEMS_FILE_NAME);
                printf("Index de l'item a supprimer: ");
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
                    printf("Monstre ajoute\n");
                else
                    printf("Erreur a l'ecriture\n");
            } break;
            case 2:
                print_all_entities(MONSTERS_FILE_NAME);
                break;
            case 3: {
                print_all_entities(MONSTERS_FILE_NAME);
                printf("Index du monstre a modifier : ");
                size_t idx;
                if (scanf("%zu", &idx) != 1) {
                    clear_stdin();
                    break;
                }
                clear_stdin();
                Entity updated = {0};
                prompt_entity(&updated);
                if (update_entity_in_file(MONSTERS_FILE_NAME, idx, &updated))
                    printf("Monstre mis a jour\n");
                else
                    printf("Erreur : indice invalide ou mise a jour impossible\n");
            } break;
            case 4: {
                print_all_entities(MONSTERS_FILE_NAME);
                printf("Index du monstre a supprimer : ");
                size_t idx;
                if (scanf("%zu", &idx) != 1) {
                    clear_stdin();
                    break;
                }
                clear_stdin();
                if (delete_entity_in_file(MONSTERS_FILE_NAME, idx))
                    printf("Monstre supprime\n");
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

int creative_mode(void)
{
    printf("\n");
    run_room_menu();
    run_item_menu();
    run_monster_menu();

    // printf("=== TBOB Multi-Floor Generator ===\n\n");
    
    // RoomTemplate *templates = NULL;
    // size_t template_count = 0;

    // if (!load_room_templates("rooms.rtbob", &templates, &template_count)) {
    //     fprintf(stderr, "Failed to load room templates!\n");
    //     return 1;
    // }
    
    // printf("Loaded %zu room templates\n\n", template_count);

    // for (int floor = 0; floor < 3; ++floor) {
    //     printf("\n========== GENERATING FLOOR %d ==========/n", floor);
    //     Room *rooms = NULL;
    //     size_t room_count = 0;

    //     if (!generate_floor(floor, templates, template_count,
    //                         "monstres.mtbob", "items.itbob",
    //                         &rooms, &room_count)) {
    //         fprintf(stderr, "Error generating floor %d!\n", floor);
    //         continue;
    //     }

    //     printf("Generated %zu rooms for floor %d\n", room_count, floor);

    //     for (size_t i = 0; i < room_count; ++i) {
    //         const char *type;
    //         switch (i) {
    //             case 0: type = "SPAWNER"; break;
    //             case 11: type = "ITEM_ROOM"; break;
    //             case 12: type = "BOSS_ROOM"; break;
    //             case 13: type = "BONUS_ITEM"; break;
    //             default: type = "NORMAL";
    //         }
    //         printf("  Room %zu (%s): %dx%d\n", i, type, rooms[i].height, rooms[i].width);
    //     }

    //     free_rooms(rooms, room_count);
    // }

    // free_room_templates(templates, template_count);

    // printf("\n========== GENERATION COMPLETE ==========/n");

    // printf("=== FLOOR GENERATION TEST ===\n\n");
    // if (generate_and_display_floor(0, "rooms.rtbob", "monstres.mtbob", "items.itbob")) {
    //     printf("\nFloor 0 generated successfully!\n");
    // } else {
    //     fprintf(stderr, "Failed to generate floor 0\n");
    //     return 1;
    // }

    return 0;
}
