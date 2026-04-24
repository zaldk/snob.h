#define SNOB_IMPLEMENTATION
#include "snob.h"

#define BUILD_DIR ".build"

int main(int argc, char** argv) {
    REBUILD_THYSELF(argc, argv);

    init_ignored_dir(BUILD_DIR, ".gitignore", 1);

    ShellCommand cmd = {0};
    cmd_append(&cmd, "gcc", "-o", BUILD_DIR"/hello.exe", "hello.c");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, BUILD_DIR"/hello.exe");
    if (!cmd_run(&cmd)) return 1;

    return 0;
}
