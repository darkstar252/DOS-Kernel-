# Windows 3.x /3 contract for FreeDOS/RetroDOS

This note records the INT 2Fh contract that Windows 3.0/3.1 uses in enhanced mode, and the concrete place where the FreeDOS/RetroDOS kernel currently falls short.

Sources used:
- EDR-DOS: `drdos/int2f.asm` (`WindowsHooks`, `WindowsStartup`, `WindowsShutdown`, `WindowsDOSMGR`)
- FreeDOS kernel: `kernel/int2f.asm`, `kernel/inthndlr.c`
- RBIL / known DOS-Windows compatibility notes

## Executive summary

Windows 3.x has three startup modes:
- `/R` real mode: standard INT 21h-only DOS interface
- `/S` standard mode: 286 protected mode, DOS is still a normal DOS
- `/3` enhanced mode (386): VMM/WIN386 + VM instancing, with the DOS exposing a per-VM data contract

The crucial missing implementation for FreeDOS/RetroDOS is the Windows 386 mode contract exposed through `INT 2Fh AH=16h`:
- `AX=1605h` : Windows startup broadcast
- `AX=1606h` : Windows shutdown broadcast
- `AX=1607h BX=0015h` : DOS manager / DOS API query

The EDR-DOS implementation shows that this is not a cosmetic compatibility feature: it is the mechanism by which Win386 decides that the DOS is safe to use in the VM model.

## The evidence in EDR-DOS

In `drdos/int2f.asm`, the Windows hook is entered through `WindowsHooks`:

```assembly
WindowsHooks:
; AH = 16h, it's a windows broadcast
; AL = subfunction, other regs as appropriate

    cmp     al,07h          ; 1607: Virtual device init
     je     WindowsDOSMGR
    test    dx,1            ; is it a DOSX broadcast ?
     jnz    WindowsExit
    cmp     al,05h          ; 1605: Windows enhanced mode init
     je     WindowsStartup
    cmp     al,06h          ; 1606: Windows enhanced mode exit
     je     WindowsShutdown
WindowsExit:
    iret
```

This shows that `INT 2Fh AH=16h` is the Windows broadcast vector, and that the important subfunctions are precisely:
- `AL=05h` => startup
- `AL=06h` => shutdown
- `AL=07h` => DOS manager query

### 1605h startup path

```assembly
WindowsStartup:
    push    ds
    call    get_dseg         ; DS -> our data
    inc     criticalSectionEnable
    inc     WindowsHandleCheck

    mov     SwStartupInfo+2,bx
    mov     SwStartupInfo+4,es
    push    ds
    pop     es
    mov     bx,offset SwStartupInfo
    jmp     int2F_BIOS       ; pass on to the BIOS
```

The startup hook is intentionally minimal and is designed to notify the DOS that Windows is starting, while still passing the call through to the BIOS chain.

### 1606h shutdown path

```assembly
WindowsShutdown:
    push    ds
    call    get_dseg
    dec     criticalSectionEnable
    dec     WindowsHandleCheck
    pop     ds
    sub     dx,dx            ; return success
    iret
```

### 1607h DOS manager path

This is the important callback for Win386 / DOSMGR:

```assembly
WindowsDOSMGR:
    cmp     bx,15h           ; is it DOS manager?
     jne    WindowsExit
    jcxz   WindowsCX0
    dec     cx
     jcxz   WindowsCX1
    dec     cx
    dec     cx
    dec     cx
     jcxz   WindowsCX4
    dec     cx
     jcxz   WindowsCX5
    iret
```

The interpretation is:
- `BX == 0015h` identifies the DOSMGR requester
- then `CX` selects sub-operation(s) that the DOSMGR is probing

The known concrete cases in the EDR-DOS source are:

```assembly
WindowsCX0:
    push    ds
    call    get_dseg
    push    ds
    pop     es
    pop     ds
    lea     bx,windowsData   ; ES:BX -> secret variables
    inc     cx               ; tell them we've responded
    iret
```

This is a critical clue: Win386 is asking the DOS for a table of instanceable data, and EDR-DOS returns a private data block (`windowsData`) via `ES:BX`.

```assembly
WindowsCX1:
    mov     bx,dx            ; entry DX=1Fh, exit BX=1Fh
    mov     ax,0B97Ch        ; AX, DX are magic values
    mov     dx,0A2ABh
    iret
```

This is a protocol marker / magic handshake, and it is visible in the implementation: the DOS returns a fixed magic pair, not just a value-less success flag.

```assembly
WindowsCX4:
    xor     dx,dx
    iret
```

```assembly
WindowsCX5:
; entry: ES:DI -> device driver
; determine device driver size in bytes
```

This suggests that the DOSMGR negotiation includes more than a bare version string: there is also device / instance metadata and per-VM data sizing.

## Contract in pseudocode form

This is the minimal contract that the DOS must support for Windows 3.x enhanced mode.

### 1. Startup broadcast

```text
INT 2Fh AH=16h AL=05h

Input:
    AL = 05h
    DX = broadcast flags / phase info, depending on call stage

Output:
    CF = 0 if supported
    DOS sets its internal critical-section state and Windows-handle state
    call falls through to BIOS if needed
```

EDR-DOS behavior:
- increments `criticalSectionEnable`
- increments `WindowsHandleCheck`
- then passes to the BIOS chain

### 2. Shutdown broadcast

```text
INT 2Fh AH=16h AL=06h

Input:
    AL = 06h

Output:
    CF = 0
    DOS decrements the same internal state used for Windows
```

EDR-DOS behavior:
- decrements `criticalSectionEnable`
- decrements `WindowsHandleCheck`
- returns success

### 3. DOSMGR query

```text
INT 2Fh AH=16h AL=07h

Input:
    BX = 0015h  ; request DOS manager API
    CX = subfunction selector
    DX = value depending on probe stage
    ES:DI = optional driver pointer for some subfuncs

Output:
    If DOS recognises DOSMGR, it must answer some or all of these probes:
    CX=0 -> return ES:BX -> instance data block
    CX=1 -> return magic values / capability marker
    CX=4 -> return success/zero state
    CX=5 -> return metadata for the driver or instance table
```

The concrete EDR-DOS source shows the first four probes in `WindowsDOSMGR` and the instance block at `WindowsCX0`.

## What FreeDOS currently does

In the FreeDOS/RetroDOS path, `kernel/int2f.asm` routes `AH=16h` to the internal DOS call handler:

```assembly
Int2f3:
    cmp     ax,1680h
    je      WinIdle
    cmp     ah,12h
    je      IntDosCal
    cmp     ah,13h
    je      IntDosCal
    cmp     ah,16h
    je      IntDosCal
    cmp     ah,46h
    je      IntDosCal
```

Then `IntDosCal` simply routes to:

```assembly
SwitchToInt2fStack
extern   _int2F_12_handler
call _int2F_12_handler
DoneInt2fStack
```

This is the crucial gap. FreeDOS is not implementing the Windows-specific `1605h/1606h/1607h` contract at the `INT 2Fh AH=16h` layer; it is just dispatching the generic internal INT2F calls to `_int2F_12_handler`.

In other words, the current FreeDOS path has no Windows 386 startup / DOSMGR handshake.

That is why Windows 3.0 enhanced mode (`/3`) is the hard target, and why the real-mode `/R` and standard-mode `/S` paths can work while `/3` stalls or aborts.

## Important distinction: /R, /S, /3

- `/R` (real mode): no explicit Win386 / DOSMGR handshake required; standard DOS APIs are sufficient
- `/S` standard mode: enough for Win 3.x as a normal Windows client; no per-VM DOS instance contract required
- `/3` enhanced mode: requires the VMM/VM contract and the `INT 2Fh AH=16h` DOSMGR handshake; this is the missing piece in FreeDOS/RetroDOS

Therefore, the first realistic target is Windows 3.0 enhanced mode, not 3.1, because Win 3.1 adds AARD and more DOSMGR validation.

## Minimal implementation plan for FreeDOS/RetroDOS

The correct path is not to fake random versions. The correct path is:

1. Handle `INT 2Fh AH=16h AL=05h/06h/07h` in `kernel/int2f.asm`
2. Implement the DOSMGR probe logic in a minimal, explicit handler
3. Return a valid instance data block for the DOS data segment
4. Return the correct magic values for the `CX==1` probe
5. Keep the `instance` block aligned with the DOS internal data that the kernel actually owns

The previous “just say DOS 5.00” trick is not sufficient. The critical missing feature is the instance contract, not the version string.

## Requirements before changing code

Before writing a kernel patch, the next mandatory steps are:

1. Map the internal DOS data addresses used by FreeDOS/RetroDOS for:
   - current PSP
   - current DTA
   - InDOS flag
   - critical error state
   - CDS / current directory structure
2. Confirm the exact offsets in the DOS data segment that Windows expects to be instanceable
3. Measure with DEBUG and compare against the exact `WindowsCX0`/`WindowsCX1` contract seen in EDR-DOS

This is the minimum required to keep the patch grounded in the actual kernel layout instead of guessing.

## Final conclusion

The contract is real, it is implemented in EDR-DOS, and the current FreeDOS routing in `kernel/int2f.asm` does not implement it. The missing behavior is not “just a version number”: it is the Windows-specific handshake and instance data contract required by VMM mode.

The correct target is a kernel-level implementation of the `INT 2Fh AH=16h` Windows hooks, starting with `1605h`, `1606h`, and `1607h BX=0015h`, with the DOS returning a valid per-VM instance block.

---

This document is intentionally conservative. It does not claim unsupported offsets or unsupported subfunctions beyond what the EDR-DOS source shows directly.
