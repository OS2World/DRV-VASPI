/* stamp.c
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1993 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Wed Dec 22 1993
 */
 
static char *rcsid =
"$Id: stamp.c,v 1.2 1994/02/08 17:29:00 rommel Exp $";
static char *rcsrev = "$Revision: 1.2 $";

/* $Log: stamp.c,v $
 * Revision 1.2  1994/02/08 17:29:00  rommel
 * fixed include files (prototype for strtol)
 *
 * Revision 1.1  1994/02/06 21:56:47  rommel
 * Initial revision
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>

#define INCL_NOPMAPI
#include <os2.h>

char marker[] = "SN 00000000";
char key[]    = "VASPI";

extern Encode(void *buffer, int bytes, char *password);

char driver[16384];

struct 
{ 
  LONG Num; 
  LONG Key; 
} 
Serial = {0, 0};

int main(int argc, char **argv)
{
  int file, bytes;
  struct stat sb;
  struct utimbuf ub;
  long number;
  char *ptr;

  if (argc != 2 && argc != 3)
  {
    printf("\nUsage: stamp <driver-file> [<serial-number>]\n");
    return 1;
  }

  if (stat(argv[1], &sb) == -1 ||
      (file = open(argv[1], O_RDWR | O_BINARY)) == -1 ||
      (bytes = read(file, driver, sizeof(driver))) == -1 ||
      lseek(file, 0, SEEK_SET) != 0)
  {
    perror(argv[1]);
    return 1;
  }

  ub.actime  = sb.st_atime;
  ub.modtime = sb.st_mtime;

  for (ptr = driver; memcmp(ptr, marker, sizeof(marker)) != 0; ptr++)
    if (ptr - driver > bytes)
    {
      printf("%s: marker not found\n", argv[1]);
      return 1;
    }

  ptr += sizeof(marker);

  if (argc == 2)
  {
    memcpy(&Serial, ptr, sizeof(Serial));
    Encode(&Serial, sizeof(Serial), key);
    printf("%s: serial number '%lX'\n", argv[1], Serial.Num);
  }
  else
  {
    number = strtol(argv[2], NULL, 16);
    Serial.Num = number;
    Serial.Key = -number;
    Encode(&Serial, sizeof(Serial), key);
    memcpy(ptr, &Serial, sizeof(Serial));

    if (write(file, driver, bytes) != bytes ||
	close(file) == -1 ||
	utime(argv[1], &ub) == -1)
    {
      perror(argv[1]);
      return 1;
    }
  }

  return 0;
}

/* end of stamp.c */
