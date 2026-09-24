# RetroDOS build and test checklist

This repository is intended for a Win 3.x-era real-mode DOS kernel build, with a
small set of safe compatibility tweaks while keeping the rest of the original
DOS-C implementation intact.

## 1) Source tree expectations

- Keep the upstream DOS-C kernel layout.
- Do not commit generated binaries or disk images.
- Do not add Windows installation media, ROMs, or VM snapshots to the repo.

## 2) Build configuration

Use the project-specific build environment for DOS-C / FreeDOS-style kernel builds.
The following flags are expected to remain valid:

- `WITHFAT32` selects FAT32-aware build variants.
- The reported DOS compatibility must remain `5.00` in both FAT32 and non-FAT32
  builds for Windows 3.x compatibility.

## 3) Safe compatibility edits

Apply only the following minimal tweaks while the project is under development:

- Keep the FAT32 build switch (`#ifdef WITHFAT32`) intact.
- Report DOS `5.00` for both build variants in `hdr/version.h`.
- Keep `OEM_ID` compatible with DOS-C / DOS API expectations.
- Keep the banner branding as `RetroDOS` for user-visible output.
- Remove duplicate SFT closing in `kernel/task.c` so `DosExec()` owns the close.

## 4) What not to add yet

Do not add or modify the following before the kernel builds and boots reliably:

- boot sector logic
- interrupt handlers in `intr.asm`
- low-level memory strategies
- UMB/HMA management code
- FreeCOM / COMMAND.COM behavior
- Win 3.x-specific runtime features beyond compatibility reporting

## 5) Smoke test sequence

After the fork builds, run this sequence in the VM:

1. `VER`
2. `MEM`
3. `DIR`
4. `ECHO RetroDOS`
5. Run a small `.COM` program
6. Run a small `.EXE` program
7. Try a Windows 3.x launcher or `WIN.COM` if available

Capture:

- DOS version reported by `VER`
- whether the process loads as `.COM` or `.EXE`
- the first actual failure message
- whether the kernel returns to shell or hangs

## 6) Expected behavior

The build should remain conservative and close to DOS-C / FreeDOS behavior:

- DOS compatibility at `5.00`
- stable real-mode execution
- no extra runtime ABI changes unless directly required
- no disk image or binary artifacts committed to Git

## 7) Useful command examples

```bash
grep -RInE 'MAJOR_RELEASE|MINOR_RELEASE|WITHFAT32' .

git diff -- hdr/version.h kernel/task.c

git diff --check
```

## 8) Notes

This checklist is intentionally small so that the next development step is clear:

- build the kernel;
- boot it in a VM;
- validate DOS 5.00 compatibility;
- then move to deeper Win 3.x issues only if the shell and program load path are stable.
