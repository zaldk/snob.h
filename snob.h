#ifndef SNOB_H
#define SNOB_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

typedef int ProcessID;

typedef struct {
    const char** items;
    size_t len, cap;
} ShellCommand;

void cmd_append_fn(ShellCommand* cmd, size_t argc, const char** argv);
int cmd_run(ShellCommand* cmd);
int cmd_await(ProcessID pid);

void rebuild_thyself(int argc, char** argv, const char* path_input);
int rebuild_needed(const char* path_input, const char* path_output);

#define SHIFT(argc, argv) (--(argc), *(argv)++)

#define REBUILD_THYSELF(ARGC, ARGV) rebuild_thyself((ARGC), (ARGV), __FILE__)

#define da_append(xs, x) \
    do { \
        if ((xs)->len >= (xs)->cap) { \
            (xs)->cap *= 2; \
            if ((xs)->cap == 0) (xs)->cap = 64; \
            (xs)->items = realloc((xs)->items, (xs)->cap*sizeof((xs)->items[0])); \
        } \
        (xs)->items[(xs)->len++] = (x); \
    } while (0)

#define cmd_append(cmd, ...) \
    cmd_append_fn((cmd), \
        sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*), \
        (const char*[]){__VA_ARGS__} \
    )

#define cmd_append_array(cmd, xs, n) \
    do { \
        if ((cmd)->cap - (cmd)->len < (n)) { \
            if ((cmd)->cap == 0) (cmd)->cap = 64; \
            while (((cmd)->cap - (cmd)->len) < (n)) (cmd)->cap *= 2; \
            (cmd)->items = realloc((cmd)->items, (cmd)->cap*sizeof((cmd)->items[0])); \
        } \
        memcpy((cmd)->items+(cmd)->len, (xs), (n)*sizeof((xs)[0])); \
        (cmd)->len += n; \
    } while (0)

#endif /* SNOB_H */



#ifdef SNOB_IMPLEMENTATION
#ifndef SNOB_IMPLEMENTATION_GUARD
#define SNOB_IMPLEMENTATION_GUARD

// stolen from github:tsoding/nob.h
void rebuild_thyself(int argc, char** argv, const char* path_input) {
    const char* path_output = SHIFT(argc, argv);
    int ret = rebuild_needed(path_input, path_output);
    if (ret < 0) exit(1); // error occured
    if (ret == 0) return; // rebuild is not needed

    char path_output_old[1024] = {0};
    snprintf(path_output_old, 1024, "%s.old", path_output);
    if (rename(path_output, path_output_old) != 0) {
        printf("[ERROR] Could not rename: %s\n", strerror(errno));
        exit(1);
    }

    ShellCommand cmd = {0};
    cmd_append(&cmd, "cc", "-o", path_output, path_input);
    if (!cmd_run(&cmd)) {
        printf("[ERROR] Could not rebuild.\n");
        rename(path_output_old, path_output);
    }

    remove(path_output_old);

    cmd_append(&cmd, path_output);
    cmd_append_array(&cmd, argv, argc);
    if (!cmd_run(&cmd)) exit(2);
    exit(0);
}

int rebuild_needed(const char* path_input, const char* path_output) {
    struct stat statbuf = {0};

    if (stat(path_output, &statbuf) < 0) {
        // NOTE: if output does not exist it 100% must be rebuilt
        if (errno == ENOENT) return 1;
        printf("[ERROR] could not stat %s: %s\n", path_output, strerror(errno));
        return -1;
    }
    time_t path_output_time = statbuf.st_mtime;

    if (stat(path_input, &statbuf) < 0) {
        // NOTE: non-existing input is an error cause it is needed for building in the first place
        printf("[ERROR] could not stat %s: %s\n", path_input, strerror(errno));
        return -1;
    }
    time_t path_input_time = statbuf.st_mtime;
    if (path_input_time > path_output_time) return 1;

    return 0;
}

void cmd_append_fn(ShellCommand* cmd, size_t argc, const char** argv) {
    for (int i = 0; i < argc; i ++) {
        da_append(cmd, argv[i]);
    }
}

int cmd_run(ShellCommand* cmd) {
    printf("[DEBUG] Running:");
    for (int i = 0; i < cmd->len; i++) {
        printf(" %s", cmd->items[i]);
    }
    printf("\n");

    pid_t pid = fork();
    if (pid < 0) {
        printf("[ERROR] Could not fork.\n");
        return 0;
    }

    if (pid == 0) {
        ShellCommand cmd_null = {0};
        cmd_append_array(&cmd_null, cmd->items, cmd->len);
        cmd_append(&cmd_null, NULL);

        if (execvp(cmd_null.items[0], (char * const*) cmd_null.items) < 0) {
            printf("[ERROR] Could not exec child process for %s: %s\n", cmd_null.items[0], strerror(errno));
            exit(1);
        }

        printf("[DEBUG] UNREACHABLE\n");
        exit(69);
    }

    int ret = cmd_await(pid);
    if (!ret) {
        printf("[ERROR] Could not await process pid=%d\n", pid);
        return 0;
    }

    cmd->len = 0;

    return 1;
}

int cmd_await(ProcessID pid) {
    for (;;) {
        int wstatus = 0;
        if (waitpid(pid, &wstatus, 0) < 0) {
            printf("[ERROR] could not wait on command (pid %d): %s\n", pid, strerror(errno));
            return 0;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                printf("[ERROR] command exited with exit code %d\n", exit_status);
                return 0;
            }
            break;
        }

        if (WIFSIGNALED(wstatus)) {
            printf("[ERROR] command process was terminated by signal %d\n", WTERMSIG(wstatus));
            return 0;
        }
    }
    return 1;
}

#endif /* SNOB_IMPLEMENTATION_GUARD */
#endif /* SNOB_IMPLEMENTATION */
