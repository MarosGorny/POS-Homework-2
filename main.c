#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


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

}

void* sadF(void* arg) {
    SAD* sad = arg;
    printf("V SADE zacina rast ovocie!!\n");
    OVOCIE ovocie;

    while (sad->pocetVytvorenehoOvocia < sad->dostatokOvociaNaSkoncenie) {
        printf("Pocet doposial narasteneho ovocia: %d\n",sad->pocetVytvorenehoOvocia);
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
    }

    printf("V SADE uz nenarastie ziadne ovocie, blizi sa zima...\n");
}



int main(int argc, char* argv[]) {
    printf("ZACINA sa ZBER OVOCIA!!\n");

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
    for (int i = 0; i < n; ++i) {
        pthread_join(zberaci[i],NULL);
    }

    pthread_mutex_destroy(&mut);
    pthread_cond_destroy(&pridaj);
    pthread_cond_destroy(&odober);


    printf("KONCI sa ZBER OVOCIA!!\n");
    return 0;
}


