/*
 * File: hdr/win16.h
 * Description: Windows 3.x enhanced mode (386) DOSMGR contract
 *
 * This header defines the minimal structures and constants needed for
 * Windows 3.0/3.1 enhanced mode (/3) support via the INT 2Fh AH=16h
 * DOSMGR contract.
 *
 * Source reference:
 *   EDR-DOS: drdos/int2f.asm (WindowsHooks, WindowsDOSMGR)
 *   FreeDOS: kernel/int2f.asm, kernel/inthndlr.c
 *
 * Key insight: Windows /3 does NOT rely on AH=30h version reporting alone.
 * It requires the DOSMGR handshake at INT 2Fh AH=16h, which provides:
 *   - AL=05h : startup broadcast
 *   - AL=06h : shutdown broadcast
 *   - AL=07h : DOSMGR queries (CX selector)
 *
 * This is the contract layer that actual Windows 386 mode implementation
 * needs; the version reported by AH=30h is secondary.
 */

#ifndef __WIN16_H
#define __WIN16_H

/*
 * Magic values returned by DOSMGR CX=1 probe.
 * These are exact values from EDR-DOS and must not be changed.
 * Windows 386 uses these to validate the DOS understands the contract.
 */
#define WIN16_DOSMGR_MAGIC_AX   0xB97C
#define WIN16_DOSMGR_MAGIC_DX   0xA2AB

/*
 * Win16 instance block structure.
 *
 * This is the per-VM DOS instance data that Windows expects to retrieve
 * when CX=0 (instance pointer query). The offsets below are placeholders
 * and MUST be measured/validated against the actual FreeDOS kernel
 * data layout using DEBUG.
 *
 * Real implementation requires:
 *   1. Measure actual offsets of _cu_psp, _dta, etc. in DOSDATA segment
 *   2. Validate with DEBUG in a real VM
 *   3. Fill in exact kernel-relative offsets
 *   4. Ensure fields are per-VM swappable (marked in flags)
 *
 * Do NOT use these offsets as final without DEBUG validation.
 */
typedef struct win16_instance_block
{
    UWORD  version;              /* [0] instance data format version */
    UWORD  current_psp;          /* [2] current process segment (offset in DOSDATA) */
    UWORD  current_dta;          /* [4] current DTA pointer (offset in DOSDATA) */
    UWORD  in_dos;               /* [6] InDOS critical section flag (offset) */
    UWORD  error_state;          /* [8] critical error state (offset) */
    UWORD  cds_ptr;              /* [10] current directory structure (offset) */
    UWORD  sda_ptr;              /* [12] swappable data area (offset) */
    UWORD  reserved1;            /* [14] reserved for future use */
} WIN16_INSTANCE_BLOCK;

/*
 * Per-subfunction return protocol for INT 2Fh AH=16h AL=07h BX=0015h
 *
 * The DOSMGR selector is in CX:
 *   CX=0 : return instance block pointer in ES:BX
 *   CX=1 : return magic capability markers (AX/DX)
 *   CX=4 : return success/failure state (XOR DX, DX for success)
 *   CX=5 : return device metadata or instance table info
 *
 * This is the structured approach used by EDR-DOS and required by
 * Windows 386 mode.
 */

/*
 * Exported from kernel/inthndlr.c:
 *   VOID ASMCFUNC _win16_dosmgr_handler(UWORD cx, UWORD dx, UWORD bx,
 *                                       UWORD es, UWORD di);
 *
 * This function handles the actual DOSMGR protocol and must be called
 * from the INT 2Fh AH=16h AL=07h handler in kernel/int2f.asm.
 */
extern VOID ASMCFUNC _win16_dosmgr_handler(void);

#endif /* __WIN16_H */
