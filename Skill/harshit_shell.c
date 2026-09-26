/*
 * HARSHIT SHELL
 * Compile: gcc harshit_shell.c -o harshit-shell
 * Run:     ./harshit-shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100

void remove_newline(char *str)
{
    str[strcspn(str, "\n")] = '\0';
}

void show_prompt()
{
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("harshit-shell:%s$ ", cwd);
    else
        printf("harshit-shell$ ");

    fflush(stdout);
}

void execute_command(char **args)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
            perror("harshit-shell");

        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

void execute_background(char **args)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
            perror("harshit-shell");

        exit(EXIT_FAILURE);
    }

    printf("[Background process started: %d]\n", pid);
}

int builtin_command(char **args)
{
    if (args[0] == NULL)
        return 1;

    if (strcmp(args[0], "exit") == 0)
    {
        printf("Goodbye from Harshit Shell!\n");
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            char *home = getenv("HOME");

            if (home != NULL)
                chdir(home);
        }
        else if (chdir(args[1]) != 0)
        {
            perror("cd");
        }

        return 1;
    }

    if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s\n", cwd);
        else
            perror("pwd");

        return 1;
    }

    if (strcmp(args[0], "help") == 0)
    {
        printf("\n========== HARSHIT SHELL ==========\n");
        printf("Built-in commands:\n");
        printf("  cd <directory>     Change directory\n");
        printf("  pwd                Show current directory\n");
        printf("  help               Show this help\n");
        printf("  exit               Exit the shell\n");

        printf("\nSupported features:\n");
        printf("  Normal commands    ls, cat, gcc, etc.\n");
        printf("  Arguments          ls -l\n");
        printf("  Pipes              ls | grep .c\n");
        printf("  Redirection        ls > file.txt\n");
        printf("  Append             ls >> file.txt\n");
        printf("  Input              sort < file.txt\n");
        printf("  Background         command &\n");
        printf("===================================\n\n");

        return 1;
    }

    return 0;
}

void execute_redirect(char **args)
{
    int input_redirect = -1;
    int output_redirect = -1;
    int append_redirect = -1;

    int i = 0;

    while (args[i] != NULL)
    {
        if (strcmp(args[i], "<") == 0)
        {
            if (args[i + 1] == NULL)
            {
                printf("Missing input file\n");
                return;
            }

            input_redirect = open(args[i + 1], O_RDONLY);

            if (input_redirect < 0)
            {
                perror("open");
                return;
            }

            args[i] = NULL;
            break;
        }

        if (strcmp(args[i], ">") == 0)
        {
            if (args[i + 1] == NULL)
            {
                printf("Missing output file\n");
                return;
            }

            output_redirect = open(
                args[i + 1],
                O_WRONLY | O_CREAT | O_TRUNC,
                0644
            );

            if (output_redirect < 0)
            {
                perror("open");
                return;
            }

            args[i] = NULL;
            break;
        }

        if (strcmp(args[i], ">>") == 0)
        {
            if (args[i + 1] == NULL)
            {
                printf("Missing output file\n");
                return;
            }

            append_redirect = open(
                args[i + 1],
                O_WRONLY | O_CREAT | O_APPEND,
                0644
            );

            if (append_redirect < 0)
            {
                perror("open");
                return;
            }

            args[i] = NULL;
            break;
        }

        i++;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        if (input_redirect != -1)
        {
            dup2(input_redirect, STDIN_FILENO);
            close(input_redirect);
        }

        if (output_redirect != -1)
        {
            dup2(output_redirect, STDOUT_FILENO);
            close(output_redirect);
        }

        if (append_redirect != -1)
        {
            dup2(append_redirect, STDOUT_FILENO);
            close(append_redirect);
        }

        execvp(args[0], args);

        perror("harshit-shell");
        exit(EXIT_FAILURE);
    }

    waitpid(pid, NULL, 0);
}

void execute_pipe(char **left, char **right)
{
    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        return;
    }

    if (pid1 == 0)
    {
        close(pipefd[0]);

        dup2(pipefd[1], STDOUT_FILENO);

        close(pipefd[1]);

        execvp(left[0], left);

        perror("harshit-shell");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();

    if (pid2 < 0)
    {
        perror("fork");
        return;
    }

    if (pid2 == 0)
    {
        close(pipefd[1]);

        dup2(pipefd[0], STDIN_FILENO);

        close(pipefd[0]);

        execvp(right[0], right);

        perror("harshit-shell");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int parse_command(char *input, char **args)
{
    int count = 0;

    char *token = strtok(input, " \t");

    while (token != NULL && count < MAX_ARGS - 1)
    {
        args[count++] = token;
        token = strtok(NULL, " \t");
    }

    args[count] = NULL;

    return count;
}

int main()
{
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    printf("\n");
    printf("========================================\n");
    printf("        WELCOME TO HARSHIT SHELL        \n");
    printf("========================================\n");
    printf("Type 'help' to see available commands.\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        show_prompt();

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }

        remove_newline(input);

        if (strlen(input) == 0)
            continue;

        parse_command(input, args);

        if (args[0] == NULL)
            continue;

        if (builtin_command(args))
            continue;

        /* Check for pipe */
        int pipe_position = -1;

        for (int i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], "|") == 0)
            {
                pipe_position = i;
                break;
            }
        }

        if (pipe_position != -1)
        {
            char *left[MAX_ARGS];
            char *right[MAX_ARGS];

            int i;

            for (i = 0; i < pipe_position; i++)
                left[i] = args[i];

            left[i] = NULL;

            int j = 0;

            for (i = pipe_position + 1; args[i] != NULL; i++)
                right[j++] = args[i];

            right[j] = NULL;

            if (left[0] != NULL && right[0] != NULL)
                execute_pipe(left, right);

            continue;
        }

        /* Check for background execution */
        int last = 0;

        while (args[last] != NULL)
            last++;

        if (last > 0 && strcmp(args[last - 1], "&") == 0)
        {
            args[last - 1] = NULL;

            if (args[0] != NULL)
                execute_background(args);

            continue;
        }

        /* Check for redirection */
        int has_redirect = 0;

        for (int i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], "<") == 0 ||
                strcmp(args[i], ">") == 0 ||
                strcmp(args[i], ">>") == 0)
            {
                has_redirect = 1;
                break;
            }
        }

        if (has_redirect)
        {
            execute_redirect(args);
            continue;
        }

        /* Normal command */
        execute_command(args);
    }

    return 0;
}
