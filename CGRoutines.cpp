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

#include "CGRoutines.h"

#include "DIBPixelData.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <crtdbg.h>

CGRenderer::CGRenderer(HWND hWnd, HDC hdc) : m_bitmapDC(0), m_bitmapData(0),
	m_bitmap(0), m_oldBitmap(0), m_cr(0), m_messageFont(0)
{
   memset (&m_bmpInfo, 0x00, sizeof (m_bmpInfo));
   InitDemo(hWnd, hdc);
}

CGRenderer::~CGRenderer()
{
   CGContextRelease(m_cr);
   CGFontRelease(m_messageFont);

   // Clean up old resoures
   ::SelectObject(m_bitmapDC, m_oldBitmap);
   ::DeleteObject(m_bitmap);
   ::DeleteDC(m_bitmapDC);
}

static CGContextRef CGContextWithHDC (HDC hdc, bool hasAlpha)
{
   HBITMAP bitmap = static_cast<HBITMAP>(GetCurrentObject (hdc, OBJ_BITMAP));

   DIBPixelData pixelData (bitmap);

   // FIXME: We can get here because we asked for a bitmap that is too big
   // when we have a tiled layer and we're compositing. In that case 
   // bmBitsPixel will be 0. This seems to be benign, so for now we will
   // exit gracefully and look at it later:
   //  https://bugs.webkit.org/show_bug.cgi?id=52041   
   _ASSERT(pixelData.bitsPerPixel() == 32);
   if (pixelData.bitsPerPixel() != 32)
      return 0;

   CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();

   CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | (hasAlpha ? kCGImageAlphaPremultipliedFirst : kCGImageAlphaNoneSkipFirst);
   CGContextRef context = CGBitmapContextCreate(pixelData.buffer(), pixelData.size ().cx, pixelData.size ().cy, 8,
                                                pixelData.bytesPerRow(), rgb, bitmapInfo);
   CGColorSpaceRelease(rgb);

   // Flip coords
   CGContextTranslateCTM(context, 0, pixelData.size().cy);
   CGContextScaleCTM(context, 1, -1);
 
   // Put the HDC In advanced mode so it will honor affine transforms.
   SetGraphicsMode(hdc, GM_ADVANCED);
    
   return context;
}

void CGRenderer::InitDemo(HWND hWnd, HDC hdc)
{
   RECT rect;
   ::GetClientRect(hWnd, &rect);

   m_bitmapDC = ::CreateCompatibleDC(hdc);

   memset (&m_bmpInfo, 0x00, sizeof (m_bmpInfo));
   m_bmpInfo.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
   m_bmpInfo.bmiHeader.biWidth = rect.right - rect.left;
   m_bmpInfo.bmiHeader.biHeight = rect.top - rect.bottom;
   m_bmpInfo.bmiHeader.biPlanes = 1;
   m_bmpInfo.bmiHeader.biBitCount = 32;
   m_bmpInfo.bmiHeader.biCompression = BI_RGB;

   m_bitmap = ::CreateDIBSection(m_bitmapDC, &m_bmpInfo, DIB_RGB_COLORS, &m_bitmapData, 0, 0);

   m_oldBitmap = (HBITMAP)SelectObject (m_bitmapDC, m_bitmap);

   m_cr = CGContextWithHDC (m_bitmapDC, true);

   memset(&m_windowsFont, 0x00, sizeof(m_windowsFont));
   ::GetObject (GetStockObject(DEFAULT_GUI_FONT), sizeof(m_windowsFont), &m_windowsFont);

   HFONT hfont = CreateFontIndirect(&m_windowsFont);

   m_messageFont = CGFontCreateWithPlatformFont(&m_windowsFont);
}


/**
  Gradient demonstration.
  Based on example from http://cairographics.org/samples/
*/
void gradientExample (CGContextRef cr)
{
   CGFloat colors[] =
   {
      1.0, 1.0, 1.0, 1.0,
      0.0, 0.0, 0.0, 1.0,
   };

   CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();

   CGGradientRef gradient = CGGradientCreateWithColorComponents(rgb, colors, 0, sizeof(colors)/(sizeof(colors[0])*4));

   CGColorSpaceRelease(rgb);

   CGPoint start = CGPointMake(0, 0);
   CGPoint end = CGPointMake(0, 256);

   CGContextDrawLinearGradient(cr, gradient, start, end, kCGGradientDrawsBeforeStartLocation);

   CGPoint startCenter = CGPointMake(112, 112);
   CGPoint endCenter = CGPointMake(128, 128);
   CGContextDrawRadialGradient(cr, gradient, startCenter, 20.0f, endCenter, 76.8f, kCGGradientDrawsBeforeStartLocation);
   CGGradientRelease(gradient);

   CGContextFlush (cr);
}

/**
  Handles WM_PAINT.
*/
LRESULT PaintCGDemo (HWND hWnd, HDC hdc)
{
   RECT rect;
   ::GetClientRect(hWnd, &rect);

   HDC bitmapDC = ::CreateCompatibleDC(hdc);

   BITMAPINFO bmpInfo;
   memset (&bmpInfo, 0x00, sizeof (bmpInfo));
   bmpInfo.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
   bmpInfo.bmiHeader.biWidth = rect.right - rect.left;
   bmpInfo.bmiHeader.biHeight = rect.top - rect.bottom;
   bmpInfo.bmiHeader.biPlanes = 1;
   bmpInfo.bmiHeader.biBitCount = 32;
   bmpInfo.bmiHeader.biCompression = BI_RGB;

   void* bitmapData = 0;
   HBITMAP bitmap = ::CreateDIBSection (bitmapDC, &bmpInfo, DIB_RGB_COLORS, &bitmapData, 0, 0);

   HBITMAP hOldBmp = (HBITMAP)SelectObject (bitmapDC, bitmap);

   CGContextRef cr = CGContextWithHDC (bitmapDC, true);

   // Draw on the cairo context.
   gradientExample (cr);

   //DIBPixelData pixelData (bitmap);
   //pixelData.writeToFile (L"E:\\output.bmp");

   ::BitBlt(hdc, 0, 0, bmpInfo.bmiHeader.biWidth, -bmpInfo.bmiHeader.biHeight, bitmapDC, 0, 0, SRCCOPY);

   CGContextRelease (cr);

   ::SelectObject(bitmapDC, hOldBmp);
   ::DeleteObject(bitmap);
   ::DeleteDC(bitmapDC);
   ::ReleaseDC(hWnd, bitmapDC);

   return 0;
}

void CGRenderer::RenderDemo (HWND hWnd, HDC hdc, int height, int width, float fps)
{
   // Reset to identity
   CGAffineTransform ctm = CGContextGetCTM(m_cr);
   CGAffineTransform inverted = CGAffineTransformInvert(ctm);
   CGContextConcatCTM(m_cr, inverted);

   CGContextSaveGState(m_cr);

   float m_radius = 0.42f;
   float m_line_width = 0.05f;

   CGContextScaleCTM(m_cr, width, height);
   CGContextTranslateCTM(m_cr, 0.5f, 0.5f);
   CGContextSetLineWidth(m_cr, m_line_width);

   // Background
   CGContextSaveGState(m_cr);
   CGContextSetRGBFillColor(m_cr, 0.337f, 0.612f, 0.117f, 0.9f);   // green
   CGRect bounds = CGRectMake(-0.5f, -0.5f, 1.0f, 1.0f);
   CGContextFillRect(m_cr, bounds);
   CGContextRestoreGState(m_cr);

   // Clock face
   CGContextAddArc(m_cr, 0.0f, 0.0f, m_radius, 0.0f, 2.0f * M_PI, 1);
   CGContextSaveGState(m_cr);
   CGContextSetRGBFillColor(m_cr, 1.0f, 1.0f, 1.0f, 0.8f);
   CGContextFillPath(m_cr);
   CGContextRestoreGState(m_cr);
   CGContextSetRGBStrokeColor(m_cr, 0.0f, 0.0f, 0.0f, 1.0f);
   CGContextAddArc(m_cr, 0.0f, 0.0f, m_radius, 0.0f, 2.0f * M_PI, 1);
   CGContextStrokePath(m_cr);

   // clock ticks
   for (int i = 0; i < 12; ++i)
   {
      float inset = 0.05f;

      CGContextSaveGState(m_cr);
      CGContextSetLineCap(m_cr, kCGLineCapRound);

      if (i % 3 != 0)
      {
         inset *= 0.8;
         CGContextSetLineWidth(m_cr, 0.03f);
      }

      const float angle = static_cast<float>(i * M_PI / 6.0);
      const float sinAngle = sinf(angle);
      const float cosAngle = cosf(angle);

      CGContextMoveToPoint(m_cr, (m_radius - inset) * cosAngle, (m_radius - inset) * sinAngle);
      CGContextAddLineToPoint(m_cr, m_radius * cosAngle, m_radius * sinAngle);
      CGContextStrokePath(m_cr);
      CGContextRestoreGState(m_cr); // stack-pen-size
   }

   // store the current time
   SYSTEMTIME time;
   GetLocalTime(&time);

   // compute the angles of the indicators of our clock
   double minutes = time.wMinute * M_PI / 30;
   double hours = time.wHour * M_PI / 6;
   double seconds= ((double)time.wSecond + (double)time.wMilliseconds / 1000) * M_PI / 30;

   CGContextSaveGState(m_cr);
   CGContextSetLineCap(m_cr, kCGLineCapRound);

   // draw the seconds hand
   CGContextSaveGState(m_cr);
   CGContextSetLineWidth(m_cr, m_line_width / 3);
   CGContextSetRGBStrokeColor(m_cr, 0.7f, 0.7f, 0.7f, 0.8f); // gray
   CGContextMoveToPoint(m_cr, 0.0f, 0.0f);
	float secondHandLength = 0.9f * m_radius;
	float sinSec = sinf(seconds);
   float cosSec = cosf(seconds);
   CGContextAddLineToPoint(m_cr, sinSec * secondHandLength, cosSec * secondHandLength);
   CGContextStrokePath(m_cr);
   CGContextRestoreGState(m_cr);

   // draw the minutes hand
   CGContextSetRGBStrokeColor(m_cr, 0.117f, 0.337f, 0.612f, 0.9f);   // blue
   CGContextMoveToPoint(m_cr, 0.0f, 0.0f);
	float minuteHandLength = 0.8f * m_radius;
	float sinMin = sinf(minutes + seconds/60);
	float cosMin = cosf(minutes + seconds/60);
   CGContextAddLineToPoint(m_cr, sinMin * minuteHandLength, cosMin * minuteHandLength);
   CGContextStrokePath(m_cr);

   // draw the hours hand
   CGContextSetRGBStrokeColor(m_cr, 0.337f, 0.612f, 0.117f, 0.9f);   // green
   CGContextMoveToPoint(m_cr, 0.0f, 0.0f);
	float hourHandLength = 0.5f * m_radius;
	float sinHours = sinf(hours + minutes / 12.0f);
	float cosHours = cosf(hours + minutes / 12.0f);
   CGContextAddLineToPoint(m_cr, sinHours * hourHandLength, cosHours * hourHandLength);
   CGContextStrokePath(m_cr);
   CGContextRestoreGState(m_cr);

   // draw a little dot in the middle
   CGContextAddArc(m_cr, 0, 0, m_line_width / 3.0, 0.0f, 2.0f * M_PI, 1);
   CGContextSetRGBFillColor(m_cr, 0.0f, 0.0f, 0.0f, 1.0f);
   CGContextFillPath(m_cr);

   CGContextRestoreGState(m_cr);

   // Display FPS:
   char message[100];
   int length = sprintf(message, "fps: %0.2g", fps);

   // Attempt to display the text:
   CGContextSetFont(m_cr, m_messageFont);

   CGContextSetCharacterSpacing (m_cr, 10);
   CGContextSetTextDrawingMode(m_cr, kCGTextStroke);
 
   CGContextShowTextAtPoint(m_cr, 10, height - 10, "Test", 9);

   CGContextSetTextPosition(m_cr, 10, height - 10);
   CGContextShowText(m_cr, "Test", 4);

   /*
    * I could not get text rendering through CoreGraphics to work under Windows
    * Draw a circle where we want the text to display:
    */
   CGContextAddArc(m_cr, 10, height - 10, 3.0, 0.0f, 2.0f * M_PI, 1);
   CGContextSetRGBStrokeColor(m_cr, 0.0f, 0.0f, 0.0f, 1.0f);
   CGContextSetRGBFillColor(m_cr, 0.0f, 0.0f, 0.0f, 1.0f);
   CGContextFillPath(m_cr);

   CGContextFlush(m_cr);

   ::BitBlt(hdc, 0, 0, m_bmpInfo.bmiHeader.biWidth, -m_bmpInfo.bmiHeader.biHeight, m_bitmapDC, 0, 0, SRCCOPY);

   /*
   RECT rect;
   rect.bottom = 0;
   rect.top = 10;
   rect.left = 0;
   rect.right = 100;
   ::DrawTextA(hdc, message, length, &rect, DT_VCENTER | DT_SINGLELINE | DT_CENTER); 
   */
}

void CGRenderer::ResizeDemo(HWND hWnd, const RECT& rect)
{
	HDC hdc = ::GetDC(hWnd);

	CGContextRelease (m_cr);

	// Clean up old resoures
	::SelectObject (m_bitmapDC, m_oldBitmap);
	::DeleteObject (m_bitmap);
	::DeleteDC (m_bitmapDC);
	::ReleaseDC (hWnd, m_bitmapDC);

	m_bitmapDC = ::CreateCompatibleDC(hdc);

	memset (&m_bmpInfo, 0x00, sizeof (m_bmpInfo));
	m_bmpInfo.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	m_bmpInfo.bmiHeader.biWidth = rect.right - rect.left;
	m_bmpInfo.bmiHeader.biHeight = rect.top - rect.bottom;
	m_bmpInfo.bmiHeader.biPlanes = 1;
	m_bmpInfo.bmiHeader.biBitCount = 32;
	m_bmpInfo.bmiHeader.biCompression = BI_RGB;

	m_bitmap = ::CreateDIBSection(m_bitmapDC, &m_bmpInfo, DIB_RGB_COLORS, &m_bitmapData, 0, 0);

	m_oldBitmap = (HBITMAP)SelectObject (m_bitmapDC, m_bitmap);

	m_cr = CGContextWithHDC (m_bitmapDC, true);

	::ReleaseDC(hWnd, hdc);
}
