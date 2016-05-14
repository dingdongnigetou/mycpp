
#ifndef __MYCPP_COMMON_MYDEFS_H__
#define __MYCPP_COMMON_MYDEFS_H__

#include "mytypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MYSPRINTFS sprintf_s

#define MYFPRINTF fprintf

#define MYPRINTF printf

#ifdef _MSWINDOWS_
#define MYSNPRINTF(xbuf, xbufsize , xfmt, ... ) _snprintf_s(xbuf,xbufsize, _TRUNCATE,xfmt, __VA_ARGS__)
#define bzero(_m, _s) memset(_m, 0, _s)
#define MYVSNPRINTF(xbuf, xbufsize , xfmt, ... ) _vsnprintf_s(xbuf,xbufsize, _TRUNCATE,xfmt, __VA_ARGS__)
#else
#define MYSNPRINTF snprintf
#define MYVSNPRINTF vsnprintf
#endif

#define KASSERTBUFFSIZE 256

#ifdef _DEBUG

static void DoAssert(char *s)
{
	MYPRINTF("%s\n", s);

#ifdef _MSWINDOWS_
	assert(0);
#else
	(*(int*)0) = 0;
#endif
}

#define MYASSERT(condition)    {                              \
	\
	if (!(condition))                                       \
{                                                       \
	char s[KASSERTBUFFSIZE];                            \
	s[KASSERTBUFFSIZE-1] = 0;                          \
	MYSNPRINTF(s,KASSERTBUFFSIZE-1, "_Assert: %s, %s, %d", #condition, __FILE__, __LINE__ ); \
	DoAssert(s);                                        \
}   }

#define MYASSERTV(condition,errNo)    {                                   \
	if (!(condition))                                                   \
{                                                                   \
	char s[KASSERTBUFFSIZE];                                        \
	s[KASSERTBUFFSIZE-1] = 0;                                      \
	MYSNPRINTF( s,KASSERTBUFFSIZE-1, "_AssertV: %s, %s, %d (%d)", #condition, __FILE__, __LINE__, errNo );    \
	DoAssert(s);                                                    \
}   }

#else
#define MYASSERT(condition) (condition)
#define MYASSERTV(condition,errNo) (condition)
#endif

static void DoAbort(char *s)
{
	MYPRINTF("%s\n", s);
	abort();
}

#define MYABORT(condition)    {                              \
	\
	if (!(condition))                                       \
{                                                       \
	char s[KASSERTBUFFSIZE];                            \
	s[KASSERTBUFFSIZE-1] = 0;                          \
	MYSNPRINTF(s,KASSERTBUFFSIZE-1, "_Abort: %s, %s, %d", #condition, __FILE__, __LINE__ ); \
	DoAbort(s);                                        \
}   }

#define MYABORTV(condition,errNo)    {                                   \
	if (!(condition))                                                   \
{                                                                   \
	char s[KASSERTBUFFSIZE];                                        \
	s[KASSERTBUFFSIZE - 1] = 0;                                      \
	MYSNPRINTF( s,KASSERTBUFFSIZE-1, "_AbortV: %s, %s, %d (%d)", #condition, __FILE__, __LINE__, errNo );    \
	DoAbort(s);                                                    \
}   }

#define MYABORTM(condition, errmsg)    {                              \
	\
	if (!(condition))                                       \
{                                                       \
	char s[KASSERTBUFFSIZE];                            \
	s[KASSERTBUFFSIZE-1] = 0;                          \
	MYSNPRINTF(s,KASSERTBUFFSIZE-1, "_Abort: %s:%d expre: %s, %s", __FILE__, __LINE__, #condition, errmsg); \
	DoAbort(s);                                        \
}   }

#define MYWARN(condition) {                                       \
	if (!(condition))                                       \
	MYPRINTF( "_Warn: %s, %s, %d\n",#condition, __FILE__, __LINE__ );     }                                                           

#define MYWARNV(condition,msg)        {                               \
	if (!(condition))                                               \
	MYPRINTF("_WarnV: %s, %s, %d (%s)\n",#condition, __FILE__, __LINE__, msg );  }                                                   

#define MYWARNVE(condition,msg,err)  {                           		\
	if (!(condition))                                               \
{   char buffer[KASSERTBUFFSIZE];								\
	buffer[KASSERTBUFFSIZE-1] = 0;                              \
	MYPRINTF("_WarnV: %s, %s, %d (%s, %s [err=%d])\n",#condition, __FILE__, __LINE__, msg, qtss_strerror(err,buffer,sizeof(buffer) -1), err );  \
}	}

#define MY_ASSERT_RET_VAL(_condition, _val) if(!(_condition)){MYASSERT(0); return _val;}
#define MY_ASSERT_RET(_condition) if(!(_condition)){ MYASSERT(0); return;}

#endif // __MYCPP_COMMON_MYDEFS_H__

