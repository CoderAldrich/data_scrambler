/*
 *
 * Copyright 2010 JiJie Shi
 *
 * This file is part of data_scrambler.
 *
 * data_scrambler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * data_scrambler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with data_scrambler.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "linguish_process.h"
#include "Result\result.h" 
#include "Utility\Utility.h"

#pragma comment(lib, "ictclas")

CResult m_ICTCLAS;
LRESULT WINAPI text_word_segment( LPCSTR text, 
								 INT32 output_format, 
								 INT32 operator_type, 
								 CStringA &text_out )
{
	LRESULT ret = ERROR_SUCCESS; 
	char *sResult,*sSource;
	ULONG m_nResultNum = 1; 

	do 
	{
		ASSERT(NULL != text ); 

		m_ICTCLAS.m_nOutputFormat=output_format;
		m_ICTCLAS.m_nOperateType=operator_type;
		clock_t start, finish;
		if(output_format!=2)
			sResult=new char [(strlen((char *)(LPCTSTR)text)+13)*3];
		else
			sResult=new char [(strlen((char *)(LPCTSTR)text)+13)*50];
		if(m_nResultNum==1)
		{
			start=clock();
			if(!m_ICTCLAS.ParagraphProcessing((char *)(LPCTSTR)text,sResult))
				text_out.Format("错误：程序初始化异常！");
			else
				text_out.Format("%s",sResult);
			finish=clock();
		}
		else
		{
			start=clock();
			sSource=new char[strlen((char *)(LPCTSTR)text)+20];
			strcpy(sSource,SENTENCE_BEGIN);
			strcat(sSource,(char *)(LPCTSTR)text);
			strcat(sSource,SENTENCE_END);
			if(!m_ICTCLAS.Processing(sSource,m_nResultNum))
				text_out.Format("错误：程序初始化异常！");
			else
			{
				finish=clock();
				text_out.Empty();
				CStringA sTemp;
				for(int i=0;i<m_ICTCLAS.m_nResultCount;i++)
				{
					sTemp.Format("Result %d(Score=%f):",i+1,m_ICTCLAS.m_dResultPossibility[i]);
					text_out+=sTemp;
					m_ICTCLAS.Output(m_ICTCLAS.m_pResult[i],sResult,true);
					text_out+=sResult;
					text_out+="\r\n";
				}
			}
			delete [] sSource;
		}
		delete [] sResult;

	}while( FALSE ); 

	return ret; 
}
