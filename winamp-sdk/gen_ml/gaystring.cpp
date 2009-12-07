#include <windows.h>
#include "gaystring.h"

GayString::GayString(const char *initial)
{
  m_buf=NULL;
  m_alloc=0;
  if (initial) Set(initial);
}

GayString::~GayString()
{
  free(m_buf);
}

void GayString::Set(const char *value)
{
  Grow(strlen(value)+1);
  strcpy(m_buf,value);
  Compact();
}
char *GayString::Get()
{ 
  return m_buf?m_buf:(char*)""; 
}

void GayString::Append(const char *append)
{
  int oldsize=m_buf ? strlen(m_buf) : 0;
  Grow(oldsize + strlen(append) + 1);
  strcpy(m_buf+oldsize,append);
  Compact();
}

void GayString::Grow(int newsize)
{
  if (m_alloc < newsize)
  {
    m_alloc=newsize+512;
    m_buf=(char*)realloc(m_buf,m_alloc);
  }
}
void GayString::Compact()
{
  if (m_buf)
  {
    m_alloc=strlen(m_buf)+1;
    m_buf=(char*)realloc(m_buf,m_alloc);
  }
}

#include <iostream>

void GayString::leftcopy(char* output, int pos)
{
	int index = 0;
  char *input = this->Get();

	for(int i = 0; i < pos; i++)
	{
		output[index] = input[i];
		index++;
	}
	output[index] = NULL;
}

void GayString::rightcopy(char* output, int pos)
{
	int index = 0;
  char *input = this->Get();
	int len = strlen(input);
  
	for(int i = pos; i < len; i++)
	{
		output[index] = input[i];
		index++;
	}
	output[index] = NULL;
}

int GayString::ifindstr(int startx, char* search)
{
  char *body = this->Get();
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

void GayString::Replace(char* replace, char* with)
{
	int pos, count = 0;
  char *source = this->Get();
	pos = ifindstr(0,replace);
	if(pos == -1)
	{
		return;
	}else{
    char* left = new char [ strlen(source) + 1 ];
		char* right = new char [ strlen(source) + 1 ];
		leftcopy(left, pos);
		rightcopy(right, pos+strlen(replace));
    this->Set(left);
    this->Append(with);
    this->Append(right);
		delete [] left;
		delete [] right;
		count++;
		Replace(replace,with);
	}
}

void GayString::SetWithFile(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if(file)
  {
    GayString buff;
    int i = 0, len = 0;

    buff.Set("");
    this->Set("");

    while(!feof(file))
    {
      char buf[513] = {NULL};
      len = fread(buf, 512, 1, file);
      buff.Append(buf);
    }
    char *g = strdup(buff.Get());
    this->Set(g);
    fclose(file);
  }
}

void GayString::WriteToFile(char *filename)
{
  FILE *file = fopen(filename, "w");
  int len = fwrite(this->Get(), 1, strlen(this->Get()), file);
  fclose(file);
}
