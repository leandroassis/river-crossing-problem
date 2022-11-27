/**
 * Grupo: Leandro Assis e Gabriel Peçanha
 * "Solução" inspirada no código: https://cs162.org/static/exams/fa99-mt1-solutions.pdf
*/
#include <stdio.h>
#include <pthread.h>

#define QTD_SERFS_GERADOS               100 // número de serfs para cruzar
#define QTD_HACKERS_GERADOS             100 // número de hackers para cruzar
#define TRIPULACAO_NECESSARIA           4   // cadeiras nos barcos
#define TOTAL_DE_BARCOS                 50  // barcos que zarparão 

pthread_mutex_t mutex_contador = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t acorda_serf = PTHREAD_COND_INITIALIZER; // condição para acordar uma thread serf
pthread_cond_t acorda_hacker = PTHREAD_COND_INITIALIZER; // condição para acorar uma thread hacker
pthread_cond_t zarpar_barco = PTHREAD_COND_INITIALIZER; // condição para acordar o capitao do barco
pthread_cond_t barco_disponivel = PTHREAD_COND_INITIALIZER; // condição para acordar novas threads que chegam na fila

unsigned short int hackers_na_fila = 0; // contador do numero de threads hackers na fila
unsigned short int serfs_na_fila = 0; // contador do numero de threads serfs na fila
unsigned short int barcoLivre = 1; // variavel de controle do barco estar livre ou em processo de boarding
unsigned short int barcos_zarpados = 0; // contador de barcos zarpados
unsigned short int pessoas_no_barco = 0; // contador de pessoas que já deram board no barco

// função para remar o barco
void rowBoat(){
    printf("Remando o barco %d.\n\n", barcos_zarpados);
}

// função para sinalizar que um thread entrou no barco
// se cargo == 1 -> thread serf entrou,
// se cargo == 0 -> thread hacker entrou
void board(int cargo){
    if(cargo) printf("Serf entrando no barco.\n");
    else printf("Hacker entrando no barco.\n");
}

// função de comportamento da thread serf
void *serf(void *arg){
    unsigned short int capitao = 0; // variavel local para controlar se a thread vai exercer papeis de capitao

    pthread_mutex_lock(&mutex_contador);
    // verifica se o numero de barcos zarpados ainda não atingiu o limite
    while(barcos_zarpados < TOTAL_DE_BARCOS){

        // se o barco não estiver livre, bota a thread para dormir até que o capitão que zarpou o ultimo barco acorde-a
        while(!barcoLivre){
            pthread_cond_wait(&barco_disponivel, &mutex_contador);
            // isso serve para não permitir que um nova thread chegue, se torne capitão e tente zarpar um novo barco
        }

        if(serfs_na_fila == 3){
            // se tiverem 3 serfs na fila acorda eles
            pthread_cond_signal(&acorda_serf);
            pthread_cond_signal(&acorda_serf);
            pthread_cond_signal(&acorda_serf);
            
            serfs_na_fila -= 3; // retira as threads da fila
            capitao = 1; // essa thread se torna a capitã
            barcoLivre = 0; // e seta o barco como NÃO livre (em processo de boarding)
        }
        else if((serfs_na_fila >= 1) && (hackers_na_fila >= 2)){
            //se tiver mais de um serf e mais de 2 hackers na fila acorda eles
            pthread_cond_signal(&acorda_serf);
            pthread_cond_signal(&acorda_hacker);
            pthread_cond_signal(&acorda_hacker);

            //retira as threads da fila
            serfs_na_fila--;
            hackers_na_fila-=2;

            capitao = 1; // essa thread se torna a capitã
            barcoLivre = 0; // e seta o barco como NÃO livre (em processo de boarding)
        }
        else{
            // senao coloca essa thread na fila e a coloca para dormir
            serfs_na_fila++;
            pthread_cond_wait(&acorda_serf, &mutex_contador); // dorme e libera o mutex
        }

        board(1); // quando a thread acorda, entra no barco
        pessoas_no_barco++;
        pthread_cond_signal(&zarpar_barco); // avisa o capitao que entrou mais um no barco

        if(capitao){
            // se a thread for o capitão dorme enquanto não tiver 4 pessoas no barco
            while(pessoas_no_barco < TRIPULACAO_NECESSARIA){
                pthread_cond_wait(&zarpar_barco, &mutex_contador); // dorme e libera o mutex até mais alguém entrar no barco
            }
            pessoas_no_barco = 0; // reseta o contador de pessoas no barco
            barcoLivre = 1; // libera o barco
            barcos_zarpados++; // incrementa os barcos zarpados
            rowBoat(); // rema o barco
            pthread_cond_signal(&barco_disponivel); // acorda uma thread para entrar na fila
        }
    }
    pthread_mutex_unlock(&mutex_contador); // libera o mutex
    pthread_cond_signal(&barco_disponivel); // acorda uma thread para entrar na fila, caso haja
    pthread_exit(NULL);
}

// função de comportamento da thread hacker (simétrica à serf)
void *hacker(void *arg){
    unsigned short int capitao = 0;

    pthread_mutex_lock(&mutex_contador);
    while(barcos_zarpados < TOTAL_DE_BARCOS){
        while(!barcoLivre){
            pthread_cond_wait(&barco_disponivel, &mutex_contador);
        }

        if(hackers_na_fila == 3){
            pthread_cond_signal(&acorda_hacker);
            pthread_cond_signal(&acorda_hacker);
            pthread_cond_signal(&acorda_hacker);

            hackers_na_fila -= 3;
            capitao = 1;
            barcoLivre = 0;
        }
        else if((hackers_na_fila >= 1) && (serfs_na_fila >= 2)){
            pthread_cond_signal(&acorda_serf);
            pthread_cond_signal(&acorda_serf);
            pthread_cond_signal(&acorda_hacker);

            hackers_na_fila--;
            serfs_na_fila-=2;
            capitao = 1;
            barcoLivre = 0;
        }
        else{
            hackers_na_fila++;
            pthread_cond_wait(&acorda_hacker, &mutex_contador);
        }

        board(0);
        pessoas_no_barco++;
        pthread_cond_signal(&zarpar_barco);

        if(capitao){
            while(pessoas_no_barco < TRIPULACAO_NECESSARIA){
                pthread_cond_wait(&zarpar_barco, &mutex_contador);
            }
            pessoas_no_barco = 0;
            barcoLivre = 1;
            barcos_zarpados++;
            rowBoat();
            pthread_cond_signal(&barco_disponivel);
        }
    }
    pthread_mutex_unlock(&mutex_contador);
    pthread_cond_signal(&barco_disponivel);
    pthread_exit(NULL);
}

// função para criar as n threads hackers
void *criaHackers(){
    pthread_t threadsHackers[QTD_HACKERS_GERADOS];

    for (int i = 0 ; i < QTD_HACKERS_GERADOS; i++) pthread_create(&threadsHackers[i], NULL, hacker, NULL);
    for (int i = 0 ; i < QTD_HACKERS_GERADOS; i++) pthread_join(threadsHackers[i], NULL);
    pthread_exit(NULL);
}

// função para criar as m threads serfs
void *criaSerfs(){
    pthread_t threadsSerfs[QTD_SERFS_GERADOS];

    for (int i = 0 ; i < QTD_SERFS_GERADOS; i++) pthread_create(&threadsSerfs[i], NULL, serf, NULL); 
    for (int i = 0 ; i < QTD_SERFS_GERADOS; i++) pthread_join(threadsSerfs[i], NULL); 
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    pthread_t criadores[2];

    // cria as threads que criam as threads hackers e serfs
    pthread_create(&criadores[0], NULL, criaHackers, NULL);
    pthread_create(&criadores[1], NULL, criaSerfs, NULL);

    pthread_join(criadores[0], NULL);
    pthread_join(criadores[1], NULL);
    return 0;
}