#include "monsters.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_newline(char *s)
{
    if (!s)
    {
        return;
    }
    size_t len = strlen(s);
    if (len == 0)
    {
        return;
    }
    if (s[len - 1] == '\n')
    {
        s[len - 1] = '\0';
    }
    if (len > 1 && s[len - 2] == '\r')
    {
        s[len - 2] = '\0';
    }
}

static char *trim_whitespace(char *s)
{
    if (!s)
    {
        return s;
    }
    while (*s && isspace((unsigned char)*s))
    {
        s++;
    }
    if (*s == '\0')
    {
        return s;
    }
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
    {
        *end-- = '\0';
    }
    return s;
}

static bool parse_line_kv(char *line, char **outKey, char **outValue)
{
    char *sep = strchr(line, '=');
    if (!sep)
    {
        return false;
    }
    *sep = '\0';
    *outKey = trim_whitespace(line);
    *outValue = trim_whitespace(sep + 1);
    return true;
}

static void write_entity_block(FILE *f, const Entity *entity)
{
    fprintf(f, "---\n");
    if (entity->name[0] != '\0')
    {
        fprintf(f, "name=%s\n", entity->name);
    }
    if (entity->hpMax != 0.0f)
    {
        fprintf(f, "hpMax=%.2f\n", entity->hpMax);
    }
    if (entity->shoot)
    {
        fprintf(f, "shoot=1\n");
    }
    if (entity->ss)
    {
        fprintf(f, "ss=1\n");
    }
    if (entity->flight)
    {
        fprintf(f, "flight=1\n");
    }
}

bool overwrite_entities_file(const char *filename, const Entity *entities, size_t count)
{
    if (!filename)
    {
        return false;
    }
    FILE *f = fopen(filename, "w");
    if (!f)
    {
        return false;
    }

    fprintf(f, "{%zu}\n", count);
    for (size_t i = 0; i < count; ++i)
    {
        write_entity_block(f, &entities[i]);
    }

    fclose(f);
    return true;
}

bool load_entities_from_file(const char *filename, Entity **outEntities, size_t *outCount)
{
    if (!outEntities || !outCount)
    {
        return false;
    }

    *outEntities = NULL;
    *outCount = 0;

    FILE *f = fopen(filename, "r");
    if (!f)
    {
        return true; // Missing file = empty list
    }

    Entity *entities = NULL;
    size_t cap = 0;

    char line[256];
    Entity current;
    bool inBlock = false;

    while (fgets(line, sizeof(line), f))
    {
        trim_newline(line);
        char *cursor = trim_whitespace(line);
        if (*cursor == '\0')
        {
            continue;
        }

        if (*cursor == '{')
        {
            // count line like {2} - ignore for parsing
            continue;
        }

        if (strcmp(cursor, "---") == 0)
        {
            if (inBlock)
            {
                // finish previous block
                if (*outCount >= cap)
                {
                    size_t newCap = cap ? cap * 2 : 8;
                    Entity *tmp = realloc(entities, newCap * sizeof(Entity));
                    if (!tmp)
                    {
                        free(entities);
                        fclose(f);
                        *outEntities = NULL;
                        *outCount = 0;
                        return false;
                    }
                    entities = tmp;
                    cap = newCap;
                }
                entities[*outCount] = current;
                (*outCount)++;
            }
            // start a new block
            inBlock = true;
            memset(&current, 0, sizeof(current));
            continue;
        }

        if (!inBlock)
        {
            continue;
        }

        char *key = NULL;
        char *value = NULL;
        if (!parse_line_kv(cursor, &key, &value))
        {
            continue;
        }

        if (strcmp(key, "name") == 0)
        {
            strncpy(current.name, value, sizeof(current.name) - 1);
            current.name[sizeof(current.name) - 1] = '\0';
        }
        else if (strcmp(key, "hpMax") == 0)
        {
            current.hpMax = (float)strtof(value, NULL);
        }
        else if (strcmp(key, "dmg") == 0)
        {
            current.dmg = (float)strtof(value, NULL);
        }
        else if (strcmp(key, "x") == 0)
        {
            current.x = (int)strtol(value, NULL, 10);
        }
        else if (strcmp(key, "y") == 0)
        {
            current.y = (int)strtol(value, NULL, 10);
        }
        else if (strcmp(key, "shoot") == 0)
        {
            current.shoot = (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0);
        }
        else if (strcmp(key, "ss") == 0)
        {
            current.ss = (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0);
        }
        else if (strcmp(key, "flight") == 0)
        {
            current.flight = (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0);
        }
    }

    // If last block didn't end with '---', add it.
    if (inBlock)
    {
        if (*outCount >= cap)
        {
            size_t newCap = cap ? cap * 2 : 8;
            Entity *tmp = realloc(entities, newCap * sizeof(Entity));
            if (!tmp)
            {
                free(entities);
                fclose(f);
                *outEntities = NULL;
                *outCount = 0;
                return false;
            }
            entities = tmp;
            cap = newCap;
        }
        entities[*outCount] = current;
        (*outCount)++;
    }

    fclose(f);
    *outEntities = entities;
    return true;
}

bool append_entity_to_file(const char *filename, const Entity *entity)
{
    if (!filename || !entity)
    {
        return false;
    }

    Entity *entities = NULL;
    size_t count = 0;
    if (!load_entities_from_file(filename, &entities, &count))
    {
        return false;
    }

    Entity *tmp = realloc(entities, (count + 1) * sizeof(Entity));
    if (!tmp)
    {
        free(entities);
        return false;
    }
    entities = tmp;
    entities[count] = *entity;
    count++;

    bool ok = overwrite_entities_file(filename, entities, count);
    free(entities);
    return ok;
}

void free_entities(Entity *entities) {
    free(entities);
}

void print_entity(const Entity *entity, size_t index) {
    if (!entity) return;
    printf("[%zu] %s\n", index, entity->name[0] != '\0' ? entity->name : "Entity");
    printf("    hpMax: %.2f\n", entity->hpMax);
    //printf("    dmg: %.2f\n", entity->dmg);
    //printf("    x: %d, y: %d\n", entity->x, entity->y);
    printf("    shoot: %s \n    ss: %s \n    flight: %s\n",
           entity->shoot ? "yes" : "no",
           entity->ss ? "yes" : "no",
           entity->flight ? "yes" : "no");
}

bool print_all_entities(const char *filename) {
    Entity *entities = NULL;
    size_t count = 0;
    if (!load_entities_from_file(filename, &entities, &count))
        return false;

    if (count == 0) {
        printf("No entities found (file: %s)\n", filename);
        free_entities(entities);
        return true;
    }

    for (size_t i = 0; i < count; ++i) {
        print_entity(&entities[i], i);
    }

    free_entities(entities);
    return true;
}

bool update_entity_in_file(const char *filename, size_t index, const Entity *newEntity) {
    if (!newEntity)
        return false;

    Entity *entities = NULL;
    size_t count = 0;
    if (!load_entities_from_file(filename, &entities, &count))
        return false;

    if (index >= count) {
        free_entities(entities);
        return false;
    }

    entities[index] = *newEntity;
    bool ok = overwrite_entities_file(filename, entities, count);
    free_entities(entities);
    return ok;
}

bool delete_entity_in_file(const char *filename, size_t index) {
    Entity *entities = NULL;
    size_t count = 0;
    if (!load_entities_from_file(filename, &entities, &count))
        return false;

    if (index >= count) {
        free_entities(entities);
        return false;
    }

    for (size_t i = index + 1; i < count; ++i) {
        entities[i - 1] = entities[i];
    }
    --count;

    bool ok = overwrite_entities_file(filename, entities, count);
    free_entities(entities);
    return ok;
}
