#include "structs.h"
#include "rooms.h"
#include <stdbool.h>

#ifndef SALLE_H

#define SALLE_H

// Original functions
void set_RoomId(Room*, int); //Identification des salles
void initialiser_salle(Room*, int, int);
void modifier(char**, int, int);
void supprimer(char**);
void afficher_salle(Room*);
void liberer_salle_simple(Room*);

// Door configuration helper for playable room transitions
void configurer_portes_salle(Room *room, bool north, bool east, bool south, bool west);

// New floor generation functions
/// Generate a complete floor with 14 rooms (10 normal + spawner + boss + 2 item rooms)
/// This is the main entry point for floor generation.
bool generer_et_afficher_etage(int floor_num,
                               const char *rooms_template_file,
                               const char *monsters_file,
                               const char *items_file);

#endif /* SALLE_H */