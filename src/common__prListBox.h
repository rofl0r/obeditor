/*
 * myListBox.h
 *
 *  Created on: 23 avr. 2009
 *      Author: pat
 */

#ifndef PRLISTBOX_H_
#define PRLISTBOX_H_

#include <wx/listbox.h>


// Fix the SetMaxSize Bug of the wxListBox object

class prListBox : public wxListBox
{
public:
	prListBox( wxWindow* parent, wxWindowID id = wxID_ANY, const int _maxWidth = wxDefaultCoord, long style = wxLB_HSCROLL );
	~prListBox();

	virtual int  Append( const wxString&  item);
	virtual int  Append( const wxString&  item, void* _datas );
	virtual void Append( const wxArrayString& strings );
	virtual void Clear();
	virtual void Delete(unsigned int n);
	virtual void Insert(const wxString&  item, unsigned int pos);
	virtual void Insert(const wxString&  item, unsigned int pos, void* _datas);
	virtual void SetString(unsigned int n, const wxString&  string);
	virtual bool SetStringSelection(const wxString&  string);

	void SetMaxWidth( const int _maxWidth );
	int  GetMaxWidth()const;
	int  GetWidth();

private:
	void _SetMaxWidth(int _w);
	int mymaxWidth;
};

#endif /* PRLISTBOX_H_ */
