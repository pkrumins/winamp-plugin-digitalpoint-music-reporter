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
#include "winamp_config.h"
#include "dialog_config.h"

INT_PTR CALLBACK TabConfigProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {
        case WM_INITDIALOG: {
            HWND control_hwnd;

            // Limit maximum text control input length
            control_hwnd = GetDlgItem(hwndDlg, IDC_EDIT_DISPLAY_FORMAT);
            if (control_hwnd != NULL)
                SendMessage(control_hwnd, EM_SETLIMITTEXT, 128, 0);

            control_hwnd = GetDlgItem(hwndDlg, IDC_EDIT_HASH);
            if (control_hwnd != NULL)
                SendMessage(control_hwnd, EM_SETLIMITTEXT, 32, 0);

            control_hwnd = GetDlgItem(hwndDlg, IDC_EDIT_USER);
            if (control_hwnd != NULL)
                SendMessage(control_hwnd, EM_SETLIMITTEXT, 32, 0);

            // Show saved configuration options
            SetDlgItemText(hwndDlg, IDC_EDIT_USER, dpf_conf.user);
            SetDlgItemText(hwndDlg, IDC_EDIT_HASH, dpf_conf.hash);
            SetDlgItemText(hwndDlg, IDC_EDIT_DISPLAY_FORMAT, dpf_conf.format);
            SetDlgItemText(hwndDlg, IDC_STATUS, dpf_conf.plugin_status);

            CheckDlgButton(hwndDlg, IDC_ENABLE, dpf_conf.enabled ? BST_CHECKED : BST_UNCHECKED);
        }
    }
    return FALSE;
}

