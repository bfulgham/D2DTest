/*
 * Copyright (C) 2012 Brent Fulgham.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "stdafx.h"
#include "D2Dtest.h"

#include "CairoRoutines.h"
#include "CGRoutines.h"
#include "D2DRoutines.h"

#include <iostream>

#define MAX_LOADSTRING 100

HINSTANCE hInst;								// current instance
HWND g_hMainWnd;
HDC  g_hMainHDC;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
DWORD g_lastUpdate = 0;
int   g_frames = 0;

int g_Width = 400;
int g_Height = 400;

enum drawType
{
	e_Cairo,
	e_CoreGraphics,
	e_D2D
};

drawType g_DrawType = e_Cairo;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void SetClientSize(HWND hwnd, int clientWidth, int clientHeight);

LRESULT PaintCairoDemo (HDC hdc);
LRESULT PaintCGDemo (HWND hWnd, HDC hdc);

D2DRenderer* g_d2dRenderer = 0;
#if !defined(NO_CORE_GRAPHICS)
CGRenderer* g_cgRenderer = 0;
#endif
CairoRenderer* g_cairoRenderer = 0;
IRenderTest* g_currentTest = 0;

void render ()
{
   DWORD tickInterval = GetTickCount() - g_lastUpdate;
   float fps = (tickInterval) ? g_frames / (tickInterval / 1000.0f) : 0;

   if (tickInterval > 1000)
   {
      char message[100];
      int length = sprintf(message, "fps: %0.2g\n", fps);

      OutputDebugStringA(message);

      g_lastUpdate = GetTickCount();
      g_frames = 0;
   }

   g_currentTest->RenderDemo (g_hMainWnd, g_hMainHDC, g_Height, g_Width, fps);

   ++g_frames;
}

/**
  Changes the dimensions of a window's client area.
*/
void SetClientSize(HWND hwnd, int clientWidth, int clientHeight)
{
  if (!IsWindow (hwnd))
      return;

  DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
  DWORD dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
  HMENU menu = GetMenu(hwnd);
  RECT rc = { 0, 0, clientWidth, clientHeight } ;
  AdjustWindowRectEx( &rc, dwStyle, menu ? TRUE : FALSE, dwExStyle );
  SetWindowPos (hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                  SWP_NOZORDER | SWP_NOMOVE ) ;
}

int APIENTRY _tWinMain (HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPTSTR    lpCmdLine,
                        int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadString (hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString (hInstance, IDC_D2DTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass (hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators (hInstance, MAKEINTRESOURCE(IDC_D2DTEST));

	MSG msg;
	bool running = true;
	g_lastUpdate = ::GetTickCount();
	
	while (running)
	{
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}
		else
		{
			render();
			::SwapBuffers(g_hMainHDC);
			::Sleep(10);
		}
	}

	return static_cast<int>(msg.wParam);
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D2DTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_D2DTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   g_hMainWnd = ::CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_hMainWnd)
      return FALSE;

   ::SetWindowText (g_hMainWnd, L"D2DTest: Cairo");
   ::SetClientSize(g_hMainWnd, g_Height, g_Width);

   ::ShowWindow(g_hMainWnd, nCmdShow);
   ::UpdateWindow(g_hMainWnd);

   g_hMainHDC = ::GetDC(g_hMainWnd);

   PIXELFORMATDESCRIPTOR pfd;
   memset(&pfd, 0x00, sizeof(pfd));
   pfd.nSize = sizeof(pfd);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 24;
   pfd.cDepthBits = 16;
   pfd.iLayerType = PFD_MAIN_PLANE;
   int iFormat = ChoosePixelFormat(g_hMainHDC, &pfd);
   SetPixelFormat(g_hMainHDC, iFormat, &pfd);

   g_cairoRenderer = new CairoRenderer(g_hMainWnd, g_hMainHDC);
#if !defined(NO_CORE_GRAPHICS)
   g_cgRenderer = new CGRenderer(g_hMainWnd, g_hMainHDC);
#endif
   g_d2dRenderer = new D2DRenderer(g_hMainWnd, g_hMainHDC);
   g_currentTest = g_cairoRenderer;

   return TRUE;
}

static void SwitchDrawType (HWND hWnd, drawType changeToType)
{
   g_DrawType = changeToType;

	switch (changeToType)
	{
	case e_D2D:
		::SetWindowText (hWnd, L"D2DTest: Direct2D");
      g_currentTest = g_d2dRenderer;
		break;
#if !defined(NO_CORE_GRAPHICS)
	case e_CoreGraphics:
		::SetWindowText (hWnd, L"D2DTest: CoreGraphics");
      g_currentTest = g_cgRenderer;
		break;
#endif
	case e_Cairo:
	default:
		::SetWindowText (hWnd, L"D2DTest: Cairo");
      g_currentTest = g_cairoRenderer;
		break;
	}

   RECT rect;
   ::GetWindowRect (hWnd, &rect);
   ::InvalidateRect(hWnd, &rect, TRUE);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   int wmId, wmEvent;
   PAINTSTRUCT ps;
   HDC hdc;

   switch (message)
   {
   case WM_COMMAND:
      wmId    = LOWORD (wParam);
      wmEvent = HIWORD (wParam);
      // Parse the menu selections:
      switch (wmId)
      {
      case IDM_ABOUT:
         DialogBox (hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
         break;
      case IDM_EXIT:
         DestroyWindow (hWnd);
         break;
      case IDM_CAIRO:
         SwitchDrawType (hWnd, e_Cairo);
         break;
      case IDM_CG:
         SwitchDrawType (hWnd, e_CoreGraphics);
         break;
      case IDM_D2D:
         SwitchDrawType (hWnd, e_D2D);
         break;

      default:
         return DefWindowProc (hWnd, message, wParam, lParam);
      }
      break;
   case WM_PAINT:
      hdc = ::BeginPaint (hWnd, &ps);
      g_currentTest->RenderDemo(hWnd, hdc, g_Height, g_Width, 0.0f);
      ::EndPaint (hWnd, &ps);
      break;
   case WM_SIZE:
      {
         RECT rect;
         ::GetClientRect (hWnd, &rect);

         g_Height = rect.bottom;
         g_Width = rect.right;

         g_d2dRenderer->ResizeDemo(hWnd, rect);
#if !defined(NO_CORE_GRAPHICS)
         g_cgRenderer->ResizeDemo (hWnd, rect);
#endif
         g_cairoRenderer->ResizeDemo (hWnd, rect);

         ::InvalidateRect(hWnd, 0, FALSE);
         render();
      }
      break;
   case WM_SIZING:
      {
         // Try to enforce a square content region aspect ratio
         RECT rect;
         ::GetClientRect (hWnd, &rect);

         RECT& rc = *((LPRECT)lParam);

         LONG topDecor = (rc.bottom - rc.top) - (rect.bottom - rect.top);

         switch (wParam)
         {
            case WMSZ_LEFT:
            case WMSZ_RIGHT:
               rc.bottom = rc.top + (rc.right - rc.left) + topDecor;
               break;

            case WMSZ_TOP:
            case WMSZ_BOTTOM:
               rc.right = rc.left + (rc.bottom - rc.top);
               break;

            // otherwise, just follow the width changes:
            case WMSZ_BOTTOMLEFT:
            case WMSZ_TOPLEFT:
            case WMSZ_BOTTOMRIGHT:
            case WMSZ_TOPRIGHT:
            default:
               rc.bottom = rc.top + (rc.right - rc.left) + topDecor;
               break;
         }
         return DefWindowProc (hWnd, message, wParam, lParam);
      }
      break;
   case WM_DESTROY:
      PostQuitMessage (0);
      break;
   default:
      return DefWindowProc (hWnd, message, wParam, lParam);
   }
   return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   switch (message)
   {
   case WM_INITDIALOG:
      return (INT_PTR)TRUE;

   case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
      {
         EndDialog(hDlg, LOWORD(wParam));
         return (INT_PTR)TRUE;
      }
      break;
   }
   return (INT_PTR)FALSE;
}
