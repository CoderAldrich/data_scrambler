#include "stdafx.h"
#include "data_scrambler.h"
#include "data_list_dlg.h"
//#import <mshtml.tlb>
#include <mshtml.h>
#include <comutil.h>
#include <comdef.h>
#include <comdefsp.h>
#include "mshtml_addition.h"
#include <comutil.h>

/**************************************************************
如何提高图像的有效性，可能会取出一些重要性不大的图片。
**************************************************************/

voidDisplayImage(HDC hDC, CString fileName)  
02.   {  
	03.          CString fileExt; //图片的扩展名  
	04.          int len = fileName.GetLength();  
	05.          for(int i=len-1; i>=0; i--) //得到图片的扩展名  
		06.          {  
			07.                  if(fileName[ i ] == '.')  
				08.                  {  
					09.                          fileExt=fileName.Mid(i+1);  
					10.                          break;  
					11.                  }  
			12.          }  
	13.          fileExt.MakeLower(); //将扩展名转为小写  
	14.          if(fileExt != _T(""))  
		15.          {  
			16.                  //创建CxImage对象，其中静态方法CxImage::GetTypeIdFromName用来根据扩展名获得图片格式的ID代表  
				17.                  CxImage image(fileName,CxImage::GetTypeIdFromName(fileExt));  
			18.                  if(image.IsValid())  
				19.                  {  
					20.                          image.Draw(hDC);  
					21.                          image.Destroy();  
					22.                  }  
			23.          }  
	24.   }  


void DisplayImage(HDC hDC, LPCTSTR szImagePath)  
02.   {  
	03.          HANDLE hFile=CreateFile(szImagePath, GENERIC_READ, FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//从指定的路径szImagePath中读取文件句柄  
	04.          DWORD dwFileSize=GetFileSize(hFile, NULL);//获得图片文件的大小，用来分配全局内存  
	05.          HGLOBAL hImageMemory=GlobalAlloc(GMEM_MOVEABLE, dwFileSize);//给图片分配全局内存  
	06.          void *pImageMemory=GlobalLock(hImageMemory); //锁定内存  
	07.          DWORD dwReadedSize; //保存实际读取的文件大小  
	08.          ReadFile(hFile, pImageMemory, dwFileSize,&dwReadedSize, NULL); //读取图片到全局内存当中  
	09.          GlobalUnlock(hImageMemory); //解锁内存  
	10.          CloseHandle(hFile); //关闭文件句柄  
	11.     
		12.          IStream *pIStream;//创建一个IStream接口指针，用来保存图片流  
	13.          IPicture *pIPicture;//创建一个IPicture接口指针，表示图片对象  
	14.          CreateStreamOnHGlobal(hImageMemory, false,&pIStream) //用全局内存初使化IStream接口指针  
		15.          OleLoadPicture(pIStream, 0, false, IID_IPicture,(LPVOID*)&(pIPicture));//用OleLoadPicture获得IPicture接口指针  
	16.     
		17.          //得到IPicture COM接口对象后，你就可以进行获得图片信息、显示图片等操作  
		18.          OLE_XSIZE_HIMETRIC hmWidth;  
	19.          OLE_YSIZE_HIMETRIC hmHeight;  
	20.          pIPicture->get_Width(&hmWidth);//用接口方法获得图片的宽和高  
	21.          pIPicture->get_Height(&hmHeight);  
	22.          pIPicture->Render(hDC,0,0,100,100,0,hmHeight,hmWidth,-hmHeight,NULL);//在指定的DC上绘出图片  
	23.     
		24.          GlobalFree(hImageMemory); //释放全局内存  
	25.          pIStream->Release(); //释放pIStream  
	26.          pIPicture->Release(); //释放pIPicture  
	27.   }  
