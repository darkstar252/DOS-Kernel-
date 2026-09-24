# First source patch: EXEC SFT ownership

The concrete source patch is:

- [0001-task-close-exec-sft-once.patch](../patches/0001-task-close-exec-sft-once.patch)
- [kernel/task.c in the branch](https://github.com/darkstar252/DOS-Kernel-/blob/win30-486-vga/kernel/task.c)

## What it changes

`DosExec()` opens the executable SFT and already closes it after
`DosComLoader()` or `DosExeLoader()` returns. Both loader functions also
closed the same SFT internally. The patch removes the two loader-side
closes and leaves ownership with `DosExec()`.

This keeps the change limited to the EXEC path and covers COM, MZ EXE,
LOAD, OVERLAY, and loader-error returns without changing the memory
allocator or interrupt ABI.

## Applying it locally

From the repository root on a system with `patch`:

```sh
patch -p1 < patches/0001-task-close-exec-sft-once.patch
```

Then inspect the result:

```sh
git diff -- kernel/task.c
git diff --check
```

The expected diff removes only the two `DosCloseSft(fd, FALSE);` calls
inside `DosComLoader()` and `DosExeLoader()`. `DosExec()` keeps its close.

Build and test the resulting source in the 86Box DOS toolchain. Do not
commit generated binaries, disk images, ROMs, or Windows installation media.
