#include "crud1_salle.h"
#include "salle.h"
#include "structs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//Getter and setter

void set_RoomId(Room *R, int id){
    R->id = id;
}



void create(Room *R, int n, int m){
    
    int i, j;
    R->height = n;
    R->width = m;
    R->grid = malloc(sizeof(char*) * n);
    //Allocation de l'espace et Initialisation 
    //de la salle à des espaces vides partout
    for(i = 0; i < n; i += 1){
        R->grid[i] = malloc(m);
        for(j = 0; j < m; j += 1){
            R->grid[i][j] = ' ';
        }
    }
    
    //Murs et portes
        //M_H et M_B
        
    for(j = 0; j < m; j += 1){
        if(j == m/2){
            R->grid[0][j] = 'D';
            R->grid[n-1][j] = 'D';
        }/*
        else if(j == m/2+1){
            R->grid[0][j] = 'W';
            R->grid[n-1][j] = 'D';
        }*/
        else{
            R->grid[0][j] = 'W';
            R->grid[n-1][j] = 'W';
        }
    }

        //M_D et M_G
    for(i = 1; i < n-1; i += 1){
        if(i == n/2){
            R->grid[i][0] = 'D';
            R->grid[i][m-1] = 'D';
        }
        else{
            R->grid[i][0] = 'W';
            R->grid[i][m-1] = 'W';
        }
    }
    /*
    //Diagonale_1
    R->grid[1][1] = 'H'; R->grid[1][2] = 'G'; R->grid[2][2] = 'G'; R->grid[2][1] = 'G'; 
    R->grid[n-2][m-2] = 'H'; R->grid[n-2][m-3] = 'G'; R->grid[n-3][m-3] = 'G'; R->grid[n-3][m-2] = 'G'; 
    //Diagonale_2
    R->grid[n-3][1] = 'G'; R->grid[n-3][2] = 'G'; R->grid[n-2][2] = 'G';
    R->grid[2][m-2] = 'G'; R->grid[2][m-3] = 'G'; R->grid[1][m-3] = 'G';
    */


    //Ajout d'éléments dans la salle

    
}


void modificate(char** s, int n, int m){
    (void)s;
    (void)n;
    (void)m;
}


void delete(char** s){
    (void)s;
}

void show_room(Room *R){
    
    int i, j;
    for(i = 0; i < R->height; i += 1){
        for(j = 0; j < R->width; j += 1){
            printf("%c ", R->grid[i][j]);
        }
        printf("\n");
    }
    
    //freeR(R);
}

void configure_room_doors(Room *room, bool north, bool east, bool south, bool west) {
    if (!room) return;
    int midW = room->width / 2;
    int midH = room->height / 2;

    // Asegurarse de no tener puertas sobrantes en el borde
    for (int j = 0; j < room->width; ++j) {
        room->grid[0][j] = 'W';
        room->grid[room->height - 1][j] = 'W';
    }
    for (int i = 0; i < room->height; ++i) {
        room->grid[i][0] = 'W';
        room->grid[i][room->width - 1] = 'W';
    }

    if (north) room->grid[0][midW] = 'D';
    if (south) room->grid[room->height - 1][midW] = 'D';
    if (west)  room->grid[midH][0] = 'D';
    if (east)  room->grid[midH][room->width - 1] = 'D';
}

void freeRoom(Room *R) {
    if (!R) return;
    if (R->grid) {
        for (int i = 0; i < R->height; ++i) {
            free(R->grid[i]);
        }
        free(R->grid);
    }
}

// ==================== Floor Generation ====================

bool generate_and_display_floor(int floor_num,
                               const char *rooms_template_file,
                               const char *monsters_file,
                               const char *items_file) {
    if (!rooms_template_file || !monsters_file || !items_file)
        return false;

    // Load room templates
    RoomTemplate *templates = NULL;
    size_t template_count = 0;

    if (!load_room_templates(rooms_template_file, &templates, &template_count)) {
        fprintf(stderr, "Error loading room templates from %s\n", rooms_template_file);
        return false;
    }

    // Generate the floor
    Room *rooms = NULL;
    size_t room_count = 0;

    if (!generate_floor(floor_num, templates, template_count,
                       monsters_file, items_file,
                       &rooms, &room_count)) {
        fprintf(stderr, "Error generating floor %d\n", floor_num);
        free_room_templates(templates, template_count);
        return false;
    }

    // Display generated rooms
    printf("\n========== FLOOR %d ==========\n\n", floor_num);
    printf("Total rooms generated: %zu\n\n", room_count);

    for (size_t i = 0; i < room_count; ++i) {
        const char *room_type = "Normal";
        if (i == 0) room_type = "SPAWNER";
        else if (i == 11) room_type = "ITEM ROOM";
        else if (i == 12) room_type = "BOSS ROOM";
        else if (i == 13) room_type = "BONUS ITEM ROOM";

        printf("\n--- Room #%zu (%s) - ID: %d ---\n", i, room_type, rooms[i].id);
        show_room(&rooms[i]);
    }

    printf("\n========== FLOOR %d COMPLETE ==========\n", floor_num);

    // Cleanup
    free_rooms(rooms, room_count);
    free_room_templates(templates, template_count);

    return true;
}