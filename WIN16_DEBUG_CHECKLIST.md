# Windows 3.x /3 debug checklist

This file captures the next engineering step after the contract analysis and the minimal DOSMGR stub.

## 1) Correct contract layer

The real Windows 3.x /3 contract is at:

- `INT 2Fh AH=16h`
- `AL=05h` startup broadcast
- `AL=06h` shutdown broadcast
- `AL=07h` DOSMGR query

This is the layer that must be handled before any version banner or `AH=30h` compatibility hack is considered.

## 2) Minimal DOSMGR instance block

The minimal kernel-owned structure is conceptually:

- version
- current PSP pointer
- current DTA pointer
- InDOS flag location
- critical error state
- current directory structure pointer
- SDA / swap state pointer

The corresponding contract is defined in `hdr/win16.h`.

This block is intentionally conservative and must be validated with DEBUG before it is considered final.

## 3) Required DEBUG validation plan

Before locking any final offsets, validate the following in the FreeDOS data segment:

1. Current PSP
   - determine where the active process segment is stored
   - ensure each VM sees the correct PSP

2. Current DTA
   - locate the active DTA pointer used by DOS functions
   - ensure it is not global across VMs

3. InDOS flag
   - confirm the DOS critical section flag is exposed correctly
   - ensure Win386 sees the kernel state it expects

4. Critical error state
   - locate the state used by INT 24 and equivalent error-handling flows

5. Current directory structure / CDS
   - verify the current directory chain is represented correctly

6. SDA / swap data area
   - confirm the DOS instance structure can be swapped per VM without corrupting global state

7. Kernel data segment behaviour under Win386
   - test startup broadcasting and DOSMGR probes in a real VM
   - confirm that the DOSMGR return address and instance pointer survive the full Win386 handshake

## Implementation rule

Do not close this as final until all of the above are verified in a real VM. The current layer is intentionally a stub that identifies the correct engineering path, not a claim of complete Win386 compatibility.
