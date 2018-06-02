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
// DragDropContainer.cpp: implementation of the CDragDropContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DragDropContainer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDragDropContainer::CDragDropContainer()
{
  m_bDragging = FALSE;
  m_pDragSource = NULL;
  m_pDropTarget = NULL;
}

CDragDropContainer::~CDragDropContainer()
{

}

void CDragDropContainer::RegisterDragSource(CDragDropTreeCtrl *pTreeCtrl)
{
  m_pDragSource = pTreeCtrl;
  if( m_pDragSource == m_pFirstTree )
  {
    m_pDropTarget = m_pSecondTree;
  }
  else
  {
    m_pDropTarget = m_pFirstTree;
  }
}

void CDragDropContainer::SetIsDragging(BOOL bDragging)
{
  m_bDragging = bDragging;
}

BOOL CDragDropContainer::IsDragging()
{
  return m_bDragging;
}

void CDragDropContainer::SetFirstComponent(CDragDropTreeCtrl *pFirstTree)
{
  m_pFirstTree = pFirstTree;
}

void CDragDropContainer::SetSecondComponent(CDragDropTreeCtrl *pSecondTree)
{
  m_pSecondTree = pSecondTree;
}

CDragDropTreeCtrl* CDragDropContainer::GetDragSource()
{
  return m_pDragSource;
}

CDragDropTreeCtrl* CDragDropContainer::GetDropTarget()
{
  return m_pDropTarget;
}

void CDragDropContainer::SetDragImage(CImageList *pImageList)
{
  m_pImageList = pImageList;
}

CImageList* CDragDropContainer::GetDragImage()
{
  return m_pImageList;
}
