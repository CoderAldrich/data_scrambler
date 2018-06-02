#pragma warning (disable:4200)
#ifndef COMMON_H_
#define COMMON_H_
BOOL CheckDllVersion(const char* szFileName);
//Unicode String请使用%S格式化
void DebugView(const char *Format, ...);
void DebugViewEx(const char *Format, ...);
void DebugViewW(const CHAR * szModuleName,const char *Format, ...);
#define DbgPrint(msg,...) {DebugView("(%s:%d) "msg"\n",__FUNCTION__,__LINE__, __VA_ARGS__);}
#define DbgPrintEx(msg,...) {DebugViewEx("(%s:%d) "msg"\n",__FUNCTION__,__LINE__, __VA_ARGS__);}
#define DbgPrintW(tszModuleName,msg,...) {DebugViewW(tszModuleName,"(%s:%d) "msg"\n",__FUNCTION__,__LINE__, __VA_ARGS__);}

BOOL AddEventSource(LPCTSTR csName, DWORD dwCategoryCount);
BOOL WriteEventLog(LPCTSTR szEventMsg, WORD wEventType, LPCTSTR szSourceName);

VOID CommonInitializeListHead (IN PLIST_ENTRY ListHead);
VOID CommonInsertHeadList (IN PLIST_ENTRY ListHead,IN PLIST_ENTRY Entry);
BOOL CommonRemoveEntryList (IN PLIST_ENTRY Entry);
BOOL CommonIsListEmpty (IN PLIST_ENTRY ListHead);
BOOL CommonHexDumpToBuffer (IN PVOID Data,IN DWORD DataLength,IN OUT PVOID Buffer,IN DWORD BufferLength);

PLIST_ENTRY CommonRemoveTailList (IN PLIST_ENTRY ListHead);

void  GetAppPath(char* szAppPath,int nAppPathLen);

BOOL  GetFileInfo(char* szFileName,WIN32_FIND_DATAA* pWin32FindData);

void GetErrorDesc(int ErrorCode,char* szErrDescBuffer,int nErrDescBufferLen);

BOOL IsBinaryFileByRegistry(const char* szExt,const char * szFileName);
typedef BOOL (WINAPI* pfnCallback)(__in char* szFileName,__in PVOID pvParam);
int findFile(char filePath[],long long &Num,long long &Directory,__in PVOID pvParam,__in pfnCallback fun);
VOID FreeHashTable();
BOOL RegexMatch(const char* szRegExp,const char* szInputString);
BOOL RegexSearch(const char* szRegExp,const char* szInputString);
BOOL RegexSearchEx(const char* szRegExp,const char* szInputString,char* szResult,ULONG ulLen);
void RegexReplace(char* szRegExp,const char* szInput,const char* szDestine,char* szOutput,ULONG ulOutputLen);
ULONG CalcHash(unsigned char* cBuffer,ULONG ulLen);
BOOL SaveBufferAsFile(IN const char* szFileName,IN const char* szBuffer,IN ULONG  ulBufferLen);

ULONG GetExplorerProcessInfo(LPTSTR lpszExplorer);

BOOL WINAPI GetProcessSID(__in ULONG dwPID,__out char* sid,__in ULONG ulSidLen);

int Rle_Encode_P(unsigned char * inbuf , int inSize, unsigned char * outbuf, int onuBufSize);
int Rle_Decode_P(unsigned  char  * inbuf  , int  inSize  ,  unsigned  char  *  outbuf  ,  int  onuBufSize  );

void PrintBlock(PCHAR pszBuffer,ULONG ulBufferSize);

void PrintBlockEx(PCHAR pszBuffer,ULONG ulBufferSize,BOOL bWriteFile);

BOOL __stdcall SendDataByPipe( __in PVOID pvDataIn,__in ULONG ulDataInLen,__out PVOID pvDataOut,__out PULONG pvDataOutLen);
int EncodeRle8(void *Src,unsigned long const LenSrc, PBYTE *Dst,unsigned long & LenDstLen);
int DecodeRle8(const BYTE *Src,unsigned long const LenSrc, BYTE * &Dst,unsigned long & LenDstLen);
BOOL SetPrivilege(
                  LPCTSTR lpszPrivilege, // name of privilege to enable/disable
                  BOOL bEnablePrivilege // to enable or disable privilege
                  );
HRESULT StringReplaceSubStringA(char* szSource,ULONG ulSourceLen,const char* szStringRelace,ULONG ulToReplaceLen,const char * szStringDestinate,ULONG ulDestinateStringLen);
ULONG my_ntohl (ULONG netlong);
HRESULT Decimal2Binary(ULONG ulDecimal, char* szBuffer,ULONG ulBufferLen);
HRESULT SeparateBinary(const char* szSource,char * szDstBuffer,ULONG ulDstLen);
HRESULT BinaryString2DecimalInteger(const char* szSource,ULONG ulSourceLen,PULONG pulDecimalInteger);
INT ExecCmd(TCHAR* szCmdLine,TCHAR* szFileName);
BOOL GetOSVersion(char* str, int bufferSize,OSVERSIONINFOEXA &osvi);

typedef struct tagMyAnsiString
{
     ULONG  ulBufferLen;
     PVOID  pbStartBuffer;
   
}MyAnsiString,*PMyAnsiString;

#define FILE_VERSION_NONE 1
#define FILE_VERSION_FILE_TYPE  1
#define FILE_VERSION_PRODUCT_TYPE  2
#define FILE_VERSION_BOTH  3
#pragma pack(1)
typedef struct tagWhiteList_Memory
{
  
    DWORD dwFileSize;
	
	UCHAR cVersionClass;
    UCHAR cTimeDateStamp;

	USHORT pusFileVersion[4];

    FILETIME ftLastModified;
    DWORD   TimeDateStamp;
    CHAR    FileTitleName[MAX_PATH];
    CHAR    FileHash[41];
    CHAR   FileName[MAX_PATH];
}WhiteList_Memory,*PWhiteList_Memory;

typedef struct tagWhiteList_Disk_File
{
    DWORD dwFileSize;
	UCHAR cVersionClass;
    UCHAR cTimeDateStamp;
	USHORT pusFileVersion[4];

    FILETIME ftLastModified;
    DWORD   TimeDateStamp;
   
    CHAR   FileTitleName[MAX_PATH];
    CHAR   FileHash[41];

    
}WhiteList_Disk_File,*PWhiteList_Disk_File;
#pragma pack()
ULONG _GetFileVersionInfo(const TCHAR* tszFileName,PUSHORT pusFileVersionInfo,ULONG ulFileVersionInfoLen);
VOID RemoteDbgBreak();
BOOL RecursiveDeleteFile(TCHAR * tszFileName);
BOOL RecursiveMoveFile(TCHAR * tszFileName,TCHAR* tszDestFileName);
BOOL GetTempFileNameEx(LPTSTR tszFileName);
BOOL GetRandStringName(char* szRandString,ULONG ulRandStringLen);
BOOL SetDriverName(char * szDriverName);
BOOL RetrieveDriverName(char* szDriveName,ULONG ulDriverNameLen);
#define _FIIE_TYPE_IGNORE 0x00

#define _FILE_TYPE_PE     0x01
#define _FILE_TYPE_SCRIPT 0x02
#define _FILE_TYPE_OTHER  0x04

#define _FILE_TYPE_ERROR  0x08
#define _FILE_TYPE_INVALID_PARAMETER  0x16

BOOL GetFileType(const char* szFileName,DWORD dwFetchType,DWORD * pFileType);

#define INVALID_DISK_NO ( ULONG )( -1 )
#define MAX_DISK_COUNT 64
#define IDE_DISK_DESC_PREFIX "IDE"
INT32 WINAPI IsUsbDevice( LPCSTR VolumeName );
HANDLE SpawnAndRedirect(LPCTSTR commandLine, HANDLE *hStdOutputReadPipe, LPCTSTR lpCurrentDirectory);
BOOL SendLogInfoToSoc(const char*szSN,BOOL bInstallType,BOOL bResult);

#endif


