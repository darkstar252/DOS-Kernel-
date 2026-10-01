/*
 * File: hdr/win16.h
 * Description: Windows 3.x enhanced mode (386) DOSMGR contract
 *
 * This header defines the structures and constants needed for Windows 3.0/3.1
 * enhanced mode (/3) support via the INT 2Fh AH=16h DOSMGR contract.
 *
 * The DOSMGR instance block is the per-VM data structure that Windows 386 mode
 * needs to track DOS state for each virtual machine. The offsets defined here
 * correspond to the actual FreeDOS kernel data layout in _internal_data.
 *
 * Reference FreeDOS kernel/kernel.asm:
 *   _internal_data base offset corresponds to INT21/5D06 return
 *   _ErrorMode      @ +00h
 *   _InDOS          @ +01h
 *   _dta            @ +0Ch (4 bytes, DWORD)
 *   _cu_psp         @ +10h (2 bytes, WORD)
 *
 * CDS (Current Directory Structure) is maintained separately in the DOS
 * internal state, indexed by drive letter.
 *
 * SDA (Swappable Data Area) overlaps with the PSP and critical error handling.
 */

#ifndef __WIN16_H
#define __WIN16_H

/*
 * Magic values for DOSMGR capability probe (CX=1).
 * These are exact values from EDR-DOS and must not be changed.
 * Windows 386 mode uses these to validate the DOS supports the contract.
 */
#define WIN16_DOSMGR_MAGIC_AX   0xB97C
#define WIN16_DOSMGR_MAGIC_DX   0xA2AB

/*
 * Win16 instance block structure.
 *
 * This is the per-VM DOS instance data that Windows 386 enhanced mode
 * expects to retrieve when CX=0 (instance pointer query).
 *
 * The offsets here correspond to the FreeDOS _internal_data layout.
 * Windows uses these offsets to read/write per-VM state without direct
 * kernel calls.
 *
 * Layout within the Windows DOSMGR instance block:
 *   Offset 00h: version
 *   Offset 02h: InDOS flag (from _internal_data + 01h)
 *   Offset 03h: Critical error mode (from _internal_data + 00h)
 *   Offset 04h: DTA (from _internal_data + 0Ch, DWORD)
 *   Offset 08h: Current PSP (from _internal_data + 10h, WORD)
 *   Offset 0Ah: CDS array base pointer
 *   Offset 0Eh: SDA base pointer
 *   Offset 12h: reserved
 */
typedef struct win16_instance_block
{
    UWORD  version;              /* [0] instance data format version (1) */
    UBYTE  in_dos;               /* [2] InDOS critical section flag */
    UBYTE  error_mode;           /* [3] Critical error mode (ErrorMode) */
    ULONG  dta;                  /* [4] Current DTA far pointer (DWORD) */
    UWORD  current_psp;          /* [8] Current PSP segment */
    UWORD  cds_ptr;              /* [A] CDS array base offset in DOSDATA */
    UWORD  sda_ptr;              /* [C] SDA / reserved data area offset */
    UWORD  reserved;             /* [E] reserved for future use */
} WIN16_INSTANCE_BLOCK;

/*
 * Offsets within FreeDOS _internal_data segment (used internally).
 * These correspond to the actual FreeDOS kernel layout.
 * Reference: kernel/kernel.asm, _internal_data label
 */
#define INTERNAL_DATA_ERROR_MODE   0x00    /* ErrorMode flag */
#define INTERNAL_DATA_INDOS        0x01    /* InDOS flag */
#define INTERNAL_DATA_DTA          0x0C    /* DTA (DWORD) */
#define INTERNAL_DATA_CU_PSP       0x10    /* Current PSP (WORD) */

/*
 * Per-subfunction return protocol for INT 2Fh AH=16h AL=07h BX=0015h
 *
 * The DOSMGR selector is in CX:
 *   CX=0 : return instance block pointer in ES:BX
 *           (win16_instance_block far pointer)
 *   CX=1 : return magic capability markers in AX/DX
 *           (AX = 0xB97C, DX = 0xA2AB)
 *   CX=4 : return success probe in DX
 *           (DX = 0 on success)
 *   CX=5 : return device metadata or instance table info
 *
 * This is the structured approach used by EDR-DOS and required by
 * Windows 386 enhanced mode.
 */

/*
 * Exported from kernel/inthndlr.c:
 *   VOID ASMCFUNC _win16_startup_handler(void)
 *   VOID ASMCFUNC _win16_shutdown_handler(void)
 *   VOID ASMCFUNC _win16_dosmgr_handler(void)
 *
 * These functions handle the Windows 3.x /3 contract and must be called
 * from the INT 2Fh AH=16h handler in kernel/int2f.asm.
 */
extern VOID ASMCFUNC _win16_startup_handler(void);
extern VOID ASMCFUNC _win16_shutdown_handler(void);
extern VOID ASMCFUNC _win16_dosmgr_handler(void);

/*
 * Global instance state for Windows 3.x /3 tracking.
 * Set to 1 when Windows startup broadcast is received (AL=05h).
 * Set to 0 when Windows shutdown broadcast is received (AL=06h).
 */
extern UBYTE win16_instanced;

#endif /* __WIN16_H */
