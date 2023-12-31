
#include "types.h"
#include "user.h"

void heavy_tesk()
{
    int i = 0;
    while (i < 1000000000)
    {
        i++;
    }
    printf(1, "done heavy\n");
}

void fork_heavy_task_child()
{
    int pid = fork();
    if (pid == 0)
    {
        printf(1, "child started\n");
        heavy_tesk();
        printf(1, "child finished\n");
        exit();
    }
    else if (pid < 0)
    {
        printf(1, "fork failed\n");
        exit();
    }
    else
    {
        // in parent
    }
}

int main(int argc, char *argv[])
{
    printf(1, "test_priority started\n");
    /*acquire_priority_lock();
    heavy_tesk();
    release_priority_lock();*/

    int pid = fork(); /*
     int pid2 = fork();
     int pid3 = fork();
     int pid4 = fork();
     int pid5 = fork();*/
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