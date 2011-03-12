/*
 * wListAnims.h
 *
 *  Created on: 6 nov. 2008
 *      Author: pat
 */

#ifndef WLISTANIMS_H_
#define WLISTANIMS_H_

#include "common__tools.h"

//****************************************************
//****************************************************

class myListBox : public wxListBox
{
public:
	myListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxT("listBox"));
	void OnCharPress(wxKeyEvent& event);
	virtual void OnSelectionChange( wxCommandEvent& event );
	virtual void EvtGetFocus(wxFocusEvent&);
private:
    DECLARE_EVENT_TABLE()
};


//****************************************************
//****************************************************

class wListAnims : public myListBox
{
public:
	wListAnims(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxT("listBox"));
	void AssociateToListFrame( wxWindow* _wList_frames);
	virtual ~wListAnims();

	bool pSetSelection( const wxString& s );
	
	virtual void EvtGetFocus(wxFocusEvent& event );
	virtual void EvtCharPress(wxKeyEvent& event);

	void Refresh_List(bool b_keep_current_selection = true);
	void RefreshSelection();
	void ReloadLastSessionAnim();
	virtual void OnSelectionChange( wxCommandEvent& event );
	virtual void ProcessSelectionChange();

	wxWindow* wList_frames;

private:
	bool loading;
	wxString last_session_anim;
	
	// map between indice of wxListBox Order and indice of ObFile Anim order
	int map_ind_numAnim[200];
    DECLARE_EVENT_TABLE()
};


//****************************************************

class wListAnims_Editable : public wListAnims
{
public:
	wListAnims_Editable(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxT("listBox"));
	virtual ~wListAnims_Editable();

	virtual void EvtCharPress(wxKeyEvent& event);
	void EvtDeleteAnim(wxCommandEvent& event );
	void EvtCopyAnim(wxCommandEvent& event );
	void EvtNewAnim(wxCommandEvent& event );
	void EvtRenameAnim(wxCommandEvent& event );
	void EvtContextMenu( wxContextMenuEvent& event );

private:
	wxMenu *popMenu;
    DECLARE_EVENT_TABLE()
};

#endif
