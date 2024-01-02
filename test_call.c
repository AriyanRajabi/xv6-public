#include "types.h"
#include "user.h"

int main(void)
{
    reset_syscall_count();
    get_syscall_count();
    exit();
}