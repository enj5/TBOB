#ifndef ROOMS_H
#define ROOMS_H

#include <stdbool.h>
#include <stddef.h>
#include "structs.h"

/// Room template structure (for loading from .rtbob file)
typedef struct {
    char name[50];      // Room template name (e.g., "BasicRoom", "TreeRoom")
    int height;         // Room height
    int width;          // Room width
    char **grid;        // Grid layout
} RoomTemplate;

/// Load all room templates from the file.
/// Returns true on success; on failure *outRooms will be set to NULL and *outCount to 0.
bool load_room_templates(const char *filename, RoomTemplate **outRooms, size_t *outCount);

/// Free an array returned by load_room_templates().
void free_room_templates(RoomTemplate *rooms, size_t count);

/// Print a single room template to stdout.
void print_room_template(const RoomTemplate *room, size_t index);

/// Print all room templates stored in the file.
bool print_all_room_templates(const char *filename);

/// Generate a floor with 14 rooms (10 normal + 1 spawner + 1 boss room + 2 item rooms).
/// Normal rooms have random monsters (0-6) from monsters file.
/// Item rooms have a single random item from items file.
/// Boss room is empty with single entrance door.
/// Spawner room is empty with player starting position in center.
/// 
/// Parameters:
///   - floor_num: Floor number (0-indexed)
///   - room_templates: Available room templates
///   - template_count: Number of templates
///   - monsters_file: Filename for monsters (.mtbob)
///   - items_file: Filename for items (.itbob)
///   - outRooms: Output array of generated Room structs (must be freed by caller)
///   - outCount: Number of rooms generated (should be 14)
/// 
/// Returns: true on success, false on failure
bool generate_floor(int floor_num, 
                   const RoomTemplate *room_templates, size_t template_count,
                   const char *monsters_file,
                   const char *items_file,
                   Room **outRooms, size_t *outCount);

// Fonctions additionnelles pour jeu avec salles personnalisées
void create_spawner_room_custom(Room *room, int id, int height, int width);
void create_normal_room_custom(Room *room, int id, int height, int width);
void create_item_room_custom(Room *room, int id, int height, int width, char item_code);
void create_boss_room_custom(Room *room, int id, int height, int width);

/// Free an array of Room structs returned by generate_floor().
void free_rooms(Room *rooms, size_t count);

#endif // ROOMS_H
