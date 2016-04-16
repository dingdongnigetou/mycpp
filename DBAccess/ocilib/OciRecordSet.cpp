
#include <sstream>

#include "OciRecordSet.h"
#include "OciConnection.h"

COciRecordSet::COciRecordSet( OCI_Statement *pStmt, COciConnection* pOciConn )
:pStmt_(pStmt)
,pOciConn_(pOciConn)
{
	mapParamsList_.clear();
	mapDateList_.clear();
	mapLobList_.clear();
	mapDateArrayList_.clear();
	mapLobArrayList_.clear();

	pResultSet_ = OCI_GetResultset(pStmt_);   // bind模式下结果集无效
	if ( pResultSet_ )
		MoveNext(); // 需要先移到第一条记录
}

COciRecordSet::~COciRecordSet()
{
	OCI_ReleaseResultsets(pStmt_);
	OCI_StatementFree(pStmt_);
	pStmt_ = nullptr;
	pResultSet_ = nullptr;

	for (auto m : mapParamsList_)
	{
		if (nullptr != m.second)
			delete[] m.second;
	}
	mapParamsList_.clear();

	for (auto m : mapDateList_)
	{
		if (nullptr != m.second)
			OCI_DateFree(m.second);
	}
	mapDateList_.clear();

	for (auto m : mapLobList_)
	{
		if (nullptr != m.second)
			OCI_LobFree(m.second);
	}
	mapLobList_.clear();

	for (auto m : mapDateArrayList_)
	{
		if (nullptr != (*m.second))
			OCI_DateArrayFree(m.second);
	}
	mapDateArrayList_.clear();

	for (auto m : mapLobArrayList_)
	{
		if (nullptr != (*m.second))
			OCI_LobArrayFree(m.second);
	}
	mapLobArrayList_.clear();
}

bool COciRecordSet::Eof()
{
	DB_POINTER_CHECK_RET(pStmt_,false);
	DB_POINTER_CHECK_RET(pResultSet_,false);

	return bEof_;
}

// bool COciRecordSet::MoveLast()
// {
// 	DB_POINTER_CHECK_RET(pStmt_,false);
// 	DB_POINTER_CHECK_RET(pResultSet_,false);
// 
// 	if ( !OCI_FetchLast(pResultSet_) )
// 		return false;
// 
// 	return true;
// }

bool COciRecordSet::MoveNext()
{
	DB_POINTER_CHECK_RET(pStmt_,false);
	DB_POINTER_CHECK_RET(pResultSet_,false);

	if ( !OCI_FetchNext(pResultSet_) )
	{
		if ( OCI_ErrorGetOCICode(OCI_GetLastError()) == OCI_ERR_NONE )
			bEof_ = true;
		else
			pOciConn_->SetLastError();
		return false;
	}

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

bool COciRecordSet::GetValue( const char* szFieldName, 
							 void* pBuf,
							 unsigned int iBufSize, 
							 unsigned int* iFactLen, 
							 EnumDataType eType /* = DT_UNKNOWN */ )
{
	DB_POINTER_CHECK_RET(pStmt_,false);
	DB_POINTER_CHECK_RET(pResultSet_,false);
	DB_POINTER_CHECK_RET(szFieldName,false);
	DB_POINTER_CHECK_RET(pBuf,false);
	if ( iBufSize == 0 )
	{
		MYASSERT(iBufSize>0);
		return false;
	}

	if ( OCI_GetColumn2(pResultSet_,szFieldName) == nullptr )  // 判断是否存在
	{
		pOciConn_->SetLastError();
		return false;
	}

	unsigned int iValLen = 0;  // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			auto eDataType = getDataType(OCI_GetColumnIndex(pResultSet_,szFieldName), iBufSize);
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
		*(Int8*)pBuf = (Int8)OCI_GetShort2(pResultSet_, szFieldName);
		break;
	case DT_UINT8:
		iValLen = sizeof(UInt8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt8*)pBuf = (UInt8)OCI_GetUnsignedShort2(pResultSet_, szFieldName);
		break;
	case DT_INT16:
		iValLen = sizeof(Int16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int16*)pBuf = OCI_GetShort2(pResultSet_, szFieldName);
		break;
	case DT_UINT16:
		iValLen = sizeof(UInt16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt16*)pBuf = OCI_GetUnsignedShort2(pResultSet_, szFieldName);
		break;
	case DT_INT32:
		iValLen = sizeof(Int32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int32*)pBuf = OCI_GetInt2(pResultSet_, szFieldName);
		break;
	case DT_UINT32:
		iValLen = sizeof(UInt32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt32*)pBuf = OCI_GetUnsignedInt2(pResultSet_, szFieldName);
		break;
	case DT_INT64:
		iValLen = sizeof(Int64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int64*)pBuf = OCI_GetBigInt2(pResultSet_, szFieldName);
		break;
	case DT_UINT64:
		iValLen = sizeof(UInt64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt64*)pBuf = OCI_GetUnsignedBigInt2(pResultSet_, szFieldName);
		break;
	case DT_TIME:
		{
			auto pColumn = OCI_GetColumn2(pResultSet_, szFieldName);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=nullptr);
				return false;
			}

			if ( OCI_ColumnGetSize(pColumn) > 0 )
			{
				auto pDate = OCI_GetDate2(pResultSet_, szFieldName);
				if ( pDate )
				{
					if ( !OCI_DateToText(pDate,
						"YYYY-MM-DD HH24:MI:SS",
						iBufSize,
						(otext*)pBuf) )
						return false;

					iValLen = strlen((char*)pBuf);
				}
			}
		}
		break;
	case DT_FLOAT:
		iValLen = sizeof(float);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(float*)pBuf = OCI_GetFloat2(pResultSet_, szFieldName);
		break;
	case DT_DOUBLE:
		iValLen = sizeof(double);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(double*)pBuf = OCI_GetDouble2(pResultSet_, szFieldName);
		break;
	case DT_STRING:
		{
			auto pColumn = OCI_GetColumn2(pResultSet_, szFieldName);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=nullptr);
				return false;
			}

			auto czVal = OCI_GetString2(pResultSet_, szFieldName);
			if ( OCI_ColumnGetSize(pColumn) > 0 && czVal)
			{
				iValLen = strlen(czVal);
				iValLen = iValLen>iBufSize?iBufSize:iValLen;
				memcpy(pBuf,
					czVal,
					iValLen);				
			}
			else
			{
				iValLen = 0;
			}
			((char*)pBuf)[iValLen] = '\0';
		}
		break;
	case DT_CLOB:
		{
			if ( !getLob(szFieldName, pBuf, iBufSize, &iValLen, true) )
				return false;
		}
		break;
	case DT_BLOB:
		{
			if ( !getLob(szFieldName, pBuf, iBufSize, &iValLen, false) )
				return false;
		}
		break;
	default:
		{
			auto eDataType = getDataType(OCI_GetColumnIndex(pResultSet_,szFieldName), iBufSize);
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

bool COciRecordSet::GetValue( unsigned int iColumn, 
							 void* pBuf,
							 unsigned int iBufSize, 
							 unsigned int* iFactLen, 
							 EnumDataType eType /* = DT_UNKNOWN */ )
{
	DB_POINTER_CHECK_RET(pStmt_,false);
	DB_POINTER_CHECK_RET(pResultSet_,false);
	DB_POINTER_CHECK_RET(pBuf, false);

	if ( iColumn < 1 
		|| iBufSize == 0 )
	{
		MYASSERT(iColumn>0); // Column indexes start with 1 in OCILIB
		MYASSERT(iBufSize>0);
		return false;
	}

	if ( OCI_GetColumn(pResultSet_,iColumn) == nullptr )  // 判断是否存在
	{
		pOciConn_->SetLastError();
		return false;
	}

	unsigned int iValLen = 0;    // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			auto eDataType = getDataType(iColumn, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !GetValue(iColumn, pBuf, iBufSize, &iValLen, eDataType) )
				return false;
		}
		break;
	case DT_INT8:
		iValLen = sizeof(Int8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int8*)pBuf = (Int8)OCI_GetShort(pResultSet_, iColumn);
		break;
	case DT_UINT8:
		iValLen = sizeof(UInt8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt8*)pBuf = (UInt8)OCI_GetUnsignedShort(pResultSet_, iColumn);
		break;
	case DT_INT16:
		iValLen = sizeof(Int16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int16*)pBuf = OCI_GetShort(pResultSet_, iColumn);
		break;
	case DT_UINT16:
		iValLen = sizeof(UInt16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt16*)pBuf = OCI_GetUnsignedShort(pResultSet_, iColumn);
		break;
	case DT_INT32:
		iValLen = sizeof(Int32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int32*)pBuf = OCI_GetInt(pResultSet_, iColumn);
		break;
	case DT_UINT32:
		iValLen = sizeof(UInt32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt32*)pBuf = OCI_GetUnsignedInt(pResultSet_, iColumn);
		break;
	case DT_INT64:
		iValLen = sizeof(Int64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int64*)pBuf = OCI_GetBigInt(pResultSet_, iColumn);
		break;
	case DT_UINT64:
		iValLen = sizeof(UInt64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt64*)pBuf = OCI_GetUnsignedBigInt(pResultSet_, iColumn);
		break;
	case DT_TIME:
		{
			auto pColumn = OCI_GetColumn(pResultSet_, iColumn);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=nullptr);
				return false;
			}

			//iValLen = OCI_ColumnGetSize(pColumn);
			if ( OCI_ColumnGetSize(pColumn) > 0 )
			{
				auto pDate = OCI_GetDate(pResultSet_, iColumn);
				if ( pDate )
				{
					if ( !OCI_DateToText(pDate,
						"YYYY-MM-DD HH24:MI:SS",
						iBufSize,
						(otext*)pBuf) )
						return false;

					iValLen = strlen((char*)pBuf);
				}
			}
		}
		break;
	case DT_FLOAT:
		iValLen = sizeof(float);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(float*)pBuf = OCI_GetFloat(pResultSet_, iColumn);
		break;
	case DT_DOUBLE:
		iValLen = sizeof(double);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(double*)pBuf = OCI_GetDouble(pResultSet_, iColumn);
		break;
	case DT_STRING:
		{
			auto pColumn = OCI_GetColumn(pResultSet_, iColumn);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=nullptr);
				return false;
			}
	
			auto czVal = OCI_GetString(pResultSet_, iColumn);
			if ( OCI_ColumnGetSize(pColumn) > 0 && czVal)
			{
				iValLen = strlen(czVal);
				iValLen = iValLen>iBufSize?iBufSize:iValLen;
				memcpy(pBuf,
					czVal,
					iValLen);				
			}
			else
			{
				iValLen = 0;
			}
			((char*)pBuf)[iValLen] = '\0';
		}
		break;
	case DT_CLOB:
		{
			if ( !getLob(iColumn, pBuf, iBufSize, &iValLen, true) )
				return false;
		}
		break;
	case DT_BLOB:
		{
			if ( !getLob(iColumn, pBuf, iBufSize, &iValLen, false) )
				return false;
		}
		break;
	default:
		{
			auto eDataType = getDataType(iColumn, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !GetValue(iColumn, pBuf, iBufSize, &iValLen, eDataType) )
				return false;
		}
		break;
	}

	if ( iFactLen )
		*iFactLen = iValLen;

	return true;
}

unsigned int COciRecordSet::GetRowsMoved()
{
	DB_POINTER_CHECK_RET(pResultSet_,0);

	return OCI_GetRowCount(pResultSet_);
}

unsigned int COciRecordSet::GetColumns()
{
	DB_POINTER_CHECK_RET(pResultSet_,0);

	return OCI_GetColumnCount(pResultSet_);
}

const char* COciRecordSet::GetColumnName( unsigned int iColIndex )
{
	DB_POINTER_CHECK_RET(pResultSet_,nullptr);

	return OCI_ColumnGetName(OCI_GetColumn(pResultSet_,iColIndex));
}

void COciRecordSet::SetBindRows( unsigned int iSize )
{
	MYASSERT(pStmt_!=nullptr);

	if ( pStmt_ && iSize > 1 )
		OCI_BindArraySetSize(pStmt_, iSize);

	iBindRows_ = iSize;
}

unsigned int COciRecordSet::GetBindRows()
{
	DB_POINTER_CHECK_RET(pStmt_,0);

	return iBindRows_;
}

bool COciRecordSet::BindField( unsigned int iRowIndex,
							  unsigned int iValueIndex,
							  EnumDataType eType, 
							  void *pBuf, 
							  unsigned int iBufSize /* = 0 */,
							  unsigned int iFactLen /* = 0 */,
							  bool bNull /* = false */ )
{
	DB_POINTER_CHECK_RET(pStmt_,false);
	//DB_POINTER_CHECK_RET(pResultSet_,false);   // bind模式下结果集无效
	DB_POINTER_CHECK_RET(pBuf, false);
	MYASSERT(iRowIndex<iBindRows_);
	if ( iRowIndex >= iBindRows_ )
		return false;

	switch ( eType )
	{
	case DT_NUMBER:
		{
			auto eDataType = getDataType(iValueIndex+1, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !BindField(iRowIndex, iValueIndex, eDataType, pBuf, iBufSize, iFactLen, bNull) )
				return false;
		}
		break;
	case DT_INT8:
		if ( iBindRows_ == 1 )
		{
			short iVal = *(Int8*)pBuf;
			if ( !OCI_BindShort(pStmt_, toBindName(iValueIndex).c_str(), &iVal) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new short[iBindRows_];
				if ( !OCI_BindArrayOfShorts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (short*)(*iter).second;
			pTemp[iRowIndex] = *(Int8*)pBuf;
		}
		break;
	case DT_UINT8:
		if ( iBindRows_ == 1 )
		{
			auto iVal = *(unsigned short*)pBuf;
			if ( !OCI_BindUnsignedShort(pStmt_, toBindName(iValueIndex).c_str(), &iVal) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new unsigned short[iBindRows_];
				if ( !OCI_BindArrayOfUnsignedShorts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (unsigned short*)(*iter).second;
			pTemp[iRowIndex] = *(UInt8*)pBuf;
		}
		break;
	case DT_INT16:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindShort(pStmt_, toBindName(iValueIndex).c_str(), (short*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new short[iBindRows_];
				if ( !OCI_BindArrayOfShorts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (short*)(*iter).second;
			pTemp[iRowIndex] = *(short*)pBuf;
		}
		break;
	case DT_UINT16:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindUnsignedShort(pStmt_, toBindName(iValueIndex).c_str(), (unsigned short*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new unsigned short[iBindRows_];
				if ( !OCI_BindArrayOfUnsignedShorts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (unsigned short*)(*iter).second;
			pTemp[iRowIndex] = *(unsigned short*)pBuf;
		}
		break;
	case DT_INT32:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindInt(pStmt_, toBindName(iValueIndex).c_str(), (int*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new int[iBindRows_];
				if ( !OCI_BindArrayOfInts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (int*)(*iter).second;
			pTemp[iRowIndex] = *(int*)pBuf;
		}
		break;
	case DT_UINT32:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindUnsignedInt(pStmt_, toBindName(iValueIndex).c_str(), (unsigned int*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new unsigned int[iBindRows_];
				if ( !OCI_BindArrayOfUnsignedInts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (unsigned int*)(*iter).second;
			pTemp[iRowIndex] = *(unsigned int*)pBuf;
		}
		break;
	case DT_INT64:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindBigInt(pStmt_, toBindName(iValueIndex).c_str(), (big_int*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new big_int[iBindRows_];
				if ( !OCI_BindArrayOfBigInts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (big_int*)(*iter).second;
			pTemp[iRowIndex] = *(big_int*)pBuf;
		}
		break;
	case DT_UINT64:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindUnsignedBigInt(pStmt_, toBindName(iValueIndex).c_str(), (big_uint*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new big_uint[iBindRows_];
				if ( !OCI_BindArrayOfUnsignedBigInts(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (big_uint*)(*iter).second;
			pTemp[iRowIndex] = *(big_uint*)pBuf;
		}
		break;
	case DT_TIME:
		if ( iBindRows_ == 1 )
		{
			auto pDate = OCI_DateCreate( OCI_StatementGetConnection(pStmt_) );
			if ( !pDate )
				return false;
			if ( !OCI_DateFromText(pDate, (otext*)pBuf, "YYYY-MM-DD HH24:MI:SS") )
			{
				OCI_DateFree(pDate);
				return false;
			}
			if ( !OCI_BindDate(pStmt_, toBindName(iValueIndex).c_str(), pDate) )
			{
				OCI_DateFree(pDate);
				return false;
			}
			mapDateList_.insert(std::map<unsigned int, OCI_Date*>::value_type(iValueIndex, pDate));
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pDate = OCI_DateArrayCreate(OCI_StatementGetConnection(pStmt_), iBindRows_);
				if ( !OCI_BindArrayOfDates(pStmt_, toBindName(iValueIndex).c_str(), pDate, 0) )
				{
					OCI_DateArrayFree(pDate);
					return false;
				}
				mapDateArrayList_.insert(std::map<unsigned int, OCI_Date**>::value_type(iValueIndex, pDate));
			}

			auto iter = mapDateArrayList_.find(iValueIndex);
			if ( iter == mapDateArrayList_.end() )
				return false;

			auto pTemp = (OCI_Date**)(*iter).second;
			if ( !OCI_DateFromText(*pTemp, (otext*)pBuf, "YYYY-MM-DD HH24:MI:SS") )
				return false;
		}
		break;
	case DT_FLOAT:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindFloat(pStmt_, toBindName(iValueIndex).c_str(), (float*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				float *pData = new float[iBindRows_];
				if ( !OCI_BindArrayOfFloats(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (float*)(*iter).second;
			pTemp[iRowIndex] = *(float*)pBuf;
		}
		break;
	case DT_DOUBLE:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindDouble(pStmt_, toBindName(iValueIndex).c_str(), (double*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new double[iBindRows_];
				if ( !OCI_BindArrayOfDoubles(pStmt_, toBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (double*)(*iter).second;
			pTemp[iRowIndex] = *(double*)pBuf;
		}
		break;
	case DT_STRING:
		if ( iBindRows_ == 1 )
		{
			if ( !OCI_BindString(pStmt_, toBindName(iValueIndex).c_str(), (otext*)pBuf, iFactLen) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pData = new char[iBindRows_*(iBufSize+1)];   // 考虑结束符
				memset(pData, 0x0, iBindRows_*(iBufSize+1));
				if ( !OCI_BindArrayOfStrings(pStmt_, toBindName(iValueIndex).c_str(), (otext*)pData, iBufSize, 0) )
				{
					delete []pData;
					return false;
				}
				mapParamsList_.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			auto iter = mapParamsList_.find(iValueIndex);
			if ( iter == mapParamsList_.end() )
				return false;

			auto pTemp = (char*)(*iter).second;
			memcpy(pTemp+iBufSize*iRowIndex+iRowIndex, 
				pBuf, 
				iFactLen>iBufSize?iBufSize:iFactLen);
		}
		break;
	case DT_CLOB:
		if ( iBindRows_ == 1 )
		{
			auto pLob = OCI_LobCreate( OCI_StatementGetConnection(pStmt_), OCI_CLOB );
			if ( !pLob )
				return false;
			if ( OCI_LobWrite(pLob, pBuf, iFactLen) != iFactLen )
			{
				OCI_LobFree(pLob);
				return false;
			}
			if ( !OCI_BindLob(pStmt_, toBindName(iValueIndex).c_str(), pLob) )
			{
				OCI_LobFree(pLob);
				return false;
			}
			mapLobList_.insert(std::map<unsigned int, OCI_Lob*>::value_type(iValueIndex, pLob));
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pLob = OCI_LobArrayCreate(OCI_StatementGetConnection(pStmt_), OCI_CLOB, iBindRows_);
				if ( !OCI_BindArrayOfLobs(pStmt_, toBindName(iValueIndex).c_str(), pLob, OCI_CLOB, 0) )
				{
					OCI_LobArrayFree(pLob);
					return false;
				}
				mapLobArrayList_.insert(std::map<unsigned int, OCI_Lob**>::value_type(iValueIndex, pLob));
			}

			auto iter = mapLobArrayList_.find(iValueIndex);
			if ( iter == mapLobArrayList_.end() )
				return false;

			auto pTemp = (OCI_Lob**)(*iter).second;
			OCI_LobWrite(pTemp[iRowIndex], pBuf, iFactLen);
		}
		break;
	case DT_BLOB:
		if ( iBindRows_ == 1 )
		{
			auto pLob = OCI_LobCreate( OCI_StatementGetConnection(pStmt_), OCI_BLOB );
			if ( !pLob )
				return false;
			if ( OCI_LobWrite(pLob, pBuf, iFactLen) != iFactLen )
			{
				OCI_LobFree(pLob);
				return false;
			}
			if ( !OCI_BindLob(pStmt_, toBindName(iValueIndex).c_str(), pLob) )
			{
				OCI_LobFree(pLob);
				return false;
			}
			mapLobList_.insert(std::map<unsigned int, OCI_Lob*>::value_type(iValueIndex, pLob));
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				auto pLob = OCI_LobArrayCreate(OCI_StatementGetConnection(pStmt_), OCI_BLOB, iBindRows_);
				if ( !OCI_BindArrayOfLobs(pStmt_, toBindName(iValueIndex).c_str(), pLob, OCI_BLOB, 0) )
				{
					OCI_LobArrayFree(pLob);
					return false;
				}
				mapLobArrayList_.insert(std::map<unsigned int, OCI_Lob**>::value_type(iValueIndex, pLob));
			}

			auto iter = mapLobArrayList_.find(iValueIndex);
			if ( iter == mapLobArrayList_.end() )
				return false;

			auto pTemp = (OCI_Lob**)(*iter).second;
			OCI_LobWrite(pTemp[iRowIndex], pBuf, iFactLen);
		}
		break;
	default:
		return false;
	}

	return true;
}

bool COciRecordSet::getLob( const char* szFieldName, 
						   void* pBuf,
						   unsigned int iBufSize,
						   unsigned int* iFactLen,
						   bool bChar )
{
	auto pColumn = OCI_GetColumn2(pResultSet_, szFieldName);
	if ( !pColumn )
	{
		MYASSERT(pColumn!=nullptr);
		return false;
	}

	unsigned int iColumnType = OCI_ColumnGetType(pColumn);
	if ( iColumnType != OCI_CDT_LOB )
	{
		MYASSERT(iColumnType==OCI_CDT_LOB);
		return false;
	}

	auto pLob = OCI_GetLob2(pResultSet_, szFieldName);
	if ( !pLob )
	{
		MYASSERT(pLob!=nullptr);
		return false;
	}

	*iFactLen = (unsigned int)OCI_LobGetLength(pLob);  // 需要考虑一个字符占用的字节数？
	if ( *iFactLen > 0 )
	{
		if ( bChar )
		{
			auto iBytes = iBufSize;
			OCI_LobRead2(pLob, pBuf, &iBufSize, &iBytes);
			*iFactLen = iBytes;
		}
		else
			OCI_LobRead2(pLob, pBuf, &iBufSize, &iBufSize);
	}

	return true;
}

bool COciRecordSet::getLob( unsigned int iColumn, 
						   void* pBuf,
						   unsigned int iBufSize,
						   unsigned int* iFactLen,
						   bool bChar )
{
	auto pColumn = OCI_GetColumn(pResultSet_, iColumn);
	if ( !pColumn )
	{
		MYASSERT(pColumn!=nullptr);
		return false;
	}

	auto iColumnType = OCI_ColumnGetType(pColumn);
	if ( iColumnType != OCI_CDT_LOB )
	{
		MYASSERT(iColumnType==OCI_CDT_LOB);
		return false;
	}

	auto pLob = OCI_GetLob(pResultSet_, iColumn);
	if ( !pLob )
	{
		MYASSERT(pLob!=nullptr);
		return false;
	}

	*iFactLen = (unsigned int)OCI_LobGetLength(pLob);    // 需要考虑一个字符占用的字节数？
	if ( *iFactLen > 0 )
	{
		if ( bChar )
		{
			auto iBytes = iBufSize;
			OCI_LobRead2(pLob, pBuf, &iBufSize, &iBytes);
			*iFactLen = iBytes;
		}
		else
			OCI_LobRead2(pLob, pBuf, nullptr, &iBufSize);
	}

	return true;
}

std::string COciRecordSet::toBindName(unsigned int iValueIndex)
{
	std::ostringstream ss;
	ss.str("");
	ss<<":"<<iValueIndex;
	return ss.str();
}

EnumDataType COciRecordSet::getDataType( unsigned int iColumn, unsigned int iBufSize )
{
	auto pColumn = OCI_GetColumn(pResultSet_, iColumn);
	if ( !pColumn )
	{
		MYASSERT(pColumn!=nullptr);
		return DT_UNKNOWN;
	}

	switch ( OCI_ColumnGetType(pColumn) )
	{
	case OCI_CDT_NUMERIC:    // short, int, long long, float, double
		{
			auto p = OCI_ColumnGetPrecision(pColumn);
			auto s = OCI_ColumnGetScale(pColumn);

			if ( s==0 && iBufSize<sizeof(short) )
				return DT_INT8;
			else if ( s==0 && iBufSize==sizeof(short) )
				return DT_INT16;
			else if ( s==0 && iBufSize==sizeof(int) )
				return DT_INT32;
			else if ( s==0 && iBufSize==sizeof(long long) )
				return DT_INT64;
			else if ( s!=0 && abs(p-s)<7 && iBufSize==sizeof(float) )
				return DT_FLOAT;
			else if ( s!=0 /*&& abs(p-s)>=7*/ /*&& iBufSize==sizeof(double)*/ )
				return DT_DOUBLE;
		}
		break;
	case OCI_CDT_DATETIME:   // OCI_Date *
		return DT_TIME;
		break;
	case OCI_CDT_TEXT:       // otext *
		return DT_STRING;
		break;
	case OCI_CDT_LOB:       // OCI_Lob *
		if ( OCI_ColumnGetSubType(pColumn) == OCI_BLOB )
			return DT_BLOB;
		else
			return DT_CLOB;
		break;
	case OCI_CDT_LONG:      //  OCI_Long *
	case OCI_CDT_FILE:      //  OCI_File *
	case OCI_CDT_RAW:         // void *
	case OCI_CDT_CURSOR:    // OCI_Statement *
	case OCI_CDT_TIMESTAMP:  // OCI_Timestamp *
	case OCI_CDT_INTERVAL:   // OCI_Interval *
	case OCI_CDT_OBJECT:      // OCI_Object *
	case OCI_CDT_COLLECTION:  // OCI_Coll *
	case OCI_CDT_REF:         // OCI_Ref *
		return DT_UNKNOWN;
		break;
	default:
		return DT_UNKNOWN;
	}

	return DT_UNKNOWN;
}
