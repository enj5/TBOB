#include "crud1_structs.h"
#include <stdio.h>

#ifndef CRUD1_SALLE_H

#define CRUD1_SALLE_H


void creer(Room*, int, int, int);
void resize(Room*, int, int);
void modif(Room*, int);
void DelInFile(int, int);
void show(Room);
void WriteInFile(Room*, FILE*);
void ReadInFile(Room*, FILE*);
void freeR(Room*);

#endif /* CRUD1_SALLE_H */