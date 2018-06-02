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

// DragEventListener.h: interface for the CDragEventListener class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAGEVENTLISTENER_H__54270A47_B749_49E6_8CFC_53F14E9EA13D__INCLUDED_)
#define AFX_DRAGEVENTLISTENER_H__54270A47_B749_49E6_8CFC_53F14E9EA13D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDragEventListener  
{
public:
	virtual void OnDragMove(POINT point) =0;
	virtual void OnDragRelease(POINT point,HTREEITEM hDropTarget) =0;
	virtual void OnDrag() =0;

};

#endif // !defined(AFX_DRAGEVENTLISTENER_H__54270A47_B749_49E6_8CFC_53F14E9EA13D__INCLUDED_)
