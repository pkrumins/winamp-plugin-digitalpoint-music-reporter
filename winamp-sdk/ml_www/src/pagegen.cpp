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

#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "../../winamp/wa_ipc.h"
#include "../../gen_ml/ml.h"
#include "pagegen.h"
#include "wwwutil.h"


int MemPageGenerator::GetData(char *buf, int size) // return 0 when done
{
  int a=m_buf_size-m_buf_pos;
  if (a < size) size=a;
  memcpy(buf,m_buf+m_buf_pos,size);
  m_buf_pos+=size;
  return size;
}

WAPlaylistGenerator::WAPlaylistGenerator(HWND hwndWinamp, int is_admin)
{ 
  m_is_admin=is_admin;
  tracks_pos=-1;
  m_hwnd=hwndWinamp; 
  tracks_len=SendMessage(m_hwnd,WM_WA_IPC,0,IPC_GETLISTLENGTH);

  m_playstat=SendMessage(m_hwnd,WM_WA_IPC,0,IPC_ISPLAYING);
  m_wa_whichtrack=SendMessage(m_hwnd,WM_WA_IPC,0,IPC_GETLISTPOS);
  m_wa_repeat=SendMessage(m_hwnd,WM_WA_IPC,0,IPC_GET_REPEAT);
  m_wa_shuffle=SendMessage(m_hwnd,WM_WA_IPC,0,IPC_GET_SHUFFLE);


  curbuf_pos=0;

  curbuf.Set("<HTML><HEAD>" CSS_STR "</HEAD><BODY>");

  if (!m_wa_repeat && !m_wa_shuffle)
  {
    char buf[512];
    int l=tracks_len-m_wa_whichtrack;
    sprintf(buf,"<TABLE width=100%%><TR CLASS=rowclass0><TD>%d Item%s in Queue%s:</TD></TR>",l,l==1?"":"s",is_admin?" [<a href=\"left.html?remove=all\">Delete Queue</a>]":"");
    curbuf.Append(buf);
  }
  else
  {
    char buf[512];
    sprintf(buf,"<TABLE width=100%%><TR CLASS=rowclass0><TD>%d Item%s in Playlist%s:</TD></TR>",tracks_len,tracks_len==1?"":"s",is_admin?" [<a href=\"left.html?remove=all\">Delete Playlist</a>]":"");
    curbuf.Append(buf);
  }


  curbuf_len=strlen(curbuf.Get());
}

int WAPlaylistGenerator::GetData(char *buf, int size) // return 0 when done
{
  if (size<1) return 0;

  int addl=curbuf_len-curbuf_pos;
  if (addl > size) addl=size;
  if (addl) memcpy(buf,curbuf.Get()+curbuf_pos,addl);
  curbuf_pos += addl;
  buf += addl;
  size -= addl;
  if (curbuf_pos >= curbuf_len)
  {
    curbuf_pos=curbuf_len=0;
    tracks_pos++;

    if (!tracks_pos && !m_wa_repeat && !m_wa_shuffle) // if we are in this mode, skip tracks_pos to play pos
    {
      if (m_wa_whichtrack >= 0 && m_wa_whichtrack < tracks_len)
      {
        tracks_pos=m_wa_whichtrack;
      }
    }

    if (tracks_pos>tracks_len) return addl;
    if (tracks_pos == tracks_len)
    {
      curbuf_pos=0;
      
      curbuf.Set("</table>");

      curbuf.Append("Playback State:");
      if (m_playstat==1) 
        curbuf.Append(" Playing");
      else if (m_playstat==3) 
        curbuf.Append(" Paused");
      else
        curbuf.Append(" Stopped");

      if (m_is_admin)
      {
        char buf[512];
        sprintf(buf," [Repeat: <a href=\"left.html?repeat=%d\">%s</a>] [Shuffle: <a href=\"left.html?shuffle=%d\">%s</a>]",
            !m_wa_repeat,m_wa_repeat?"On":"Off",
            !m_wa_shuffle,m_wa_shuffle?"On":"Off"
          );
        curbuf.Append(buf);
      }

      curbuf.Append("<BR>");

      if (m_is_admin)
      {
        curbuf.Append("[<a href=\"left.html?action=b15s\">Rew15s</a>] ");
        curbuf.Append("[<a href=\"left.html?action=prev\">Prev</a>] ");
        curbuf.Append("[<a href=\"left.html?action=play\">Play</a>] ");
        curbuf.Append("[<a href=\"left.html?action=pause\">Pause</a>] ");
        curbuf.Append("[<a href=\"left.html?action=stop\">Stop</a>] ");
        curbuf.Append("[<a href=\"left.html?action=next\">Next</a>] ");
        curbuf.Append("[<a href=\"left.html?action=f15s\">Fwd15s</a>]<BR>");


        curbuf.Append("[Set Visualization: <a href=\"left.html?vis=2\">Fullscreen</a>|<a href=\"left.html?vis=1\">On</a>|<a href=\"left.html?vis=0\">Off</a>]<BR>");
        curbuf.Append("[<a href=\"left.html?vidfs=1\">Toggle Video Fullscreen</a>]<BR>");
      }

      curbuf.Append("</html>");
      curbuf_len=strlen(curbuf.Get());

      return addl+GetData(buf,size);
    }
    else
    { 
      char *n=(char*)SendMessage(m_hwnd,WM_WA_IPC,tracks_pos,IPC_GETPLAYLISTTITLE);
      curbuf_pos=0;
      {
        char b[512];
        int tp=(!m_wa_repeat && !m_wa_shuffle) ? 
          tracks_pos-m_wa_whichtrack+1 : tracks_pos+1;

          wsprintf(b,"<TR class=rowclass%d><TD> %d. ",1+(tp&1),
              tp);
        curbuf.Set(b);
      }

      if (m_is_admin)
      {
        char b[1024];
        sprintf(b,"[<a href=\"left.html?remove=%d\">Delete</a>] [<a href=\"left.html?jump=%d\">Jumpto</a>] ",
          tracks_pos,
          tracks_pos);

        curbuf.Append(b);
      }

      if (m_wa_whichtrack == tracks_pos)
      {
        curbuf.Append("<b>");
        curbuf.Append(n);
        curbuf.Append("</b>");

        int pos=SendMessage(m_hwnd,WM_WA_IPC,0,IPC_GETOUTPUTTIME)/1000;
        int len=SendMessage(m_hwnd,WM_WA_IPC,1,IPC_GETOUTPUTTIME);
        char b[512];
        if (m_playstat)
        {
          if (len >= 0)
            wsprintf(b," [%d:%02d/%d:%02d]",pos/60,pos%60,len/60,len%60);
          else
            wsprintf(b," [%d:%02d/?]",pos/60,pos%60);
        }
        else 
          wsprintf(b," [%d:%02d]",len/60,len%60);
        curbuf.Append(b);
      }
      else 
        curbuf.Append(n);
      curbuf.Append("</td></tr>");

      curbuf_len=strlen(curbuf.Get());
      return addl+GetData(buf,size);
    }
  }
  return addl;
}

MLQueryGenerator::~MLQueryGenerator()
{ 
  if (m_qs) SendMessage(m_hwndLibrary,WM_ML_IPC,(WPARAM)m_qs,ML_IPC_DB_FREEQUERYRESULTS);
  m_qs=0;
}

#define SKIP_THE_AND_WHITESPACE(x) { while (!isalnum(*x) && *x) x++; if (!strnicmp(x,"the ",4)) x+=4; while (*x == ' ') x++; }
static int STRCMP_NULLOK(const char *pa, const char *pb)
{
  if (!pa) pa="";
  else SKIP_THE_AND_WHITESPACE(pa)

  if (!pb) pb="";
  else SKIP_THE_AND_WHITESPACE(pb)
  
  return stricmp(pa,pb);
}

int sortFunc(const void *elem1, const void *elem2)
{
  itemRecord *a=(itemRecord*)elem1;
  itemRecord *b=(itemRecord*)elem2;

  int use_by=0;

#define RETIFNZ(v) if ((v)<0) return -1; if ((v)>0) return 1;

  // this might be too slow, but it'd be nice
  int x;
  for (x = 0; x < 3; x ++)
  {
    if (use_by == 0) // artist -> album -> track -> title
    {
      int v=STRCMP_NULLOK(a->artist,b->artist);
      RETIFNZ(v)
      use_by=1;
    }
    else if (use_by == 1) // album -> track -> title -> artist
    {
      int v=STRCMP_NULLOK(a->album,b->album);
      RETIFNZ(v)
      use_by=2;
    }
    else if (use_by == 2) // track -> title -> artist -> album
    {
      int v1=a->track;
      int v2=b->track;
      if (v1<0)v1=0;
      if (v2<0)v2=0;
      RETIFNZ(v1-v2)
      use_by=3;     
    }
    else if (use_by == 3) // title -> artist -> album -> track
    {
      int v=STRCMP_NULLOK(a->title,b->title);
      RETIFNZ(v)
      use_by=0;
    }
    else break; // no sort order?
  } 
#undef RETIFNZ
  return 0;
}

static int sortFunc_artist(const void *elem1, const void *elem2)
{
  itemRecord *a=(itemRecord*)elem1;
  itemRecord *b=(itemRecord*)elem2;

  return STRCMP_NULLOK(a->artist,b->artist);
}


char *get_artist_list(HWND hwndLib)
{
  static char *lastlist;
  static time_t last_upd;

  time_t now;
  time(&now);

  if (!lastlist || now > last_upd + 15*60) //15mn rebuild of artist index
  {
    last_upd=now;
    if (lastlist) GlobalFree((HGLOBAL)lastlist);
    int lastlist_alloc=32768;
    lastlist=(char*)GlobalAlloc(GMEM_FIXED,lastlist_alloc);
    int lastlist_pos=0;
    lastlist[0]=lastlist[1]=0;
  
    mlQueryStruct qs={0,};
    qs.query="artist != \"\" & !artist isempty";
    SendMessage(hwndLib,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_RUNQUERY);
    if (qs.results.Size>0)
    {
      char *lastartist="";
      if (qs.results.Size>1) qsort(qs.results.Items,qs.results.Size,sizeof(itemRecord),sortFunc_artist);
      int x;
      for (x = 0; x < qs.results.Size; x ++)
      {
        char *thisartist = qs.results.Items[x].artist;
        if (thisartist && STRCMP_NULLOK(lastartist,thisartist))
        {
          if (*thisartist)
          {
            int l=strlen(thisartist);
            if (lastlist_alloc < lastlist_pos + l + 2)
            {
              lastlist_alloc = (lastlist_pos + l + 2) + 32768;
              char *newlist=(char*)GlobalAlloc(GMEM_FIXED,lastlist_alloc);
              if (newlist && lastlist)
              {
                memcpy(newlist,lastlist,lastlist_pos);
                GlobalFree(lastlist);
                lastlist=newlist;
              }
              //if (!lastlist) return 0;
            }
            memcpy(lastlist+lastlist_pos,thisartist,l+1);
            lastlist_pos += l+1;
            lastlist[lastlist_pos]=0;
            lastartist=thisartist;
          }
        }       
      }
    }
    SendMessage(hwndLib,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_FREEQUERYRESULTS);
  }

  if (lastlist && *lastlist) return lastlist;
  return NULL;
}


MLQueryGenerator::MLQueryGenerator(HWND hwndLibrary, mlQueryStruct *qs, char *qstr, int is_right, int allow_control, int allow_stream)
{
  m_lastartist=0;
  m_lastalbum=0;
  m_rowstate=0;
  m_isadmin=allow_control&2;
  m_iscontrol=is_right;
  m_pagename=m_iscontrol?"right.html":"/";
  m_canstream=allow_stream;
  m_hwndLibrary=hwndLibrary;
  item_pos=-1;
  m_qs=qs;
  curbuf_pos=0;
  curbuf.Set("<HTML><HEAD>" CSS_STR);
  if (!is_right) curbuf.Append("<TITLE>Winamp Library Streaming Interface</TITLE>");
  curbuf.Append("</HEAD><BODY>");
  curbuf_len=0;

  HWND hwndWinamp = FindWindow("Winamp v1.x",NULL); 

  GayString buff, css, index, path((char*)SendMessage(hwndWinamp,WM_WA_IPC,0,IPC_GETINIFILE));
  path.Replace("Winamp.ini", "");
  path.Append("plugins\\ml_www\\");
  path.Append("templates\\070.css");

  buff.SetWithFile(path.Get());
  css.Set("<style type=\"text/css\">\n");
  css.Append(buff.Get());
  css.Append("\n</style>");
  
  index.Set("<!doctype html public \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n<html>\n<head>\n<title> ml_www </title>\n<SCRIPT LANGUAGE=\"JavaScript\">\n<!-- Begin\n\nfunction ArtistDropdownGo(page, query)\n{\nif(query != \" \" && query != \"\")\n{\nwindow.location=page+'?query='+query;\n}else{\nwindow.location=page;\n}\n}\n\n// End -->\n</script>\n</head>\n\n<body>\n");
  index.Append(css.Get());
  index.Append("</head>\n\n<body><div id=\"container\">\n");

  curbuf.Set(index.Get());
  curbuf_len = strlen(curbuf.Get());

  char *artistlist=get_artist_list(hwndLibrary);
  char buf[1024];
  
  GayString dropdown;
  if (artistlist && *artistlist) 
  {
    sprintf(buf, "<div id=\"artistdropdown\"><form onSubmit=\"ArtistDropdownGo('%s', document.artistdropdown.value);\">\n<select name=\"query\" onChange=\"ArtistDropdownGo('%s', this.value);\">\n<option value=\"\">Home</option>\n;\">\n<option value=\"\">Browse by Artist</option>\n;\">", m_pagename, m_pagename);
    curbuf.Append(buf);
    curbuf_len = strlen(curbuf.Get());
        
    while (*artistlist)
    {
      bool is_sel = false;
      char artist_encoded[512];
      sprintf(buf,"?artist LIKE \"%.200s\"",artistlist);
      if(!strcmp((m_qs) ? m_qs->query : (char*)"", buf)) is_sel = true;
      WebServerBaseClass::url_encode(buf,artist_encoded,sizeof(artist_encoded));
          
      if(is_sel)
      {
        sprintf(buf, "<option value=\"%s\" selected>--%s</option>\n", artist_encoded, artistlist);
       }else{
        sprintf(buf, "<option value=\"%s\">--%s</option>\n", artist_encoded, artistlist);
      }
      curbuf.Append(buf);
      curbuf_len = strlen(curbuf.Get());
      artistlist+=strlen(artistlist)+1;
    }
    curbuf.Append("</select>&nbsp;<input type=\"submit\" value=\"view\">\n</form></div>");
    curbuf_len = strlen(curbuf.Get());
  }

  if (allow_control && allow_stream)
  {
    if (!is_right)
      curbuf.Append("<div id=\"toplinks\">[Stream Mode] [<a href=\"/ctrl\">Remote Control Mode</a>]</div>");
    else
      curbuf.Append("<div id=\"toplinks\">[<a href=\"/\" target=\"_top\">Stream Mode</a>] [Remote Control Mode]</div>");
  }

  curbuf_len=sprintf(buf,"<div id=\"searchform\"><form method=url action=\"%s\">"
                  "<input type=text class=editclass name=query width=50>&nbsp;&nbsp;&nbsp;"
                  "<input type=submit class=buttonclass value=\"search\">",m_pagename);
  curbuf.Append(buf);

  if (m_qs)
  {
    wsprintf(buf," [<a href=\"%s\">Browse by Artist</a>]",m_pagename);
    curbuf.Append(buf);
  }
  curbuf.Append("</Form></div>");

  if (m_qs)
  {
    sprintf(buf,"<div id=\"queryresults\">Found %d%s results:"
                ,m_qs->results.Size,m_qs->results.Size == m_qs->max_results ? "+" : "");
    curbuf.Append(buf);
    if (m_qs->results.Size>1)
    {
      // sort results
      qsort(m_qs->results.Items,m_qs->results.Size,sizeof(itemRecord),sortFunc);
    }
  }
  else
  {
    curbuf.Append("<div id=\"intropage\"></div>");
  }
  curbuf_len=strlen(curbuf.Get());
}

int MLQueryGenerator::GetData(char *buf, int size) // return 0 when done
{
  if (size<1) return 0;

  int addl=curbuf_len-curbuf_pos;
  if (addl > size) addl=size;
  if (addl) memcpy(buf,curbuf.Get()+curbuf_pos,addl);
  curbuf_pos += addl;
  buf += addl;
  size -= addl;
  if (curbuf_pos >= curbuf_len)
  {
    int len=m_qs?m_qs->results.Size:0;

    curbuf_pos=curbuf_len=0;
    item_pos++;
    if (item_pos>len) return addl;

    if (item_pos == len)
    {
      if (m_qs && m_rowstate) curbuf.Append("</table></div>");
      else curbuf.Set("");
      curbuf.Append("</body></html>");
      curbuf_len=strlen(curbuf.Get()); 
      return addl+GetData(buf,size);
    }
    else
    {

      curbuf_pos=0;
      curbuf.Set("");

      if (m_qs) 
      {
        char bf[2048];
        if ((STRCMP_NULLOK(m_lastartist,m_qs->results.Items[item_pos].artist) ||
            STRCMP_NULLOK(m_lastalbum,m_qs->results.Items[item_pos].album)) &&
            
            (
            m_qs->results.Items[item_pos].artist||
            m_qs->results.Items[item_pos].album           
            ))
        {
          m_lastartist=m_qs->results.Items[item_pos].artist;
          m_lastalbum=m_qs->results.Items[item_pos].album;

          // show new artist/album row
          char artist_encoded[256];
          char album_encoded[512];
          if (m_qs->results.Items[item_pos].artist)
          {
            sprintf(bf,"?artist LIKE \"%.200s\"",m_qs->results.Items[item_pos].artist);
            WebServerBaseClass::url_encode(bf,artist_encoded,sizeof(artist_encoded));
          }
          if (m_qs->results.Items[item_pos].album)
          {
            sprintf(bf,"?album LIKE \"%.200s\"",m_qs->results.Items[item_pos].album);

            if (m_qs->results.Items[item_pos].artist) 
              sprintf(bf+strlen(bf)," & artist LIKE \"%.200s\"",m_qs->results.Items[item_pos].artist);

            WebServerBaseClass::url_encode(bf,album_encoded,sizeof(album_encoded));
          }

          if (m_rowstate) 
          {
            m_rowstate=2;
            curbuf.Append("</table><BR>");
          }

          sprintf(bf,"<table width=100%%><tr class=rowclass0><td colspan=2 nowrap>",1+(m_rowstate&1));
          curbuf.Append(bf);
          if (m_qs->results.Items[item_pos].artist && *m_qs->results.Items[item_pos].artist)
          {
            sprintf(bf,"Artist: <a href=\"%s?query=%s\">%.200s</a>",
                m_pagename,artist_encoded,m_qs->results.Items[item_pos].artist);
            curbuf.Append(bf);
            if (m_iscontrol)
            {
              if (m_isadmin)
              {
                sprintf(bf," [<a target=wwLeft href=\"left.html?enqueue=q&query=%s&play=1\">Play Artist</a>] ",artist_encoded);
                curbuf.Append(bf);
              }
              sprintf(bf," [<a target=wwLeft href=\"left.html?enqueue=q&query=%s\">Enqueue Artist</a>]",artist_encoded);
              curbuf.Append(bf);
            }
            if (m_canstream)
            {
              sprintf(bf," [<a href=\"stream.m3u?query=%s\">Stream Artist</a>]",artist_encoded);
              curbuf.Append(bf);
            }
            curbuf.Append("<BR>");
          }
          if (m_qs->results.Items[item_pos].artist && *m_qs->results.Items[item_pos].artist)
          {
            sprintf(bf,"Album: <a href=\"%s?query=%s\">%.200s</a>",
                m_pagename,album_encoded,m_qs->results.Items[item_pos].album);
            curbuf.Append(bf);
            if (m_iscontrol)
            {
              if (m_isadmin)
              {
                sprintf(bf," [<a target=wwLeft href=\"left.html?enqueue=q&query=%s&play=1\">Play Album</a>] ",album_encoded);
                curbuf.Append(bf);
              }
              sprintf(bf," [<a target=wwLeft href=\"left.html?enqueue=q&query=%s\">Enqueue Album</a>]",album_encoded);
              curbuf.Append(bf);
            }
            if (m_canstream)
            {
              sprintf(bf," [<a href=\"stream.m3u?query=%s\">Stream Album</a>]",album_encoded);
              curbuf.Append(bf);
            }

            curbuf.Append("<BR>");
          }
          curbuf.Append("</td></tr>");
          m_rowstate++;

        }
       
        sprintf(bf,"<tr class=rowclass%d><td nowrap>%.200s",
            1+(m_rowstate&1),m_qs->results.Items[item_pos].title?m_qs->results.Items[item_pos].title:"");
        m_rowstate++;

        curbuf.Append(bf);

        char *p=getRecordExtendedItem(m_qs->results.Items+item_pos,"DBIDX");
        if (p && *p)
        {
          char *ep;
          unsigned int idx=strtoul(p,&ep,10);
          unsigned int hashidx=crc32str(m_qs->results.Items[item_pos].filename);

          curbuf.Append(" ");

          if (m_iscontrol)
          {
            if (m_isadmin)
            {
              sprintf(bf,"[<a target=wwLeft href=\"left.html?enqueue=%u_%u&play=1\">Play</a>] ",idx,hashidx);
              curbuf.Append(bf);
            }

            sprintf(bf,"[<a target=wwLeft href=\"left.html?enqueue=%u_%u\">Enqueue</a>]", idx,hashidx);
            curbuf.Append(bf);
          }

          if (m_canstream)
          {
            sprintf(bf,"%s[<a href=\"stream.m3u?idx=%u_%u\">Stream</a>]",m_iscontrol?" ":"",idx,hashidx);
            curbuf.Append(bf);

            char *fne = m_qs->results.Items[item_pos].filename;
            while (*fne) fne++;
            while (fne >= m_qs->results.Items[item_pos].filename && *fne != '/' && *fne != '\\') fne--; 

            char b[1024],b2[2048];
            WebServerBaseClass::url_encode(++fne,b,sizeof(b));

            sprintf(b2," [<a href=\"fetch-dl?idx=%u_%u&fn=%s\">Download</a>]",idx,hashidx, b);
            curbuf.Append(b2);
          }
        }

        curbuf.Append("</TD><TD nowrap>");

        if (m_qs->results.Items[item_pos].track>0 || m_qs->results.Items[item_pos].length >= 0)
        {
          if (m_qs->results.Items[item_pos].track>0)
          {
            char b[512];
            sprintf(b,"Track %d",m_qs->results.Items[item_pos].track);
            curbuf.Append(b);
            if (m_qs->results.Items[item_pos].length >= 0)
              curbuf.Append(" ");
          }
          if (m_qs->results.Items[item_pos].length >= 0)
          {
            char b[512];
            sprintf(b,"[%d:%02d]",m_qs->results.Items[item_pos].length/60,m_qs->results.Items[item_pos].length%60);
            curbuf.Append(b);
          }
        }
        curbuf.Append("</TD></tR>\n");
      }
      curbuf_len=strlen(curbuf.Get());

      return addl+GetData(buf,size);
    }
  }
  return addl;
}


FilePageGenerator::FilePageGenerator(char *fn, JNL_HTTPServ *serv)
{
  m_file_pos=m_file_len=0;
  m_fp=fopen(fn,"rb");
  int resume_end=0;
  int resume_pos=0;
  if (m_fp)
  {
    char *range = serv->getheader("Range");
    if (range)
    {
      if (!strnicmp(range,"bytes=",6))
      {
        range+=6;
        char *t=range;
        while ((*t < '0' || *t  > '9') && *t) t++;
        while (*t >= '0' && *t <= '9') 
        {
          resume_pos*=10;
          resume_pos+=*t-'0';
          t++;
        }
        if (*t != '-') resume_pos=0;
        else if (t[1])
        {
          resume_end=atoi(t+1);
        }
      }
    }

    fseek(m_fp,0,SEEK_END);
    m_file_len=ftell(m_fp);
    char buf[512];
    wsprintf(buf,"Content-Length: %d",m_file_len);
    serv->set_reply_header(buf);

    int m_file_len_orig=m_file_len;

    if (resume_end && resume_end < m_file_len) m_file_len=resume_end;
    if (resume_pos > 0 && resume_pos < m_file_len) m_file_pos = resume_pos;
    fseek(m_fp,m_file_pos,SEEK_SET);

    wsprintf(buf,"Content-Range: bytes=%d-%d/%d",resume_pos,resume_end,m_file_len_orig);
    serv->set_reply_header(buf);
  }
}

FilePageGenerator::~FilePageGenerator()
{
  if (m_fp) fclose(m_fp);
}

int FilePageGenerator::GetData(char *buf, int size) // return 0 when done
{
  if (!m_fp) return 0;
  if (m_file_pos+size > m_file_len) size=m_file_len - m_file_pos;
  int l=fread(buf,1,size,m_fp);
  m_file_pos+=l;
  return l;
}