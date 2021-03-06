#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

typedef enum send_mode_t
{
    MODE_KILL,
    MODE_SIGQUEUE,
    MODE_SIGRT,
} send_mode_t;

static const char HELP[] =
        "SO Lab4 Zad3a - Jakub Karbowski\n"
        "Usage:\n"
        "%s CATCHER_PID NUM_SIGNALS kill|sigqueue|sigrt\n";

static volatile int g_usr1_count = 0;
static volatile int g_usr2_count = 0;
static volatile int g_catcher_received = -1;

static void handler(int sig, siginfo_t *info, void *ucontext)
{
    if (sig == SIGUSR1 || sig == SIGRTMIN + 0)
        g_usr1_count++;
    else if (sig == SIGUSR2 || sig == SIGRTMIN + 1)
        g_usr2_count++;

    if (info->si_code == SI_QUEUE && sig == SIGUSR2)
        g_catcher_received = info->si_value.sival_int;
}

int main(int argc, char **argv)
{

    // ------------- ARG PARSING

    if (argc != 4)
    {
        fprintf(stderr, HELP, argv[0]);
        return -1;
    }

    char *endptr;
    pid_t catcher_pid = (pid_t) strtol(argv[1], &endptr, 10);
    if (*endptr || catcher_pid < 0)
    {
        fprintf(stderr, "Invalid catcher PID: %s\n", argv[1]);
        return -1;
    }

    int to_send = (int) strtol(argv[2], &endptr, 10);
    if (*endptr || to_send < 0)
    {
        fprintf(stderr, "Invalid N: %s\n", argv[2]);
        return -1;
    }

    send_mode_t mode;
    if (!strcmp("kill", argv[3])) mode = MODE_KILL;
    else if (!strcmp("sigqueue", argv[3])) mode = MODE_SIGQUEUE;
    else if (!strcmp("sigrt", argv[3])) mode = MODE_SIGRT;
    else
    {
        fprintf(stderr, "Invalid mode: %s\n", argv[3]);
        return -1;
    }

    // ------------- SIGNAL SETUP

    struct sigaction act = {0};
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGRTMIN + 0, &act, NULL);
    sigaction(SIGRTMIN + 1, &act, NULL);

    // ------------- SEND PING

    for (int i = 0; i < to_send; ++i)
    {
        switch (mode)
        {
            case MODE_KILL:
                kill(catcher_pid, SIGUSR1);
                break;

            case MODE_SIGQUEUE:
                sigqueue(catcher_pid, SIGUSR1, (union sigval) 0);
                break;

            case MODE_SIGRT:
                kill(catcher_pid, SIGRTMIN + 0);
                break;
        }
    }

    // ------------- SEND STOP

    switch (mode)
    {
        case MODE_KILL:
            kill(catcher_pid, SIGUSR2);
            break;

        case MODE_SIGQUEUE:
            sigqueue(catcher_pid, SIGUSR2, (union sigval) 0);
            break;

        case MODE_SIGRT:
            kill(catcher_pid, SIGRTMIN + 1);
            break;
    }

    // ------------- WAIT FOR PONG

    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGRTMIN+1);
    // block USR2 to safely access g_usr2_count
    sigprocmask(SIG_BLOCK, &set, &oldset);
    // -- critical section start
    while (!g_usr2_count)
    {
        // -- critical section end
        // restore old mask and wait for a signal
        sigsuspend(&oldset);
        // handler was executed
        // USR2 is blocked again
        // -- critical section start
    }
    // -- critical section end
    // restore original mask
    sigprocmask(SIG_SETMASK, &oldset, NULL);

    if (mode == MODE_SIGQUEUE)
        printf("Catcher got %d/%d signals\n", g_catcher_received, to_send);

    printf("Sender got %d/%d signals\n", g_usr1_count, to_send);

    return 0;
}
