# snob.h - ~Next~ Nextest generation of the NoBuild!

This library is the nextest generation of the NoBuild idea.
"snob" stands for "Simplified NO Build" and the literal meaning:

    snob /snŏb/ noun
    - One who despises, ignores, or is patronizing to those he or she considers inferior.
    - One who is convinced of his or her superiority in matters of taste or intellect.
    - A vulgar person who affects to be better, richer, or more fashionable, than he really is; a vulgar upstart; one who apes his superiors.

But in all seriousness, this is just a simplified version of the original
[nob.h](https://github.com/tsoding/nob.h)
that has only the bare minimum i needed for my own projects.

Go check out the full library!

## Usage

You only need the `snob.h` file (it is written in STB-like style), then you can create a file like `snob.c`:

```c
#define SNOB_IMPLEMENTATION // *MUST* be defined before the include - thanks to the STB-style
#include "snob.h"

int main(int argc, char** argv) {
    // with this you only need to compile the build script once,
    // then future invocations of the executable will automatically
    // rebuild the build file, if there were changes.
    REBUILD_THYSELF(argc, argv);

    // example code
    ShellCommand cmd = {0};
    cmd_append(&cmd, "ls", "-la");
    if (!cmd_run(&cmd)) return 1;

    return 0;
}
```

## TODO

- Support other OS (currently only linux/posix is supported)
- Add prefixes to minimize possible name collisions
- Write docs in the file
- Maybe introduce versions for the library?
