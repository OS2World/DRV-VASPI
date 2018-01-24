/* vaspi.h
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1994 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Mon Dec 20 1993
 */
 
/* $Id: vaspi.h,v 1.11 1994/09/14 12:15:28 rommel Exp $ */
/* $Revision: 1.11 $ */

/*
 * $Log: vaspi.h,v $
 * Revision 1.11  1994/09/14 12:15:28  rommel
 * new init method, allows real stub for specific DOS sessions
 *
 * Revision 1.10  1994/05/26 12:48:32  rommel
 * fix for NetWare requester
 *
 * Revision 1.9  1994/04/20 11:53:35  rommel
 * fourth release
 *
 * Revision 1.8  1994/03/29 16:08:40  rommel
 * third release
 *
 * Revision 1.7  1994/03/11 17:25:59  rommel
 * changes for VDHPushInt method
 *
 * Revision 1.6  1994/03/09 17:29:37  rommel
 * new version using VDHPushFarCall
 *
 * Revision 1.5  1994/02/02 13:22:39  rommel
 * beta release
 *
 * Revision 1.4  1994/01/09 20:19:53  rommel
 * first somewhat working version
 *
 * Revision 1.3  1993/12/22 18:21:48  rommel
 * Added interface to physical device.
 *
 * Revision 1.2  1993/12/22  13:06:21  rommel
 * Restructured, stub DD added.
 *
 * Revision 1.1  1993/12/20  16:16:41  rommel
 * Initial revision
 */

#ifndef _VASPI_H
#define _VASPI_H

#define EXPIRE          15

#define PDDNAME         "VASPIDD$"
#define PROPERTYNAME    "ASPI_AVAILABLE"

#define ASPI_REQ        0x80
#define ASPI_VDDREQ     0x81
#define ASPI_DEBUG      0x90
#define ASPI_DEMO       0x91

#define MPLEX           0x2F
#define MPLEX_POST      0xC701
#define MPLEX_ASPI      0xC702
#define INIT_INT        0x5A

#define FALSE  0
#define TRUE   1

extern _System _printf(char *, ...);
#define printf _printf

/* main.c */

extern HVDM owner_VDM;
extern HHOOK hBPHook;
extern HHOOK hTimerHook;
extern HHOOK hRetHook;
extern VPDOSDDTYPE DosDDHeader;
extern HFILE hASPI;
extern BOOL bDemoVersion;
extern BOOL bASPIAvailable;
extern PCHAR pBuffer;

/* aspi.c */

extern BOOL HOOKENTRY VirtualASPI(PVOID pHookData, PCRF pcrf);
extern BOOL HOOKENTRY PostTimer(PVOID pHookData, PCRF pcrf);
extern BOOL HOOKENTRY PostReturn(PVOID pHookData, PCRF pcrf);

#endif /* _VASPI_H */

/* end of vaspi.h */
