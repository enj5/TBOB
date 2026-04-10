#include "rooms.h"
#include "monsters.h"
#include "items.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ==================== Helper Functions ====================

static void trim_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n')
        s[len - 1] = '\0';
    if (len > 1 && s[len - 2] == '\r')
        s[len - 2] = '\0';
}

static char *trim_whitespace(char *s) {
    if (!s) return s;
    while (*s && isspace((unsigned char)*s))
        s++;
    if (*s == '\0')
        return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        *end-- = '\0';
    return s;
}

static bool parse_line_kv(char *line, char **outKey, char **outValue) {
    char *sep = strchr(line, '=');
    if (!sep) return false;
    *sep = '\0';
    *outKey = trim_whitespace(line);
    *outValue = trim_whitespace(sep + 1);
    return true;
}

static int parse_int(const char *s) {
    return (int)strtol(s, NULL, 10);
}

// ==================== Room Template File I/O ====================

bool load_room_templates(const char *filename, RoomTemplate **outRooms, size_t *outCount) {
    if (!outRooms || !outCount)
        return false;

    *outRooms = NULL;
    *outCount = 0;

    FILE *f = fopen(filename, "r");
    if (!f)
        return true; // Missing file = empty list

    RoomTemplate *rooms = NULL;
    size_t cap = 0;

    char line[256];
    RoomTemplate current;
    bool inBlock = false;

    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        char *cursor = trim_whitespace(line);
        if (*cursor == '\0')
            continue;

        if (*cursor == '{') {
            // count line like {2} - ignore for parsing
            continue;
        }

        if (strcmp(cursor, "---") == 0) {
            if (inBlock) {
                // finish previous block
                if (*outCount >= cap) {
                    cap = (cap == 0) ? 4 : cap * 2;
                    RoomTemplate *tmp = realloc(rooms, cap * sizeof(RoomTemplate));
                    if (!tmp) {
                        free(rooms);
                        fclose(f);
                        return false;
                    }
                    rooms = tmp;
                }
                rooms[(*outCount)++] = current;
            }
            inBlock = true;
            memset(&current, 0, sizeof(current));
            continue;
        }

        if (inBlock) {
            char *key, *value;
            if (parse_line_kv(cursor, &key, &value)) {
                if (strcmp(key, "name") == 0) {
                    strncpy(current.name, value, sizeof(current.name) - 1);
                } else if (strcmp(key, "height") == 0) {
                    current.height = parse_int(value);
                } else if (strcmp(key, "width") == 0) {
                    current.width = parse_int(value);
                } else if (strcmp(key, "grid") == 0) {
                    // Grid data would be loaded line by line in a real scenario
                    // For now, we'll initialize it in generate_floor
                }
            }
        }
    }

    // Finish last block
    if (inBlock) {
        if (*outCount >= cap) {
            cap = (cap == 0) ? 4 : cap * 2;
            RoomTemplate *tmp = realloc(rooms, cap * sizeof(RoomTemplate));
            if (!tmp) {
                free(rooms);
                fclose(f);
                return false;
            }
            rooms = tmp;
        }
        rooms[(*outCount)++] = current;
    }

    *outRooms = rooms;
    fclose(f);
    return true;
}

void free_room_templates(RoomTemplate *rooms, size_t count) {
    if (!rooms) return;
    for (size_t i = 0; i < count; ++i) {
        if (rooms[i].grid) {
            for (int j = 0; j < rooms[i].height; ++j) {
                free(rooms[i].grid[j]);
            }
            free(rooms[i].grid);
        }
    }
    free(rooms);
}

void print_room_template(const RoomTemplate *room, size_t index) {
    if (!room) return;
    printf("[%zu] %s (%dx%d)\n", index, room->name, room->height, room->width);
}

bool print_all_room_templates(const char *filename) {
    RoomTemplate *rooms = NULL;
    size_t count = 0;

    if (!load_room_templates(filename, &rooms, &count))
        return false;

    for (size_t i = 0; i < count; ++i) {
        print_room_template(&rooms[i], i);
    }

    free_room_templates(rooms, count);
    return true;
}

// ==================== Floor Generation ====================

/// Create a room with given dimensions and initialize with empty space and walls
static void initialize_room(Room *room, int id, int height, int width) {
    room->id = id;
    room->height = height;
    room->width = width;
    room->grid = malloc(sizeof(char*) * height);
    
    for (int i = 0; i < height; ++i) {
        room->grid[i] = malloc(width);
        for (int j = 0; j < width; ++j) {
            room->grid[i][j] = ' ';
        }
    }

    // Add walls and doors
    // Top and bottom walls with middle doors
    for (int j = 0; j < width; ++j) {
        if (j == width / 2) {
            room->grid[0][j] = 'D';
            room->grid[height - 1][j] = 'D';
        } else {
            room->grid[0][j] = 'W';
            room->grid[height - 1][j] = 'W';
        }
    }

    // Left and right walls with middle doors
    for (int i = 1; i < height - 1; ++i) {
        if (i == height / 2) {
            room->grid[i][0] = 'D';
            room->grid[i][width - 1] = 'D';
        } else {
            room->grid[i][0] = 'W';
            room->grid[i][width - 1] = 'W';
        }
    }
}

/// Create a spawner room (empty, player starts in center)
static void create_spawner_room(Room *room, int id) {
    initialize_room(room, id, 9, 15);
    // Place player marker at center
    room->grid[4][7] = 'P'; // P for Player start position
}

// Fonction personnalisée (nmx + largeur variables)
void creer_salle_spawn_personnalisee(Room *room, int id, int height, int width) {
    initialize_room(room, id, height, width);
    room->grid[height / 2][width / 2] = 'P';
}

void creer_salle_normale_personnalisee(Room *room, int id, int height, int width) {
    initialize_room(room, id, height, width);
}

void creer_salle_objet_personnalisee(Room *room, int id, int height, int width, char item_code) {
    initialize_room(room, id, height, width);
    room->grid[height / 2][width / 2] = item_code;
}

void creer_salle_boss_personnalisee(Room *room, int id, int height, int width) {
    initialize_room(room, id, height, width);
    // boss avec porte en haut (par défaut) et centre vide
    room->grid[0][width / 2] = 'D';
    room->grid[height / 2][width / 2] = 'B';
}

/// Create a boss room (only entrance door, empty otherwise)
static void create_boss_room(Room *room, int id) {
    room->id = id;
    room->height = 9;
    room->width = 15;
    room->grid = malloc(sizeof(char*) * 9);

    for (int i = 0; i < 9; ++i) {
        room->grid[i] = malloc(15);
        for (int j = 0; j < 15; ++j) {
            room->grid[i][j] = ' ';
        }
    }

    // Top wall with single entrance
    for (int j = 0; j < 15; ++j) {
        if (j == 7) {
            room->grid[0][j] = 'D';
        } else {
            room->grid[0][j] = 'W';
        }
    }

    // Bottom, left, right walls (closed)
    for (int j = 0; j < 15; ++j) {
        room->grid[8][j] = 'W';
    }
    for (int i = 1; i < 8; ++i) {
        room->grid[i][0] = 'W';
        room->grid[i][14] = 'W';
    }
}

/// Create an item room (only entrance, item at center)
static void create_item_room(Room *room, int id, char item_code) {
    room->id = id;
    room->height = 9;
    room->width = 15;
    room->grid = malloc(sizeof(char*) * 9);

    for (int i = 0; i < 9; ++i) {
        room->grid[i] = malloc(15);
        for (int j = 0; j < 15; ++j) {
            room->grid[i][j] = ' ';
        }
    }

    // Top wall with single entrance
    for (int j = 0; j < 15; ++j) {
        if (j == 7) {
            room->grid[0][j] = 'D';
        } else {
            room->grid[0][j] = 'W';
        }
    }

    // Bottom, left, right walls (closed)
    for (int j = 0; j < 15; ++j) {
        room->grid[8][j] = 'W';
    }
    for (int i = 1; i < 8; ++i) {
        room->grid[i][0] = 'W';
        room->grid[i][14] = 'W';
    }

    // Place item at center
    room->grid[4][7] = item_code; // 'I' for item placeholder
}

/// Add monsters to a room
/// Randomly selects 0-6 monsters, at most 2 types
/// 10% chance for each monster to be a champion (2x HP and 2x DMG)
static void add_monsters_to_room(Room *room, int count,
                                 const Entity *monsters, size_t monster_count,
                                 int floor_num) {
    (void) monsters;
    (void) floor_num;

    if (monster_count == 0 || count == 0)
        return;

    // Pick up to 2 different monster types
    int type1 = rand() % monster_count;
    int type2 = type1;
    if (monster_count > 1 && rand() % 100 < 50) {
        type2 = rand() % monster_count;
    }

    int added = 0;
    int max_attempts = 50;
    int attempts = 0;

    while (added < count && attempts < max_attempts) {
        attempts++;

        // Random position in room (avoid walls and doors)
        int x = 1 + rand() % (room->width - 2);
        int y = 1 + rand() % (room->height - 2);

        // Check if position is free
        if (room->grid[y][x] == ' ') {
            // Decide which monster type
            int type = (rand() % 100 < 50) ? type1 : type2;
            
            // 10% chance for champion
            bool is_champion = rand() % 100 < 10;
            
            // Mark position with character indicating monster type and champion status
            // C or M for type1, c or m for type2
            char marker;
            if (type == type1)
                marker = is_champion ? 'C' : 'M';
            else
                marker = is_champion ? 'c' : 'm';

            room->grid[y][x] = marker;
            
            // In a real implementation, you'd store the actual monster entity data
            // For now, we're just marking the position
            added++;
        }
    }
}

bool generate_floor(int floor_num,
                   const RoomTemplate *room_templates, size_t template_count,
                   const char *monsters_file,
                   const char *items_file,
                   Room **outRooms, size_t *outCount) {
    if (!outRooms || !outCount)
        return false;

    *outRooms = NULL;
    *outCount = 0;

    // Load monsters and items
    Entity *monsters = NULL;
    size_t monster_count = 0;
    Item *items = NULL;
    size_t item_count = 0;

    if (!load_entities_from_file(monsters_file, &monsters, &monster_count)) {
        return false;
    }

    if (!load_items_from_file(items_file, &items, &item_count)) {
        free_entities(monsters);
        return false;
    }

    // Allocate room array (14 rooms total)
    const size_t total_rooms = 14;
    Room *rooms = malloc(sizeof(Room) * total_rooms);
    if (!rooms) {
        free_entities(monsters);
        free_items(items);
        return false;
    }

    srand((unsigned int)time(NULL) + floor_num);

    int room_id = 0;

    // 1. Create spawner room (room 0)
    create_spawner_room(&rooms[room_id], floor_num * 100 + room_id);
    room_id++;

    // 2. Create 10 normal rooms with random monsters
    for (int i = 0; i < 10; ++i) {
        int template_idx = (template_count > 0) ? rand() % template_count : 0;
        
        if (template_count > 0 && room_templates) {
            initialize_room(&rooms[room_id], floor_num * 100 + room_id,
                           room_templates[template_idx].height,
                           room_templates[template_idx].width);
        } else {
            initialize_room(&rooms[room_id], floor_num * 100 + room_id, 9, 15);
        }

        // Add 0-6 random monsters
        int monster_count_in_room = rand() % 7; // 0-6
        if (monsters) {
            add_monsters_to_room(&rooms[room_id], monster_count_in_room, 
                               monsters, monster_count, floor_num);
        }

        room_id++;
    }

    // 3. Create item room
    char item_code = 'I';
    if (item_count > 0) {
        // In a real implementation, pick a random item
        // For now, we'll just use a placeholder
        create_item_room(&rooms[room_id], floor_num * 100 + room_id, item_code);
    } else {
        initialize_room(&rooms[room_id], floor_num * 100 + room_id, 9, 15);
    }
    room_id++;

    // 4. Create boss room
    create_boss_room(&rooms[room_id], floor_num * 100 + room_id);
    room_id++;

    // 5. Create bonus item room
    if (item_count > 0) {
        create_item_room(&rooms[room_id], floor_num * 100 + room_id, item_code);
    } else {
        initialize_room(&rooms[room_id], floor_num * 100 + room_id, 9, 15);
    }
    room_id++;

    *outRooms = rooms;
    *outCount = total_rooms;

    free_entities(monsters);
    free_items(items);

    return true;
}

void liberer_salles(Room *rooms, size_t count) {
    if (!rooms) return;
    for (size_t i = 0; i < count; ++i) {
        if (rooms[i].grid) {
            for (int j = 0; j < rooms[i].height; ++j) {
                free(rooms[i].grid[j]);
            }
            free(rooms[i].grid);
        }
    }
    free(rooms);
}
