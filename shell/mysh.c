/*
 * mysh.c - A simple command-line shell supporting piping and redirection.
 *
 * Usage: ./mysh
 * Reads one line of commands from stdin, executes them, then exits.
 *
 * Grammar:
 *   start   ::= pipes "\n"
 *   pipes   ::= command ("|" command)*
 *   command ::= program ("<" STRING)? (">" STRING)?
 *   program ::= STRING STRING*
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS   64
#define MAX_CMDS   16
#define MAX_LINE   4096
#define MAX_TOKENS (MAX_ARGS * MAX_CMDS)

typedef struct {
    char *argv[MAX_ARGS]; /* program + arguments, NULL-terminated */
    int   argc;
    char *infile;         /* input redirection file, or NULL */
    char *outfile;        /* output redirection file, or NULL */
} Command;

/*
 * tokenize - Split line into whitespace-separated tokens in-place.
 * Modifies line by inserting NUL bytes.
 * Returns the number of tokens stored in tokens[].
 */
static int tokenize(char *line, char **tokens, int max_tokens)
{
    int n = 0;
    char *p = line;

    while (*p && n < max_tokens - 1) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == '\0' || *p == '\n')
            break;
        tokens[n++] = p;
        while (*p && *p != ' ' && *p != '\t' && *p != '\n')
            p++;
        if (*p)
            *p++ = '\0';
    }
    tokens[n] = NULL;
    return n;
}

/*
 * parse_command - Parse one command from tokens[0..ntokens-1].
 * Stops at a "|" token or end of tokens.
 * Returns the number of tokens consumed (not including a trailing "|").
 */
static int parse_command(char **tokens, int ntokens, Command *cmd)
{
    int i = 0;
    cmd->argc   = 0;
    cmd->infile  = NULL;
    cmd->outfile = NULL;

    while (i < ntokens && strcmp(tokens[i], "|") != 0) {
        if (strcmp(tokens[i], "<") == 0) {
            i++;
            if (i < ntokens && strcmp(tokens[i], "|") != 0)
                cmd->infile = tokens[i++];
        } else if (strcmp(tokens[i], ">") == 0) {
            i++;
            if (i < ntokens && strcmp(tokens[i], "|") != 0)
                cmd->outfile = tokens[i++];
        } else {
            if (cmd->argc < MAX_ARGS - 1)
                cmd->argv[cmd->argc++] = tokens[i];
            i++;
        }
    }
    cmd->argv[cmd->argc] = NULL;
    return i;
}

/*
 * execute_pipeline - Fork and exec all commands, connecting them with pipes.
 * Handles input/output redirection per command.
 */
static void execute_pipeline(Command *cmds, int ncmds)
{
    int   pipes[MAX_CMDS - 1][2];
    pid_t pids[MAX_CMDS];

    /* Create all needed pipes up front. */
    for (int j = 0; j < ncmds - 1; j++) {
        if (pipe(pipes[j]) < 0) {
            perror("pipe");
            return;
        }
    }

    for (int j = 0; j < ncmds; j++) {
        pids[j] = fork();
        if (pids[j] < 0) {
            perror("fork");
            /* Close remaining pipe ends and wait for already-started children. */
            for (int k = j; k < ncmds - 1; k++) {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }
            for (int k = 0; k < j; k++)
                waitpid(pids[k], NULL, 0);
            return;
        }

        if (pids[j] == 0) {
            /* ----- child process ----- */

            /* Connect stdin: pipe from previous command or file redirect. */
            if (j > 0) {
                if (dup2(pipes[j - 1][0], STDIN_FILENO) < 0) {
                    perror("dup2 stdin pipe");
                    exit(1);
                }
            }
            if (cmds[j].infile) {
                int fd = open(cmds[j].infile, O_RDONLY);
                if (fd < 0) {
                    perror(cmds[j].infile);
                    exit(1);
                }
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("dup2 stdin file");
                    exit(1);
                }
                close(fd);
            }

            /* Connect stdout: pipe to next command or file redirect. */
            if (j < ncmds - 1) {
                if (dup2(pipes[j][1], STDOUT_FILENO) < 0) {
                    perror("dup2 stdout pipe");
                    exit(1);
                }
            }
            if (cmds[j].outfile) {
                int fd = open(cmds[j].outfile,
                              O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror(cmds[j].outfile);
                    exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2 stdout file");
                    exit(1);
                }
                close(fd);
            }

            /* Close all pipe file descriptors in the child. */
            for (int k = 0; k < ncmds - 1; k++) {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }

            execvp(cmds[j].argv[0], cmds[j].argv);
            perror(cmds[j].argv[0]);
            exit(1);
        }
    }

    /* Parent: close all pipe ends. */
    for (int j = 0; j < ncmds - 1; j++) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    /* Wait for all children. */
    for (int j = 0; j < ncmds; j++)
        waitpid(pids[j], NULL, 0);
}

int main(void)
{
    char   line[MAX_LINE];
    char  *tokens[MAX_TOKENS];
    Command cmds[MAX_CMDS];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        /* Strip trailing newline. */
        line[strcspn(line, "\n")] = '\0';

        int ntokens = tokenize(line, tokens, MAX_TOKENS);
        if (ntokens == 0)
            continue;

        /* Parse commands separated by "|". */
        int ncmds = 0;
        int i = 0;
        while (i < ntokens && ncmds < MAX_CMDS) {
            int consumed = parse_command(tokens + i, ntokens - i,
                                         &cmds[ncmds]);
            i += consumed;
            if (cmds[ncmds].argc > 0)
                ncmds++;
            /* Skip the "|" separator. */
            if (i < ntokens && strcmp(tokens[i], "|") == 0)
                i++;
        }

        if (ncmds == 0)
            continue;

        execute_pipeline(cmds, ncmds);
    }

    return 0;
}
