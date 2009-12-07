/*
** Peteris Krumins (peter@catonmat.net)
** http://www.catonmat.net  --  good coders code, great reuse
**
** A Winamp plugin for reporting what music you are listening to
** Digital Point Forums. It was written in 2006.
**
** Latest version and explanation of how it was written is always at:
** http://www.catonmat.net/projects/dpf-winamp-music-reporter
*/

#include <windows.h>
#include <windowsx.h>

#include "resource.h"
#include "logo.h"

HBITMAP hbmImage;
HBITMAP hbmMask;

void
init_logo(HWND hwnd)
{
    BITMAP bm;
    HDC hdcSrc;
    HDC hdcDst;
    HBITMAP hbmSrcT;
    HBITMAP hbmDstT;
    COLORREF clrTopLeft;
    COLORREF clrSaveBk;
    COLORREF clrSaveDstText;

    HINSTANCE hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    hbmImage = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DP));

    GetObject(hbmImage, sizeof(bm), &bm);
    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

    hdcSrc = CreateCompatibleDC(NULL);
    hdcDst = CreateCompatibleDC(NULL);
      
    hbmSrcT = SelectBitmap(hdcSrc, hbmImage);
    hbmDstT = SelectBitmap(hdcDst, hbmMask);
      
    clrTopLeft = GetPixel(hdcSrc, 0, 0);
    clrSaveBk = SetBkColor(hdcSrc, clrTopLeft);

    BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

    clrSaveDstText = SetTextColor(hdcSrc, RGB(255,255,255));
    SetBkColor(hdcSrc, RGB(0,0,0));
      
    BitBlt(hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, hdcDst, 0, 0, SRCAND);

    SetTextColor(hdcDst,clrSaveDstText);
      
    SetBkColor(hdcSrc,clrSaveBk);
    SelectBitmap(hdcSrc,hbmSrcT);
    SelectBitmap(hdcDst,hbmDstT);
      
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
}

void
draw_dpf_logo(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdcMem;
    HBITMAP hbmT;
    BITMAP bm;
    HDC hdc = BeginPaint(hwnd, &ps);

    hdcMem = CreateCompatibleDC(NULL);

    hbmT = SelectBitmap(hdcMem, hbmMask);

    GetObject(hbmMask, sizeof(bm), &bm);

    BitBlt(hdc, 110, 415, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCAND);

    SelectBitmap(hdcMem, hbmImage);
    BitBlt(hdc, 110, 415, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCPAINT);
      
    SelectBitmap(hdcMem, hbmT);
    DeleteDC(hdcMem);

    EndPaint(hwnd, &ps);
}

void
cleanup_logo()
{
    DeleteObject(hbmImage);
    DeleteObject(hbmMask);
}

