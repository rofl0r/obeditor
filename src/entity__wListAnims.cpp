/*
 * wListAnims.cpp
 *
 *  Created on: 6 nov. 2008
 *      Author: pat
 */

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <list>
using namespace std;

#include "ob_editor.h"
#include "entity__wListAnims.h"
#include "entity__enums.h"


//****************************************************
//*********** new event
DEFINE_EVENT_TYPE(wxEVT_ANIM_LIST_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_ANIM_SELECTED_CHANGE)



//****************************************************
//****************************************************

myListBox::myListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, int n,
		const wxString choices[], long style, const wxValidator& validator, const wxString& name)
:wxListBox(parent, id, pos, size, n, choices, style, validator, name)
{
}

void myListBox::OnCharPress(wxKeyEvent& event)
{
	int kc = event.GetKeyCode();
	wxChar ch = event.GetKeyCode();
	wxString temp = ch;
	temp = temp.Upper();
	ch = temp[0];

	wxCommandEvent _event;
	if( kc == WXK_DOWN )
	{
		int new_ind = (GetSelection() + 1 < (int)GetCount() ) ? GetSelection() + 1 : GetCount() -1;
		SetSelection(new_ind );
		OnSelectionChange( _event );
		return;
	}
	if( kc == WXK_UP )
	{
		int new_ind = (GetSelection() - 1 >= 0 ) ? GetSelection() - 1 : 0;
		SetSelection( new_ind );
		OnSelectionChange(  _event );
		return;
	}
	if( kc == WXK_PAGEUP )
	{
		int new_ind = (GetSelection() - 10 >= 0 ) ? GetSelection() - 10 : 0;
		SetSelection( new_ind );
		OnSelectionChange(  _event );
		return;
	}
	if( kc == WXK_PAGEDOWN )
	{
		int new_ind = (GetSelection() + 10 < (int)GetCount() ) ? GetSelection() + 10 : GetCount() -1;
		SetSelection( new_ind );
		OnSelectionChange( _event );
		return;
	}

	for( size_t i = 0; i < GetCount();i++)
	{
		wxString temp = GetString(i).Upper();
		wxChar t = temp[0];

		if( t == ch )
		{
			SetSelection(i);
			OnSelectionChange( _event );
			return;
		}
	}
	event.Skip();
}


void myListBox::OnSelectionChange( wxCommandEvent& event )
{
	event.Skip();
}

void myListBox::EvtGetFocus(wxFocusEvent& event )
{
	event.Skip();
}


//****************************************************
//*********** new event

// Event table
BEGIN_EVENT_TABLE(myListBox, wxListBox)
	EVT_SET_FOCUS(myListBox::EvtGetFocus)
	EVT_CHAR(myListBox::OnCharPress)
END_EVENT_TABLE()



//****************************************************
//			LISTBOX ANIMS
//****************************************************

wListAnims::wListAnims(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, int n,
		const wxString choices[], long style, const wxValidator& validator, const wxString& name)
:myListBox(parent, id, pos, size, n, choices, style, validator, name)
{
	last_session_anim = wxString();
	loading = false;
	wxSize _t(200, 400 );
	SetMinSize( _t );
	arr_anims = NULL;
	arr_anims_count = 0;
	wList_frames = NULL;
}

//*************************************************************************

void wListAnims::AssociateToListFrame( wxWindow* _wList_frames)
{
	wList_frames = _wList_frames;
}


//*************************************************************************

void wListAnims::ReloadLastSessionAnim()
{
	// Load the last anim for this entity
	last_session_anim =  ConfigEdit_Read( wxT("last_anim_edited") );
}

//*************************************************************************

wListAnims::~wListAnims()
{
	if( arr_anims != NULL )
		delete[] arr_anims;
	arr_anims = NULL;
	arr_anims_count = 0;
}


//*************************************************************************

void wListAnims::RefreshSelection()
{
}

//*************************************************************************
/* Refresh the list of anims from the previously loaded arr_anims.
 * Also restore the previous anim selected, if possible
 * Finaly refresh the client zone.
 */
void wListAnims::Refresh_List( bool b_keep_current_selection )
{
	if( entity == NULL || loading )
		return;

	loading = true;

	wxString prev_selected = GetStringSelection();

	// First valid refresh
	if( last_session_anim != wxString() )
	{
		prev_selected = last_session_anim;
		last_session_anim = wxString();
	}
	else
		prev_selected = last_anim_selected;

	// Clear Previous content
	if( GetCount() > 0 )
		Clear();

	// Sort the Anims by Names
	list<int> l_order;
	for( int i = 0; i < arr_anims_count; i ++)
	{
		wxString curr_anim_name = arr_anims[i]->GetToken(0);
			
		list<int>::iterator it (l_order.begin())
					, it_end( (l_order.end()) );
		for( ; it != it_end; it++ )
		{
			wxString anim_name = arr_anims[*it]->GetToken(0);
			
			if( P_StrCmp( curr_anim_name, anim_name ) < 0 )
				break;
		}
		l_order.insert( it, i );
	}
	
	// Fill the listBox items
	int i = 0;
	while( l_order.empty() == false )
	{
		int curr_ind = l_order.front();
		l_order.pop_front();
		map_ind_numAnim[i] = curr_ind;
		Append( arr_anims[curr_ind]->GetToken(0) );
		i++;
	}

	loading = false;

	// Try to restore the last edited anim
	int _ind = 0;
	if( arr_anims_count != 0 )
	{
		if( b_keep_current_selection )
		{
			if( prev_selected != wxString() )
			{
				for( int i = 0; i < arr_anims_count; i ++)
					if( arr_anims[i]->GetToken(0) == prev_selected )
					{
						_ind = i;
						break;
					}
			}
		}
		else
		{
			for( int i = 0; i < arr_anims_count; i ++)
			{
				if( arr_anims[i] == curr_anim )
				{
					_ind = i;
					break;
				}
			}
		}
		curr_anim = arr_anims[_ind];
		last_anim_selected = curr_anim->GetToken( 0 );
	}
	else
	{
		if( arr_anims_count == 0 )
		{
			curr_anim = NULL;
			last_anim_selected = wxString();
		}
		else
		{
			curr_anim = arr_anims[0];
			last_anim_selected = curr_anim->GetToken( 0 );
		}
	}

	if( _ind < 0 || _ind >= arr_anims_count ||  curr_anim == NULL )
		Select( wxNOT_FOUND );
	else
		pSetSelection(curr_anim->arr_token[0] );

	ProcessSelectionChange();
	Refresh();
}

//*************************************************************************
void 
wListAnims::OnSelectionChange( wxCommandEvent& event )
{
	if( loading )
		return;
	ProcessSelectionChange();
	wxCommandEvent _evt(wxEVT_ANIM_SELECTED_CHANGE);
	ProcessEvent( _evt);
}

//*************************************************************************
void 
wListAnims::ProcessSelectionChange()
{
	if( loading )
		return;

	int ind = GetSelection();
	curr_anim = NULL;
	last_anim_selected = wxString();
	if( map_ind_numAnim[ind] >= 0 && map_ind_numAnim[ind] < (int) GetCount() )
	{
		curr_anim = arr_anims[map_ind_numAnim[ind]];
		last_anim_selected = curr_anim->GetToken( 0 );
	}

	if( entityFrame->May_Register_ControlsState() == true )
	{
// 		entityFrame->hist_anim_selected = GetStringSelection();
	entityFrame->Register_ControlsState_inHistory(wxT("ProcessSelectionChange"));
	}
}

//*************************************************************************
bool
wListAnims::pSetSelection( const wxString& s )
{
	
	wxArrayString arr = GetStrings();
	for( size_t i = 0; i < arr.GetCount(); i++ )
	{
		if( arr[i] == s )
		{
			SetSelection( i );
			return true;
		}
	}
	return false;
}


//*************************************************************************
void 
wListAnims::EvtGetFocus(wxFocusEvent& event )
{
	event.Skip();
}


//*************************************************************************

void wListAnims::EvtCharPress(wxKeyEvent& event)
{
	int kc = event.GetKeyCode();

	if( wList_frames != NULL )
	{
		// Resend left and right events to the list of anims
		if( kc == WXK_LEFT ||  kc == WXK_RIGHT  || kc == WXK_HOME || kc == WXK_END)
		{
			wList_frames->ProcessEvent( event );
			return;
		}
	}

	event.Skip();
}


//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************

// Event table
BEGIN_EVENT_TABLE(wListAnims, myListBox)
	EVT_LISTBOX(wxID_ANY,wListAnims::OnSelectionChange)
	EVT_SET_FOCUS(wListAnims::EvtGetFocus)
	EVT_CHAR(wListAnims::EvtCharPress)
END_EVENT_TABLE()



//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************

wListAnims_Editable::wListAnims_Editable(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, int n,
		const wxString choices[], long style, const wxValidator& validator, const wxString& name)
:wListAnims(parent, id, pos, size, n, choices, style, validator, name)
{
	popMenu = new 	wxMenu();
	popMenu->Append( CMD_NEW, wxT("New Anim" ));
	popMenu->Append( CMD_DEL, wxT("Delete Anim") );
	popMenu->Append( CMD_COPY, wxT("Copy Anim" ));
	popMenu->Append( CMD_RENAME, wxT("Rename Anim") );
}


//*************************************************************************

wListAnims_Editable::~wListAnims_Editable()
{
	delete popMenu;
}

//*************************************************************************

void wListAnims_Editable::EvtRenameAnim(wxCommandEvent& event )
{
	int indanim = GetSelection();
	if( indanim < 0 || indanim > arr_anims_count )
		return;

	// Get the elt under the mouse
	wxTextEntryDialog *_dialog = new wxTextEntryDialog(this, wxT("Rename Anim"), wxT("New name for the anim "), wxT("UNNAMED"),
				wxOK | wxCANCEL | wxCENTRE );
	int response = _dialog->ShowModal();

	if( response  == wxID_OK )
	{
		wxString animname = _dialog->GetValue().Trim().Trim(false);
		if( animname == wxString())
			wxMessageBox( wxT("Anim name is empty"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		else
		{
			arr_anims[indanim]->SetToken(0 , animname );
			SetString(indanim, animname);

			// Mark entity as changed
			entity->SetChanged();
		}
	}
}


//*************************************************************************

void wListAnims_Editable::EvtNewAnim( wxCommandEvent& event )
{
//	int indanim = GetSelection();

	wxTextEntryDialog _dialog(this, wxT("New anim"), wxT("Name for the new anim "), wxT("UNNAMED"),
				wxOK | wxCANCEL | wxCENTRE );
	int response = _dialog.ShowModal();

	if( response  == wxID_OK )
	{
		wxString animname = _dialog.GetValue().Trim().Trim(false);
		if( animname == wxString())
			wxMessageBox( wxT("Anim name is empty"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		else
		{
			theHistoryManager.Set_State( false );
			ob_anim* newanim = new ob_anim();
			newanim->SetName(wxT("anim"));
			newanim->SetToken(0 ,animname );
			theHistoryManager.Set_State( true );
			
			theHistoryManager.GroupStart( wxT("New Anim ") + animname );
			entity->obj_container->Add_SubObj( newanim );
			theHistoryManager.GroupEnd();

			// Post the change to the parent
			wxCommandEvent _evt( wxEVT_ANIM_LIST_CHANGE );
			GetParent()->ProcessEvent( _evt );

			// Refresh the list
			Refresh_List();

			// now the list should be updated => so select the newly created anim
			Select( FindString( animname ));
			wxCommandEvent event;
			OnSelectionChange(event);

			// Mark entity as changed
			entity->SetChanged();
		}
	}
}


//*************************************************************************

void wListAnims_Editable::EvtCopyAnim(wxCommandEvent& event )
{
	int indanim = GetSelection();
	if( indanim < 0 || indanim > arr_anims_count )
		return;

	// Get the elt under the mouse
	wxTextEntryDialog *_dialog = new wxTextEntryDialog(this, wxT("Anim copy"), wxT("Name for the copied anim "), wxT("UNNAMED"),
				wxOK | wxCANCEL | wxCENTRE );
	int response = _dialog->ShowModal();

	if( response  == wxID_OK )
	{
		wxString animname = _dialog->GetValue().Trim().Trim(false);
		if( animname == wxString())
			wxMessageBox( wxT("Anim name is empty"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		else
		{
			theHistoryManager.Set_State( false );
			ob_anim* newanim = (ob_anim*) arr_anims[indanim]->Clone();
			newanim->SetToken(0 ,animname );
			theHistoryManager.Set_State( true );
			
			theHistoryManager.GroupStart( wxT("Copy Anim ") + arr_anims[indanim]->GetToken(0) );
			entity->obj_container->Add_SubObj( newanim );
			theHistoryManager.GroupEnd();

			// Post the change to the parent
			wxCommandEvent _evt( wxEVT_ANIM_LIST_CHANGE );
			ProcessEvent( _evt );

			// Refresh the list
			Refresh_List();

			// now the list should be updated => so select the newly created anim
			Select( FindString( animname ));
			wxCommandEvent event;
			OnSelectionChange(event);

			// Mark entity as changed
			entity->SetChanged();
		}
	}
}


void wListAnims_Editable::EvtDeleteAnim(wxCommandEvent& event )
{
	int indanim = GetSelection();
	if( indanim < 0 || indanim >= arr_anims_count )
		return;

	// Make the guy to be sure
	int res = wxMessageBox( wxT("Are really sure that you want to delete the anim <") +
	arr_anims[indanim]->GetToken(0).Upper() + wxT("> ?"),
				wxT("Hey !"), wxYES_NO | wxICON_INFORMATION, this );

	if( res == wxYES )
	{
		theHistoryManager.GroupStart( wxT("Del Anim ") + arr_anims[indanim]->GetToken(0) );
		arr_anims[indanim]->Rm();
		theHistoryManager.GroupEnd();
		
		ent_g_flag |= GM_ANIM_DELETION;

		// Post the change to the parent
		wxCommandEvent _evt( wxEVT_ANIM_LIST_CHANGE );
		ProcessEvent( _evt );

		// Refresh the list
		Refresh_List();

		// now the list should be updated => so select the newly created anim
		if( indanim >= arr_anims_count )
			indanim = arr_anims_count -1;
		if( indanim == -1 )
			indanim = wxNOT_FOUND;

		Select( indanim );
		wxCommandEvent event;
		OnSelectionChange(event);

		ent_g_flag &= ~GM_ANIM_DELETION;
		// Mark entity as changed
		entity->SetChanged();
	}
}


//*************************************************************************

void wListAnims_Editable::EvtContextMenu( wxContextMenuEvent& event )
{
	int indanim = HitTest( ScreenToClient(wxGetMousePosition() ));
	if( indanim != GetSelection() && indanim >= 0 && indanim < arr_anims_count)
	{
		Select( indanim );
		wxCommandEvent event;
		OnSelectionChange(event);
	}

	PopupMenu( popMenu, ScreenToClient(wxGetMousePosition()) );
	event.Skip();
}

//*************************************************************************
void 
wListAnims_Editable::EvtCharPress(wxKeyEvent& event)
{
	int kc = event.GetKeyCode();

	if( wList_frames != NULL )
	{
		if( kc == WXK_DELETE)
		{
			wxCommandEvent new_event(wxEVT_COMMAND_MENU_SELECTED,CMD_DEL);
			wList_frames->ProcessEvent( event );
	//		AddPendingEvent(new_event);
			return;
		}
	}

	wListAnims::EvtCharPress(event);
}


//*************************************************************************
//*************************************************************************
//*************************************************************************
// Event table
BEGIN_EVENT_TABLE(wListAnims_Editable, wListAnims)
	EVT_MENU( CMD_NEW, wListAnims_Editable::EvtNewAnim)
	EVT_MENU( CMD_RENAME, wListAnims_Editable::EvtRenameAnim)
	EVT_MENU( CMD_COPY, wListAnims_Editable::EvtCopyAnim)
	EVT_MENU( CMD_DEL, wListAnims_Editable::EvtDeleteAnim)
	EVT_CONTEXT_MENU(wListAnims_Editable::EvtContextMenu)
	EVT_CHAR(wListAnims_Editable::EvtCharPress)
END_EVENT_TABLE()
