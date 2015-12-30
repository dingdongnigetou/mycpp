
#include <sstream>

#include "OciRecordSet.h"
#include "OciConnection.h"

COciRecordSet::COciRecordSet( OCI_Statement *pStmt, COciConnection* pOciConn )
:m_pStmt(pStmt)
,m_pResultSet(NULL)
,m_pOciConn(pOciConn)
,m_bEof(false)
,m_iBindRows(0)
{
	m_mapParamsList.clear();
	m_mapDateList.clear();
	m_mapLobList.clear();
	m_mapDateArrayList.clear();
	m_mapLobArrayList.clear();

	m_pResultSet = OCI_GetResultset(m_pStmt);   // bind模式下结果集无效
	if ( m_pResultSet )
		MoveNext(); // 需要先移到第一条记录
}

COciRecordSet::~COciRecordSet( void )
{
	OCI_ReleaseResultsets(m_pStmt);
	OCI_StatementFree(m_pStmt);
	m_pStmt = NULL;
	m_pResultSet = NULL;


	for (std::map<unsigned int, void*>::iterator iter = m_mapParamsList.begin();
		iter != m_mapParamsList.end();
		iter ++ )
	{
		if ( (*iter).second )
			delete [](*iter).second;
	}
	m_mapParamsList.clear();

	for (std::map<unsigned int, OCI_Date*>::iterator iter = m_mapDateList.begin();
		iter != m_mapDateList.end();
		iter ++ )
	{
		if ( (*iter).second )
			OCI_DateFree((*iter).second);
	}
	m_mapDateList.clear();

	for (std::map<unsigned int, OCI_Lob*>::iterator iter = m_mapLobList.begin();
		iter != m_mapLobList.end();
		iter ++ )
	{
		if ( (*iter).second )
			OCI_LobFree((*iter).second);
	}
	m_mapLobList.clear();

	for (std::map<unsigned int, OCI_Date**>::iterator iter = m_mapDateArrayList.begin();
		iter != m_mapDateArrayList.end();
		iter ++ )
	{
		if ( *(*iter).second )
			OCI_DateArrayFree((*iter).second);
	}
	m_mapDateArrayList.clear();

	for (std::map<unsigned int, OCI_Lob**>::iterator iter = m_mapLobArrayList.begin();
		iter != m_mapLobArrayList.end();
		iter ++ )
	{
		if ( *(*iter).second )
			OCI_LobArrayFree((*iter).second);
	}
	m_mapLobArrayList.clear();
}

bool COciRecordSet::Eof( void )
{
	DB_POINTER_CHECK_RET(m_pStmt,false);
	DB_POINTER_CHECK_RET(m_pResultSet,false);

	return m_bEof;
}

// bool COciRecordSet::MoveLast( void )
// {
// 	DB_POINTER_CHECK_RET(m_pStmt,false);
// 	DB_POINTER_CHECK_RET(m_pResultSet,false);
// 
// 	if ( !OCI_FetchLast(m_pResultSet) )
// 		return false;
// 
// 	return true;
// }

bool COciRecordSet::MoveNext( void )
{
	DB_POINTER_CHECK_RET(m_pStmt,false);
	DB_POINTER_CHECK_RET(m_pResultSet,false);

	if ( !OCI_FetchNext(m_pResultSet) )
	{
		if ( OCI_ErrorGetOCICode(OCI_GetLastError()) == OCI_ERR_NONE )
			m_bEof = true;
		else
			m_pOciConn->SetLastError();
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
	DB_POINTER_CHECK_RET(m_pStmt,false);
	DB_POINTER_CHECK_RET(m_pResultSet,false);
	DB_POINTER_CHECK_RET(szFieldName,false);
	DB_POINTER_CHECK_RET(pBuf,false);
	if ( iBufSize == 0 )
	{
		MYASSERT(iBufSize>0);
		return false;
	}

	if ( OCI_GetColumn2(m_pResultSet,szFieldName) == NULL )  // 判断是否存在
	{
		m_pOciConn->SetLastError();
		return false;
	}

	unsigned int iValLen = 0;  // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			EnumDataType eDataType = GetDataType(OCI_GetColumnIndex(m_pResultSet,szFieldName), iBufSize);
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
		*(Int8*)pBuf = (Int8)OCI_GetShort2(m_pResultSet, szFieldName);
		break;
	case DT_UINT8:
		iValLen = sizeof(UInt8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt8*)pBuf = (UInt8)OCI_GetUnsignedShort2(m_pResultSet, szFieldName);
		break;
	case DT_INT16:
		iValLen = sizeof(Int16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int16*)pBuf = OCI_GetShort2(m_pResultSet, szFieldName);
		break;
	case DT_UINT16:
		iValLen = sizeof(UInt16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt16*)pBuf = OCI_GetUnsignedShort2(m_pResultSet, szFieldName);
		break;
	case DT_INT32:
		iValLen = sizeof(Int32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int32*)pBuf = OCI_GetInt2(m_pResultSet, szFieldName);
		break;
	case DT_UINT32:
		iValLen = sizeof(UInt32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt32*)pBuf = OCI_GetUnsignedInt2(m_pResultSet, szFieldName);
		break;
	case DT_INT64:
		iValLen = sizeof(Int64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int64*)pBuf = OCI_GetBigInt2(m_pResultSet, szFieldName);
		break;
	case DT_UINT64:
		iValLen = sizeof(UInt64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt64*)pBuf = OCI_GetUnsignedBigInt2(m_pResultSet, szFieldName);
		break;
	case DT_TIME:
		{
			OCI_Column *pColumn = OCI_GetColumn2(m_pResultSet, szFieldName);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=NULL);
				return false;
			}

			//iValLen = OCI_ColumnGetSize(pColumn);
			if ( OCI_ColumnGetSize(pColumn) > 0 )
			{
				OCI_Date *pDate = OCI_GetDate2(m_pResultSet, szFieldName);
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
		*(float*)pBuf = OCI_GetFloat2(m_pResultSet, szFieldName);
		break;
	case DT_DOUBLE:
		iValLen = sizeof(double);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(double*)pBuf = OCI_GetDouble2(m_pResultSet, szFieldName);
		break;
	case DT_STRING:
		{
			OCI_Column *pColumn = OCI_GetColumn2(m_pResultSet, szFieldName);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=NULL);
				return false;
			}

			//iValLen = OCI_ColumnGetSize(pColumn);
			const char *czVal = OCI_GetString2(m_pResultSet, szFieldName);
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
			if ( !GetLob(szFieldName, pBuf, iBufSize, &iValLen, true) )
				return false;
		}
		break;
	case DT_BLOB:
		{
			if ( !GetLob(szFieldName, pBuf, iBufSize, &iValLen, false) )
				return false;
		}
		break;
	default:
		{
			EnumDataType eDataType = GetDataType(OCI_GetColumnIndex(m_pResultSet,szFieldName), iBufSize);
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
	DB_POINTER_CHECK_RET(m_pStmt,false);
	DB_POINTER_CHECK_RET(m_pResultSet,false);
	DB_POINTER_CHECK_RET(pBuf, false);

	if ( iColumn < 1 
		|| iBufSize == 0 )
	{
		MYASSERT(iColumn>0); // Column indexes start with 1 in OCILIB
		MYASSERT(iBufSize>0);
		return false;
	}

	if ( OCI_GetColumn(m_pResultSet,iColumn) == NULL )  // 判断是否存在
	{
		m_pOciConn->SetLastError();
		return false;
	}

	unsigned int iValLen = 0;    // 值长度

	switch ( eType )
	{
	case DT_NUMBER:
		{
			EnumDataType eDataType = GetDataType(iColumn, iBufSize);
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
		*(Int8*)pBuf = (Int8)OCI_GetShort(m_pResultSet, iColumn);
		break;
	case DT_UINT8:
		iValLen = sizeof(UInt8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt8*)pBuf = (UInt8)OCI_GetUnsignedShort(m_pResultSet, iColumn);
		break;
	case DT_INT16:
		iValLen = sizeof(Int16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int16*)pBuf = OCI_GetShort(m_pResultSet, iColumn);
		break;
	case DT_UINT16:
		iValLen = sizeof(UInt16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt16*)pBuf = OCI_GetUnsignedShort(m_pResultSet, iColumn);
		break;
	case DT_INT32:
		iValLen = sizeof(Int32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int32*)pBuf = OCI_GetInt(m_pResultSet, iColumn);
		break;
	case DT_UINT32:
		iValLen = sizeof(UInt32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt32*)pBuf = OCI_GetUnsignedInt(m_pResultSet, iColumn);
		break;
	case DT_INT64:
		iValLen = sizeof(Int64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int64*)pBuf = OCI_GetBigInt(m_pResultSet, iColumn);
		break;
	case DT_UINT64:
		iValLen = sizeof(UInt64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt64*)pBuf = OCI_GetUnsignedBigInt(m_pResultSet, iColumn);
		break;
	case DT_TIME:
		{
			OCI_Column *pColumn = OCI_GetColumn(m_pResultSet, iColumn);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=NULL);
				return false;
			}

			//iValLen = OCI_ColumnGetSize(pColumn);
			if ( OCI_ColumnGetSize(pColumn) > 0 )
			{
				OCI_Date *pDate = OCI_GetDate(m_pResultSet, iColumn);
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
		*(float*)pBuf = OCI_GetFloat(m_pResultSet, iColumn);
		break;
	case DT_DOUBLE:
		iValLen = sizeof(double);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(double*)pBuf = OCI_GetDouble(m_pResultSet, iColumn);
		break;
	case DT_STRING:
		{
			OCI_Column *pColumn = OCI_GetColumn(m_pResultSet, iColumn);
			if ( !pColumn )
			{
				MYASSERT(pColumn!=NULL);
				return false;
			}
	
			//iValLen = OCI_ColumnGetSize(pColumn);
			const char *czVal = OCI_GetString(m_pResultSet, iColumn);
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
			if ( !GetLob(iColumn, pBuf, iBufSize, &iValLen, true) )
				return false;
		}
		break;
	case DT_BLOB:
		{
			if ( !GetLob(iColumn, pBuf, iBufSize, &iValLen, false) )
				return false;
		}
		break;
	default:
		{
			EnumDataType eDataType = GetDataType(iColumn, iBufSize);
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

unsigned int COciRecordSet::GetRowsMoved( void )
{
	DB_POINTER_CHECK_RET(m_pResultSet,0);

	return OCI_GetRowCount(m_pResultSet);
}

unsigned int COciRecordSet::GetColumns( void )
{
	DB_POINTER_CHECK_RET(m_pResultSet,0);

	return OCI_GetColumnCount(m_pResultSet);
}

const char* COciRecordSet::GetColumnName( unsigned int iColIndex )
{
	DB_POINTER_CHECK_RET(m_pResultSet,NULL);

	return OCI_ColumnGetName(OCI_GetColumn(m_pResultSet,iColIndex));
}

void COciRecordSet::SetBindRows( unsigned int iSize )
{
	MYASSERT(m_pStmt!=NULL);

	if ( m_pStmt && iSize > 1 )
		OCI_BindArraySetSize(m_pStmt, iSize);

	m_iBindRows = iSize;
}

unsigned int COciRecordSet::GetBindRows( void )
{
	DB_POINTER_CHECK_RET(m_pStmt,0);

	return m_iBindRows;
}

bool COciRecordSet::BindField( unsigned int iRowIndex,
							  unsigned int iValueIndex,
							  EnumDataType eType, 
							  void *pBuf, 
							  unsigned int iBufSize /* = 0 */,
							  unsigned int iFactLen /* = 0 */,
							  bool bNull /* = false */ )
{
	DB_POINTER_CHECK_RET(m_pStmt,false);
	//DB_POINTER_CHECK_RET(m_pResultSet,false);   // bind模式下结果集无效
	DB_POINTER_CHECK_RET(pBuf, false);
	MYASSERT(iRowIndex<m_iBindRows);
	if ( iRowIndex >= m_iBindRows )
		return false;

	switch ( eType )
	{
	case DT_NUMBER:
		{
			EnumDataType eDataType = GetDataType(iValueIndex+1, iBufSize);
			if ( eDataType == DT_UNKNOWN
				|| eDataType == DT_NUMBER )
				return false;

			if ( !BindField(iRowIndex, iValueIndex, eDataType, pBuf, iBufSize, iFactLen, bNull) )
				return false;
		}
		break;
	case DT_INT8:
		if ( m_iBindRows == 1 )
		{
			short iVal = *(Int8*)pBuf;
			if ( !OCI_BindShort(m_pStmt, ToBindName(iValueIndex).c_str(), &iVal) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				short *pData = new short[m_iBindRows];
				if ( !OCI_BindArrayOfShorts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			short *pTemp = (short*)(*iter).second;
			pTemp[iRowIndex] = *(Int8*)pBuf;
		}
		break;
	case DT_UINT8:
		if ( m_iBindRows == 1 )
		{
			unsigned short iVal = *(UInt8*)pBuf;
			if ( !OCI_BindUnsignedShort(m_pStmt, ToBindName(iValueIndex).c_str(), &iVal) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				unsigned short *pData = new unsigned short[m_iBindRows];
				if ( !OCI_BindArrayOfUnsignedShorts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			unsigned short *pTemp = (unsigned short*)(*iter).second;
			pTemp[iRowIndex] = *(UInt8*)pBuf;
		}
		break;
	case DT_INT16:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindShort(m_pStmt, ToBindName(iValueIndex).c_str(), (short*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				short *pData = new short[m_iBindRows];
				if ( !OCI_BindArrayOfShorts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			short *pTemp = (short*)(*iter).second;
			pTemp[iRowIndex] = *(short*)pBuf;
		}
		break;
	case DT_UINT16:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindUnsignedShort(m_pStmt, ToBindName(iValueIndex).c_str(), (unsigned short*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				unsigned short *pData = new unsigned short[m_iBindRows];
				if ( !OCI_BindArrayOfUnsignedShorts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			unsigned short *pTemp = (unsigned short*)(*iter).second;
			pTemp[iRowIndex] = *(unsigned short*)pBuf;
		}
		break;
	case DT_INT32:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindInt(m_pStmt, ToBindName(iValueIndex).c_str(), (int*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				int *pData = new int[m_iBindRows];
				if ( !OCI_BindArrayOfInts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			int *pTemp = (int*)(*iter).second;
			pTemp[iRowIndex] = *(int*)pBuf;
		}
		break;
	case DT_UINT32:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindUnsignedInt(m_pStmt, ToBindName(iValueIndex).c_str(), (unsigned int*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				unsigned int *pData = new unsigned int[m_iBindRows];
				if ( !OCI_BindArrayOfUnsignedInts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			unsigned int *pTemp = (unsigned int*)(*iter).second;
			pTemp[iRowIndex] = *(unsigned int*)pBuf;
		}
		break;
	case DT_INT64:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindBigInt(m_pStmt, ToBindName(iValueIndex).c_str(), (big_int*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				big_int *pData = new big_int[m_iBindRows];
				if ( !OCI_BindArrayOfBigInts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			big_int *pTemp = (big_int*)(*iter).second;
			pTemp[iRowIndex] = *(big_int*)pBuf;
		}
		break;
	case DT_UINT64:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindUnsignedBigInt(m_pStmt, ToBindName(iValueIndex).c_str(), (big_uint*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				big_uint *pData = new big_uint[m_iBindRows];
				if ( !OCI_BindArrayOfUnsignedBigInts(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			big_uint *pTemp = (big_uint*)(*iter).second;
			pTemp[iRowIndex] = *(big_uint*)pBuf;
		}
		break;
	case DT_TIME:
		if ( m_iBindRows == 1 )
		{
			OCI_Date *pDate = OCI_DateCreate( OCI_StatementGetConnection(m_pStmt) );
			if ( !pDate )
				return false;
			if ( !OCI_DateFromText(pDate, (otext*)pBuf, "YYYY-MM-DD HH24:MI:SS") )
			{
				OCI_DateFree(pDate);
				return false;
			}
			if ( !OCI_BindDate(m_pStmt, ToBindName(iValueIndex).c_str(), pDate) )
			{
				OCI_DateFree(pDate);
				return false;
			}
			m_mapDateList.insert(std::map<unsigned int, OCI_Date*>::value_type(iValueIndex, pDate));
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				OCI_Date **pDate = OCI_DateArrayCreate(OCI_StatementGetConnection(m_pStmt), m_iBindRows);
				if ( !OCI_BindArrayOfDates(m_pStmt, ToBindName(iValueIndex).c_str(), pDate, 0) )
				{
					OCI_DateArrayFree(pDate);
					return false;
				}
				m_mapDateArrayList.insert(std::map<unsigned int, OCI_Date**>::value_type(iValueIndex, pDate));
			}

			std::map<unsigned int, OCI_Date**>::iterator iter = m_mapDateArrayList.find(iValueIndex);
			if ( iter == m_mapDateArrayList.end() )
				return false;

			OCI_Date **pTemp = (OCI_Date**)(*iter).second;
			if ( !OCI_DateFromText(*pTemp, (otext*)pBuf, "YYYY-MM-DD HH24:MI:SS") )
				return false;
		}
		break;
	case DT_FLOAT:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindFloat(m_pStmt, ToBindName(iValueIndex).c_str(), (float*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				float *pData = new float[m_iBindRows];
				if ( !OCI_BindArrayOfFloats(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			float *pTemp = (float*)(*iter).second;
			pTemp[iRowIndex] = *(float*)pBuf;
		}
		break;
	case DT_DOUBLE:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindDouble(m_pStmt, ToBindName(iValueIndex).c_str(), (double*)pBuf) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				double *pData = new double[m_iBindRows];
				if ( !OCI_BindArrayOfDoubles(m_pStmt, ToBindName(iValueIndex).c_str(), pData, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			double *pTemp = (double*)(*iter).second;
			pTemp[iRowIndex] = *(double*)pBuf;
		}
		break;
	case DT_STRING:
		if ( m_iBindRows == 1 )
		{
			if ( !OCI_BindString(m_pStmt, ToBindName(iValueIndex).c_str(), (otext*)pBuf, iFactLen) )
				return false;
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				char *pData = new char[m_iBindRows*(iBufSize+1)];   // 考虑结束符
				memset(pData, 0x0, m_iBindRows*(iBufSize+1));
				if ( !OCI_BindArrayOfStrings(m_pStmt, ToBindName(iValueIndex).c_str(), (otext*)pData, iBufSize, 0) )
				{
					delete []pData;
					return false;
				}
				m_mapParamsList.insert(std::map<unsigned int, void*>::value_type(iValueIndex, pData));
			}

			std::map<unsigned int, void*>::iterator iter = m_mapParamsList.find(iValueIndex);
			if ( iter == m_mapParamsList.end() )
				return false;

			char *pTemp = (char*)(*iter).second;
			memcpy(pTemp+iBufSize*iRowIndex+iRowIndex, 
				pBuf, 
				iFactLen>iBufSize?iBufSize:iFactLen);
		}
		break;
	case DT_CLOB:
		if ( m_iBindRows == 1 )
		{
			OCI_Lob *pLob = OCI_LobCreate( OCI_StatementGetConnection(m_pStmt), OCI_CLOB );
			if ( !pLob )
				return false;
			if ( OCI_LobWrite(pLob, pBuf, iFactLen) != iFactLen )
			{
				OCI_LobFree(pLob);
				return false;
			}
			if ( !OCI_BindLob(m_pStmt, ToBindName(iValueIndex).c_str(), pLob) )
			{
				OCI_LobFree(pLob);
				return false;
			}
			m_mapLobList.insert(std::map<unsigned int, OCI_Lob*>::value_type(iValueIndex, pLob));
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				OCI_Lob **pLob = OCI_LobArrayCreate(OCI_StatementGetConnection(m_pStmt), OCI_CLOB, m_iBindRows);
				if ( !OCI_BindArrayOfLobs(m_pStmt, ToBindName(iValueIndex).c_str(), pLob, OCI_CLOB, 0) )
				{
					OCI_LobArrayFree(pLob);
					return false;
				}
				m_mapLobArrayList.insert(std::map<unsigned int, OCI_Lob**>::value_type(iValueIndex, pLob));
			}

			std::map<unsigned int, OCI_Lob**>::iterator iter = m_mapLobArrayList.find(iValueIndex);
			if ( iter == m_mapLobArrayList.end() )
				return false;

			OCI_Lob **pTemp = (OCI_Lob**)(*iter).second;
			OCI_LobWrite(pTemp[iRowIndex], pBuf, iFactLen);
		}
		break;
	case DT_BLOB:
		if ( m_iBindRows == 1 )
		{
			OCI_Lob *pLob = OCI_LobCreate( OCI_StatementGetConnection(m_pStmt), OCI_BLOB );
			if ( !pLob )
				return false;
			if ( OCI_LobWrite(pLob, pBuf, iFactLen) != iFactLen )
			{
				OCI_LobFree(pLob);
				return false;
			}
			if ( !OCI_BindLob(m_pStmt, ToBindName(iValueIndex).c_str(), pLob) )
			{
				OCI_LobFree(pLob);
				return false;
			}
			m_mapLobList.insert(std::map<unsigned int, OCI_Lob*>::value_type(iValueIndex, pLob));
		}
		else
		{
			if ( iRowIndex == 0 )
			{
				OCI_Lob **pLob = OCI_LobArrayCreate(OCI_StatementGetConnection(m_pStmt), OCI_BLOB, m_iBindRows);
				if ( !OCI_BindArrayOfLobs(m_pStmt, ToBindName(iValueIndex).c_str(), pLob, OCI_BLOB, 0) )
				{
					OCI_LobArrayFree(pLob);
					return false;
				}
				m_mapLobArrayList.insert(std::map<unsigned int, OCI_Lob**>::value_type(iValueIndex, pLob));
			}

			std::map<unsigned int, OCI_Lob**>::iterator iter = m_mapLobArrayList.find(iValueIndex);
			if ( iter == m_mapLobArrayList.end() )
				return false;

			OCI_Lob **pTemp = (OCI_Lob**)(*iter).second;
			OCI_LobWrite(pTemp[iRowIndex], pBuf, iFactLen);
		}
		break;
	default:
		return false;
	}


	return true;
}

bool COciRecordSet::GetLob( const char* szFieldName, 
						   void* pBuf,
						   unsigned int iBufSize,
						   unsigned int* iFactLen,
						   bool bChar )
{
	OCI_Column *pColumn = OCI_GetColumn2(m_pResultSet, szFieldName);
	if ( !pColumn )
	{
		MYASSERT(pColumn!=NULL);
		return false;
	}

	unsigned int iColumnType = OCI_ColumnGetType(pColumn);
	if ( iColumnType != OCI_CDT_LOB )
	{
		MYASSERT(iColumnType==OCI_CDT_LOB);
		return false;
	}

	OCI_Lob *pLob = OCI_GetLob2(m_pResultSet, szFieldName);
	if ( !pLob )
	{
		MYASSERT(pLob!=NULL);
		return false;
	}

	*iFactLen = (unsigned int)OCI_LobGetLength(pLob);  // 需要考虑一个字符占用的字节数？
	if ( *iFactLen > 0 )
	{
		//OCI_LobRead(pLob, pBuf, iBufSize/**iFactLen>iBufSize?iBufSize:*iFactLen*/);
		if ( bChar )
		{
			unsigned int iBytes = iBufSize;
			OCI_LobRead2(pLob, pBuf, &iBufSize, &iBytes);
			*iFactLen = iBytes;
		}
		else
			OCI_LobRead2(pLob, pBuf, &iBufSize, &iBufSize);
	}

	return true;
}

bool COciRecordSet::GetLob( unsigned int iColumn, 
						   void* pBuf,
						   unsigned int iBufSize,
						   unsigned int* iFactLen,
						   bool bChar )
{
	OCI_Column *pColumn = OCI_GetColumn(m_pResultSet, iColumn);
	if ( !pColumn )
	{
		MYASSERT(pColumn!=NULL);
		return false;
	}

	unsigned int iColumnType = OCI_ColumnGetType(pColumn);
	if ( iColumnType != OCI_CDT_LOB )
	{
		MYASSERT(iColumnType==OCI_CDT_LOB);
		return false;
	}

	OCI_Lob *pLob = OCI_GetLob(m_pResultSet, iColumn);
	if ( !pLob )
	{
		MYASSERT(pLob!=NULL);
		return false;
	}

	*iFactLen = (unsigned int)OCI_LobGetLength(pLob);    // 需要考虑一个字符占用的字节数？
	if ( *iFactLen > 0 )
	{
		//OCI_LobRead(pLob, pBuf, iBufSize/**iFactLen>iBufSize?iBufSize:*iFactLen*/);
		if ( bChar )
		{
			unsigned int iBytes = iBufSize;
			OCI_LobRead2(pLob, pBuf, &iBufSize, &iBytes);
			*iFactLen = iBytes;
		}
		else
			OCI_LobRead2(pLob, pBuf, NULL, &iBufSize);
	}

	return true;
}

std::string COciRecordSet::ToBindName(unsigned int iValueIndex)
{
	std::ostringstream ss;
	ss.str("");
	ss<<":"<<iValueIndex;
	return ss.str();
}

EnumDataType COciRecordSet::GetDataType( unsigned int iColumn, unsigned int iBufSize )
{
	OCI_Column *pColumn = OCI_GetColumn(m_pResultSet, iColumn);
	if ( !pColumn )
	{
		MYASSERT(pColumn!=NULL);
		return DT_UNKNOWN;
	}

	switch ( OCI_ColumnGetType(pColumn) )
	{
	case OCI_CDT_NUMERIC:    // short, int, long long, float, double
		{
			int p = OCI_ColumnGetPrecision(pColumn);
			int s = OCI_ColumnGetScale(pColumn);

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