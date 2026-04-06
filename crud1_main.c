#include "crud1_salle.h"
#include "crud1_structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h> // Nécessaire pour getch() sur Windows


int main(int argc, char const *argv[])
{

    //Variables génériques
    int i, nS;
    char menu, keep, choix;
    Room tab_Room[26]; //Tableau des salles
    Room spawner, itRoom, bRoom, itRoomB; //Salles spéciales
    

    //Boucle unique des actions du menu
    do
    {
        system("cls");

        //Title
        printf("\n--- CRUD des pieces du donjon---\n\n");
        
        //Affichage du menu
        printf("Menu :\n");
        printf("(10 salles par defaut sont existantes)\n");
        printf("(Tapez le numero correspondant a l'action de votre choix)\n\n");
        printf("1- Afficher les salles\n");
        printf("2- Creer une nouvelle salle\n"); //Juste créer une salle vide nxm avec un id = nS+1. Les ajouts d'obstacles se feront dans le 3.Modifier
        printf("3- Modifier une salle\n");
        printf("4- Supprimer une salle\n");
        printf("5- Quitter\n");

        //Choix de l'option
        do
        {
            printf("\nEntrer votre choix (1|2|3|4|5) : ");
            scanf(" %c", &menu);
        } while(menu != '1' && menu != '2' && menu != '3' && menu != '4' && menu != '5');
        printf("\n\n");

        //Chargement initial du fichier dans le tableau des salles
        FILE* fich = fopen("salles.rtbob", "r"); // Ouverture du fichier en lecture 
            //Chargement du contenu dans le tableau des salles
        if(fich != NULL){
            fscanf(fich, "{%d}\n", &nS); //Lecture nbre de salles
            for(i = 0; i < nS; i++){
                ReadInFile(&tab_Room[i], fich); //Lecture de chaque salle
            }
            //fclose(fich);
        }
    
        switch(menu)
        {
            case '1':
                //FILE* fich = fopen("salles.rtbob", "r"); // Ouverture du fichier en lecture
                printf("\n");
                printf("{%d}\n", nS);
                for(i = 0; i < nS; i++){
                    printf("[%d|%d]%d\n", tab_Room[i].height, tab_Room[i].width, tab_Room[i].id);
                    show(tab_Room[i]); //Affichage de la salle dans la console
                }
                //fclose(fich);
                //Liberation de l'espace alloue
                for(i = 0; i < nS; i++)
                    freeR(&tab_Room[i]);
                
                break;
            case '2':
                char keep_create;
                Room New_Room;
                do
                {
                    system("cls");
                    nS = nS + 1;
                    creer(&tab_Room[nS-1], nS, 9, 15); //Creation d'une salle 9x15 vide
                    fclose(fich);
                    fich = fopen("salles.rtbob", "w+");   
                    fprintf(fich, "{%d}\n", nS);
                    for(i = 0; i < nS; i++){    
                        WriteInFile(&tab_Room[i], fich);
                    }
                    fclose(fich);
                    printf("\nSALLE AJOUTEE AVEC SUCCES!");
                    printf("\nAjouter une nouvelle salle ?");
                    do
                    {
                        printf("(o/n) : ");
                        scanf(" %c", &keep_create);
                    } while (keep_create != 'o' && keep_create != 'n');
                } while (keep_create == 'o');
                
                break;
            case '3':
                char on, keep_up;
                int num;
                //MODIFICATION DES OBSTACLES
                printf("\n");
                do
                {
                    system("cls");
                    printf("Entrer le numero de la salle");
                    do{
                        printf("\n[1, %d] : ", nS); //Restriction au nombre de salle existant
                        scanf(" %d", &num);
                        /*
                        while(scanf(" %d", &num) == 0){
                            printf("\n[1, %d] : ", nS);
                            while(getchar() != '\n') //vider le buffer
                                printf("\n[1, %d] : ", nS);
                        }
                        */  //TENTATIVE D'IMPOSER L'ENTRER D'UNE VALEUR ENTIERE! A REVOIR
                    }while(num < 1 || num > nS);

                    //Modification de la salle
                    modif(&tab_Room[num-1], num);
                    
                    //Reecriture dans le fichier
                    //rewind(fich); //Retour au début du fichier
                    fclose(fich);//Fermeture pour vider le fichier avant de reecrire 
                    fich = fopen("salles.rtbob", "w+");   
                    fprintf(fich, "{%d}\n", nS);
                    for(i = 0; i < nS; i++){    
                        WriteInFile(&tab_Room[i], fich);
                    }
                    fclose(fich); 
                    do
                    {
                        printf("\nModifier une autre salle? (o/n) : ");
                        scanf(" %c", &keep_up);
                    } while (keep_up != 'o' && keep_up != 'n');    
                } while (keep_up == 'o');
                break;
            case '4':
                int del;
                char keep_del;
                fclose(fich);
                do
                {
                    system("cls");
                    //Cas Aucune salle dans le fichier
                    if(nS == 0){
                        printf("Aucune salles existante!"); 
                        keep_del = 'n';
                    }
                    //Il y a au moins une salle
                    else{
                        printf("Entrez le numero de la salle a supprimer");
                        //Conditionnement des valeurs d'entree
                        do
                        {
                            printf("[1, %d] : ", nS);
                            scanf(" %d", &del);
                        } while (del < 1 || del > nS); //Pas frocément optimal - à revoir
                        
                        DelInFile(del, nS); //Suppression et reecriture dans le fichier
                        printf("Voulez-vous supprimer une autre salle?\n");
                        do
                        {
                            printf("(o/n) : ");
                            scanf(" %c", &keep_del);
                        } while (keep_del != 'o' && keep_del != 'n');
                    }
                } while (keep_del == 'o');
                
                
                break;
            case '5':
                system("cls");
                printf("\nVous allez quitter le menu.\n");
                break;
            default:
                printf("ERREUR DE SAISIE!");
                break;
        }
        fclose(fich);
        do
        {
            printf("\nRetourner au Menu --> Tapez R | Quitter --> Tapez Q : ");
            scanf(" %c", &keep);
        } while (keep != 'R' && keep != 'Q');
    } while (keep == 'R');
    
    
    for(i = 0; i < nS; i++)
        freeR(&tab_Room[i]);

    return 0;
}
