#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BROJ_FILOZOFA 5

 
pthread_mutex_t monitor; // mutex omogucava medjusobno iskljucivanje              
pthread_cond_t red[BROJ_FILOZOFA];    // red uvjeta za svakog filozofa 


int vilica[BROJ_FILOZOFA];            
char filozof[BROJ_FILOZOFA];          


void ispiši_stanje(int n) {
    printf("Stanje filozofa: ");
    for (int i = 0; i < BROJ_FILOZOFA; i++) {
        printf("%c ", filozof[i]);
    }
    printf(" (Promjenu izazvao filozof %d)\n", n);
}


void misliti() {
    sleep(3); 
}


void jesti(int n) {
    
    pthread_mutex_lock(&monitor); // zakljucava mutex i ulazi u kriticni odsjecak 
    
    filozof[n] = 'o'; 
    ispiši_stanje(n); 
   
    while (vilica[n] == 0 || vilica[(n + 1) % BROJ_FILOZOFA] == 0) {
        // pthread_cond_wait privremeno otključava mutex i stavlja dretvu u red čekanja 
        pthread_cond_wait(&red[n], &monitor); 
    }
    
    // kada dretva dočeka signal i ponovno zaključa mutex, uzima obje vilice 
    vilica[n] = 0; 
    vilica[(n + 1) % BROJ_FILOZOFA] = 0; 
    
    filozof[n] = 'X'; 
    ispiši_stanje(n);
    
    pthread_mutex_unlock(&monitor); 
    
    sleep(2); 
    
    pthread_mutex_lock(&monitor);  
    
    filozof[n] = 'O'; 
    
    vilica[n] = 1;
    vilica[(n + 1) % BROJ_FILOZOFA] = 1; 
    
    pthread_cond_signal(&red[(n - 1 + BROJ_FILOZOFA) % BROJ_FILOZOFA]);  
    pthread_cond_signal(&red[(n + 1) % BROJ_FILOZOFA]);                 
    
    ispiši_stanje(n); 
    
    pthread_mutex_unlock(&monitor); 
}


void* filozof_dretva(void* arg) {
    int n = *(int*)arg; //redni broj filozofa
    free(arg);         
     
    while (1) {
        misliti(); 
        jesti(n);  
    }
    
    return NULL;
}

int main() {

    //niz koji ce cuvati identifikatore
    pthread_t dretve[BROJ_FILOZOFA];
    
    // inicijalizacija mutexa  
    pthread_mutex_init(&monitor, NULL); 

    // inicijalizacija uvjetnih varijabli
    for (int i = 0; i < BROJ_FILOZOFA; i++) {
        pthread_cond_init(&red[i], NULL); 
        vilica[i] = 1;                    
        filozof[i] = 'O';               
    }
    
    printf("PROBLEM PET FILOZOFA: \n");
    printf("O - razmišlja, X - jede, o - čeka na vilice\n\n");
    
    // stvaranje dretvi za svakog filozofa 
    for (int i = 0; i < BROJ_FILOZOFA; i++) {
        int* id = malloc(sizeof(int)); // alokacija memorije kako dretve ne bi dijelile isti pokazivač na indeks
        *id = i;
        if (pthread_create(&dretve[i], NULL, filozof_dretva, id) != 0) { 
            perror("Greška pri kreiranju dretve");
            return 1;
        }
    }
    
    for (int i = 0; i < BROJ_FILOZOFA; i++) { //govori mainu ceka da threadove
        pthread_join(dretve[i], NULL); // nece ih docekati jer je beskonacna petlja(while(1)) 
    }
    
    return 0;
}
