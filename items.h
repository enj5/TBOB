#ifndef ITEMS_H
#define ITEMS_H

#include <stdbool.h>
#include <stddef.h>
#include "structs.h"

/// Default filename used to store items.
#define ITEMS_FILE_NAME "items.itbob"

/// Append an item to the items file. Creates the file if it doesn't exist.
bool append_item_to_file(const char *filename, const Item *item);

/// Load all items from the file. The caller must free the returned array via free_items().
/// Returns true on success; on failure *outItems will be set to NULL and *outCount to 0.
bool load_items_from_file(const char *filename, Item **outItems, size_t *outCount);

/// Write all items to the file (overwrites existing file).
bool overwrite_items_file(const char *filename, const Item *items, size_t count);

/// Free an array returned by load_items_from_file().
void free_items(Item *items);

/// Print a single item to stdout.
void print_item(const Item *item, size_t index);

/// Print all items stored in the file.
/// Returns true on success (file exists and was read), false otherwise.
bool print_all_items(const char *filename);

/// Update the item at the given 0-based index in the file.
/// Returns true if the index was valid and file was updated.
bool update_item_in_file(const char *filename, size_t index, const Item *newItem);

/// Delete the item at the given 0-based index in the file.
/// Returns true if the index was valid and file was updated.
bool delete_item_in_file(const char *filename, size_t index);

#endif // ITEMS_H
