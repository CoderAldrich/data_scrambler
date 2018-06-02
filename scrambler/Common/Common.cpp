
#include <windows.h>

#include <regex>
#include <string>
#include <atlbase.h>
#include <atlstr.h>
#include <strsafe.h>
#include <stdio.h>
//#include "error.h"

#include "../inc/define.h"
#include "../inc/error.h"
#include "Common.h"
#include <Sddl.h>
#include <comdef.h>
#include <tlhelp32.h>


#include <hash_map> 

#include <map>
#pragma comment(lib,"Version.lib")


#pragma warning (disable:4995)
#pragma warning (disable:4996)
#define ARRAY_SIZE( arr ) sizeof( arr ) / sizeof( arr[ 0 ] )


using namespace std;  
using namespace stdext;  
#define SCAN_WHOLE_FILE 1
#define HASH_SIZE 40

ULONG g_ulFailedCount = 0;




using namespace std;
using namespace std::tr1;

#define CONST_VERSION_STRING "{FB2A1B93-8111-4738-9533-420D670DE062}"
__inline CHAR *DummyFunc( CHAR *VerString );
static char *XDHY_MAGIC_STRING = DummyFunc( CONST_VERSION_STRING );

__inline CHAR *DummyFunc( CHAR *VerString )
{
    return( VerString );
} /* MyVersion */

//检查常量DLL二进制文件是否包含CONST_VERSION_STRING

BOOL CheckDllVersion(const char* szFileName)
{
    //calc md5 file
    BOOL bResult = FALSE;

    BOOL bOnlyModifiedDate = FALSE;

    LPSTR lpstrCmdLine = GetCommandLineA();
    if(StrStrIA(lpstrCmdLine," /c")||StrStrIA(lpstrCmdLine," -c")||StrStrIA(lpstrCmdLine," /d")||StrStrIA(lpstrCmdLine," -d"))
    {
        bOnlyModifiedDate = TRUE;
    }
    HANDLE hFile = CreateFileA(szFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if( hFile != INVALID_HANDLE_VALUE )
    {
        char *buf=NULL;
        DWORD dwFileSizeHigh;
        BOOL  bMapView = FALSE;
        int dwFileSize = GetFileSize(hFile,&dwFileSizeHigh);
        int nPointerSize = sizeof(char *);
        HANDLE hFileMapOfView=CreateFileMapping(hFile,0,PAGE_READONLY,0,dwFileSize,NULL);
        if(hFileMapOfView)
        {



            buf=(char *)MapViewOfFile(hFileMapOfView,FILE_MAP_READ,0,0,dwFileSize);
            if(buf)
            {
                __try
                {
                    PIMAGE_DOS_HEADER DosHeader;
                    PIMAGE_NT_HEADERS NtHeader;


                    DosHeader = (PIMAGE_DOS_HEADER)buf;
                    if (_strnicmp ((PCHAR)(&DosHeader->e_magic),"MZ",2) == 0)
                    {
                        if (DosHeader->e_lfanew > 0 &&  DosHeader->e_lfanew < (LONG)(dwFileSize - sizeof (IMAGE_NT_HEADERS)))
                        {
                            NtHeader = (PIMAGE_NT_HEADERS)(buf + DosHeader->e_lfanew);

                            if (_strnicmp ((PCHAR)(&NtHeader->Signature),"PE",2) == 0)
                            {
                                PBYTE pbMachine = (PBYTE)(&NtHeader->FileHeader.Machine);
                                if( pbMachine[0]==0x64 && pbMachine[1]==0x86)
                                {
                                    if(nPointerSize ==4)
                                    {
                                        if(bOnlyModifiedDate)
                                        {
                                            printf("%s is 64位dll，不能被32位进程加载\r\n",szFileName);
                                        }
                                        bResult = FALSE;

                                    }
                                    else
                                    {
                                        bResult = TRUE;
                                    }
                                }
                                else
                                {
                                    if(nPointerSize ==8)
                                    {
                                        if(bOnlyModifiedDate)
                                        {
                                            printf("%s is 32位dll，不能被64位进程加载\r\n",szFileName);
                                        } 
                                        bResult = FALSE;

                                    }
                                    else
                                    {
                                        bResult = TRUE;
                                    }
                                }
                            }
                        }
                    }



                    if(bResult)
                    {

                        ULONG i = 0x10000;

                        bResult = FALSE;

                        ULONG ulVersionLen = sizeof(CONST_VERSION_STRING);
                        while(i<dwFileSize-ulVersionLen)
                        {
                            if(memcmp(&buf[i],CONST_VERSION_STRING,ulVersionLen)==0)
                            {
                                bResult = TRUE;
                                break;
                            }
                            i++;
                        }


                    }
                }
                __except(GetExceptionCode()==EXCEPTION_IN_PAGE_ERROR ?
EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    // Failed to read from the view.
                }
                UnmapViewOfFile(buf);
            }



            CloseHandle(hFileMapOfView);
        }
        CloseHandle(hFile);
    }

    else
    {
        if(bOnlyModifiedDate)
        {
            printf("filename:%s is not existed\r\n",szFileName);
        }
    }
    return bResult;
}

//---------------------------------------------------------------------------------------
void DebugView (const char *Format, ...) 
{ 
#ifdef _DEBUG
    DWORD dwLastError= GetLastError();
    PCHAR    OutputString = NULL;//[4096] = {0};
    va_list al; 

    OutputString = (PCHAR)malloc (4096);
    if (OutputString)
    {
        memset (OutputString,0,4096);

        va_start (al,Format); 
        _vsnprintf_s (OutputString,4096,4092,Format,al); 
        va_end (al); 

        OutputDebugStringA ((LPCSTR)OutputString);

        free (OutputString);
    }
#endif

}
//定义的hash函数

ULONG CalcHash(unsigned char* cBuffer,ULONG ulLen)
{
    ULONG hash = 0xAAAAAAAA;
    for(ULONG i = 0; i < ulLen; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ cBuffer[i] ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ cBuffer[i] ^ (hash >> 5)));
        }
    }
    return hash;

}

//获取主程序的工作目录，目录结尾无"\\"
void GetAppPath(char* szAppPath,int nAppPathLen)
{
    if(!szAppPath)
        return;
    if(IsBadWritePtr(szAppPath,nAppPathLen))
        return;
    memset(szAppPath,0,nAppPathLen);
    GetModuleFileNameA(NULL,szAppPath,nAppPathLen);
    char* szSlash = strrchr(szAppPath,'\\');
    if(szSlash)
    {
        *szSlash=0;
    }

}
void ClearLogFile()
{
    char szSystemRoot[MAX_PATH]={0};
    char szLogFile[MAX_PATH]={0};
    GetSystemDirectoryA(szSystemRoot,_countof(szSystemRoot));
    StringCbCopyA(szLogFile,sizeof(szLogFile),szSystemRoot);
    StringCbCatA(szLogFile,sizeof(szLogFile),"\\xdhy*.log");
    WIN32_FIND_DATAA wfd ={0};
    HANDLE hFind = FindFirstFileA(szLogFile,&wfd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        SYSTEMTIME st={0};
        GetLocalTime(&st);
        st.wHour = 0;
        st.wMinute = 0;
        st.wSecond = 0;
        st.wMilliseconds = 0;
        double dtNow;

        SystemTimeToVariantTime(&st,&dtNow);

        do 
        {
            if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
            {
                FILETIME ftLocal;
                SYSTEMTIME stFileWrite={0};
                FileTimeToLocalFileTime(&wfd.ftLastWriteTime,&ftLocal);
                FileTimeToSystemTime(&ftLocal,&stFileWrite);
                stFileWrite.wHour = 0;
                stFileWrite.wMinute = 0;
                stFileWrite.wSecond = 0;
                stFileWrite.wMilliseconds = 0;
                double dtFile;
                SystemTimeToVariantTime(&stFileWrite,&dtFile);
                if((dtNow-dtFile)>7)
                {
                    char szCurrentLogFile[MAX_PATH]={0};
                    StringCbCopyA(szCurrentLogFile,sizeof(szCurrentLogFile),szSystemRoot);
                    StringCbCatA(szCurrentLogFile,sizeof(szCurrentLogFile),"\\");
                    StringCbCatA(szCurrentLogFile,sizeof(szCurrentLogFile),wfd.cFileName);
                    DeleteFileA(szCurrentLogFile);
                }
            }

        } while (FindNextFileA(hFind,&wfd));

        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

}
void DebugViewEx (const char *Format, ...) 
{ 
    HANDLE        hFile;
    LARGE_INTEGER FileSize;
    PCHAR         OutputString = NULL;//[4096] = {0};
    va_list       al; 

    char          szFileName[MAX_PATH]={0};
    char          szAppPath[MAX_PATH]={0};

 
    DWORD dwLastError = GetLastError();
    OutputString = (PCHAR)malloc (4096);
    if (OutputString == NULL)
    {
        if(dwLastError != NOERROR)
            SetLastError(dwLastError);
        return;
    }


    memset (OutputString,0,4096);

    va_start (al,Format); 
    _vsnprintf_s (OutputString,4096,4092,Format,al); 
    va_end (al); 

    OutputDebugStringA ((LPCSTR)OutputString);

    static ULONG s_ulLastHash=0;

    ULONG ulTempHash = (ULONG) CalcHash((unsigned char *)OutputString,(ULONG)strlen(OutputString));
    if(ulTempHash == s_ulLastHash)
    {
        free (OutputString);
        if(dwLastError != NOERROR)
            SetLastError(dwLastError);
        return;
    }
    s_ulLastHash = ulTempHash;


    memset (szAppPath,0,sizeof(szAppPath));

    ClearLogFile();
    char szSystemRoot[MAX_PATH]={0};
   
    GetSystemDirectoryA(szSystemRoot,_countof(szSystemRoot));
    SYSTEMTIME stCurrentTIme={0};
    GetLocalTime(&stCurrentTIme);
  
    StringCbPrintfA(szFileName,sizeof(szFileName),"%s\\xdhysvc_%04u_%02u_%02u.log",szSystemRoot,stCurrentTIme.wYear,stCurrentTIme.wMonth,stCurrentTIme.wDay);

 

 

ReOpen:
    hFile = CreateFileA (szFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {

        if(dwLastError != NOERROR)
            SetLastError(dwLastError);
        free (OutputString);

        return;
    }

    if (GetFileSizeEx (hFile,&FileSize) == FALSE)
    {
        CloseHandle (hFile);
        if(dwLastError != NOERROR)
            SetLastError(dwLastError);
        free (OutputString);

        return;
    }

    if (FileSize.QuadPart >= 1 * 1024 * 1024 * 1024)
    {
        CloseHandle (hFile);
        DeleteFileA (szFileName);

        goto ReOpen;
    }

    SetFilePointer (hFile,0,NULL,FILE_END);

    CHAR       TimeBuffer[1024] = {0};
    SYSTEMTIME SystemTime;
    GetLocalTime (&SystemTime);
    sprintf_s (TimeBuffer,1024,"%04d-%02d-%02d %02d:%02d:%02d ",SystemTime.wYear,SystemTime.wMonth,SystemTime.wDay,SystemTime.wHour,SystemTime.wMinute,SystemTime.wSecond);
    WriteFile (hFile,TimeBuffer,(DWORD)strlen (TimeBuffer),&FileSize.LowPart,NULL);

    const char* szCmdLine = GetCommandLineA();
    WriteFile (hFile,szCmdLine,(DWORD)strlen (szCmdLine),&FileSize.LowPart,NULL);
    
    char szPid [20] ={0};
    StringCbPrintfA(szPid,sizeof(szPid),"(%u)",GetCurrentProcessId());
    WriteFile (hFile,szPid,(DWORD)strlen (szPid),&FileSize.LowPart,NULL);

    FileSize.QuadPart = strlen (OutputString);

    if (FileSize.QuadPart >= 4080)
    {
        CloseHandle (hFile);

        free (OutputString);
        if(dwLastError != NOERROR)
            SetLastError(dwLastError);
        return;
    }

    OutputString[(DWORD)FileSize.QuadPart] = '\r';
    OutputString[(DWORD)FileSize.QuadPart + 1] = '\n';
    OutputString[(DWORD)FileSize.QuadPart + 2] = 0;

    WriteFile (hFile,OutputString,(DWORD)strlen (OutputString),&FileSize.LowPart,NULL);

    CloseHandle (hFile);

    free (OutputString);
    if(dwLastError != NOERROR)
        SetLastError(dwLastError);

    return;
}







void DebugViewW (const CHAR * tszModuleName,const char *Format, ...)
{
    PCHAR         OutputString = NULL;//[4096] = {0};
    va_list       al; 



    DWORD         dwLastError = GetLastError();

    OutputString = (PCHAR)malloc (4096);
    if (OutputString == NULL)
    {
        if(dwLastError != NOERROR)
            SetLastError(dwLastError);
        return;
    }
    memset (OutputString,0,4096);

    va_start (al,Format); 
    _vsnprintf_s (OutputString,4096,4092,Format,al); 
    va_end (al); 

    /*EVENTLOG_SUCCESS
    EVENTLOG_AUDIT_FAILURE    
    EVENTLOG_AUDIT_SUCCESS
    EVENTLOG_ERROR_TYPE
    EVENTLOG_INFORMATION_TYPE
    EVENTLOG_WARNING_TYPE*/


    WriteEventLog(CA2CT(OutputString),EVENTLOG_AUDIT_FAILURE,CA2CT(tszModuleName));


    free (OutputString);
    if(dwLastError != NOERROR)
        SetLastError(dwLastError);

    return;
}
BOOL AddEventSource(LPCTSTR csName, DWORD dwCategoryCount)

{

    HKEY         hRegKey = NULL; 

    DWORD   dwError = 0;

    TCHAR     szPath[ MAX_PATH ];

    CRegKey reg;

    StringCbPrintf(szPath,sizeof(szPath), _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"), csName );

    if(reg.Open(HKEY_LOCAL_MACHINE,szPath)== ERROR_SUCCESS)
    {
        TCHAR tszKeyValue[256] = {0};

        DWORD dwLen = _countof(tszKeyValue);
        if(reg.QueryStringValue(_T("EventMessageFile"),tszKeyValue,&dwLen)== ERROR_SUCCESS)
        {
            reg.Close();
            return TRUE;
        }
        StringCbCopy(szPath,sizeof(szPath),_T("%SystemRoot%\\System32\\eventdll.DLL"));

        reg.SetStringValue(_T("EventMessageFile"),szPath,REG_EXPAND_SZ);
        reg.Close();
    }


    // Create the event source registry key

    dwError = RegCreateKey( HKEY_LOCAL_MACHINE, szPath, &hRegKey );

    if (dwError != 0)

    {

        DbgPrint("RegCreateKey failed for %d",GetLastError());

        return FALSE;

    }



    // Name of the PE module that contains the message resource

    //GetModuleFileName( NULL, szPath, MAX_PATH );
    StringCbCopy(szPath,sizeof(szPath),_T("%SystemRoot%\\System32\\eventdll.DLL"));



    // Register EventMessageFile

    dwError = RegSetValueEx( hRegKey, _T("EventMessageFile"), 0, REG_EXPAND_SZ, (PBYTE) szPath, ((DWORD)_tcslen( szPath) + 1) * sizeof TCHAR ); 

    if (dwError == 0)

    {

        // Register supported event types

        DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 

        dwError = RegSetValueEx( hRegKey, _T("TypesSupported"), 0, REG_DWORD, (LPBYTE) &dwTypes, sizeof dwTypes );



        // If we want to support event categories, we have also to register     the CategoryMessageFile.

        // and set CategoryCount. Note that categories need to have the message ids 1 to CategoryCount!



        if(dwError == 0 && dwCategoryCount > 0 ) 

        {

            dwError = RegSetValueEx( hRegKey, _T("CategoryMessageFile"), 0, REG_EXPAND_SZ, (PBYTE) szPath, ((DWORD)_tcslen( szPath) + 1) * sizeof TCHAR );

            if (dwError == 0)

                dwError = RegSetValueEx( hRegKey, _T("CategoryCount"), 0, REG_DWORD, (PBYTE) &dwCategoryCount, sizeof dwCategoryCount );

        }

    }



    RegCloseKey( hRegKey );



    return TRUE;

}
BOOL WriteEventLog(LPCTSTR szEventMsg, WORD wEventType, LPCTSTR szSourceName)  
{  
    HANDLE hEventLog;  
    BOOL bSuccesful;  
    BOOL bReturn = TRUE; 

    TCHAR szEventName[MAX_PATH] = {0};
    //-------------------------------------------------------------------------  

    if(!szSourceName || !_tcsclen(szSourceName))
    {
        _tcscpy(szEventName,_T("SecurityTerminal"));
    }
    else
    {
        _tcscpy(szEventName,(LPTSTR)szSourceName);
    }

    /*  AddEventSource(szSourceName,1024);*/
    hEventLog = RegisterEventSource(NULL,szEventName);  
    if(NULL == hEventLog)  
    {  
        DbgPrint("RegisterEventSource Failed");  
        return false;  
    }  
    bSuccesful = ReportEvent(hEventLog,  
        wEventType,  
        0,  
        1024,  
        NULL,  
        1,  
        0,  
        &szEventMsg,  
        NULL);  

    if(false == bSuccesful)  
    {  
        DbgPrint("ReportEvent Failed");  
        bReturn = false;  
    }  

    DeregisterEventSource(hEventLog);  

    return bReturn;  

}  


void DebugViewEvent (const char *Format, ...) 
{ 
    PCHAR         OutputString = NULL;//[4096] = {0};
    va_list       al; 



    OutputString = (PCHAR)malloc (4096);
    if (OutputString == NULL)
    {
        return;
    }
    memset (OutputString,0,4096);

    va_start (al,Format); 
    _vsnprintf_s (OutputString,4096,4092,Format,al); 
    va_end (al); 

    free (OutputString);

    return;
}

//---------------------------------------------------------------------------------------
VOID CommonInitializeListHead (IN PLIST_ENTRY ListHead)
{
    if (ListHead != NULL)
    {
        ListHead->Flink = ListHead->Blink = ListHead;
    }
}

BOOL CommonIsListEmpty (IN PLIST_ENTRY ListHead)
{
    if (ListHead)
    {
        return ((ListHead)->Flink == (ListHead));
    }

    return FALSE;
}

VOID CommonInsertHeadList (IN PLIST_ENTRY ListHead,IN PLIST_ENTRY Entry)
{
    if (ListHead != NULL && Entry != NULL)
    {
        PLIST_ENTRY Flink;

        Flink = ListHead->Flink;
        Entry->Flink = Flink;
        Entry->Blink = ListHead;
        Flink->Blink = Entry;
        ListHead->Flink = Entry;
    }
}

PLIST_ENTRY CommonRemoveTailList (IN PLIST_ENTRY ListHead)
{
    if (ListHead != NULL)
    {
        PLIST_ENTRY Blink;
        PLIST_ENTRY Entry;

        if (CommonIsListEmpty (ListHead) == FALSE)
        {
            Entry = ListHead->Blink;
            Blink = Entry->Blink;
            ListHead->Blink = Blink;
            Blink->Flink = ListHead;
            return Entry;
        }		
    }

    return NULL;
}

BOOL CommonRemoveEntryList (IN PLIST_ENTRY Entry)
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    if (Entry)
    {
        Flink = Entry->Flink;
        Blink = Entry->Blink;
        Blink->Flink = Flink;
        Flink->Blink = Blink;

        return (BOOL)(Flink == Blink);
    }

    return FALSE;
}

//---------------------------------------------------------------------------------------
BOOL CommonHexDumpToBuffer (IN PVOID Data,IN DWORD DataLength,IN OUT PVOID Buffer,IN DWORD BufferLength)
{
    PBYTE                 DataOut = NULL;

    CHAR                  TempBuffer[10] = {0};

    DWORD                 HaveDoCount = 0;
    DWORD                 i;

    if (Data == NULL || Buffer == NULL)
    {
        return FALSE;
    }

    if (DataLength > BufferLength / 2)
    {
        return FALSE;
    }

    memset(Buffer,0,BufferLength);

    DataOut = (PBYTE)Data;

    for (i=0;i<DataLength;i++)
    {
        StringCbPrintfA(TempBuffer,sizeof(TempBuffer),"%02x",DataOut[i]);
        StringCbCatA((STRSAFE_LPSTR)Buffer,BufferLength,TempBuffer);
    }

    return TRUE;
}

//如果文件存在，返回TRUE，pWin32FindData返回匹配的文件信息，包括文件大小，文件属性
//否则返回FALSE

BOOL  GetFileInfo(char* szFileName,WIN32_FIND_DATAA* pWin32FindData)
{
    HANDLE hFind = FindFirstFileA(szFileName,pWin32FindData);
    if(INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);
        return TRUE;
    }
    return FALSE;

}

//获得GetLastError错误的对应的错误描述信息

void GetErrorDesc(int ErrorCode,char* szErrDescBuffer,int nErrDescBufferLen)
{
    LPVOID lpMsgBuf=NULL;
    DWORD dwReturn = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        ErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &lpMsgBuf,
        0, NULL );

    if(dwReturn && lpMsgBuf)
    {
        StringCbCopyA(szErrDescBuffer,nErrDescBufferLen,(LPSTR)lpMsgBuf); 
        LocalFree(lpMsgBuf);
    }

}
#if 1
inline size_t CString_Hash_Value(const CStringA& str)   
{   
    size_t value = _HASH_SEED;   
    size_t size  = str.GetLength();   
    if (size > 0) {   
        size_t temp = (size / 16) + 1;   
        size -= temp;   
        for (size_t idx = 0; idx <= size; idx += temp) {   
            value += (size_t)str[(int)idx];   
        }   
    }   
    return(value);   
}  

class CString_Ext_Hash_Compare : public hash_compare<CStringA>   
{   
public:   
    size_t operator()(const CStringA& _Key) const   
    {   
        return((size_t)CString_Hash_Value(_Key));  
    }  

    BOOL operator()(const CStringA& _Keyval1, const CStringA& _Keyval2) const   
    {   
        //return (comp(_Keyval1, _Keyval2));   
        return (BOOL)_Keyval1.CompareNoCase(_Keyval2);
    }   
}; 

typedef struct tagExtMatchResult
{
    BOOL				bRegistryIsPE;
    BOOL                bIsPE;
    BOOL				bFileHeaderIsPE;
    CHAR               *szFileName;
}ExtMatchResult,*PExtMatchResult;

hash_map<CStringA, ExtMatchResult, CString_Ext_Hash_Compare> ExtHash;

BOOL IsBinaryFileByRegistry(const char* szExt,const char * szFileName,FILETIME* pftLastModifiedFileTime,BOOL* pbIsScript)
{
    //带有.分隔符
    BOOL bResult = TRUE;
    static char szLastFileName[MAX_PATH] ={0};
    static FILETIME ftLastFileName;
    if(szLastFileName[0]== '\0')
    {
        StringCbCopyA(szLastFileName,sizeof(szLastFileName),szFileName);
        char * szLastSlash = strrchr(szLastFileName,'\\');
        if(szLastSlash)
        {
            szLastSlash++;
            *szLastSlash = 0;
        }
    }

    if(pbIsScript)
    {
        *pbIsScript = FALSE;
    }
    TCHAR tszKeyPath[256]={0};
    if(stricmp(szExt,".exe")==0)
    {
        return TRUE;
    }
    else if(stricmp(szExt,".dll")==0)
    {
        return TRUE;
    }
    else if(stricmp(szExt,".com")==0)
    {
        return TRUE;
    }
    else if(stricmp(szExt,".sys")==0)
    {
        return TRUE;
    }
    else if(stricmp(szExt,".ime")==0)
    {
        return TRUE;
    }
    else if(stricmp(szExt,".cmd")==0)
    {
        if(pbIsScript)
        {
            *pbIsScript = TRUE;
        }
        return TRUE;
    }
    else if(stricmp(szExt,".bat")==0)
    {
        if(pbIsScript)
        {
            *pbIsScript = TRUE;
        }
        return TRUE;
    }
    else if(stricmp(szExt,".msi")==0)
    {
        if(pbIsScript)
        {
            *pbIsScript = TRUE;
        }
        return TRUE;
    }
    else if(stricmp(szExt,".mst")==0)
    {
        if(pbIsScript)
        {
            *pbIsScript = TRUE;
        }
        return TRUE;
    }
    CStringA strKey;
    strKey = szExt;

    strKey.MakeLower();
    if(ExtHash.find(strKey)!=ExtHash.end())
    {
        if(ExtHash[strKey].bRegistryIsPE)
        {
            return ExtHash[strKey].bIsPE;
        }
        else if(ExtHash[strKey].bFileHeaderIsPE)
        {
            if(StrStrIA(szFileName,szLastFileName)==NULL)
            {

                ExtHash[strKey].bFileHeaderIsPE = FALSE;
                free(ExtHash[strKey].szFileName);
                ExtHash[strKey].szFileName = NULL;
                ExtHash[strKey].bIsPE = FALSE;

                StringCbCopyA(szLastFileName,sizeof(szLastFileName),szFileName);
                char * szLastSlash = strrchr(szLastFileName,'\\');
                if(szLastSlash)
                {
                    szLastSlash++;
                    *szLastSlash = 0;
                }

            }

            else
            {
                //比较时间
                SYSTEMTIME stFile = {0};
                SYSTEMTIME stLastFile = {0};

                FileTimeToSystemTime(pftLastModifiedFileTime,&stFile);
                FileTimeToSystemTime(&ftLastFileName,&stLastFile);
                if(stFile.wYear == stLastFile.wYear &&
                    stFile.wMonth == stLastFile.wMonth &&
                    stFile.wDay == stLastFile.wDay )
                {
                    return ExtHash[strKey].bIsPE;
                }
                else
                {
                    memcpy(&ftLastFileName,pftLastModifiedFileTime,sizeof(FILETIME));
                }

            }

        }


    }

    StringCbPrintf(tszKeyPath,sizeof(tszKeyPath),_T("%s\\PersistentHandler"),CA2T(szExt));
    bool bFind = FALSE;
    CRegKey reg;
    if(reg.Open(HKEY_CLASSES_ROOT,tszKeyPath)==ERROR_SUCCESS)
    {
        TCHAR tszKeyValue[256] = {0};
        DWORD dwKeyLen = _countof(tszKeyValue);
        if (reg.QueryStringValue(NULL,tszKeyValue,&dwKeyLen) ==ERROR_SUCCESS)
        {

            if(_tcsicmp(tszKeyValue,_T("{098f2470-bae0-11cd-b579-08002b30bfeb}"))!=0)
            {
                bFind = TRUE;
                bResult = FALSE;
            }

        }

        reg.Close();
    }
    if(bFind)
    {
        ExtHash[strKey].bRegistryIsPE= TRUE;
        size_t nExtLen = strlen(szExt)+1;
        ExtHash[strKey].szFileName = (char*)malloc(nExtLen);
        if(ExtHash[strKey].szFileName )
        {
            StringCbCopyA(ExtHash[strKey].szFileName,nExtLen,szExt);
        }
        ExtHash[strKey].bIsPE=bResult;

        return bResult;

    }

    else
    {
        BOOL bPEFile=FALSE;
        HANDLE hFile = CreateFileA (szFileName,GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return TRUE;
        }
        else
        {
            unsigned char szPEHearder[2];
            DWORD dwFileSize = GetFileSize(hFile,NULL);
            if (dwFileSize <= 2)
            {
                CloseHandle(hFile);

                return FALSE;
            }
            DWORD dwRead = 0;
            ReadFile(hFile,szPEHearder,sizeof(szPEHearder),&dwRead,NULL);
            if(szPEHearder[0]=='M' && szPEHearder[1]=='Z')
            {
                bPEFile = TRUE;
            }
            else
            {
                bPEFile = FALSE;              
            }
            size_t nExtLen = strlen(szExt)+1;
            ExtHash[strKey].szFileName = (char*)malloc(nExtLen);
            if(ExtHash[strKey].szFileName )
            {
                StringCbCopyA(ExtHash[strKey].szFileName,nExtLen,szExt);
            }
            ExtHash[strKey].bIsPE=bPEFile;
            ExtHash[strKey].bFileHeaderIsPE=TRUE;

            CloseHandle(hFile);
            return bPEFile;
        }



    }
    return bResult;
}
BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize,
                      WORD wLangId, DWORD& dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
    LPWORD lpwData = NULL;
    for (lpwData = (LPWORD) lpData;
        (LPBYTE) lpwData < ((LPBYTE) lpData) + unBlockSize;
        lpwData += 2)
    {
        if (*lpwData == wLangId)
        {
            dwId = *((DWORD *) lpwData);
            return TRUE;
        }
    }

    if (!bPrimaryEnough)
        return FALSE;

    for (lpwData = (LPWORD) lpData;
        (LPBYTE) lpwData < ((LPBYTE) lpData) + unBlockSize;
        lpwData += 2)
    {
        if (((*lpwData) & 0x00FF) == (wLangId & 0x00FF))
        {
            dwId = *((DWORD *) lpwData);
            return TRUE;
        }
    }

    return FALSE;
}

ULONG _GetFileVersionInfo(const TCHAR* tszFileName,PUSHORT pusFileVersionInfo,ULONG ulFileVersionInfoLen)
{
    ULONG ulVersionType = 0;

    DWORD dwHandle =0;
    DWORD dwFileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)
        tszFileName,
        &dwHandle);
    if (!dwFileVersionInfoSize)
        return ulVersionType;

    LPVOID lpData = (LPVOID)new BYTE[dwFileVersionInfoSize];
    if (!lpData)
        return ulVersionType;
    try
    {
        VS_FIXEDFILEINFO FixedFileInfo = {0};

        if (!GetFileVersionInfo((LPTSTR) tszFileName, dwHandle,
            dwFileVersionInfoSize, lpData))
        {
            ulVersionType = FILE_VERSION_NONE;
            throw FALSE;
        }

        // catch default information
        LPVOID lpInfo=NULL;
        UINT unInfoLen=0;
        if (VerQueryValue(lpData, _T("\\"), &lpInfo, &unInfoLen))
        {
            ATLASSERT(unInfoLen == sizeof(FixedFileInfo));
            if (unInfoLen == sizeof(FixedFileInfo))
                memcpy(&FixedFileInfo, lpInfo, unInfoLen);
        }

        lpInfo=NULL;
        unInfoLen=0;
        // find best matching language and codepage
        VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo,
            &unInfoLen);

        DWORD dwLangCode = 0;
        if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(),
            dwLangCode, FALSE))
        {
            if (!GetTranslationId(lpInfo, unInfoLen,
                GetUserDefaultLangID(), dwLangCode, TRUE))
            {
                if (!GetTranslationId(lpInfo, unInfoLen,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                    dwLangCode, TRUE))
                {
                    if (GetTranslationId(lpInfo, unInfoLen,
                        MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL),
                        dwLangCode, TRUE))
                    {					
                        if(lpInfo != NULL)					

                            // use the first one we can get
                            dwLangCode = *((DWORD *) lpInfo);
                        else 
                            return ulVersionType;
                    }
                }
            }
        }

        CString strSubBlock;
        strSubBlock.Format(_T("\\StringFileInfo\\%04X%04X\\"),
            dwLangCode & 0x0000FFFF,
            (dwLangCode & 0xFFFF0000) >> 16);

        // catch string table
#if 0
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("CompanyName")),
            &lpInfo, &unInfoLen))
            strCompanyName = CString((LPCTSTR) lpInfo);
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("FileDescription")),
            &lpInfo, &unInfoLen))
            strFileDescription = CString((LPCTSTR) lpInfo);
#endif
        lpInfo=NULL;
        unInfoLen=0;
        //CString strSubBlock;
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("FileVersion")),
            &lpInfo, &unInfoLen))
        {
            if(((LPCTSTR) lpInfo)[0] != _T('\0'))
            {
                ulVersionType = FILE_VERSION_FILE_TYPE;
                int i=0;

                LPCTSTR lpstTemp = (LPCTSTR)lpInfo;
                TCHAR * tszTemp = NULL;
                while(i<4)
                {
                    pusFileVersionInfo[4-i-1] =  (USHORT)_tcstoul(lpstTemp,&tszTemp,10);
                    lpstTemp = _tcschr(lpstTemp,_T('.'));
                    if(!lpstTemp)
                    {
                        break;
                    }
                    lpstTemp ++;
                    i++;
                }
            }
        }


#if 0
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("InternalName")),
            &lpInfo, &unInfoLen))
            strInternalName = CString((LPCTSTR) lpInfo);
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("LegalCopyright")),
            &lpInfo, &unInfoLen))
            strLegalCopyright = CString((LPCTSTR) lpInfo);
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("OriginalFileName")),
            &lpInfo, &unInfoLen))
            strOriginalFileName = CString((LPCTSTR) lpInfo);
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("ProductName")),
            &lpInfo, &unInfoLen))
            strProductName = CString((LPCTSTR) lpInfo);
#endif
        lpInfo=NULL;
        unInfoLen=0;
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("ProductVersion")),
            &lpInfo, &unInfoLen))
        {
            if(((LPCTSTR) lpInfo)[0] != _T('\0'))
            {

                if(ulVersionType == 0 || ulVersionType == FILE_VERSION_NONE)
                {
                    ulVersionType = FILE_VERSION_PRODUCT_TYPE;
                    int i=0;

                    LPCTSTR lpstTemp = (LPCTSTR)lpInfo;
                    TCHAR * tszTemp = NULL;
                    while(i<4)
                    {
                        pusFileVersionInfo[4-i-1] = (USHORT) _tcstoul(lpstTemp,&tszTemp,10);
                        lpstTemp = _tcschr(lpstTemp,_T('.'));
                        if(!lpstTemp)
                        {
                            break;
                        }
                        lpstTemp ++;
                        i++;
                    }
                }

            }
        }


#if 0
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("Comments")),
            &lpInfo, &unInfoLen))
            strComments = CString((LPCTSTR) lpInfo);
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("LegalTrademarks")),
            &lpInfo, &unInfoLen))
            strLegalTrademarks = CString((LPCTSTR) lpInfo);
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("PrivateBuild")),
            &lpInfo, &unInfoLen))
            strPrivateBuild = CString((LPCTSTR) lpInfo);
        if (VerQueryValue(lpData,
            (LPTSTR) (LPCTSTR) (strSubBlock + _T("SpecialBuild")),
            &lpInfo, &unInfoLen))
            strSpecialBuild = CString((LPCTSTR) lpInfo);
#endif

    }
    catch (...)
    {

    }

    delete[] lpData;

    return ulVersionType;
}
#pragma data_seg("MyData")
#define HASH_TABLE_SIZE 2048
typedef  CSimpleArray<WhiteList_Memory> WhiteList_Memory_Array;
WhiteList_Memory_Array* g_pWhiteListMemoryArray = NULL ;

CRITICAL_SECTION g_ListEntryCriticalSection;
BOOL             g_bInitHash = FALSE;
BOOL             g_bFinish = FALSE;
CSimpleArray<WhiteList_Memory>g_arrayWhiteList;
#pragma data_seg()
#pragma comment(linker,"/SECTION:MyData,RWS")




//BYTE byteBuffer[1024] = {0};
LPTSTR lpszWhiteListSlotName = TEXT("\\\\.\\mailslot\\{FB2A1B94-8112-4738-9534-420D670DE063}");


DWORD WINAPI ReadMailSlotWhiteListProcRead(IN LPVOID lpParameter)
{
    if(!g_pWhiteListMemoryArray)
    {
        return 0;
    }
    WhiteList_Disk_File WhiteListRecord = {0};

    HANDLE hSlot = INVALID_HANDLE_VALUE;


    SECURITY_ATTRIBUTES SecAttr = {0};  
    SECURITY_DESCRIPTOR SecDesc = {0};  



    SecAttr.nLength = sizeof(SecAttr);  
    SecAttr.bInheritHandle = TRUE;  
    SecAttr.lpSecurityDescriptor = &SecDesc;  

    InitializeSecurityDescriptor(&SecDesc, SECURITY_DESCRIPTOR_REVISION);  
    SetSecurityDescriptorDacl(&SecDesc, TRUE, 0, FALSE);  
    hSlot = CreateMailslot(lpszWhiteListSlotName, 
        0,                             // no maximum message size 
        MAILSLOT_WAIT_FOREVER,         // no time-out for operations 
        (LPSECURITY_ATTRIBUTES) &SecDesc); // default security

    if (hSlot == INVALID_HANDLE_VALUE) 
    { 
        //DbgPrintW(EXE_SECURITY_TERMINAL_NAME_A,"CreateMailslot failed with %d\n", GetLastError());
        if (GetLastError()==ERROR_ALREADY_EXISTS)
        {


        }
        return FALSE; 
    } 



    while(!g_bFinish)
    {
        DWORD cbMessage, cMessage, cbRead; 
        BOOL fResult; 

        DWORD cAllMessages; 
        HANDLE hEvent;
        OVERLAPPED ov;

        cbMessage = cMessage = cbRead = 0; 

        hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("ExampleSlot"));
        if( NULL == hEvent )
            return FALSE;
        ov.Offset = 0;
        ov.OffsetHigh = 0;
        ov.hEvent = hEvent;

        //在Windows NT和Windows 2000中，若消息长度超过426个字节，便必须在一个SMB会话之上，

        fResult = GetMailslotInfo( hSlot, (LPDWORD) NULL, &cbMessage, &cMessage,(LPDWORD) NULL);   // no read time-out 

        if (!fResult) 
        { 
            DbgPrintEx("GetMailslotInfo failed with %d.\n", GetLastError()); 
            CloseHandle(hEvent);
            Sleep(10);
            continue;
        } 

        if (cbMessage == MAILSLOT_NO_MESSAGE) 
        { 
            //printf("Waiting for a message...\n"); 
            CloseHandle(hEvent);
            Sleep(10);
            continue;
        } 

        cAllMessages = cMessage; 

        while (cMessage != 0)  // retrieve all messages
        { 

            if(cbMessage != sizeof(WhiteList_Disk_File)) 
            {


                break;
            }
            else if(cbMessage == sizeof(BOOL))
            {

                break;
            }

            else
            {
                fResult = ReadFile(hSlot, 
                    &WhiteListRecord, 
                    cbMessage, 
                    &cbRead, 
                    &ov); 


                if (!fResult) 
                { 
                    DbgPrintEx("ReadFile failed with %d.\n", GetLastError()); 



                    break;
                } 

                if(WhiteListRecord.dwFileSize == 0 && WhiteListRecord.ftLastModified.dwHighDateTime == 0 && WhiteListRecord.ftLastModified.dwLowDateTime == 0
                    )
                {
                    FreeHashTable();


                    break;
                }

                strlwr(WhiteListRecord.FileTitleName);
                ULONG ulHashPos = CalcHash((unsigned char *)WhiteListRecord.FileTitleName,(ULONG)strlen(WhiteListRecord.FileTitleName)) % HASH_TABLE_SIZE;
                PWhiteList_Memory pWhiteListListEntry = NULL;
                pWhiteListListEntry = (PWhiteList_Memory)malloc(sizeof(WhiteList_Memory));

                if(pWhiteListListEntry)
                {
                    ZeroMemory(pWhiteListListEntry,sizeof(WhiteList_Memory));
                    pWhiteListListEntry->dwFileSize = WhiteListRecord.dwFileSize;

                    pWhiteListListEntry->cVersionClass = WhiteListRecord.cVersionClass;
                    memcpy(pWhiteListListEntry->pusFileVersion,WhiteListRecord.pusFileVersion,sizeof(WhiteListRecord.pusFileVersion));
                    memcpy(&pWhiteListListEntry->ftLastModified,&WhiteListRecord.ftLastModified,sizeof(WhiteListRecord.ftLastModified));

                    pWhiteListListEntry->cTimeDateStamp = WhiteListRecord.cTimeDateStamp;
                    pWhiteListListEntry->TimeDateStamp=WhiteListRecord.TimeDateStamp;

                    StringCbCopyA(pWhiteListListEntry->FileHash ,sizeof(pWhiteListListEntry->FileHash),WhiteListRecord.FileHash);

                    StringCbCopyA(pWhiteListListEntry->FileTitleName ,sizeof(pWhiteListListEntry->FileTitleName),WhiteListRecord.FileTitleName);

                    memset(pWhiteListListEntry->FileName,0,sizeof(pWhiteListListEntry->FileName));
                    EnterCriticalSection (&g_ListEntryCriticalSection);
                    g_pWhiteListMemoryArray[ulHashPos].Add(*pWhiteListListEntry);

                    LeaveCriticalSection (&g_ListEntryCriticalSection);

                    free(pWhiteListListEntry);


                }

            }

            fResult = GetMailslotInfo(hSlot,  // mailslot handle 
                (LPDWORD) NULL,               // no maximum message size 
                &cbMessage,                   // size of next message 
                &cMessage,                    // number of messages 
                (LPDWORD) NULL);              // no read time-out 

            if (!fResult) 
            { 
                DbgPrintEx("GetMailslotInfo failed (%d)\n", GetLastError());
                break;
            } 
        } 




        if(hEvent)
        {
            CloseHandle(hEvent);
        }


        Sleep(10);



    }

    return 1;

}

VOID FreeHashTable()
{
    g_bFinish = TRUE;

    if(g_bInitHash)
    {
        char szImportFileName[MAX_PATH] ={0};
        GetAppPath(szImportFileName,sizeof(szImportFileName));
        StringCbCatA(szImportFileName,sizeof(szImportFileName),"\\db\\importsyswhitelist.dat");

        HANDLE hImportFile = CreateFileA (szImportFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if(hImportFile != INVALID_HANDLE_VALUE)
        {


            SetFilePointer (hImportFile,0,NULL,FILE_END);
            DWORD dwWritten = 0;

            WriteFile(hImportFile,g_arrayWhiteList.GetData(),g_arrayWhiteList.GetSize()*sizeof(WhiteList_Memory),&dwWritten,0);

            g_arrayWhiteList.RemoveAll();
            CloseHandle(hImportFile);
        }



        hash_map<CStringA, ExtMatchResult>::iterator iter = ExtHash.begin();

        while (iter != ExtHash.end())
        {
            if(iter->second.szFileName)
            {
                free(iter->second.szFileName);
                iter->second.szFileName = NULL;
            }
            ++iter;

        }

        ExtHash.clear();

        if(g_pWhiteListMemoryArray)
        {
            for(ULONG i=0;i<HASH_TABLE_SIZE;i++)
            {
                g_pWhiteListMemoryArray[i].RemoveAll();
            }
            delete []g_pWhiteListMemoryArray;
            g_pWhiteListMemoryArray = NULL;
        }

    }
}
#endif
LPTSTR lpszSlotNamePushInstall = TEXT("\\\\.\\mailslot\\{FB2A1B93-8111-4738-9533-420D670DE000}");
typedef struct __MAILSLOT_INFO
{
    char  flag[8];
    DWORD cmdtype;
    /**
    *
    * 是否等待进程退出
    */
    BOOL  wait;
    DWORD cmd;
    DWORD errcode;
    char  InstallInfo[256];
}MAILSLOT_INFO,*PMAILSLOT_INFO;
#define PSCMD_COMMUNICATION_MESSAGE 5 
INT32 send_client_install_response( MAILSLOT_INFO *msg, ULONG cb_msg_len )
{
    INT32 ret = 0; 
    INT32 _ret; 
    ULONG written; 

    BOOL bFile = FALSE;

    if(g_ulFailedCount >= 5)
    {

        return ret;
    }
    do 
    {
        SECURITY_ATTRIBUTES SecAttr = {0};  
        SECURITY_DESCRIPTOR SecDesc = {0};  

        SecAttr.nLength = sizeof(SecAttr);  
        SecAttr.bInheritHandle = TRUE;  
        SecAttr.lpSecurityDescriptor = &SecDesc;  

        InitializeSecurityDescriptor(&SecDesc, SECURITY_DESCRIPTOR_REVISION);  
        SetSecurityDescriptorDacl(&SecDesc, TRUE, 0, FALSE);  


        HANDLE hFile = CreateFile(lpszSlotNamePushInstall,GENERIC_WRITE,FILE_SHARE_READ,(LPSECURITY_ATTRIBUTES) &SecAttr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,(HANDLE) NULL); 

        if (hFile == INVALID_HANDLE_VALUE) 
        {
            g_ulFailedCount ++;
            TCHAR tszLogPath[MAX_PATH] ={0};
            GetModuleFileName(NULL,tszLogPath,_countof(tszLogPath));
            TCHAR* tszTemp = _tcsrchr(tszLogPath,_T('\\'));
            if(tszTemp)
            {
                tszTemp++;
                *tszTemp = 0;

                _tcscat(tszLogPath,_T("log"));
                DWORD dwAttribute = GetFileAttributes(tszLogPath);
                if(dwAttribute == INVALID_FILE_ATTRIBUTES)
                {
                    CreateDirectory(tszLogPath,NULL);
                }
                TCHAR tszLogFile[MAX_PATH] ={0};
                SYSTEMTIME st ={0};
                GetLocalTime(&st);
                StringCbPrintf(tszLogFile,sizeof(tszLogFile),_T("%s\\%04d_%02d_%02d.txt"),tszLogPath,
                    st.wYear,
                    st.wMonth,
                    st.wDay);

                hFile = CreateFile(tszLogFile,GENERIC_WRITE,FILE_SHARE_READ,(LPSECURITY_ATTRIBUTES) &SecAttr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,(HANDLE) NULL); 

                if (hFile == INVALID_HANDLE_VALUE) 
                {
                    DbgPrintEx("CreateFile failed with %d.\n", GetLastError()); 
                    return FALSE;
                }
                char*  szCRLN  = "\r\n";
                SetFilePointer (hFile,0,NULL,FILE_END);
                WriteFile(hFile, msg->InstallInfo, (DWORD)strlen(msg->InstallInfo), &written, NULL); 
                _ret = WriteFile(hFile, szCRLN, 2, &written, NULL); 


            }
            /*	DbgPrintEx("CreateFile failed with %d.\n", GetLastError()); 
            return FALSE; */
        } 
        else
        {
            _ret = WriteFile(hFile, msg, cb_msg_len, &written, NULL); 
        }

        CloseHandle(hFile); 



    } while ( FALSE );

    return ret;
}

INT32 output_to_std_out( ULONG type, ULONG err_code, LPCSTR fmt, ... )
{
    INT32 ret = 0; 
    //INT32 _ret; 
    va_list al = NULL; 
    PMAILSLOT_INFO pMailSlot = NULL; 



    do 
    {

        if( fmt == NULL )
        {
            ret = COMMON_INVALID_PARAMETER; 
            break; 
        }

        pMailSlot = ( MAILSLOT_INFO* )malloc( sizeof( MAILSLOT_INFO ) ); 
        if( pMailSlot == NULL )
        {
            ret = COMMON_INSUFFICIENT_RESOURCES; 
            break; 
        }

        pMailSlot->cmd = type; 
        pMailSlot->cmdtype = type; 
        pMailSlot->wait = FALSE; 
        pMailSlot->errcode = err_code; 

        memset( pMailSlot->flag, 0, sizeof( pMailSlot->flag ) ); 


        va_start( al, fmt ); 

        vsnprintf( pMailSlot->InstallInfo, 
            sizeof( pMailSlot->InstallInfo ), 
            fmt, al ); 

        va_end( al ); 


        pMailSlot->InstallInfo[ ARRAYSIZE( pMailSlot->InstallInfo ) - 1 ] = '\0'; 

        //DebugView("%s\r\n",pMailSlot->InstallInfo);

        ret = send_client_install_response( pMailSlot, 
            sizeof( MAILSLOT_INFO ) ); 

        if( ret != 0 )
        {
            break; 
        }

    }while( FALSE );

    if( pMailSlot != NULL )
    {
        free( pMailSlot ); 
    }

    return ret; 
}



DWORD WINAPI SendFileNameToPipe(LPVOID pvParam)
{
    const char* szFileName = (const char* )pvParam;
    output_to_std_out( PSCMD_COMMUNICATION_MESSAGE, 
        0,
        "正在扫描%s", 
        szFileName); 
    free(pvParam);
    pvParam = NULL;

    return TRUE;


}

int findFile(char filePath[],long long &Num,long long &Directory,__in PVOID pvParam,__in pfnCallback fun) 
{  
    char szFind[MAX_PATH];  
    SYSTEMTIME st ={0};

    WIN32_FIND_DATAA *pFindFileData = NULL; 
    pFindFileData = (PWIN32_FIND_DATAA)malloc(sizeof(WIN32_FIND_DATAA));
    if(pFindFileData== NULL)
    {
        return 0; 
    }

    static CHAR szMethod[30] = {0};
    static char szModifiedTime[20] ={0}; 
    static char szLastModifiedTime[20]={0};


    static BOOL bOnlyModifiedDate = FALSE;
    static BOOL bInstallCentersvc = FALSE;

    LPSTR lpstrCmdLine = NULL;
    HANDLE hFind; 
    if(StrStrIA(filePath,"\\*.") == NULL)
    {
        StringCbCopyA(szFind,sizeof(szFind),filePath);  
        if(szFind[strlen(szFind)-1] != '\\')
            StringCbCatA(szFind,sizeof(szFind),"\\*.*");//利用通配符找这个目录下的所以文件，包括目录
        else
            StringCbCatA(szFind,sizeof(szFind),"*.*");//利用通配符找这个目录下的所以文件，包括目录
    }
    else
    {
        StringCbCopyA(szFind,sizeof(szFind),filePath);  
        char* szTemp  = strrchr(filePath,'\\');
        if(*szTemp)
        {
            *szTemp = 0;
        }
    }
    hFind=FindFirstFileA(szFind,pFindFileData); 
    if(INVALID_HANDLE_VALUE==hFind  ) 
    { 
        free(pFindFileData);
        pFindFileData = NULL;
        return 0; 
    }  

    CHAR* szIniFilePath = NULL;
    szIniFilePath = (char*) malloc(MAX_PATH);
    if(szIniFilePath == NULL)
    {
        free(pFindFileData);
        pFindFileData = NULL;
        return 0; 
    }
    //_pgmptr
    //GetModuleFileNameA(NULL,szModuleName,_countof(szModuleName));
    GetLocalTime(&st);

    lpstrCmdLine = GetCommandLineA();


#if 1   
    if(StrStrIA(lpstrCmdLine," /i")||StrStrIA(lpstrCmdLine," -i"))
    {
        bInstallCentersvc = TRUE;

        if(!g_bInitHash  && pvParam)
        {
            GetAppPath(szIniFilePath,MAX_PATH);
            //非程序所在目录执行
            if(StrStrIA(szFind,szIniFilePath) == NULL)
            {
                StringCbCatA(szIniFilePath,MAX_PATH,"\\config.ini");    
                GetPrivateProfileStringA("Setup:SystemProcessPaths","method","normal",szMethod,_countof(szMethod),szIniFilePath);
                if (_stricmp(szMethod,"fast")==0)
                {
                    InitializeCriticalSection (&g_ListEntryCriticalSection);

                    g_pWhiteListMemoryArray = (WhiteList_Memory_Array*) new WhiteList_Memory_Array [HASH_TABLE_SIZE];
                    if(g_pWhiteListMemoryArray)
                    {


                        HANDLE  hThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ReadMailSlotWhiteListProcRead,NULL,0,NULL);
                        CloseHandle(hThread);

                        char szImportFileName[MAX_PATH] ={0};
                        GetAppPath(szImportFileName,sizeof(szImportFileName));
                        StringCbCatA(szImportFileName,sizeof(szImportFileName),"\\db\\importsyswhitelist.dat");

                        HANDLE hImportFile = CreateFileA (szImportFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
                        if(hImportFile != INVALID_HANDLE_VALUE)
                        {


                            DWORD dwFileSize = GetFileSize(hImportFile,NULL);

                            WhiteList_Disk_File WhiteListRecord = {0};
                            WhiteList_Memory    WhiteListMemory = {0};

                            for (ULONG i=0; i<dwFileSize/sizeof(WhiteList_Memory);i++)
                            {
                                DWORD dwRead = 0;
                                BOOL bResult = ReadFile(hImportFile,&WhiteListMemory,sizeof(WhiteList_Memory),&dwRead,NULL);
                                if (bResult && dwRead == sizeof(WhiteList_Memory))
                                {
                                    WhiteListRecord.ftLastModified = WhiteListMemory.ftLastModified;
                                    WhiteListRecord.dwFileSize  = WhiteListMemory.dwFileSize;
                                    WhiteListRecord.cVersionClass  = WhiteListMemory.cVersionClass;
                                    WhiteListRecord.cTimeDateStamp  = WhiteListMemory.cTimeDateStamp;	
                                    memcpy(WhiteListRecord.pusFileVersion,WhiteListMemory.pusFileVersion,sizeof(WhiteListMemory.pusFileVersion));
                                    StringCbCopyA(WhiteListRecord.FileTitleName,sizeof(WhiteListRecord.FileTitleName),WhiteListMemory.FileTitleName);
                                    StringCbCopyA(WhiteListRecord.FileHash,sizeof(WhiteListRecord.FileHash),WhiteListMemory.FileHash);


                                    strlwr(WhiteListRecord.FileTitleName);
                                    ULONG ulHashPos = CalcHash((unsigned char *)WhiteListRecord.FileTitleName,(ULONG)strlen(WhiteListRecord.FileTitleName)) % HASH_TABLE_SIZE;
                                    PWhiteList_Memory pWhiteListListEntry = NULL;
                                    pWhiteListListEntry = (PWhiteList_Memory)malloc(sizeof(WhiteList_Memory));

                                    if(pWhiteListListEntry)
                                    {
                                        ZeroMemory(pWhiteListListEntry,sizeof(WhiteList_Memory));
                                        pWhiteListListEntry->dwFileSize = WhiteListRecord.dwFileSize;

                                        pWhiteListListEntry->cVersionClass = WhiteListRecord.cVersionClass;
                                        memcpy(pWhiteListListEntry->pusFileVersion,WhiteListRecord.pusFileVersion,sizeof(WhiteListRecord.pusFileVersion));
                                        memcpy(&pWhiteListListEntry->ftLastModified,&WhiteListRecord.ftLastModified,sizeof(WhiteListRecord.ftLastModified));

                                        pWhiteListListEntry->cTimeDateStamp = WhiteListRecord.cTimeDateStamp;
                                        pWhiteListListEntry->TimeDateStamp=WhiteListRecord.TimeDateStamp;

                                        StringCbCopyA(pWhiteListListEntry->FileHash ,sizeof(pWhiteListListEntry->FileHash),WhiteListRecord.FileHash);

                                        StringCbCopyA(pWhiteListListEntry->FileTitleName ,sizeof(pWhiteListListEntry->FileTitleName),WhiteListRecord.FileTitleName);

                                        memset(pWhiteListListEntry->FileName,0,sizeof(pWhiteListListEntry->FileName));
                                        EnterCriticalSection (&g_ListEntryCriticalSection);
                                        g_pWhiteListMemoryArray[ulHashPos].Add(*pWhiteListListEntry);

                                        LeaveCriticalSection (&g_ListEntryCriticalSection);
                                        free(pWhiteListListEntry);


                                    }


                                }

                            }
                            CloseHandle(hImportFile);




                        }


                        g_bInitHash = TRUE;
                    }

                }

            }


        }

    }

    else  
#endif
        if(pvParam == NULL)//删除临时文件时用NULL
        {
            bOnlyModifiedDate =  FALSE;
        }
        else if(StrStrIA(lpstrCmdLine," /c")||StrStrIA(lpstrCmdLine," -c"))
        {

            bOnlyModifiedDate = TRUE;

        }

        GetSystemDirectoryA(szIniFilePath,MAX_PATH);

        if(szIniFilePath[strlen(szIniFilePath)-1] != '\\')
        {
            StringCbCatA(szIniFilePath,MAX_PATH,"\\");
        }			

        StringCbCatA(szIniFilePath,MAX_PATH,"FileNameList.ini");


        DWORD dwTimeStamp = 0;
        char * szFileName = NULL;
        szFileName = (char*) malloc(MAX_PATH);
        if(szFileName == NULL)
        {
            free(pFindFileData);
            pFindFileData = NULL;

            free(szIniFilePath);
            szIniFilePath = NULL;

            return 0;
        }

        while(1) 
        {  
            if((pFindFileData->dwFileAttributes &  FILE_ATTRIBUTE_REPARSE_POINT)==FILE_ATTRIBUTE_REPARSE_POINT)
            {
                //DbgPrintEx("junction:%s\\%s",filePath,pFindFileData->cFileName);
                goto find_next;
            }

            if(pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//如果这是目录
            {  
               
                BOOL bDot = FALSE;
                if((pFindFileData->cFileName[0]=='.' && pFindFileData->cFileName[1]=='\0') || (pFindFileData->cFileName[1]=='.' &&  pFindFileData->cFileName[2]=='\0'))
                {
                    bDot = TRUE;
                }


                if(!bDot)//每个目录下有个目录可以返回上一层。如果不是返回目录
                {  
                    Directory++;  

                    StringCbCopyA(szFind,sizeof(szFind),filePath);  
                    if(szFind[strlen(szFind)-1] != '\\')
                    {
                        StringCbCatA(szFind,sizeof(szFind),"\\");//利用通配符找这个目录下的所以文件，包括目录
                    }				
                    StringCbCatA(szFind,sizeof(szFind),pFindFileData->cFileName);

                    DWORD dwTickCountStart = GetTickCount();
                    findFile(szFind,Num,Directory,pvParam,fun);//寻找这个目录下面的文件
                    DWORD dwTickCountEnd = GetTickCount();
                    if((dwTickCountEnd-dwTickCountStart)>=3000)
                    {
                        if(StrStrIA(lpstrCmdLine," /i")||StrStrIA(lpstrCmdLine," -i"))
                        {
                            if(g_ulFailedCount < 5)
                            {

                                char* szAyncMemory = (char*)malloc(strlen(szFind)+1);
                                StringCbCopyA(szAyncMemory,strlen(szFind)+1,szFind);

                                HANDLE  hPipeThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SendFileNameToPipe,szAyncMemory,0,NULL);
                                CloseHandle(hPipeThread);
                            }

                        }

                    }
                  

                

                } 
            } 
            else 
            {  
                Num++;  
                dwTimeStamp = 0;


                StringCbCopyA(szFileName,MAX_PATH,filePath); 
                if(szFileName[strlen(szFileName)-1] != '\\')
                {
                    StringCbCatA(szFileName,MAX_PATH,"\\");
                }			
                StringCbCatA(szFileName,MAX_PATH,pFindFileData->cFileName);



                if(fun)
                {

                    if(bOnlyModifiedDate)   
                    {  /*
                       /C 参数运行时
                       */
                        if(StrStrIA(szFileName,".dll") || StrStrIA(szFileName,".exe")|| StrStrIA(szFileName,".sys"))
                        {
                            SYSTEMTIME stModifiedTime ={0};
                            FileTimeToSystemTime(&pFindFileData->ftLastWriteTime,&stModifiedTime);

                            StringCbPrintfA(szModifiedTime,sizeof(szModifiedTime),"%04d-%02d-%02d %02d:%02d:%02d",stModifiedTime.wYear,stModifiedTime.wMonth,stModifiedTime.wDay,stModifiedTime.wHour,stModifiedTime.wMinute,stModifiedTime.wSecond);

                            GetPrivateProfileStringA(filePath,szFileName,"",szLastModifiedTime,_countof(szLastModifiedTime),szIniFilePath);
                            if(_stricmp(szLastModifiedTime,szModifiedTime)!=0)
                            {
                                printf("filename:%s √\r\n",szFileName);
                                fun(szFileName,pvParam);
                            }

                        }
                    }
                    else
                    {
#if 1                   
                        if(bInstallCentersvc && _stricmp(szMethod,"fast")==0)
                        {
                            /*
                            /i参数运行
                            */


                            char* szDot = strrchr(szFileName,'.');
                            if(!szDot)
                            {
                                //DbgPrintEx("%s without ext,fast scan ignore",szFileName);
                                goto find_next;
                            }
                            else if(g_bInitHash && g_pWhiteListMemoryArray)
                            {
                                BOOL bIsScript = FALSE;
                                if(IsBinaryFileByRegistry(szDot,szFileName,&pFindFileData->ftLastWriteTime,&bIsScript))
                                {

                                    if(!bIsScript)
                                    {
                                        strlwr(pFindFileData->cFileName);
                                        ULONG ulHashPos2 = CalcHash((unsigned char *)pFindFileData->cFileName,(ULONG)strlen(pFindFileData->cFileName)) % HASH_TABLE_SIZE;

                                        WhiteList_Memory_Array g_WhiteListMemory = g_pWhiteListMemoryArray[ulHashPos2];

                                        BOOL bFind = FALSE;
                                        WhiteList_Memory* pListEntry = NULL;
                                        //while (pListEntry != g_pWhiteListMemoryArray[ulHashPos2].end())
                                        for (int ulCount =0; ulCount < g_WhiteListMemory.GetSize();ulCount++)
                                        {
                                            pListEntry = &g_WhiteListMemory[ulCount];
                                            if(pListEntry->dwFileSize == pFindFileData->nFileSizeLow  &&
                                                _stricmp((CHAR*)pListEntry->FileTitleName,pFindFileData->cFileName)==0)
                                            {
                                                if(memcmp(&pListEntry->ftLastModified,&pFindFileData->ftLastWriteTime,sizeof(FILETIME))==0)
                                                {
                                                    if(pListEntry->cTimeDateStamp)
                                                    {
                                                        HANDLE hProgram = CreateFileA (szFileName,GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
                                                        if (hProgram != INVALID_HANDLE_VALUE)
                                                        {


                                                            DWORD dwRead = 0;
                                                            BYTE* byteBuffer = (BYTE*) malloc(1024*sizeof(BYTE));
                                                            if(byteBuffer)
                                                            {
                                                                ReadFile(hProgram,byteBuffer,1024*sizeof(BYTE),&dwRead,NULL);  

                                                                PIMAGE_DOS_HEADER DosHeader = NULL;
                                                                PIMAGE_NT_HEADERS NtHeader = NULL;



                                                                DosHeader = (PIMAGE_DOS_HEADER)byteBuffer;
                                                                if (_strnicmp ((PCHAR)(&DosHeader->e_magic),"MZ",2) == 0)
                                                                {
                                                                    if (DosHeader->e_lfanew < 0 || DosHeader->e_lfanew > (LONG)(dwRead - sizeof (IMAGE_NT_HEADERS)))
                                                                    {

                                                                    }
                                                                    else
                                                                    {
                                                                        NtHeader = (PIMAGE_NT_HEADERS)(byteBuffer + DosHeader->e_lfanew);

                                                                        if((DWORD)NtHeader <((DWORD)byteBuffer + dwRead))
                                                                        {
                                                                            if (_strnicmp ((PCHAR)(&NtHeader->Signature),"PE",2) != 0)
                                                                            {

                                                                            }
                                                                            else{
                                                                                if (FlagOn (NtHeader->FileHeader.Characteristics,IMAGE_FILE_EXECUTABLE_IMAGE)||
                                                                                    FlagOn (NtHeader->FileHeader.Characteristics,IMAGE_FILE_DLL))
                                                                                {
                                                                                    dwTimeStamp = NtHeader->FileHeader.TimeDateStamp;
                                                                                }
                                                                            }
                                                                        }

                                                                    }

                                                                }


                                                                CloseHandle(hProgram);
                                                                free(byteBuffer);
                                                                byteBuffer = NULL;
                                                                if(dwTimeStamp != pListEntry->TimeDateStamp)
                                                                {
                                                                    //DbgPrintEx("%s  TimeDateStamp different",pFindFileData->cFileName);
                                                                    bFind = FALSE;
                                                                    break;
                                                                }
                                                            }



                                                        }

                                                    }

                                                    UCHAR cVersionClass;
                                                    USHORT pusFileVersion[4];
                                                    cVersionClass = (UCHAR)_GetFileVersionInfo(CA2T(szFileName),pusFileVersion,sizeof(pusFileVersion));
                                                    if(cVersionClass == 0 || cVersionClass == FILE_VERSION_NONE)
                                                    {
                                                        if(pListEntry->cVersionClass == cVersionClass)
                                                        {
                                                            bFind = TRUE;
                                                            break;
                                                        }

                                                    }
                                                    else
                                                    {
                                                        if(pListEntry->cVersionClass == cVersionClass
                                                            && memcmp(pListEntry->pusFileVersion,&pusFileVersion,sizeof(pusFileVersion))==0)
                                                        {
                                                            bFind = TRUE;
                                                            break;
                                                        }
                                                        else
                                                        {
                                                            //DbgPrintEx("%s  version different",pFindFileData->cFileName);
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    //DbgPrintEx("%s modified time different",pFindFileData->cFileName);
                                                }


                                            }




                                        }

                                        if (bFind)
                                        {
                                            /*  char szImportFileName[MAX_PATH] ={0};
                                            GetAppPath(szImportFileName,sizeof(szImportFileName));
                                            StringCbCatA(szImportFileName,sizeof(szImportFileName),"\\db\\importsyswhitelist.dat");

                                            HANDLE hImportFile = CreateFileA (szImportFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
                                            if(hImportFile != INVALID_HANDLE_VALUE)
                                            {

                                            SetFilePointer (hImportFile,0,NULL,FILE_END);
                                            DWORD dwWritten = 0;
                                            StringCbCopyA(pListEntry->FileName,sizeof(pListEntry->FileName),szFileName);
                                            WriteFile(hImportFile,pListEntry,sizeof(WhiteList_Memory),&dwWritten,0);

                                            CloseHandle(hImportFile);
                                            }*/
                                            StringCbCopyA(pListEntry->FileName,sizeof(pListEntry->FileName),szFileName);
                                            //DbgPrintEx("%u-%s\r\n",g_arrayWhiteList.GetSize(),pFindFileData->cFileName);

                                            g_arrayWhiteList.Add(*pListEntry);
                                            goto find_next;
                                        }
                                    }


                                }
                                else
                                {

                                    goto find_next;
                                }
                            }




                        }
#endif
                        ULONG ulExtLen = (ULONG) strlen(pFindFileData->cFileName);
                        char* szTemp = pFindFileData->cFileName;
                        if(ulExtLen >=4)
                        {
                            if(StrStrIA(&szTemp[ulExtLen-4],".bat") || StrStrIA(&szTemp[ulExtLen-4],".cmd")||StrStrIA(&szTemp[ulExtLen-4],".msi"))
                            {

                            }
                            else if(_stricmp(szMethod,"fast")==0)
                            {
                                
                                if(pFindFileData->nFileSizeLow < sizeof(IMAGE_NT_HEADERS)+sizeof(IMAGE_DOS_HEADER))
                                {
                                    goto find_next;
                                }
                            }
                        }

                        if(pvParam)
                        {
                            fun(szFileName,pvParam);
                        }
                        else
                        {
                            fun(szFileName,pFindFileData);
                        }


                    }

                    if(bInstallCentersvc || bOnlyModifiedDate)
                    {
                        if(StrStrIA(szFileName,".dll") || StrStrIA(szFileName,".exe")||StrStrIA(szFileName,".sys"))
                        {
                            if((StrStrIA(_pgmptr,filePath) && bInstallCentersvc)||bOnlyModifiedDate)
                            {
                                SYSTEMTIME stModifiedTime ={0};
                                FileTimeToSystemTime(&pFindFileData->ftLastWriteTime,&stModifiedTime);

                                StringCbPrintfA(szModifiedTime,sizeof(szModifiedTime),"%04d-%02d-%02d %02d:%02d:%02d",stModifiedTime.wYear,stModifiedTime.wMonth,stModifiedTime.wDay,stModifiedTime.wHour,stModifiedTime.wMinute,stModifiedTime.wSecond);
                                WritePrivateProfileStringA(filePath,szFileName,szModifiedTime,szIniFilePath);
                            }

                        }


                    }

                }

            }  
find_next:		
            if(!FindNextFileA(hFind,pFindFileData))//寻找下一个文件
            {

                break; 
            }
        }  
        FindClose(hFind);//	关闭句柄
        free(pFindFileData);
        pFindFileData = NULL;

        free(szFileName);
        szFileName = NULL;

        free(szIniFilePath);
        szIniFilePath = NULL;


        return 0; 

}

/************************************************************************/
/*    
BOOL RegexMatch(const char* szRegExp,const char* szInputString,char* szOutputMatchResult,ULONG ulOutputMatchBufferLen)
szRegExp 为正则表达式字符串
szInputString 为要匹配的字符串

*/
/************************************************************************/
BOOL RegexMatch(const char* szRegExp,const char* szInputString)
{
    basic_regex<char> regex(szRegExp,basic_regex<char>::icase);
    if(regex_match(szInputString, regex))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
//从源字符串中搜到匹配的字符串 ，返回true
//调用方法：RegexSearch("(\\w+day)", "Saturday and Sunday");
BOOL RegexSearch(const char* szRegExp,const char* szInputString)
{
    const std::tr1::regex pattern(szRegExp);
    std::string strSource = szInputString;
    std::tr1::smatch result;
    bool bMatch = std::tr1::regex_search(strSource, result, pattern);
    if(bMatch)
    {
        for(size_t i = 1; i < result.size(); ++i)
        {
            string str = result[i];
            DbgPrint("%s",str.c_str() );
        }
    }
    return bMatch;
}

BOOL RegexSearchEx(const char* szRegExp,const char* szInputString,char* szResult,ULONG ulLen)
{
    const std::tr1::regex pattern(szRegExp);
    std::string strSource = szInputString;
    const std::tr1::sregex_token_iterator end;  //需要注意一下这里
    bool bMatch = FALSE;
    ULONG nOffset = 0;
    ULONG nStrLen  = 0;
    memset(szResult,0,ulLen);
    for (std::tr1::sregex_token_iterator iter(strSource.begin(),strSource.end(), pattern);iter != end ; ++iter)
    {
        std::string str = *iter;

        nStrLen = (ULONG)str.size();
        if(nOffset + nStrLen >= ulLen)
            break;
        memcpy(&szResult[nOffset],str.c_str(),nStrLen);
        nOffset += nStrLen;

        szResult[nOffset]=0;
        nOffset++;

        DbgPrint("%s\r\n",str.c_str());
        bMatch = TRUE;
    }


    return bMatch;
}

//char szBuffer[512]={0};
//RegexReplace("(\\ba (a|e|i|u|o))+", "This is a element and this a unique ID.","an $2",szBuffer,sizeof(szBuffer));

void RegexReplace(char* szRegExp,const char* szInput,const char* szDestine,char* szOutput,ULONG ulOutputLen)
{
    std::string text = szInput;
    // regular expression with two capture groups
    const std::tr1::regex pattern(szRegExp);
    // the pattern for the transformation, using the second

    // capture group

    std::string replace = szDestine;
    std::string newtext = std::tr1::regex_replace(text, pattern, replace);
    StringCbCopyA(szOutput,ulOutputLen,newtext.c_str());
    return ;
}
//用于处理xml应答的情况，返回xml串请求一般也是xml编码，无需调用cointialize函数

BOOL SaveBufferAsFile(IN const char* szFileName,IN const char* szBuffer,IN ULONG  ulBufferLen)
{
    BOOL bResult = FALSE;
    
    HANDLE hFile = CreateFileA (szFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwWritten = 0;
        DWORD dwFileSize = GetFileSize(hFile,NULL);
        if(dwFileSize)
        {
            SetFilePointer (hFile,0,NULL,FILE_END);
        }

        WriteFile(hFile,szBuffer,ulBufferLen,&dwWritten,NULL);
        CloseHandle(hFile);
        bResult = TRUE;
    }
    return bResult;
}

ULONG GetExplorerProcessInfo(LPTSTR lpszExplorer)
{
    HANDLE hProcessSnap;

    ULONG ulPID = -1;
    PROCESSENTRY32 pe32;
    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if( hProcessSnap == INVALID_HANDLE_VALUE )
    {
        //printError( TEXT("CreateToolhelp32Snapshot (of processes)") );
        return( -1 );
    }
    // Set the size of the structure before using it.
    pe32.dwSize = sizeof( PROCESSENTRY32 );

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if( !Process32First( hProcessSnap, &pe32 ) )
    {
        //printError( TEXT("Process32First") ); // show cause of failure
        CloseHandle( hProcessSnap );          // clean the snapshot object
        return( -1 );
    }
    // Now walk the snapshot of processes, and
    // display information about each process in turn
    do
    {
        // Retrieve the priority class.
        if(lstrcmpi(lpszExplorer,pe32.szExeFile)==0)
        {
            ulPID = pe32.th32ProcessID;
        }


    } while( Process32Next( hProcessSnap, &pe32 ) );

    CloseHandle( hProcessSnap );
    return( ulPID );
}

BOOL WINAPI GetProcessSID(__in ULONG dwPID,__out char* sid,__in ULONG ulSidLen)
{
    CHAR                           szErrorDesc[256] ={0};
    BOOL                           bResult = FALSE;
    HANDLE                         hToken = NULL;
    TOKEN_PRIVILEGES               tkp;
    HANDLE                         hProcess = NULL; 
    HANDLE                         hUserTokenDup = NULL;
    DWORD                          dwTokenRequired = 0;

    PTOKEN_USER                    pTokenUserInfo = NULL;

    LPSTR                          sidAlloc = NULL;
#pragma region 提升权限

    if(!OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))
    {
        int nErrorCode = GetLastError ();
        GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));


        DbgPrintEx("无法打开存取令牌! error:%s",szErrorDesc)
            bResult = FALSE;
        goto Done;
    }
    LookupPrivilegeValue(NULL,_T("SeDebugPrivilege"),&tkp.Privileges[0].Luid);
    tkp.PrivilegeCount=1;
    tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken,FALSE,&tkp,0,(PTOKEN_PRIVILEGES)NULL,0);
    if(GetLastError()!=ERROR_SUCCESS)
    {
        int nErrorCode = GetLastError ();
        GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));

        DbgPrintEx("调整权限失败!error:%s",szErrorDesc)
            bResult = FALSE;
        goto Done;
    }
#pragma endregion 提升权限



    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,dwPID);
    if (!hProcess)
    {
        int nErrorCode = GetLastError ();
        GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));

        DbgPrintEx ("OpenProcess called error : %08x(%s)",nErrorCode,szErrorDesc);
        bResult = FALSE;
        goto Done;
    }




    if(!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS_P, &hToken))
    {
        int nErrorCode = GetLastError ();
        GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));

        DbgPrintEx ("OpenProcessToken called error : %08x(%s)",nErrorCode,szErrorDesc);
        bResult = FALSE;
        goto Done;
    }

    if(!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hUserTokenDup))
    {

        int nErrorCode = GetLastError ();
        GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));

        DbgPrintEx ("DuplicateTokenEx called error : %08x(%s)",nErrorCode,szErrorDesc);
        bResult = FALSE;
        goto Done;
    }

    if(NULL == GetTokenInformation(hUserTokenDup,TokenUser,NULL,0,&dwTokenRequired))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
        {	

            int nErrorCode = GetLastError ();
            GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));

            DbgPrintEx ("GetTokenInformation called error : %08x(%s)",nErrorCode,szErrorDesc);

            bResult = FALSE;
            goto Done;
        }

        pTokenUserInfo = (PTOKEN_USER)HeapAlloc(GetProcessHeap(),
            HEAP_ZERO_MEMORY, dwTokenRequired);
        if(pTokenUserInfo == NULL)
        {
            int nErrorCode = GetLastError ();
            GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));
            DbgPrintEx ("HeapAlloc called error : %08x(%s)",nErrorCode,szErrorDesc);
            bResult = FALSE;
            goto Done;
        }
    }
    if(FALSE == GetTokenInformation(hUserTokenDup,TokenUser,pTokenUserInfo,dwTokenRequired,&dwTokenRequired))
    {
        int nErrorCode = GetLastError ();
        GetErrorDesc(nErrorCode,szErrorDesc,sizeof(szErrorDesc));
        DbgPrintEx ("GetTokenInformation called error : %08x(%s)",nErrorCode,szErrorDesc);
        bResult = FALSE;
        goto Done;
    }
    PSID pSid = pTokenUserInfo->User.Sid;

    if(ConvertSidToStringSidA(pSid,&sidAlloc))
    {
        StringCbCopyA(sid,ulSidLen,sidAlloc);
        LocalFree(sidAlloc);
        sidAlloc = NULL;
        bResult = TRUE;
    }

Done:

    if(pTokenUserInfo)
    {
        HeapFree(GetProcessHeap(),0,pTokenUserInfo);
        pTokenUserInfo = NULL;
    }
    if(hUserTokenDup)
    {
        CloseHandle(hUserTokenDup);
        hUserTokenDup = NULL;
    }
    if(hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if(hProcess)	
    {
        CloseHandle(hProcess);
        hProcess = NULL;
    }

    return bResult;

}

int Rle_Encode_P(unsigned char * inbuf , int inSize, unsigned char * outbuf, int onuBufSize)
{
    unsigned	char*src =inbuf;
    int i;
    int encSize =0;
    while(src <(inbuf +inSize))
    {
        unsigned	char	byValue =	*	src++;
        i =	1;
        while((*src==byValue)&&(i<63))
        {
            src++;
            i++;
        }
        if((encSize+i+1)>onuBufSize)
            /*
            输出缓冲区空间不够了
            */
        {
            return -1;
        }
        if(i>1)
        {
            outbuf[encSize++]=i|0xC0;
            outbuf[encSize++]=byValue;
        }
        else
        {
            if((byValue&0xC0)	==0xC0)
            {
                outbuf[encSize++]=0xC1;
            }
            outbuf[	encSize++]=byValue;
        }
    }
    return encSize;
}
int Rle_Decode_P(unsigned  char  *  inbuf  ,  int  inSize  ,  unsigned  char  *  outbuf  ,  int  onuBufSize  )
{ 
    unsigned char * src  = inbuf ;
    int i ;
    int decSize  = 0 ;
    int count  = 0 ; 	
    while(src<(inbuf+inSize))
    {
        unsigned char byValue=*src++;
        int	count=1;
        if((byValue&0xC0)==0xC0)
            /*			是否有块属性标志			*/
        {
            count=byValue&0x3F;
            /*			低位是			count*/
            byValue=*src++;
        }
        else
        {
            count=1;
        }
        if((decSize+count)>	onuBufSize)
            /*	输出缓冲区空间不够	了	*/	
        {
            return	-1;
        }
        for(i=0;i<count;i++)
        {	
            outbuf[decSize++]	=	byValue;
        }
    }
    return	decSize	;
}

void PrintBlock(PCHAR pszBuffer,ULONG ulBufferSize)
{
    return PrintBlockEx(pszBuffer,ulBufferSize,FALSE);
}
void PrintBlockEx(PCHAR pszBuffer,ULONG ulBufferSize,BOOL bWriteFile)
{
    CString strBuffer;
    TCHAR tszFormatBuffer[1024] = {0};


    for (ULONG nOffset = 0; nOffset < ulBufferSize; nOffset += 0x10) 
    {

        ULONG nBytes, nIdx;

        //
        // 显示地址
        //
        wsprintf(tszFormatBuffer,_T("%04x "),  nOffset);
        strBuffer += tszFormatBuffer;
        //
        // 显示16进制数据
        //
        nBytes = min(0x10, ulBufferSize - nOffset);


        for (nIdx = 0; nIdx < nBytes; nIdx++) {

            wsprintf(tszFormatBuffer,_T("%02x %s"), ((PUCHAR)pszBuffer)[nOffset + nIdx], ((nIdx + 1) % 0x8) ? _T("") : _T(" "));
            strBuffer += tszFormatBuffer;
        }

        for ( ; nIdx < 0x10; nIdx++) {
            wsprintf(tszFormatBuffer,_T(" %s"), ((nIdx + 1) % 0x8) ? _T("") : _T(" "));
            strBuffer += tszFormatBuffer;
        }

        //
        // 显示ascii格式数据
        //
        for (nIdx = 0; nIdx < nBytes; nIdx++) {
            wsprintf(tszFormatBuffer,_T("%c"), isprint(((PUCHAR)pszBuffer)[nOffset + nIdx]) ? ((PUCHAR)pszBuffer)[nOffset + nIdx] : _T('.'));
            strBuffer += tszFormatBuffer;
        }


        strBuffer += _T("\r\n");
    }

    if(bWriteFile)
    {
        DebugViewEx("%s",CT2CA(strBuffer));
    }
    else
    {
        OutputDebugStringA("/************************************************/\r\n");
        OutputDebugString(strBuffer.GetBuffer());
        strBuffer.ReleaseBuffer();
        OutputDebugStringA("/************************************************/\r\n");
    }
}


int EncodeRle8(void *Src,unsigned long const LenSrc, PBYTE *Dst,unsigned long & LenDstLen)   
{   
    char *pStart=(char *)Src;   
    char *pEnd=pStart+1;   
    // char *pDst=(char *)Dst; //为了引用方便定义了一个变量，不想引用的时候总是进行类型转换   
    CSimpleArray<BYTE>arrayOut;
    unsigned long DstCnt=0,Cnt;   


    while(pEnd<(char *)Src+LenSrc)   
    {   
        //如果前两个数字相同，就搜索这个相等的数字，一直到搜索到第一个不相等的为止   
        if(*pStart==*pEnd)   
        {   
            char* pEnd2=pEnd; 
            do   
            {   
                pEnd++;   
            }while(*pEnd==*pStart&&pEnd<(char *)Src+LenSrc);   


            if((pStart+1)<pEnd2)
            {
                continue;
            }


            //把pStart和pEnd之间的相等的数据存入dst，包括pStart，不包括pEnd   
            //并令pStart=pEnd   
            //哈哈pEnd=pEnd+1；   
            //若相等的数据块的数据个数大于255，则分为几个Encoded Code数据块的形式   
            while(pEnd-pStart>=0x7f)   
            {   


                arrayOut.Add(0x7f);
                DstCnt++;

                arrayOut.Add(*pStart);
                DstCnt++;

                /*memset(szTemp,*pStart,0X7F);
                PrintBlockEx(szTemp,0X7F,FALSE);*/
                pStart+=0x7f;   
            }  

            //把相等的数据块的剩余数据做成一个Encoded Code数据块的形式，但是数据块的长度可能为1   
            //构造长度为1的数据块不知道合理不合理？？   
            if(pEnd>pStart+1)   
            {   
                BOOL bGotoBegin = FALSE;
                if (pEnd- pStart == 2 && pEnd[0] != pStart[0])
                {
                    pEnd++;
                    continue;
                }
                BYTE nCount =(BYTE) (pEnd-pStart);
                nCount &= 0x7f;

                arrayOut.Add(nCount);
                DstCnt++;
                /*memset(szTemp,*pStart,pEnd-pStart);
                PrintBlockEx(szTemp,pEnd-pStart,FALSE);*/
                arrayOut.Add(*pStart);
                DstCnt++;

                pStart=pEnd;   
                pEnd++;   
            }   

        }   
        //else里面搜索不相等的数据，一直搜索到   
        else   
        {   
            while(*pEnd!=*(pEnd+1) && (pEnd+1)<(char *)Src+LenSrc )   
            {   
                pEnd++;   
            } 
            if ((pEnd+2)<(char *)Src+LenSrc )
            {
                if(*(pEnd+1)!=*(pEnd+2))
                {
                    pEnd++;
                    continue;
                }

            }
            //把pStart和pEnd之间的不相等的数据存入dst，包括pStart，不包括pEnd   
            //并令pStart=pEnd+1   
            //并pEnd=pEnd+2   
            //如果数据个数大于255，则用绝对方式生成数据   
            while(pEnd-pStart>=0x7f)   
            {   


                arrayOut.Add(0xff);
                DstCnt++;

                Cnt=0;   
                /*PrintBlockEx(pStart,0X7F,FALSE);*/
                while(Cnt<0x7f)   
                {   

                    arrayOut.Add(*pStart++);
                    DstCnt++;
                    Cnt++;   
                }   
            }   
            //数据个数小于255个，但是大于三个，用绝对方式   
            if(pEnd>pStart+2)   
            {   
                BYTE nCount =(BYTE) (pEnd-pStart);
                nCount |=0x80;

                arrayOut.Add(nCount);
                DstCnt++;
                /*PrintBlockEx(pStart,pEnd-pStart,FALSE);*/
                while(pStart<pEnd)   
                {   

                    arrayOut.Add(*pStart++);
                    DstCnt++;
                }   
            }   
            //如果找到的不相等的数据块的大小不足3个字节，我的处理方法是用Encoded mode   
            //生成一个或者两个Encoded mode的数据块，不知道这样做合理不合理   
            else if(pEnd-pStart)   
            {   
                BOOL bGotoBegin = FALSE;
                while(  (pEnd+2)<(char *)Src+LenSrc && (pEnd-pStart) ==2 && pEnd[0]==pEnd[1]  )
                {
                    if(pEnd[2]!=pEnd[1])
                    {
                        pEnd++;

                        bGotoBegin = TRUE;
                        break;;

                    }
                    else
                    {
                        break;
                    }

                }

                if(bGotoBegin)
                {
                    continue;
                }

retry:
                if(pEnd- pStart >=0x7f)
                {

                    arrayOut.Add(0xff);
                    DstCnt++;
                    /*PrintBlockEx(pStart,0X7F,FALSE);*/
                    for (int i=0;i<0x7f;i++)
                    {
                        arrayOut.Add(*pStart++);
                        DstCnt++;
                    }
                    goto retry;
                }
                else {


                    BYTE nCount  = (BYTE) (pEnd-pStart);
                    nCount |= 0x80;


                    arrayOut.Add(nCount);
                    DstCnt++;

                    //PrintBlockEx(pStart,pEnd-pStart,FALSE);
                    for (int i=0;i<(nCount & 0x7f);i++)
                    {
                        arrayOut.Add(*pStart++);
                        DstCnt++;
                    }
                }

            }   
            pEnd=pStart+1;   
        }   
    }   
    //跳到这里说明pStart或者是最后一位，或者是已经结束，如果是最后一位，把这位存到dst里面就可以了   
    //用一个Encoded mode的数据块存储的   
    if(pStart<(char *)Src+LenSrc && pEnd-1 > pStart)   
    {   

        BYTE nCount  = (BYTE) (pEnd-1-pStart);
        nCount |= 0x80;


        arrayOut.Add(nCount);
        DstCnt++;

        //PrintBlockEx(pStart,pEnd-1-pStart,FALSE);
        for (int i=0;i<(nCount & 0x7f);i++)
        {
            arrayOut.Add(*pStart++);
            DstCnt++;
        }

    }   
    LenDstLen = DstCnt;

    *Dst = (BYTE*)malloc(LenDstLen);
    if(*Dst == NULL)
        return 0;
    else
    {
        memcpy(*Dst,arrayOut.GetData(),LenDstLen);
    }
    //返回值是生成的目标的长度。   
    return DstCnt;   
}

int DecodeRle8(const BYTE *Src,unsigned long const LenSrc, BYTE * &Dst,unsigned long & LenDstLen)
{
    CSimpleArray<BYTE> arrayOut;
    ULONG nOffset =0;

    for (ULONG nOffset=0;nOffset<LenSrc;nOffset++)
    {
        int nCount = Src[nOffset] & 0x80;
        if( nCount == 0)
        {
            for (int j=0;j<Src[nOffset];j++)
            {
                arrayOut.Add(Src[nOffset+1]);
            }
            //	printf(" same:0x%08x Last count:%d size:%d\r\n",nOffset,Src[nOffset],arrayOut.GetSize() );
            //	PrintBlockEx((PCHAR)&Src[nOffset],2,FALSE);
            nOffset ++;


        }
        else
        {
            int nCount = Src[nOffset] & 0x7f;

            //		printf(" different:0x%08x Last count:%d size:%d\r\n",nOffset,Src[nOffset+1] ,arrayOut.GetSize());
            //		PrintBlockEx((PCHAR)&Src[nOffset],Src[nOffset+1]+2,FALSE);

            for (int j=0;j<nCount;j++)
            {
                arrayOut.Add(Src[nOffset+1]);
                nOffset++;
            }


        }
    }
    LenDstLen = arrayOut.GetSize();
    Dst =(BYTE*) malloc(LenDstLen);
    if(!Dst)
        return 0;
    memcpy(Dst,arrayOut.GetData(),LenDstLen);


    return LenDstLen;
}


BOOL SetPrivilege(
                  LPCTSTR lpszPrivilege, // name of privilege to enable/disable
                  BOOL bEnablePrivilege // to enable or disable privilege
                  )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    HANDLE hToken; // access token handle
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

    if ( !LookupPrivilegeValue(
        NULL, // lookup privilege on local system
        lpszPrivilege, // privilege to lookup
        &luid ) ) // receives LUID of privilege
    {
        //printf("LookupPrivilegeValue error: %u\n", GetLastError() );
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if ( !AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES) NULL,
        (PDWORD) NULL) )
    {
        //printf("AdjustTokenPrivileges error: %u\n", GetLastError() );
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {
        //printf("The token does not have the specified privilege. \n");
        return FALSE;
    }

    return TRUE;
}

HRESULT StringReplaceSubStringA(char* szSource,ULONG ulSourceLen,const char* szStringRelace,ULONG ulToReplaceLen,const char * szStringDestinate,ULONG ulDestinateStringLen)
{
    ULONG iPosSourceBefore = 0;
    ULONG iPosSourceAfter = 0;
    ULONG iPosToReplace = 0;
    ULONG iPosDestinate = 0;
    BOOL bFind = FALSE;
    if(!szSource || !szStringRelace || !szStringDestinate)
    {
        return STRSAFE_E_INVALID_PARAMETER;
    }
    if(ulSourceLen < ulToReplaceLen || ulDestinateStringLen > ulToReplaceLen)
    {
        return STRSAFE_E_INVALID_PARAMETER;
    }
    while(ulSourceLen - iPosSourceBefore >=ulToReplaceLen)
    {
        if(_strnicmp(&szSource[iPosSourceBefore],szStringRelace,ulToReplaceLen) != 0)
        {
            if(iPosSourceAfter < iPosSourceBefore)
            {
                szSource[iPosSourceAfter]=szSource[iPosSourceBefore];
            }

            iPosSourceBefore++;
            iPosSourceAfter++;

        }
        else
        {
            bFind = TRUE;

            memcpy(&szSource[iPosSourceAfter],szStringDestinate,ulDestinateStringLen);
            iPosSourceAfter += ulDestinateStringLen;
            iPosSourceBefore += ulToReplaceLen;
        }
    }
    while(iPosSourceBefore < ulSourceLen)
    {
        szSource[iPosSourceAfter] =  szSource[iPosSourceBefore];
        iPosSourceAfter++;
        iPosSourceBefore++;
    }
    if(bFind)
    {
        szSource[iPosSourceAfter] = 0;
    }
    return bFind?S_OK:S_FALSE;

}
//调用方法示例：
//char szBuffer[33]={0};
//char szDstBuffer[40]={0};
//Decimal2Binary(0xabcd0e0f,szBuffer,sizeof(szBuffer));
//SeparateBinary(szBuffer,szDstBuffer,sizeof(szDstBuffer));
//ULONG ulTemp = 0;
//BinaryString2DecimalInteger(szDstBuffer,strlen(szDstBuffer),&ulTemp);
ULONG my_ntohl (ULONG netlong)
{

    char szHexDump[9] = {0};
    char* szTemp = (char*)&netlong;
    ULONG ulCount = 0;
    for (int i=0;i<4;i++)
    {
        BYTE byTemp = szTemp[4-i-1];

        BYTE HiByte = byTemp >> 4;
        if((HiByte & 0x0f) > 9)
            szHexDump[ulCount] = (HiByte & 0x0f) -0x0a + 'A';
        else 
            szHexDump[ulCount] = (HiByte & 0x0f)  + '0';
        ulCount++;

        BYTE LoByte = byTemp & 0x0f;
        if((LoByte & 0x0f) > 9)
            szHexDump[ulCount] = (LoByte & 0x0f) -0x0a + 'A';
        else 
            szHexDump[ulCount] = (LoByte & 0x0f)  + '0';
        ulCount++;



    }
    szHexDump[ulCount]  = 0;
    netlong = szHexDump[0]>='A'?szHexDump[0]-'A'+0x0a:szHexDump[0]-'0';
    ULONG ulShiftCount =0;
    for(int i= 1; i<8;i++)
    {
        if(netlong > 0){
            ULONG ulTemp =(szHexDump[i]>='A'?szHexDump[i]-'A'+0x0a:szHexDump[i]-'0');
            ULONG ulShiftTempCount = 0;
            do 
            {
                ulTemp = ulTemp << 4;
                ulShiftTempCount++;

            } while (ulShiftTempCount <= ulShiftCount);

            netlong = netlong + ulTemp;
            ulShiftCount ++;

        }
        else
            netlong = netlong + (szHexDump[i]>='A'?szHexDump[i]-'A'+0x0a:szHexDump[i]-'0');
    }



    return netlong;
}
HRESULT Decimal2Binary(ULONG ulDecimal, char* szBuffer,ULONG ulBufferLen)
{
    if(ulDecimal <2)
    {
        StringCbCatA(szBuffer,ulBufferLen,ulDecimal==1 ?"1":"0");

        return S_OK;
    }
    if(ulDecimal >=2)
    {
        Decimal2Binary(ulDecimal/2,szBuffer,ulBufferLen);
        StringCbCatA(szBuffer,ulBufferLen,ulDecimal%2 == 1 ?"1":"0");


    }
    return S_OK;
}
HRESULT SeparateBinary(const char* szSource,char * szDstBuffer,ULONG ulDstLen)
{
    ULONG nSourceLen = (ULONG)strlen(szSource);
    int nSeparateCount = nSourceLen % 4;
    ULONG iCount = 0;
    char szBuffer[2] = {0};
    for(iCount =0 ; iCount < nSourceLen;iCount++)
    {
        if(iCount && (iCount % 4) == 0 )
        {
            StringCbCatA(szDstBuffer,ulDstLen,",");
        }
        szBuffer[0] = szSource[iCount];
        StringCbCatA(szDstBuffer,ulDstLen,szBuffer);

    }

    return S_OK;

}

HRESULT BinaryString2DecimalInteger(const char* szSource,ULONG ulSourceLen,PULONG pulDecimalInteger)
{
    //0x0031f640 "1111,0000,1110,0000,1101,1100,1011,1010"
    //0xf0e0dcba
    char* szIndex[] =
    {
        "0000",
        "0001",
        "0010",
        "0011",
        "0100",
        "0101",
        "0110",
        "0111",
        "1000",
        "1001",
        "1010",
        "1011",
        "1100",
        "1101",
        "1110",
        "1111",
    };
    char * pszTemp = (char*)szSource;

    ULONG ulTemp = 0;

    while (pszTemp <(szSource+ulSourceLen))
    {
        if(pszTemp[0] == ',')
        {
            pszTemp++;
            continue;
        }


        for (ULONG i= 0;i< ARRAYSIZE(szIndex);i++)
        {
            if(_strnicmp(pszTemp,szIndex[i],4)==0||atoi(szIndex[i])==atoi(pszTemp))
            {

                ulTemp = ulTemp * 16+i;

                break;
            }
        }
        pszTemp += 4;
    }
    if(pulDecimalInteger)
    {
        *pulDecimalInteger = ulTemp;
    }
    return S_OK;
}
HANDLE SpawnAndRedirect(LPCTSTR commandLine, HANDLE *hStdOutputReadPipe, LPCTSTR lpCurrentDirectory)
{
    HANDLE hStdOutputWritePipe, hStdOutput, hStdError;
    CreatePipe(hStdOutputReadPipe, &hStdOutputWritePipe, NULL, 0);	// create a non-inheritable pipe
    DuplicateHandle(GetCurrentProcess(), hStdOutputWritePipe,
        GetCurrentProcess(), &hStdOutput,	// duplicate the "write" end as inheritable stdout
        0, TRUE, DUPLICATE_SAME_ACCESS);
    DuplicateHandle(GetCurrentProcess(), hStdOutput,
        GetCurrentProcess(), &hStdError,	// duplicate stdout as inheritable stderr
        0, TRUE, DUPLICATE_SAME_ACCESS);
    CloseHandle(hStdOutputWritePipe);								// no longer need the non-inheritable "write" end of the pipe

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);	// (this is bad on a GUI app)
    si.hStdOutput = hStdOutput;
    si.hStdError  = hStdError;
    si.wShowWindow = SW_HIDE;						// IMPORTANT: hide subprocess console window
    TCHAR commandLineCopy[1024];					// CreateProcess requires a modifiable buffer
    _tcscpy(commandLineCopy, commandLine);

    TCHAR tszWorkDirectory[MAX_PATH]={0};
    if(!lpCurrentDirectory)
    {
        _tcscpy(tszWorkDirectory,commandLine);
        if(tszWorkDirectory[1]==_T(':') && 
            tszWorkDirectory[2]==_T('\\'))
        {

            TCHAR* tszTemp =_tcsrchr(tszWorkDirectory,_T('\\'));
            if(tszTemp)
                *tszTemp = 0;
            lpCurrentDirectory = tszWorkDirectory;
        }
    }

    if (!CreateProcess(	NULL, commandLineCopy, NULL, NULL, TRUE,
        CREATE_NEW_CONSOLE, NULL, lpCurrentDirectory, &si, &pi))
    {
        CloseHandle(hStdOutput);
        CloseHandle(hStdError);
        CloseHandle(*hStdOutputReadPipe);
        *hStdOutputReadPipe = INVALID_HANDLE_VALUE;
        return NULL;
    }

    CloseHandle(pi.hThread);
    CloseHandle(hStdOutput);
    CloseHandle(hStdError);
    return pi.hProcess;
}


INT ExecCmd(TCHAR* szCmdLine,TCHAR* szFileName)
{

    HANDLE hFile = CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,
        NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    HANDLE hOutput, hProcess;


    hProcess = SpawnAndRedirect(szCmdLine, &hOutput, NULL);


    if (!hProcess) 
    {
        CloseHandle(hFile);
        DeleteFile(szFileName);
        return 0;
    }

    CHAR buffer[128];
    DWORD read;
    DWORD dwDummy=0 ;
    while (ReadFile(hOutput, buffer, sizeof(buffer), &read, NULL))
    {

        WriteFile(hFile, buffer, read, &dwDummy, NULL);
    }
    CloseHandle(hOutput);
    CloseHandle(hProcess);
    CloseHandle(hFile);
    return 1;
}
typedef void (WINAPI *pfnGetNativeSystemInfo)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *pfnGetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
#define PRODUCT_PROFESSIONAL	0x00000030
#define VER_SUITE_WH_SERVER	0x00008000
BOOL GetOSVersion(char* str, int bufferSize,OSVERSIONINFOEXA &osvi)
{

    SYSTEM_INFO si = {0};
    BOOL bOSVERSIONINFOEXA = FALSE;
    DWORD dwType = 0; 
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA)); 

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
    bOSVERSIONINFOEXA = GetVersionExA((OSVERSIONINFOA*) &osvi); 
    if(bOSVERSIONINFOEXA == 0)
    {
        return FALSE; // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    }
    pfnGetNativeSystemInfo fnGetNativeSystemInfo = (pfnGetNativeSystemInfo) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
    if(NULL != fnGetNativeSystemInfo)
    {
        fnGetNativeSystemInfo(&si);
    }
    else
    {
        GetSystemInfo(&si); // Check for unsupported OS
    }
    if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4 ) {
        return FALSE;
    } 
    StringCbCopyA(str,bufferSize,"Microsoft ");
    if ( osvi.dwMajorVersion == 6 )
    {
        if( osvi.dwMinorVersion == 0 )
        {
            if( osvi.wProductType == VER_NT_WORKSTATION )
            {
                StringCbCatA(str,bufferSize,"Windows Vista");
            }
            else
            {
                StringCbCatA(str,bufferSize,"Windows Server 2008 ");
            }
        }  
        if ( osvi.dwMinorVersion == 1 )
        {
            if( osvi.wProductType == VER_NT_WORKSTATION )
            {
                StringCbCatA(str,bufferSize,"Windows 7 ");
            }
            else 
            {
                StringCbCatA(str,bufferSize,"Windows Server 2008 R2 ");
            }
        }  
        pfnGetProductInfo fnGetProductInfo = (pfnGetProductInfo) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
        fnGetProductInfo( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);  
        switch( dwType )
        {
        case PRODUCT_ULTIMATE:
            StringCbCatA(str,bufferSize,"Ultimate Edition");
            break;
        case PRODUCT_PROFESSIONAL:
            StringCbCatA(str,bufferSize,"Professional");
            break;
        case PRODUCT_HOME_PREMIUM:
            StringCbCatA(str,bufferSize,"Home Premium Edition");
            break;
        case PRODUCT_HOME_BASIC:
            StringCbCatA(str,bufferSize,"Home Basic Edition");
            break;
        case PRODUCT_ENTERPRISE:
            StringCbCatA(str,bufferSize,"Enterprise Edition");
            break;
        case PRODUCT_BUSINESS:
            StringCbCatA(str,bufferSize,"Business Edition");
            break;
        case PRODUCT_STARTER:
            StringCbCatA(str,bufferSize,"Starter Edition");
            break;
        case PRODUCT_CLUSTER_SERVER:
            StringCbCatA(str,bufferSize,"Cluster Server Edition");
            break;
        case PRODUCT_DATACENTER_SERVER:
            StringCbCatA(str,bufferSize,"Datacenter Edition");
            break;
        case PRODUCT_DATACENTER_SERVER_CORE:
            StringCbCatA(str,bufferSize,"Datacenter Edition (core installation)");
            break;
        case PRODUCT_ENTERPRISE_SERVER:
            StringCbCatA(str,bufferSize,"Enterprise Edition");
            break;
        case PRODUCT_ENTERPRISE_SERVER_CORE:
            StringCbCatA(str,bufferSize,"Enterprise Edition (core installation)");
            break;
        case PRODUCT_ENTERPRISE_SERVER_IA64:
            StringCbCatA(str,bufferSize,"Enterprise Edition for Itanium-based Systems");
            break;
        case PRODUCT_SMALLBUSINESS_SERVER:
            StringCbCatA(str,bufferSize,"Small Business Server");
            break;
        case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
            StringCbCatA(str,bufferSize,"Small Business Server Premium Edition");
            break;
        case PRODUCT_STANDARD_SERVER:
            StringCbCatA(str,bufferSize,"Standard Edition");
            break;
        case PRODUCT_STANDARD_SERVER_CORE:
            StringCbCatA(str,bufferSize,"Standard Edition (core installation)");
            break;
        case PRODUCT_WEB_SERVER:
            StringCbCatA(str,bufferSize,"Web Server Edition");
            break;
        }
    }
    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
    {
        if( GetSystemMetrics(SM_SERVERR2) )
        {
            StringCbCatA(str,bufferSize,"Windows Server 2003 R2, ");
        }
        else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
        {
            StringCbCatA(str,bufferSize,"Windows Storage Server 2003");
        }
        else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
        {
            StringCbCatA(str,bufferSize,"Windows Home Server");
        }
        else if( osvi.wProductType == VER_NT_WORKSTATION &&si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
        {
            StringCbCatA(str,bufferSize,"Windows XP Professional x64 Edition");
        }
        else
        {
            StringCbCatA(str,bufferSize,"Windows Server 2003, ");  // Test for the server type.
        }
        if ( osvi.wProductType != VER_NT_WORKSTATION )
        {
            if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                {
                    StringCbCatA(str,bufferSize,"Datacenter Edition for Itanium-based Systems");
                }
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                {
                    StringCbCatA(str,bufferSize,"Enterprise Edition for Itanium-based Systems");
                }
            }  
            else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                {
                    StringCbCatA(str,bufferSize,"Datacenter x64 Edition");
                }
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                {
                    StringCbCatA(str,bufferSize,"Enterprise x64 Edition");
                }
                else
                {
                    StringCbCatA(str,bufferSize,"Standard x64 Edition");
                }
            }  
            else
            {
                if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                {
                    StringCbCatA(str,bufferSize,"Compute Cluster Edition");
                }
                else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                {
                    StringCbCatA(str,bufferSize,"Datacenter Edition");
                }
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                {
                    StringCbCatA(str,bufferSize,"Enterprise Edition");
                }
                else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                {
                    StringCbCatA(str,bufferSize,"Web Edition");
                }
                else 
                {
                    StringCbCatA(str,bufferSize,"Standard Edition");
                }
            }
        }
    }
    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
    {
        StringCbCatA(str,bufferSize,"Windows XP ");
        if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
        {
            StringCbCatA(str,bufferSize,"Home Edition");
        }
        else 
        {
            StringCbCatA(str,bufferSize,"Professional");
        }
    }
    if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
    {
        StringCbCatA(str,bufferSize,"Windows 2000 ");  
        if ( osvi.wProductType == VER_NT_WORKSTATION )
        {
            StringCbCatA(str,bufferSize,"Professional");
        }
        else 
        {
            if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
            {
                StringCbCatA(str,bufferSize,"Datacenter Server");
            }
            else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
            {
                StringCbCatA(str,bufferSize,"Advanced Server");
            }
            else
            {
                StringCbCatA(str,bufferSize,"Server");
            }
        }
    } // Include service pack (if any) and build number. if(wcslen(osvi.szCSDVersion) > 0) {
    StringCbCatA(str,bufferSize, osvi.szCSDVersion);

    CHAR szVersion[] = {0};
    StringCbPrintfA(szVersion,sizeof(szVersion),"(build %u)",osvi.dwBuildNumber);
    StringCbCatA(str,bufferSize,szVersion);
    if ( osvi.dwMajorVersion >= 6 ) {
        if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
        {
            StringCbCatA(str,bufferSize,", 64-bit");
        }
        else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
        {
            StringCbCatA(str,bufferSize,", 32-bit");
        }
    }

    return TRUE; 
}
VOID RemoteDbgBreak()
{
#ifdef _DEBUG
    LPTSTR lpstCmdLine = GetCommandLine();
    if(StrStrIA(_pgmptr,"CenterSvc") && 
        (StrStrI(lpstCmdLine,_T("/srv")) == NULL || StrStrI(lpstCmdLine,_T("-srv")) == NULL)
        )
    {

        printf("Press Any key to continue");
        int ch;
        ch=getchar();
    }
    else
    {
        BOOL bWait = TRUE;
        CHAR                            IniFilePath[MAX_PATH] = {0};
        GetAppPath(IniFilePath,sizeof(IniFilePath));
        StringCbCatA(IniFilePath,sizeof(IniFilePath),"\\Config.ini");

        BOOL bBreak = FALSE;
        while(bWait)
        {
            bBreak = GetPrivateProfileIntA("Platform","debugbreak",0,IniFilePath);
            if (bBreak)
            {
                break;
            }
            Sleep(100);
        }
    }

#endif
}
BOOL RecursiveMoveFile(TCHAR * pFrom,TCHAR * pTo)
{
    SHFILEOPSTRUCT FileOp={0};
    FileOp.fFlags = FOF_NOCONFIRMATION| //不出现确认对话框
        FOF_SILENT|FOF_NOERRORUI; //需要时直接创建一个文件夹,不需用户确定
    TCHAR tszSrcFileName[MAX_PATH]={0};
    TCHAR tszDstFileName[MAX_PATH]={0};
    ZeroMemory(tszSrcFileName,sizeof(tszSrcFileName));
    ZeroMemory(tszDstFileName,sizeof(tszDstFileName));
    StringCbCopy(tszSrcFileName,sizeof(tszSrcFileName),pFrom);
    StringCbCopy(tszDstFileName,sizeof(tszDstFileName),pTo);

    FileOp.hNameMappings = NULL;

    FileOp.hwnd = NULL;

    FileOp.lpszProgressTitle = NULL;

    FileOp.pFrom = tszSrcFileName;
    FileOp.pTo = tszDstFileName;
    FileOp.wFunc = FO_RENAME;

    return SHFileOperation(&FileOp) ==0;
}
BOOL RecursiveDeleteFile(TCHAR * tszFileName)
{
    TCHAR tszExpandString[MAX_PATH] = {0};
    SHFILEOPSTRUCT FileOp={0};
    FileOp.fFlags = FOF_SILENT|FOF_NOERRORUI |   //允许放回回收站
        FOF_NOCONFIRMATION; //不出现确认对话框
    if(tszFileName[0] == _T('%'))
    {

        ExpandEnvironmentStrings(tszFileName,tszExpandString,sizeof(tszExpandString)/sizeof(tszExpandString[0]));
        FileOp.pFrom = tszExpandString;   
    }
    else
    {
        StringCbCopy(tszExpandString,sizeof(tszExpandString),tszFileName);
        FileOp.pFrom = tszExpandString;
    }

    FileOp.pTo = NULL;      //一定要是NULL
    FileOp.wFunc = FO_DELETE;    //删除操作

    return  SHFileOperation(&FileOp)==0;

}
BOOL GetTempFileNameEx(LPTSTR tszFileName)
{
    TCHAR OutPath[MAX_PATH] ={0};
    int StrLen = GetTempPath(_countof(OutPath), OutPath);
    GetTempFileName(OutPath,_T("~xx"),16,tszFileName);
    return TRUE;
}
BOOL GetPreviousDriverName(char* szDriverName,ULONG ulDriverNameLen)
{
#pragma  region 查找fsxdhy 驱动名称

        BOOL     bFind = FALSE;
        DWORD    cSubKeys=0;               // number of subkeys 
        DWORD    cbMaxSubKey;              // longest subkey size 
        HKEY    hKey = NULL;
        LSTATUS hr = ERROR_SUCCESS;
        hr=RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services", 0, KEY_ALL_ACCESS , &hKey); 
        if(hr==ERROR_SUCCESS) 
        { 
            hr = RegQueryInfoKeyA(
                hKey,
                NULL,                // buffer for class name 
                NULL,           // size of class string 
                NULL,                    // reserved 
                &cSubKeys,               // number of subkeys 
                &cbMaxSubKey,            // longest subkey size 
                NULL,            // longest class string 
                NULL,                // number of values for this key 
                NULL,            // longest value name 
                NULL,         // longest value data 
                NULL,   // security descriptor 
                NULL);       // last write time 
            if(hr == ERROR_SUCCESS)
            {
                if (cSubKeys)
                {
                    #define MAX_KEY_LENGTH 255
                    CHAR    achKey[MAX_KEY_LENGTH]={0};   // buffer for subkey name
                    DWORD cbName = MAX_KEY_LENGTH;
                    CSimpleArray<CStringA> m_strKeyNameArray;
                  
                    for (ULONG i=0; i<cSubKeys; i++) 
                    { 
                        cbName = MAX_KEY_LENGTH;
                      
                        hr = RegEnumKeyExA(hKey, i,
                            achKey, 
                            &cbName, 
                            NULL, 
                            NULL, 
                            NULL, 
                            NULL); 
                        if (hr == ERROR_SUCCESS) 
                        {
                          
                            if(strlen(achKey)==8)
                            {
                                TCHAR tszKeyName[256]={0};
                                StringCbPrintf(tszKeyName,sizeof(tszKeyName),_T("SYSTEM\\CurrentControlSet\\Services\\%s"),CA2T(achKey));
                                CRegKey reg;
                                if(reg.Open(HKEY_LOCAL_MACHINE,tszKeyName) == ERROR_SUCCESS)
                                {
                                    TCHAR tszDriverName[MAX_PATH] = {0};
                                    DWORD dwCountOfDriverName = _countof(tszDriverName);
                                    if(reg.QueryStringValue(_T("ImagePath"),tszDriverName,&dwCountOfDriverName)== ERROR_SUCCESS)
                                    {
                                        if(StrStrI(tszDriverName,_T("\\fsxdhy.sys")))
                                        {
                                             bFind = TRUE;
                                             StringCbCopyA(szDriverName,ulDriverNameLen+1,CT2A(tszDriverName));
                                             //ulDriverNameLen = dwCountOfDriverName;
                                             /*FsDriverUninstall(CA2W(achKey),nOprVir);
                                             DbgPrint("FsDriverUninstall:%s",achKey);*/
                                        }
                                    }
                                    reg.Close();

                                }
                                
                                
                            }

                        }
                        else if(hr == ERROR_NO_MORE_ITEMS)
                        {
                            break;
                        }

                     }
                  
                }

            }
           
            RegCloseKey(hKey);
        } 
    #pragma endregion 查找fsxdhy 驱动名称
    return bFind;
}
BOOL GetRandStringName(char* szRandString,ULONG ulRandStringLen)
{
    //定义随机生成字符串表
    if(GetPreviousDriverName(szRandString,ulRandStringLen))
    {
        return TRUE;
    }
    char *str = "0123456789abcdefghijklmnopqrstuvwxyz";
    ULONG i,lstr;
    char ss[2] = {0};
    lstr = (ULONG)strlen(str);//计算字符串长度
    srand((unsigned int)time((time_t *)NULL));//使用系统时间来初始化随机数发生器
    for(i = 1; i <= ulRandStringLen; i++){//按指定大小返回相应的字符串
        sprintf(ss,"%c",str[(rand() % lstr)]);//rand()%lstr 可随机返回0-71之间的整数, str[0-71]可随机得到其中的字符
        StringCbCatA(szRandString,ulRandStringLen,ss);
    }
    return TRUE;
}
BOOL RetrieveDriverName(char* szDriveName,ULONG ulDriverNameLen)
{
    BOOL bResult = FALSE;
    CRegKey reg;
    LRESULT hr = reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\XDHY"));
    if(hr == ERROR_SUCCESS)
    {
        TCHAR tszDriverName[MAX_PATH] ={0};
        DWORD dwLen = _countof(tszDriverName);
        hr = reg.QueryStringValue(_T("DriverName"),tszDriverName,&dwLen);
        if (hr == ERROR_SUCCESS)
        {
            StringCbCopyA(szDriveName,ulDriverNameLen,CT2A(tszDriverName));
            bResult = TRUE;
        }
        reg.Close();
    }
    if (bResult == FALSE)
    {
        StringCbCopyA(szDriveName,ulDriverNameLen,"fsxdhy");
    }
    return TRUE;
}
BOOL SetDriverName(char * szDriverName)
{
    BOOL bResult = FALSE;
    CRegKey reg;
    LRESULT hr = reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\XDHY"));
    if(hr == ERROR_SUCCESS)
    {
        TCHAR tszDriverName[MAX_PATH] ={0};
        hr = reg.SetStringValue(_T("DriverName"),CA2T(szDriverName));
        if (hr == ERROR_SUCCESS)
        {

            bResult = TRUE;
        }
        reg.Close();
    }
    return bResult;
}


BOOL GetFileType(const char* szFileName,DWORD dwFetchType,DWORD * pFileType)
{
    if(pFileType == NULL)
    {
        return FALSE;
    }
    if(szFileName == NULL || szFileName[0] == 0)
    {

        *pFileType = _FILE_TYPE_INVALID_PARAMETER;
        return TRUE;
    }
    if(dwFetchType == _FIIE_TYPE_IGNORE)
    {
        *pFileType = _FIIE_TYPE_IGNORE;
        return TRUE;
    }
    *pFileType = _FILE_TYPE_OTHER;

    if (StrStrIA(szFileName,".bat")||StrStrIA(szFileName,".cmd")||StrStrIA(szFileName,".msi"))
    {
        *pFileType = _FILE_TYPE_SCRIPT;
        return TRUE;
    }




    HANDLE hProgram = CreateFileA (szFileName,GENERIC_READ,FILE_SHARE_DELETE|FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (hProgram != INVALID_HANDLE_VALUE)
    {
        DWORD dwRead = 0;
        BYTE* byteBuffer = (BYTE*) malloc(2048*sizeof(BYTE));
        if(byteBuffer)
        {
            ReadFile(hProgram,byteBuffer,1024*sizeof(BYTE),&dwRead,NULL);  

            PIMAGE_DOS_HEADER DosHeader = NULL;
            PIMAGE_NT_HEADERS NtHeader = NULL;



            DosHeader = (PIMAGE_DOS_HEADER)byteBuffer;
            if (_strnicmp ((PCHAR)(&DosHeader->e_magic),"MZ",2) == 0)
            {
                if (DosHeader->e_lfanew < 0 || DosHeader->e_lfanew > (LONG)(dwRead - sizeof (IMAGE_NT_HEADERS)))
                {
                    *pFileType = _FILE_TYPE_OTHER;
                }
                else
                {
                    NtHeader = (PIMAGE_NT_HEADERS)(byteBuffer + DosHeader->e_lfanew);

                    if((DWORD)NtHeader <((DWORD)byteBuffer + dwRead))
                    {
                        if (_strnicmp ((PCHAR)(&NtHeader->Signature),"PE",2) == 0)
                        {
                            *pFileType = _FILE_TYPE_PE;
                        }
                        else
                        {
                            *pFileType = _FILE_TYPE_OTHER;
                        }

                    }
                    else
                    {
                        *pFileType = _FILE_TYPE_OTHER;  
                    }

                }

            }


            else
            {
                *pFileType = _FILE_TYPE_OTHER;
            }
            CloseHandle(hProgram);
            free(byteBuffer);
            byteBuffer = NULL;

        }
    }


    else
    {
        *pFileType = _FILE_TYPE_ERROR;
        return TRUE;
    }


    return TRUE;
}

INT32 WINAPI CheckDiskUsbBusFromDiskNo( ULONG DiskNo, BOOLEAN *IsUsbBus )
{
    INT32 Ret = 0; 
    INT32 _Ret; 
    STORAGE_PROPERTY_QUERY Query; 
    STORAGE_ADAPTER_DESCRIPTOR AdapterDesc; 
    HRESULT hr; 
#define PHYSICAL_DISK_DEV_SYMBOL_NAME "\\\\.\\PHYSICALDRIVE%u"
    CHAR DiskDevName[ MAX_PATH ]; 
    HANDLE DiskDev = INVALID_HANDLE_VALUE; 
    ULONG RetSize = 0; 
    //ULONG BufSize; 

    do 
    {
        if( NULL == IsUsbBus )
        {
            Ret = COMMON_INVALID_PARAMETER; 
            break; 
        }

        *IsUsbBus = FALSE; 

        hr = StringCbPrintfA( DiskDevName, sizeof( DiskDevName ), PHYSICAL_DISK_DEV_SYMBOL_NAME, DiskNo ); 
        if( FAILED( hr ) )
        {
            Ret = COMMON_UNSUCCESSFUL; 
            break; 
        }

        DiskDev = CreateFileA( DiskDevName,
            GENERIC_READ, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            NULL, 
            OPEN_EXISTING, 
            0, 
            NULL ); 

        if( DiskDev == NULL 
            || DiskDev == INVALID_HANDLE_VALUE ) 
        { 
            Ret = COMMON_INVALID_PARAMETER; 
            break; 
        } 

        printf( "open disk %u device successfully\n", DiskNo ); 

        Query.PropertyId = StorageAdapterProperty;  
        Query.QueryType = PropertyStandardQuery;  

        AdapterDesc.Size = sizeof( AdapterDesc ); 

        _Ret = DeviceIoControl( DiskDev, 
            IOCTL_STORAGE_QUERY_PROPERTY, 
            &Query,   
            sizeof( STORAGE_PROPERTY_QUERY ), 
            &AdapterDesc, 
            sizeof( AdapterDesc ), 
            &RetSize, 
            ( LPOVERLAPPED )NULL ); 

        if( _Ret == FALSE )
        {
#ifdef _DEBUG
            LPVOID ErrorMsg = NULL; 

            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS, 
                NULL, 
                GetLastError(), 
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language 
                ( LPTSTR )&ErrorMsg, 
                0, 
                NULL ); 

            if( ErrorMsg != NULL )
            {
                LocalFree( ErrorMsg ); 
            }
#endif //_DEBUG

            Ret = COMMON_UNSUCCESSFUL; 
            break; 
        }

        if( AdapterDesc.BusType == BusTypeUsb )
        {
            *IsUsbBus = TRUE; 
            break; 
        }
    }while( FALSE ); 

    if( DiskDev != INVALID_HANDLE_VALUE 
        && DiskDev != NULL )
    {
        CloseHandle( DiskDev ); 
    }

    return Ret; 
}


INT32 WINAPI GetDiskNumbers( HANDLE VolumeDev, 
                            ULONG *DiskNumbers, 
                            ULONG Count, 
                            ULONG *RetCount )
{
    INT32 Ret = 0; 
    INT32 _Ret; 
    INT32 i; 
    ULONG BufSize; 
    ULONG RetSize; 
    ULONG ErrorCode; 
    BOOLEAN SizeCorrect = FALSE; 
    PVOLUME_DISK_EXTENTS VolDiskExt = NULL; 

    do 
    {
        if( VolumeDev == INVALID_HANDLE_VALUE 
            || VolumeDev == NULL )
        {
            Ret = COMMON_INVALID_PARAMETER; 
            break; 
        }

        if( DiskNumbers == NULL )
        {
            Ret = COMMON_INVALID_PARAMETER; 
            break; 
        }

        if( Count == 0 )
        {
            Ret = COMMON_INVALID_PARAMETER; 
            break; 
        }


        if( RetCount == NULL )
        {
            Ret = COMMON_INVALID_PARAMETER; 
            break; 
        }

        *RetCount = 0; 

        BufSize = sizeof( VOLUME_DISK_EXTENTS ) + ( MAX_DISK_COUNT * sizeof( DISK_EXTENT ) ); 

        //for( ; ; )
        //{
        VolDiskExt = ( PVOLUME_DISK_EXTENTS )malloc( BufSize ); 
        if( VolDiskExt == NULL )
        {
            Ret = COMMON_INSUFFICIENT_RESOURCES; 
            break; 
        }

        _Ret = DeviceIoControl( VolumeDev, 
            IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, 
            NULL,   
            0, 
            VolDiskExt, 
            BufSize, 
            &RetSize, 
            ( LPOVERLAPPED )NULL ); 

        if( _Ret == FALSE )
        {
            ErrorCode = GetLastError(); 

            //DbgPrintEx( "get the volume extens for the volume error %u", ErrorCode ); 

            if( ErrorCode == ERROR_MORE_DATA )
            {

            }
            //       //BufSize += sizeof; 

            //       if( SizeCorrect == TRUE )
            //       {
            //                break; 
            //       }
            //       else
            //       {
            //                SizeCorrect = TRUE; 
            //       }
            //}
            //else
            {
                Ret = COMMON_UNSUCCESSFUL; 
                break; 
            }
        }
        //else
        //{
        //       break; 
        //}
        //}

        if( Ret != 0 )
        {
            break; 
        }

        *RetCount = VolDiskExt->NumberOfDiskExtents; 

        for( i = 0; ( ULONG )i < Count; i ++ )
        {
            DiskNumbers[ i ] = VolDiskExt->Extents[ i ].DiskNumber; 

            if( ( ULONG )i >= VolDiskExt->NumberOfDiskExtents )
            {
                DiskNumbers[ i ] = ( ULONG )INVALID_DISK_NO; 
            }
        }

        if( VolDiskExt->NumberOfDiskExtents > Count )
        {
            Ret = COMMON_BUFFER_TO_SMALL; 
            break; 
        }

    }while( FALSE );

    if( VolDiskExt != NULL )
    {
        free( VolDiskExt ); 
    }

    return Ret; 
}

INT32 WINAPI IsUsbDevice( LPCSTR VolumeName ) 
{
    INT32 Ret = FALSE; 
    INT32 _Ret; 
    //LSTATUS __Ret;
    HANDLE VolumeDev = INVALID_HANDLE_VALUE; 
    ULONG RetSize = 0; 
    HRESULT hr; 
    ULONG BufSize; 
    INT32 i; 
    //ULONG ErrorCode; 

#define DEFAULT_INFO_SIZE ( sizeof( DRIVE_LAYOUT_INFORMATION ) + ( 26 * 4 ) * sizeof( PARTITION_INFORMATION ) )

    //PPARTITION_INFORMATION PartInfo; 
    CHAR DevName[ MAX_PATH ]; 
    PSTORAGE_DEVICE_NUMBER DiskNum; 
    PSTORAGE_ADAPTER_DESCRIPTOR AdapterDesc; 
    PVOLUME_DISK_EXTENTS VolDisk; 

    //PDRIVE_LAYOUT_INFORMATION DiskLayout; 
    PVOID InfoBuf = NULL; 

    do 
    {
        BufSize = DEFAULT_INFO_SIZE; 

        if( VolumeName == NULL )
        {
            //Ret = COMMON_INVALID_PARAMETER; 
            break; 
        }

        InfoBuf = malloc( BufSize ); 
        if( InfoBuf == NULL ) 
        { 
            break; 
        } 

        //PartInfo = ( PPARTITION_INFORMATION )InfoBuf; 
        DiskNum = ( PSTORAGE_DEVICE_NUMBER )InfoBuf; 
        AdapterDesc = ( PSTORAGE_ADAPTER_DESCRIPTOR )InfoBuf; 
        VolDisk = ( PVOLUME_DISK_EXTENTS )InfoBuf; 

        hr = StringCbPrintfA( DevName, sizeof( DevName ), "\\\\.\\%s", VolumeName ); 
        if( FAILED( hr ) )
        {
            break; 
        }

        VolumeDev = CreateFileA( DevName,
            GENERIC_READ, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            NULL, 
            OPEN_EXISTING, 
            0, 
            NULL ); 

        if( VolumeDev == NULL 
            || VolumeDev == INVALID_HANDLE_VALUE ) 
        {
            break; 
        }

        {
            STORAGE_PROPERTY_QUERY Query; 

            Query.PropertyId = StorageAdapterProperty;  
            Query.QueryType = PropertyStandardQuery;  

            AdapterDesc->Size = BufSize; 

            _Ret = DeviceIoControl( VolumeDev, 
                IOCTL_STORAGE_QUERY_PROPERTY, 
                &Query,   
                sizeof( STORAGE_PROPERTY_QUERY ), 
                AdapterDesc, 
                BufSize, 
                &RetSize, 
                ( LPOVERLAPPED )NULL ); 

            if( _Ret == TRUE )
            {
                if( AdapterDesc->BusType == BusTypeUsb )
                {
                    Ret = TRUE; 
                    break; 
                }

                break; 
            }
        }

        do 
        {
            ULONG DiskCount; 
            ULONG DiskNos[ MAX_DISK_COUNT ]; 
            BOOLEAN IsUsbBus; 

            Ret = GetDiskNumbers( VolumeDev, DiskNos, ARRAYSIZE( DiskNos ), &DiskCount ); 
            if( Ret != 0 )
            {
                if( Ret != COMMON_BUFFER_TO_SMALL )
                {
                    break; 
                }
            }



            for( i = 0; i < ARRAYSIZE( DiskNos ); i ++ )
            {
                if( DiskNos[ i ] == INVALID_DISK_NO )
                {
                    continue; 
                }

                Ret = CheckDiskUsbBusFromDiskNo( DiskNos[ i ], &IsUsbBus ); 
                if( Ret != 0 )
                {
                    continue; 
                }

                if( IsUsbBus == TRUE )
                {
                    Ret = TRUE; 
                    break; 
                }
            }
        }while( FALSE ); 

        if( Ret == TRUE )
        {
            break; 
        }

    }while( FALSE );

    if( VolumeDev != INVALID_HANDLE_VALUE )
    {
        CloseHandle( VolumeDev ); 
    }

    if( InfoBuf != NULL )
    {
        free( InfoBuf ); 
    }

    return Ret; 
} 

