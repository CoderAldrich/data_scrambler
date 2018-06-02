
#include "Wmi.h"
#include "Common.h"
#include "md5.h"
#include <strsafe.h>


//#include <Rpc.h>
//#pragma comment(lib,"Rpcrt4.lib")
#import "progid:WbemScripting.SWbemLocator" named_guids
#define  IDENTIFY_BUFFER_SIZE  512
#pragma warning(disable:4996)

//  IOCTL commands
#define  DFP_GET_VERSION          0x00074080
#define  DFP_SEND_DRIVE_COMMAND   0x0007c084
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition

#define SMART_GET_VERSION               CTL_CODE(IOCTL_DISK_BASE, 0x0020, METHOD_BUFFERED, FILE_READ_ACCESS)
#define SMART_SEND_DRIVE_COMMAND        CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA            CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#pragma pack(1)
typedef struct _GETVERSIONOUTPARAMS
{
	BYTE bVersion;      // Binary driver version.
	BYTE bRevision;     // Binary driver revision.
	BYTE bReserved;     // Not used.
	BYTE bIDEDeviceMap; // Bit map of IDE devices.
	DWORD fCapabilities; // Bit mask of driver capabilities.
	DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;



typedef struct _IDENTIFY_DATA {
	USHORT GeneralConfiguration;            // 00 00
	USHORT NumberOfCylinders;               // 02  1
	USHORT Reserved1;                       // 04  2
	USHORT NumberOfHeads;                   // 06  3
	USHORT UnformattedBytesPerTrack;        // 08  4
	USHORT UnformattedBytesPerSector;       // 0A  5
	USHORT SectorsPerTrack;                 // 0C  6
	USHORT VendorUnique1[3];                // 0E  7-9
	USHORT SerialNumber[10];                // 14  10-19
	USHORT BufferType;                      // 28  20
	USHORT BufferSectorSize;                // 2A  21
	USHORT NumberOfEccBytes;                // 2C  22
	USHORT FirmwareRevision[4];             // 2E  23-26
	USHORT ModelNumber[20];                 // 36  27-46
	UCHAR  MaximumBlockTransfer;            // 5E  47
	UCHAR  VendorUnique2;                   // 5F
	USHORT DoubleWordIo;                    // 60  48
	USHORT Capabilities;                    // 62  49
	USHORT Reserved2;                       // 64  50
	UCHAR  VendorUnique3;                   // 66  51
	UCHAR  PioCycleTimingMode;              // 67
	UCHAR  VendorUnique4;                   // 68  52
	UCHAR  DmaCycleTimingMode;              // 69
	USHORT TranslationFieldsValid:1;        // 6A  53
	USHORT Reserved3:15;
	USHORT NumberOfCurrentCylinders;        // 6C  54
	USHORT NumberOfCurrentHeads;            // 6E  55
	USHORT CurrentSectorsPerTrack;          // 70  56
	ULONG  CurrentSectorCapacity;           // 72  57-58
	USHORT CurrentMultiSectorSetting;       //     59
	ULONG  UserAddressableSectors;          //     60-61
	USHORT SingleWordDMASupport : 8;        //     62
	USHORT SingleWordDMAActive : 8;
	USHORT MultiWordDMASupport : 8;         //     63
	USHORT MultiWordDMAActive : 8;
	USHORT AdvancedPIOModes : 8;            //     64
	USHORT Reserved4 : 8;
	USHORT MinimumMWXferCycleTime;          //     65
	USHORT RecommendedMWXferCycleTime;      //     66
	USHORT MinimumPIOCycleTime;             //     67
	USHORT MinimumPIOCycleTimeIORDY;        //     68
	USHORT Reserved5[2];                    //     69-70
	USHORT ReleaseTimeOverlapped;           //     71
	USHORT ReleaseTimeServiceCommand;       //     72
	USHORT MajorRevision;                   //     73
	USHORT MinorRevision;                   //     74
	USHORT Reserved6[50];                   //     75-126
	USHORT SpecialFunctionsEnabled;         //     127
	USHORT Reserved7[128];                  //     128-255
} IDENTIFY_DATA, *PIDENTIFY_DATA;

#pragma pack()

#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.
//  Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS 
#define  CAP_IDE_ID_FUNCTION             1  // ATA ID command supported
#define  CAP_IDE_ATAPI_ID                2  // ATAPI ID command supported
#define  CAP_IDE_EXECUTE_SMART_FUNCTION  4  // SMART commannds supported

BYTE IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];


// DoIDENTIFY
// FUNCTION: Send an IDENTIFY command to the drive
// bDriveNum = 0-3
// bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
BOOL DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
				 PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
				 PDWORD lpcbBytesReturned)
{
	// Set up data structures for IDENTIFY command.
	pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;
	pSCIP -> irDriveRegs.bFeaturesReg = 0;
	pSCIP -> irDriveRegs.bSectorCountReg = 1;
	//pSCIP -> irDriveRegs.bSectorNumberReg = 1;
	pSCIP -> irDriveRegs.bCylLowReg = 0;
	pSCIP -> irDriveRegs.bCylHighReg = 0;

	// Compute the drive number.
	pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

	// The command can either be IDE identify or ATAPI identify.
	pSCIP -> irDriveRegs.bCommandReg = bIDCmd;
	pSCIP -> bDriveNumber = bDriveNum;
	pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;

	return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
		(LPVOID) pSCIP,
		sizeof(SENDCMDINPARAMS) - 1,
		(LPVOID) pSCOP,
		sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
		lpcbBytesReturned, NULL) );
}

char *ConvertToString (DWORD diskdata [256],
					   int firstIndex,
					   int lastIndex,
					   char* buf)
{
	int index = 0;
	int position = 0;

	//  each integer has two characters stored in it backwards
	for (index = firstIndex; index <= lastIndex; index++)
	{
		//  get high byte for 1st character
		buf [position++] = (char) (diskdata [index] / 256);

		//  get low byte for 2nd character
		buf [position++] = (char) (diskdata [index] % 256);
	}

	//  end the string 
	buf[position] = '\0';

	//  cut off the trailing blanks
	for (index = position - 1; index > 0 && isspace(buf [index]); index--)
		buf [index] = '\0';

	return buf;
}


char * flipAndCodeBytes (const char * str,
						 int pos,
						 int flip,
						 char * buf)
{
	int i;
	int j = 0;
	int k = 0;

	buf [0] = '\0';
	if (pos <= 0)
		return buf;

	if ( ! j)
	{
		char p = 0;

		// First try to gather all characters representing hex digits only.
		j = 1;
		k = 0;
		buf[k] = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = tolower(str[i]);

			if (isspace(c))
				c = '0';

			++p;
			buf[k] <<= 4;

			if (c >= '0' && c <= '9')
				buf[k] |= (unsigned char) (c - '0');
			else if (c >= 'a' && c <= 'f')
				buf[k] |= (unsigned char) (c - 'a' + 10);
			else
			{
				j = 0;
				break;
			}

			if (p == 2)
			{
				if (buf[k] != '\0' && ! isprint(buf[k]))
				{
					j = 0;
					break;
				}
				++k;
				p = 0;
				buf[k] = 0;
			}

		}
	}

	if ( ! j)
	{
		// There are non-digit characters, gather them as is.
		j = 1;
		k = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = str[i];

			if ( ! isprint(c))
			{
				j = 0;
				break;
			}

			buf[k++] = c;
		}
	}

	if ( ! j)
	{
		// The characters are not there or are not printable.
		k = 0;
	}

	buf[k] = '\0';

	if (flip)
		// Flip adjacent characters
		for (j = 0; j < k; j += 2)
		{
			char t = buf[j];
			buf[j] = buf[j + 1];
			buf[j + 1] = t;
		}

		// Trim any beginning and end space
		i = j = -1;
		for (k = 0; buf[k] != '\0'; ++k)
		{
			if (! isspace(buf[k]))
			{
				if (i < 0)
					i = k;
				j = k;
			}
		}

		if ((i >= 0) && (j >= 0))
		{
			for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
				buf[k - i] = buf[k];
			buf[k - i] = '\0';
		}

		return buf;
}




//获得硬盘序列号
BOOL GetFirstHardDiskSerialNumber(__inout char* pvSerialNumber,__in ULONG ulBufferLen)
{
	char PhysicalDrive [MAX_PATH]={0};
	BOOL bResult = FALSE;
    char IniFilePath[MAX_PATH] = {0};
    
    char szIP[30] = {0};
    CHAR HostName[128]={0};

    GetAppPath(IniFilePath,sizeof(IniFilePath));
    StringCbCatA(IniFilePath,sizeof(IniFilePath),"\\Config.ini");

    GetPrivateProfileStringA("NetworkComm:Local","Ipv4","127.0.0.1",szIP,_countof(szIP),IniFilePath);

   
    GetPrivateProfileStringA("Platform","HostName","",HostName,_countof(HostName),IniFilePath);


	StringCbCopyA(PhysicalDrive,sizeof(PhysicalDrive), "\\\\.\\PhysicalDrive0");

	HANDLE hPhysicalDriveIOCTL =  CreateFileA (PhysicalDrive,
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE , NULL,
		OPEN_EXISTING, 0, NULL);
	while (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)  
	{

		STORAGE_PROPERTY_QUERY query;
		DWORD cbBytesReturned = 0;
		char buffer [512];

		memset ((void *) & query, 0, sizeof (query));
		query.PropertyId = StorageDeviceProperty;
		query.QueryType = PropertyStandardQuery;

		memset (buffer, 0, sizeof (buffer));

		if ( DeviceIoControl (hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
			& query,
			sizeof (query),
			& buffer,
			sizeof (buffer),
			& cbBytesReturned, NULL) )
		{
			 STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *) & buffer;
			 if(descrip->SerialNumberOffset != -1)
			 {
				 CHAR szSerialNo[256]={0};
				 flipAndCodeBytes (buffer,
					 descrip -> SerialNumberOffset,
					 1, szSerialNo );
				 ULONG i=0;
				 for(i=0;i<strlen(szSerialNo);i++)
				 {
					 if(szSerialNo[i] != ' ')
					 {
						 break;
					 }
				 }
				 StringCbCopyA(pvSerialNumber,ulBufferLen,&szSerialNo[i]);
				 if(strlen(pvSerialNumber))
				 {
					 bResult=TRUE;
					 break;
				 }
				
				
			 }
		}
		GETVERSIONINPARAMS GetVersionParams;
	
		// Get the version, etc of PhysicalDrive IOCTL
		memset ((void*) & GetVersionParams, 0, sizeof(GetVersionParams));

		if (  DeviceIoControl (hPhysicalDriveIOCTL, SMART_GET_VERSION,
			NULL, 
			0,
			&GetVersionParams, sizeof (GETVERSIONINPARAMS),
			&cbBytesReturned, NULL) )
		{
			ULONG CommandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
			PSENDCMDINPARAMS Command = (PSENDCMDINPARAMS) malloc (CommandSize);
            if(Command)
            {
			// Retrieve the IDENTIFY data
			// Prepare the command
			    #define ID_CMD          0xEC            // Returns ID sector for ATA
			    Command -> irDriveRegs.bCommandReg = ID_CMD;
			    DWORD BytesReturned = 0;
			    if ( DeviceIoControl (hPhysicalDriveIOCTL, 
				    SMART_RCV_DRIVE_DATA, Command, sizeof(SENDCMDINPARAMS),
				    Command, CommandSize,
				    &BytesReturned, NULL) )
			    {
				    DWORD diskdata [256];
				    USHORT *pIdSector = (USHORT *)
					    (PIDENTIFY_DATA) ((PSENDCMDOUTPARAMS) Command) -> bBuffer;

				    for (int ijk = 0; ijk < 256; ijk++)
					    diskdata [ijk] = pIdSector [ijk];

				    CHAR szSerialNo[256]={0};
				    ConvertToString(diskdata,10,19,szSerialNo);
				    ULONG i=0;
				    for(i=0;i<strlen(szSerialNo);i++)
				    {
					    if(szSerialNo[i] != ' ')
					    {
						    break;
					    }
				    }
    				
				    StringCbCopyA(pvSerialNumber,ulBufferLen,&szSerialNo[i]);
    				
				    if(strlen(pvSerialNumber))
				    {
                        if(Command)
                        {
                            free(Command);
                            Command = NULL;
                        }
					    bResult=TRUE;
					    break;
				    }
			    }
                if(Command)
                {
                    free(Command);
                    Command = NULL;
                }
             
            }

		}
		GETVERSIONOUTPARAMS VersionParams;
	
		// Get the version, etc of PhysicalDrive IOCTL
		memset ((void*) &VersionParams, 0, sizeof(VersionParams));


		if ( ! DeviceIoControl (hPhysicalDriveIOCTL, DFP_GET_VERSION,
			NULL, 
			0,
			&VersionParams,
			sizeof(VersionParams),
			&cbBytesReturned, NULL) )
		{         

			DWORD err = GetLastError ();
			AtlTrace ("\n Read PhysicalDrive  ERROR"
				"\nDeviceIoControl(%d, DFP_GET_VERSION) returned 0, error is %d\n",
				(int) hPhysicalDriveIOCTL, (int) err);
			

		}

		// If there is a IDE device at number "i" issue commands
		// to the device
		if (VersionParams.bIDEDeviceMap == FALSE)
		{

			AtlTrace ("\n Read PhysicalDrive  ERROR"
				"\nNo device found at position  (%d)\n",
				(int) VersionParams.bIDEDeviceMap);
		
		}
		else
		{
			BYTE             bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
			SENDCMDINPARAMS  scip;
			//SENDCMDOUTPARAMS OutCmd;

			// Now, get the ID sector for all IDE devices in the system.
			// If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
			// otherwise use the IDE_ATA_IDENTIFY command
			bIDCmd = (VersionParams.bIDEDeviceMap >> 0 & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;

			memset (&scip, 0, sizeof(scip));
			memset (IdOutCmd, 0, sizeof(IdOutCmd));

			if ( DoIDENTIFY (hPhysicalDriveIOCTL, 
				&scip, 
				(PSENDCMDOUTPARAMS)&IdOutCmd, 
				(BYTE) bIDCmd,
				(BYTE) 0,
				&cbBytesReturned))
			{
				DWORD diskdata [256];
				int ijk = 0;
				USHORT *pIdSector = (USHORT *)
					((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;

				for (ijk = 0; ijk < 256; ijk++)
					diskdata [ijk] = pIdSector [ijk];

				//PrintIdeInfo (0, diskdata);
				CHAR szSerialNo[256]={0};
				ConvertToString(diskdata,10,19,szSerialNo);
				ULONG i=0;
				for(i=0;i<strlen(szSerialNo);i++)
				{
					if(szSerialNo[i] != ' ')
					{
						break;
					}
				}
				StringCbCopyA(pvSerialNumber,ulBufferLen,&szSerialNo[i]);
				if(strlen(pvSerialNumber))
				{
					bResult=TRUE;
					break;
				}

			}
		}

		if(!bResult) 
		{
			DISK_GEOMETRY diskGeometry;
			PARTITION_INFORMATION partitionInfo;
			DWORD dwBytes = 0;
			//
			// 获得磁盘大小
			//
			if (DeviceIoControl(hPhysicalDriveIOCTL,
				IOCTL_DISK_GET_DRIVE_GEOMETRY,    // 调用了CTL_CODE macro宏
				NULL,
				0,
				&diskGeometry,
				sizeof(DISK_GEOMETRY),
				&dwBytes,
				NULL))
			{


				if (DeviceIoControl(hPhysicalDriveIOCTL,
					IOCTL_DISK_GET_PARTITION_INFO,
					NULL,
					0,
					&partitionInfo,
					sizeof(PARTITION_INFORMATION),
					&dwBytes,
					NULL)) 
				{
					LONGLONG sectorCount = partitionInfo.PartitionLength.QuadPart / diskGeometry.BytesPerSector;
					LARGE_INTEGER offset;

					// sector数所占字节
					offset.QuadPart = (sectorCount-1) * diskGeometry.BytesPerSector;
					SetFilePointer(hPhysicalDriveIOCTL, offset.LowPart, &offset.HighPart, FILE_BEGIN);
					unsigned char lpBuffer[512]={0};
					DWORD dwSize = sizeof(lpBuffer);
					// 读取扇区的数据
					if (ReadFile(hPhysicalDriveIOCTL, lpBuffer, dwSize, &dwBytes, NULL)) 
					{
						DWORD i=0;
						for (i=0;i<dwSize;i++)
						{
							if(lpBuffer[i])
							{
								break;
							}
						}
						if(i==dwSize)
						{

							//内容全为0
                            MD5_CTX                          Ctx = {0} ;

                         
                         
							GUID guid = {0};
							CoCreateGuid(&guid);
							
							char strGuid[33]={0};
							StringCbPrintfA(strGuid,sizeof(strGuid),"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
								guid.Data1,
								guid.Data2,
								guid.Data3,
								guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]
								);

							if (DeviceIoControl (hPhysicalDriveIOCTL, FSCTL_LOCK_VOLUME,
								NULL, 0, NULL, 0, &dwBytes, NULL) == FALSE)
							{
								OutputDebugStringA("FSCTL_LOCK_VOLUME error");
							}
                           
                            memcpy(lpBuffer,strGuid,strlen((const char *)strGuid));

                            SetFilePointer(hPhysicalDriveIOCTL, offset.LowPart, &offset.HighPart, FILE_BEGIN);//驱动器找不到请求的扇区 0x1b

                            if(WriteFile(hPhysicalDriveIOCTL, lpBuffer, dwSize, &dwBytes, NULL))
                            {
                                StringCbCopyA(pvSerialNumber,ulBufferLen,(LPCSTR)strGuid);
                                bResult = TRUE;
                            }
                            else
                            {
                                //写入磁盘最后扇区失败
                                OutputDebugStringA("write last sector error");
                            }
                          

							

						}
						else if(i == 0x0 && strlen((const char *)lpBuffer)==0x20)
						{
                            int j=0;
                            for (j=0;j<0x20;j++)
                            {
                                if(isalnum(lpBuffer[j]) ==0 )
                                {
                                    break;
                                }
                            }
                            if(j==0x20)
                            {
							    StringCbCopyNA(pvSerialNumber,ulBufferLen,(LPCSTR)lpBuffer,0x20);
							    bResult = TRUE;
                            }
                          
						}
                        else
                        {
                            OutputDebugStringA("last sector has not containt sn");
                        }


					}



				}
			}
		}
		break;

	}


	
	CloseHandle (hPhysicalDriveIOCTL);

	if(bResult)
	{
		_strlwr(pvSerialNumber);
     
	}
    else
    {
        CRegKey reg;
        if(reg.Open(HKEY_LOCAL_MACHINE,_T("Software\\xdhy"))==ERROR_SUCCESS)
        {
            TCHAR tszBiosSerialNumber[256]={0};
            DWORD dwLen = sizeof(tszBiosSerialNumber);
            LONG hr = reg.QueryStringValue(_T("BIOS"),tszBiosSerialNumber,&dwLen);
            if (hr == ERROR_SUCCESS && dwLen >0)
            {

                StringCbCopyA(pvSerialNumber,ulBufferLen,CT2A(tszBiosSerialNumber));
                _strlwr(pvSerialNumber);
                bResult = TRUE;
            }
            reg.Close();
        }
    }
	return bResult;

}



CWmiInfo::CWmiInfo(void)
{
	m_pWbemSvc=NULL;
	m_pWbemLoc=NULL;
	m_pEnumClsObj = NULL;

}

CWmiInfo::~CWmiInfo(void)
{
	m_pWbemSvc=NULL;
	m_pWbemLoc=NULL;
	m_pEnumClsObj = NULL;
}

HRESULT CWmiInfo::InitWmi()
{
	HRESULT hr;

	//一、初始化COM组件
	//初始化COM
	hr=::CoInitializeEx(0,COINIT_MULTITHREADED);
	if (SUCCEEDED(hr) || RPC_E_CHANGED_MODE == hr)
	{
		//设置进程的安全级别，（调用COM组件时在初始化COM之后要调用CoInitializeSecurity设置进程安全级别，否则会被系统识别为病毒）
		hr=CoInitializeSecurity(NULL,
			-1,
			NULL,                   
			NULL,
			RPC_C_AUTHN_LEVEL_PKT,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			EOAC_NONE,
			NULL);
		//VERIFY(SUCCEEDED(hr));

		//二、创建一个WMI命名空间连接
		//创建一个CLSID_WbemLocator对象
		hr=CoCreateInstance(CLSID_WbemLocator,
			0,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator,
			(LPVOID*)&m_pWbemLoc);
		//VERIFY(SUCCEEDED(hr));

		//使用m_pWbemLoc连接到"root\cimv2"并设置m_pWbemSvc的指针
		hr=m_pWbemLoc->ConnectServer(CComBSTR(L"root\\cimv2"),
			NULL,
			NULL,
			0,
			NULL,
			0,
			0,
			&m_pWbemSvc);
		//VERIFY(SUCCEEDED(hr));

		//三、设置WMI连接的安全性
		hr=CoSetProxyBlanket(m_pWbemSvc,
			RPC_C_AUTHN_WINNT,
			RPC_C_AUTHZ_NONE,
			NULL,
			RPC_C_AUTHN_LEVEL_CALL,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			EOAC_NONE);
		//VERIFY(SUCCEEDED(hr));

	}
	return(hr);
}

HRESULT CWmiInfo::ReleaseWmi()
{
	HRESULT hr;

	if (NULL != m_pWbemSvc)
	{
		hr=m_pWbemSvc->Release();
	}
	if (NULL != m_pWbemLoc)
	{
		hr=m_pWbemLoc->Release();
	}
	if (NULL != m_pEnumClsObj)
	{
		hr=m_pEnumClsObj->Release();
	}

	::CoUninitialize();

	return(hr);
}

BOOL CWmiInfo::GetWMIQueryResult(CString strSQL,map<CStringA, CSimpleArray<CStringA>> &table)
{
     BOOL bRet = FALSE;

    USES_CONVERSION;


    VARIANT vtProp;
    ULONG uReturn;
    HRESULT hr;
    TCHAR tszFieldNameList[1024] = {0};

    TCHAR* tszRealFieldNamleList = NULL;

   StringCbCopy(tszFieldNameList,sizeof(tszFieldNameList),strSQL.GetBuffer(0));
   TCHAR * tszFrom = StrStrI(tszFieldNameList,_T(" from "));
   if(tszFrom)
   {
       *tszFrom = 0;
   }
   if(_tcslen(tszFieldNameList))
   {
        tszRealFieldNamleList =tszFieldNameList + 7;
        while(tszRealFieldNamleList[0] ==_T(' ') )
        {
            tszRealFieldNamleList++;
        }
        int iLen = _tcslen(tszRealFieldNamleList);
        while(tszRealFieldNamleList[iLen-1] == _T(' '))
        {
            tszRealFieldNamleList[iLen-1] = 0;
            iLen--;
        }
   }
   else
   {
       return FALSE;
   }
    BOOL bAsterisk=FALSE;
    if(StrStrI(tszFieldNameList,_T("*")))
    {
        bAsterisk = TRUE;
    }
    else
    {
        bAsterisk = FALSE;
    }
    m_pEnumClsObj = NULL;
    m_pWbemClsObj = NULL;
    if (NULL != m_pWbemSvc)
    {
      
       
            CComBSTR bstrSQL = strSQL;
          
            hr=m_pWbemSvc->ExecQuery(CComBSTR("WQL"),bstrSQL,WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,&m_pEnumClsObj);
            if (SUCCEEDED(hr))
            {
                //初始化vtProp值
                VariantInit(&vtProp);
                uReturn=0;

                while (m_pEnumClsObj)  
                {  
                //返回从当前位置起的第一个对象到m_pWbemClsObj中
                    hr=m_pEnumClsObj->Next(WBEM_INFINITE,1,&m_pWbemClsObj,&uReturn);
                    if(SUCCEEDED(hr)&&uReturn>0) 
                    {
                        m_pWbemClsObj->BeginEnumeration(0);
                        CComBSTR bstrFieldName;
                        _variant_t varFieldValue;
                         HRESULT hr = S_OK;
                        do 
                        {
                            hr = m_pWbemClsObj->Next(0,&bstrFieldName,&varFieldValue,NULL,NULL);
                            if(varFieldValue.vt != VT_NULL && varFieldValue.vt != VT_EMPTY)
                            {
                                CStringA strFieldName = CW2A(bstrFieldName);
                                if(bAsterisk || StrStrI(tszRealFieldNamleList,OLE2T(bstrFieldName)) != NULL)
                                { 
                                    bRet = TRUE;
                                    CString strFieldValue;

                                    VariantToString(&varFieldValue,strFieldValue);
                                    map<CStringA, CSimpleArray<CStringA>>::iterator iter;
                                    iter = table.find(strFieldName);
                                    if (iter != table.end())
                                    {
                                        CStringA strFieldValueA = CT2A(strFieldValue);
                                        iter->second.Add(strFieldValueA);
                                    }
                                    else
                                    {
                                        CSimpleArray<CStringA> arrayFieldValue;
                                        CStringA strFieldValueA = CT2A(strFieldValue);
                                        arrayFieldValue.Add(strFieldValueA);
                                        table.insert(make_pair(strFieldName,arrayFieldValue));

                                    }
                                }

                            }
                          
                           

                        } while (hr != WBEM_S_NO_MORE_DATA);
                        if(m_pWbemClsObj)
                        {
                            m_pWbemClsObj->EndEnumeration();
                        }
                        else
                        {
                            break;
                        }
                        
         
                    }
                    else
                    {
                        break;
                    }
                }
            }
       

    }
    if(NULL != m_pEnumClsObj) 
    {
        hr=m_pEnumClsObj->Release();
        m_pEnumClsObj = NULL;
    }
    if(NULL != m_pWbemClsObj) 
    {
        hr=m_pWbemClsObj->Release();
        m_pWbemClsObj = NULL;
    }

    return bRet;
}
#if 0
BOOL CWmiInfo::GetSingleItemInfo(CString Class,CString ClassMember,CString &chRetValue)
{
	USES_CONVERSION;

	
	VARIANT vtProp;
	ULONG uReturn;
	HRESULT hr;
	BOOL bRet = FALSE;
	BOOL bPropertyExist = FALSE;
	m_pEnumClsObj = NULL;
	m_pWbemClsObj = NULL;
	if (NULL != m_pWbemSvc)
	{
		//查询类ClassName中的所有字段,保存到m_pEnumClsObj中
		IWbemClassObject *pWbemClassObject = NULL;
		HRESULT hRes = m_pWbemSvc->GetObject(
							CComBSTR(Class), // object path
							 0L, NULL,
							 &pWbemClassObject, // pointer to object
							NULL);

		if(SUCCEEDED(hRes))
		{
			SAFEARRAY *psaNames = NULL;
			hRes = pWbemClassObject->GetNames(
						 NULL,
						 WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY,
						 NULL,
						&psaNames);
			long lLower, lUpper;
			SafeArrayGetLBound(psaNames, 1, &lLower);
			SafeArrayGetUBound(psaNames, 1, &lUpper);
			CComBSTR PropName = NULL;
			CComBSTR MethodName = NULL;
			for (long i = lLower; i <= lUpper; i++)
			{
				hRes = SafeArrayGetElement(
					psaNames,
					&i,
					&PropName);
				IWbemQualifierSet *pQualSet = NULL;

				hRes = pWbemClassObject->GetPropertyQualifierSet(
							PropName, // name of property
							&pQualSet); // qualifier set pointer
				if (SUCCEEDED(hRes))
				{
					VARIANT pVal;
					VariantClear(&pVal);
					pQualSet->Get(
						L"CIMTYPE", // property name
						0L,
						&pVal, // output to this variant
						NULL);
						
					if (SUCCEEDED(hRes))
					{
						V_BSTR(&pVal);
					}
					pQualSet->Release();

				}
				hRes = pWbemClassObject->GetMethodQualifierSet(
					PropName, // name of property
					&pQualSet); // qualifier set pointer
				if (SUCCEEDED(hRes))
				{
					pQualSet->Release();
				}

			
				CString sProp = CString(PropName);
				if(sProp.CompareNoCase(ClassMember)==0)
				{
					bPropertyExist = TRUE;
				}

			}
			SafeArrayDestroy(psaNames);
			
			pWbemClassObject->Release();
		}
	
		if(bPropertyExist)
		{
			CComBSTR bstrSQL = L"Select * from ";
			bstrSQL += Class;
			hr=m_pWbemSvc->ExecQuery(CComBSTR("WQL"),bstrSQL,WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,
				NULL,&m_pEnumClsObj);
			if (SUCCEEDED(hr))
			{
				//初始化vtProp值
				VariantInit(&vtProp);
				uReturn=0;

				//返回从当前位置起的第一个对象到m_pWbemClsObj中
				hr=m_pEnumClsObj->Next(WBEM_INFINITE,1,&m_pWbemClsObj,&uReturn);
				if(SUCCEEDED(hr)&&uReturn>0) 
				{
					//从m_pWbemClsObj中找出ClassMember标识的成员属性值,并保存到vtProp变量中
					hr=m_pWbemClsObj->Get(CComBSTR(ClassMember),0,&vtProp,0,0);
					if (SUCCEEDED(hr))
					{
						VariantToString(&vtProp,chRetValue);
						VariantClear(&vtProp);//清空vtProp
						bRet = TRUE;
					}
				}
			}
		}
	
	}
	if(NULL != m_pEnumClsObj) 
	{
		hr=m_pEnumClsObj->Release();
		m_pEnumClsObj = NULL;
	}
	if(NULL != m_pWbemClsObj) 
	{
		hr=m_pWbemClsObj->Release();
		m_pWbemClsObj = NULL;
	}
	return bRet;
}

BOOL CWmiInfo::GetGroupItemInfo(CString SQL,CString ClassMember[],int n,CString &chRetValue)
{
	USES_CONVERSION;

	
	CString result,info;
	VARIANT vtProp;
	ULONG uReturn;
	HRESULT hr;
	int i;
	BOOL bRet = FALSE;
	if (NULL  != m_pWbemSvc)
	{
		
	
		hr=m_pWbemSvc->ExecQuery(CComBSTR("WQL"), CComBSTR(SQL),WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,0,&m_pEnumClsObj);
		if (SUCCEEDED(hr))
		{
			VariantInit(&vtProp); //初始化vtProp变量
			if(m_pEnumClsObj)  
			{
				Sleep(10);
				uReturn=0;
				hr=m_pEnumClsObj->Next(WBEM_INFINITE,1,&m_pWbemClsObj,&uReturn);
				if (SUCCEEDED(hr) &&uReturn>0)
				{
					for(i=0;i<n;++i)
					{
						hr=m_pWbemClsObj->Get(CComBSTR(ClassMember[i]),0,&vtProp,0,0);
						if (SUCCEEDED(hr))
						{
							VariantToString(&vtProp,info);
							chRetValue+=info+_T("\t");
							VariantClear(&vtProp);
							bRet = TRUE;
						}
					}
					chRetValue+=_T("\r\n");
				}
			}
		}
	}

	if(NULL != m_pEnumClsObj)
	{
		hr=m_pEnumClsObj->Release();
		m_pEnumClsObj=NULL;
	}
	if(NULL != m_pWbemClsObj)
	{
		hr=m_pWbemClsObj->Release();
		m_pWbemClsObj=NULL;
	}
	return bRet;
}

#endif
void CWmiInfo::VariantToString(const LPVARIANT pVar,CString &chRetValue) const
{
	USES_CONVERSION;

	CComBSTR HUGEP* pBstr;
	BYTE HUGEP* pBuf;
	LONG low,high,i;
	switch(pVar->vt)
	{
	case VT_BSTR:
		{
			chRetValue=W2T(pVar->bstrVal);
		}
		break;
	case VT_BOOL:
		{
			if(VARIANT_TRUE==pVar->boolVal) 
				chRetValue="TRUE";
			else
				chRetValue="FALSE";
		}
		break;
	case VT_I4:
		{
			chRetValue.Format(_T("%d"),pVar->lVal);
		}
		break;
	case VT_UI1:
		{
			chRetValue.Format(_T("%d"),pVar->bVal);
		}
		break;
	case VT_UI4:
		{
			chRetValue.Format(_T("%u"),pVar->ulVal);
		}
		break;

	case VT_BSTR|VT_ARRAY:
		{
			SafeArrayAccessData(pVar->parray,(void HUGEP**)&pBstr);
			SafeArrayUnaccessData(pVar->parray);
			chRetValue=W2T(pBstr->m_str);
		}
		break;

	case VT_I4|VT_ARRAY:
		{
			SafeArrayGetLBound(pVar->parray,1,&low); 
			SafeArrayGetUBound(pVar->parray,1,&high);

			SafeArrayAccessData(pVar->parray,(void HUGEP**)&pBuf);
			SafeArrayUnaccessData(pVar->parray);
			CString strTmp;
			high=min(high,MAX_PATH*2-1);
			for(i=low;i<=high;++i)
			{
				strTmp.Format(_T("%02X"),pBuf[i]);
				chRetValue+=strTmp;
			}
		}
		break;
	default:
		break;
	}
}

BOOL GetWmiObjectJson(_bstr_t bstrWmiObject,_bstr_t& bstrJson)
{
    BOOL bResult = FALSE;
    CoInitialize(NULL);
	try
	{		
		WbemScripting::ISWbemLocatorPtr locator;
		locator.CreateInstance(WbemScripting::CLSID_SWbemLocator);
		if (locator != NULL)
		{
			WbemScripting::ISWbemServicesPtr services = locator->ConnectServer(".","root\\cimv2","","","","",0,NULL);
            _bstr_t bstrWQL;
            if(_strnicmp(bstrWmiObject.operator const char*(),"select ",7)==0)
            {
                bstrWQL = "";
            }
            else
            {
                bstrWQL = "Select * from ";
            }
            bstrWQL += bstrWmiObject;
			WbemScripting::ISWbemObjectSetPtr objects = services->ExecQuery(bstrWQL,"WQL",0x10,NULL);
			IEnumVARIANTPtr obj_enum = objects->Get_NewEnum();	
			ULONG fetched;
			_variant_t var;
            bstrJson ="[";
            ULONG uRowCount = 0;
			while (obj_enum->Next(1,&var,&fetched) == S_OK)
			{
				WbemScripting::ISWbemObjectPtr object = var;
				WbemScripting::ISWbemPropertySetPtr properties = object->Properties_;
               	IEnumVARIANTPtr prop_enum = properties->Get_NewEnum();	

                ULONG fetched;
                _variant_t varProp;
                if(uRowCount >0)
                {
                     bstrJson +=",";
                }
                uRowCount++;
                bstrJson +="{";
                ULONG ulColCount = 0;
                while (prop_enum->Next(1,&varProp,&fetched) == S_OK)
                {
                
                   WbemScripting::ISWbemPropertyPtr prop = varProp;  
                   _variant_t value = prop->GetValue();
                   if(ulColCount >0)
                   {
                       bstrJson +=",";
                   }
                    ulColCount++;
                    bstrJson +="\"";
                    bstrJson +=(const char*)_bstr_t(prop->GetName());
                    bstrJson +="\":";
                    bstrJson +="\""; 
                   switch(value.vt)
                    {
                        case VT_EMPTY:
                        case VT_DISPATCH:
            
                        case VT_NULL:
                            break;
                        case VT_R4:
                            {
                                TCHAR tszTemp[30] = {0};
                                wsprintf(tszTemp,_T("%f"),value.fltVal);
                                bstrJson += tszTemp;
                                break; 
                            }
                        case VT_R8:
                        case VT_DECIMAL:
                            {
                                TCHAR tszTemp[30] = {0};
                                wsprintf(tszTemp,_T("%f"),value.dblVal);
                                bstrJson += tszTemp;
                                break; 
                            }
                        case VT_BOOL:
                            {
                                bstrJson += value.boolVal?"true":"false";
                                break; 
                            }
                        case VT_I2:
                            {
                                TCHAR tszTemp[30] = {0};
                                wsprintf(tszTemp,_T("%d"),(short)value);
                                bstrJson +=tszTemp;
                                break;
                            }
                        case VT_I4:
                            {
                                TCHAR tszTemp[30] = {0};
                                wsprintf(tszTemp,_T("%d"),value.intVal);
                                bstrJson +=tszTemp;
                                break;
                            }
                        case VT_UI2:
                            {
                                TCHAR tszTemp[30] = {0};
                                wsprintf(tszTemp,_T("%u"),(unsigned short)value);
                                bstrJson +=tszTemp;
                                break;
                            }
                        case VT_UI4:
                            {
                                TCHAR tszTemp[30] = {0};
                                wsprintf(tszTemp,_T("%u"),value.uintVal);
                                bstrJson +=tszTemp;
                                break;
                            }
                        case VT_BSTR:
                           bstrJson += (const char*)_bstr_t(value.bstrVal);
                           break;
                        case VT_BSTR|VT_ARRAY:   
                          {    
                                _bstr_t bstrTemp;
                                SafeArrayAccessData(value.parray,(void HUGEP**)&bstrTemp);   
                                SafeArrayUnaccessData(value.parray);   
                                bstrJson +=bstrTemp;
                                break;
                          }
                        case VT_I4|VT_ARRAY:   
                            {    
                                 LONG low,high;
                                 SafeArrayGetLBound(value.parray,1,&low);    
                                 SafeArrayGetUBound(value.parray,1,&high); 
                                 BYTE HUGEP* pBuf=NULL;  
                                 SafeArrayAccessData(value.parray,(void HUGEP**)&pBuf);   
                                 SafeArrayUnaccessData(value.parray);   
                                 TCHAR tszTemp[30];    
                                 
                                 for(int i=low;i<=high;++i)   
                                 {    
                                        if(i!=low)
                                        {
                                            bstrJson += ",";
                                        }
                                        
                                        wsprintf(tszTemp,_T("%8X"),pBuf[i]);   
                                        bstrJson += tszTemp;   
                                 }   


                            }   
                            break;  
                        case VT_VARIANT|VT_ARRAY:   
                            {    
                                LONG low,high;
                                SafeArrayGetLBound(value.parray,1,&low);    
                                SafeArrayGetUBound(value.parray,1,&high); 
                                variant_t HUGEP* pBuf=NULL;  
                                SafeArrayAccessData(value.parray,(void HUGEP**)&pBuf);   
                                SafeArrayUnaccessData(value.parray);   
                                TCHAR tszTemp[30];    

                                for(int i=low;i<=high;++i)   
                                {    
                                    if(i!=low)
                                    {
                                        bstrJson += ",";
                                    }
                                    if(pBuf[i].vt == VT_I4) 
                                    {
                                        wsprintf(tszTemp,_T("%8X"),pBuf[i].intVal);   
                                        bstrJson += tszTemp;   

                                    }
                                    else if(pBuf[i].vt == VT_UI4 )
                                    {
                                        wsprintf(tszTemp,_T("%8X"),pBuf[i].uintVal);   
                                        bstrJson += tszTemp;   
                                    }
                                    else if(pBuf[i].vt == VT_I2  )
                                    {
                                        wsprintf(tszTemp,_T("%4X"),(short)pBuf[i]);   
                                        bstrJson += tszTemp;   

                                    } 
                                    else if(pBuf[i].vt == VT_UI2  )
                                    {
                                        wsprintf(tszTemp,_T("%4X"),(unsigned short)pBuf[i]);   
                                        bstrJson += tszTemp;   

                                    }
                                }   

                            }   
                            break;  
                      
                        default:
                            Sleep(10);
                            break;


                    }
                    bstrJson +="\""; 
                   //printf("Name: %s Value:%s\n",(const char*)_bstr_t(prop->GetName()),(const char*)_bstr_t(prop->GetValue()));
                }
                bstrJson +="}";
				/*WbemScripting::ISWbemPropertyPtr prop = properties->Item("AdapterTypeID",0);
				_variant_t value = prop->GetValue();*/
			/*	if (value.vt == VT_I4 && (int)value == 0)
				{
					prop = properties->Item("MACAddress",0);
					printf("MAC address found: %s\n",(const char*)_bstr_t(prop->GetValue()));
				}*/
			}
            bstrJson +="]";
            bResult = TRUE;
		}
	}
	catch (_com_error err)
	{
		printf("Error ocured: %S",err.ErrorMessage());
	}
	CoUninitialize();
    return bResult;
}






