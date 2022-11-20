#include <stdio.h>
#include <pthread.h>


#define QTD_SERFS_GERADOS               100 // número de serfs para cruzar
#define QTD_HACKERS_GERADOS             100 // número de hackers para cruzar
#define TRIPULACAO_NECESSARIA           4   // cadeiras nos barcos
#define TOTAL_DE_BARCOS                 500  // barcos que zarparão 

pthread_mutex_t mutex_contador_hackers = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_contador_serfs = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_seleciona_capitao = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t assento_serf = PTHREAD_COND_INITIALIZER;
pthread_cond_t assento_hacker = PTHREAD_COND_INITIALIZER;

__uint8_t hackers = 0;
__uint8_t serfs = 0;
__uint8_t existe_capitao = 0;
__uint8_t pessoa_no_barco = 0;


void rowBoat(){
    printf("Remando o barco.\n");
}

void board(int op){
    if(op){
        printf("Serf entrando no barco.\n");
        return;
    }
    printf("Hacker entrando no barco.\n");
}

void *serf(void *arg){
    __uint8_t capitao = 0;

    pthread_mutex_lock(&mutex_contador_serfs);
    serfs++;
    pthread_mutex_unlock(&mutex_contador_serfs);


    while(1){
        if(serfs >= TRIPULACAO_NECESSARIA){
            pthread_mutex_lock(&mutex_contador_serfs);
            serfs-=TRIPULACAO_NECESSARIA;
            pthread_mutex_unlock(&mutex_contador_serfs);

            pthread_mutex_lock(&mutex_seleciona_capitao);
            if(!existe_capitao) capitao = 1;
            existe_capitao = 1;
            for(int i = 0; i < TRIPULACAO_NECESSARIA-1; i++) pthread_cond_signal(&assento_serf);
            pthread_mutex_unlock(&mutex_seleciona_capitao);
            break;
        }
        else if((serfs >= (int) TRIPULACAO_NECESSARIA/2) && (hackers >= (int) TRIPULACAO_NECESSARIA/2)){
            pthread_mutex_lock(&mutex_contador_serfs);
            serfs -= TRIPULACAO_NECESSARIA;
            pthread_mutex_unlock(&mutex_contador_serfs);

            pthread_mutex_lock(&mutex_contador_hackers);
            hackers -= TRIPULACAO_NECESSARIA;
            pthread_mutex_unlock(&mutex_contador_hackers);

            pthread_mutex_lock(&mutex_seleciona_capitao);
            if(!existe_capitao) capitao = 1;
            existe_capitao = 1;
            for(int i = 0; i < ((int) TRIPULACAO_NECESSARIA/2)-1; i++) pthread_cond_signal(&assento_hacker);
            for(int i = 0; i < ((int) TRIPULACAO_NECESSARIA/2)-1; i++) pthread_cond_signal(&assento_serf);
            pthread_mutex_unlock(&mutex_seleciona_capitao);
            break;
        }
        else{
            pthread_mutex_lock(&mutex_contador_serfs);
            while(!existe_capitao) pthread_cond_wait(&assento_serf, &mutex_contador_serfs);
            pthread_mutex_unlock(&mutex_contador_serfs);
            break;
        }
    }

    board(1);

    if(capitao){
        pthread_mutex_lock(&mutex_seleciona_capitao);
        existe_capitao = 0;
        pthread_mutex_unlock(&mutex_seleciona_capitao);
        rowBoat();
    }

    pthread_exit(NULL);
}

void *hacker(void *arg){
    __uint8_t capitao = 0;

    pthread_mutex_lock(&mutex_contador_hackers);
    hackers++;
    pthread_mutex_unlock(&mutex_contador_hackers);


    while(1){
        if(hackers >= TRIPULACAO_NECESSARIA){
            pthread_mutex_lock(&mutex_contador_hackers);
            hackers-=TRIPULACAO_NECESSARIA;
            pthread_mutex_unlock(&mutex_contador_hackers);

            pthread_mutex_lock(&mutex_seleciona_capitao);
            if(!existe_capitao) capitao = 1;
            existe_capitao = 1;
            for(int i = 0; i < TRIPULACAO_NECESSARIA-1; i++) pthread_cond_signal(&assento_hacker);
            pthread_mutex_unlock(&mutex_seleciona_capitao);
            break;
        }
        else if((hackers >= (int) TRIPULACAO_NECESSARIA/2) && (serfs >= (int) TRIPULACAO_NECESSARIA/2)){
            pthread_mutex_lock(&mutex_contador_hackers);
            hackers -= TRIPULACAO_NECESSARIA;
            pthread_mutex_unlock(&mutex_contador_hackers);

            pthread_mutex_lock(&mutex_contador_serfs);
            serfs -= TRIPULACAO_NECESSARIA;
            pthread_mutex_unlock(&mutex_contador_serfs);

            pthread_mutex_lock(&mutex_seleciona_capitao);
            if(!existe_capitao) capitao = 1;
            existe_capitao = 1;
            for(int i = 0; i < ((int) TRIPULACAO_NECESSARIA/2)-1; i++) pthread_cond_signal(&assento_hacker);
            for(int i = 0; i < ((int) TRIPULACAO_NECESSARIA/2)-1; i++) pthread_cond_signal(&assento_serf);
            pthread_mutex_unlock(&mutex_seleciona_capitao);
            break;
        }
        else{
            pthread_mutex_lock(&mutex_contador_hackers);
            while(!existe_capitao) pthread_cond_wait(&assento_hacker, &mutex_contador_hackers);
            pthread_mutex_unlock(&mutex_contador_hackers);
            break;
        }
    }

    board(0);

    if(capitao){
        pthread_mutex_lock(&mutex_seleciona_capitao);
        existe_capitao = 0;
        pthread_mutex_unlock(&mutex_seleciona_capitao);
        rowBoat();
    }

    pthread_exit(NULL);
    // entra e soma um na fila de hackers
    // se 4 hackers - define um capitao
    // se 2 hackers - espera 2 serfs
    // se nao, dorme até que exista um capitao

    // entra no barco
    // se for o capitao, chama o zarpar
    // termina a thread
}

void *criaHackers(){
    pthread_t threadsHackers[QTD_HACKERS_GERADOS];

    for (int i = 0 ; i < QTD_HACKERS_GERADOS; i++) pthread_create(&threadsHackers[i], NULL, hacker, NULL);
    for (int i = 0 ; i < QTD_HACKERS_GERADOS; i++) pthread_join(threadsHackers[i], NULL);
    pthread_exit(NULL);
}

void *criaSerfs(){
    pthread_t threadsSerfs[QTD_SERFS_GERADOS];

    for (int i = 0 ; i < QTD_SERFS_GERADOS; i++) pthread_create(&threadsSerfs[i], NULL, serf, NULL); 
    for (int i = 0 ; i < QTD_SERFS_GERADOS; i++) pthread_join(threadsSerfs[i], NULL); 
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    pthread_t criadores[2];

    pthread_create(&criadores[0], NULL, criaHackers, NULL);
    pthread_create(&criadores[1], NULL, criaSerfs, NULL);

    pthread_join(criadores[0], NULL);
    pthread_join(criadores[1], NULL);
    return 0;
}