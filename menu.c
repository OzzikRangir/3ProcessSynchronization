#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>





#define FIFO "fifoFile"


int P1,P2,P3;
char p1[10],p2[10],p3[10];

int main(int argc, char* argv[])
{
    mknod(FIFO, S_IFIFO|0666, 0);
    FILE *f;
    char plik[20];
    f = fopen(FIFO, "r");
    fgets(plik,20,f);
    fclose(f);

    unlink(FIFO);
    sscanf(plik,"%d %d %d",&P1,&P2,&P3);

    printf("PIDy procesów:\n");
    printf("1: %d\n",P1);
    printf("2: %d\n",P2);
    printf("3: %d\n",P3);
    printf("Kliknij aby kontynuowac");
    getchar();

    int wybor, wybor2,wybor3;

    while(1)
    {
        system("clear");
        printf("\n-----___MENU___-----\n");
        printf("1: Nadawanie sygnałów\n");
        printf("2: Wyłączenie programu\n");

        scanf("%d",&wybor);

        if(wybor==1)
        {
            system("clear");
            printf("Podaj numer procesu: 1-3\n");
            scanf("%d",&wybor2);
            if((wybor2<1)&&(wybor2>3))
            {
                printf("Nie ma takiego procesu!");
                continue;
            }
            system("clear");
            printf("Proces %d:\n\n",wybor2);
            printf("Jaki sygnał chcesz wysłać, wybierz odp. operacje:\n");
            printf("1-S1(zakończ)\n2-S2(wstrzymaj)\n3-S3(wznów)?\n");
            printf("Wybór: ");
            scanf("%d",&wybor3);
            fflush (stdin);
            switch(wybor2)
            {

            case 1:
                if(wybor3==1)
                {
                    kill(P1,5);
                    kill(getpid(),SIGKILL);
                }
                if(wybor3==2)kill(P1,7);
                if(wybor3==3)
                {
                    kill(P1,8);
                }
                break;

            case 2:
                if(wybor3==1)
                {
                    kill(P2,5);
                    kill(getpid(),SIGKILL);
                }
                if(wybor3==2)kill(P2,7);
                if(wybor3==3)
                {
                    kill(P2,8);
                }
                break;

            case 3:
                if(wybor3==1)
                {
                    kill(P3,5);
                    kill(getpid(),SIGKILL);
                }
                if(wybor3==2)kill(P3,7);
                if(wybor3==3)
                {
                    kill(P3,8);
                }
                break;
            default:
                printf("Nie ma takiej operacji!!! \n\n");
            }
        }

        if(wybor==2)
        {
            printf("\n<Wyjście z programu>\n");
            sleep(1);

            kill(P1,5);
            sleep(1);
            kill(getpid(),SIGKILL);
        }



    }

}
