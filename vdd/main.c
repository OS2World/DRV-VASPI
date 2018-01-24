/* main.c
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1994 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Mon Dec 20 1993
 */
 
static char *rcsid =
"$Id: main.c,v 1.13 1994/09/23 19:44:31 rommel Exp $";
static char *rcsrev = "$Revision: 1.13 $";

/*
 * $Log: main.c,v $
 * Revision 1.13  1994/09/23 19:44:31  rommel
 * fixed DOS box exit bug when no ASPI installed
 *
 * Revision 1.12  1994/09/14 12:15:14  rommel
 * new init method, allows real stub for specific DOS sessions
 *
 * Revision 1.11  1994/04/20 11:53:48  rommel
 * fourth release
 *
 * Revision 1.10  1994/03/29 16:09:02  rommel
 * third release
 *
 * Revision 1.9  1994/03/09 17:29:54  rommel
 * new version using VDHPushFarCall
 *
 * Revision 1.8  1994/03/01 09:13:22  rommel
 * second release, /ON and /OFF options
 *
 * Revision 1.7  1994/02/04 17:44:01  rommel
 * first release
 *
 * Revision 1.6  1994/02/02 13:22:39  rommel
 * beta release
 *
 * Revision 1.5  1994/01/27 12:59:36  rommel
 * first test release
 *
 * Revision 1.4  1994/01/09 20:19:53  rommel
 * first somewhat working version
 *
 * Revision 1.3  1993/12/22 18:21:15  rommel
 * Added opening of physical device.
 *
 * Revision 1.2  1993/12/22  13:06:08  rommel
 * Restructured, stub DD added.
 *
 * Revision 1.1  1993/12/20  16:16:41  rommel
 * Initial revision
 * */

#include <mvdm.h>

#include <ctype.h>

#include "vaspi.h"
#include "stub.h"

#pragma data_seg(SWAPINSTDATA)

HVDM owner_VDM = 0;
HHOOK hTimerHook;                           /* post timer hook handle */
HHOOK hRetHook;                             /* post return hook handle */
HHOOK hBPHook;		                    /* entry hook handle */
VPVOID vpBP;				    /* breakpoint address for ARPL */

HFILE hASPI;
BOOL bDemoVersion;
BOOL bASPIAvailable, bASPIInstalled;

#define BUFFERSIZE 16 /* 64k */
PCHAR pBuffer;

static ULONG nAction;

#pragma data_seg(CSWAP_DATA)

static char *szPDDName  = PDDNAME;
static char *szProperty = PROPERTYNAME;
static BOOL bFirst = TRUE;
static HHOOK hDemoHook;                     /* demo expire timer hook handle */

#pragma alloc_text(CSWAP_TEXT,InstallStub,ExpireDemo,Create,Terminate)

/* stub DOS device driver installation */

BOOL HOOKENTRY InitInt(PCRF pcrf)
{
  /* vdhInt3(); */

  if (AX(pcrf) == MPLEX_ASPI)
  {
    AX(pcrf) = bASPIAvailable;
    BX(pcrf) = OFFSETOF16(vpBP);
    ES(pcrf) = SEGMENTOF16(vpBP);
  }

  return TRUE;
}

BOOL PRIVENTRY InstallStub(HVDM hVDM)
{
  PBYTE dosDDstart;		/* where the Dos Device Driver will go  */

  /* vdhInt3(); */

  /* install INT hook for initialization */

  if ((VDHInstallIntHook(hVDM, INIT_INT, (PUSERHOOK) InitInt, 0)) == 0)
    return FALSE;

  /* allocate the memory for the stub driver and copy it there */

  if (!(dosDDstart = VDHAllocDosMem(StubLength)))
    return FALSE;

  memcpy(dosDDstart, _StubStart, StubLength);

  if ((hBPHook = VDHAllocHook(VDH_BP_HOOK, (PFNARM) VirtualASPI, 0)) == NULL)
    return FALSE;

  vpBP = VDHArmBPHook(hBPHook); /* get ARPL address  */

  if (!VDHSetDosDevice((VPDOSDDTYPE) VPFROMP(dosDDstart)))
    return FALSE;

  return TRUE;
}

/* demo version expiration timer */

BOOL HOOKENTRY ExpireDemo(PVOID pHookData, PCRF pcrf)
{
  bASPIAvailable = FALSE;

  VDHFreePages(pBuffer);
  VDHClose(hASPI);

  return TRUE;
}

/* VDM creation and termination hook entries */

BOOL HOOKENTRY Create(HVDM hVDM)
{
  owner_VDM = hVDM;
  pBuffer = 0;

  /* vdhInt3(); */

  bASPIAvailable = VDHQueryProperty(szProperty);
  bASPIInstalled = FALSE;

  if (bASPIAvailable)
  {
    if (VDHOpen(szPDDName, &hASPI, &nAction, 0L, 0, 
		VDHOPEN_FILE_OPEN,
		VDHOPEN_ACCESS_READWRITE | 
		VDHOPEN_SHARE_DENYNONE |
		VDHOPEN_FLAGS_FAIL_ON_ERROR, 0L) == 0)
      return TRUE;

    bDemoVersion = VDHDevIOCtl(hASPI, ASPI_DEMO, 0, 0, 0, 0, 0, 0, 0);

    if (bDemoVersion)
      if (!bFirst)
	bASPIAvailable = FALSE;
      else
      {
	bFirst = FALSE;

	if ((hDemoHook = VDHAllocHook(VDH_TIMER_HOOK, (PFNARM) ExpireDemo, 0)) == NULL)
	  return FALSE;

	VDHArmTimerHook(hDemoHook, EXPIRE * 60 * 1000, owner_VDM);
      }

    if (bASPIAvailable)
    {
      if ((hTimerHook = VDHAllocHook(VDH_TIMER_HOOK, (PFNARM) PostTimer, 0)) == NULL)
	return FALSE;

      if ((hRetHook = VDHAllocHook(VDH_RETURN_HOOK, (PFNARM) PostReturn, 0)) == NULL)
	return FALSE;

      if ((pBuffer = VDHAllocPages(0, BUFFERSIZE, VDHAP_SYSTEM)) == 0)
	return FALSE;

      bASPIInstalled = TRUE;
    }
    else
      VDHClose(hASPI);
  }

  if (!InstallStub(hVDM))
    return FALSE;

  return TRUE;
}

BOOL HOOKENTRY Terminate(HVDM hVDM)
{
  /* vdhInt3(); */

  if (bASPIInstalled)
  {
    VDHFreePages(pBuffer);
    VDHClose(hASPI);
  }

  owner_VDM = 0;

  return TRUE;
}

/* init entry point called by system at load time */

#pragma alloc_text(CINIT_TEXT,Init)
#pragma entry(Init)

BOOL EXPENTRY Init(PSZ szArgs)
{
  BOOL bDefault = FALSE;

  /* vdhInt3(); */

  if (szArgs != 0 && szArgs[0] == '/' &&
      (szArgs[1] == 'O' || szArgs[1] == 'o') &&
      (szArgs[2] == 'N' || szArgs[2] == 'n'))
    bDefault = TRUE;

  /* Register a VDM creation handler entry point */

  if ((VDHInstallUserHook((ULONG) VDM_CREATE, (PUSERHOOK) Create)) == 0)
    return FALSE;

  /* Register ion handler entry point*/

  if ((VDHInstallUserHook((ULONG) VDM_TERMINATE, (PUSERHOOK) Terminate)) == 0)
    return FALSE;

  /* Register a DOS session property */
  if ((VDHRegisterProperty(szProperty, 0, 0, VDMP_BOOL, VDMP_ORD_OTHER, 
			   VDMP_CREATE, (PVOID) bDefault, 0, 0)) == 0)
    return FALSE;

  return TRUE;
}

/* end of main.c */
