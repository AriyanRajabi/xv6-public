// Long-term locks for processes

#define MAX_QUEUE_SIZE 1024  // kalloc allocates 4096 bytes (1 page) which is 1024 ints
#define EMPTY_PID 2147483647 // max int in c, because we want empty ones to be sorted to the end of the array (queue)

struct sleeplock
{
  uint locked;        // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock

  // For debugging:
  char *name; // Name of lock.
  int pid;    // Process holding lock

  // lab4
  int *pid_queue;
  int is_initilized;
};
void init_p_lock(struct sleeplock *lk, char *name);
void acquire_p_lock(struct sleeplock *lk, int pid);
void release_p_lock(struct sleeplock *lk, int pid);
int holding_p_lock(struct sleeplock *lk);