#include "types.h"
#include "user.h"
#include "fcntl.h"

void write_to_file()
{
    int fd = open("output.txt", O_CREATE | O_WRONLY);
    if (fd < 0)
    {
        printf(1, "Failed to open file\n");
        exit();
    }
    write(fd, "Child process wrote", 19);
    close(fd);
    exit();
}
int main(void)
{
    reset_syscall_count();
    for (int i = 0; i < 10; i++)
    {
        int pid = fork();

        if (pid < 0)
        {
            printf(1, "fork failed\n");
            exit();
        }
        else if (pid == 0)
        {
            write_to_file();
        }
    }
    for (int i = 0; i < 10; i++)
    {
        wait();
    }
    get_syscall_count();
    exit();
}