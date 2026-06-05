#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

/*Neka program simulira neki dugotrajni posao (slično servisima) koji koristi dvije datoteke: 
u jednu dodaje do sada izračunate vrijednosti (npr. kvadrati slijednih brojeva), a u drugu 
podatak do kuda je stigao. Npr. u obrada.txt zapisuje 1 4 9 … (svaki broj u novi red) a u 
status.txt informaciju o tome gdje je stao ili kako nastaviti. Npr. ako je zadnji broj u 
obrada.txt 100 u status.txt treba zapisati 10 tako da u idućem pokretanju može nastaviti 
raditi i dodavati brojeve.
Prije pokretanja te je datoteke potrebno ručno napraviti i inicijalizirati. Početne vrijednosti 
mogu biti iste – broj 1 u obje datoteke.
Pri pokretanju programa on bi trebao otvoriti obje datoteke, iz status.txt, pročitati tamo 
zapisanu vrijednost. Ako je ona veća od nule program nastavlja s proračunom s prvom 
idućom vrijednošću i izračunate vrijednosti nadodaje u obrada.txt. Prije nastavka rada u 
status.txt upisuje 0 umjesto prijašnjeg broja, što treba označavati da je obrada u tijeku, da 
program radi.
Na signal (npr. SIGUSR1) program treba ispisati trenutni broj koji koristi u obradi. Na 
signal SIGTERM otvoriti status.txt i tamo zapisati zadnji broj (umjesto nule koja je tamo) 
te završiti s radom.
Na SIGINT samo prekinuti s radom, čime će u status.txt ostati nula (čak se taj signal niti 
ne mora maskirati – prekid je pretpostavljeno ponašanje!). To će uzrokovati da iduće 
pokretanje detektira prekid – nula u status.txt, te će za nastavak rada, tj. Određivanje 
idućeg broja morati „analizirati“ datoteku obrada.txt i od tamo zaključiti kako nastaviti 
(pročitati zadnji broj i izračunati korijen). Operacije s datotekama, radi jednostavnosti, 
uvijek mogu biti u nizu open+fscanf/fprintf+close, tj. ne držati datoteke otvorenima da se 
izbjegnu neki drugi problemi. Ali ne mora se tako. U obradu dodati odgodu (npr. sleep(5)) 
da se uspori izvođenje */

int broj;

char obradaDatoteka[] = "obrada.txt";
char statusDatoteka[] = "status.txt";

/* Obrada broja */
int obrada(int x)
{
    return x * x;
}

/* SIGUSR1 -> ispisi trenutni broj */
void obradiSIGUSR1(int sig)
{
	 (void)sig;
    printf("\nTrenutni broj: %d\n", broj);
}

/* SIGTERM -> spremi stanje i zavrsi */
void obradiSIGTERM(int sig)
{
	 (void)sig;
    FILE *f;

    f = fopen(statusDatoteka, "w");

    if (f != NULL)
    {
        fprintf(f, "%d", broj);
        fclose(f);
    }

    printf("\nStanje spremljeno u status.txt\n");
    exit(0);
}

int main()
{
    FILE *f;
    int zadnjiBroj;
    int x;

    /* Maskiranje signala */
    signal(SIGUSR1, obradiSIGUSR1);
    signal(SIGTERM, obradiSIGTERM);

    /* Procitaj broj iz status.txt */
    f = fopen(statusDatoteka, "r");

    if (f == NULL)
    {
        printf("Ne mogu otvoriti status.txt\n");
        return 1;
    }

    fscanf(f, "%d", &broj);
    fclose(f);

    /* Ako je broj 0 -> analiza obrada.txt */
    if (broj == 0)
    {
        f = fopen(obradaDatoteka, "r");

        if (f == NULL)
        {
            printf("Ne mogu otvoriti obrada.txt\n");
            return 1;
        }

        while (fscanf(f, "%d", &zadnjiBroj) != EOF)
        {
        }

        fclose(f);

        /* Izracunaj korijen zadnjeg kvadrata */
        broj = (int)sqrt(zadnjiBroj);
    }

    /* Upisi 0 u status.txt -> obrada traje */
    f = fopen(statusDatoteka, "w");

    if (f == NULL)
    {
        printf("Ne mogu otvoriti status.txt\n");
        return 1;
    }

    fprintf(f, "0");
    fclose(f);

	printf("Poceo osnovni program PID=%d\n", getpid());

    /* Beskonacna petlja */
    while (1)
    {
        broj = broj + 1;

        x = obrada(broj);

        /* Dodaj rezultat u obrada.txt */
        f = fopen(obradaDatoteka, "a");

        if (f == NULL)
        {
            printf("Ne mogu otvoriti obrada.txt\n");
            return 1;
        }

        fprintf(f, "%d\n", x);
        fclose(f);

        sleep(5);
    }

    return 0;
}
