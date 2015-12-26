#include "MysqlRecordSet.h"

#include <sstream>
#include <stdlib.h>

#include "StrUtil.hpp"
using namespace mycpp;

CMysqlRecordSet::CMysqlRecordSet( MYSQL* pConn, MYSQL_STMT* pStmt )
:m_pMysqlConn(pConn)
,m_pResult(NULL)
,m_pStmt(pStmt)
,m_iMoveRows(0)
,m_iBindRows(0)
,m_iBindCols(0)
,m_pBindParam(NULL)
,m_bBindSuccess(false)
,m_bEof(false)
{
	m_mapFieldList.clear();

	if ( !pStmt )
	{
		m_pResult = mysql_store_result(m_pMysqlConn);

		if ( m_pResult )
			MoveNext();
	}

}

CMysqlRecordSet::~CMysqlRecordSet(void)
{
	 if( m_pResult )
	     mysql_free_result(m_pResult);

     if( m_pStmt )
         mysql_stmt_close(m_pStmt);

	 if( m_pBindParam )
		 delete []m_pBindParam;

	 m_mapFieldList.clear();
	

	 m_pResult = NULL;
	 m_pStmt = NULL; 
	 m_pBindParam = NULL;
}

bool CMysqlRecordSet::Eof( void )
{   
   MY_ASSERT_RET_VAL(m_pResult, false);
   return m_bEof;
}

bool CMysqlRecordSet::MoveNext( void )
{
	MY_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(m_pResult, false);

	if ( !mysql_fetch_row(m_pResult) )
	{
		if ( mysql_errno(m_pMysqlConn) == 0 )
			m_bEof = true;

		return false;
	}

	m_iMoveRows++;

	return true;
}

#define CHECK_DATA_LEN_LEGAL(inLen,outLen, retLen) do\
{\
	MYASSERT(outLen<=inLen);\
	if ( outLen>inLen )\
{\
	if(retLen)*retLen=outLen;\
	return false;\
}\
} while(0)


EnumDataType CMysqlRecordSet::GetDataType( unsigned int iColumn, unsigned int iBufSize )
{   
    MY_ASSERT_RET_VAL(m_pResult, DT_UNKNOWN);

	MYSQL_FIELD * pField = mysql_fetch_field_direct(m_pResult, iColumn);

	MY_ASSERT_RET_VAL(pField, DT_UNKNOWN);

	switch ( pField->type )
	{
	case MYSQL_TYPE_DECIMAL:
	case MYSQL_TYPE_NEWDECIMAL:
		{
			unsigned long  p = pField->length;     //字段长度
			unsigned int   s = pField->decimals;     //字段小数位

			if ( s==0 && iBufSize<sizeof(short) )
				return DT_INT8;
			else if ( s==0 && iBufSize==sizeof(short) )
				return DT_INT16;
			else if ( s==0 && iBufSize==sizeof(int) )
				return DT_INT32;
			else if ( s==0 && iBufSize==sizeof(long long) )
				return DT_INT64;
			else if ( s!=0 && s<7 && iBufSize==sizeof(float) )
				return DT_FLOAT;
			else if ( s!=0 /*&& abs(p-s)>=7*/ /*&& iBufSize==sizeof(double)*/ )
				return DT_DOUBLE;
		}
		break;

	case MYSQL_TYPE_TINY:
		 return DT_UINT8;
		 break;
	case MYSQL_TYPE_SHORT:
		 return DT_INT16;
		 break;
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_INT24:
		return DT_INT32;
		break;
	case MYSQL_TYPE_LONGLONG:
		return DT_INT64;
		break;
    case MYSQL_TYPE_FLOAT:
		return DT_FLOAT;
		break;
	case MYSQL_TYPE_DOUBLE:
		return DT_DOUBLE;
		break;
    
	case MYSQL_TYPE_TIME:  
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_TIMESTAMP:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_NEWDATE:
		return DT_TIME;
		break;
	case MYSQL_TYPE_STRING:
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_VARCHAR:
		return DT_STRING;
		break;
     
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
		return DT_BLOB;
	    break;

	case MYSQL_TYPE_BIT:
	case MYSQL_TYPE_SET:
	case MYSQL_TYPE_ENUM:
	case MYSQL_TYPE_GEOMETRY:
	case MYSQL_TYPE_NULL:
	case MAX_NO_FIELD_TYPES:
		return DT_UNKNOWN;
		break;
	default:
		return DT_UNKNOWN;
	}

	return DT_UNKNOWN;
}

bool  CMysqlRecordSet::GetFieldList( void )
{   
	MY_ASSERT_RET_VAL(m_pResult, false);

	m_mapFieldList.clear();

	MYSQL_FIELD_OFFSET  iCurOffset = mysql_field_tell(m_pResult); // 记录第一列的位置

	unsigned int iFieldNum =  mysql_num_fields(m_pResult);  // 字段数目

	for(unsigned int i=0;i< iFieldNum;i++)
	{
		MYSQL_FIELD* pField = mysql_fetch_field(m_pResult);
		if( !pField )
			return false;
		
	    // 转化为大写字符串
	    std::string strUpperName = StrUtil::ToUpper(std::string(pField->name));

		m_mapFieldList.insert(std::map<std::string, unsigned int>::value_type(strUpperName,i));
	}

	mysql_field_seek(m_pResult, iCurOffset);   // 回到第一列

	return true;
}

bool  CMysqlRecordSet::GetFieldIndex( const char* szFieldName,unsigned int& iIndex )
{
	MY_ASSERT_RET_VAL(szFieldName,false);

	std::string strNameUpper = StrUtil::ToUpper(std::string(szFieldName)); //转化为大写字符串

	std::map<std::string,  unsigned int>::iterator iter = m_mapFieldList.find(strNameUpper);

	if( iter == m_mapFieldList.end() )
		return false;

	iIndex = (*iter).second;
		
	return  true;
}

bool CMysqlRecordSet::GetValue( const char* szFieldName, 
			  void* pBuf,
			  unsigned int iBufSize,
			  unsigned int* iFactLen,
			  EnumDataType eType  )
{   
    MY_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(m_pResult, false);
	MY_ASSERT_RET_VAL(szFieldName, false);
	MY_ASSERT_RET_VAL(pBuf, false);
	MY_ASSERT_RET_VAL(iFactLen, false);

	if ( iBufSize == 0 )
	{
		MYASSERT(iBufSize>0);
		return false;
	}


    MYSQL_ROW  pRow = m_pResult->current_row;  //获取当前行
	MY_ASSERT_RET_VAL(pRow, false);
  
    if( m_mapFieldList.empty() )
		GetFieldList();           //字段名与序号的map

	unsigned int iIndex(0);   //获取字段在该行记录中的下标

	if( !GetFieldIndex(szFieldName, iIndex) )      //szFieldName是否为表的一个字段
         return false;

	unsigned int iValLen = 0;  // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			EnumDataType eDataType = GetDataType(iIndex, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !GetValue(szFieldName, pBuf, iBufSize, &iValLen, eDataType) )
				return false;
		}
		break;
	case DT_INT8:
		iValLen = sizeof(Int8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int8*)pBuf = StrUtil::ToNumber<Int8>(pRow[iIndex]);
		break;
	case DT_UINT8:
		iValLen = sizeof(UInt8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt8*)pBuf = StrUtil::ToNumber<UInt8>(pRow[iIndex]);
		break;
	case DT_INT16:
		iValLen = sizeof(Int16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int16*)pBuf = StrUtil::ToNumber<Int16>(pRow[iIndex]);
		break;
	case DT_UINT16:
		iValLen = sizeof(UInt16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt16*)pBuf = StrUtil::ToNumber<UInt16>(pRow[iIndex]);
		break;
	case DT_INT32:
		iValLen = sizeof(Int32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int32*)pBuf = StrUtil::ToNumber<Int32>(pRow[iIndex]);
		break;
	case DT_UINT32:
		iValLen = sizeof(UInt32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt32*)pBuf = StrUtil::ToNumber<UInt32>(pRow[iIndex]);
		break;
	case DT_INT64:
		iValLen = sizeof(Int64);
    	CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int64*)pBuf = StrUtil::ToNumber<Int64>(pRow[iIndex]);
		break;
	case DT_UINT64:
		iValLen = sizeof(UInt64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt64*)pBuf = StrUtil::ToNumber<UInt64>(pRow[iIndex]);
		break;
	case DT_TIME:
		{
			MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pResult, iIndex);
			MY_ASSERT_RET_VAL(pField, false);

			const char * czVal = pRow[iIndex];
			if ( pField->length > 0 && czVal )
			{   
				iValLen = strlen(czVal);
				memcpy(pBuf,
					czVal,
					iValLen>iBufSize?iBufSize:iValLen);
			}
			else
			{
				iValLen = 0;
			}

			((char*)pBuf)[iValLen] = '\0';
		}
		break;
	case DT_FLOAT:
		iValLen = sizeof(float);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(float*)pBuf = StrUtil::ToNumber<float>(pRow[iIndex]); 
		break;
	case DT_DOUBLE:
		iValLen = sizeof(double);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(double*)pBuf = StrUtil::ToNumber<double>(pRow[iIndex]);
		break;
	case DT_STRING:
 		{
			MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pResult, iIndex);
			MY_ASSERT_RET_VAL(pField, false);

			const char * czVal = pRow[iIndex];
			if ( pField->length > 0 && czVal )
			{   
				iValLen = strlen(czVal);
				memcpy(pBuf,
					czVal,
					iValLen>iBufSize?iBufSize:iValLen);
			}
			else
			{
				iValLen = 0;
			}
			((char*)pBuf)[iValLen] = '\0';
		}
		break;
	case DT_CLOB:
		//mysql中没有CLOB类型
	case DT_BLOB:
		{
			MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pResult, iIndex);
			MY_ASSERT_RET_VAL(pField, false);

		 	const char * czVal = pRow[iIndex];
			if ( pField->length > 0 && czVal )
			{   
				iValLen = strlen(czVal);
				memcpy(pBuf,
					czVal,
					iValLen>iBufSize?iBufSize:iValLen);
			}
			else
			{
				iValLen = 0;
			}
			((char*)pBuf)[iValLen] = '\0';

		}
		break;
	default:
		{
			EnumDataType eDataType = GetDataType(iIndex, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !GetValue(szFieldName, pBuf, iBufSize, &iValLen, eDataType) )
				return false;
		}
		break;
	}

	if ( iFactLen )
		*iFactLen = iValLen;

	return true;
}

bool CMysqlRecordSet::GetValue( unsigned int iColumn, 
			  void* pBuf,
			  unsigned int iBufSize,
			  unsigned int* iFactLen,
			  EnumDataType eType )
{
	MY_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(m_pResult, false);
	MY_ASSERT_RET_VAL(pBuf, false);
	MY_ASSERT_RET_VAL(iFactLen, false);

	if ( iBufSize == 0 )
	{
		MYASSERT(iBufSize>0);
		return false;
	}

	if ( iColumn < 1 
		|| iBufSize == 0 )
	{
		MYASSERT(iColumn>0); 
		MYASSERT(iBufSize>0);
		return false;
	}

    iColumn = iColumn - 1;  // Column indexes start with 0 in MYSQL


	MYSQL_ROW pRow = m_pResult->current_row;      //获取当前行
	MY_ASSERT_RET_VAL(pRow, false);


	unsigned int iValLen = 0;  // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			EnumDataType eDataType = GetDataType(iColumn, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !GetValue(iColumn+1, pBuf, iBufSize, &iValLen, eDataType) )
				return false;
		}
		break;
	case DT_INT8:
		iValLen = sizeof(Int8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int8*)pBuf = StrUtil::ToNumber<Int8>(pRow[iColumn]);
		break;
	case DT_UINT8:
		iValLen = sizeof(UInt8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt8*)pBuf = StrUtil::ToNumber<UInt8>(pRow[iColumn]);
		break;
	case DT_INT16:
		iValLen = sizeof(Int16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int16*)pBuf = StrUtil::ToNumber<Int16>(pRow[iColumn]);
		break;
	case DT_UINT16:
		iValLen = sizeof(UInt16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt16*)pBuf = StrUtil::ToNumber<UInt16>(pRow[iColumn]) ;
		break;
	case DT_INT32:
		iValLen = sizeof(Int32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int32*)pBuf = StrUtil::ToNumber<Int32>(pRow[iColumn]);
		break;
	case DT_UINT32:
		iValLen = sizeof(UInt32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt32*)pBuf = StrUtil::ToNumber<UInt32>(pRow[iColumn]);
		break;
	case DT_INT64:
		iValLen = sizeof(Int64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int64*)pBuf = StrUtil::ToNumber<Int64>(pRow[iColumn]);
		break;
	case DT_UINT64:
		iValLen = sizeof(UInt64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt64*)pBuf = StrUtil::ToNumber<UInt64>(pRow[iColumn]);
		break;
	case DT_TIME:
		{
			MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pResult, iColumn);
			MY_ASSERT_RET_VAL(pField, false);

			const char * czVal = pRow[iColumn];
			if ( pField->length > 0 && czVal )
			{   
				iValLen = strlen(czVal);
				memcpy(pBuf,
					czVal,
					iValLen>iBufSize?iBufSize:iValLen);
			}
			else
			{
				iValLen = 0;
			}
			((char*)pBuf)[iValLen] = '\0';
		}
		break;
	case DT_FLOAT:
		iValLen = sizeof(float);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(float*)pBuf = StrUtil::ToNumber<float>(pRow[iColumn]);
		break;
	case DT_DOUBLE:
		iValLen = sizeof(double);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(double*)pBuf = StrUtil::ToNumber<double>(pRow[iColumn]);
		break;
	case DT_STRING:
		{
			MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pResult, iColumn);
			MY_ASSERT_RET_VAL(pField, false);

			const char * czVal = pRow[iColumn];
			if ( pField->length > 0 && czVal )
			{   
				iValLen = strlen(czVal);
				memcpy(pBuf,
					czVal,
					iValLen>iBufSize?iBufSize:iValLen);
			}
			else
			{
				iValLen = 0;
			}
			((char*)pBuf)[iValLen] = '\0';
		}
		break;
	case DT_CLOB:
		//mysql中没有CLOB类型
	case DT_BLOB:
		{
			MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pResult, iColumn);
			MY_ASSERT_RET_VAL(pField, false);

            const char * czVal = pRow[iColumn];
			if ( pField->length > 0 && czVal )
			{   
				iValLen = strlen(czVal);
				memcpy(pBuf,
					czVal,
					iValLen>iBufSize?iBufSize:iValLen);
			}
			else
			{
				iValLen = 0;
			}
			((char*)pBuf)[iValLen] = '\0';
		}
		break;
	default:
		{
			EnumDataType eDataType = GetDataType(iColumn, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !GetValue(iColumn+1, pBuf, iBufSize, &iValLen, eDataType) )
				return false;
		}
		break;
	}

	if ( iFactLen )
		*iFactLen = iValLen;

	return true;
}




unsigned int CMysqlRecordSet::GetRowsMoved( void )
{
	return m_iMoveRows;
}


unsigned int CMysqlRecordSet::GetColumns( void )
{
	MY_ASSERT_RET_VAL(m_pResult, 0);

	return  mysql_num_fields(m_pResult);
}


const char*  CMysqlRecordSet::GetColumnName( unsigned int iColIndex )
{    
    MY_ASSERT_RET_VAL(m_pResult, NULL);
	MY_ASSERT_RET_VAL(iColIndex>0, NULL);
	
	MYSQL_FIELD* pField = mysql_fetch_field_direct(m_pResult, iColIndex-1);   // Column indexes start with 0 in MYSQL
	MY_ASSERT_RET_VAL(pField, NULL);
	return pField->name;
}



void CMysqlRecordSet::SetBindRows( unsigned int iSize )
{
	 MY_ASSERT_RET(m_pStmt);
	 MY_ASSERT_RET(iSize>0);


	 unsigned long iColNum = mysql_stmt_param_count(m_pStmt);   //参数中字段的数目
	 MY_ASSERT_RET(iColNum>0);

	 m_pBindParam = new MYSQL_BIND[iColNum];
	 ::memset(m_pBindParam, 0, sizeof(MYSQL_BIND)*iColNum);
	 m_iBindCols = iColNum;
     
	 m_iBindRows = iSize;
}



unsigned int CMysqlRecordSet::GetBindRows( void )
{
	MY_ASSERT_RET_VAL(m_pStmt,0);

	return m_iBindRows;
}


// 绑定字段的信息
// iRowIndex: 行序号由0开始
// iValueIndex: 值序号由0开始
// iBufSize: 字符串或者二进制数据类型时需要指定
// iFactLen: 字符串或者二进制数据类型时需要指定
bool CMysqlRecordSet::BindField( unsigned int iRowIndex, 
			   unsigned int iValueIndex,
			   EnumDataType eType, 
			   void *pBuf, 
			   unsigned int iBufSize , 
			   unsigned int iFactLen ,
			   bool bNull  )
{
	MY_ASSERT_RET_VAL(m_pMysqlConn, false);
	MY_ASSERT_RET_VAL(m_pStmt, false);
	MY_ASSERT_RET_VAL(pBuf, false);
	MY_ASSERT_RET_VAL(iRowIndex<m_iBindRows, false);
	MY_ASSERT_RET_VAL(iValueIndex<m_iBindCols, false);

	switch ( eType )
	{
	case DT_NUMBER:
		{
			EnumDataType eDataType = GetDataType(iValueIndex , iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !BindField(iRowIndex, iValueIndex, eDataType, pBuf, iBufSize, iFactLen, bNull) )
				return false;
		}
		break;
	case DT_INT8:
		{
			short iVal = *(Int8*)pBuf;
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_TINY;
			m_pBindParam[iValueIndex].buffer = &iVal;
		}

		break;
	case DT_UINT8:
		{
			unsigned short iVal = *(UInt8*)pBuf;
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_TINY;
			m_pBindParam[iValueIndex].buffer = &iVal;
 
		}
		break;
	case DT_INT16:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_SHORT;
			m_pBindParam[iValueIndex].buffer = (short*)pBuf;
		}
		break;
	case DT_UINT16:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_SHORT;
			m_pBindParam[iValueIndex].buffer = (short*)pBuf;
		}
		break;
	case DT_INT32:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_LONG;
			m_pBindParam[iValueIndex].buffer = (int*)pBuf;
		}
		break;
	case DT_UINT32:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_LONG;
			m_pBindParam[iValueIndex].buffer = (unsigned int*)pBuf;
		}
		break;
	case DT_INT64:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_LONGLONG;
			m_pBindParam[iValueIndex].buffer = (__int64*)pBuf;
		}
		break;
	case DT_UINT64:
		{
		    m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_LONGLONG;
			m_pBindParam[iValueIndex].buffer = (unsigned __int64*)pBuf;
		}
		break;
	case DT_TIME:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_TIME;
			m_pBindParam[iValueIndex].buffer_length = iFactLen;
			m_pBindParam[iValueIndex].buffer = pBuf;
			m_pBindParam[iValueIndex].is_null = (my_bool*)&bNull;
		}
		break;
	case DT_FLOAT:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_FLOAT;
			m_pBindParam[iValueIndex].buffer = (float*)pBuf;
	
		}
		break;
	case DT_DOUBLE:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_DOUBLE;
			m_pBindParam[iValueIndex].buffer = (double*)pBuf;
		}
		break;
	case DT_STRING:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_STRING;
			m_pBindParam[iValueIndex].buffer_length = iFactLen;
			m_pBindParam[iValueIndex].buffer = pBuf;
			m_pBindParam[iValueIndex].is_null = (my_bool*)&bNull;
		}
		break;
	case DT_CLOB://mysql没有clob类型
	case DT_BLOB:
		{
			m_pBindParam[iValueIndex].buffer_type = MYSQL_TYPE_BLOB;
			m_pBindParam[iValueIndex].buffer_length = iFactLen;
			m_pBindParam[iValueIndex].buffer = pBuf;
			m_pBindParam[iValueIndex].is_null = (my_bool*)&bNull;
		}
		break;
	default:
		return false;
	}
 

	if( iValueIndex == m_iBindCols-1 )    // 每行最后一列 需要执行一次
	{   
		if ( iRowIndex == 0 )      // 只需在第一行绑定参数
			if ( mysql_stmt_bind_param(m_pStmt, m_pBindParam) != 0 )
				return false;

		if ( mysql_stmt_execute(m_pStmt) != 0 )
			return false;

		::memset(m_pBindParam, 0, sizeof(MYSQL_BIND)*m_iBindCols);   // 清空

		if ( iRowIndex == m_iBindRows -1 )   // 最后一行
			m_bBindSuccess = true;
	}

	return true;
}



