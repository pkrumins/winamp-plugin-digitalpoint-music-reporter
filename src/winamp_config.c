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

#include <stdio.h>
#include <windows.h>

#include "winamp.h"
#include "winamp_config.h"

#define DEFAULT_FORMAT "%A - %T"  /* Artist - Title */

CRITICAL_SECTION cs_global_conf;

struct active_conf dpf_conf  = {
    NULL, 0, { 0 }, { 0 }, { 0 }, { "Everything is working fine." }
};

void
config_read()
{
    LRESULT res;

    res = SendMessage(plugin.hwndParent, WM_USER, 0, IPC_GETINIFILE);
    if (res < 65535) { /* no idea why 65535 */
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "Everything is working but winamp.ini configuration file was not found.\r\n"
            "Configuration may not get saved.");

        dpf_conf.ini_file = "winamp.ini";
    }
    else {
        dpf_conf.ini_file = (char *)res;
    }

    dpf_conf.enabled = GetPrivateProfileInt(PLUGIN_NAME, "enabled", 0, dpf_conf.ini_file);
    if (dpf_conf.enabled >= 1) dpf_conf.enabled = 1;
    else dpf_conf.enabled = 0;

    GetPrivateProfileString(PLUGIN_NAME, "user", NULL, dpf_conf.user, sizeof(dpf_conf.user),
        dpf_conf.ini_file);

    GetPrivateProfileString(PLUGIN_NAME, "hash", NULL, dpf_conf.hash, sizeof(dpf_conf.hash),
        dpf_conf.ini_file);

    GetPrivateProfileString(PLUGIN_NAME, "format", DEFAULT_FORMAT, dpf_conf.format, sizeof(dpf_conf.format),
        dpf_conf.ini_file);
}

void
config_write()
{
    int ret;

    ret = WritePrivateProfileString(PLUGIN_NAME,  "enabled", dpf_conf.enabled ? "1" : "0", dpf_conf.ini_file);
    ret += WritePrivateProfileString(PLUGIN_NAME, "user",    dpf_conf.user,                dpf_conf.ini_file);
    ret += WritePrivateProfileString(PLUGIN_NAME, "hash",    dpf_conf.hash,                dpf_conf.ini_file);
    ret += WritePrivateProfileString(PLUGIN_NAME, "format",  dpf_conf.format,              dpf_conf.ini_file);

    if (ret < 4) { /* 4 calls to WritePrivateProfileString above */
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "Everything is working fine but configuration could not be saved.");
    }
}

