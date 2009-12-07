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

#ifndef DPF_WINAMP_INFO_H
#define DPF_WINAMP_INFO_H

typedef enum PLAY_STATUS_T {
    PLAY_NOT_PLAYING = 0,
    PLAY_PLAYING = 1,
    PLAY_PAUSED = 3
} PLAY_STATUS;

struct song_info {
    char         artist[128];
    char         title[128];
    char         album[128];
    char         year[15];       // max for "(unknown year)"
    char         bitrate[18];    // max for "(unknown bitrate)"
    char         length[17];     // max for "(unknown length)"
    char         path[MAX_PATH];
    PLAY_STATUS  play_status;
    char         stream_title[512];
};

#define META_SIZE 128

typedef enum {
    DIFFER_NO     = 0x00, DIFFER_ARTIST = 0x01, DIFFER_TITLE       = 0x02,
    DIFFER_ALBUM  = 0x04, DIFFER_YEAR   = 0x08, DIFFER_BITRATE     = 0x10,
    DIFFER_LENGTH = 0x20, DIFFER_PATH   = 0x40, DIFFER_PLAY_STATUS = 0x80,
    DIFFER_STREAM_TITLE = 0x100,

    DIFFER_EVERYTHING = 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80 | 0x100
} songdiff_type;

#define CHECK_SONG_DIFF(diff, which) ((diff & which) != 0)
#define CHECK_DIFF_ONLY(diff, which) ((diff | which) == which)

int get_winamp_play_status();
int get_winamp_song_info(struct song_info *si);
int get_winamp_song_info_playlist(struct song_info *si);
const char *get_winamp_playlist_file();
char *get_winamp_metainfo(const char *which_info, const char *playlist_file_path);

char *format_song(const char *format, const struct song_info *si);
void make_human_readable_si(struct song_info *si);

songdiff_type song_diff(const struct song_info *si1, const struct song_info *si2);

#endif

