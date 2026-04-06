#include "crud1_salle.h"
#include "crud1_structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h> // Nécessaire pour getch() sur Windows


int main(int argc, char const *argv[])
{
    FILE* fich = fopen("salles.rtbob", "r");
    //Variables génériques
    int i, j, n = 9, m = 15, nS = 10;
    char choix;
    Room tab_Room[14];

    //Title
    printf("CRUD des pieces du donjon\n\n");


    //Prédéfinition des salles

    Room spawner, itRoom, bRoom, itRoomB; //Salles spéciales
    Room R1, R2, R3, R4, R5, R6, R7, R8, R9, R10; //10 salles
    //tab_Room[14] = {spawner, itRoom, bRoom, itRoomB, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10}; 
    
    tab_Room[0] = R1;   tab_Room[5] = R6;   tab_Room[10] = spawner;
    tab_Room[1] = R2;   tab_Room[6] = R7;   tab_Room[11] = itRoom;
    tab_Room[2] = R3;   tab_Room[7] = R8;   tab_Room[12] = bRoom;
    tab_Room[3] = R4;   tab_Room[8] = R9;   tab_Room[13] = itRoomB;
    tab_Room[4] = R5;   tab_Room[9] = R10;
    
    
    //TEST LECTURE DANS UN FICHIER
    /*
    if(fich != NULL){
        fscanf(fich, "{%d}\n", &nS); //Lecture nbre de salles
        //printf("{%d}\n", nS);
        for(i = 0; i < nS; i++){
            ReadInFile(&tab_Room[i], fich);
            //printf("[%d|%d]%d\n", tab_Room[i].height, tab_Room[i].width, tab_Room[i].id);
            //show(tab_Room[i]);
        }

        fclose(fich);
    }*/

    //TEST SUPPRESSION DANS UN FICHIER
    DelInFile(4, nS);
    


    /*
    FILE* f = fopen("salles.rtbob", "w+");
    //TEST ECRITURE DANS UN FICHIER
    if(f != NULL){
        fprintf(f, "{%d}\n", nS);
        for(i = 0; i < nS; i++){
            creer(&tab_Room[i], i+1, n, m);    
            //printf("[%d|%d]%d\n", tab_Room[i].height, tab_Room[i].width, tab_Room[i].id);
            WriteInFile(&tab_Room[i], f);
        }
        fclose(f);
    }*/
    

    /*
    char on;
    int num;
    //MODIFICATION DES OBSTACLES
    do{
        printf("Voulez-vous modifier une salle? (o/n) : ");
        scanf(" %c", &on);
        printf("\n");
    }while((on != 'o' && on != 'n'));

    if(on == 'o'){
        do{
            printf("\n");
            printf("Entrer le numero de la salle\n");
            printf("[1, 10] : ");
            scanf(" %d", &num);
        }while(num != 1 && num != 2 && num != 3 && num != 4 && num != 5 && num != 6 && num != 7 && num != 8 && num != 9 && num != 10 );
        
        modif(&tab_Room[num-1], num);
        show(tab_Room[num-1]);

    
    }*/
    
    


    

    
    for(i = 0; i < nS; i++)
        freeR(&tab_Room[i]);
    
    
    return 0;
}
