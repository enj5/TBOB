#include "items.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void write_item_block(FILE *f, const Item *item) {
    fprintf(f, "---\n");
    if (item->name[0] != '\0')
        fprintf(f, "name=%s\n", item->name);
    if (item->hpMax != 0.0f)
        fprintf(f, "hpMax=%.2f\n", item->hpMax);
    if (item->shield != 0)
        fprintf(f, "shield=%d\n", item->shield);
    if (item->dmg != 0.0f)
        fprintf(f, "dmg=%.2f\n", item->dmg);
    if (item->ps)
        fprintf(f, "ps=1\n");
    if (item->ss)
        fprintf(f, "ss=1\n");
    if (item->flight)
        fprintf(f, "flight=1\n");
}

bool overwrite_items_file(const char *filename, const Item *items, size_t count) {
    if (!filename)
        return false;
    FILE *f = fopen(filename, "w");
    if (!f)
        return false;

    fprintf(f, "{%zu}\n", count);
    for (size_t i = 0; i < count; ++i) {
        write_item_block(f, &items[i]);
    }

    fclose(f);
    return true;
}

bool load_items_from_file(const char *filename, Item **outItems, size_t *outCount) {
    if (!outItems || !outCount)
        return false;

    *outItems = NULL;
    *outCount = 0;

    FILE *f = fopen(filename, "r");
    if (!f)
        return true; // Missing file = empty list

    Item *items = NULL;
    size_t cap = 0;

    char line[256];
    Item current;
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
                    size_t newCap = cap ? cap * 2 : 8;
                    Item *tmp = realloc(items, newCap * sizeof(Item));
                    if (!tmp) {
                        free(items);
                        fclose(f);
                        *outItems = NULL;
                        *outCount = 0;
                        return false;
                    }
                    items = tmp;
                    cap = newCap;
                }
                items[*outCount] = current;
                (*outCount)++;
            }
            // start a new block
            inBlock = true;
            memset(&current, 0, sizeof(current));
            continue;
        }

        if (!inBlock)
            continue;

        char *key = NULL;
        char *value = NULL;
        if (!parse_line_kv(cursor, &key, &value))
            continue;

        if (strcmp(key, "name") == 0) {
            strncpy(current.name, value, sizeof(current.name) - 1);
            current.name[sizeof(current.name) - 1] = '\0';
        } else if (strcmp(key, "hpMax") == 0) {
            current.hpMax = (float)strtof(value, NULL);
        } else if (strcmp(key, "shield") == 0) {
            current.shield = (int)strtol(value, NULL, 10);
        } else if (strcmp(key, "dmg") == 0) {
            current.dmg = (float)strtof(value, NULL);
        } else if (strcmp(key, "ps") == 0) {
            current.ps = (bool)atoi(value);
        } else if (strcmp(key, "ss") == 0) {
            current.ss = (bool)atoi(value);
        } else if (strcmp(key, "flight") == 0) {
            current.flight = (bool)atoi(value);
        }
    }

    // If last block didn't end with '---', add it.
    if (inBlock) {
        if (*outCount >= cap) {
            size_t newCap = cap ? cap * 2 : 8;
            Item *tmp = realloc(items, newCap * sizeof(Item));
            if (!tmp) {
                free(items);
                fclose(f);
                *outItems = NULL;
                *outCount = 0;
                return false;
            }
            items = tmp;
            cap = newCap;
        }
        items[*outCount] = current;
        (*outCount)++;
    }

    fclose(f);
    *outItems = items;
    return true;
}

bool append_item_to_file(const char *filename, const Item *item) {
    if (!filename || !item)
        return false;

    Item *items = NULL;
    size_t count = 0;
    if (!load_items_from_file(filename, &items, &count))
        return false;

    Item *tmp = realloc(items, (count + 1) * sizeof(Item));
    if (!tmp) {
        free(items);
        return false;
    }
    items = tmp;
    items[count] = *item;
    count++;

    bool ok = overwrite_items_file(filename, items, count);
    free(items);
    return ok;
}

void free_items(Item *items) {
    free(items);
}

void print_item(const Item *item, size_t index) {
    if (!item) return;
    printf("[%zu] %s\n", index, item->name);
    printf("    hpMax: %.2f, shield: %d, dmg: %.2f\n", item->hpMax, item->shield, item->dmg);
    printf("    ps: %s, ss: %s, flight: %s\n",
           item->ps ? "yes" : "no",
           item->ss ? "yes" : "no",
           item->flight ? "yes" : "no");
}

bool print_all_items(const char *filename) {
    Item *items = NULL;
    size_t count = 0;
    if (!load_items_from_file(filename, &items, &count))
        return false;

    if (count == 0) {
        printf("No items found (file: %s)\n", filename);
        free_items(items);
        return true;
    }

    for (size_t i = 0; i < count; ++i) {
        print_item(&items[i], i);
    }

    free_items(items);
    return true;
}

bool update_item_in_file(const char *filename, size_t index, const Item *newItem) {
    if (!newItem)
        return false;

    Item *items = NULL;
    size_t count = 0;
    if (!load_items_from_file(filename, &items, &count))
        return false;

    if (index >= count) {
        free_items(items);
        return false;
    }

    items[index] = *newItem;
    bool ok = overwrite_items_file(filename, items, count);
    free_items(items);
    return ok;
}

bool delete_item_in_file(const char *filename, size_t index) {
    Item *items = NULL;
    size_t count = 0;
    if (!load_items_from_file(filename, &items, &count))
        return false;

    if (index >= count) {
        free_items(items);
        return false;
    }

    for (size_t i = index + 1; i < count; ++i) {
        items[i - 1] = items[i];
    }
    --count;

    bool ok = overwrite_items_file(filename, items, count);
    free_items(items);
    return ok;
}
