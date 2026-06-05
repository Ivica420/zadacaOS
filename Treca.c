#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

/*Ostvariti program koji simulira tijek rezervacije stolova u nekom
restoranu. Program na početku treba stvoriti određeni broj dretvi koji
se zadaje u naredbenom retku. Svaka dretva/proces nakon isteka
jedne sekunde provjerava ima li slobodnih stolova te slučajno
odabire jedan od njih. Nakon odabira dretva ulazi u kritični odsječak
te ponovo provjerava je li odabrani stol slobodan. Ako jest, označava
stol zauzetim i izlazi iz kritičnog odsječka. U oba slučaja, nakon
obavljene operacije ispisuje trenutno stanje svih stolova te podatke o
obavljenoj rezervaciji. Prilikom ispisa za svaki stol mora biti vidljivo
je li slobodan ili broj dretve/procesa koja je taj stol rezervirala. Broj
stolova se također zadaje u naredbenom retku. Svaka dretvaponavlja isti postupak sve dok više nema slobodnih stolova.
Program završava kada sve dretve završe. Zastitu kriticnog odsječka ostvaruje se pomoću Lamportove algoritma.*/ 

int *REZ;        // stolovi (-1 slobodan, inace ID dretve)
int *BROJ;       // Lamport broj po dretvi
int *ULAZ;       // Lamport ulaz
int brSt;        // broj stolova
int brDr;        // broj dretvi

// provjera jesu li svi stolovi zauzeti
int sve_zauzeto() {
    for (int i = 0; i < brSt; i++) {
        if (REZ[i] == -1) return 0;
    }
    return 1;
}

// max broj za Lamport
int max_broj() {
    int m = 0;
    for (int i = 0; i < brDr; i++) {
        if (BROJ[i] > m) m = BROJ[i];
    }
    return m;
}

// Lamport ulazak
void udji_u_kriticni(int i) {
    ULAZ[i] = 1;
    BROJ[i] = max_broj() + 1;
    ULAZ[i] = 0;

    for (int j = 0; j < brDr; j++) {
        if (j == i) continue;

        while (ULAZ[j] != 0);
        while (BROJ[j] != 0 &&
              (BROJ[j] < BROJ[i] ||
              (BROJ[j] == BROJ[i] && j < i)));
    }
}

// Lamport izlazak
void izadji(int i) {
    BROJ[i] = 0;
}

// ispis stanja
void ispisi() {
    printf("Stanje: ");
    for (int i = 0; i < brSt; i++) {
        if (REZ[i] == -1)
            printf("-");
        else
            printf("%d", REZ[i] + 1);
    }
    printf("\n");
}

// dretva
void *dretva(void *arg) {
    int id = *(int *)arg;

    while (!sve_zauzeto()) {

        sleep(1);

        if (sve_zauzeto()) break;

        int stol = rand() % brSt;

        printf("Dretva %d: odabirem stol %d\n", id + 1, stol + 1);

        udji_u_kriticni(id);

        if (REZ[stol] == -1) {
            REZ[stol] = id;
            printf("Dretva %d: rezerviram stol %d\n", id + 1, stol + 1);
        } else {
            printf("Dretva %d: neuspjela rezervacija stola %d\n", id + 1, stol + 1);
        }

        ispisi();

        izadji(id);
    }

    free(arg);
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    printf("Broj dretvi: ");
    scanf("%d", &brDr);

    printf("Broj stolova: ");
    scanf("%d", &brSt);

    pthread_t *threads = malloc(sizeof(pthread_t) * brDr);

    REZ = malloc(sizeof(int) * brSt);
    BROJ = malloc(sizeof(int) * brDr);
    ULAZ = malloc(sizeof(int) * brDr);

    for (int i = 0; i < brSt; i++) REZ[i] = -1;
    for (int i = 0; i < brDr; i++) {
        BROJ[i] = 0;
        ULAZ[i] = 0;
    }

    // stvaranje dretvi (SAMO JEDNOM)
    for (int i = 0; i < brDr; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, dretva, id);
    }

    // cekanje dretvi
    for (int i = 0; i < brDr; i++) {
        pthread_join(threads[i], NULL);
    }

    free(REZ);
    free(BROJ);
    free(ULAZ);
    free(threads);

    printf("Svi stolovi zauzeti. Kraj.\n");

    return 0;
}
