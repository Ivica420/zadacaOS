#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

/* Napišite program koji demonstrira korištenje zajedničke
memorije, te uključuje mogućnost prijevremenog izlaska iz
programa (ctrl+C). S tim da prekidna rutina briše zauzete
sustavske resurse (semafore i zajedničku memoriju) prije
no što program završi.
Ostvariti sustav paralelnih procesa/dretvi. */

// Zajednicke varijable
int *PRAVO, *ZASTAVICA;
int pravoId, zastavicaId;

// PID-ovi child procesa
pid_t p1 = -1, p2 = -1;

// ulazak u kriticni odsjek
void udi_u_kriticni_odsjecak(int i, int j) {


    ZASTAVICA[i] = 1;
    *PRAVO = j;

    while (ZASTAVICA[j] == 1 && *PRAVO == j) {
        // cekanje
    }
}

// Izlazak iz kriticnog odsjeka
void izadji_iz_kriticnog_odsjecka(int i,int j) {
    (void)j;
    ZASTAVICA[i] = 0;
}

// Funkcija procesa
void proc(int i, int j) {

    int k, m;

    for (k = 1; k < 5; k++) {

        udi_u_kriticni_odsjecak(i, j);

        // Kriticni odsjek
        for (m = 1; m < 5; m++) {

            printf("Proces: %d | k: %d | m: %d\n", i, k, m);
            fflush(stdout);

            sleep(1);
        }

        izadji_iz_kriticnog_odsjecka(i, j);
    }
}

// Brisanje shared memory i gasenje programa
void brisi(int sig) {

    (void)sig;

    printf("\nGasim program...\n");

    // Gasi child procese
    if (p1 > 0)
        kill(p1, SIGKILL);

    if (p2 > 0)
        kill(p2, SIGKILL);

    // Odspajanje shared memory
    shmdt(PRAVO);
    shmdt(ZASTAVICA);

    // Brisanje shared memory segmenta
    shmctl(pravoId, IPC_RMID, NULL);
    shmctl(zastavicaId, IPC_RMID, NULL);

    exit(0);
}

int main() {

    // Kreiranje shared memory
    pravoId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0660);

    zastavicaId = shmget(IPC_PRIVATE,sizeof(int) * 2,IPC_CREAT | 0660);

    // Spajanje shared memory
    PRAVO = (int *) shmat(pravoId, NULL, 0);

    ZASTAVICA = (int *) shmat(zastavicaId, NULL, 0);

    // Inicijalizacija
    *PRAVO = 0;

    ZASTAVICA[0] = 0;
    ZASTAVICA[1] = 0;

    // Signal handler za Ctrl+C
    signal(SIGINT, brisi);

    // Prvi child proces
    p1 = fork();

    if (p1 == 0) {

        proc(0, 1);
        exit(0);
    }

    // Drugi child proces
    p2 = fork();

    if (p2 == 0) {

        proc(1, 0);
        exit(0);
    }

    // Roditelj ceka child procese
    wait(NULL);
    wait(NULL);

    // Ciscenje memorije
    brisi(0);

    return 0;
}
