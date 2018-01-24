/* aspi2.c
 *
 * Sample OS/2 ASPI application
 * (C) 1994 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Tue Dec 21 1993
 */
 
static char *rcsid =
"$Id: aspi2.c,v 1.6 1994/05/10 09:28:01 rommel Exp $";
static char *rcsrev = "$Revision: 1.6 $";

/* RCS change log:
 *
 * $Log: aspi2.c,v $
 * Revision 1.6  1994/05/10 09:28:01  rommel
 * comment correction
 *
 * Revision 1.5  1994/05/10 09:17:56  rommel
 * detection of up to eight LUN's per target added
 *
 * Revision 1.4  1994/03/01 09:11:08  rommel
 * bug fixes for second release
 *
 * Revision 1.3  1994/02/02 12:54:02  rommel
 * beta release
 *
 * Revision 1.2  1994/01/27 13:10:00  rommel
 * cleanup
 *
 * Revision 1.1  1994/01/16 11:54:34  rommel
 * Initial revision
 * */

#define	INCL_DOSDEVICES
#define	INCL_DOSDEVIOCTL
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>

#include "aspi.h"

#define ASPI_REQ 0x80
#define MAX_CDB_LENGTH 12
#define MAX_SENSE_LENGTH 18

static HFILE hASPI;

int InitASPI(void)
{
#ifdef __32BIT__
  ULONG nAction;
#else
  USHORT nAction;
#endif

  return (DosOpen("VASPIDD$", &hASPI, &nAction, 0L, 0, FILE_OPEN,
		  OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |
		  OPEN_FLAGS_FAIL_ON_ERROR, 0L) == 0);
}

int ASPI(PVOID pParms, USHORT cbParms, PVOID pData, USHORT cbData)
{
#ifdef __32BIT__
  ULONG ulParmLengthInOut = cbParms, ulDataLengthInOut = cbData;
  return DosDevIOCtl(hASPI, ASPI_REQ, 
		     ((PASPI_SRB_HEADER) pParms) -> CommandCode,
		     pParms, cbParms, &ulParmLengthInOut,
		     pData, cbData, &ulDataLengthInOut);
#else
  return DosDevIOCtl2(pData, cbData, pParms, cbParms, 
		      ((PASPI_SRB_HEADER) pParms) -> CommandCode, 
		      ASPI_REQ, hASPI);
#endif
}

int main(int argc, char **argv)
{
  ASPI_SRB_INQUIRY inquiry;
  ASPI_SRB_DEVICE_TYPE type;
  ASPI_SRB_EXECUTE_IO *exec;
  char inq[64];
  int rc, adapters, adapter, target, lun, size;

  size = sizeof(ASPI_SRB_EXECUTE_IO) + MAX_CDB_LENGTH + MAX_SENSE_LENGTH;
  exec = (ASPI_SRB_EXECUTE_IO *) malloc(size);

  if (!InitASPI()) 
  {
    printf("No ASPI found.\n");
    return 1;
  }
  
  memset(&inquiry, 0, sizeof(inquiry));

  inquiry.SRBHdr.CommandCode = ASPI_CMD_ADAPTER_INQUIRY;
  inquiry.SRBHdr.AdapterIndex = 0;

  rc = ASPI(&inquiry, sizeof(inquiry), NULL, 0);
  
  if (rc == 0 && inquiry.SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR)
  {
    printf("\nSCSI manager: Name '%-16.16s', %d adapter(s)\n",
	   inquiry.ManagerName, inquiry.AdapterCount);

    adapters = inquiry.AdapterCount;

    for (adapter = 0; adapter < adapters; adapter++)
    {
      memset(&inquiry, 0, sizeof(inquiry));
    
      inquiry.SRBHdr.CommandCode = ASPI_CMD_ADAPTER_INQUIRY;
      inquiry.SRBHdr.AdapterIndex = adapter;

      rc = ASPI(&inquiry, sizeof(inquiry), NULL, 0);

      if (rc == 0 && inquiry.SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR)
	printf("\nAdapter %d: Target %d, Name '%-16.16s'\n", 
	       adapter, inquiry.AdapterTargetID, inquiry.AdapterName);
      else
	continue;

      for (target = 0; target <= 7; target++)
	for (lun = 0; lun <= 7; lun++)
	{
	  memset(&type, 0, sizeof(type));

	  type.SRBHdr.CommandCode = ASPI_CMD_GET_DEVICE_TYPE;
	  type.SRBHdr.AdapterIndex = adapter;
	  type.DeviceTargetID = target;
	  type.DeviceTargetLUN = lun;

	  rc = ASPI(&type, sizeof(type), NULL, 0);

	  if (rc == 0 && type.SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR)
	    printf("  Target %d, LUN %d: Type %d", target, lun, type.DeviceType);
	  else
	    continue;

	  memset(exec, 0, size);
	  memset(inq, 0, sizeof(inq));

	  exec -> SRBHdr.CommandCode = ASPI_CMD_EXECUTE_IO;
	  exec -> SRBHdr.ASPIReqFlags = 
	    ASPI_REQFLAG_DIR_TO_HOST | ASPI_REQFLAG_POST_ENABLE;
	  exec -> SRBHdr.AdapterIndex = adapter;
	  exec -> DeviceTargetID = target;
	  exec -> DeviceTargetLUN = lun;
	  exec -> DataXferLen = sizeof(inq);
	  exec -> SenseDataLen = MAX_SENSE_LENGTH;
	  exec -> SCSICDBLen = 6;
	  exec -> SCSICDBStart[0] = 0x12;
	  exec -> SCSICDBStart[4] = sizeof(inq);

	  rc = ASPI(exec, size, &inq, sizeof(inq));

	  if (rc == 0 && 
	      exec -> SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR &&
	      exec -> HostStatus == ASPI_HSTATUS_NO_ERROR &&
	      exec -> TargetStatus == ASPI_TSTATUS_NO_ERROR)
	    printf(", Name '%-28.28s'\n", inq + 8);
	  else
	    printf(" (error retrieving name)\n");
	}
    }
  }

  return 0;
}

/* end of aspi2.c */
