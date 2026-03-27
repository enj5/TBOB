#include "crud1_salle.h"
#include "crud1_structs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//Getter and setter


void creer(Room *R, int id, int n, int m){
    
    int i, j;
    R->id = id;
    R->height = n;
    R->width = m;
    R->grid = malloc(sizeof(char*) * n);
    //Allocation de l'espace et Initialisation 
    //de la salle à des espaces vides partout
    for(i = 0; i < n; i += 1){
        R->grid[i] = malloc(m);
        for(j = 0; j < m; j += 1){
            R->grid[i][j] = ' ';
        }
    }
    
    //Murs et portes
        //M_H et M_B
        
    for(j = 0; j < m; j += 1){
        if(j == m/2){
            R->grid[0][j] = 'D';
            R->grid[n-1][j] = 'D';
        }/*
        else if(j == m/2+1){
            R->grid[0][j] = 'W';
            R->grid[n-1][j] = 'D';
        }*/
        else{
            R->grid[0][j] = 'W';
            R->grid[n-1][j] = 'W';
        }
    }

        //M_D et M_G
    for(i = 1; i < n-1; i += 1){
        if(i == n/2){
            R->grid[i][0] = 'D';
            R->grid[i][m-1] = 'D';
        }
        else{
            R->grid[i][0] = 'W';
            R->grid[i][m-1] = 'W';
        }
    }
    /*
    //Diagonale_1
    R->grid[1][1] = 'H'; R->grid[1][2] = 'G'; R->grid[2][2] = 'G'; R->grid[2][1] = 'G'; 
    R->grid[n-2][m-2] = 'H'; R->grid[n-2][m-3] = 'G'; R->grid[n-3][m-3] = 'G'; R->grid[n-3][m-2] = 'G'; 
    //Diagonale_2
    R->grid[n-3][1] = 'G'; R->grid[n-3][2] = 'G'; R->grid[n-2][2] = 'G';
    R->grid[2][m-2] = 'G'; R->grid[2][m-3] = 'G'; R->grid[1][m-3] = 'G';
    */


    //Ajout d'éléments dans la salle

    
}

void resize(Room *R, int n, int m){

    char** T;
    int i, j;
    R->height = n;
    R->width = m;

    T = malloc(sizeof(char*) * n);
    //Allocation de l'espace et Initialisation 
    //de la salle à des espaces vides partout
    for(i = 0; i < n; i += 1){
        T[i] = malloc(m);
        for(j = 0; j < m; j += 1){
            T[i][j] = R->grid[i][j];
        }
    }

    // A IMPLEMENTER !!! TAILLE DES ROOMS VARIABLE

}

void modif(Room* R, int id){

    //Salles modifiables par l'user

    bool running = true;
    char choice, tmp, ME, touche;
    int i, j, X, Y, nextX, nextY;

    printf("Modifications possibles : \n");
    printf("Pour ajouter, appuyer : G, R, S ou H\n");
    printf("Pour supprimer, appuyer : E\n\n");

    printf("Un caractere 'M' ou 'E' va apparaitre dans la room,\n");
    printf("deplacer le a l'aide des directions indiquees,\n");
    printf("appuyer sur entrer apprès avoir positionne\n");
    printf("le curseur au bon endroit.\n\n");
    do{
        printf("Votre choix (G, R, S, H ou E): ");
        scanf(" %c", &choice);  
    }while(choice != 'G' && choice != 'R' && choice != 'S' && choice != 'H' && choice != 'E');

    

    if(choice == 'E'){
        ME = 'E';
    }
    else
        ME = 'M';

    /*
    for(i= 1; i < R->height-1 ; i++){
        for(j = 1; j < R->width-1; j++){
            if(R->grid[i][j] == ' '){
                R->grid[i][j] = ME;
                //X = i; Y = j;
                break;
            }
        }
        break;
    }*/
    //R->grid[R->height/2][R->width/2] = ME;

    X = R->width/2;
    Y = R->height/2; 
    tmp = R->grid[Y][X];
    

    while (running) {

        
        R->grid[Y][X] = ME;

        nextX = X;
        nextY = Y;

        printf("\n");
        show(*R);
        
        
        printf("\n");
        printf("\n");
        printf("Deplacements [Gauche (Q); Droite (D); Haut (Z); Bas (S)] | Fin (X): ");
        scanf(" %c", &touche); 
        //int touche = getch();
        if(touche != 'X'){
            if (touche == 'Z'){ 
                nextY--;
            }
            if (touche == 'S') 
                nextY++;
            if (touche == 'Q') 
                nextX--;
            if (touche == 'D') 
                nextX++;
        
        
        // Collision : on ne bouge que si la case n'est pas un Mur ou un Rocher
            if (R->grid[nextY][nextX] != 'W' && R->grid[nextY][nextX] != 'D') {
                R->grid[Y][X] = tmp;
                X = nextX;
                Y = nextY;
                tmp = R->grid[Y][X];
                //R->grid[Y][X] = ' ';
            }
        }
        else
            {
                if(choice == 'E')
                    R->grid[Y][X] = ' ';
                else
                    R->grid[Y][X] = choice;
                running = false;
            }
        
    }
    show(*R);

}


//Delete is In progress... ça ne marche pas encore!

void DelInFile(int id, int nS){
    

    FILE* fich = fopen("salles.rtbob", "r");
    int i;
    Room tab_Room[nS];
    //Room tab_Room2[nS-1];

    //LECTURE
    if(fich != NULL){
        fscanf(fich, "{%d}\n", &nS); //Lecture nbre de salles
        for(i = 0; i < nS; i++){
            ReadInFile(&tab_Room[i], fich);
        }

        fclose(fich);
    }
    //REECRITURE
    FILE* f = fopen("salles.rtbob", "w+");
    if(f != NULL){
        fprintf(f, "{%d}\n", nS-1);
        for(i = 0; i < id-1; i++){    
            WriteInFile(&tab_Room[i], f);
        }
        for(i = id; i < nS; i++){
            tab_Room[i].id--;     
            WriteInFile(&tab_Room[i], f);
        }
        fclose(f);
    }

    

}


void show(Room R){
    
    system("cls");
    int i, j;
    for(i = 0; i < R.height; i += 1){
        for(j = 0; j < R.width; j += 1){
            printf("%c ", R.grid[i][j]);
        }
        printf("\n");
    }
}


void WriteInFile(Room *R, FILE* f){
    
    //FILE* f = fopen("salles.rtbob", "r");
    int i, j;
    //fprintf(f, "\n{%d}", nR);
    fprintf(f, "[%d|%d]%d", R->height, R->width, R->id);
    fputs("\n", f);
    for(i = 0; i < R->height; i += 1){
        for(j = 0; j < R->width; j += 1){
            fprintf(f, "%c ", R->grid[i][j]);
        }
        fprintf(f, "\n");
    }
}


void ReadInFile(Room *R, FILE* fich){
    
    //FILE* fich = fopen("salles.rtbob", "r");

    int i, k, j, s, n, m, nS;
    
    //if(fich != NULL){ //Vérification si le fichier est bien lu
        //fscanf(fich, "{%d}\n", &nS); //Lecture nbre de salles
        //for(s = 0; s < 3; s++){
            
            fscanf(fich, "[%d|%d]%d\n", &n, &m, &i); //Récupération des caract. des salles
            //Affectation des caract. aux salles
            R->id = i;
            R->height = n;
            R->width = m;

            //Allocation de l'espace (car la matrice est un pointeur)
            R->grid = malloc(sizeof(char*) * n);
            for(k = 0; k < n; k += 1){
                R->grid[k] = malloc(m);
                for(j = 0; j < m; j += 1){
                    R->grid[k][j] = ' ';
                }
            }

            //Lecture et stockage des caractères dans ma matrice
            //printf("[%d|%d]%d\n", R[s].height, R[s].width, R[s].id);
            for(k = 0; k < n; k++){
                for(j = 0; j < m; j++){
                    fscanf(fich, "%c", &R->grid[k][j]); 
                    fseek(fich, 1, SEEK_CUR);
                    //printf("%c ", R->grid[k][j]);
                }    
                fscanf(fich, "\n");
                //printf("\n");
            }

            //Affichage de la matrice créée
            //printf("[%d|%d]%d\n", R->height, R->width, R->id);
            //show(&R);
            
        //}
    //    fclose(fich);
    //}
    
}


void freeR(Room *R){
    int i;
    for(i=0; i < R->height; i++)
        free(R->grid[i]);
    free(R->grid);
}