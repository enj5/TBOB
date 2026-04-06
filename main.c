#include "creative_mode.h"
#include "play_mode.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

static void trim_newline(char *s)
{
    if (!s) return;
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n')
        s[len - 1] = '\0';
}

static bool read_int(const char *prompt, int *out, int min, int max)
{
    char buffer[128];
    long val;
    char *end;

    if (!prompt || !out || min > max) return false;

    while (true) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) return false;
        trim_newline(buffer);

        if (buffer[0] == '\0') {
            printf("Entrée vide. Réessayez.\n");
            continue;
        }

        val = strtol(buffer, &end, 10);
        if (end == buffer || *end != '\0') {
            printf("Entrée invalide. Entrez un entier.\n");
            continue;
        }

        if (val < min || val > max) {
            printf("Hors de portée (%d-%d).\n", min, max);
            continue;
        }

        *out = (int)val;
        return true;
    }
}


int main(void)
{
    while (true) {
        printf("=== TBOB MENU ===\n");
        printf("1) Creative mode\n");
        printf("2) Play mode\n");
        printf("0) Quitter\n");

        int choix = 0;
        if (!read_int("Choix : ", &choix, 0, 2)) {
            printf("Erreur de lecture.\n");
            continue;
        }

        switch (choix) {
            case 1:
                creative_mode();
                break;
            case 2:
                play_mode();
                break;
            case 0:
                printf("Au revoir !\n");
                return 0;
            default:
                printf("Choix invalide.\n");
                break;
        }

        printf("\n--- Retour au menu principal ---\n\n");
    }
}
