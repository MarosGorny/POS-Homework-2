#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>


typedef enum ovocie {
    JABLKO, HRUSKA, SLIVKA
} OVOCIE;

typedef struct spolData {
    OVOCIE* pozemok;
    int aktualnyPocetOvocia;
    int kapacitaSadu;

    pthread_mutex_t* mut;
    pthread_cond_t* pridaj;
    pthread_cond_t* odober;
}SPOL;

typedef struct zberac{
    int id;
    int casPresunu;
    int pocetDobrehoOvocia;
    int pocetZlehoOvocia;
    int kapacitaKosika;
    int aktualnyPocetOvocia;
    int dostatokOvociaNaSkoncenie;

    SPOL* data;
}ZBERAC;

typedef struct sad{
    int dostatokOvociaNaSkoncenie;
    int pocetVytvorenehoOvocia;

    SPOL* data;
}SAD;

void* zberacF(void* arg) {
    ZBERAC* zberac = arg;

    printf("Zberac[%d] je pripraveny zberat ovocie\n",zberac->id);

    while(zberac->pocetZlehoOvocia + zberac->pocetDobrehoOvocia < zberac->dostatokOvociaNaSkoncenie) {
        zberac->casPresunu =  1+ (rand()%4);
        printf("Zberac[%d] sa presuva %d sek. ku pozemku\n",zberac->id,zberac->casPresunu);
        sleep(zberac->casPresunu);
        pthread_mutex_lock(zberac->data->mut);

        while (zberac->data->aktualnyPocetOvocia <= 0) {
            printf("Zberac[%d] nenasiel ziadne ovocie, ide cakat vonku. Pocet ovocia v sade: %d\n",zberac->id,zberac->data->aktualnyPocetOvocia);
            pthread_cond_wait(zberac->data->odober,zberac->data->mut);
            printf("V sade pribudlo ovocie, zberac[%d] ide nazbierat ovocie\n",zberac->id);
        }

        while (zberac->aktualnyPocetOvocia < zberac->kapacitaKosika) {
            if (zberac->pocetZlehoOvocia + zberac->pocetDobrehoOvocia >= zberac->dostatokOvociaNaSkoncenie) {
                printf("Zberac[%d] uz naplnil svoje kvoty, prestava zbierat\n",zberac->id);
                break;
            }
            if(zberac->data->aktualnyPocetOvocia == 0) {
                printf("Zberac[%d] si vsimol, ze sad je prazdny, ide ku autu.Pocet ovocia v kosiku: %d\n",zberac->id,zberac->aktualnyPocetOvocia);
                break;
            }
            OVOCIE ovocie = zberac->data->pozemok[zberac->data->aktualnyPocetOvocia-1];
            char ovocieString[8];
            if(ovocie == JABLKO) {
                strcpy(ovocieString,"Jablko");
            } else if (ovocie == HRUSKA) {
                strcpy(ovocieString,"Hrusku");
            } else {
                strcpy(ovocieString,"Slivku");
            }
            zberac->data->aktualnyPocetOvocia--;
            zberac->aktualnyPocetOvocia++;
            printf("Zberac[%d] zodvihol %s, pocet v kosiku: %d\n",zberac->id,ovocieString,zberac->aktualnyPocetOvocia);

            int nahoda = rand()%100;
            bool zleOvocie = false;
            if(ovocie == JABLKO) {
                if(nahoda<30) {
                    zberac->pocetZlehoOvocia++;
                    zleOvocie = true;
                } else {
                    zberac->pocetDobrehoOvocia++;
                }
            } else if (ovocie == HRUSKA) {
                if(nahoda<60) {
                    zberac->pocetZlehoOvocia++;
                    zleOvocie = true;
                } else {
                    zberac->pocetDobrehoOvocia++;
                }
            } else if (ovocie == SLIVKA) {
                if(nahoda<10) {
                    zberac->pocetZlehoOvocia++;
                    zleOvocie = true;
                } else {
                    zberac->pocetDobrehoOvocia++;
                }
            }
            if(zleOvocie) {
                printf("\t Ach... Zberac[%d] si vsimol, ze ovocie ktore zodvihol je pokazene...\n",zberac->id);
            }
        }

        pthread_cond_signal(zberac->data->pridaj);
        pthread_mutex_unlock(zberac->data->mut);
        printf("Zberac[%d] sa presuva %d sek. ku autu\n",zberac->id,zberac->casPresunu);
        printf("\tSpolu ovocie:%d\n\tDobre ovocie:%d\n\tPokazene ovocie:%d\n",zberac->pocetDobrehoOvocia+zberac->pocetZlehoOvocia,zberac->pocetDobrehoOvocia,zberac->pocetZlehoOvocia);
        zberac->aktualnyPocetOvocia = 0;
        sleep(zberac->casPresunu);

    }

    printf("Zberac[%d] odchadza domov\n",zberac->id);

    int *ret = malloc(sizeof (int));
    *ret = zberac->pocetDobrehoOvocia;
    pthread_exit(ret);
}

void* sadF(void* arg) {
    SAD* sad = arg;
    printf("V SADE zacina sezona!!\n");
    OVOCIE ovocie;

    while (sad->pocetVytvorenehoOvocia < sad->dostatokOvociaNaSkoncenie) {
        int nahoda = rand()%100;

        pthread_mutex_lock(sad->data->mut);

        while (sad->data->aktualnyPocetOvocia >= sad->data->kapacitaSadu) {
            printf("Sad je zarasteny, ovocie prestalo rast. Pocet ovocia v sade: %d\n",sad->data->aktualnyPocetOvocia);
            pthread_cond_wait(sad->data->pridaj,sad->data->mut);
            printf("V sade je opat miesto na rast ovocia. Pocet ovocia v sade: %d\n",sad->data->aktualnyPocetOvocia);
        }
        if(nahoda < 50) {
            ovocie = JABLKO;
        } else if (nahoda < 80) {
            ovocie = HRUSKA;
        } else {
            ovocie = SLIVKA;
        }

        sad->data->pozemok[sad->data->aktualnyPocetOvocia] = ovocie;
        sad->data->aktualnyPocetOvocia++;
        sad->pocetVytvorenehoOvocia++;
        if(ovocie == JABLKO) {
            printf("V sade narastlo JABLKO. Pocet ovocia v sade: %d\n",sad->data->aktualnyPocetOvocia);
        } else if (ovocie == HRUSKA) {
            printf("V sade narastla HRUSKA. Pocet ovocia v sade: %d\n",sad->data->aktualnyPocetOvocia);
        } else {
            printf("V sade narastla SLIVKA. Pocet ovocia v sade: %d\n", sad->data->aktualnyPocetOvocia);
        }
        pthread_cond_signal(sad->data->odober);
        pthread_mutex_unlock(sad->data->mut);
        printf("\tPocet doposial narasteneho ovocia: %d\n",sad->pocetVytvorenehoOvocia);
        printf("\tSad si dava odpocinok... ZzZ...\n");
        sleep(1);

    }

    printf("V SADE uz nenarastie ziadne ovocie, blizi sa zima...\n");
}



int main(int argc, char* argv[]) {
    printf("ZACINA SA ZBER OVOCIA!!\n");

    int k = 4;
    int n;
    if(argc < 2) {
        n = 6;
    } else {
        n = atoi(argv[1]);
    }
    srand(time(NULL));

    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t odober = PTHREAD_COND_INITIALIZER;
    pthread_cond_t pridaj = PTHREAD_COND_INITIALIZER;

    OVOCIE pozemok[15];
    SPOL spData = {pozemok,0,15,&mut,&pridaj,&odober};

    pthread_t zberaci[n];
    pthread_t sad;

    ZBERAC zberaciD[n];
    SAD sadD = {k*4*n,0,&spData}; //PRIDAJ VECI

    pthread_create(&sad,NULL,sadF,&sadD);
    for (int i = 0; i < n; i++) {
        zberaciD[i].id = i+1;
        zberaciD[i].casPresunu = 1+(rand()%4);
        zberaciD[i].kapacitaKosika = 4;
        zberaciD[i].aktualnyPocetOvocia = 0;
        zberaciD[i].dostatokOvociaNaSkoncenie = k * 4;
        zberaciD[i].pocetDobrehoOvocia = 0;
        zberaciD[i].pocetZlehoOvocia = 0;
        zberaciD[i].data = &spData;

        pthread_create(&zberaci[i],NULL,zberacF,&zberaciD[i]);
    }


    pthread_join(sad,NULL);
    void *result = 0;
    int dobreOvocie[n];
    for (int i = 0; i < n; ++i) {
        pthread_join(zberaci[i],&result);
        dobreOvocie[i] = *((int*)result);
        free(result);
    }

    pthread_mutex_destroy(&mut);
    pthread_cond_destroy(&pridaj);
    pthread_cond_destroy(&odober);

    printf("KONCI SA ZBER OVOCIA!!\n");
    printf("\n");
    for (int i = 0; i < n; ++i) {
        printf("Zberac[%d]\n",i+1);
        printf("\tDobre ovocie: %d [%.2f %%]\n",dobreOvocie[i],(dobreOvocie[i]/16.0)*100);
        int zleOvocie = 16-dobreOvocie[i];
        printf("\tPokazene ovocie: %d [%.2f %%]\n",zleOvocie,(zleOvocie/16.0)*100);
    }

    return 0;
}


