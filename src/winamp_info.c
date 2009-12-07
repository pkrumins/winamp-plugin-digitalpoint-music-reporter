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
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "winamp.h"
#include "winamp_info.h"

static int
get_current_track_number()
{
    return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
}

const char *
get_winamp_playlist_file()
{
    int track_id;
    const char *path;

    track_id = get_current_track_number();
    path = (char *)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)track_id, IPC_GETPLAYLISTFILE);

    return path;
}

const char *
get_winamp_playlist_title()
{
    int track_id;
    const char *title;

    track_id = get_current_track_number();
    title = (char *)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)track_id, IPC_GETPLAYLISTTITLE);

    return title;
}

int
get_winamp_play_status()
{
    return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
}

char *
get_winamp_metainfo(const char *which_info, const char *playlist_file_path)
{
    char *ret_data;
    char *which_info_buf;
    char *playlist_file_path_buf;
    extendedFileInfoStruct xstruct;
    
    if (strcmp(which_info, "Bitrate") == 0) {
        /* need to retrieve bitrate differently */
        int bitrate = (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 1, IPC_GETINFO);
        ret_data = calloc(12, sizeof(char));
        _snprintf(ret_data, 12, "%d", bitrate);
        
        return ret_data;
    }

    which_info_buf = _strdup(which_info);
    playlist_file_path_buf = _strdup(playlist_file_path);

    ret_data = calloc(META_SIZE + 1, sizeof(char));

    xstruct.filename = playlist_file_path_buf;
    xstruct.metadata = which_info_buf;
    xstruct.ret = ret_data;
    xstruct.retlen = META_SIZE;

    if (!SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&xstruct,
                    IPC_GET_EXTENDED_FILE_INFO_HOOKABLE))
    {
        /* no extended metainfo */
        sprintf(ret_data, "no_ex");
    }

    free(which_info_buf);
    free(playlist_file_path_buf);

    return ret_data;
}

int
get_winamp_song_info(struct song_info *si)
{
    #define MAX_INFOS 5

    const char *path;
    char *info_ptr;
    const char *infos[MAX_INFOS] = {
        "Artist", "Title",   "Album",
        "Year",   "Bitrate",
    }; 
    const size_t buf_sizes[MAX_INFOS] = {
        sizeof(si->artist), sizeof(si->title),    sizeof(si->album),
        sizeof(si->year),   sizeof(si->bitrate),
    };
    char *buf_ptrs[MAX_INFOS] = {
        si->artist, si->title, si->album,
        si->year, si->bitrate,
    };
    unsigned int iter;

    path = get_winamp_playlist_file();
    if (path == NULL) return 0;

    for (iter = 0; iter < MAX_INFOS; iter++)
    {
        info_ptr = get_winamp_metainfo(infos[iter], path);
        strncpy(buf_ptrs[iter], info_ptr, buf_sizes[iter]);
        buf_ptrs[iter][buf_sizes[iter] - 1] = '\0';
        free(info_ptr);
    }

    info_ptr = get_winamp_metainfo("Length", path);
    iter = atoi(info_ptr); /* milliseconds */
    _snprintf(si->length, sizeof(si->length), "%d:%02d",
        iter/1000/60, (iter/1000)%60);
    si->length[sizeof(si->length) - 1] = '\0';
    free(info_ptr);

    strncpy(si->path, path, MAX_PATH);
    si->path[MAX_PATH - 1] = '\0';

    si->play_status = get_winamp_play_status();

    return 1;
}

int
get_winamp_song_info_playlist(struct song_info *si)
{
    const char *ptp;
    char playlist_title[512] = { 0 };
    char *endptr, *beginptr;
    int open_parens = 0;
    
    ptp = get_winamp_playlist_title();
    if (ptp == NULL) return 0;

    strncpy(playlist_title, ptp, sizeof(playlist_title) - 1);
    beginptr = playlist_title;

    /* check if we don't have "[Buffer: NNN%] " string in front
    */
    /*if (strncmp(playlist_title, "[Buffer: ", 9) == 0) {
          beginptr = strchr(playlist_title, ']');
          if (beginptr == NULL) return;
          beginptr += 2;
      }
    */

    /* if there is "[Buffer: NNN%] " string in front, do nothing */
    if (strncmp(playlist_title, "[Buffer: ", 9) == 0) return 0;

    /* if there is "[Connecting: " string in front, do nothing */
    if (strncmp(playlist_title, "[Connecting", 11) == 0) return 0;

    /* now we have seomthing like
    ** 1. Boogie Pimps - Ive Got The Music In Me (Radio Version) (PulsRadio (www.pulsradio.com))
    */
    /*    artist       - title                                   (radio (station))
    */

    endptr = playlist_title + strlen(playlist_title);
    if (endptr == playlist_title) return 0;

    /* get rid of radio station */
    if (*(endptr - 1) == ')') {
        endptr--;
        do {
            switch(*endptr) {
            case ')':
                open_parens++;
                break;
            case '(':
                open_parens--;
                break;
            }
            endptr--;
        } while(open_parens != 0 && endptr != beginptr);

        if (endptr != beginptr) {
            while (endptr != beginptr && *endptr == ' ')
                endptr--;

            *(endptr + 1) = '\0';
        }
    }

    /* now we have seomthing like
    ** 1. Boogie Pimps - Ive Got The Music In Me (Radio Version)
    */
    /*    artist       - title
    */
    
    /* try to extract artist and title
    ** artist = strstr(endptr, " - ");
	*/
    
    /* copy to si */
    strncpy(si->stream_title, beginptr, sizeof(si->stream_title));
    si->stream_title[sizeof(si->stream_title) - 1] = '\0';

    return 1;
}

char *
format_song(const char *format, const struct song_info *si)
{
    char *ret_buf;
    char in;
    size_t alloc, length, strindex;
    size_t fieldlen, skipfield, newlen;
    const char *field;
    
    if (strlen(format) == 0) return format_song("%A - %T", si);

    alloc = strlen(format) + 1;
    length = alloc - 1;
    strindex = 0;

    ret_buf = malloc(sizeof(char) * alloc);
    if (ret_buf == NULL) return NULL;

    while (length--) {
        in = *format;
        switch (in) {
        case '%':
            skipfield = 0;

            switch (*(format + 1)) {
            case '\0':
                skipfield = 1;
                break;
            case 'A':
            case 'a':
                field = si->artist;
                break;
            case 'T':
            case 't':
                field = si->title;
                break;
            case 'L':
            case 'l':
                field = si->album;
                break;
            case 'Y':
            case 'y':
                field = si->year;
                break;
            case 'B':
            case 'b':
                field = si->bitrate;
                break;
            case 'E':
            case 'e':
                field = si->length;
                break;
            case 'P':
            case 'p':
                field = si->path;
                break;
            case '%':
                ret_buf[strindex++] = in;
                format++;
                length--;
            default:
                skipfield = 1;
                break;
            }

            if (skipfield) break;
            
            fieldlen = strlen(field);
            newlen = alloc + fieldlen - 2;

            if (newlen > alloc) {
                char *newretbuf = realloc(ret_buf, sizeof(char) * newlen);
                if (newretbuf == NULL) {
                    free(ret_buf);
                    return NULL;
                }
                ret_buf = newretbuf;
                alloc = newlen;
            }
            strcpy(&ret_buf[strindex], field);
            strindex += fieldlen;
            format++;
            length--;

            break;
        default:
            ret_buf[strindex++] = in;
        }
        format++;
    }
    ret_buf[strindex] = '\0';

    return ret_buf;
}

void make_human_readable_si(struct song_info *si)
{
    #define MAX_SEQ 7

    char *what_unknown[MAX_SEQ] = {
        "artist",  "title",  "album", "year",
        "bitrate", "length", "path"
    };
    char *info_seq[MAX_SEQ] = {
        si->artist,  si->title,  si->album, si->year,
        si->bitrate, si->length, si->path
    };
    int iter;

    for (iter = 0; iter < MAX_SEQ; iter++) {
        if (strcmp(info_seq[iter], "no_ex") == 0) {
            strcpy(info_seq[iter], "(unknown ");
            strcat(info_seq[iter], what_unknown[iter]);
            strcat(info_seq[iter], ")");
        }
    }
}

songdiff_type 
song_diff(const struct song_info *si1, const struct song_info *si2)
{
    songdiff_type ret = DIFFER_NO;

    if (strcmp(si1->album, si2->album))
        ret |= DIFFER_ALBUM;
    if (strcmp(si1->artist, si2->artist))
        ret |= DIFFER_ARTIST;
    if (strcmp(si1->length, si2->length))
        ret |= DIFFER_LENGTH;
    if (strcmp(si1->path, si2->path))
        ret |= DIFFER_PATH;
    if (strcmp(si1->title, si2->title))
        ret |= DIFFER_TITLE;
    if (strcmp(si1->year, si2->year))
        ret |= DIFFER_YEAR;
    if (strcmp(si1->bitrate, si2->bitrate))
        ret |= DIFFER_BITRATE;
    if (si1->play_status != si2->play_status)
        ret |= DIFFER_PLAY_STATUS;
    if (strcmp(si1->stream_title, si2->stream_title))
        ret |= DIFFER_STREAM_TITLE;

    return ret;
}

