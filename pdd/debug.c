/* debug.c
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1994 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Wed Mar 09 1994
 */
 
static char *rcsid =
"$Id: debug.c,v 1.1 1994/03/09 16:54:04 rommel Exp $";
static char *rcsrev = "$Revision: 1.1 $";

/* $Log: debug.c,v $
 * Revision 1.1  1994/03/09 16:54:04  rommel
 * Initial revision
 * */

#define	INCL_DOSDEVICES
#define	INCL_DOSDEVIOCTL
#include <os2.h>

#define ASPI_DEBUG 0x90

#define error(msg) \
{                  \
  printf(msg);     \
  return 1;        \
}

int main(int argc, char **argv)
{
  HFILE hASPI;
  USHORT nAction;

  if (argc != 2)
    error("Usage: debug [0 | 1 | 2]\n");

  if (DosOpen("VASPIDD$", &hASPI, &nAction, 0L, 0, FILE_OPEN,
	      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |
	      OPEN_FLAGS_FAIL_ON_ERROR, 0L)) 
    error("VASPIDD not found.\n");
  
  if ((nAction = atoi(argv[1])) > 2)
    error("Invalid mode.\n");

  if (DosDevIOCtl(0, 0, nAction, ASPI_DEBUG, hASPI))
    error("Cannot access VASPIDD.\n");
 
  return 0;
}

/* end of debug.c */
