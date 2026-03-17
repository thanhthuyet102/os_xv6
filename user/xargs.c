#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command>...\n");
        exit(1);
    }

    char *xargv[MAXARG];
    int xargc = 0;
    int start_idx = 1;

    if (argc >= 3 && strcmp(argv[1], "-n") == 0) {
        start_idx = 3;
    }

    if (start_idx >= argc) {
        exit(1);
    }

    for (int i = start_idx; i < argc; i++) {
        xargv[xargc] = argv[i];
        xargc++;
    }

    char buf[512];
    int buf_idx = 0;
    char c;

    while (read(0, &c, 1) == 1) {
        if (c == '\n') {
            buf[buf_idx] = '\0';

            xargv[xargc] = buf;
            xargv[xargc + 1] = 0;

            int pid = fork();
            if (pid == 0) {
                exec(xargv[0], xargv);
                fprintf(2, "exec %s failed\n", xargv[0]);
                exit(1);
            } else if (pid > 0) {
                wait(0);
            } else {
                fprintf(2, "fork failed\n");
                exit(1);
            }

            buf_idx = 0;
        } else {
            buf[buf_idx] = c;
            buf_idx++;
        }
    }

    exit(0);
}