#ifndef MONSTERS_H
#define MONSTERS_H

#include <stdbool.h>
#include <stddef.h>
#include "structs.h"

/// Default filename used to store entities (monsters).
#define MONSTERS_FILE_NAME "monstres.mtbob"

/// Append an entity to the monsters file. Creates the file if it doesn't exist.
bool append_entity_to_file(const char *filename, const Entity *entity);

/// Load all entities from the file. The caller must free the returned array via free_entities().
/// Returns true on success; on failure *outEntities will be set to NULL and *outCount to 0.
bool load_entities_from_file(const char *filename, Entity **outEntities, size_t *outCount);

/// Write all entities to the file (overwrites existing file).
bool overwrite_entities_file(const char *filename, const Entity *entities, size_t count);

/// Free an array returned by load_entities_from_file().
void free_entities(Entity *entities);

/// Print a single entity to stdout.
void print_entity(const Entity *entity, size_t index);

/// Print all entities stored in the file.
/// Returns true on success (file exists and was read), false otherwise.
bool print_all_entities(const char *filename);

/// Update the entity at the given 0-based index in the file.
/// Returns true if the index was valid and file was updated.
bool update_entity_in_file(const char *filename, size_t index, const Entity *newEntity);

/// Delete the entity at the given 0-based index in the file.
/// Returns true if the index was valid and file was updated.
bool delete_entity_in_file(const char *filename, size_t index);

#endif // MONSTERS_H
