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
#include <windowsx.h>
#include <stdio.h>
#include "../../gen_ml/ml.h"
#include "../../gen_ml/listview.h"
#include "../../gen_ml/itemlist.h"
#include "../../winamp/wa_ipc.h"
#include "../../jnetlib/jnetlib.h"
#include "../../jnetlib/webserver.h"
#include "../../gen_ml/gaystring.h"
#include "../../winamp/ipc_pe.h"
#include "pagegen.h"
#include "wwwutil.h"
#include "resource.h"


#define STYLE_TEXT

#define VERSION "0.41a"

extern winampMediaLibraryPlugin plugin;
static HWND m_workwnd;

extern int sortFunc(const void *elem1, const void *elem2);
void strreplace(char* source, char* replace, char* with);
int ifindstr(int startx,char* body,char* search);
void leftcopy(char* input,char* output,int pos);
void rightcopy(char* input,char* output,int pos);


int g_config_max_cons=30;
char g_config_port_str[128]="80";
int g_config_query_max=200;
int g_config_enable=0;
int g_config_do_auth=1;
int g_config_refresh=5;

int request_is_image(char *req)
{
  char * buf = strdup(req);
  char * acceptable_ext[4] = {"JPG", "JPEG", "PNG", "GIF"};
  /*for(unsigned int i = 0; i <= strlen(buf) - 1; i++)
  {
    *buf++;
  }
  if(*--buf != '/') *buf++;*/
  while(*buf != '.' && *buf != '\0')
  {
    *buf++;
  }
  *buf++;
  int is_image = 0;
  for(int i = 0; i < 4; i++)
  {
    if(!_strnicmp(buf, acceptable_ext[i], 3)) is_image = 1;
  }
  return(is_image);
}

char * request_get_ext(char * req)
{
  char * buf = strdup(req);
  while(*buf != '.' && *buf != '\0')
  {
    *buf++;
  }
  *buf++;
  return(buf);
}

struct
{
  int allow; // not used
  char user[64],pass[64]; // if these are empty, dont require auth
  int control;//2 for admin, 1 for minor
  int stream;
} g_config_userInfo[4];

class mlwwwServer : public WebServerBaseClass
{
public:
  mlwwwServer() { } 
  virtual IPageGenerator *onConnection(JNL_HTTPServ *serv, int port);

};


IPageGenerator *mlwwwServer::onConnection(JNL_HTTPServ *serv, int port)
{
  serv->set_reply_header("Server: ml_www/" VERSION);
  serv->set_reply_header("Connection: close");

  IPageGenerator *pagegen=0;
  char *req=serv->get_request_file();
  char authbuf[512];

  if (!strcmp(req,"/style.css"))
  {
    char fne[1024];
    GetModuleFileName(plugin.hDllInstance,fne,sizeof(fne)-32);
    strcat(fne,".css");
    serv->set_reply_string("HTTP/1.1 200 OK");

    pagegen=new FilePageGenerator(fne,serv);
    if (((FilePageGenerator*)pagegen)->is_error())
    {
      delete pagegen;
      pagegen=0;
    }
    serv->set_reply_header("Content-Type: text/css");
    serv->send_reply();
    return pagegen;

  }

  int is_ctrl=!strcmp(req,"/ctrl");
  int is_left=!strcmp(req,"/left.html");
  int is_right=!strcmp(req,"/right.html");
  int is_index=!strcmp(req,"/")||!strcmp(req,"/index.html");
  int is_streampls=!strcmp(req,"/stream.m3u");
  int is_fetch=!strcmp(req,"/fetch");
  if (!is_fetch) is_fetch=strcmp(req,"/fetch-dl")?0:2;
  int is_image = request_is_image(req);

  

  if (is_image)
  {
    char *fne = (char*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIDIRECTORY);
    strcat(fne, (char*)"\\plugins\\ml_www");
    strreplace(req, (char*)"/", (char*)"\\");
    strcat(fne, req);
    

    serv->set_reply_string("HTTP/1.1 200 OK");

    pagegen=new FilePageGenerator(fne,serv);
    if (((FilePageGenerator*)pagegen)->is_error())
    {
      delete pagegen;
      pagegen=0;
    }
    char buf[1024] = "Content-Type: image/";
    strcat(buf, request_get_ext(req));
    serv->set_reply_header(buf);
    serv->send_reply();
    return pagegen;
  }
  // we allow streaming without pass, just require the other info to be correct

  int allow_control=0;
  int allow_stream=0;

  if (parseAuth(serv->getheader("Authorization"),authbuf,sizeof(authbuf)) != 1) strcpy(authbuf,":");

  if(g_config_do_auth)
  {
    int x;
    for (x = 0; x < sizeof(g_config_userInfo)/sizeof(g_config_userInfo[0]); x ++)
    {
      char buf[256];
      wsprintf(buf,"%s:%s",g_config_userInfo[x].user,g_config_userInfo[x].pass);
      if (!strcmp(authbuf,buf))
      {
        allow_control = g_config_userInfo[x].control;
        allow_stream = g_config_userInfo[x].stream;      
        break;
      }
    }
  }
  else
  {
    allow_control=1;
    allow_stream=1;
  }


  {
    int authed=0;
    if (is_streampls || is_fetch) authed=allow_stream;
    else if (is_ctrl || is_left || is_right) authed=allow_control;
    else if (is_index)
    {
      if (allow_stream)
      {
        authed=1;
      }
      else if (allow_control) // make index.html show /ctrl if no streaming
      {
        is_ctrl=1;
        is_index=0;
        authed=1;
      }
    }
    else 
    {
      serv->set_reply_string("HTTP/1.1 404 Not Found");
      serv->send_reply();
      return 0;
    }

    if (!authed)
    {
      serv->set_reply_string("HTTP/1.1 401 Authorization Required");
      serv->set_reply_header("WWW-Authenticate: Basic Realm=\"ml_www\"");
      serv->send_reply();
      return 0;
    }
  }

  serv->set_reply_string("HTTP/1.1 200 OK");

  if (is_fetch)
  {
    char *idx=serv->get_request_parm("idx");
    char *fn=serv->get_request_parm("fn");
    if (idx && fn)
    {
      char *oidx=idx;
      char *hash;
      char clientfne[1024];
      char buf[512];
      url_decode(idx,buf,sizeof(buf));
      url_decode(fn,clientfne,sizeof(clientfne));
      idx=buf;

      if ((hash=strstr(idx,"_")))
      {
        unsigned int ihash=strtoul(hash+1,&hash,10);

        mlQueryStruct qs={0,};
        qs.query=(char *)strtoul(idx,&idx,10);
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_RUNQUERY_INDEX);
        if (qs.results.Size>0 && crc32str(qs.results.Items[0].filename) == ihash)
        {
          char *fne = qs.results.Items[0].filename;
          while (*fne) fne++;
          while (fne >= qs.results.Items[0].filename && *fne != '/' && *fne != '\\') fne--; 
          if (!strcmp(++fne,clientfne))
          {
            pagegen=new FilePageGenerator(qs.results.Items[0].filename,serv);
            if (((FilePageGenerator*)pagegen)->is_error())
            {
              delete pagegen;
              pagegen=0;
              serv->set_reply_string("HTTP/1.1 500 SERVER ERROR");
            }
            else
            {
              char buf[1024];
              char *filename=fne;
              while (*fne != '.' && *fne) fne++;
              if (*fne) fne++;
              else fne = "unknown";
              char *typep=getRecordExtendedItem(qs.results.Items,"TYPE");
              wsprintf(buf,"Content-Type: %s/%.10s", typep && atoi(typep)?"video":"audio",fne); // set our content type to the extension
              serv->set_reply_header(buf);
              
              if (is_fetch!=2 && qs.results.Items[0].artist && qs.results.Items[0].title)
                wsprintf(buf,"Content-Disposition: attachment; filename=%.300s - %.300s",qs.results.Items[0].artist,qs.results.Items[0].title);
              else 
                wsprintf(buf,"Content-Disposition: attachment; filename=%.500s",filename);
              serv->set_reply_header(buf);
            }
          }
          else 
          {
            serv->set_reply_string("HTTP/1.1 404 Not Found");
          }
        }
        else serv->set_reply_string("HTTP/1.1 404 Not Found");
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_FREEQUERYRESULTS);
      }
      else serv->set_reply_string("HTTP/1.1 404 Not Found");
    }      
  }
  else if (is_streampls)
  {
    char *idx=serv->get_request_parm("idx");
    if (idx)
    {
      char *oidx=idx;
      char *hash;
      char buf[512];
      url_decode(idx,buf,sizeof(buf));
      idx=buf;

      if ((hash=strstr(idx,"_")))
      {
        unsigned int ihash=strtoul(hash+1,&hash,10);

        mlQueryStruct qs={0,};
        qs.query=(char *)strtoul(idx,&idx,10);
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_RUNQUERY_INDEX);
        if (qs.results.Size>0 && crc32str(qs.results.Items[0].filename) == ihash)
        {
          char buf[1024];
          GayString gs;
          char *fne = qs.results.Items[0].filename;
          while (*fne) fne++;
          while (fne >= qs.results.Items[0].filename && *fne != '/' && *fne != '\\') fne--; 
          fne++;

          sprintf(buf,"#EXTM3U\n"
                       "#EXTINF:%d, ",qs.results.Items[0].length);
          gs.Set(buf);
          if (qs.results.Items[0].artist && qs.results.Items[0].title)
          {
            gs.Append(qs.results.Items[0].artist);
            gs.Append(" - ");
            gs.Append(qs.results.Items[0].title);
          }
          else
            gs.Append(fne);
          gs.Append("\nhttp://");
          if (strcmp(authbuf,":")) // auth
          {
            gs.Append(authbuf);
            gs.Append("@");
          }
          char *host=serv->getheader("host");
          gs.Append(host&&*host ? host : "localhost");
          if (!host || !strstr(host,":"))
          {
            sprintf(buf,":%d",port);
            gs.Append(buf);
          }
          sprintf(buf,"/fetch?idx=%.64s&fn=",oidx);
          gs.Append(buf);
          url_encode(fne,buf,sizeof(buf));
          gs.Append(buf);
          gs.Append("\n");
          // file found, generate playlist
          pagegen=new MemPageGenerator(strdup(gs.Get()));                         
          serv->set_reply_header("Content-Type: audio/x-mpegurl");
        }
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_FREEQUERYRESULTS);
      }
    }  
    else if ((idx = serv->get_request_parm("query")))
    {
      char buf[512];
      url_decode(idx,buf,sizeof(buf));

      mlQueryStruct qs={0,};
      qs.max_results=g_config_query_max;
      qs.query=buf;
      SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_RUNQUERY_SEARCH);
      if (qs.results.Size>0)
      {
        if (qs.results.Size>1)
          qsort(qs.results.Items,qs.results.Size,sizeof(itemRecord),sortFunc);

        GayString gs;
        int x;
        gs.Set("#EXTM3U\n");

        for (x = 0; x < qs.results.Size; x ++)
        {
          char buf[1024];
          char *fne = qs.results.Items[x].filename;
          while (*fne) fne++;
          while (fne >= qs.results.Items[x].filename && *fne != '/' && *fne != '\\') fne--; 
          fne++;

          sprintf(buf,"#EXTINF:%d, ",qs.results.Items[x].length);
          gs.Append(buf);
          if (qs.results.Items[x].artist && qs.results.Items[x].title)
          {
            gs.Append(qs.results.Items[x].artist);
            gs.Append(" - ");
            gs.Append(qs.results.Items[x].title);
          }
          else gs.Append(fne);
          gs.Append("\nhttp://");
          if (strcmp(authbuf,":")) // auth
          {
            gs.Append(authbuf);
            gs.Append("@");
          }
          char *host=serv->getheader("host");
          gs.Append(host&&*host ? host : "localhost");
          if (!host || !strstr(host,":"))
          {
            sprintf(buf,":%d",port);
            gs.Append(buf);
          }

          char *p=getRecordExtendedItem(qs.results.Items+x,"DBIDX");
          unsigned int dbidx=p ? atoi(p) : 0;
          unsigned int hashidx=crc32str(qs.results.Items[x].filename);

          sprintf(buf,"/fetch?idx=%u_%u&fn=",dbidx,hashidx);
          gs.Append(buf);

          url_encode(fne,buf,sizeof(buf));
          gs.Append(buf);
          gs.Append("\n");
        }
        // file found, generate playlist
        pagegen=new MemPageGenerator(strdup(gs.Get()));                         
        serv->set_reply_header("Content-Type: audio/x-mpegurl");
      }
      SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_FREEQUERYRESULTS);
    }
  }
  else if (is_ctrl)
  {
    char *maintext="<HTML><HEAD><TITLE>Winamp Remote Control</TITLE>" CSS_STR "</HEAD>"
      "<FRAMESET COLS=\"45%%,*\" CLASS=FSCLASS>"
      "<FRAME SRC=\"/left.html\" NAME=\"wwLeft\" CLASS=FSLCLASS>"
      "<FRAME SRC=\"/right.html\" NAME=\"wwRight\" CLASS=FSRCLASS>"
      "</FRAMESET>"
  	  "</HTML>";
    pagegen=new MemPageGenerator(strdup(maintext));
    serv->set_reply_header("Content-Type: text/html");
  }
  else if (is_left)
  {
    char *parm=serv->get_request_parm("enqueue");

    if (parm)
    {
      char *play=serv->get_request_parm("play");
      char *hash;
      char buf[512];
      url_decode(parm,buf,sizeof(buf));
      char *quer;
      char *enqueue=buf;
      int isplay=0;
      if (play && atoi(play)) isplay=1;
      if (!(allow_control & 2) && isplay) isplay=0;

      if (!strcmp(enqueue,"q") && (quer = serv->get_request_parm("query")))
      {
        url_decode(quer,buf,sizeof(buf));
        mlQueryStruct qs={0,};
        qs.query=buf;
        qs.max_results=g_config_query_max;

        // ML_IPC_DB_RUNQUERY_SEARCH will automatically turn ? into a query :)
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_RUNQUERY_SEARCH);
        if (qs.results.Size>0)
        {
          if (qs.results.Size>1)
            qsort(qs.results.Items,qs.results.Size,sizeof(itemRecord),sortFunc);
          int l=isplay ? SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETLISTLENGTH): 0;
          mlSendToWinampStruct ws={ML_TYPE_ITEMRECORDLIST,(void*)&qs.results,1};
          SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&ws,ML_IPC_SENDTOWINAMP);
          if (isplay)
          {
            SendMessage(plugin.hwndWinampParent,WM_WA_IPC,l,IPC_SETPLAYLISTPOS);
            SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); // stop button, literally
            SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); // play button, literally
          }
        }
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_FREEQUERYRESULTS);


      }
      else if ((hash=strstr(enqueue,"_")))
      {
        unsigned int ihash=strtoul(hash+1,&hash,10);

        mlQueryStruct qs={0,};
        qs.query=(char *)strtoul(enqueue,&enqueue,10);
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_RUNQUERY_INDEX);
        if (qs.results.Size>0 && crc32str(qs.results.Items[0].filename) == ihash)
        {
          int l=isplay ? SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETLISTLENGTH): 0;
          mlSendToWinampStruct ws={ML_TYPE_ITEMRECORDLIST,(void*)&qs.results,1};
          SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&ws,ML_IPC_SENDTOWINAMP);
          if (isplay)
          {
            SendMessage(plugin.hwndWinampParent,WM_WA_IPC,l,IPC_SETPLAYLISTPOS);
            SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); // stop button, literally
            SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); // play button, literally
          }
        }
        SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&qs,ML_IPC_DB_FREEQUERYRESULTS);
      }
    }
    else if (allow_control&2) // admin actions
    {
      if ((parm = serv->get_request_parm("shuffle")))
      {
        SendMessage(plugin.hwndWinampParent,WM_WA_IPC,!!atoi(parm),IPC_SET_SHUFFLE);
      }
      else if ((parm = serv->get_request_parm("repeat")))
      {
        SendMessage(plugin.hwndWinampParent,WM_WA_IPC,!!atoi(parm),IPC_SET_REPEAT);
      }
      else if ((parm = serv->get_request_parm("remove")))
      {
        if (!strcmp(parm,"all"))
        {
          SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
        }
        else
        {
          int a=atoi(parm);
          if (a >= 0 && a < SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETLISTLENGTH))
          {
            HWND peWnd = (HWND)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,IPC_GETWND_PE,IPC_GETWND);
            if (peWnd) SendMessage(peWnd,WM_WA_IPC,IPC_PE_DELETEINDEX,a);
          }
        }
      }
      else if ((parm = serv->get_request_parm("jump")))
      {
        int a=atoi(parm);
        if (a >= 0 && a < SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETLISTLENGTH))
        {
          SendMessage(plugin.hwndWinampParent,WM_WA_IPC,a,IPC_SETPLAYLISTPOS);
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); // stop button, literally
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); // play button, literally
        }
      }
      else if ((parm = serv->get_request_parm("action")))
      {
        if (!strcmp(parm,"prev"))
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40044,0);
        else if (!strcmp(parm,"play"))
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0);
        else if (!strcmp(parm,"pause"))
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40046,0);
        else if (!strcmp(parm,"stop"))
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0);
        else if (!strcmp(parm,"next"))
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40048,0);
        else if (!strcmp(parm,"f15s"))
        {
          int x; for (x=0; x<3;x++) SendMessage(plugin.hwndWinampParent,WM_COMMAND,40060,0);          
        }
        else if (!strcmp(parm,"b15s"))
        {
          int x; for (x=0; x<3;x++) SendMessage(plugin.hwndWinampParent,WM_COMMAND,40061,0);
        }
      }      
      else if ((parm = serv->get_request_parm("vis")))
      {
        int a=atoi(parm);
        if (!a)
        {
          if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_ISVISRUNNING))
          {
            HWND hExternalVisWindow=(HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVISWND);
            if (hExternalVisWindow) SendMessage(hExternalVisWindow,WM_USER+1666,1,15);
            SendMessage(plugin.hwndWinampParent,WM_COMMAND,40192,0);//toggle vis
          }
        }
        else
        {
          if (!SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_ISVISRUNNING))
          {
            SendMessage(plugin.hwndWinampParent,WM_COMMAND,40192,0);//toggle vis
            if (a>1)
            {
              KillTimer(m_workwnd,2);
              SetTimer(m_workwnd,2,500,NULL);
            }
          }
          else if (a>1)
          {
            PostMessage(plugin.hwndWinampParent, WM_COMMAND, ID_VIS_FS, 0);
          }
        }
      }
      else if ((parm = serv->get_request_parm("vidfs")))
      {
        if (atoi(parm))
        {
          // stop vis if running
          if (SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_ISVISRUNNING))
          {
            HWND hExternalVisWindow=(HWND)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVISWND);
            if (hExternalVisWindow) SendMessage(hExternalVisWindow,WM_USER+1666,1,15);
            SendMessage(plugin.hwndWinampParent,WM_COMMAND,40192,0);//toggle vis
          }
          SendMessage(plugin.hwndWinampParent,WM_COMMAND,40337,0);//toggle video fullscreen
        }
      }
    }
    
    pagegen=new WAPlaylistGenerator(plugin.hwndWinampParent,allow_control&2);
    serv->set_reply_header("Refresh: 5; URL=/left.html");
    serv->set_reply_header("Content-Type: text/html");
  }
  else if (is_right||is_index)
  {
    mlQueryStruct *qs=NULL;
    char *p=serv->get_request_parm("query");
    char *the_qstr=p;
    if (p && *p)
    {
      char buf[512];
      url_decode(p,buf,sizeof(buf));

      qs=(mlQueryStruct*)calloc(1,sizeof(mlQueryStruct));
      qs->query = buf;
      qs->max_results = g_config_query_max;
      SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)qs,ML_IPC_DB_RUNQUERY_SEARCH);
    }
    pagegen=new MLQueryGenerator(plugin.hwndLibraryParent,qs,the_qstr,is_right,allow_control,allow_stream);
    serv->set_reply_header("Content-Type: text/html");
  }

  serv->send_reply();

  return pagegen;
}

LRESULT CALLBACK plugin_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

mlwwwServer *m_server;
char *conf_file;
static char *m_classname="ml_www";

void updateServerPorts()
{
  while (m_server->getListenPort(0))
    m_server->removeListenIdx(0);

  if (!g_config_enable) return;

  m_server->setMaxConnections(g_config_max_cons);

  char *p=g_config_port_str;
  while (*p == ',' || *p == ' ') p++;
  int poo;
  while ((poo=atoi(p)))
  {
    m_server->addListenPort(poo);
    p=strstr(p,",");
    if (!p) break;
    while (*p == ',' || *p == ' ') p++;
  }
}

int init() 
{
  conf_file=(char*)SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_GETINIFILE);

  GetPrivateProfileStruct(m_classname,"userinfo",g_config_userInfo,sizeof(g_config_userInfo),conf_file);
  GetPrivateProfileString(m_classname,"port","80",g_config_port_str,sizeof(g_config_port_str),conf_file);
  g_config_max_cons=GetPrivateProfileInt(m_classname,"max_cons",g_config_max_cons,conf_file);
  g_config_enable=GetPrivateProfileInt(m_classname,"enabled",g_config_enable,conf_file);
  g_config_do_auth=GetPrivateProfileInt(m_classname,"do_auth",g_config_do_auth,conf_file);
  g_config_refresh=GetPrivateProfileInt(m_classname,"refresh",g_config_refresh,conf_file);
 
  JNL::open_socketlib();
  m_server = new mlwwwServer;

  updateServerPorts();

	WNDCLASS wc={0,};	
	wc.lpfnWndProc = plugin_wndproc;
	wc.hInstance = plugin.hDllInstance;
	wc.lpszClassName = m_classname;
	RegisterClass(&wc);
  m_workwnd=CreateWindowEx(0,m_classname,m_classname,0,0,0,0,0,NULL,NULL,plugin.hDllInstance,0);

  return 0;
}


void quit() 
{
  if (m_workwnd) DestroyWindow(m_workwnd);
  m_workwnd=0;
  delete m_server;
  UnregisterClass(m_classname,plugin.hDllInstance);
}

LRESULT CALLBACK plugin_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_CREATE:
      SetTimer(hwnd,1,50,NULL);
    return 0;
    case WM_TIMER:
      if (wParam == 1)
      {
        static int in_timer;
        if (!in_timer && g_config_enable)
        {
          in_timer=1;
          m_server->run();
          in_timer=0;
        }
      }
      if (wParam == 2)
      {
        KillTimer(hwnd,2);
        PostMessage(plugin.hwndWinampParent, WM_COMMAND, ID_VIS_FS, 0);
      }
    return 0;
    case WM_DESTROY:
      KillTimer(hwnd,1);
    return 0;
  }
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

static BOOL CALLBACK configDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      if (g_config_enable) CheckDlgButton(hwndDlg,IDC_ENABLE,BST_CHECKED);
      SetDlgItemText(hwndDlg,IDC_PORTS,g_config_port_str);
      SetDlgItemInt(hwndDlg,IDC_MAXCON,g_config_max_cons,FALSE);
      SetDlgItemInt(hwndDlg,IDC_MAXRES,g_config_query_max,FALSE);    
      SetDlgItemInt(hwndDlg,IDC_REFRESH,g_config_refresh,FALSE);

      SetDlgItemText(hwndDlg,IDC_USER_1,g_config_userInfo[0].user);
      SetDlgItemText(hwndDlg,IDC_USER_2,g_config_userInfo[1].user);
      SetDlgItemText(hwndDlg,IDC_USER_3,g_config_userInfo[2].user);
      SetDlgItemText(hwndDlg,IDC_USER_4,g_config_userInfo[3].user);
      SetDlgItemText(hwndDlg,IDC_PASS_1,g_config_userInfo[0].pass);
      SetDlgItemText(hwndDlg,IDC_PASS_2,g_config_userInfo[1].pass);
      SetDlgItemText(hwndDlg,IDC_PASS_3,g_config_userInfo[2].pass);
      SetDlgItemText(hwndDlg,IDC_PASS_4,g_config_userInfo[3].pass);
      if (g_config_do_auth) CheckDlgButton(hwndDlg,IDC_AUTHCHECK,BST_CHECKED);
      if (g_config_userInfo[0].control&2) CheckDlgButton(hwndDlg,IDC_ALLOW_1,BST_CHECKED);
      if (g_config_userInfo[1].control&2) CheckDlgButton(hwndDlg,IDC_ALLOW_2,BST_CHECKED);
      if (g_config_userInfo[2].control&2) CheckDlgButton(hwndDlg,IDC_ALLOW_3,BST_CHECKED);
      if (g_config_userInfo[3].control&2) CheckDlgButton(hwndDlg,IDC_ALLOW_4,BST_CHECKED);
      if (g_config_userInfo[0].control&1) CheckDlgButton(hwndDlg,IDC_CONTROL_1,BST_CHECKED);
      if (g_config_userInfo[1].control&1) CheckDlgButton(hwndDlg,IDC_CONTROL_2,BST_CHECKED);
      if (g_config_userInfo[2].control&1) CheckDlgButton(hwndDlg,IDC_CONTROL_3,BST_CHECKED);
      if (g_config_userInfo[3].control&1) CheckDlgButton(hwndDlg,IDC_CONTROL_4,BST_CHECKED);
      if (g_config_userInfo[0].stream) CheckDlgButton(hwndDlg,IDC_STREAM_1,BST_CHECKED);
      if (g_config_userInfo[1].stream) CheckDlgButton(hwndDlg,IDC_STREAM_2,BST_CHECKED);
      if (g_config_userInfo[2].stream) CheckDlgButton(hwndDlg,IDC_STREAM_3,BST_CHECKED);
      if (g_config_userInfo[3].stream) CheckDlgButton(hwndDlg,IDC_STREAM_4,BST_CHECKED);

    return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          g_config_enable = !!IsDlgButtonChecked(hwndDlg,IDC_ENABLE);
          g_config_do_auth = !!IsDlgButtonChecked(hwndDlg,IDC_AUTHCHECK);
          GetDlgItemText(hwndDlg,IDC_PORTS,g_config_port_str,sizeof(g_config_port_str));
          {
            BOOL t;
            int v=GetDlgItemInt(hwndDlg,IDC_MAXCON,&t,FALSE);
            if (t) g_config_max_cons=v;
            v=GetDlgItemInt(hwndDlg,IDC_MAXRES,&t,FALSE);
            if (t) g_config_query_max=v;
            v=GetDlgItemInt(hwndDlg,IDC_REFRESH,&t,FALSE);
            if (t) g_config_refresh=v;
          }
          g_config_userInfo[0].control=!!IsDlgButtonChecked(hwndDlg,IDC_CONTROL_1);
          g_config_userInfo[1].control=!!IsDlgButtonChecked(hwndDlg,IDC_CONTROL_2);
          g_config_userInfo[2].control=!!IsDlgButtonChecked(hwndDlg,IDC_CONTROL_3);
          g_config_userInfo[3].control=!!IsDlgButtonChecked(hwndDlg,IDC_CONTROL_4);
          g_config_userInfo[0].control|=IsDlgButtonChecked(hwndDlg,IDC_ALLOW_1)?2:0;
          g_config_userInfo[1].control|=IsDlgButtonChecked(hwndDlg,IDC_ALLOW_2)?2:0;
          g_config_userInfo[2].control|=IsDlgButtonChecked(hwndDlg,IDC_ALLOW_3)?2:0;
          g_config_userInfo[3].control|=IsDlgButtonChecked(hwndDlg,IDC_ALLOW_4)?2:0;
          g_config_userInfo[0].stream=!!IsDlgButtonChecked(hwndDlg,IDC_STREAM_1);
          g_config_userInfo[1].stream=!!IsDlgButtonChecked(hwndDlg,IDC_STREAM_2);
          g_config_userInfo[2].stream=!!IsDlgButtonChecked(hwndDlg,IDC_STREAM_3);
          g_config_userInfo[3].stream=!!IsDlgButtonChecked(hwndDlg,IDC_STREAM_4);
          
          GetDlgItemText(hwndDlg,IDC_USER_1,g_config_userInfo[0].user,64);
          GetDlgItemText(hwndDlg,IDC_USER_2,g_config_userInfo[1].user,64);
          GetDlgItemText(hwndDlg,IDC_USER_3,g_config_userInfo[2].user,64);
          GetDlgItemText(hwndDlg,IDC_USER_4,g_config_userInfo[3].user,64);
          GetDlgItemText(hwndDlg,IDC_PASS_1,g_config_userInfo[0].pass,64);
          GetDlgItemText(hwndDlg,IDC_PASS_2,g_config_userInfo[1].pass,64);
          GetDlgItemText(hwndDlg,IDC_PASS_3,g_config_userInfo[2].pass,64);
          GetDlgItemText(hwndDlg,IDC_PASS_4,g_config_userInfo[3].pass,64);

          updateServerPorts();

          WritePrivateProfileStruct(m_classname,"userinfo",g_config_userInfo,sizeof(g_config_userInfo),conf_file);
          WritePrivateProfileString(m_classname,"port",g_config_port_str,conf_file);
          {
            char buf[512];
            wsprintf(buf,"%d",g_config_max_cons);
            WritePrivateProfileString(m_classname,"max_cons",buf,conf_file);
          }
          WritePrivateProfileString(m_classname,"enabled",g_config_enable?"1":"0",conf_file);
          WritePrivateProfileString(m_classname,"do_auth",g_config_do_auth?"1":"0",conf_file);
          {
            char buf[32];
            wsprintf(buf,"%d",g_config_refresh);
            WritePrivateProfileString(m_classname,"refresh",buf,conf_file);
          }

          EndDialog(hwndDlg,0);
        break;
        case IDCANCEL:
          EndDialog(hwndDlg,0);
        break;
      }
    return 0;
  }
  return 0;
}

void leftcopy(char* input,char* output,int pos)
{
	int index = 0;
	for(int i = 0; i < pos; i++)
	{
		output[index] = input[i];
		index++;
	}
	output[index] = 0;
}

void rightcopy(char* input,char* output,int pos)
{
	int index = 0;
	int len = strlen(input);
	for(int i = pos; i < len; i++)
	{
		output[index] = input[i];
		index++;
	}
	output[index] = 0;
}

int ifindstr(int startx,char* body,char* search)
{
	int len = strlen(body);
	int len2 = strlen(search); // search len
            		
	for(int i = startx; i < len; i++)
	{
		if(body[i] == search[0])
		{
			bool ichk = true;
			for(int z = 0; z < len2; z++)
			{
				if(body[i+z] != search[z])
				{
					ichk = false;
				}
			}
			if(ichk == true)
			{
				return i;
			}
		}
	}
	return -1; // failure
}

void strreplace(char* source, char* replace, char* with)
{
	int pos, count = 0;
	pos = ifindstr(0,source,replace);
	if(pos == -1)
	{
		return;
	}else{
		char* left = new char [ strlen(source) + 1 ];
		char* right = new char [ strlen(source) + 1 ];
		leftcopy(source,left,pos);
		rightcopy(source,right,pos+strlen(replace));
		//char *sex = new char[strlen(left)+strlen(right)+strlen(with)];
		strcpy(source,left);
		strcat(source,with);
		strcat(source,right);
		delete [] left;
		delete [] right;
		count++;
		strreplace(source,replace,with);
	}
}

int PluginMessageProc(int message_type, int param1, int param2, int param3)
{
  switch (message_type)
  {
    case ML_MSG_CONFIG:
      DialogBox(plugin.hDllInstance,MAKEINTRESOURCE(IDD_DIALOG1),(HWND)param1,configDialogProc);
    return 1;
  }
  return 0;
}

winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"ml_www v" VERSION,
	init,
	quit,
  PluginMessageProc,
};

extern "C" {

__declspec( dllexport ) winampMediaLibraryPlugin * winampGetMediaLibraryPlugin()
{
	return &plugin;
}

};