#include "types.h"
#include "user.h"
#define ITERATIONS 10000000000000

void heavy_tesk()
{
    acquire_priority_lock();
    long long int i = 0;
    while (i < ITERATIONS)
    {
        i++;

        long long int j = 0;
        while (j < ITERATIONS)
        {
            j++;
            long long int k = 0;
            while (k < ITERATIONS)
            {
                k++;
            }
        }
    }
    release_priority_lock();
}

int main(int argc, char *argv[])
{
    int pid = fork();
    if (pid == 0)
    {
        heavy_tesk();
        exit();
    }
    else if (pid < 0)
    {
        printf(1, "fork failed\n");
        exit();
    }
    else
    {
        int pid = fork();
        if (pid == 0)
        {
            heavy_tesk();
            exit();
        }
        else if (pid > 0)
        {
            int pid = fork();
            if (pid == 0)
            {
                heavy_tesk();
                exit();
            }
        }
        else // pid < 0
        {
            printf(1, "fork failed\n");
            exit();
        }
        // in parent
    }
    wait();
    wait();
    wait();
    exit();
}