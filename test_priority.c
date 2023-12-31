
#include "types.h"
#include "user.h"
#define ITERATIONS 300000000000

void heavy_tesk()
{
    long long int i = 0;
    while (i < ITERATIONS)
    {
        i++;
    }
}

int main(int argc, char *argv[])
{
    printf(1, "test_priority started\n");

    int pid = fork();
    if (pid == 0)
    {
        acquire_priority_lock();
        heavy_tesk();
        release_priority_lock();
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
            acquire_priority_lock();
            heavy_tesk();
            release_priority_lock();
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

                acquire_priority_lock();
                heavy_tesk();
                release_priority_lock();
                exit();
            }
        }
        // in parent
    }
    wait();
    wait();
    wait();
    exit();
}