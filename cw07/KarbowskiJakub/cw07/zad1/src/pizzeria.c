#include "pizzeria.h"

#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define PIZZERIA_SHM_BASE 31253242

pizzeria_t *pizzeria;

int pizzeria_create()
{
    int mem = shmget(PIZZERIA_SHM_BASE + getpid(), sizeof *pizzeria, IPC_CREAT | 0600);
    pizzeria = shmat(mem, 0, 0);

    pizzeria->shm = mem;

    pizzeria->furnace.write_head = 0;
    pizzeria->furnace.read_head = 0;
    pizzeria->furnace.usage = 0;

    pizzeria->table.write_head = 0;
    pizzeria->table.read_head = 0;
    pizzeria->table.usage = 0;

    pizzeria->sem_set = semget(IPC_PRIVATE, PIZZERIA_SEM_MAX, IPC_CREAT | 0600);

    // unlocked
    semctl(pizzeria->sem_set, FURNACE_LOCK_SEM, SETVAL, 1);
    // no used
    semctl(pizzeria->sem_set, FURNACE_USED_SEM, SETVAL, 0);
    // all free
    semctl(pizzeria->sem_set, FURNACE_FREE_SEM, SETVAL, FURNACE_SIZE);

    semctl(pizzeria->sem_set, TABLE_LOCK_SEM, SETVAL, 1);
    semctl(pizzeria->sem_set, TABLE_USED_SEM, SETVAL, 0);
    semctl(pizzeria->sem_set, TABLE_FREE_SEM, SETVAL, TABLE_SIZE);

    return 0;
}

int pizzeria_delete()
{
    semctl(pizzeria->sem_set, PIZZERIA_SEM_MAX, IPC_RMID);
    shmctl(pizzeria->shm, IPC_RMID, 0);
    return 0;
}

int pizzeria_load()
{
    int mem = shmget(PIZZERIA_SHM_BASE + getppid(), 0, 0);
    pizzeria = shmat(mem, 0, 0);
    return 0;
}
