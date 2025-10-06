// pipes_processes3.c
// Demonstrates three-process piping: cat scores | grep <argument> | sort

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        exit(1);
    }

    int pipe1[2]; // pipe between cat and grep
    int pipe2[2]; // pipe between grep and sort

    pipe(pipe1);
    pipe(pipe2);

    pid_t p1, p2;

    p1 = fork();

    if (p1 < 0)
    {
        perror("fork failed");
        exit(1);
    }

    // PARENT PROCESS (P1): cat scores
    if (p1 > 0)
    {
        // Fork another child for grep
        p2 = fork();

        if (p2 < 0)
        {
            perror("fork failed");
            exit(1);
        }

        // CHILD PROCESS (P2): grep <argument>
        if (p2 == 0)
        {
            pid_t p3 = fork();

            // CHILD’S CHILD (P3): sort
            if (p3 == 0)
            {
                // sort reads from pipe2[0]
                dup2(pipe2[0], STDIN_FILENO);

                // close unused ends
                close(pipe1[0]);
                close(pipe1[1]);
                close(pipe2[1]);
                close(pipe2[0]);

                char *sort_args[] = {"sort", NULL};
                execvp("sort", sort_args);
                perror("execvp sort failed");
                exit(1);
            }

            // grep reads from pipe1[0] and writes to pipe2[1]
            dup2(pipe1[0], STDIN_FILENO);
            dup2(pipe2[1], STDOUT_FILENO);

            // close all unused pipe ends
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe1[0]);
            close(pipe2[1]);

            char *grep_args[] = {"grep", argv[1], NULL};
            execvp("grep", grep_args);
            perror("execvp grep failed");
            exit(1);
        }

        // PARENT PROCESS (P1) continues: cat scores
        // cat writes to pipe1[1]
        dup2(pipe1[1], STDOUT_FILENO);

        // close unused ends
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe1[1]);

        char *cat_args[] = {"cat", "scores", NULL};
        execvp("cat", cat_args);
        perror("execvp cat failed");
        exit(1);
    }

    return 0;
}
