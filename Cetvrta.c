#include <stdio.h>      
#include <stdlib.h>     
#include <pthread.h>    
#include <semaphore.h>  
#include <unistd.h>     

#define N 5                    
#define uk_posjetitelja 15 

sem_t sem_mjesta;       // Opći semafor
sem_t sem_spremni;      // Binarni semafor
sem_t sem_kraj_voznje;  // Opći semafor
sem_t sem_svi_sisli;    // Binarni semafor
sem_t mutex;            // Semafor za međusobno isključivanje

int br_ukrcanih = 0;    // Prati koliko je posjetitelja trenutno sjelo na vrtuljak.
int br_iskrcanih = 0;   // Prati koliko je posjetitelja ustalo i sišlo s vrtuljka.


void* posjetitelj(void* arg) {
    int id = *(int*)arg; // Kastanje i dereferenciranje generičkog pokazivača kako bi dobili ID dretve

    // 1. FAZA UKRCAVANJA
    sem_wait(&sem_mjesta); // Posjetitelj pokušava uzeti mjesto. Ako je vrijednost 0, blokira se i čeka.

    sem_wait(&mutex);      // Ulazak u kritični odsječak kako bismo sigurno promijenili zajedničku varijablu
    br_ukrcanih++;         // Povećavamo broj ukrcanih putnika
    printf("Posjetitelj %d: sjedi\n", id); 
    
    // Ako je ovaj posjetitelj upravo popunio zadnje slobodno mjesto (N), on budi vrtuljak
    if (br_ukrcanih == N) {
        sem_post(&sem_spremni); // Signalizira vrtuljku da je pun i da se može pokrenuti
    }
    sem_post(&mutex);      // Izlazak iz kritičnog odsječka

    // 2. FAZA VOŽNJE
    sem_wait(&sem_kraj_voznje); // Posjetitelj se ovdje blokira i uživa u vožnji dok mu vrtuljak ne javi kraj

    // 3. FAZA ISKRCAVANJA
    sem_wait(&mutex);      // Ponovno ulazimo u kritični odsječak za promjenu druge varijable
    br_iskrcanih++;        // Povećavamo broj ljudi koji su sišli s vrtuljka
    printf("Posjetitelj %d: ustani/sidji\n", id); 
    
    // Ako je ovaj posjetitelj zadnji (N-ti) koji silazi, on javlja vrtuljku da je potpuno prazan
    if (br_iskrcanih == N) {
        sem_post(&sem_svi_sisli); // Signalizira vrtuljku da su prethodni posjetitelji sišli
    }
    sem_post(&mutex);      // Izlazak iz kritičnog odsječka

    return NULL;           // Dretva posjetitelja uspješno završava i izlazi
}


void* vrtuljak(void* arg) {
	(void)arg;
    int odradjeno_voznji = 0;

    while (1) {
       
        printf("\nVrtuljak je prazan.\n");

       
        for (int i = 0; i < N; i++) {
            sem_post(&sem_mjesta);
        }

        
        sem_wait(&sem_spremni);

        printf("Vrtuljak: pokreni\n");
        sleep(2);
        printf("Vrtuljak: zaustavi\n");

        br_ukrcanih = 0;

       
        for (int i = 0; i < N; i++) {
            sem_post(&sem_kraj_voznje);
        }

       
        sem_wait(&sem_svi_sisli);
        br_iskrcanih = 0;

        
        odradjeno_voznji++;
        if (odradjeno_voznji == uk_posjetitelja / N) {
            break; 
        }
    }
    return NULL;
}

int main() {
    // Inicijalizacija semafora: (adresa semafora, 0 znaci za dretve, pocetna vrijednost)
    sem_init(&sem_mjesta, 0, 0);      // Na početku vrtuljak ima točno N slobodnih mjesta
    sem_init(&sem_spremni, 0, 0);     // Inicijalno 0 jer vrtuljak nije pun
    sem_init(&sem_kraj_voznje, 0, 0); // Inicijalno 0 jer vožnja nije gotova
    sem_init(&sem_svi_sisli, 0, 0);   // Inicijalno 0 jer nitko još nije sišao
    sem_init(&mutex, 0, 1);           // Mutex se postavlja na 1 jer je ulaz u kritični odsječak slobodan

    pthread_t dretva_vrtuljak;
    pthread_t dretve_posjetitelji[uk_posjetitelja];
    int id[uk_posjetitelja];

    
    pthread_create(&dretva_vrtuljak, NULL, vrtuljak, NULL);


    for (int i = 0; i < uk_posjetitelja; i++) {
        id[i] = i + 1; 
        pthread_create(&dretve_posjetitelji[i], NULL, posjetitelj, &id[i]);
        usleep(100000); 
    }

    
    for (int i = 0; i < uk_posjetitelja; i++) {
        pthread_join(dretve_posjetitelji[i], NULL);
    }

    // Nasilno gasenje while jer je u beskonacnoj petlji 
    pthread_cancel(dretva_vrtuljak);

    sem_destroy(&sem_mjesta);
    sem_destroy(&sem_spremni);
    sem_destroy(&sem_kraj_voznje);
    sem_destroy(&sem_svi_sisli);
    sem_destroy(&mutex);

    printf("\nSvi posjetitelji su zavrsili voznju.\n");
    return 0; 
}
