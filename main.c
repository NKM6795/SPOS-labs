#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>

#include "demofuncs.h"

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

#define INIT_RESULT 1000000000
#define ESC_KEY 27
#define INIT_STOP_RESULT 100
#define CHAR_TO_EXIT 2
#define CHAR_TO_SLEEP 3
#define WAITING_FOR_USER_INPUT 4

typedef int returnableType;


returnableType fResult = INIT_RESULT;
returnableType gResult = INIT_RESULT;
char waitStopResult = INIT_STOP_RESULT;


returnableType FFunction(int x)
{
    return f_func_imul(x);
}

returnableType GFunction(int x)
{
    return g_func_imul(x);
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
#ifdef WAIT_ESC
    printw("Click 'esc' to stop the program\n");
    refresh();
    noecho();
    keypad(stdscr, true);
    while (true)
    {
        char current_char = getch();
        if (current_char == ESC_KEY)
        {
            waitStopResult = CHAR_TO_EXIT;
            pthread_cond_signal(&cond);
            return NULL;
        }
    }
#else
    while (true)
    {
        sleep(2);
        waitStopResult = WAITING_FOR_USER_INPUT;

        printw("The program is working, click\n\t'a' to continue\n\t'b' to continue without prompt\n\t'c' to cancel\n");
        refresh();
        while (true)
        {
            char current_char = getch();
            printw("\n");
            refresh();

            if (current_char == 'c')
            {
                waitStopResult = CHAR_TO_EXIT;
                pthread_cond_signal(&cond);
                return NULL;
            }
            else if (current_char == 'b')
            {
                waitStopResult = CHAR_TO_SLEEP;
                pthread_cond_signal(&cond);
                return NULL;
            }
            else if (current_char == 'a')
            {
                waitStopResult = INIT_STOP_RESULT;
                pthread_cond_signal(&cond);
                break;
            }
        }
    }
#endif
}

void stop_program()
{
    printw("Program was stopped\n");
    if (fResult == INIT_RESULT)
    {
        printw("F function was not calculated\n");
    }
    if (gResult == INIT_RESULT)
    {
        printw("G function was not calculated\n");
    }
    printw("Result: undefined\n");
    refresh();
}

void wait_result()
{
    while (true)
    {
        pthread_cond_wait(&cond, &lock);
        if (waitStopResult == WAITING_FOR_USER_INPUT)
        {
            continue;
        }

        if (fResult == 0 || gResult == 0)
        {
            printw("Result: 0\n");
            return;
        }

        if (fResult != INIT_RESULT && gResult != INIT_RESULT)
        {
            returnableType result = (fResult * gResult);
            printw("Result: %d\n", result);
            return;
        }

        if (waitStopResult == CHAR_TO_EXIT)
        {
            stop_program();
            return;
        }
    }
}

int main()
{
    initscr();

    pthread_t threadF;
    pthread_t threadG;
    pthread_t threadWaitCancel;

    while (true)
    {
        printw("\nPlease enter X (-1 to exit)\n");
        refresh();

        int x;
        scanw("%d", &x);

        clear();
        printw("\nProgram was started with X  = %d\n", x);
        refresh();

        if (x == -1)
        {
            break;
        }

        fResult = INIT_RESULT;
        gResult = INIT_RESULT;
        waitStopResult = INIT_STOP_RESULT;

        if (pthread_create(&threadF, NULL, CalculateF, &x))
        {
            printw("Error creating F function\n");
        }
        if (pthread_create(&threadG, NULL, CalculateG, &x))
        {
            printw("Error creating G function\n");
        }
        if (pthread_create(&threadWaitCancel, NULL, WaitStop, NULL))
        {
            printw("Error creating WaitStop function\n");
        }
        refresh();

        pthread_mutex_lock(&lock);

        wait_result();
        echo();
        keypad(stdscr, false);
        refresh();

        pthread_mutex_unlock(&lock);

        if (fResult == INIT_RESULT && pthread_cancel(threadF))
        {
            printw("Error canceling F function\n");
        }
        if (gResult == INIT_RESULT && pthread_cancel(threadG))
        {
            printw("Error canceling G function\n");
        }
        if (waitStopResult == INIT_STOP_RESULT && pthread_cancel(threadWaitCancel))
        {
            printw("Error canceling WaitStop function\n");
        }
        refresh();
    }
    
    endwin();
    return 0;
}