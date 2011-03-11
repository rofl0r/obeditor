/*
 * prListBox.cpp
 *
 *  Created on: 23 avr. 2009
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__prListBox.h"


//**************************************************************************************
//**************************************************************************************

prListBox::prListBox( wxWindow* _parent, wxWindowID _id, const int _maxWidth, long style )
:wxListBox( _parent, _id, wxDefaultPosition,wxDefaultSize, 0, NULL, style )
{
	mymaxWidth = _maxWidth;
}

//**************************************************************************************
//**************************************************************************************

prListBox::~prListBox()
{
}

//**************************************************************************************
//**************************************************************************************
int 
prListBox::Append( const wxString&  item)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	int res = wxListBox::Append( item );
	_SetMaxWidth( _w );
	return res;
}

//**************************************************************************************
int 
prListBox::Append( const wxString&  item, void* _datas)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	int res = wxListBox::Append( item, _datas );
	_SetMaxWidth( _w );
	return res;
}

//**************************************************************************************
//**************************************************************************************

void prListBox::Append( const wxArrayString& strings)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	wxListBox::Append( strings );
	_SetMaxWidth( _w );
}


//**************************************************************************************
//**************************************************************************************

void prListBox::Clear()
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	wxListBox::Clear();
	_SetMaxWidth( _w );
}


//**************************************************************************************
//**************************************************************************************

void prListBox::Delete(unsigned int n)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	wxListBox::Delete(n);
	_SetMaxWidth( _w );
}


//**************************************************************************************
//**************************************************************************************
void 
prListBox::Insert(const wxString&  item, unsigned int pos)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	wxListBox::Insert(item, pos);
	_SetMaxWidth( _w );
}

//**************************************************************************************
void 
prListBox::Insert(const wxString&  item, unsigned int pos, void* _datas)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	wxListBox::Insert(item, pos, _datas);
	_SetMaxWidth( _w );
}

//**************************************************************************************
//**************************************************************************************

void prListBox::SetString(unsigned int n, const wxString&  string)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	wxListBox::SetString(n,string);
	_SetMaxWidth( _w );
}


//**************************************************************************************
//**************************************************************************************

bool prListBox::SetStringSelection(const wxString&  string)
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	bool res = wxListBox::SetStringSelection(string);
	_SetMaxWidth( _w );
	return res;
}


//**************************************************************************************
//**************************************************************************************

void prListBox::SetMaxWidth( const int _maxWidth )
{
	int _w = GetWidth();
	SetMinSize( wxSize( 10, wxDefaultCoord ));
	mymaxWidth = _maxWidth;
	_SetMaxWidth( _w );
}


//**************************************************************************************
//**************************************************************************************

int  prListBox::GetMaxWidth()const
{
	return mymaxWidth;
}

//**************************************************************************************
//**************************************************************************************

int  prListBox::GetWidth()
{
	wxSize s = GetMinSize();
	return s.GetWidth();
}

//**************************************************************************************
//**************************************************************************************

void prListBox::_SetMaxWidth( int _w )
{
	int _w0, _w1, _h;
	GetBestSize(&_w0, &_h );
	GetSize(&_w1, &_h );
	if( _w0 > mymaxWidth || _w1 > mymaxWidth )
		_w = mymaxWidth;

	SetMinSize( wxSize( _w, wxDefaultCoord ));
	SetMaxSize( wxSize( _w, wxDefaultCoord ));

	if( GetContainingSizer() != NULL )
		GetContainingSizer()->Layout();
}


