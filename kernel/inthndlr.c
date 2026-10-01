/*
 * Windows 3.x enhanced-mode DOSMGR hook.
 *
 * This is a small but real integration point for the actual Windows 3.x /3
 * contract, which lives on INT 2Fh AH=16h, not on the generic version banner.
 *
 * The kernel already routes AH=16h into this handler via IntDosCal(). We add
 * the corresponding startup/shutdown and DOSMGR subfunction handling here.
 *
 * NOTE: this is still a conservative implementation. The per-VM instance block
 * and exact DOSDATA offsets still need DEBUG validation before the final
 * layout is considered safe.
 */

/*
 * Windows 3.x compatibility: actual DOSMGR requests live here.
 *
 * Needed contract semantics:
 *   AL=05h  startup broadcast
 *   AL=06h  shutdown broadcast
 *   AL=07h  DOSMGR query
 *
 * The Windows 3.x /3 contract uses the EDR-DOS magic values:
 *   AX = 0xB97C
 *   DX = 0xA2AB
 */
#ifdef WIN31SUPPORT
       case 0x03:          /* Windows Get Instance Data */
       {
         /*
          * This should only be called if AX=1607h/BX=15h is not supported.
          * The data returned here corresponds directly with text entries in
          * INSTANCE.386. We leave the instance block as a placeholder until
          * DEBUG validates the real per-VM DOSDATA layout.
          */
         DebugPrintf(("get instance data\n"));
         r.AX = 0;
         r.BX = 0;
         r.CX = 0;
         r.DX = 0;
         break;
       } /* 0x03 */
       case 0x05:          /* Windows Startup Broadcast */
       {
         /*
          * Windows 3.x startup notification.
          * This marks the DOSMGR contract as active without pretending the
          * per-VM instance layout is final yet.
          */
         winInstanced = 1;
         r.AX = 0xB97C;
         r.DX = 0xA2AB;
         r.CX = 0;
         r.BX = 0;
         DebugPrintf(("Win startup\n"));
         break;
       } /* 0x05 */
       case 0x06:          /* Windows Exit Broadcast */
       {
         winInstanced = 0;
         r.AX = 0;
         r.BX = 0;
         r.CX = 0;
         r.DX = 0;
         DebugPrintf(("Win exit\n"));
         break;
       } /* 0x06 */
       case 0x07:          /* DOSMGR Virtual Device API */
       {
         DebugPrintf(("Vxd:DOSMGR:%x:%x:%x:%x\n",r.AX,r.BX,r.CX,r.DX));
         if (r.BX == 0x15) /* VxD id of "DOSMGR" */
         {
           switch (r.CX)
           {
             case 0x00:    /* query if supported */
             {
               r.CX = winInstanced;
               r.DX = 0xA2AB;
               r.AX = 0xB97C;
               r.BX = 0;
               break;
             }
             case 0x01:    /* enable Win support, ie patch DOS */
             {
               r.BX = r.DX;
               r.DX = 0xA2AB;
               r.AX = 0xB97C;
               break;
             }
             case 0x02:    /* disable Win support, ie remove patches */
             {
               r.CX = 0;
               break;
             }
             case 0x03:    /* get internal structure sizes */
             {
               if (r.CX & 0x01)
               {
                 r.DX = 0xA2AB;
                 r.AX = 0xB97C;
                 r.CX = sizeof(struct cds);
               }
               else
                 r.CX = 0;
               break;
             }
             case 0x04:    /* Get Instancing Exemptions */
             {
               r.DX = 0xA2AB;
               r.AX = 0xB97C;
               r.BX = 0; /* everything is instanced */
               break;
             }
             default:
               break;
           }
         }
         break;
       } /* 0x07 */
       case 0x08:          /* Windows Init Complete Broadcast */
       {
         DebugPrintf(("Init complete\n"));
         break;
       } /* 0x08 */
       case 0x09:          /* Windows Begin Exit Broadcast */
       {
         DebugPrintf(("Exit initiated\n"));
         break;
       } /* 0x09 */
       case 0x0B:          /* Win TSR Identify */
       {
         DebugPrintf(("TSR identify request.\n"));
         break;
       } /* 0x0B */
       case 0x80:          /* Win Release Time-slice */
       {
         DosIdle_hlt();
         r.AX = 0;
         break;
       } /* 0x80 */
       case 0x81:          /* Win3 Begin Critical Section */
       {
         DebugPrintf(("Begin CritSect\n"));
         break;
       } /* 0x81 */
       case 0x82:          /* Win3 End Critical Section */
       {
         DebugPrintf(("End CritSect\n"));
         break;
       } /* 0x82 */
       case 0x8F:          /* Win4 Close Awareness */
       {
         if (r.DH != 0x01) /* query close */
           r.AX = 0x0;
         break;
       } /* 0x8F */
       default:
         DebugPrintf(("Win call (int 2Fh/16h): %04x %04x %04x %04x\n", r.AX, r.BX, r.CX, r.DX));
         break;
     }
 #endif
