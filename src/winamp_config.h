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

#ifndef DPF_WINAMP_CONFIG_H
#define DPF_WINAMP_CONFIG_H

#include <windows.h>

struct active_conf {
    char *ini_file;
    int  enabled;
    char user[33];
    char hash[33];
    char format[129];
    char plugin_status[292];
};

extern struct active_conf dpf_conf;
extern CRITICAL_SECTION cs_global_conf;

void config_read();
void config_write();

#endif

