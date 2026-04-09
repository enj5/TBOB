- Les lignes de codes identifiés comme **superflus** sont à enlever!!! Voir avec Miguel  
    - L22/play_mode `static int load_monsters(const char *filename, Entity monsters[], int max_monsters)`  


JE ME SUIS ARRETE A LA LIGNE 225

``` C


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

    typedef struct { int x, y; } Coord; // Structure pour les position dans la mini map
    Coord pool[29]; // Ah oui le tableau contenant les positions des 30 Rooms

    Entity Lenina;
    Entity jogger;

    jogger.dmg = 1.0f;
    jogger.hpMax = 5.0f;

    int idx = 0;
    // Tableau de la mini map - Dans ce double for on défini les diff. positions
    // On affecte les coordonnées, sauf en x = 2 et y = 2
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 5; ++x) {
            if (x == 2 && y == 2) continue; // Pourquoi spécialement en (2,2)
            pool[idx++] = (Coord){x, y};
        }
    }

    srand((unsigned int)time(NULL)); // Initialisation pour le rand()(nombre pseudo aléatoire sur 32 bits)!!!
    // pour obtenir une valeur différente à chaque seconde
    for (int i = 28; i > 0; --i) {
        int j = rand() % (i + 1); // Nombre aléatoire entre 0 et i+1
        Coord tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
        // Ici est redistribué au hasard les positions (x,y) dans le tableau des positions
    }
    // On est sur d'avoir les 27??

    int layout[6][5]; // Matrice 6x5
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 5; ++x)
            layout[y][x] = -1; // initialisation de toute la matrice à -1
            //J'imagine que c'est pour savoir si une position a été modifiée ou non

    Coord room_position[14];
    room_position[0] = (Coord){2, 2}; // C'est pour ça que le (2,2) a été réservé
    layout[2][2] = 0; // On dit que la position (2,2) a été attribuée

    bool used[6][5] = {0}; // Initialisation de toute la matrice à 0
    used[2][2] = true; // On dit également que la position (2,2) a été attribuée
    // A quoi sert encore layout donc??



```
