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

#include "CairoRoutines.h"

#include <cairo/cairo.h>
#include <cairo/cairo-win32.h>

#include <crtdbg.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <cstdio>

CairoRenderer::CairoRenderer(HWND hWnd, HDC hdc) : m_surface(0), m_cr(0), m_hdc(0)
{
   InitDemo(hWnd, hdc);
}

CairoRenderer::~CairoRenderer()
{
   cairo_destroy(m_cr);
   cairo_surface_destroy(m_surface);
}

void CairoRenderer::InitDemo(HWND hWnd, HDC hdc)
{
   m_hdc = hdc;
   m_surface = cairo_win32_surface_create (hdc);
   m_cr = cairo_create (m_surface);
}

/**
  Gradient demonstration.
  Taken from http://cairographics.org/samples/
*/
static void gradientExample(cairo_t* cr)
{
   cairo_pattern_t* pat = cairo_pattern_create_linear (0.0, 0.0,  0.0, 256.0);
   cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
   cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
   cairo_rectangle (cr, 0, 0, 256, 256);
   cairo_set_source (cr, pat);
   cairo_fill (cr);
   cairo_pattern_destroy (pat);

   pat = cairo_pattern_create_radial (115.2, 102.4, 25.6,
                                     102.4,  102.4, 128.0);
   cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
   cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
   cairo_set_source (cr, pat);
   cairo_arc (cr, 128.0, 128.0, 76.8, 0, 2 * M_PI);
   cairo_fill (cr);
   cairo_pattern_destroy (pat);
}

/**
  Handles WM_PAINT.
*/
LRESULT PaintCairoDemo(HDC hdc)
{
   // Create the cairo surface and context.
   cairo_surface_t* surface = cairo_win32_surface_create (hdc);
   cairo_t* cr = cairo_create (surface);
  
   // Draw on the cairo context.
   gradientExample (cr);

   // Cleanup.
   cairo_destroy (cr);
   cairo_surface_destroy (surface);

   return 0;
}

void CairoRenderer::RenderDemo(HWND hWnd, HDC hdc, int height, int width, float fps)
{
   cairo_identity_matrix(m_cr);

   cairo_save(m_cr);

   double m_radius = 0.42;
   double m_line_width = 0.05;

   cairo_scale(m_cr, width, height);
   cairo_translate(m_cr, 0.5, 0.5);

   cairo_set_line_width(m_cr, m_line_width);

   // Background
   cairo_save(m_cr);
   cairo_set_source_rgba(m_cr, 0.337, 0.612, 0.117, 0.9);   // green
   cairo_paint(m_cr);
   cairo_restore(m_cr);

   // Clock face:
   cairo_save(m_cr);
   cairo_new_sub_path(m_cr);
   cairo_arc(m_cr, 0, 0, m_radius, 0, 2 * M_PI);
   cairo_save(m_cr);
   cairo_set_source_rgba(m_cr, 1.0, 1.0, 1.0, 0.8);
   cairo_fill_preserve(m_cr);
   cairo_restore(m_cr);
   cairo_close_path(m_cr);
   cairo_stroke(m_cr);
   cairo_restore(m_cr);

   // clock ticks
   for (int i = 0; i < 12; ++i)
   {
      double inset = 0.05;

      cairo_save(m_cr);
      cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_ROUND);

      if (i % 3 != 0)
      {
         inset *= 0.8;
         cairo_set_line_width(m_cr, 0.03);
      }

      const double angle = i * M_PI / 6.0f;
      const double sinAngle = std::sin(angle);
      const double cosAngle = std::cos(angle);

      cairo_move_to(m_cr, (m_radius - inset) * cosAngle, (m_radius - inset) * sinAngle);
      cairo_line_to (m_cr, m_radius * cosAngle, m_radius * sinAngle);
      cairo_stroke(m_cr);
      cairo_restore(m_cr); // stack-pen-size
   }

   // store the current time
   SYSTEMTIME time;
   GetLocalTime(&time);

   // compute the angles of the indicators of our clock
   double minutes = time.wMinute * M_PI / 30;
   double hours = time.wHour * M_PI / 6;
   double seconds= ((double)time.wSecond + (double)time.wMilliseconds / 1000) * M_PI / 30;

   cairo_save(m_cr);
   cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_ROUND);

   // draw the seconds hand
   cairo_save(m_cr);
   cairo_set_line_width(m_cr, m_line_width / 3);
   cairo_set_source_rgba(m_cr, 0.7, 0.7, 0.7, 0.8); // gray
   cairo_move_to(m_cr, 0, 0);
	double secondHandLength = 0.9 * m_radius;
	double sinSec = std::sin(seconds);
	double cosSec = std::cos(seconds);
   cairo_line_to(m_cr, sinSec * secondHandLength, -cosSec * secondHandLength);
   cairo_stroke(m_cr);
   cairo_restore(m_cr);

   // draw the minutes hand
   cairo_set_source_rgba(m_cr, 0.117, 0.337, 0.612, 0.9);   // blue
   cairo_move_to(m_cr, 0, 0);
	double minuteHandLength = 0.8 * m_radius;
	double sinMin = std::sin(minutes + seconds/60);
	double cosMin = std::cos(minutes + seconds/60);
   cairo_line_to(m_cr, sinMin * minuteHandLength, -cosMin * minuteHandLength);
   cairo_stroke(m_cr);

   // draw the hours hand
   cairo_set_source_rgba(m_cr, 0.337, 0.612, 0.117, 0.9);   // green
   cairo_move_to(m_cr, 0, 0);
	double hourHandLength = 0.5 * m_radius;
	double sinHours = std::sin(hours + minutes / 12.0);
	double cosHours = std::cos(hours + minutes / 12.0);
   cairo_line_to(m_cr, sinHours * hourHandLength, -cosHours * hourHandLength);
   cairo_stroke(m_cr);
   cairo_restore(m_cr);

   // draw a little dot in the middle
   cairo_arc(m_cr, 0, 0, m_line_width / 3.0, 0, 2 * M_PI);
   cairo_fill(m_cr);
   cairo_stroke(m_cr);

   cairo_restore(m_cr);

   // Display FPS:
   cairo_select_font_face(m_cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
   cairo_set_font_size(m_cr, 11.0);
   cairo_move_to(m_cr, 0, 10.0);

   char message[100];
   sprintf(message, "fps: %0.2g", fps);
   cairo_show_text(m_cr, message);

   cairo_surface_flush(m_surface);

   if (cairo_status(m_cr) != CAIRO_STATUS_SUCCESS)
      printf("render failed with %s\n", cairo_status_to_string(cairo_status(m_cr)));
}

void CairoRenderer::ResizeDemo(HWND hWnd, const RECT& rect)
{
   ::ReleaseDC(hWnd, m_hdc);

   m_hdc = ::GetDC(hWnd);

   cairo_destroy(m_cr);
   cairo_surface_destroy(m_surface);

   m_surface = cairo_win32_surface_create(m_hdc);
   m_cr = cairo_create(m_surface);
}
