#include "MysqlRecordSet.h"

#include <sstream>
#include <stdlib.h>

#include "StrUtil.hpp"
using namespace mycpp;

CMysqlRecordSet::CMysqlRecordSet( MYSQL* pConn, MYSQL_STMT* pStmt )
:pMysqlConn_(pConn)
,pStmt_(pStmt)
{
	mapFieldList_.clear();

	if ( !pStmt )
	{
		pResult_ = mysql_store_result(pMysqlConn_);

		if ( pResult_ )
			MoveNext();
	}
}

CMysqlRecordSet::~CMysqlRecordSet()
{
	 if( pResult_ )
	     mysql_free_result(pResult_);

     if( pStmt_ )
         mysql_stmt_close(pStmt_);

	 if( pBindParam_ )
		 delete []pBindParam_;

	 mapFieldList_.clear();
	
	 pResult_ = nullptr;
	 pStmt_ = nullptr; 
	 pBindParam_ = nullptr;
}

bool CMysqlRecordSet::Eof()
{   
   MY_ASSERT_RET_VAL(pResult_, false);
   return bEof_;
}

bool CMysqlRecordSet::MoveNext()
{
	MY_ASSERT_RET_VAL(pMysqlConn_, false);
	MY_ASSERT_RET_VAL(pResult_, false);

	if ( !mysql_fetch_row(pResult_) )
	{
		if ( mysql_errno(pMysqlConn_) == 0 )
			bEof_ = true;

		return false;
	}

	iMoveRows_++;

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


EnumDataType CMysqlRecordSet::getDataType( unsigned int iColumn, unsigned int iBufSize )
{   
    MY_ASSERT_RET_VAL(pResult_, DT_UNKNOWN);

	auto pField = mysql_fetch_field_direct(pResult_, iColumn);

	MY_ASSERT_RET_VAL(pField, DT_UNKNOWN);

	switch ( pField->type )
	{
	case MYSQL_TYPE_DECIMAL:
	case MYSQL_TYPE_NEWDECIMAL:
		{
			auto p = pField->length;     //字段长度
			auto s = pField->decimals;     //字段小数位

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

bool  CMysqlRecordSet::getFieldList()
{   
	MY_ASSERT_RET_VAL(pResult_, false);

	mapFieldList_.clear();

	auto iCurOffset = mysql_field_tell(pResult_); // 记录第一列的位置

	auto iFieldNum =  mysql_num_fields(pResult_);  // 字段数目

	for(unsigned int i=0;i< iFieldNum;i++)
	{
		auto pField = mysql_fetch_field(pResult_);
		if( !pField )
			return false;
		
	    // 转化为大写字符串
	    auto strUpperName = StrUtil::ToUpper(std::string(pField->name));

		mapFieldList_.insert(std::map<std::string, unsigned int>::value_type(strUpperName,i));
	}

	mysql_field_seek(pResult_, iCurOffset);   // 回到第一列

	return true;
}

bool  CMysqlRecordSet::getFieldIndex( const char* szFieldName,unsigned int& iIndex )
{
	MY_ASSERT_RET_VAL(szFieldName,false);

	auto strNameUpper = StrUtil::ToUpper(std::string(szFieldName)); //转化为大写字符串

	auto iter = mapFieldList_.find(strNameUpper);

	if( iter == mapFieldList_.end() )
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
    MY_ASSERT_RET_VAL(pMysqlConn_, false);
	MY_ASSERT_RET_VAL(pResult_, false);
	MY_ASSERT_RET_VAL(szFieldName, false);
	MY_ASSERT_RET_VAL(pBuf, false);
	MY_ASSERT_RET_VAL(iFactLen, false);

	if ( iBufSize == 0 )
	{
		MYASSERT(iBufSize>0);
		return false;
	}

    auto pRow = pResult_->current_row;  //获取当前行
	MY_ASSERT_RET_VAL(pRow, false);
  
    if( mapFieldList_.empty() )
		getFieldList();           //字段名与序号的map

	unsigned int iIndex(0);   //获取字段在该行记录中的下标

	if( !getFieldIndex(szFieldName, iIndex) )      //szFieldName是否为表的一个字段
         return false;

	unsigned int iValLen = 0;  // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			auto eDataType = getDataType(iIndex, iBufSize);
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
			auto pField = mysql_fetch_field_direct(pResult_, iIndex);
			MY_ASSERT_RET_VAL(pField, false);

			auto czVal = pRow[iIndex];
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
			auto pField = mysql_fetch_field_direct(pResult_, iIndex);
			MY_ASSERT_RET_VAL(pField, false);

			auto czVal = pRow[iIndex];
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
			auto pField = mysql_fetch_field_direct(pResult_, iIndex);
			MY_ASSERT_RET_VAL(pField, false);

		 	auto czVal = pRow[iIndex];
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
			auto eDataType = getDataType(iIndex, iBufSize);
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
	MY_ASSERT_RET_VAL(pMysqlConn_, false);
	MY_ASSERT_RET_VAL(pResult_, false);
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


	auto pRow = pResult_->current_row;      //获取当前行
	MY_ASSERT_RET_VAL(pRow, false);

	unsigned int iValLen = 0;  // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			auto eDataType = getDataType(iColumn, iBufSize);
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
			auto pField = mysql_fetch_field_direct(pResult_, iColumn);
			MY_ASSERT_RET_VAL(pField, false);

			auto czVal = pRow[iColumn];
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
			auto pField = mysql_fetch_field_direct(pResult_, iColumn);
			MY_ASSERT_RET_VAL(pField, false);

			auto czVal = pRow[iColumn];
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
			auto pField = mysql_fetch_field_direct(pResult_, iColumn);
			MY_ASSERT_RET_VAL(pField, false);

            auto czVal = pRow[iColumn];
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
			auto eDataType = getDataType(iColumn, iBufSize);
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

unsigned int CMysqlRecordSet::GetRowsMoved()
{
	return iMoveRows_;
}

unsigned int CMysqlRecordSet::GetColumns()
{
	MY_ASSERT_RET_VAL(pResult_, 0);
	return  mysql_num_fields(pResult_);
}

const char*  CMysqlRecordSet::GetColumnName( unsigned int iColIndex )
{    
    MY_ASSERT_RET_VAL(pResult_, nullptr);
	MY_ASSERT_RET_VAL(iColIndex>0, nullptr);
	
	auto pField = mysql_fetch_field_direct(pResult_, iColIndex-1);   // Column indexes start with 0 in MYSQL
	MY_ASSERT_RET_VAL(pField, nullptr);
	return pField->name;
}

void CMysqlRecordSet::SetBindRows( unsigned int iSize )
{
	 MY_ASSERT_RET(pStmt_);
	 MY_ASSERT_RET(iSize>0);

	 auto iColNum = mysql_stmt_param_count(pStmt_);   //参数中字段的数目
	 MY_ASSERT_RET(iColNum>0);

	 pBindParam_ = new MYSQL_BIND[iColNum];
	 ::memset(pBindParam_, 0, sizeof(MYSQL_BIND)*iColNum);
	 iBindCols_ = iColNum;
     
	 iBindRows_ = iSize;
}

unsigned int CMysqlRecordSet::GetBindRows()
{
	MY_ASSERT_RET_VAL(pStmt_,0);
	return iBindRows_;
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
	MY_ASSERT_RET_VAL(pMysqlConn_, false);
	MY_ASSERT_RET_VAL(pStmt_, false);
	MY_ASSERT_RET_VAL(pBuf, false);
	MY_ASSERT_RET_VAL(iRowIndex<iBindRows_, false);
	MY_ASSERT_RET_VAL(iValueIndex<iBindCols_, false);

	switch ( eType )
	{
	case DT_NUMBER:
		{
			auto eDataType = getDataType(iValueIndex , iBufSize);
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
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_TINY;
			pBindParam_[iValueIndex].buffer = &iVal;
		}

		break;
	case DT_UINT8:
		{
			unsigned short iVal = *(UInt8*)pBuf;
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_TINY;
			pBindParam_[iValueIndex].buffer = &iVal;
 
		}
		break;
	case DT_INT16:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_SHORT;
			pBindParam_[iValueIndex].buffer = (short*)pBuf;
		}
		break;
	case DT_UINT16:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_SHORT;
			pBindParam_[iValueIndex].buffer = (short*)pBuf;
		}
		break;
	case DT_INT32:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_LONG;
			pBindParam_[iValueIndex].buffer = (int*)pBuf;
		}
		break;
	case DT_UINT32:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_LONG;
			pBindParam_[iValueIndex].buffer = (unsigned int*)pBuf;
		}
		break;
	case DT_INT64:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_LONGLONG;
			pBindParam_[iValueIndex].buffer = (__int64*)pBuf;
		}
		break;
	case DT_UINT64:
		{
		    pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_LONGLONG;
			pBindParam_[iValueIndex].buffer = (unsigned __int64*)pBuf;
		}
		break;
	case DT_TIME:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_TIME;
			pBindParam_[iValueIndex].buffer_length = iFactLen;
			pBindParam_[iValueIndex].buffer = pBuf;
			pBindParam_[iValueIndex].is_null = (my_bool*)&bNull;
		}
		break;
	case DT_FLOAT:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_FLOAT;
			pBindParam_[iValueIndex].buffer = (float*)pBuf;
	
		}
		break;
	case DT_DOUBLE:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_DOUBLE;
			pBindParam_[iValueIndex].buffer = (double*)pBuf;
		}
		break;
	case DT_STRING:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_STRING;
			pBindParam_[iValueIndex].buffer_length = iFactLen;
			pBindParam_[iValueIndex].buffer = pBuf;
			pBindParam_[iValueIndex].is_null = (my_bool*)&bNull;
		}
		break;
	case DT_CLOB://mysql没有clob类型
	case DT_BLOB:
		{
			pBindParam_[iValueIndex].buffer_type = MYSQL_TYPE_BLOB;
			pBindParam_[iValueIndex].buffer_length = iFactLen;
			pBindParam_[iValueIndex].buffer = pBuf;
			pBindParam_[iValueIndex].is_null = (my_bool*)&bNull;
		}
		break;
	default:
		return false;
	}
 
	if( iValueIndex == iBindCols_-1 )    // 每行最后一列 需要执行一次
	{   
		if ( iRowIndex == 0 )      // 只需在第一行绑定参数
			if ( mysql_stmt_bind_param(pStmt_, pBindParam_) != 0 )
				return false;

		if ( mysql_stmt_execute(pStmt_) != 0 )
			return false;

		::memset(pBindParam_, 0, sizeof(MYSQL_BIND)*iBindCols_);   // 清空

		if ( iRowIndex == iBindRows_ -1 )   // 最后一行
			bBindSuccess_ = true;
	}

	return true;
}

