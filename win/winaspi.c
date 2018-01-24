/* winaspi.c
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1993 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Wed Mar 02 1994
 */

static char *rcsid =
"$Id: winaspi.c,v 1.5 1994/05/26 10:55:52 rommel Exp $";
static char *rcsrev = "$Revision: 1.5 $";

/*
 * $Log: winaspi.c,v $
 * Revision 1.5  1994/05/26 10:55:52  rommel
 * fix for NetWare Requester conflict
 *
 * Revision 1.4  1994/03/29 16:06:24  rommel
 * third release
 *
 * Revision 1.3  1994/03/03 16:51:37  rommel
 * first working version
 *
 * Revision 1.2  1994/03/02 15:42:59  rommel
 * first (empty) prototype
 *
 * Revision 1.1  1994/03/02 14:54:16  rommel
 * Initial revision
 */

#define MAXBUFFER   65536L
#define MAXSRB      1024

#define MPLEX       0x2F
#define MPLEX_POST  0xC701
#define MPLEX_ASPI  0xC702

#include <windows.h>
extern DWORD FAR PASCAL GlobalDosAlloc(DWORD);
extern WORD FAR PASCAL GlobalDosFree(WORD);

#include <memory.h>

#include "../sdk/windows/winaspi.h"
typedef SRB_ExecSCSICmd6 FAR *LPSRBIO;
typedef VOID (FAR PASCAL *LPPOSTPROC)(LPSRB lpSRB);

WORD wSegBuffer, wSegSRB;
WORD wSelBuffer, wSelSRB;
LPBYTE lpBuffer;
LPSRBIO lpLowSRB;

WORD FAR PASCAL SendASPICommand(LPSRB lpSRB)
{
  LPSRBIO lpSRBIO;
  WORD nSRBSize;
  LPBYTE lpData;
  DWORD nDataSize;
  LPPOSTPROC ASPIPostProc;
  BOOL bPost;

  lpSRBIO   = (LPSRBIO) lpSRB;

  lpData    = lpSRBIO -> SRB_BufPointer;
  nDataSize = lpSRBIO -> SRB_BufLen;

  if (nDataSize > 65536L)
    return SS_BUFFER_TO_BIG;

  switch (lpSRBIO -> SRB_Cmd)
  {
  case SC_HA_INQUIRY:
    nSRBSize = sizeof(SRB_HAInquiry);
    break;
  case SC_GET_DEV_TYPE:
    nSRBSize = sizeof(SRB_GDEVBlock);
    break;
  case SC_EXEC_SCSI_CMD:
    nSRBSize = sizeof(SRB_ExecSCSICmd6);
    nSRBSize += lpSRBIO -> SRB_CDBLen - 6;
    nSRBSize += lpSRBIO -> SRB_SenseLen - SENSE_LEN;
    break;
  case SC_ABORT_SRB:
    nSRBSize = sizeof(SRB_Abort);
    break;
  case SC_RESET_DEV:
    nSRBSize = sizeof(SRB_BusDeviceReset);
    break;
  }

  _fmemcpy((LPBYTE) lpLowSRB, (LPBYTE) lpSRBIO, nSRBSize);
  bPost = (lpSRBIO -> SRB_Flags & SRB_POSTING) != 0;
  lpLowSRB -> SRB_Flags &= ~SRB_POSTING;
  lpLowSRB -> SRB_BufPointer = (LPBYTE) MAKELONG(0, wSegBuffer);

  if (nDataSize)
    _fmemcpy(lpBuffer, lpData, (size_t) nDataSize);

  _asm
  {
    mov bx,wSegSRB
    mov ax,MPLEX_ASPI
    int MPLEX
  }

  _fmemcpy((LPBYTE) lpSRBIO, (LPBYTE) lpLowSRB, nSRBSize);

  if (nDataSize)
    _fmemcpy(lpData, lpBuffer, (size_t) nDataSize);

  if (bPost)
  {
    ASPIPostProc = lpSRBIO -> SRB_PostProc;
    ASPIPostProc(lpSRB);
  }

  return lpSRBIO -> SRB_Status;
}

WORD FAR PASCAL GetASPISupportInfo(VOID)
{
  SRB_HAInquiry SRB;

  _fmemset(&SRB, 0, sizeof(SRB)); /* that is a shortcut for INQUIRY ... */

  if (SendASPICommand((LPSRB) &SRB) != SS_COMP || SRB.SRB_Status != SS_COMP)
    return (SS_NO_ASPI << 8) | 0;

  return (SS_COMP << 8) | SRB.HA_Count;
}

BOOL FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg,
                        WORD cbHeapSize, LPSTR lpszCmdLine)
{
  DWORD dwRes;

  if ( (dwRes = GlobalDosAlloc(MAXBUFFER)) == 0 )
    return FALSE;

  wSegBuffer   = HIWORD(dwRes);
  wSelBuffer   = LOWORD(dwRes);
  lpBuffer     = (LPBYTE) MAKELONG(0, wSelBuffer);

  if ( (dwRes = GlobalDosAlloc(MAXSRB)) == 0 )
    return FALSE;

  wSegSRB   = HIWORD(dwRes);
  wSelSRB   = LOWORD(dwRes);
  lpLowSRB  = (LPSRBIO) MAKELONG(0, wSelSRB);

  return TRUE;
}

BOOL FAR PASCAL WEP(WORD nExitType)
{
  GlobalDosFree(wSelBuffer);
  GlobalDosFree(wSelSRB);

  return TRUE;
}

/* end of winaspi.c */
