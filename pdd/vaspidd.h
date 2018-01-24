/* vaspidd.h
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1994 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Wed Dec 22 1993
 */

/* developed from sample code from Adaptec */

/* $Id: vaspidd.h,v 1.4 1994/04/20 11:53:22 rommel Exp $ */
/* $Revision: 1.4 $ */

/* $Log: vaspidd.h,v $
/* Revision 1.4  1994/04/20 11:53:22  rommel
/* fourth release
/*
/* Revision 1.3  1994/03/09 16:45:22  rommel
/* added debug mode
/*
 * Revision 1.2  1994/02/02 13:20:27  rommel
 * beta release
 *
 * Revision 1.1  1994/01/09 20:24:50  rommel
 * Initial revision
 * */

typedef USHORT       OFFSET;
typedef USHORT       SEG;

#define DAW_CHR      0x8000   /* Character device driver */
#define DAW_IDC      0x4000   /* IDC available */
#define DAW_IBM      0x2000   /* Non-IBM block device driver */
#define DAW_SHR      0x1000   /* Support device sharing */
#define DAW_OPN      0x0800   /* Device supports open/close */
#define DAW_LEVEL1   0x0080   /* Level 1 OS/2 */
#define DAW_LEVEL2   0x0100   /* Level 2 DevIOCtl2 */
#define DAW_LEVEL3   0x0180   /* Level 3 Bit Strip */
#define DAW_GIO      0x0040   /* Support generic IOCtl */
#define DAW_CLK      0x0008   /* Clock device */
#define DAW_NUL      0x0004   /* NULL device */
#define DAW_SCR      0x0002   /* STDOUT (screen) */
#define DAW_KBD      0x0001   /* STDIN (keyboard) */

/* IOCtl Category */
#define ASPI_REQ     0x80
#define ASPI_VDDREQ  0x81
#define ASPI_DEBUG   0x90
#define ASPI_DEMO    0x91

/* Request packet commands */
#define RPINIT       0x00
#define RPOPEN       0x0D
#define RPCLOSE      0x0E
#define RPIOCTL      0x10
#define RPDEINSTALL  0x14

/* RPstatus bit values */
#define RPERR        0x8000
#define RPDEV        0x4000
#define RPBUSY       0x0200
#define RPDONE       0x0100

/* Error codes returned in RPstatus */
#define ERROR_WRITE_PROTECT         0x0000
#define ERROR_BAD_UNIT              0x0001
#define ERROR_NOT_READY             0x0002
#define ERROR_BAD_COMMAND           0x0003
#define ERROR_CRC                   0x0004
#define ERROR_BAD_LENGTH            0x0005
#define ERROR_SEEK                  0x0006
#define ERROR_NOT_DOS_DISK          0x0007
#define ERROR_SECTOR_NOT_FOUND      0x0008
#define ERROR_OUT_OF_PAPER          0x0009
#define ERROR_WRITE_FAULT           0x000A
#define ERROR_READ_FAULT            0x000B
#define ERROR_GEN_FAILURE           0x000C
#define ERROR_DISK_CHANGE           0x000D
#define ERROR_WRONG_DISK            0x000F
#define ERROR_UNCERTAIN_MEDIA       0x0010
#define ERROR_CHAR_CALL_INTERRUPTED 0x0011
#define ERROR_NO_MONITOR_SUPPORT    0x0012
#define ERROR_INVALID_PARAMETER     0x0013
#define ERROR_DEVICE_IN_USE         0x0014
#define DISABLE _asm {cli}
#define ENABLE _asm {sti}

typedef struct AttachArea
{
  VOID (FAR *rEntryPoint)(SEG DS, PASPI_SRB_HEADER pSRB);
  SEG rDS;
  VOID (FAR *pEntryPoint)(SEL DS, PASPI_SRB_HEADER pSRB);
  SEL pDS;
} 
ATTACHAREA;
