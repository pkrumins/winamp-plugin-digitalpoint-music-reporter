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
#include <process.h>
#include <commctrl.h>

#include <stdio.h>

#include "resource.h"

#include "winamp.h"
#include "winamp_config.h"
#include "dialog_main.h"
#include "notifier.h"

static int initCommControls();
static int initThread();

static int  init();
static void config();
static void quit();

static HANDLE thread_quit_event;

winampGeneralPurposePlugin plugin =
{
    GPPHDR_VER,
    PLUGIN_NAME " v" PLUGIN_VERSION,
    init,
    config,
    quit,
};

BOOL WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void)hinstDLL;
    (void)fdwReason;
    (void)lpvReserved;

    return TRUE;
}

__declspec(dllexport) winampGeneralPurposePlugin *
winampGetGeneralPurposePlugin()
{
    return &plugin;
}

static void __cdecl
checker_thread(void *dummy)
{
    int enabled;
    DWORD wait_result;

    (void) dummy;

    while (1) {
        EnterCriticalSection(&cs_global_conf);
        enabled = dpf_conf.enabled;
        LeaveCriticalSection(&cs_global_conf);

        if (enabled) try_notify_song_change();

        wait_result = WaitForSingleObject(thread_quit_event, 5*1000);
        switch (wait_result) {
        case WAIT_OBJECT_0:
            DeleteCriticalSection(&cs_global_conf);
            CleanupCurl();
            _endthread();
        }
    }
}

void config()
{
    ShowMainDialog();
}

static int
init()
{
    /* Initialize common controls */
    initCommControls();

    /* Initialize cURL library */
    initCurl();

    /* Initialize config structure */
    config_read();

    /* Initialize and create checker thread */
    initThread();

    return 0;
}

static int
initCommControls()
{
    BOOL initret;

    INITCOMMONCONTROLSEX ics = {
        sizeof(INITCOMMONCONTROLSEX),
        ICC_TAB_CLASSES|ICC_UPDOWN_CLASS
    };
    initret = InitCommonControlsEx(&ics);

    if (initret) return 0;   /* initret true means failure */
    return 1;                /* success */
}

static int
initThread()
{
    uintptr_t thread_ret;

    InitializeCriticalSection(&cs_global_conf);
    thread_quit_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    /* Create a thread and with checkr_thread entry point */
    thread_ret = _beginthread(checker_thread, 0, NULL);
    if (thread_ret == -1L) {
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "Failed creating a thread.");
        return 0;
    }
    CloseHandle((HANDLE)thread_ret);

    return 1;
}

static void
quit()
{
    config_write();

    SetEvent(thread_quit_event);
}

