
#ifndef MYCPP_COMMON_MYTYPES_H
#define MYCPP_COMMON_MYTYPES_H 


#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

//定义使用window 平台
# define _MSWINDOWS_

#if defined(_WIN64) || defined(WIN64)
//64 位系统定义
# define __LP64__
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#endif // end defined(_MSC_VER)


#if defined(_MSWINDOWS_)
	//
	// Windows/Visual C++
	//
	//typedef signed long ssize_t;
	typedef long pid_t;

	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed __int64         Int64;
	typedef unsigned __int64       UInt64;

	#define MYATOMIC               volatile LONG

#if defined(__LP64__)
//指针长度是 64 位
#define PTR_IS_64_BIT 1
	typedef signed __int64     IntPtr;
	typedef unsigned __int64   UIntPtr;
#else
	typedef signed long        IntPtr;
	typedef unsigned long      UIntPtr;

	#define MYCALLBACK CALLBACK
#endif

	//有64 位长度数据定义
#define HAVE_INT64 1

#define EXPORT_API     extern "C" __declspec(dllexport) 
#define IMPORT_API     extern "C" __declspec(dllimport)
#define EXPORT_CLASS     __declspec(dllexport) 
#define IMPORT_CLASS     __declspec(dllimport)

#define INLINE    	inline


//个数化打印定义
// 如 :  Int64 v=0; UInt x=0; printf( "%" _PINT64 ",%" _PUINT32 "\n", v, X);

#define _PINT64 "I64d"
#define _PUINT64 "I64u"

#ifdef __LP64__
# define _PINT32  "d"
# define _PUINT32 "u"
#else
# define _PINT32  "ld"
# define _PUINT32 "lu"
#endif


#define MIN_INT8  _I8_MIN
#define MAX_INT8  _I8_MAX
#define MAX_UINT8 _UI8_MAX

#define MIN_INT16  _I16_MIN
#define MAX_INT16  _I16_MAX
#define MAX_UINT16 _UI16_MAX

#define MIN_INT32 _I32_MIN
#define MAX_INT32 _I32_MAX
#define MAX_UINT32 _UI32_MAX

#define MIN_INT64  _I64_MIN
#define MAX_INT64  _I64_MAX
#define MAX_UINT64  _UI64_MAX


#elif defined(__GNUC__) || defined(_LINUX)
	//
	// Unix/GCC || linux
	//
	typedef signed char            Int8;
	typedef unsigned char          UInt8;
	typedef signed short           Int16;
	typedef unsigned short         UInt16;
	typedef signed int             Int32;
	typedef unsigned int           UInt32;
	typedef signed long            IntPtr;
	typedef unsigned long          UIntPtr;

	typedef void * HANDLE;

	#define MYATOMIC volatile long


	#define CALLBACK 

#define INVALID_HANDLE_VALUE NULL

#if defined(__LP64__)

	//指针长度是 64 位
#define PTR_IS_64_BIT 1
	typedef signed long        Int64;
	typedef unsigned long      UInt64;
#else
	typedef signed long long   Int64;
	typedef unsigned long long UInt64;
#endif
#define HAVE_INT64 1



#define INLINE	inline



#define _PINT64 "lld"
#define _PUINT64 "llu"
#ifdef __LP64__

# define _PINT32  "d"
# define _PUINT32 "u"
#else

# define _PINT32  "ld"
# define _PUINT32 "lu"

#endif

#define EXPORT_API   extern "C"  
#define IMPORT_API   extern "C"  
#define EXPORT_CLASS    
#define IMPORT_CLASS  
	//回调函数 
#ifndef CALLBACK
#define CALLBACK
#endif



#define MIN_INT8  -128
#define MAX_INT8  127
#define MAX_UINT8 0xff

#define MIN_INT16  -32768
#define MAX_INT16  32767
#define MAX_UINT16 0xffff


#define MIN_INT32 -2147483648
#define MAX_INT32 2147483647
#define MAX_UINT32 0xffffffff


#define MIN_INT64  -9223372036854775808ll
#define MAX_INT64  9223372036854775807ll
#define MAX_UINT64  0xffffffffffffffffllu


#else // end linux 
#error "Not support os"
#endif


typedef UInt8 Byte;

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
# define TRUE 0x1
# define FALSE 0
#endif

#endif // MYCPP_COMMON_MYTYPES_H

