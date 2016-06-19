
#include <sstream>

#include "AdoRecordSet.h"
#include "AdoConnection.h"

CAdoRecordSet::CAdoRecordSet(const char* szSql, CAdoConnection* pConn)
{
	pRecordSet_.CreateInstance("ADODB.Recordset");
	open(szSql);
}

CAdoRecordSet::~CAdoRecordSet()
{
	if (IsOpen())
	{
		close();
	}
	if (pRecordSet_ != nullptr)
	{
		pRecordSet_.Release();
		pRecordSet_ = nullptr;
	}
}

bool CAdoRecordSet::Eof()
{
	DB_POINTER_CHECK_RET(pRecordSet_,true);

	try
	{
		if (pRecordSet_->RecordCount > 0 )
		{
			pConn_->ClearError();
			return pRecordSet_->adoEOF == VARIANT_TRUE ? true : false;
		}
		else
		{
			return true;
		}
		
	}
	catch (...)
	{
		pConn_->SetLastError();
		return true;
	}

}

bool CAdoRecordSet::IsOpen()
{
	DB_POINTER_CHECK_RET(pRecordSet_, false);

	try
	{
		if (pRecordSet_->GetState() & adStateOpen)
		{
			pConn_->ClearError();
			return true;
		}
		else
		{
			pConn_->SetLastError();
			return false;
		}
	}
	catch (...)
	{
		pConn_->SetLastError();
		return false;
	}
}

bool CAdoRecordSet::open(const char* szSql)
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);

	try
	{
		if (IsOpen()) 
		{
			close();
		}

		pRecordSet_->PutFilter("");
		pRecordSet_->CursorLocation	=	adUseClient;
		auto hr = pRecordSet_->Open(_variant_t(szSql),_variant_t((IDispatch*)pConn_->GetRawConnRef(), true),
			adOpenStatic, adLockOptimistic, adCmdText);

		if(!SUCCEEDED(hr))   
		{
			return false;
		}

		if (pRecordSet_->RecordCount > 0)
		{
			pRecordSet_->MoveFirst();
		}

		return true;
	}
	catch (...)
	{
		pConn_->SetLastError();
		return false;
	}
}

bool CAdoRecordSet::close()
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);

	try
	{
		if (pRecordSet_->State != adStateClosed)
		{
			if (getEditMode() == adEditNone)
			{
				cancelUpdate();
			}

			pRecordSet_->Close();

			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		pConn_->SetLastError();
	    return false;
	}
}

bool CAdoRecordSet::getEditMode()
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);

	try
	{
		return pRecordSet_->GetEditMode() == adEditNone ? true : false;
	}
	catch (...)
	{
		pConn_->SetLastError();
		return false;
	} 
}

bool CAdoRecordSet::cancelUpdate()
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);

	try
	{
		if (getEditMode() || pRecordSet_->CancelUpdate() == S_OK)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		pConn_->SetLastError();
		return false;
	}
}

bool CAdoRecordSet::MoveNext()
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);

	try
	{
		return SUCCEEDED(pRecordSet_->MoveNext());
	}
	catch (...)
	{
		pConn_->SetLastError();
		return false;
	}
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

FieldPtr CAdoRecordSet::getField(const char* szFieldName)
{
	try
	{
		return getFields()->GetItem(_variant_t(szFieldName));
	}
	catch (...)
	{
		pConn_->SetLastError();
		return nullptr;
	}
}

FieldsPtr CAdoRecordSet::getFields()
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);

	try
	{
		return pRecordSet_->GetFields();
	}
	catch (...)
	{
		pConn_->SetLastError();
		return nullptr;
	} 
}

bool CAdoRecordSet::GetValue( const char* szFieldName, 
							 void* pBuf,
							 unsigned int iBufSize, 
							 unsigned int* iFactLen, 
							 EnumDataType eType /* = DT_UNKNOWN */ )
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);
	DB_POINTER_CHECK_RET(szFieldName,false);
	DB_POINTER_CHECK_RET(pBuf,false);
	if ( iBufSize == 0 )
	{
		MYASSERT(iBufSize>0);
		return false;
	}

	if ( getField(szFieldName) == nullptr )  // 判断是否存在
	{
		pConn_->SetLastError();
		return false;
	}

	unsigned int iValLen = 0;  // 值长度

	auto value = pRecordSet_->GetCollect(_variant_t(szFieldName));
	switch ( eType )
	{
	case DT_INT8:
		iValLen = sizeof(Int8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int8*)pBuf = value.cVal;
		break;
	case DT_UINT8:
		iValLen = sizeof(UInt8);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt8*)pBuf = value.cVal;
		break;
	case DT_INT16:
		iValLen = sizeof(Int16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int16*)pBuf = value.iVal;
		break;
	case DT_UINT16:
		iValLen = sizeof(UInt16);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt16*)pBuf = value.uiVal;
		break;
	case DT_INT32:
		iValLen = sizeof(Int32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int32*)pBuf = value.intVal;
		break;
	case DT_UINT32:
		iValLen = sizeof(UInt32);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt32*)pBuf = value.uintVal;
		break;
	case DT_INT64:
		iValLen = sizeof(Int64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(Int64*)pBuf = value.llVal;
		break;
	case DT_UINT64:
		iValLen = sizeof(UInt64);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(UInt64*)pBuf = value.ullVal;
		break;
	case DT_FLOAT:
		iValLen = sizeof(float);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(float*)pBuf = value.fltVal;
		break;
	case DT_NUMBER:
	case DT_DOUBLE:
		iValLen = sizeof(double);
		CHECK_DATA_LEN_LEGAL(iBufSize, iValLen, iFactLen);
		*(double*)pBuf = value.dblVal;
		break;
	case DT_TIME:
	case DT_STRING:
	    {
			auto szValue = std::string((char*)(_bstr_t)value);

			iValLen = szValue.size();
			iValLen = iValLen > iBufSize ? iBufSize : iValLen;

			memcpy(pBuf,szValue.c_str(),iValLen);
	     	((char*)pBuf)[iValLen] = '\0';
	    }
		break;
	default:
		{
			auto eDataType = getDataType(szFieldName);
			if ( eDataType == DT_UNKNOWN)
				return false;

			if ( !GetValue(szFieldName, pBuf, iBufSize, &iValLen, eDataType) )
				return false;
		}
		break;
	}

	*iFactLen = iValLen;

	return true;
}

bool CAdoRecordSet::GetValue( unsigned int iColumn, 
							 void* pBuf,
							 unsigned int iBufSize, 
							 unsigned int* iFactLen, 
							 EnumDataType eType /* = DT_UNKNOWN */ )
{
	DB_POINTER_CHECK_RET(pRecordSet_,false);
	DB_POINTER_CHECK_RET(pBuf, false);

	if ( iColumn < 1 
		|| iBufSize == 0 )
	{
		MYASSERT(iColumn>0); 
		MYASSERT(iBufSize>0);
		return false;
	}

	// not implement

	return false;
}

unsigned int CAdoRecordSet::GetRowsMoved()
{
	DB_POINTER_CHECK_RET(pRecordSet_,0);

	return 0;
}

unsigned int CAdoRecordSet::GetColumns()
{
	DB_POINTER_CHECK_RET(pRecordSet_,0);

	return 0;
}

const char* CAdoRecordSet::GetColumnName( unsigned int iColIndex )
{
	DB_POINTER_CHECK_RET(pRecordSet_,nullptr);

	return nullptr;
}

void CAdoRecordSet::SetBindRows( unsigned int iSize )
{
}

unsigned int CAdoRecordSet::GetBindRows( )
{
	return 0;
}

bool CAdoRecordSet::BindField( unsigned int iRowIndex,
							  unsigned int iValueIndex,
							  EnumDataType eType, 
							  void *pBuf, 
							  unsigned int iBufSize /* = 0 */,
							  unsigned int iFactLen /* = 0 */,
							  bool bNULL /* = false */ )
{

	return true;
}

std::string CAdoRecordSet::toBindName(unsigned int iValueIndex)
{
	std::ostringstream ss;
	ss.str("");
	ss<<":"<<iValueIndex;
	return ss.str();
}

EnumDataType CAdoRecordSet::getDataType(const char* szFieldName)
{
	auto type = getField(szFieldName)->GetType();
	switch (type)
	{
	case adTinyInt:
		return DT_INT8;
	case adSmallInt:
		return DT_INT16;
	case adInteger:
		return DT_INT32;
	case adBigInt:
		return DT_INT64;
	case adUnsignedTinyInt:
		return DT_UINT8;
	case adUnsignedSmallInt:
		return DT_UINT16;
	case adUnsignedInt:
		return DT_UINT32;
	case adUnsignedBigInt:
		return DT_UINT64;
	case adSingle:
		return DT_FLOAT;
	case adDouble:
		return DT_DOUBLE;
	case adDecimal:
	case adNumeric:
		return DT_NUMBER;
	case adDate:
	case adDBDate:
	case adDBTime:
	case adDBTimeStamp:
		return DT_TIME;
	case adBSTR:
	case adChar:
	case adVarChar:
	case adLongVarChar:
	case adWChar:
	case adVarWChar:
	case adLongVarWChar:
		return DT_STRING;
	case adEmpty:
	case adIUnknown:
	case adBinary:
	case adVarBinary:
	case adLongVarBinary:
	case adChapter:
	case adFileTime:
	case adPropVariant:
	case adVarNumeric:
	case adArray:
	case adCurrency:
	case adBoolean:
	case adError:
	case adUserDefined:
	case adVariant:
	case adIDispatch:
	case adGUID:
	default:
		return DT_UNKNOWN;
	}
}
