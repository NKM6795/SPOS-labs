#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>


pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

#define INIT_RESULT 1000000000
#define INIT_STOP_RESULT '&'
typedef int returnableType;


returnableType fResult = INIT_RESULT;
returnableType gResult = INIT_RESULT;
char waitStopResult = INIT_STOP_RESULT;

returnableType FFunction(int x)
{
    sleep(4);
    return 1;
}

returnableType GFunction(int x)
{
    sleep(10);
    return 0;
}


void* CalculateF(void* x)
{
    fResult = FFunction(*((int*)x));
    pthread_cond_signal(&cond);
    return NULL;
}

void* CalculateG(void* x)
{
    gResult = GFunction(*((int*)x));
    pthread_cond_signal(&cond);
    return NULL;
}

void* WaitStop()
{
    printf("The program works, enter 's' to stop the program\n");
    while (true)
    {
        waitStopResult = getchar();
        if (waitStopResult == 's')
        {
            pthread_cond_signal(&cond);
            return NULL;
        }
    }
}


int main()
{
    printf("Please enter X\n");

    int x;
    scanf("%d", &x);

    pthread_t threadF;
    pthread_create(&threadF, NULL, CalculateF, &x);

    pthread_t threadG;
    pthread_create(&threadG, NULL, CalculateG, &x);

    pthread_t threadWaitCancel;
    pthread_create(&threadWaitCancel, NULL, WaitStop, NULL);

    pthread_cond_wait(&cond, &lock);
    if (waitStopResult == 's')
    {
        printf("Program was stopped");
        return 0;
    }

    if (fResult == 0 || gResult == 0)
    {
        printf("Short-circuit\n");
        printf("Program was ended, result: 0");
        return 0;
    }

    pthread_cond_wait(&cond, &lock);

    if (waitStopResult == 's')
    {
        printf("Program was stopped");
        return 0;
    }

    returnableType result = (fResult * gResult);

    printf("Program was ended, result: %d", result);
    return 0;
}