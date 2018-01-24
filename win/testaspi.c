/* testaspi.c
 *
 * virtual ASPI driver for OS/2 2.x
 * (C) 1993 ARS Computer und Consulting GmbH
 *
 * Author:  Kai Uwe Rommel <rommel@jonas>
 * Created: Wed Mar 02 1994
 */

static char *rcsid =
"$Id: testaspi.c,v 1.1 1994/03/09 15:52:48 rommel Exp $";
static char *rcsrev = "$Revision: 1.1 $";

#include <windows.h>
#include "winaspi.h"
 
HANDLE hInst;
char szAppName[] = "TestASPI";
char szTitle[]   = "Test WINASPI";


LONG FAR PASCAL WindowProc(HWND hWnd, unsigned iMessage, WORD wParam, LONG lParam)
{
  WORD nResult;
 
  switch ( iMessage )
  {

  case WM_CREATE:
    DebugBreak();
    nResult = GetASPISupportInfo();
    break;
 
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hWnd, iMessage, wParam, lParam);

  }
 
  return 0L;
}


int WinInit(HANDLE hInstance)
{
  WNDCLASS WndClass;

  WndClass.style         = NULL;
  WndClass.lpfnWndProc   = WindowProc;
  WndClass.hInstance     = hInstance;
  WndClass.cbClsExtra    = 0;
  WndClass.cbWndExtra    = 0;
  WndClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  WndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
  WndClass.lpszMenuName  = NULL;
  WndClass.lpszClassName = szAppName;

  return RegisterClass(&WndClass);
}
 
 
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
  HWND hWnd;
  MSG msg;
 
  if ( ! hPrevInstance )
    if ( ! WinInit(hInstance) )
      return FALSE;
 
  hInst = hInstance;
 
  hWnd = CreateWindow(szAppName, szTitle,
                      WS_OVERLAPPEDWINDOW,
                      CW_USEDEFAULT, CW_USEDEFAULT,
                      CW_USEDEFAULT, CW_USEDEFAULT,
                      NULL, NULL, hInstance, NULL);
 
  if ( ! hWnd )
    return FALSE;
 
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
 
  while ( GetMessage(&msg, NULL, NULL, NULL) )
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
 
  return msg.wParam;
}

/* end of testaspi.c */
