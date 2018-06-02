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

// DragDropContainer.h: interface for the CDragDropContainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAGDROPCONTAINER_H__14DF1D87_F782_42DA_8CCF_8FBC3668B612__INCLUDED_)
#define AFX_DRAGDROPCONTAINER_H__14DF1D87_F782_42DA_8CCF_8FBC3668B612__INCLUDED_

#include "DragDropTreeCtrl.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CDragDropTreeCtrl;
class CDragDropContainer  
{
public:
	CImageList* GetDragImage();
	void SetDragImage(CImageList *pImageList);
	CDragDropTreeCtrl* GetDropTarget();
	CDragDropTreeCtrl* GetDragSource();
	void SetSecondComponent(CDragDropTreeCtrl *pSecondTree);
	void SetFirstComponent(CDragDropTreeCtrl *pFirstTree);
	BOOL IsDragging();
	void SetIsDragging(BOOL bDragging);
	void RegisterDragSource(CDragDropTreeCtrl *pTreeCtrl);
	CDragDropContainer();
	virtual ~CDragDropContainer();

private:
	CImageList *m_pImageList;
	CDragDropTreeCtrl *m_pSecondTree;
	CDragDropTreeCtrl *m_pFirstTree;
	CDragDropTreeCtrl *m_pDragSource;
  CDragDropTreeCtrl *m_pDropTarget;
  	BOOL m_bDragging;
};

#endif // !defined(AFX_DRAGDROPCONTAINER_H__14DF1D87_F782_42DA_8CCF_8FBC3668B612__INCLUDED_)
