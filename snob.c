#define SNOB_IMPLEMENTATION
#include "snob.h"

int main(int argc, char** argv) {
    REBUILD_THYSELF(argc, argv);

    ShellCommand cmd = {0};
    cmd_append(&cmd, "ls", "-la");
    if (!cmd_run(&cmd)) return 1;

    return 0;
}
