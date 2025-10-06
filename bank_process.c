#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void ChildProcess(int []);

int main()
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    srand(time(NULL));

    // Create shared memory for two integers: BankAccount and Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }

    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if ((long) ShmPTR == -1) {
        printf("*** shmat error ***\n");
        exit(1);
    }

    // Initialize shared memory
    ShmPTR[0] = 0;  // BankAccount
    ShmPTR[1] = 0;  // Turn (0 = parent, 1 = child)

    printf("Parent initializes BankAccount = %d and Turn = %d\n", ShmPTR[0], ShmPTR[1]);
    printf("Parent is about to fork a child process...\n");

    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    } 
    else if (pid == 0) {
        ChildProcess(ShmPTR);
        exit(0);
    } 
    else {
        // ---------- Parent process (Dear Old Dad) ----------
        int i, account, balance;

        for (i = 0; i < 25; i++) {
            sleep(rand() % 6); // Sleep 0–5 sec

            while (ShmPTR[1] != 0); // Wait for Turn == 0

            account = ShmPTR[0];
            if (account <= 100) {
                balance = rand() % 100;
                if (balance % 2 == 0) {
                    account += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
            }

            ShmPTR[0] = account;
            ShmPTR[1] = 1; // Give turn to Child
        }

        wait(&status);
        printf("Parent: Child process has completed.\n");

        shmdt((void *) ShmPTR);
        shmctl(ShmID, IPC_RMID, NULL);
        printf("Parent: Shared memory removed. Exiting.\n");
        exit(0);
    }
}

void ChildProcess(int SharedMem[])
{
    int i, account, balance;

    for (i = 0; i < 25; i++) {
        sleep(rand() % 6); // Sleep 0–5 sec

        while (SharedMem[1] != 1); // Wait for Turn == 1

        account = SharedMem[0];
        balance = rand() % 50; // Needs between 0–49

        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        SharedMem[0] = account;
        SharedMem[1] = 0; // Give turn back to Dad
    }

    printf("Child: Completed all transactions.\n");
}
