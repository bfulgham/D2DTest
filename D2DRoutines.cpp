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

#include "D2DRoutines.h"

#include <d2d1.h>
#include <d2d1helper.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <crtdbg.h>

#include <strsafe.h>

D2DRenderer::D2DRenderer (HWND hWnd, HDC hdc) : m_pDirect2dFactory(0), m_pRenderTarget(0),
	m_pGreenBrush(0), m_pWhiteBrush(0), m_pBlackBrush(0), m_pGreyBrush(0),
	m_pBlueBrush(0), m_pRoundCapStyle(0), m_pDirectWriteFactory(0), m_pTextFormat(0)
{
   InitDemo (hWnd, hdc);
}

template <class T> void SafeRelease(T **ppT)
{
   if (*ppT)
   {
      (*ppT)->Release();
      *ppT = NULL;
   }
}

D2DRenderer::~D2DRenderer ()
{
   SafeRelease(&m_pDirect2dFactory);
   SafeRelease(&m_pDirectWriteFactory);
   SafeRelease(&m_pRenderTarget);
   SafeRelease(&m_pGreenBrush);
   SafeRelease(&m_pWhiteBrush);
   SafeRelease(&m_pBlackBrush);
   SafeRelease(&m_pGreyBrush);
   SafeRelease(&m_pBlueBrush);
   SafeRelease(&m_pRoundCapStyle);
   SafeRelease(&m_pTextFormat);
}

void D2DRenderer::InitDemo (HWND hWnd, HDC hdc)
{
   HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
   _ASSERT (SUCCEEDED(hr));
   if (!SUCCEEDED(hr))
      return;

   hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDirectWriteFactory),
                            reinterpret_cast<IUnknown **>(&m_pDirectWriteFactory));
   if (!SUCCEEDED(hr))
      return;

   RECT rect;
   ::GetClientRect (hWnd, &rect);

   D2D1_SIZE_U size = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);

   hr = m_pDirect2dFactory->CreateHwndRenderTarget (D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties (hWnd, size), &m_pRenderTarget);
   if (!SUCCEEDED(hr))
      return;

   hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.337f, 0.612f, 0.117f, 0.9f), &m_pGreenBrush);
   if (!SUCCEEDED(hr))
      return;

   hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.8f), &m_pWhiteBrush);
   if (!SUCCEEDED(hr))
      return;

   hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &m_pBlackBrush);
   if (!SUCCEEDED(hr))
      return;

   hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.7f, 0.7f, 0.7f, 0.8f), &m_pGreyBrush);
   if (!SUCCEEDED(hr))
      return;

   hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.117f, 0.337f, 0.612f, 0.9f), &m_pBlueBrush);
   if (!SUCCEEDED(hr))
      return;

   hr = m_pDirect2dFactory->CreateStrokeStyle (D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND), 0, 0, &m_pRoundCapStyle);
   if (!SUCCEEDED(hr))
      return;

   hr = m_pDirectWriteFactory->CreateTextFormat(L"Verdana", 0, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                                                DWRITE_FONT_STRETCH_NORMAL, 11.0, L"", &m_pTextFormat);
}

void D2DRenderer::RenderDemo (HWND hWnd, HDC hdc, int height, int width, float fps)
{
   _ASSERT (m_pRenderTarget);
   if (!m_pRenderTarget)
      return;

   HRESULT hr = S_OK;

   m_pRenderTarget->BeginDraw();

   // Reset to identity
   m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

   float m_radius = 0.42f;
   float m_line_width = 0.05f;

   const D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(width, height);

   m_pRenderTarget->SetTransform(scale);

   D2D1::Matrix3x2F curr;
   m_pRenderTarget->GetTransform (&curr);

   const D2D1::Matrix3x2F trans = D2D1::Matrix3x2F::Translation(0.5f * width, 0.5f * height);
   m_pRenderTarget->SetTransform(curr * trans);

   D2D1_RECT_F rect = D2D1::RectF(-0.5f, -0.5f, 0.5f, 0.5f);
   m_pRenderTarget->FillRectangle(rect, m_pGreenBrush);

   D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(0.0f, 0.0f), m_radius, m_radius);
   m_pRenderTarget->FillEllipse(ellipse, m_pWhiteBrush);
   m_pRenderTarget->DrawEllipse(ellipse, m_pBlackBrush, m_line_width);

   //clock ticks
   for (int i = 0; i < 12; ++i)
   {
      float inset = 0.05f;
      float strokeWidth = m_line_width;

      if (i % 3 != 0)
      {
         inset *= 0.8f;
         strokeWidth = 0.03f;
      }

      const float angle = static_cast<float>(i * M_PI / 6.0);
      const float sinAngle = sinf(angle);
      const float cosAngle = cosf(angle);

      D2D_POINT_2F startPoint = D2D1::Point2F((m_radius - inset) * cosAngle, (m_radius - inset) * sinAngle);
      D2D_POINT_2F endPoint = D2D1::Point2F(m_radius * cosAngle, m_radius * sinAngle);

      m_pRenderTarget->DrawLine (startPoint, endPoint, m_pBlackBrush, strokeWidth, m_pRoundCapStyle);
   }

   // store the current time
   SYSTEMTIME time;
   GetLocalTime(&time);

   // compute the angles of the indicators of our clock
   double minutes = time.wMinute * M_PI / 30;
   double hours = time.wHour * M_PI / 6;
   double seconds= ((double)time.wSecond + (double)time.wMilliseconds / 1000) * M_PI / 30;

   // draw the seconds hand
   float secondHandLength = 0.9f * m_radius;
   float sinSec = sinf(seconds);
   float cosSec = -cosf(seconds);
   D2D_POINT_2F endSecondsPoint = D2D1::Point2F (sinSec * secondHandLength, cosSec * secondHandLength);

   m_pRenderTarget->DrawLine (ellipse.point, endSecondsPoint, m_pGreyBrush, m_line_width / 3, m_pRoundCapStyle);

   // draw the minutes hand
   float minuteHandLength = 0.8f * m_radius;
   float sinMin = sinf(minutes + seconds/60);
   float cosMin = -cosf(minutes + seconds/60);
   D2D_POINT_2F endMinutesPoint = D2D1::Point2F (sinMin * minuteHandLength, cosMin * minuteHandLength);

   m_pRenderTarget->DrawLine (ellipse.point, endMinutesPoint, m_pBlueBrush, m_line_width, m_pRoundCapStyle);

   // draw the hours hand
   float hourHandLength = 0.5f * m_radius;
   float sinHours = sinf(hours + minutes / 12.0f);
   float cosHours = -cosf(hours + minutes / 12.0f);
   D2D_POINT_2F endHoursPoint = D2D1::Point2F (sinHours * hourHandLength, cosHours * hourHandLength);

   m_pRenderTarget->DrawLine (ellipse.point, endHoursPoint, m_pGreenBrush, m_line_width, m_pRoundCapStyle);

   // draw a little dot in the middle
   D2D1_ELLIPSE dot = D2D1::Ellipse(D2D1::Point2F(0.0f, 0.0f), m_line_width / 3.0f, m_line_width / 3.0f);
   m_pRenderTarget->FillEllipse(dot, m_pBlackBrush);

   // Display FPS:
   m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
   D2D1_SIZE_F renderTargetSize = m_pRenderTarget->GetSize();
   wchar_t message[100];
   int length = swprintf(message, 100, L"fps: %0.2g", fps);
   m_pRenderTarget->DrawText(message, length, m_pTextFormat,
                             D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
                             m_pBlackBrush);

   hr = m_pRenderTarget->EndDraw();

   //if (hr != D2DERR_RECREATE_TARGET)
   //   printf("render failed with %s\n", cairo_status_to_string(cairo_status(g_cr)));
}

void D2DRenderer::ResizeDemo(HWND hWnd, const RECT& rect)
{
	D2D1_SIZE_U size = D2D1::SizeU(rect.right, rect.bottom);
	m_pRenderTarget->Resize(size);
}

