#include "structs.h"
#include "rooms.h"
#include <stdbool.h>

#ifndef SALLE_H

#define SALLE_H

// Original functions
void set_RoomId(Room*, int); //Identification des salles
void create(Room*, int, int);
void modificate(char**, int, int);
void delete(char**);
void show_room(Room*);
void freeRoom(Room*);

// Door configuration helper for playable room transitions
void configure_room_doors(Room *room, bool north, bool east, bool south, bool west);

// New floor generation functions
/// Generate a complete floor with 14 rooms (10 normal + spawner + boss + 2 item rooms)
/// This is the main entry point for floor generation.
bool generate_and_display_floor(int floor_num,
                               const char *rooms_template_file,
                               const char *monsters_file,
                               const char *items_file);

#endif /* SALLE_H */