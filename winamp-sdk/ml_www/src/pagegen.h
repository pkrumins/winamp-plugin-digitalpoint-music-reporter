#ifndef _PAGEGEN_H_
#define _PAGEGEN_H_


#define CSS_STR "<link rel=\"stylesheet\" href=\"/style.css\" type=\"text/css\">"

#include "../../jnetlib/webserver.h"
#include "../../gen_ml/gaystring.h"

class MemPageGenerator : public IPageGenerator
{
  public:
    virtual ~MemPageGenerator() { free(m_buf); }
    MemPageGenerator(char *buf, int buf_len=-1) { m_buf=buf; if (buf_len >= 0) m_buf_size=buf_len; else m_buf_size=strlen(buf); m_buf_pos=0; }
    virtual int GetData(char *buf, int size); // return 0 when done

  private:
    char *m_buf;
    int m_buf_size;
    int m_buf_pos;
};
/*
  WWW plug-in for the Winamp 2.9+ media library
  Copyright (C) 2003-2004 Nullsoft, Inc.
  
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

class WAPlaylistGenerator : public IPageGenerator
{
  public:
    virtual ~WAPlaylistGenerator() { }
    WAPlaylistGenerator(HWND hwndWinamp, int is_admin);
    virtual int GetData(char *buf, int size); // return 0 when done

  private:
    HWND m_hwnd;
    int tracks_pos;
    int tracks_len;
    GayString curbuf;
    int curbuf_pos,curbuf_len;

    int m_wa_whichtrack;
    int m_wa_repeat;
    int m_wa_shuffle;
    int m_playstat;

    int m_is_admin;
};


class MLQueryGenerator : public IPageGenerator
{
  public:
    virtual ~MLQueryGenerator();
    MLQueryGenerator(HWND hwndLibrary, mlQueryStruct *qs, char *qstr, int is_right, int allow_control, int allow_stream);
    virtual int GetData(char *buf, int size); // return 0 when done

  private:
    HWND m_hwndLibrary;
    int item_pos;
    mlQueryStruct *m_qs;

    GayString curbuf;

    int m_rowstate;
    char *m_pagename;
    char *m_lastartist, *m_lastalbum;

    int curbuf_pos,curbuf_len;
    int m_iscontrol;
    int m_canstream;
    int m_isadmin;
};

class FilePageGenerator : public IPageGenerator
{
  public:
    virtual ~FilePageGenerator();
    FilePageGenerator(char *fn, JNL_HTTPServ *serv);
    virtual int GetData(char *buf, int size); // return 0 when done
    int is_error() { return !m_fp; }

  private:
    FILE *m_fp;
    int m_file_pos;
    int m_file_len;
};

#endif//_PAGEGEN_H_