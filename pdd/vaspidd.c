/* vaspidd.c
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1993 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Wed Dec 22 1993
 */

#define VERSION "1.6"

/* developed from sample code from Adaptec */
 
static char *rcsid =
"$Id: vaspidd.c,v 1.17 1994/09/23 19:51:24 rommel Exp $";
static char *rcsrev = "$Revision: 1.17 $";

/* 
 * $Log: vaspidd.c,v $
 * Revision 1.17  1994/09/23 19:51:24  rommel
 * seventh release
 *
 * Revision 1.16  1994/09/14 12:13:36  rommel
 * sixt release
 *
 * Revision 1.15  1994/08/09 15:32:03  rommel
 * corrected EES license
 *
 * Revision 1.14  1994/08/09 13:58:14  rommel
 * added "EDV + Elektronic Systeme" licensing
 *
 * Revision 1.13  1994/05/26 12:47:56  rommel
 * fifth release
 *
 * Revision 1.12  1994/04/20 11:52:19  rommel
 * fourth release
 *
 * Revision 1.11  1994/03/29 15:59:25  rommel
 * third release
 *
 * Revision 1.10  1994/03/09 16:56:26  rommel
 * removed breakpoint
 *
 * Revision 1.9  1994/03/09 16:53:39  rommel
 * added debug on/off IOCTL
 *
 * Revision 1.8  1994/03/04 14:41:22  rommel
 * driver is now reentrant
 *
 * Revision 1.7  1994/03/01 15:27:39  rommel
 * second release
 *
 * Revision 1.6  1994/03/01 09:09:20  rommel
 * bug fixes for second release
 *
 * Revision 1.5  1994/02/07 19:28:47  rommel
 * bug fix in debugging code
 *
 * Revision 1.4  1994/02/06 21:55:15  rommel
 * first release
 *
 * Revision 1.3  1994/02/02 13:20:27  rommel
 * beta release
 *
 * Revision 1.2  1994/01/27 13:00:49  rommel
 * first test release
 *
 * Revision 1.1  1994/01/09 20:24:50  rommel
 * Initial revision
 */

#define INCL_NOPMAPI
#include <os2.h>

#include <devcmd.h>
#include <devclass.h>

#include <strat2.h>
#include <reqpkt.h>

#include <iorb.h>
#include <addcalls.h>
#include <scsi.h>
#include <dhcalls.h>

#include "../sdk/os2/aspi.h"
#include "vaspidd.h"

#define BREAKPOINT       _asm int 3

PFN Device_Help = 0;		/* Pointer to Dev_Help */
ATTACHAREA OS2ASPI;		/* OS2ASPI entry information */
BOOL bDemoVersion;

char *CommandName[] =
{
  "ADAPTER_INQUIRY",
  "GET_DEVICE_TYPE",
  "EXECUTE_IO",
  "ABORT_IO",
  "RESET_DEVICE",
  "SET_ADAPTER_PARMS"
};

#define MAX_CMD (sizeof(CommandName) / sizeof(char *))

int DebugPort = 0;

void SendChar(char chr)
{
  _asm
  {
    mov dx,DebugPort
    add dx,5
  again:
    in al,dx
    and al,0x60
    cmp al,0x60
    jne again

    mov al,chr
    mov dx,DebugPort
    out dx,al
  }
}

void PutMessage(char _far *msg)
{
  while (*msg)
    SendChar(*msg++);
}

void HexString(ULONG val, char *buf, int digits)
{
  static char hex[] = "0123456789ABCDEF";

  for (buf += digits; digits; digits--)
  {
    *--buf = hex[val & 0x0F];
    val >>= 4;
  }
}

void DebugASPI(PRP_GENIOCTL ptr)
{
  int cmd = ptr -> Function;
  PASPI_SRB_EXECUTE_IO pSRB;
  ULONG addr;

  PutMessage("VASPIDD: ");

  if (cmd < MAX_CMD)
  {
    PutMessage(CommandName[cmd]);

    if (cmd == ASPI_CMD_GET_DEVICE_TYPE ||
	cmd == ASPI_CMD_EXECUTE_IO ||
	cmd == ASPI_CMD_RESET_DEVICE )
    {
      char *msg = ": 0 0 0";

      pSRB = (PASPI_SRB_EXECUTE_IO) (ptr -> ParmPacket);

      msg[2] = (char) ('0' + pSRB -> SRBHdr.AdapterIndex);
      msg[4] = (char) ('0' + pSRB -> DeviceTargetID);
      msg[6] = (char) ('0' + pSRB -> DeviceTargetLUN);

      PutMessage(msg);

      if (cmd == ASPI_CMD_EXECUTE_IO)
      {
	msg = " : 00 00000000 0000";
	HexString(pSRB -> SCSICDBStart[0], msg + 3, 2);
	addr = 0;
	if (ptr -> DataPacket)
	  DevHelp_VirtToPhys(ptr -> DataPacket, &addr);
	HexString(addr, msg + 6, 8);
	HexString(pSRB -> DataXferLen, msg + 15, 4);
	PutMessage(msg);

	if (pSRB -> SRBHdr.ASPIReqFlags & ASPI_REQFLAG_POST_ENABLE)
	  PutMessage(" (post)");
      }

      if (cmd == ASPI_CMD_GET_DEVICE_TYPE)
	PutMessage("\r\n");
    }
    else
      PutMessage("\r\n");
  }
  else
    PutMessage("ERROR: unknown function\r\n");
}

void DebugResult(PRP_GENIOCTL ptr)
{
  PASPI_SRB_EXECUTE_IO pSRB = (PASPI_SRB_EXECUTE_IO) (ptr -> ParmPacket);
  char *msg = " -> 00 00 00\r\n";

  HexString(pSRB -> SRBHdr.ASPIStatus, msg + 4, 2);
  HexString(pSRB -> HostStatus, msg + 7, 2);
  HexString(pSRB -> TargetStatus, msg + 10, 2);
  PutMessage(msg);
}

#define DEBUG_MSG(msg)   if (DebugPort) PutMessage(msg)
#define DEBUG_ASPI(ptr)  if (DebugPort) DebugASPI(ptr)
#define DEBUG_RES(ptr)   if (DebugPort) DebugResult(ptr)

BOOL PhysicalPointer(PASPI_SRB_EXECUTE_IO pSRB, PUCHAR dataBufferPtr, 
		     BOOL bVDDCall, PULONG pDataHandle)
{
  /* If the command does not transfer data, no pointer is required */
  if (pSRB -> DataXferLen == 0)
  {
    pSRB -> ppDataBuffer = 0;
    return TRUE;
  }

  if (DevHelp_Lock(SELECTOROF(dataBufferPtr),
		   1,	/* long term lock */
		   0,	/* block until available */
		   pDataHandle))
  {
    DEBUG_MSG("VASPIDD: data lock failed\r\n");
    *pDataHandle = -1;
    if (!bVDDCall)
      return FALSE;
  }

  if (!bVDDCall &&
      DevHelp_VerifyAccess(SELECTOROF(dataBufferPtr), OFFSETOF(dataBufferPtr),
			   (USHORT) pSRB -> DataXferLen, VERIFY_READWRITE))
  {
    DevHelp_UnLock(*pDataHandle);
    return FALSE;
  }

  return (DevHelp_VirtToPhys(dataBufferPtr, &(pSRB -> ppDataBuffer)) == 0);
}

void _far _loadds ASPI_Post(USHORT DataSeg, PASPI_SRB_HEADER pSRBH)
{
  USHORT awakeCount;

  /* DEBUG_MSG("VASPIDD: post\r\n"); */

  /* Wake up the thread which made the request. */
  DevHelp_ProcRun((ULONG) pSRBH, &awakeCount);

  /* Return control to OS2ASPI.DMD */
}

int DriverEntry(PRPH rp)
{
  ULONG dataHandle;		/* Handle for locked data buffer */
  ULONG parameterHandle;	/* Handle for locked memory */
  ULONG parameterPtr;		/* Temporary variable for physical ptr */
  PRP_GENIOCTL rpIOCTL;
  PASPI_SRB_EXECUTE_IO pSRB_IO;
  PASPI_SRB_RESET_DEVICE pSRB_RD;
  USHORT result = STDON;
  int Init(PRPINITIN pRPI);
  BOOL bVDDCall = FALSE;
  BOOL bPost = FALSE;

  switch (rp -> Cmd)
  {

  case RPINIT:
    return Init((PRPINITIN) rp);

  case RPOPEN:
    return STDON;

  case RPCLOSE:
    return STDON;

  case RPIOCTL:

    rpIOCTL = (PRP_GENIOCTL) rp;

    /* BREAKPOINT; */

    if (rpIOCTL -> Category == ASPI_DEBUG)
    {
      switch (rpIOCTL -> Function)
      {
      case 0:
	DebugPort = 0;
	break;
      case 1:
	DebugPort = 0x03F8;
	break;
      case 2:
	DebugPort = 0x02F8;
	break;
      }
      return STDON;
    }

    if (rpIOCTL -> Category == ASPI_DEMO)
      return STDON | (bDemoVersion ? 0 : STERR);

    if (rpIOCTL -> Category == ASPI_VDDREQ)
      bVDDCall = TRUE;
    else
      if (rpIOCTL -> Category != ASPI_REQ)
	return (STDON | STERR | ERROR_BAD_COMMAND);

    if (DevHelp_Lock(SELECTOROF(rpIOCTL -> ParmPacket),
		     1,		/* long term lock */
		     0,		/* block until available */
		     (PULONG) &parameterHandle))
    {
      DEBUG_MSG("VASPIDD: parameter lock failed\r\n");
      return STDON | STERR | ERROR_GEN_FAILURE;
    }

    switch (rpIOCTL -> Function)
    {

    case ASPI_CMD_ADAPTER_INQUIRY:
    case ASPI_CMD_GET_DEVICE_TYPE:

      DEBUG_ASPI(rpIOCTL);

      if (!bVDDCall && 
	  DevHelp_VerifyAccess(SELECTOROF(rpIOCTL -> ParmPacket),
			       OFFSETOF(rpIOCTL -> ParmPacket),
			       sizeof(ASPI_SRB_HEADER), VERIFY_READWRITE))
      {
	result = STDON | STERR | ERROR_GEN_FAILURE;
	break;
      }

      (*OS2ASPI.pEntryPoint)(OS2ASPI.pDS,
			     (PASPI_SRB_HEADER) rpIOCTL -> ParmPacket);

      break;

    case ASPI_CMD_EXECUTE_IO:

      DEBUG_ASPI(rpIOCTL);

      if (!bVDDCall &&
	  DevHelp_VerifyAccess(SELECTOROF(rpIOCTL -> ParmPacket),
			       OFFSETOF(rpIOCTL -> ParmPacket),
			       sizeof(ASPI_SRB_EXECUTE_IO), VERIFY_READWRITE))
      {
	result = STDON | STERR | ERROR_GEN_FAILURE;
	break;
      }

      if (DevHelp_VirtToPhys(rpIOCTL -> ParmPacket, &parameterPtr))
      {
	result = STDON | STERR | ERROR_GEN_FAILURE;
	break;
      }

      pSRB_IO = (PASPI_SRB_EXECUTE_IO) (rpIOCTL -> ParmPacket);

      /* Copy the physical SRB pointer into the SRB */
      pSRB_IO -> ppSRB = parameterPtr;
      /* Copy the post routine address into the SRB */
      pSRB_IO -> PM_PostAddress = ASPI_Post;
      /* Copy the post routine data selector to the SRB */
      pSRB_IO -> PM_DataSeg = OS2ASPI.pDS;

      /* always use post mode, even if not requested */
      bPost = (pSRB_IO -> SRBHdr.ASPIReqFlags & ASPI_REQFLAG_POST_ENABLE) != 0;
      pSRB_IO -> SRBHdr.ASPIReqFlags |= ASPI_REQFLAG_POST_ENABLE;

      /* Create a physical pointer to the data buffer */
      if (!PhysicalPointer(pSRB_IO, rpIOCTL -> DataPacket, 
			   bVDDCall, &dataHandle))
      {
	result = STDON | STERR | ERROR_GEN_FAILURE;
	break;
      }

      /* Send the ASPI request to OS2ASPI.DMD */
      (*OS2ASPI.pEntryPoint)(OS2ASPI.pDS,
			     (PASPI_SRB_HEADER) rpIOCTL -> ParmPacket);

      DISABLE;

      /* Make sure the command has completed */
      while (pSRB_IO -> SRBHdr.ASPIStatus == ASPI_STATUS_IN_PROGRESS)
      {
	DevHelp_ProcBlock(parameterPtr, (ULONG) -1, WAIT_IS_INTERRUPTABLE);
	DISABLE;
      }

      ENABLE;

      /* undo undercover posting */
      if (!bPost)
	pSRB_IO -> SRBHdr.ASPIReqFlags &= ~ASPI_REQFLAG_POST_ENABLE;

      /* Unlock the data buffer */
      if (pSRB_IO -> DataXferLen && dataHandle != -1)
	DevHelp_UnLock(dataHandle);

      DEBUG_RES(rpIOCTL);

      break;

    case ASPI_CMD_ABORT_IO:

      DEBUG_ASPI(rpIOCTL);

      if (!bVDDCall && 
	  DevHelp_VerifyAccess(SELECTOROF(rpIOCTL -> ParmPacket),
			       OFFSETOF(rpIOCTL -> ParmPacket),
			       sizeof(ASPI_SRB_ABORT_IO), VERIFY_READWRITE))
      {
	result = STDON | STERR | ERROR_GEN_FAILURE;
	break;
      }

      /* Create a physical pointer to the SRB */
      if (DevHelp_VirtToPhys(rpIOCTL -> ParmPacket, &parameterPtr))
      {
	result = STDON | STERR | ERROR_GEN_FAILURE;
	break;
      }

      /* Copy the physical SRB pointer into the SRB */
      ((PASPI_SRB_ABORT_IO) (rpIOCTL -> ParmPacket)) -> ppSRB = parameterPtr;

      /* Send the ASPI request to OS2ASPI.DMD */
      (*OS2ASPI.pEntryPoint)(OS2ASPI.pDS,
			     (PASPI_SRB_HEADER) rpIOCTL -> ParmPacket);

      break;

    case ASPI_CMD_RESET_DEVICE:

      DEBUG_ASPI(rpIOCTL);

      if (!bVDDCall &&
	  DevHelp_VerifyAccess(SELECTOROF(rpIOCTL -> ParmPacket),
			       OFFSETOF(rpIOCTL -> ParmPacket),
			       sizeof(ASPI_SRB_RESET_DEVICE), VERIFY_READWRITE))
      {
	result = STDON | STERR | ERROR_GEN_FAILURE;
	break;
      }

      pSRB_RD = (PASPI_SRB_RESET_DEVICE) (rpIOCTL -> ParmPacket);

      /* Copy the post routine address into the SRB */
      pSRB_RD -> PM_PostAddress = ASPI_Post;
      /* Copy the post routine data selector to the SRB */
      pSRB_RD -> PM_DataSeg = OS2ASPI.pDS;

      /* Send the ASPI request to OS2ASPI.DMD */
      (*OS2ASPI.pEntryPoint)(OS2ASPI.pDS,
			     (PASPI_SRB_HEADER) rpIOCTL -> ParmPacket);

      DISABLE;

      /* Make sure the command has completed */
      if (pSRB_RD -> SRBHdr.ASPIReqFlags & ASPI_REQFLAG_POST_ENABLE)
	while (pSRB_RD -> SRBHdr.ASPIStatus == ASPI_STATUS_IN_PROGRESS)
	{
	  DevHelp_ProcBlock(parameterPtr, (ULONG) -1, WAIT_IS_INTERRUPTABLE);
	  DISABLE;
	}

      ENABLE;

      DEBUG_RES(rpIOCTL);

      break;

    default:
      DEBUG_ASPI(rpIOCTL);
      result = STDON | STERR | ERROR_BAD_COMMAND;
      break;
    }

    /* Unlock the parameter packet.  */
    DevHelp_UnLock(parameterHandle);

    if (result != STDON)
    {
      DEBUG_MSG("VASPIDD: call failed\r\n");
      ((PASPI_SRB_HEADER) (rpIOCTL -> ParmPacket)) -> ASPIStatus = 
	ASPI_STATUS_ABORTED;
    }

    return result;

  default:
    return STDON | STERR | ERROR_BAD_COMMAND;
  }
}

/* initialization time only code */

extern APIRET APIENTRY DosPutMessage(USHORT hf, USHORT cbMsg, PCHAR pchMsg);

char DevName[]   = "\r\nVirtual ASPI Device Driver, Version " VERSION;

#ifdef EES
char Copyright[] = "\r\nEDV + Elektronic Systeme\r\n";
char Copyleft[]  = 
#else
char Copyright[] = 
#endif
                   "\r\n(C) 1994 ARS Computer und Consulting GmbH\r\n";

char Fail[]      = "This device driver cannot be loaded.\r\n";
char NoASPI[]    = "No ASPI manager (OS2ASPI.DMD) found.\r\n";
char CrLf[]      = "\r\n";

char DemoVer[]   = ", Evaluation Version";
char SerialNum[] = ", SN 00000000";

struct 
{ 
  LONG Num; 
  LONG Key; 
} 
Serial = {0xC6AB6894, 0xAAB149AA};

BOOL Legal(void)
{
  extern Encode(void *buffer, int bytes, char *password);

  /* BREAKPOINT; */

  Encode(&Serial, sizeof(Serial), "VASPI");

  if (Serial.Num + Serial.Key != 0)
  {
    DosPutMessage(1, sizeof(Fail) - 1, Fail);
    return FALSE;
  }

  bDemoVersion = (Serial.Num == 0);

  HexString(Serial.Num, SerialNum + 5, 8);

  DosPutMessage(1, sizeof(DevName) - 1, DevName);

  if (bDemoVersion)
    DosPutMessage(1, sizeof(DemoVer) - 1, DemoVer);
  else
    DosPutMessage(1, sizeof(SerialNum) - 1, SerialNum);

  DosPutMessage(1, sizeof(Copyright) - 1, Copyright);

  return TRUE;
}

int Init(PRPINITIN pRPI)
{
  PRPINITOUT pRPO = (PRPINITOUT) pRPI;
  PSZ pStr;
  extern int _edata, _etext;

  /* Initialize DevHelp entry point */
  Device_Help = pRPI -> DevHlpEP;

  /* Write copyright message etc. */
  if (!Legal())
    return RPDONE | RPERR | ERROR_BAD_COMMAND;

  /* check if debugging is required */
  for (pStr = pRPI -> InitArgs; *pStr != 0 && *pStr != ' '; pStr++);
  if (*pStr)
  {
    while (*pStr == ' ')
      pStr++;

    if (pStr[0] == '/' && (pStr[1] == 'c' || pStr[1] == 'C'))
      switch(pStr[2])
      {
      case '1':
	DebugPort = 0x03F8;
	break;
      case '2':
	DebugPort = 0x02F8;
	break;
      default:
	DebugPort = 0;
	break;
      }
  }
  
  /* Attach to OS2ASPI.DMD */
  if (DevHelp_AttachDD("SCSIMGR$", (NPBYTE) &OS2ASPI))
  {
    DosPutMessage(1, sizeof(NoASPI) - 1, NoASPI);
    /* The driver failed to install */
    pRPO -> CodeEnd = 0;
    pRPO -> DataEnd = 0;
    return RPDONE | RPERR | ERROR_BAD_COMMAND;
  }
  else
  {
    DosPutMessage(1, sizeof(CrLf) - 1, CrLf);
    /* The driver was succesfully installed */
    pRPO -> CodeEnd = (USHORT) &_etext;
    pRPO -> DataEnd = (USHORT) &_edata;
    return RPDONE;
  }
}
