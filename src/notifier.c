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

#include <curl/curl.h>

#include "notifier.h"
#include "winamp_config.h"
#include "winamp_info.h"

static CURL *curl;
static char curl_error_buf[CURL_ERROR_SIZE];

static void request_dpf_song_url(const struct song_info *si);
static void do_curl_request(const char *url);
static char *create_get_url(const char *user, const char *hash, const char *song);
static char *get_formatted_song(const struct song_info *si);

int
initCurl()
{
    CURLcode curl_ret;

    /* Initialize global cURL data */
    curl_ret = curl_global_init(CURL_GLOBAL_WIN32);
    if (curl_ret != CURLE_OK) {
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "LibCURL failure: %s.", curl_easy_strerror(curl_ret));
        return 0;
    }

    /* Initialize cURL easy handle */
    curl = curl_easy_init();
    if (curl == NULL) {
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "LibCURL failure: something went wrong.");
        return 0;
    }

    /* Set the error buffer for cURL errors */
    curl_ret = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_error_buf);
    if (curl_ret != CURLE_OK) {
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "LibCURL failure: failed setting error buffer.");
        return 0;
    }

    return 1;
}

void
CleanupCurl()
{
    curl_easy_cleanup(curl);
}

void try_notify_song_change()
{
    static struct song_info old_si;
    struct song_info si = { 0 };
    songdiff_type differ;
    int ret;
    
    ret = get_winamp_song_info(&si);
    if (ret != 0) {
        if (strstr(si.path, "http://") != NULL) {
            /* we got internet stream and we have a problem.
            ** retrieve the song title/artist from playlist
            ** TODO: really retrieve title/artist, for the moment retrieves stream_title
            */
            if (!get_winamp_song_info_playlist(&si)) return;
        }

        differ = song_diff(&si, &old_si);
        if (differ != DIFFER_NO) { /* if song info's differ */
            if (!(si.play_status == PLAY_PLAYING && CHECK_DIFF_ONLY(differ, DIFFER_BITRATE))
                || strncmp(dpf_conf.plugin_status, "LibCURL", 7) == 0)
            {
                /* does not differ only in bitrate OR there was an error (retry) */
                request_dpf_song_url(&si);
                
                old_si = si;
            }
        }
    }
}

static void
request_dpf_song_url(const struct song_info *si)
{
    if (si == NULL) return;

    if (curl != NULL) {
        char *url;
        char *song;

        EnterCriticalSection(&cs_global_conf);

        song = get_formatted_song(si);
        if (song == NULL) {
            _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
                    "Error: not enough memory.");
            LeaveCriticalSection(&cs_global_conf);
            return;
        }
        url = create_get_url(dpf_conf.user, dpf_conf.hash, song);
        if (url == NULL) {
            _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
                    "Error: not enough memory.");
            free(song);
            LeaveCriticalSection(&cs_global_conf);
            return;
        }

        LeaveCriticalSection(&cs_global_conf);

        do_curl_request(url);

        free(url);
        free(song);
    }
}

static char *
get_formatted_song(const struct song_info *si)
{
    #define SONG_RET_BUF_SIZE 1536
    
    char *songret;
    char *song_to_format;
    char *escaped_display;
    int do_free_song_to_format;
    struct song_info local_si = *si;
    
    /* if the playback is paused or stopped, return empty song (not listening to any) */
    if (si->play_status == PLAY_PAUSED || si->play_status == PLAY_NOT_PLAYING) {
        songret = calloc(4, sizeof(char));
        return songret;
    }

    songret = malloc(sizeof(char) * SONG_RET_BUF_SIZE);
    if (songret == NULL) return NULL;

    /* see if we have internet stream, if so, song title does not get formatted
    ** otherwise we format the song according to user defined format
	*/
    if (strlen(si->stream_title) == 0) {
        /* not an internet stream */
        char *formatted_display;

        make_human_readable_si(&local_si);

        formatted_display = format_song(dpf_conf.format, &local_si);
        if (formatted_display == NULL) {
            free(songret);
            return NULL;
        }

        song_to_format = formatted_display;
        do_free_song_to_format = 1;
    }
    else {
        /* internet stream */
        
        song_to_format = local_si.stream_title;
        do_free_song_to_format = 0;
    }

    escaped_display = curl_easy_escape(curl, song_to_format, 0);
    if (escaped_display == NULL) {
        if (do_free_song_to_format) free(song_to_format);
        return NULL;
    }

    strncpy(songret, escaped_display, SONG_RET_BUF_SIZE - 1);
    songret[SONG_RET_BUF_SIZE - 1] = '\0';

    curl_free(escaped_display);
    if (do_free_song_to_format) free(song_to_format);

    return songret;
}

static char *
create_get_url(const char *user, const char *hash, const char *song)
{
    #define URL_RET_BUF_SIZE 2048

    const char str_format[] = "%s?u=%s&p=%s&song=%s";
    const char DPF_ITUNES_URL[] = "http://forums.digitalpoint.com/itunes.php";
	/*const char DPF_ITUNES_URL[] = "http://192.168.1.2:5000/itunes.php";*/

    char *returl = malloc(sizeof(char) * URL_RET_BUF_SIZE);
    if (returl == NULL) return NULL;

    _snprintf(returl, URL_RET_BUF_SIZE, str_format, DPF_ITUNES_URL, user, hash, song);

	return returl;
}

static void
do_curl_request(const char *url)
{
    static char last_plugin_status[sizeof(dpf_conf.plugin_status)];
    CURLcode curl_ret;

    /* save last plugin status to restore it if we had a LibCURL error but then
    ** the error disappeared
    */
    if (strncmp(dpf_conf.plugin_status, "LibCURL", 7) != 0) {
        strncpy(last_plugin_status, dpf_conf.plugin_status, sizeof(last_plugin_status) - 1);
    }

    curl_ret = curl_easy_setopt(curl, CURLOPT_URL, url);
    if (curl_ret != CURLE_OK) {
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "LibCURL failure: %s", curl_easy_strerror(curl_ret));
        return;
    }
    
    curl_ret = curl_easy_perform(curl);
    if (curl_ret != CURLE_OK) {
        _snprintf(dpf_conf.plugin_status, sizeof(dpf_conf.plugin_status),
            "LibCURL failure: %s", curl_error_buf);
        return;
    }

    if (strncmp(dpf_conf.plugin_status, last_plugin_status, 8) != 0) {
        strncpy(dpf_conf.plugin_status, last_plugin_status, sizeof(last_plugin_status) - 1);
    }
}

