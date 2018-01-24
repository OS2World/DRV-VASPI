/* aspi.c
 *
 * Sample DOS ASPI application
 * (C) 1994 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Tue Dec 21 1993
 */
 
static char *rcsid =
"$Id: aspi.c,v 1.4 1994/02/02 12:54:02 rommel Exp $";
static char *rcsrev = "$Revision: 1.4 $";

/* $Log: aspi.c,v $
 * Revision 1.4  1994/02/02 12:54:02  rommel
 * beta release
 *
 * Revision 1.3  1994/02/02 12:52:38  rommel
 * fixed ASPI detection, error handling
 *
 * Revision 1.2  1994/01/27 13:09:51  rommel
 * cleanup
 *
 * Revision 1.1  1994/01/16 11:54:34  rommel
 * Initial revision
 * */

#include <stdio.h>
#include <stdlib.h>

#include "aspi.h"

#define MAX_CDB_LENGTH 12
#define MAX_SENSE_LENGTH 18

static void FAR *aspi;

int InitASPI(void)
{
  static char devname[] = "SCSIMGR$";

  _asm
  {
    mov	  ax,0x3D00
    mov	  dx,offset devname
    int	  0x21
    mov	  bx,ax
    mov   ax,0
    jc	  done
    mov	  ax,0x4402
    mov	  dx,offset aspi
    mov	  cx,4
    int	  0x21
    mov   ax,0
    jc	  done
    mov	  ax,0x3E00
    int	  0x21
    mov   ax,1
  done:
  }
}

void ASPI(void FAR *srbptr)
{
  _asm
  {
    les   bx,srbptr
    push  es
    push  bx
    call  aspi
  }

  while (((PASPI_SRB_HEADER) srbptr) -> ASPIStatus == ASPI_STATUS_IN_PROGRESS);
}

int main(int argc, char **argv)
{
  ASPI_SRB_INQUIRY inquiry;
  ASPI_SRB_DEVICE_TYPE type;
  ASPI_SRB_EXECUTE_IO *exec;
  char inq[64];
  int adapters, adapter, target, size;

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

  ASPI(&inquiry);
  
  if (inquiry.SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR)
  {
    printf("\nSCSI manager: Name '%16.16s', %d adapter(s)\n",
	   inquiry.ManagerName, inquiry.AdapterCount);

    adapters = inquiry.AdapterCount;

    for (adapter = 0; adapter < adapters; adapter++)
    {
      memset(&inquiry, 0, sizeof(inquiry));
    
      inquiry.SRBHdr.CommandCode = ASPI_CMD_ADAPTER_INQUIRY;
      inquiry.SRBHdr.AdapterIndex = adapter;

      ASPI(&inquiry);

      if (inquiry.SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR)
	printf("\nAdapter %d: Target %d, Name '%16.16s'\n", 
	       adapter, inquiry.AdapterTargetID, inquiry.AdapterName);
      else
	continue;

      for (target = 0; target <= 7; target++)
      {
	memset(&type, 0, sizeof(type));

	type.SRBHdr.CommandCode = ASPI_CMD_GET_DEVICE_TYPE;
	type.SRBHdr.AdapterIndex = adapter;
	type.DeviceTargetID = target;
	type.DeviceTargetLUN = 0;

	ASPI(&type);

	if (type.SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR)
	  printf("  Target %d, LUN 0: Type %d", target, type.DeviceType);
	else
	  continue;

	memset(exec, 0, size);
	memset(inq, 0, sizeof(inq));

	exec -> SRBHdr.CommandCode = ASPI_CMD_EXECUTE_IO;
	exec -> SRBHdr.ASPIReqFlags = ASPI_REQFLAG_DIR_TO_HOST;
	exec -> SRBHdr.AdapterIndex = adapter;
	exec -> DeviceTargetID = target;
	exec -> DeviceTargetLUN = 0;
	exec -> DataXferLen = sizeof(inq);
	exec -> SenseDataLen = MAX_SENSE_LENGTH;
	exec -> ppDataBuffer = inq;
	exec -> SCSICDBLen = 6;
	exec -> SCSICDBStart[0] = 0x12;
	exec -> SCSICDBStart[4] = sizeof(inq);

	ASPI(exec);

	if (exec -> SRBHdr.ASPIStatus == ASPI_STATUS_NO_ERROR &&
	    exec -> HostStatus == ASPI_HSTATUS_NO_ERROR &&
	    exec -> TargetStatus == ASPI_TSTATUS_NO_ERROR)
	  printf(", Name '%s'\n", inq + 8);
	else
	  printf(" (error retrieving name)\n");
      }
    }
  }

  return 0;
}

/* end of aspi.c */
