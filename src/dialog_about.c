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

#include "resource.h"
#include "dialog_about.h"

/* This code is based on code that was found on codeguru a long time ago */

#define PROP_ORIGINAL_PROC    "__Hyperlink__CM__Original__Proc__"
#define PROP_UNDERLINE_FONT   "__Hyperlink__CM__Underline__Font__"
#define PROP_HYPERLINK        "__Hyperlink__CM__"
#define PROP_ORIGINAL_FONT    "__Hyperlink__CM__Original__Font__"

static LRESULT CALLBACK HyperLinkStaticTextProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void MakeClickable(HWND hwndDlg, int id);

INT_PTR CALLBACK TabAboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {

        case WM_INITDIALOG: {
            // peter@catonmat.net
            MakeClickable(hwndDlg, IDC_PETER_EMAIL);

            // http://www.catonmat.net
            MakeClickable(hwndDlg, IDC_PETER_URL);

            // http://www.abuzant.com
            MakeClickable(hwndDlg, IDC_ABUZANT_URL);

            return TRUE;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_PETER_EMAIL:
                    ShellExecute(NULL, NULL, "mailto:peter@catonmat.net", NULL, NULL, SW_SHOWNORMAL);
                    return TRUE;
                case IDC_PETER_URL:
                    ShellExecute(NULL, "open", "http://www.catonmat.net", NULL, NULL, SW_SHOWNORMAL);
                    return TRUE;
                case IDC_ABUZANT_URL:
                    ShellExecute(NULL, "open", "http://www.abuzant.com", NULL, NULL, SW_SHOWNORMAL);
                    return TRUE;
            }
            break;
        }
	    case WM_CTLCOLORSTATIC: {
			HDC  hdc     = (HDC)wParam;
			HWND hwndCtl = (HWND)lParam;

            if (GetDlgItem(hwndDlg, IDC_PETER) == hwndCtl) {
                return (INT_PTR)GetStockObject(WHITE_BRUSH);
            }

            if (GetDlgItem(hwndDlg, IDC_ABUZANT_URL) == hwndCtl) {
                SetTextColor(hdc, RGB(0, 0, 192));
                SetBkColor(hdc, (COLORREF)GetSysColor(COLOR_BTNFACE)); 
                return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
            }

            if (GetProp(hwndCtl, PROP_HYPERLINK) != NULL) {
                SetTextColor(hdc, RGB(0, 0, 192));
                SetBkMode(hdc, TRANSPARENT);
                return (INT_PTR)GetStockObject(WHITE_BRUSH);
            }

            SetBkColor(hdc, (COLORREF)GetSysColor(COLOR_BTNFACE));
            return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
		}
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwndDlg, &ps);
            HPEN whitePen = CreatePen(PS_SOLID, 1, RGB(0xFF, 0xFF, 0xFF));
            HBRUSH whiteBrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
            HPEN hpenOld;
            HBRUSH hbrushOld;

            hpenOld   = SelectObject(hdc, whitePen);
            hbrushOld = SelectObject(hdc, whiteBrush);
            Rectangle(hdc, 29, 37, 200, 206);

            SelectObject(hdc, hpenOld);
            DeleteObject(whitePen);
            SelectObject(hdc, hbrushOld);
            DeleteObject(whiteBrush);

            EndPaint(hwndDlg, &ps);

            return TRUE;
        }
    }
    return FALSE;
}

static void
MakeClickable(HWND hwndDlg, int id)
{
    HWND clickable_ctrls = GetDlgItem(hwndDlg, id);
    if (clickable_ctrls != NULL) {
        WNDPROC pfnOrigProc;
        HFONT hOrigFont, hFont;
        LOGFONT lf;

        // make sure control will send notifications
        DWORD dwStyle = GetWindowLong(clickable_ctrls, GWL_STYLE);
        SetWindowLong(clickable_ctrls, GWL_STYLE, dwStyle | SS_NOTIFY);

        // subclass the control
        pfnOrigProc = (WNDPROC)GetWindowLong(clickable_ctrls, GWL_WNDPROC);
        SetProp(clickable_ctrls, PROP_ORIGINAL_PROC, (HANDLE)pfnOrigProc);
        SetWindowLong(clickable_ctrls, GWL_WNDPROC, (LONG)(WNDPROC)HyperLinkStaticTextProc);

        // create and updated font with underline
        hOrigFont = (HFONT)SendMessage(clickable_ctrls, WM_GETFONT, 0, 0);
        SetProp(clickable_ctrls, PROP_ORIGINAL_FONT, (HANDLE)hOrigFont);

        GetObject(hOrigFont, sizeof(lf), &lf);
        lf.lfUnderline = TRUE;

        hFont = CreateFontIndirect(&lf);
        SetProp(clickable_ctrls, PROP_UNDERLINE_FONT, (HANDLE)hFont);

        // Set a flag on the control so we know what color it should be.
        SetProp(clickable_ctrls, PROP_HYPERLINK, (HANDLE)1);
    }
}

static LRESULT CALLBACK
HyperLinkStaticTextProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwndDlg, PROP_ORIGINAL_PROC);

    switch (uMsg) {
        case WM_SETCURSOR: {
            // Because IDC_HAND is not available on all operating
            // systems, we will load the arrow cursor if IDC_HAND is not
            // present.
            HCURSOR hCursor = LoadCursor(NULL, IDC_HAND);
            if (NULL == hCursor) {
                hCursor = LoadCursor(NULL, IDC_ARROW);
            }
            SetCursor(hCursor);

            return TRUE;
        }

        case WM_MOUSEMOVE:
			if (GetCapture() != hwndDlg)
			{
				HFONT hFont = (HFONT)GetProp(hwndDlg, PROP_UNDERLINE_FONT);
				SendMessage(hwndDlg, WM_SETFONT, (WPARAM)hFont, FALSE);
				InvalidateRect(hwndDlg, NULL, FALSE);
				SetCapture(hwndDlg);
			}
			else
			{
				RECT rect;
                POINT pt = { LOWORD(lParam), HIWORD(lParam) };

                GetWindowRect(hwndDlg, &rect);
				ClientToScreen(hwndDlg, &pt);

				if (!PtInRect(&rect, pt))
				{
					HFONT hFont = (HFONT)GetProp(hwndDlg, PROP_ORIGINAL_FONT);
					SendMessage(hwndDlg, WM_SETFONT, (WPARAM)hFont, FALSE);
					InvalidateRect(hwndDlg, NULL, FALSE);
					ReleaseCapture();
				}
			}
			break;

        case WM_DESTROY: {
            HFONT hOrigFont;
            HFONT hFont;

			SetWindowLong(hwndDlg, GWL_WNDPROC, (LONG)pfnOrigProc);
			RemoveProp(hwndDlg, PROP_ORIGINAL_PROC);

			hOrigFont = (HFONT)GetProp(hwndDlg, PROP_ORIGINAL_FONT);
			SendMessage(hwndDlg, WM_SETFONT, (WPARAM)hOrigFont, 0);
			RemoveProp(hwndDlg, PROP_ORIGINAL_FONT);

			hFont = (HFONT)GetProp(hwndDlg, PROP_UNDERLINE_FONT);
			DeleteObject(hFont);
			RemoveProp(hwndDlg, PROP_UNDERLINE_FONT);

			RemoveProp(hwndDlg, PROP_HYPERLINK);

			break;
        }
    }

    return CallWindowProc(pfnOrigProc, hwndDlg, uMsg, wParam, lParam);
}

