# Windows 3.0 loader regression notes

The loader regression for the `win30-486-vga` branch is intentionally independent of Windows installation media.

Expected behavior after the loader ownership fix:

- a COM program can be executed and return to COMMAND.COM;
- an MZ EXE can be executed and return to COMMAND.COM;
- repeated execution does not consume handles;
- an unsuccessful load closes its input handle exactly once;
- Windows 3.0 `WIN.COM` can proceed to the next startup stage without an SFT being closed twice.

Suggested DOS-side checks:

```text
MEM
WIN.COM
MEM
```

Repeat the `WIN.COM` attempt after returning to DOS. If the system does not return to DOS, record the 86Box machine profile, available conventional memory, CONFIG.SYS, AUTOEXEC.BAT, and the exact screen message.
