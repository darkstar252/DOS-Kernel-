/*
 * Windows 3.x enhanced-mode DOSMGR hook.
 *
 * This is intentionally a conservative implementation stub. It is placed in
 * the correct layer for the actual Windows 3.0/3.1 /3 contract:
 *
 *   INT 2Fh AH=16h
 *   AL=05h  startup broadcast
 *   AL=06h  shutdown broadcast
 *   AL=07h  DOSMGR query
 *
 * It does not pretend to be a final per-VM layout: the real DOSDATA
 * offsets must be measured in DEBUG before they are assigned to the per-VM
 * instance block. This file deliberately keeps the contract isolated so it
 * can be connected into the kernel later without changing the generic
 * INT2F/12xx flow.
 */

#include "../hdr/portab.h"
#include "../hdr/win16.h"

/*
 * Minimal Windows 3.x startup/shutdown state placeholders.
 * The real implementation must track per-VM state and eventually be wired to
 * the DOSDATA segment for current PSP, DTA, InDOS, critical error state,
 * current directory/CDS, and SDA.
 */
VOID ASMCFUNC _win16_startup(void)
{
    /*
     * AL=05h startup broadcast: accept the contract at the correct layer.
     * Real implementation: mark Windows startup present and enable any
     * internal state needed by DOSMGR.
     */
}

VOID ASMCFUNC _win16_shutdown(void)
{
    /*
     * AL=06h shutdown broadcast: clear the same state.
     */
}

/*
 * Minimal DOSMGR contract handler.
 *
 * This is the first-pass logic for Windows 386 enhanced mode.
 * It is not a final instance layout. It exists so the kernel has a single
 * canonical place to handle DOSMGR requests without mixing them with the
 * generic INT 2Fh/12xx handlers.
 */
VOID ASMCFUNC _win16_dosmgr_handler(UWORD cx, UWORD dx, UWORD bx,
                                   UWORD es, UWORD di)
{
    switch (cx)
    {
        case 0:
            /*
             * CX=0: return pointer to DOS instance block.
             * The assembly wrapper must place ES:BX -> win16 instance.
             * Final instance layout must be measured against actual DOSDATA.
             */
            break;

        case 1:
            /*
             * CX=1: return magic capability markers.
             * EDR-DOS uses AX=0xB97C and DX=0xA2AB.
             */
            break;

        case 4:
            /*
             * CX=4: success probe.
             * Return success in DX as zero, as used by EDR-DOS.
             */
            break;

        case 5:
            /*
             * CX=5: driver metadata / instance metadata query.
             * This path is intentionally left conservative until the real
             * DOSDATA layout is known and validated with DEBUG.
             */
            break;

        default:
            /* Unknown probe; ignore for now. */
            break;
    }
}
