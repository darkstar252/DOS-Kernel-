# Windows 3.0 real-mode compatibility work

This branch targets Windows 3.0 real mode on a 486/VGA machine in 86Box.

## First loader finding

The DOS executable loader is the first compatibility boundary to test. `DosExec()` in `kernel/task.c` invokes `DosComLoader()` or `DosExeLoader()` and then closes the SFT. Both loader paths currently also close the SFT internally. This creates a double-close path for successful COM and EXE loads.

The first behavioral patch should make file-handle ownership explicit: `DosExec()` owns the SFT opened for the execution request, and the loader functions must not close that SFT. Error paths should return through `DosExec()`, which performs the single close.

This matters to Windows startup because `WIN.COM` performs repeated executable and support-file opens during initialization. A second close can operate on a reused or already-invalid SFT and obscure the original startup failure.

## Test procedure

1. Build this branch in the 86Box DOS build environment.
2. Boot the resulting kernel on the 486/VGA profile.
3. Run a small COM program, then a normal MZ EXE, and verify that the parent shell remains usable.
4. Install and run Windows 3.0 in real mode.
5. Record the first visible error and the last successful file operation.

Do not add Windows installation media, ROMs, disk images, or generated binaries to this repository.

## Scope

This document records the first narrowly scoped compatibility issue. Copyright, licensing, and provenance notices in the original source remain unchanged.
