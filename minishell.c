#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_ARGS 100
#define MAX_HISTORY 100

extern char **environ;

char history[MAX_HISTORY][MAX_LINE];
int history_count = 0;

void sigint_handler(int sig)
{
    (void)sig;
    printf("\n");
    fflush(stdout);
}

void sigtstp_handler(int sig)
{
    (void)sig;
    printf("\nCtrl+Z disabled\n");
    fflush(stdout);
}

void print_prompt()
{
    char cwd[1024];
    char host[256];

    char *user = getenv("USER");
    if(user == NULL)
        user = "user";

    getcwd(cwd, sizeof(cwd));
    gethostname(host, sizeof(host));

    printf(
        "\033[1;32m%s@%s\033[0m:"
        "\033[1;34m%s\033[0m$ ",
        user,
        host,
        cwd
    );

    fflush(stdout);
}

int main()
{
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    char line[MAX_LINE];
    char original_line[MAX_LINE];
    char *args[MAX_ARGS];

    int last_status = 0;

    while(1)
    {
        print_prompt();

        if(fgets(line, sizeof(line), stdin) == NULL)
            break;

        // Save original line before any modifications
        strcpy(original_line, line);
        
        // Remove trailing newline for history
        size_t len = strlen(original_line);
        if(len > 0 && original_line[len-1] == '\n')
            original_line[len-1] = '\0';

        int i = 0;
        char *token = strtok(line, " \t\n");

        while(token != NULL && i < MAX_ARGS - 1)
        {
            args[i++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[i] = NULL;

        if(args[0] == NULL)
            continue;

        // Handle history recall (!n) BEFORE saving to history
        if(args[0][0] == '!')
        {
            int num = atoi(&args[0][1]);
            
            if(num < 1 || num > history_count)
            {
                printf("No such command in history\n");
                continue;
            }
            
            // Copy the recalled command
            strcpy(line, history[num - 1]);
            printf("%s\n", line);
            
            // Re-tokenize the recalled command
            i = 0;
            token = strtok(line, " \t\n");
            while(token != NULL && i < MAX_ARGS - 1)
            {
                args[i++] = token;
                token = strtok(NULL, " \t\n");
            }
            args[i] = NULL;
            
            if(args[0] == NULL)
                continue;
        }
        
        // Save command to history (save original_line for !n commands)
        if(history_count < MAX_HISTORY)
        {
            // If this was a !n command, save the original !n line
            if(args[0][0] == '!')
                strcpy(history[history_count], original_line);
            else
                strcpy(history[history_count], original_line);
            history_count++;
        }

        /* exit */
        if(strcmp(args[0], "exit") == 0)
        {
            printf("Goodbye!\n");
            break;
        }

        /* pwd */
        if(strcmp(args[0], "pwd") == 0)
        {
            char cwd[1024];
            if(getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("%s\n", cwd);
                last_status = 0;
            }
            else
            {
                perror("pwd");
                last_status = 1;
            }
            continue;
        }

        /* cd */
        if(strcmp(args[0], "cd") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Usage: cd directory\n");
                last_status = 1;
            }
            else
            {
                if(chdir(args[1]) != 0)
                {
                    perror("cd");
                    last_status = 1;
                }
                else
                {
                    last_status = 0;
                }
            }
            continue;
        }

        /* env */
        if(strcmp(args[0], "env") == 0)
        {
            for(char **env = environ; *env != NULL; env++)
            {
                printf("%s\n", *env);
            }
            last_status = 0;
            continue;
        }

        /* printenv */
        if(strcmp(args[0], "printenv") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Usage: printenv VARIABLE\n");
                last_status = 1;
            }
            else
            {
                char *value = getenv(args[1]);
                if(value)
                {
                    printf("%s\n", value);
                    last_status = 0;
                }
                else
                {
                    printf("Variable not found\n");
                    last_status = 1;
                }
            }
            continue;
        }

        /* set */
        if(strcmp(args[0], "set") == 0)
        {
            if(args[1] == NULL || args[2] == NULL)
            {
                printf("Usage: set VARIABLE VALUE\n");
                last_status = 1;
            }
            else
            {
                setenv(args[1], args[2], 1);
                last_status = 0;
            }
            continue;
        }

        /* status */
        if(strcmp(args[0], "status") == 0)
        {
            printf("Last exit code: %d\n", last_status);
            continue;
        }

        /* history */
        if(strcmp(args[0], "history") == 0)
        {
            for(int h = 0; h < history_count; h++)
            {
                printf("%d %s\n", h + 1, history[h]);
            }
            continue;
        }

        /* calc */
        if(strcmp(args[0], "calc") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Usage: calc [add|sub|mul|div|avg|max|min]\n");
                continue;
            }

            if(args[2] == NULL)
            {
                printf("Please provide numbers\n");
                continue;
            }

            if(strcmp(args[1], "add") == 0)
            {
                int sum = 0;
                for(int j = 2; args[j] != NULL; j++)
                    sum += atoi(args[j]);
                printf("%d\n", sum);
            }
            else if(strcmp(args[1], "sub") == 0)
            {
                int result = atoi(args[2]);
                for(int j = 3; args[j] != NULL; j++)
                    result -= atoi(args[j]);
                printf("%d\n", result);
            }
            else if(strcmp(args[1], "mul") == 0)
            {
                int result = 1;
                for(int j = 2; args[j] != NULL; j++)
                    result *= atoi(args[j]);
                printf("%d\n", result);
            }
            else if(strcmp(args[1], "div") == 0)
            {
                if(args[3] != NULL)
                {
                    printf("Usage: calc div dividend divisor\n");
                    continue;
                }
                int dividend = atoi(args[2]);
                int divisor = atoi(args[3]);
                if(divisor == 0)
                {
                    printf("Error: Division by zero\n");
                }
                else
                {
                    printf("%d\n", dividend / divisor);
                }
            }
            else if(strcmp(args[1], "avg") == 0)
            {
                int sum = 0;
                int count = 0;
                for(int j = 2; args[j] != NULL; j++)
                {
                    sum += atoi(args[j]);
                    count++;
                }
                printf("%.2f\n", (float)sum / count);
            }
            else if(strcmp(args[1], "max") == 0)
            {
                int max = atoi(args[2]);
                for(int j = 3; args[j] != NULL; j++)
                {
                    int n = atoi(args[j]);
                    if(n > max)
                        max = n;
                }
                printf("%d\n", max);
            }
            else if(strcmp(args[1], "min") == 0)
            {
                int min = atoi(args[2]);
                for(int j = 3; args[j] != NULL; j++)
                {
                    int n = atoi(args[j]);
                    if(n < min)
                        min = n;
                }
                printf("%d\n", min);
            }
            else
            {
                printf("Unknown calc operation\n");
            }
            continue;
        }

        /* help - FIXED */
        if(strcmp(args[0], "help") == 0)
        {
            printf("\033[1;33m=== MyShell Built-in Commands ===\033[0m\n");
            printf("  \033[1;32mcd\033[0m <dir>       - Change directory\n");
            printf("  \033[1;32mpwd\033[0m            - Print working directory\n");
            printf("  \033[1;32menv\033[0m            - Show all environment variables\n");
            printf("  \033[1;32mprintenv\033[0m <var> - Show specific environment variable\n");
            printf("  \033[1;32mset\033[0m <var> <val> - Set environment variable\n");
            printf("  \033[1;32mhistory\033[0m        - Show command history\n");
            printf("  \033[1;32mstatus\033[0m         - Show last command exit status\n");
            printf("  \033[1;32mcalc\033[0m [add|sub|mul|div|avg|max|min] <numbers>\n");
            printf("  \033[1;32mweather\033[0m <city>  - Get weather (requires weather.py)\n");
            printf("  \033[1;32mnews\033[0m <category> - Get news (requires news.py)\n");
            printf("  \033[1;32mhelp\033[0m            - Show this help message\n");
            printf("  \033[1;32mexit\033[0m            - Exit shell\n");
            printf("\n\033[1;33mFeatures:\033[0m\n");
            printf("  • Pipe support: cmd1 | cmd2\n");
            printf("  • Redirection: <, >, >>\n");
            printf("  • Command history: !n\n");
            printf("  • Signal handling: Ctrl+C, Ctrl+Z disabled\n");
            continue;
        }

        /* weather */
        if(strcmp(args[0], "weather") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Usage: weather city\n");
                continue;
            }

            char command[512];
            snprintf(command, sizeof(command), "python3 weather.py \"%s\" 2>/dev/null", args[1]);
            
            if(system(command) != 0)
            {   
                printf("Error: weather.py not found or failed. Make sure python3 and weather.py exist.\n");
            }
            continue;
        }

        /* news */
        if(strcmp(args[0], "news") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Usage: news [world|india|technology|sports]\n");
                continue;
            }

            char command[512];
            snprintf(command, sizeof(command), "python3 news.py %s 2>/dev/null", args[1]);
            
            if(system(command) != 0)
            {
                printf("Error: news.py not found or failed. Make sure python3 and news.py exist.\n");
            }
            continue;
        }

        /* single pipe support */
        int pipe_pos = -1;
        for(i = 0; args[i] != NULL; i++)
        {
            if(strcmp(args[i], "|") == 0)
            {
                pipe_pos = i;
                args[i] = NULL;
                break;
            }
        }

        if(pipe_pos != -1)
        {
            int fd[2];
            if(pipe(fd) < 0)
            {
                perror("pipe");
                continue;
            }

            pid_t pid1 = fork();
            if(pid1 == 0)
            {
                signal(SIGINT, SIG_DFL);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(args[0], args);
                fprintf(stderr, "myshell: command not found: %s\n", args[0]);
                exit(127);
            }

            pid_t pid2 = fork();
            if(pid2 == 0)
            {
                signal(SIGINT, SIG_DFL);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(args[pipe_pos + 1], &args[pipe_pos + 1]);
                fprintf(stderr, "myshell: command not found: %s\n", args[pipe_pos + 1]);
                exit(127);
            }

            close(fd[0]);
            close(fd[1]);
            
            int status1, status2;
            waitpid(pid1, &status1, 0);
            waitpid(pid2, &status2, 0);
            
            if(WIFEXITED(status2))
                last_status = WEXITSTATUS(status2);
            
            continue;
        }

        /* redirection */
        char *input_file = NULL;
        char *output_file = NULL;
        int append = 0;

        for(i = 0; args[i] != NULL; i++)
        {
            if(strcmp(args[i], "<") == 0)
            {
                input_file = args[i + 1];
                args[i] = NULL;
            }
            else if(strcmp(args[i], ">") == 0)
            {
                output_file = args[i + 1];
                append = 0;
                args[i] = NULL;
            }
            else if(strcmp(args[i], ">>") == 0)
            {
                output_file = args[i + 1];
                append = 1;
                args[i] = NULL;
            }
        }

        pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork");
            last_status = 1;
        }
        else if(pid == 0)
        {
            signal(SIGINT, SIG_DFL);

            if(input_file != NULL)
            {
                int fd = open(input_file, O_RDONLY);
                if(fd < 0)
                {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if(output_file != NULL)
            {
                int fd;
                if(append)
                    fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else
                    fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                
                if(fd < 0)
                {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args);
            fprintf(stderr, "myshell: command not found: %s\n", args[0]);
            exit(127);
        }
        else
        {
            int status;
            wait(&status);
            if(WIFEXITED(status))
                last_status = WEXITSTATUS(status);
        }
    }

    return 0;
}