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

#ifndef _GAYSTRING_H_
#define _GAYSTRING_H_

class GayString
{
public:
  GayString(const char *initial=NULL);
  ~GayString();
  void Set(const char *value);
  char *Get();

  void Append(const char *append);
  void Grow(int newsize);
  void Compact();

  void Replace(char *replace, char *with);
  void SetWithFile(const char *filename);
  void WriteToFile(char *filename);
  

private:

  int ifindstr(int start, char *search);
  void leftcopy(char* output, int pos);
  void rightcopy(char* output, int pos);
  char *m_buf;
  int m_alloc;
};

#endif//_GAYSTRING_H_
