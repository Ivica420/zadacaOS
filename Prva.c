#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

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
