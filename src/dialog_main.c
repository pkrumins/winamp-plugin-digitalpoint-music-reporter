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
#include <commctrl.h>
#include "resource.h"

#include "winamp.h"
#include "logo.h"

#include "dialog_main.h"
#include "dialog_config.h"
#include "dialog_about.h"

#include "winamp_config.h"

static int dialog_open;
static HWND config_dialog;

static INT_PTR CALLBACK MainDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void HandleTabNotify(HWND hwnd, LPNMHDR pnmhdr);
static void SetTabPage(HWND hwnd, int page);
static void SetupTabs(HWND hwnd);

void
ShowMainDialog()
{
    if (dialog_open) {
        if (IsWindow(config_dialog)) {
            SetFocus(config_dialog);
            return;
        }
        else {
            dialog_open = 0;
        }
    }
    
    DialogBox(plugin.hDllInstance, MAKEINTRESOURCE(IDD_MAIN), plugin.hwndParent, MainDialogProc);
}

static INT_PTR CALLBACK
MainDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;
    
    switch (uMsg) {
    case WM_INITDIALOG:
        dialog_open = 1;
        config_dialog = hwndDlg;

        SetWindowText(hwndDlg, PLUGIN_NAME);

        // draw the DPF logo
        init_logo(hwndDlg);

        // setup the tabs
        SetupTabs(hwndDlg);

        // activate the 1st tab (Configuration)
        SetTabPage(hwndDlg, 0);

        return TRUE;

    case WM_NOTIFY:
        if (wParam == IDC_TAB) {
            // change configuration dialog tab
            HandleTabNotify(hwndDlg, (LPNMHDR)lParam);
            return TRUE;
        }
        break;

    case WM_COMMAND: {
        HWND hConf  = GetDlgItem(hwndDlg, IDD_CONF);

        switch (LOWORD(wParam)) {
            case IDOK:
                if (hConf) {
                    EnterCriticalSection(&cs_global_conf);

                    dpf_conf.enabled = IsDlgButtonChecked(hConf, IDC_ENABLE); 
                    GetDlgItemText(hConf, IDC_EDIT_USER, dpf_conf.user, sizeof(dpf_conf.user));
                    GetDlgItemText(hConf, IDC_EDIT_HASH, dpf_conf.hash, sizeof(dpf_conf.hash));
                    GetDlgItemText(hConf, IDC_EDIT_DISPLAY_FORMAT, dpf_conf.format, sizeof(dpf_conf.format));

                    LeaveCriticalSection(&cs_global_conf);

                    config_write();
                }

            case IDCANCEL: {
                dialog_open = 0;
                cleanup_logo();
                EndDialog(hwndDlg, 0);
                return TRUE;
            }
        }
        break;
    }

    case WM_PAINT:
        draw_dpf_logo(hwndDlg);

        return TRUE;
    }
	return FALSE;
}

static void
HandleTabNotify(HWND hwnd, LPNMHDR pnmhdr)
{
    HWND hctl = GetDlgItem(hwnd, IDC_TAB);

    if(pnmhdr->code == TCN_SELCHANGE)
    {
        int p = TabCtrl_GetCurSel(hctl);

        SetTabPage(hwnd, p);
    }
}

static void
SetTabPage(HWND hwnd, int page)
{
    HWND hConf  = GetDlgItem(hwnd, IDD_CONF);
    HWND hAbout = GetDlgItem(hwnd, IDD_About);

    switch (page) {
        case 0: 
            if(!hConf) {
                hConf = CreateDialog((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
                                     MAKEINTRESOURCE(IDD_CONF), hwnd, TabConfigProc);
                SetWindowLong(hConf, GWL_ID, IDD_CONF);
            }

            SetWindowPos(hConf, 0, 0, 0, 0, 0,
                         SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
            SetFocus(GetWindow(hConf, GW_CHILD));

            if (hAbout) {
                SetWindowPos(hAbout, 0, 0, 0, 0, 0,
                             SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
            }

            break;

        case 1:
            if(!hAbout) {
                hAbout = CreateDialog((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
                                      MAKEINTRESOURCE(IDD_About), hwnd, TabAboutProc);
                SetWindowLong(hAbout, GWL_ID, IDD_About);
            }

            SetWindowPos(hAbout, 0, 0, 0, 0, 0,
                         SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
            SetFocus(GetWindow(hAbout, GW_CHILD));

            if (hConf) {
                SetWindowPos(hConf, 0, 0, 0, 0, 0,
                             SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
            }

            break;
    }
}

static void
SetupTabs(HWND hwnd)
{
    HWND hctl = GetDlgItem(hwnd, IDC_TAB);

    TCITEM item;

    item.mask = TCIF_TEXT;
    item.pszText = "Configuration";
    TabCtrl_InsertItem(hctl, 0, &item);

    item.pszText = "About";
    TabCtrl_InsertItem(hctl, 2, &item);
}

