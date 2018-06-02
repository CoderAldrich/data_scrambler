#pragma once
#include <atlbase.h>
#include <atlstr.h>
#include <WbemIdl.h>
#include <map>
#include <comutil.h>
#pragma comment(lib,"WbemUuid.lib")

using namespace std;
BOOL GetFirstHardDiskSerialNumber(__inout char* pvSerialNumber,__in ULONG ulBufferLen);
BOOL GetWmiObjectJson(_bstr_t bstrWmiObject,_bstr_t& bstrJson);
class CWmiInfo
{
public:
	CWmiInfo(void);
	~CWmiInfo(void);

public:
	HRESULT InitWmi();    //初始化WMI
	HRESULT ReleaseWmi(); //释放

    BOOL GetWMIQueryResult(CString strSQL,map<CStringA, CSimpleArray<CStringA>> &table);
#if 0
	BOOL GetSingleItemInfo(CString,CString,CString&);
	BOOL GetGroupItemInfo(CString,CString[],int,CString&);
	BOOL GetHDID(PCHAR pIDBufer);
#endif
private:
	void VariantToString(const LPVARIANT,CString &) const;//将Variant类型的变量转换为CString
private:
	IEnumWbemClassObject* m_pEnumClsObj;
	IWbemClassObject* m_pWbemClsObj;
	IWbemServices* m_pWbemSvc;
	IWbemLocator* m_pWbemLoc;
};

