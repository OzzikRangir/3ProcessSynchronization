#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>


#define FIFO "fifoFile"
#define FIFO2 "fifoP1P2"
#define FILENAME "plik"
#define SIZEBUFF 256
#define MAX 16

void proces1();
void proces2();
void proces3();

struct mymsgbuf
{
    long mtype;
    int pid;
    char txt[50];
} msg;

struct mymsgbuf buf;

int pipeP1P2[2];
int PID1,PID2,PID3;
unsigned char *shm, *s;
int shmid;
int pauza=0, qid;
sem_t * semaphoreP2P3;
FILE *f;

int openQueue()
{
    key_t keyval = ftok(".", 'm');
    if ((qid = msgget(keyval, IPC_CREAT | 0660)) == -1)
        return (-1);

    return (qid);
}

int sendMessage(int qid, struct mymsgbuf *qbuf)
{
    int result;
    int length = sizeof(struct mymsgbuf) - sizeof(long);

    if ((result = msgsnd(qid, qbuf, length, 0)) == -1)
        return (-1);

    return (result);
}

int readMessage(int qid, long type, struct mymsgbuf *qbuf)
{
    int result;
    int length = sizeof(struct mymsgbuf) - sizeof(long);

    if ((result = msgrcv(qid, qbuf, length, type, 0)) == -1)
        return (-1);

    return (result);
}
int removeTrash()
{
    if (msgctl(qid, IPC_RMID, 0) == -1)
        return (-1);
    shmctl(shmid, IPC_RMID, 0);
    unlink(FIFO2);
    remove(FILENAME);
    sem_close(semaphoreP2P3);
    printf("Usunięcie kolejki, semafora, fifo i pliku\n");
    return (0);
}


static int P1,P2,P3;

int P;
int PIPEPID1[2],PIPEPID2[2];



void pauseProcess()
{
    pauza = 1;
    printf("\nPID: %d wstrzymanie \n", getpid());
    strcpy(msg.txt, "stop");

    if(getpid()==P1)
    {
        msg.mtype = P2;
        sendMessage(qid, &msg);
        msg.mtype = P3;
        sendMessage(qid, &msg);

        kill(P2, 4);
        kill(P3, 4);
    }
    if(getpid()==P2)
    {
        msg.mtype = P1;
        sendMessage(qid, &msg);
        msg.mtype = P3;
        sendMessage(qid, &msg);

        kill(P1, 4);
        kill(P3, 4);
    }

    if(getpid()==P3)
    {
        msg.mtype = P1;
        sendMessage(qid, &msg);
        msg.mtype = P2;
        sendMessage(qid, &msg);

        kill(P1, 4);
        kill(P2, 4);
    }

}

void finish()
{
    strcpy(msg.txt, "koniec");
    if(getpid()==P1)
    {
        msg.mtype = P2;
        sendMessage(qid, &msg);
        msg.mtype = P3;
        sendMessage(qid, &msg);

        kill(P2, 4);
        kill(P3, 4);
    }
    if(getpid()==P2)
    {
        msg.mtype = P1;
        sendMessage(qid, &msg);
        msg.mtype = P3;
        sendMessage(qid, &msg);

        kill(P1, 4);
        kill(P3, 4);
    }

    if(getpid()==P3)
    {
        msg.mtype = P1;
        sendMessage(qid, &msg);
        msg.mtype = P2;
        sendMessage(qid, &msg);

        kill(P1, 4);
        kill(P2, 4);
    }
    printf("\nPID: %d, zakonczenie pracy\n", getpid());
    kill(getppid(),SIGTERM);

    exit(1);
}

void resumeProcess()
{
    printf("\nPID: %d, wznowienie\n", getpid());
    pauza = 0;
    strcpy(msg.txt, "start");

    if(getpid()==P1)
    {
        msg.mtype = P2;
        sendMessage(qid, &msg);
        msg.mtype = P3;
        sendMessage(qid, &msg);

        kill(P2, 4);
        kill(P3, 4);
    }
    if(getpid()==P2)
    {
        msg.mtype = P1;
        sendMessage(qid, &msg);
        msg.mtype = P3;
        sendMessage(qid, &msg);
        kill(P1, 4);
        kill(P3, 4);
    }

    if(getpid()==P3)
    {
        msg.mtype = P1;
        sendMessage(qid, &msg);
        msg.mtype = P2;
        sendMessage(qid, &msg);

        kill(P1, 4);
        kill(P2, 4);
    }

}
void infoSignal()
{
    readMessage(qid, getpid(), &buf);
    printf("\nProces %d odczytal sygnal informacyjny z poleceniem '%s'\n",getpid(), buf.txt);
    if (strcmp(buf.txt, "koniec") == 0)
    {
        printf("PID: %d, zakonczenie pracy\n", getpid());
        usleep(1000);

        exit(1);
    }
    else if (strcmp(buf.txt, "stop") == 0)
    {
        if(pauza!=1)
        {
            pauza = 1;//zmiana zmiennej pauzy
            printf("PID: %d, wstrzymanie\n", getpid());
        }
    }
    else if (strcmp(buf.txt, "start") == 0)
    {
        if(pauza!=0)
        {
            pauza = 0;//zmiana zmiennej pauzy
            printf("PID: %d, wznowienie\n", getpid());
        }
    }
    else
    {
        printf("\nNieznana wiadomosc\n");
        printf("\n%s\n", buf.txt);
    }
}


void killCreator()
{
    removeTrash();
    /
    printf("Zakonczenie pracy procesu glownego");
    exit(1);
}


int main(int argc, char **argv)
{
    semaphoreP2P3=mmap(0,sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED,0,0);
    sem_init(semaphoreP2P3,1,0);


    if ((mknod(FIFO2, S_IFIFO|0666, 0)) < 0)
    {
        perror("FIFOERR");
        exit(1);
    }


    if (pipe(PIPEPID1) == -1)
    {
        perror("pipe failed");
        exit(1);
    }


    if (pipe(PIPEPID2) == -1)
    {
        perror("pipe failed");
        exit(1);
    }


    openQueue();

    if((P1=fork())==0)
    {
        P1=getpid();
        proces1();
    }
    else if(P1<0)
    {
        perror("fork failed");
        exit(2);
    }
    else if((P2=fork())==0)
    {
        P2=getpid();
        proces2();
    }
    else if(P2<0)
    {
        perror("fork failed");
        exit(2);
    }
    else if((P3=fork())==0)
    {
        P3=getpid();
        proces3();
    }
    else if(P3<0)
    {
        perror("fork failed");
        exit(2);

    }

    else
    {
        signal(SIGTERM, killCreator);

        system("x-terminal-emulator -e ./Sterowanie");


        printf("P1(proces 1)=%d\n",P1);
        printf("P2(proces 2)=%d\n",P2);
        printf("P3(proces 3)=%d\n\n",P3);

        mknod(FIFO, S_IFIFO|0666, 0);

        FILE *p;
        p = fopen(FIFO, "w");
        fprintf(p, "%d %d %d",P1,P2,P3);
        fclose(p);

        unlink(FIFO);


        mknod(FILENAME,0666, 0);
        f = fopen(FILENAME, "w");
        fclose(f);



        close(PIPEPID1[0]);
        write(PIPEPID1[1], &P1, sizeof(int));
        write(PIPEPID1[1], &P2, sizeof(int));
        write(PIPEPID1[1], &P3, sizeof(int));
        close(PIPEPID1[1]);


        close(PIPEPID2[0]);
        write(PIPEPID2[1], &P1, sizeof(int));
        write(PIPEPID2[1], &P2, sizeof(int));
        write(PIPEPID2[1], &P3, sizeof(int));
        close(PIPEPID2[1]);

    }


    pause();
}



void proces1()
{
    printf("Uruchomienie 1 procesu\n");




    close(PIPEPID1[1]);
    read(PIPEPID1[0], &P1, sizeof(int));
    read(PIPEPID1[0], &P2, sizeof(int));
    read(PIPEPID1[0], &P3, sizeof(int));
    close(PIPEPID1[0]);


    signal(5, finish);
    signal(4, infoSignal);
    signal(7, pauseProcess);
    signal(8, resumeProcess);
    unsigned char bufor[16];
    while(1)
    {
        if(pauza!=1)
        {
            if(fgets(bufor,MAX+1,stdin)!=NULL)
            {
                f = fopen(FIFO2, "w");
                fputs(bufor,f);
                fclose(f);
                usleep(10000);
            }
            else break;
        }
    }
}


void proces2(void)
{
    printf("Uruchomienie 2 procesu\n");

    signal(5, finish);
    signal(4, infoSignal);
    signal(7, pauseProcess);
    signal(8, resumeProcess);


    close(PIPEPID2[1]);
    read(PIPEPID2[0], &P1, sizeof(int));
    read(PIPEPID2[0], &P2, sizeof(int));
    read(PIPEPID2[0], &P3, sizeof(int));
    close(PIPEPID2[0]);

    unsigned char bufor[16];

    while(1)
    {

        if(pauza!=1)
        {
            f = fopen(FIFO2, "r");
            fgets(bufor,MAX+1,f);
            fclose(f);
            for(int i=0; i<MAX; i++)
            {
                if(bufor[i]!=0)
                {

                    f = fopen(FILENAME, "a");


                    if(bufor[i]<16)
                    {

                        fprintf(f,"%x",0);
                        fprintf(f,"%x",bufor[i]);
                    }
                    else
                        fprintf(f,"%x",bufor[i]);
                    bufor[i]=0;
                    fclose(f);
                }

                else break;

            }

            sem_post(semaphoreP2P3);
        }
        if(pauza==1)
            for(int i=0; i<MAX*2; i++)
                s[i]=0;


    }
}

void proces3(void)
{
    printf("Uruchomienie 3 procesu\n");


    signal(5, finish);
    signal(4, infoSignal);
    signal(7, pauseProcess);
    signal(8, resumeProcess);


    char fileIn[32];
    for(int i=0; i<MAX*2; i++)
    {
        fileIn[i]=0;
    }
    while(1)
    {
        sem_wait(semaphoreP2P3);
        if(pauza!=1)
        {
            f = fopen(FILENAME, "r");

            fgets(fileIn,MAX*2+1,f);
            fclose(f);
            if(fileIn[0]!=0)
            {


                for(int i=0; i<MAX*2; i++)
                {
                    fputc(fileIn[i],stderr);
                    fileIn[i]=0;
                    if(i%2==1)
                    {
                        fputc(32,stderr);
                    }
                }
                fputc('\n',stderr);

                fflush(stderr);
            }
        }
        f = fopen(FILENAME, "w");
        fclose(f);
    }

}


