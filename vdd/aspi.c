/* aspi.c
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1994 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Wed Dec 22 1993
 */
 
static char *rcsid =
"$Id: aspi.c,v 1.9 1994/04/20 11:54:13 rommel Exp $";
static char *rcsrev = "$Revision: 1.9 $";

/* $Log: aspi.c,v $
/* Revision 1.9  1994/04/20 11:54:13  rommel
/* fourth release
/*
/* Revision 1.8  1994/03/29 16:10:05  rommel
/* third release
/*
 * Revision 1.7  1994/03/09 17:29:12  rommel
 * new version using VDHPushFarCall
 *
 * Revision 1.6  1994/02/04 17:43:31  rommel
 * first release
 *
 * Revision 1.5  1994/02/02 13:22:39  rommel
 * beta release
 *
 * Revision 1.4  1994/01/27 12:59:49  rommel
 * first test release
 *
 * Revision 1.3  1994/01/09 20:19:53  rommel
 * first somewhat working version
 *
 * Revision 1.2  1993/12/22  18:20:21  rommel
 * First stage, simple commands somewhat implemented.
 *
 * Revision 1.1  1993/12/22  13:04:49  rommel
 * Initial revision
 * */

#include <mvdm.h>

#include "../sdk/os2/aspi.h"
#include "vaspi.h"

#pragma data_seg(SWAPINSTDATA)

#define MAXSRB   32
#define NEXT(x)  ((x) == MAXSRB - 1 ? 0 : (x) + 1)

typedef struct { USHORT nOffs; USHORT nSeg; } PSRB;
static PSRB rpSRB[MAXSRB];
static int nHead = 0, nTail = 0;

/* Make sure all of the reserved fields are set to zero. */
/* The HP ScanJet ASPI-based device driver doesn't do that in all cases. */

#define zero(type, ptr, field) \
    memset(((type) (ptr)) -> field, 0, sizeof(((type) (ptr)) -> field))

#pragma alloc_text(CSWAP_TEXT,EnqueueSRB,DequeueSRB,GetSRB)

BOOL EnqueueSRB(USHORT nSeg, USHORT nOffs)
{
  int nNew;

  if ((nNew = NEXT(nHead)) == nTail)
    return FALSE;

  rpSRB[nHead].nSeg  = nSeg;
  rpSRB[nHead].nOffs = nOffs;

  if (nHead == nTail) /* queue was empty until now */
    VDHArmTimerHook(hTimerHook, 10, owner_VDM);

  nHead = nNew;

  return TRUE;
}

BOOL DequeueSRB(void)
{
  if (nHead == nTail)
    return FALSE;

  nTail = NEXT(nTail);

  if (nHead != nTail) /* queue is still not empty */
    VDHArmTimerHook(hTimerHook, 10, owner_VDM);

  return TRUE;
}

BOOL GetSRB(PUSHORT pSeg, PUSHORT pOffs)
{
  if (nHead == nTail)
    return FALSE;

  *pSeg  = rpSRB[nTail].nSeg;
  *pOffs = rpSRB[nTail].nOffs;

  return TRUE;
}

#pragma alloc_text(CSWAP_TEXT,NativeASPI,VirtualASPI)

BOOL NativeASPI(NPASPI_SRB_HEADER pSRB, ULONG nSRBSize,
		PVOID pData, ULONG nDataSize)
{
  static ULONG ulParmLengthInOut, ulDataLengthInOut;

  ulParmLengthInOut = nSRBSize;
  ulDataLengthInOut = nDataSize;

  return VDHDevIOCtl(hASPI, ASPI_VDDREQ, pSRB -> CommandCode,
		     pSRB, nSRBSize, &ulParmLengthInOut,
		     pData, nDataSize, &ulDataLengthInOut) != 0;
}

BOOL HOOKENTRY VirtualASPI(PVOID pHookData, PCRF pcrf)
{
  ASPI_SRB_HEADER *pSRB;
  ASPI_SRB_EXECUTE_IO SaveSRB;
  PCHAR pData;
  ULONG nData, nSRB;
  BOOL bOk = TRUE;

  VDHPopInt();                          /* to prevent ROM code execution   */

  pSRB = PFROMVADDR(ES(pcrf), BX(pcrf));
  pSRB -> ASPIStatus = ASPI_STATUS_INVALID_COMMAND; /* default response */

  if (!bASPIAvailable)
    return TRUE;

  /* vdhInt3(); */

  switch (pSRB -> CommandCode)
  {

  case ASPI_CMD_ADAPTER_INQUIRY:
    zero(PASPI_SRB_INQUIRY, pSRB, Reserved_1);
    bOk = NativeASPI(pSRB, sizeof(ASPI_SRB_INQUIRY), NULL, 0);
    break;

  case ASPI_CMD_GET_DEVICE_TYPE:
    zero(PASPI_SRB_DEVICE_TYPE, pSRB, Reserved_1);
    bOk = NativeASPI(pSRB, sizeof(ASPI_SRB_DEVICE_TYPE), NULL, 0);
    break;

  case ASPI_CMD_EXECUTE_IO:

    zero(PASPI_SRB_EXECUTE_IO, pSRB, Reserved_1);
    zero(PASPI_SRB_EXECUTE_IO, pSRB, ASPIWorkSpace);

    pData = PFROMVP(((ASPI_SRB_EXECUTE_IO *) pSRB) -> ppDataBuffer);
    nData = ((ASPI_SRB_EXECUTE_IO *) pSRB) -> DataXferLen;

    nSRB = sizeof(ASPI_SRB_EXECUTE_IO);
    nSRB += ((ASPI_SRB_EXECUTE_IO *) pSRB) -> SCSICDBLen;
    nSRB += ((ASPI_SRB_EXECUTE_IO *) pSRB) -> SenseDataLen;

    /* save a copy of the SRB */
    SaveSRB = * (ASPI_SRB_EXECUTE_IO *) pSRB;

    if (nData)
    {
      /* tests indicate that we apparently have to double buffer
       * the transfer memory region 
       */
      memcpy(pBuffer, pData, nData);
      bOk = NativeASPI(pSRB, nSRB, pBuffer, nData);
      memcpy(pData, pBuffer, nData);
    }
    else
      bOk = NativeASPI(pSRB, nSRB, 0, 0);

    /* these fields were altered by VASPIDD and should be restored */
    ((ASPI_SRB_EXECUTE_IO *) pSRB) -> ppDataBuffer   = SaveSRB.ppDataBuffer;
    ((ASPI_SRB_EXECUTE_IO *) pSRB) -> ppSRB          = SaveSRB.ppSRB;
    ((ASPI_SRB_EXECUTE_IO *) pSRB) -> PM_PostAddress = SaveSRB.PM_PostAddress;
    ((ASPI_SRB_EXECUTE_IO *) pSRB) -> PM_DataSeg     = SaveSRB.PM_DataSeg;

    break;

  case ASPI_CMD_ABORT_IO:
    zero(PASPI_SRB_ABORT_IO, pSRB, Reserved_1);
    bOk = NativeASPI(pSRB, sizeof(ASPI_SRB_ABORT_IO), NULL, 0);
    break;

  case ASPI_CMD_RESET_DEVICE:
    zero(PASPI_SRB_RESET_DEVICE, pSRB, Reserved_1);
    zero(PASPI_SRB_RESET_DEVICE, pSRB, Reserved_2);
    zero(PASPI_SRB_RESET_DEVICE, pSRB, ASPIWorkSpace);
    bOk = NativeASPI(pSRB, sizeof(ASPI_SRB_RESET_DEVICE), NULL, 0);
    break;

  case ASPI_CMD_SET_ADAPTER_PARMS:
    zero(PASPI_SRB_ADAPTER_PARMS, pSRB, Reserved_1);
    bOk = NativeASPI(pSRB, sizeof(ASPI_SRB_ADAPTER_PARMS), NULL, 0);
    break;

  default:
    pSRB -> ASPIStatus = ASPI_STATUS_INVALID_COMMAND;
    break;
  }

  if (!bOk && pSRB -> ASPIStatus == ASPI_STATUS_IN_PROGRESS)
    pSRB -> ASPIStatus = ASPI_STATUS_ABORTED;

  if (pSRB -> ASPIReqFlags & ASPI_REQFLAG_POST_ENABLE)
    EnqueueSRB(ES(pcrf), BX(pcrf));

  return TRUE;                                  /* claimed the interrupt   */
}

#pragma alloc_text(CSWAP_TEXT,PostTimer,PostReturn)

BOOL HOOKENTRY PostTimer(PVOID pHookData, PCRF pcrf)
{
  static USHORT nSeg, nOffs;

  /* vdhInt3(); */

  GetSRB(&nSeg, &nOffs);

  VDHArmReturnHook(hRetHook, VDHARH_CSEIP_HOOK);
  VDHPushRegs(VDHREG_AX | VDHREG_BX | VDHREG_CX);

  AX(pcrf) = MPLEX_POST;
  BX(pcrf) = nOffs;
  CX(pcrf) = nSeg;

  VDHPushInt(MPLEX);

  return TRUE;
}

BOOL HOOKENTRY PostReturn(PVOID pHookData, PCRF pcrf)
{
  /* vdhInt3(); */

  VDHPopRegs(VDHREG_AX | VDHREG_BX | VDHREG_CX);

  DequeueSRB();

  return TRUE;
}

/* end of aspi.c */
