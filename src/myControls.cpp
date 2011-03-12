#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__tools.h"
#include "myControls_ANN.h"
#include "myControls.h"

using namespace std;


//-----------------------------------------------------------------------------
DEFINE_EVENT_TYPE(wxEVT_COMBOBOXAUTOCOMPLETE_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_COMBOBOXAUTOCOMPLETE_VALID )
//-----------------------------------------------------------------------------
enum
{
	CAC_INITIAL,
	CAC_AUTOCOMPLETE,
	CAC_STANDALONE,
	CAC_MATCHING,
};

ComboBoxAutoComplete::ComboBoxAutoComplete( wxWindow* _parent, wxWindowID _id, bool _b_insensitive, const wxString& value, const wxPoint& pos, const wxSize& size, int more_style )
:wxControl( _parent, _id, pos, size, more_style )
{
	theCombo = new pComboCtrl_AutoComplete( this, _b_insensitive, more_style );
	wxWindow* topLW = wxGetTopLevelParent(this);
	if( topLW != NULL )
	{
		topLW->Connect( wxEVT_MOVE, wxMoveEventHandler(pComboCtrl_AutoComplete::Evt_Move)
				,NULL, theCombo);
				
		topLW->Connect(wxEVT_ACTIVATE,
				 wxActivateEventHandler(pComboCtrl_AutoComplete::Evt_Activate)
				,NULL, theCombo);
	}

	pPopupWindow_Combo * t = new pPopupWindow_Combo( wxGetTopLevelParent(this), _b_insensitive, more_style );
	thePopup = t->theListView;

	thePopup->theComboBoxAutoComplete = this;
	thePopup->theCbCtrl = theCombo;
	thePopup->b_insensitive = _b_insensitive;
	
	theCombo->popup = thePopup;
	theCombo->Set_Value( value );
}

//-----------------------------------------------------------------------------
ComboBoxAutoComplete::~ComboBoxAutoComplete()
{
}
	
//-----------------------------------------------------------------------------
bool 
ComboBoxAutoComplete::IsShownOnScreen()const
{
//	return theCombo->IsShownOnScreen();
	return wxControl::IsShownOnScreen();
}

//-----------------------------------------------------------------------------
wxSize
ComboBoxAutoComplete::DoGetBestSize()const
{
	return theCombo->GetBestSize();
}

//-----------------------------------------------------------------------------
bool 
ComboBoxAutoComplete::Set_Value( const wxString& that )
{
	return theCombo->Set_Value( that );
}

//-----------------------------------------------------------------------------
void  
ComboBoxAutoComplete::Add(const wxString& _choice, void* data )
{
	theCombo->Add( _choice, data );
}

//-----------------------------------------------------------------------------
void  
ComboBoxAutoComplete::Set_Choices(const wxArrayString& _choices, void** _datas )
{
	theCombo->Set_Choices( _choices, _datas );
}

//-----------------------------------------------------------------------------
void* 
ComboBoxAutoComplete::Remove(const wxString& _choice )
{
	return theCombo->Remove( _choice );
}
	
//-----------------------------------------------------------------------------
bool  
ComboBoxAutoComplete::Selection_IsAMatch()
{
	return theCombo->Selection_IsAMatch();
}

//-----------------------------------------------------------------------------
wxString  
ComboBoxAutoComplete::Get_Value()const
{
	return theCombo->Get_Value();
}

//-----------------------------------------------------------------------------
void* 
ComboBoxAutoComplete::GetSelection_Data()
{
	return theCombo->GetSelection_Data();
}

//-----------------------------------------------------------------------------
const std::vector<void*>& 
ComboBoxAutoComplete::Get_All_Datas()
{
	return theCombo->Get_All_Datas();
}

//-----------------------------------------------------------------------------
void 
ComboBoxAutoComplete::DoSetSize(int x, int y,
				  int width, int height,
				  int sizeFlags )
{
	theCombo->SetSize( 0,0,width,height,sizeFlags );
	wxControl::DoSetSize( x, y, width, height, sizeFlags );
}

//-----------------------------------------------------------------------------
void 
ComboBoxAutoComplete::Evt_Size(wxSizeEvent& evt)
{
	theCombo->SetSize( evt.GetSize() );
	evt.Skip();
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(ComboBoxAutoComplete, wxControl)
    EVT_SIZE(ComboBoxAutoComplete::Evt_Size)
END_EVENT_TABLE()
	
//********************************************************************
//********************************************************************
//********************************************************************
//--------------------------------------------------------------------

pComboCtrl_AutoComplete::pComboCtrl_AutoComplete( wxWindow* _parent, bool _b_insensitive, int more_style )
:wxTextCtrl( _parent, wxID_ANY, wxString(),wxDefaultPosition,wxDefaultSize, 
		 wxTE_PROCESS_ENTER| more_style )
{
	b_insensitive = _b_insensitive;
	popup = NULL;
	m_flags = 0;
	m_state = CAC_INITIAL;
}

//------------------------------------------------------------------
pComboCtrl_AutoComplete::~pComboCtrl_AutoComplete()
{
}

//------------------------------------------------------------------
void*
pComboCtrl_AutoComplete::Remove(const wxString& _choice )
{
	void* res = NULL;
	void* choice_rm = NULL;
	
	int ind_prec = popup->curr_selection;
	// Search for the string...
	vector<wxString>::iterator it(choices.begin()),
					it_end(choices.end());

	vector<void*>::iterator it_data(datas.begin());
					
	for( ;it != it_end; it++ )
	{
		// Is it the one ?
		if( 	Cmp((*it),_choice) == 0 )
		{
			choices.erase(it);
			res = (*it_data);
			choice_rm = res;
			datas.erase( it_data );
			break;
		}
		it_data++;
	}

	// If have found it
	if( choice_rm != NULL )
		// Delete it also in the popup list
		popup->Delete_Choice( choice_rm, true );

	Update_States();
	if( ind_prec != popup->curr_selection )
	{
		wxCommandEvent dummy;
		Process_Event_Change(dummy );
	}
	return res;
}

//------------------------------------------------------------------
bool 
pComboCtrl_AutoComplete::Set_Value( const wxString& that )
{
	wxTextCtrl::SetValue( that );
	return Selection_IsAMatch();
}

//------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Add(const wxString& _choice, void* _data, bool b_update_popup_size )
{
	if( _choice == wxString() )
		return;
	
	// Don't care if it's already here
	if( popup->Get_Index( _choice, _data ) != wxNOT_FOUND )
		return;

	int ind_prec = popup->curr_selection;
	
	choices.push_back( _choice );
	datas.push_back( _data  );
	
	Update_choices(b_update_popup_size);
	Update_States();
	
	if( ind_prec != popup->curr_selection )
	{
		wxCommandEvent dummy;
		Process_Event_Change(dummy );
	}
}

//------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Set_Choices(const wxArrayString& _choices, void** _datas )
{
	int ind_prec = popup->curr_selection;
	
	choices.clear();
	datas.clear();
	
	int  tay = _choices.GetCount();
	for( int i = 0; i < tay; i++ )
	{
		void* _data = NULL;
		if( _datas != NULL )
			_data = _datas[i];
		Add( _choices[i], _data, false );
	}
	
	Update_choices();
	Update_States();
	
	if( ind_prec != popup->curr_selection )
	{
		wxCommandEvent dummy;
		Process_Event_Change(dummy );
	}
}

//------------------------------------------------------------------
wxString
pComboCtrl_AutoComplete::Get_Value()const
{
	return wxTextCtrl::GetValue();
}

//------------------------------------------------------------------
void*
pComboCtrl_AutoComplete::GetSelection_Data()
{
	if( Selection_IsAMatch() == false )
		return NULL;
	
	return popup->Get_Data( popup->curr_selection );
}

//------------------------------------------------------------------
const std::vector<void*>& 
pComboCtrl_AutoComplete::Get_All_Datas()
{
	return datas;
}

//------------------------------------------------------------------
int
pComboCtrl_AutoComplete::Cmp(const wxString& s1, const wxString& s2 )
{
	if( b_insensitive )
		return s1.CmpNoCase( s2 );
	else
		return s1.Cmp( s2 );
}

//------------------------------------------------------------------
bool 
pComboCtrl_AutoComplete::Selection_IsAMatch()
{
	return (popup->curr_selection != wxNOT_FOUND);
}

//------------------------------------------------------------------
bool 
pComboCtrl_AutoComplete::IsFilteredOut(const wxString& s )
{
	wxString _curr = Get_Value();
	if( _curr == wxString() )
		return false;
	
	if( s.Len() < _curr.Len() )
		return true;
	
	wxString s2 = s;
	if( b_insensitive == true )
	{
		_curr = _curr.Upper();
		s2 = s2.Upper();
	}
	
	return ( s2.Left( _curr.Len() ) != _curr );
}


//------------------------------------------------------------------
void
pComboCtrl_AutoComplete::pSaveTextCtrlState()
{
	txt_inspt = GetInsertionPoint();
	GetSelection(&txt_from,&txt_to);
}

//------------------------------------------------------------------
void
pComboCtrl_AutoComplete::pRestoreTextCtrlState()
{
	SetSelection( txt_from, txt_to );
	SetInsertionPoint(txt_inspt);
}

//------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Update_choices(bool b_update_popup_size )
{
	// Some optimisation in the emtpy case
	if( Get_Value() == wxString() )
	{
		// Clear choice list
		popup->Clear(false);
		popup->InsertColumn(0,wxString());
		popup->curr_selection = wxNOT_FOUND;
		
		if( b_update_popup_size )
			popup->UpdateSize();
		return;
	}
	
	// Remove first
	for( int i = popup->GetCount()-1; i >=0 ;i--)
	{
		if( IsFilteredOut( popup->GetString( i ) ) == true )
			popup->Delete_Choice( i, false );
	}
	
	// Add next
	vector<wxString>::iterator it( choices.begin())
				   , it_end( choices.end());
	vector<void*>::iterator it_data( datas.begin());

	for(; it != it_end ; it++ )
	{
		if( IsFilteredOut( *it ) == false )
			popup->Add(  *it,  *it_data, false );
		it_data++;
	}

	// Try to Select the first matching one
	int ind = popup->Get_Index( Get_Value() );
	if( ind == wxNOT_FOUND )
		popup->curr_selection = wxNOT_FOUND;
	else
	{
		m_flags |= MB_SETTINGVAL;
		pSaveTextCtrlState();
		popup->Set_Selection( ind );
		wxTextCtrl::ChangeValue(popup->GetStringValue());
		pRestoreTextCtrlState();
		m_flags &= ~MB_SETTINGVAL;
	}

	if( b_update_popup_size )
		popup->UpdateSize();
}


//------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Process_Event_Change(wxEvent& orig_evt )
{
	wxCommandEvent evt( wxEVT_COMBOBOXAUTOCOMPLETE_CHANGE, this->GetId() );
	evt.SetEventObject( GetParent() );
	evt.SetTimestamp( orig_evt.GetTimestamp() );
	ProcessEvent( evt );
}

//------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Process_Event_Valid(wxEvent& orig_evt )
{
	wxCommandEvent evt( wxEVT_COMBOBOXAUTOCOMPLETE_VALID, this->GetId() );
	evt.SetEventObject( GetParent() );
	evt.SetTimestamp( orig_evt.GetTimestamp() );
	ProcessEvent( evt );
}

//------------------------------------------------------------------
void
pComboCtrl_AutoComplete::Set_BackgroundState( bool b_is_a_match )
{
	if( b_is_a_match == true )
		SetBackgroundColour( *wxWHITE );
	else
		SetBackgroundColour( wxColour( 255,200,200));
}

//------------------------------------------------------------------
void
pComboCtrl_AutoComplete::pSetPopup( bool b_show )
{
	if( m_flags & MB_SETTINGPOPUP )
		return;
	m_flags |= MB_SETTINGPOPUP;

	if( b_show == true )
	{
		if( (m_flags & MB_POPUP_SHOWN) == 0  && popup->GetCount() > 0 )
		{
			popup->ShowPopup();
			m_flags |= MB_POPUP_SHOWN;
		}
	}
	else if( (m_flags & MB_POPUP_SHOWN) > 0 )
	{
		popup->HidePopup();
		m_flags &= ~MB_POPUP_SHOWN;
	}
	
	m_flags &= ~MB_SETTINGPOPUP;
}

//------------------------------------------------------------------
void
pComboCtrl_AutoComplete::Update_States()
{
	// Back to initial state
	if( Get_Value() == wxString() )
	{
		Set_BackgroundState(false);
		pSetPopup( false );
		m_state = CAC_INITIAL;
	}
	
	// to CAC_MATCHING state
	else if( Selection_IsAMatch() == true 
		&& (m_state == CAC_AUTOCOMPLETE || m_state == CAC_INITIAL )
		)
	{
		Set_BackgroundState(true);
		pSetPopup( false );
		m_state = CAC_MATCHING;
	}
	
	// to CAC_AUTOCOMPLETE state
	else if(    Selection_IsAMatch() == false
		   && m_state != CAC_STANDALONE
		 )
	{
		Set_BackgroundState(false);
		pSetPopup( true );
		m_state = CAC_AUTOCOMPLETE;
	}
	
	else if( m_state == CAC_STANDALONE )
	{
		Set_BackgroundState(Selection_IsAMatch());
		pSetPopup( false );
	}
	else if( m_state == CAC_MATCHING )
		Set_BackgroundState(true);
		
}

//------------------------------------------------------------------
void
pComboCtrl_AutoComplete::Evt_TextChange( wxCommandEvent& evt )
{
	if( m_flags & MB_CHANGINGTEXT )
		return;
	m_flags |= MB_CHANGINGTEXT;
	
	Update_choices();
	Update_States();
	
	Process_Event_Change( evt );
	m_flags &= ~MB_CHANGINGTEXT;
	evt.Skip();
}

//--------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Evt_Size(wxSizeEvent& evt)
{
	SetSize( evt.GetSize() );
	evt.Skip();
	popup->UpdateSize();
}

//--------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Evt_Move( wxMoveEvent& evt )
{
	evt.Skip();
	popup->UpdateSize();
}

//--------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Evt_KeyUp( wxKeyEvent& evt )
{
	int kc = evt.GetKeyCode();
	if( m_flags & MB_POPUP_SHOWN )
	{
		switch( kc )
		{
			case WXK_UP:
				popup->MoveSelection( -1 );
				return;
			case WXK_DOWN:
				popup->MoveSelection(  1 );
				return;
			case WXK_PAGEUP:
				popup->MoveSelection( -5 );
				return;
			case WXK_PAGEDOWN:
				popup->MoveSelection( 5 );
				return;
			case WXK_HOME:
				popup->MoveSelection( -3000 );
				return;
			case WXK_END:
				popup->MoveSelection( 3000 );
				return;
				
			case WXK_ESCAPE:
				popup->HidePopup();
				return;
		}
	}
	else if( (kc == WXK_UP || kc == WXK_DOWN )
		&& popup->GetCount() > 0
		&& (m_flags & MB_POPUP_SHOWN) == 0
		)
	{
		popup->ShowPopup();
		return;
	}
	
	evt.Skip();
}

//--------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Evt_KeyDown( wxKeyEvent& evt )
{
	int kc = evt.GetKeyCode();
	if( m_flags & MB_POPUP_SHOWN )
	{
		switch( kc )
		{
			case WXK_UP:
			case WXK_DOWN:
			case WXK_PAGEUP:
			case WXK_PAGEDOWN:
			case WXK_HOME:
			case WXK_END:
			case WXK_ESCAPE:
				return;
		}
	}
	else if( kc == WXK_UP || kc == WXK_DOWN )
		return;
	
	evt.Skip();
}

//--------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Evt_Enter( wxCommandEvent& evt )
{
	evt.Skip();
	if( m_flags & MB_POPUP_SHOWN && popup->GetFirstSelected() >=0 )
		popup->pValidSelection();
	else
	{
		pSetPopup( false );
		Process_Event_Valid( evt );
	}
}


//--------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Evt_Activate( wxActivateEvent& evt )
{
	if( evt.GetActive() == false )
	{
		if( (m_flags & MB_POPUP_SHOWN) > 0 && popup != NULL )
		{
			popup->HidePopup();
		}
	}
	else if( (m_flags & MB_POPUP_SHOWN) > 0 && popup != NULL )
	{
			popup->ShowPopup();
	}
	evt.Skip();
}

//--------------------------------------------------------------------
void 
pComboCtrl_AutoComplete::Evt_FocusOut( wxFocusEvent& evt )
{
	if( popup == NULL )
		return;
	
	if( 	   evt.GetWindow() != popup 
		&& evt.GetWindow() != popup->GetParent()
		)
	{
		pSetPopup( false );
		Process_Event_Valid( evt );
	}
}

//------------------------------------------------------------------------
BEGIN_EVENT_TABLE(pComboCtrl_AutoComplete, wxTextCtrl)
	EVT_TEXT( wxID_ANY, pComboCtrl_AutoComplete::Evt_TextChange )
	EVT_SIZE(pComboCtrl_AutoComplete::Evt_Size)
	EVT_MOVE(pComboCtrl_AutoComplete::Evt_Move)
	EVT_KEY_DOWN(pComboCtrl_AutoComplete::Evt_KeyDown)
	EVT_KEY_UP(pComboCtrl_AutoComplete::Evt_KeyUp)
	EVT_KILL_FOCUS(pComboCtrl_AutoComplete::Evt_FocusOut)
	EVT_COMMAND(wxID_ANY,wxEVT_COMMAND_TEXT_ENTER,pComboCtrl_AutoComplete::Evt_Enter)
END_EVENT_TABLE()

//********************************************************************
//********************************************************************
//********************************************************************
//--------------------------------------------------------------------
pPopupWindow_Combo::pPopupWindow_Combo(wxWindow* parent,bool _b_insensitive, int more_style )
:wxPopupWindow(parent,wxBORDER_DOUBLE|wxFRAME_NO_TASKBAR |wxFRAME_FLOAT_ON_PARENT)
{
	theListView = new pListViewPopup(this, _b_insensitive, more_style );
	Connect( wxEVT_SET_FOCUS, wxFocusEventHandler(pListViewPopup::Evt_FocusIn)
				,NULL,theListView);

/*	Connect(wxEVT_ACTIVATE,
				wxActivateEventHandler(pListViewPopup::Evt_Activate)
			,NULL, theListView);
	Connect( wxEVT_MOVE, wxMoveEventHandler(pListViewPopup::Evt_Move)
				,NULL, theListView);*/
}

//********************************************************************
//********************************************************************
//********************************************************************
//--------------------------------------------------------------------
pListViewPopup::pListViewPopup( wxWindow* parent, bool b_insensitive, int more_style )
:wxListView(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize
			, wxLC_SINGLE_SEL|wxLC_REPORT|wxLC_HRULES|wxLC_NO_HEADER
			  |wxHSCROLL|wxVSCROLL | wxBORDER_SUNKEN|more_style)
{
	InsertColumn(0,wxString());
	curr_selection = wxNOT_FOUND;
	b_insensitive = b_insensitive;
	x = y = wxDefaultCoord;
};

//--------------------------------------------------------------------
pListViewPopup::~pListViewPopup()
{
	theCbCtrl->popup = NULL;
}

//--------------------------------------------------------------------
void	
pListViewPopup::Clear(bool b_update_size)
{
	wxListView::ClearAll();
	if( b_update_size )
		UpdateSize();
}

//------------------------------------------------------------------
void
pListViewPopup::Delete_Choice( int ind, bool b_update_size )
{
	if( ind >= 0 && ind < GetCount() )
	{
		wxListView::DeleteItem( ind );
		if( ind == curr_selection )
			curr_selection = wxNOT_FOUND;
		else if( ind < curr_selection )
			curr_selection--;
	}
	if( b_update_size )
		UpdateSize();
}

//------------------------------------------------------------------
void
pListViewPopup::Delete_Choice( void* _data, bool b_update_size )
{
	for( int i = 0; i < GetCount(); i++ )
	{
		if( Get_Data( i ) == _data )
		{
			wxListView::DeleteItem( i );
			if( i == curr_selection )
				curr_selection = wxNOT_FOUND;
			else if( i < curr_selection )
				curr_selection--;
			break;
		}
	}
	if( b_update_size )
		UpdateSize();
}

//--------------------------------------------------------------------
void	
pListViewPopup::Add( const wxString& s, void* _data, bool b_update_size )
{
	  if( Get_Index( s, _data ) != wxNOT_FOUND )
		  return;
	  
	  int c = GetCount();
	  for( int i = 0 ; i < c; i++ )
		  if( Cmp(GetString( i ), s ) > 0 )
		  {
			  wxListView::InsertItem( i, s );
			  wxListView::SetItemData( i, (long) _data );
			  if( curr_selection != wxNOT_FOUND && i <= curr_selection )
				  curr_selection++;
			  if( b_update_size )
				UpdateSize();
			  return;
		  }

	// Not insert  =>  Append 
	wxListView::InsertItem( c, s );
	wxListView::SetItemData( c, (long) _data );
	if( b_update_size )
		UpdateSize();
}

//--------------------------------------------------------------------
// Translate string into a list selection
void 
pListViewPopup::SetStringValue(const wxString& s)
{
	// Nothing to change if the current selection == s
	if( curr_selection != wxNOT_FOUND )
		if( Cmp( GetString( curr_selection ), s ) == 0 )
			return;

	// Select the first avalaible
	Set_Selection( FindItem(-1,s) );
}

//--------------------------------------------------------------------
void
pListViewPopup::Set_Selection( int ind )
{
	if ( ind >= 0 && ind < GetCount() )
		curr_selection = ind;
	else
		curr_selection = wxNOT_FOUND;
	
	wxListView::Select(curr_selection);
}

//--------------------------------------------------------------------
void
pListViewPopup::pValidSelection()
{
	int ind = wxListView::GetFirstSelected();
	if( ind < 0 || ind >= GetCount())
		return;
	curr_selection = ind;
	theCbCtrl->ChangeValue(GetStringValue());
	theCbCtrl->SetInsertionPointEnd();
	if( theCbCtrl->m_state != CAC_STANDALONE)
		theCbCtrl->m_state = CAC_MATCHING;
	theCbCtrl->Set_BackgroundState( true );
	wxCommandEvent dummy;
	theCbCtrl->Process_Event_Change(dummy );

	HidePopup();
	
//	wxCommandEvent evt( wxEVT_COMMAND_TEXT_UPDATED );
//	theCbCtrl->AddPendingEvent( evt );
//	theCbCtrl->SetFocus();
}

//--------------------------------------------------------------------
int
pListViewPopup::GetCount()const
{
	return wxListView::GetItemCount();
}

//--------------------------------------------------------------------
wxString
pListViewPopup::GetString(int ind)
{
	if ( ind >= 0 && ind < GetCount() )
		return wxListView::GetItemText( ind );
	return wxEmptyString;
}

//--------------------------------------------------------------------
// Get list selection as a string
wxString 
pListViewPopup::GetStringValue() const
{
	if ( curr_selection != wxNOT_FOUND )
		return wxListView::GetItemText(curr_selection);
	return wxEmptyString;
}

//------------------------------------------------------------------
void*
pListViewPopup::Get_Data( int ind )
{
	if( ind < 0 || ind >= GetCount() )
		return NULL;
	return (void*) GetItemData( ind );
}

//------------------------------------------------------------------
int
pListViewPopup::FindItem( int i, const wxString& s )
{
	if( i < 0 )
		i = 0;
	int c = GetCount();
	for( ; i < c; i++ )
	{
		wxString s2 = GetItemText(i);
		if( Cmp(s,s2) == 0 )
			return i;
	}
	return wxNOT_FOUND;
}

//------------------------------------------------------------------
int 	 
pListViewPopup::Get_Index( const wxString& s, void* data )
{
	int start = -1;
	while( 1 )
	{
		start = FindItem(start,s);
		if( start < 0 )
			return wxNOT_FOUND;
		wxString itstr = wxListView::GetItemText( start );
		if( Cmp(itstr,s) == 0 )
		{
			if( data == NULL )
				return start;
			if( data == Get_Data(start) )
				return start;
		}
		start++;
	}
	
	return wxNOT_FOUND;
}

//------------------------------------------------------------------
int
pListViewPopup::Get_Index_Data( void* data )
{
	if( data == NULL )
		return wxNOT_FOUND;
	
	int res = wxListCtrl::FindItem( -1, (long) data );
	if( res < 0 )
		return wxNOT_FOUND;
	return res;
}

//------------------------------------------------------------------
void 
pListViewPopup::MoveSelection( int count )
{
	int ind_curr = wxListView::GetFirstSelected();
	if( ind_curr < 0 )
	{
		if( count > 0 )
			ind_curr = 0;
		else
			ind_curr = GetCount() - 1;
	}
	else
	{
		ind_curr += count;
		if( ind_curr < 0 )
			ind_curr = 0;
		else if( ind_curr >= GetCount() )
			ind_curr = GetCount() -1;
	}
	
	wxListView::Select(ind_curr);
	
	// Move the view to show the current selected
	wxListView::Focus(ind_curr);
}


//------------------------------------------------------------------
int
pListViewPopup::Cmp(const wxString& s1, const wxString& s2 )
{
	if( b_insensitive )
		return s1.CmpNoCase( s2 );
	else
		return s1.Cmp( s2 );
}

//------------------------------------------------------------------
void 
pListViewPopup::DoSetSize(int x, int y,
				  int width, int height,
				  int sizeFlags )
{
	if( 	   HasScrollbar(wxVERTICAL) )
		width+= wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
#ifdef __WXMSW__
	width+= 5;
//	height+= 5;
#endif
	GetParent()->SetSize(x,y,width,height);
	wxListView::DoSetSize(wxDefaultCoord,wxDefaultCoord
				   ,width,height
				   ,sizeFlags );
}

//------------------------------------------------------------------
void
pListViewPopup::ComputePosition( int w, int h, int&x, int& y )
{
	// Compute the coords of the popup
	wxPoint pt( theCbCtrl->GetScreenPosition() );
	wxSize  ps( theCbCtrl->GetSize());
	wxSize  s( w,h );
	wxRect display_r = wxGetClientDisplayRect();
	
	// compute x coord
	if( pt.x + ps.GetWidth() > display_r.x + display_r.width  )
		x = display_r.x + display_r.width - s.GetWidth();
	else
		x = pt.x;
	
	// compute y coord
	if( pt.y + ps.GetHeight() + s.GetHeight() < display_r.y + display_r.height )
		y = pt.y + ps.GetHeight();
	else
		y = pt.y - s.GetHeight();
	
}

//------------------------------------------------------------------
void
pListViewPopup::UpdateSize(bool b_update_pos )
{
	wxSize spaces = GetItemSpacing();
	int w,h;
	if( GetCount() > 0 )
	{
		int c = GetCount();
		h = 10;
		
		// compute some infos about the size of the content of the listView
		int max_w = 40;
		wxRect r;
		for( int i = 0; i < c ;i++)
		{
			GetItemRect( 0, r, wxLIST_RECT_BOUNDS );
			h += r.height;
			max_w = max(max_w,r.width);
		}
		
		// Compute the Height of the popup
		wxWindow* topLW = wxGetTopLevelParent(theCbCtrl);
		if( topLW != NULL )
		{
			int min_h = min(200,h);
			h = min(h, topLW->GetSize().GetHeight() * 4 / 5 );
			h = max( h, min_h );
		}

		// Compute the Width of the popup
		w = min( theCbCtrl->GetSize().GetWidth(), max_w );
	}
	else
	{
		w = GetSize().GetWidth();
		h = 10;
	}

	if( b_update_pos == false )
	{
		SetSize( wxSize(w, h) );
	}
	
	// Also update position
	else
	{
		ComputePosition( w, h, x, y );
		SetSize( x,y,w,h );
	}
}

//------------------------------------------------------------------
void
pListViewPopup::ShowPopup()
{
	if( 	   theCbCtrl->m_state == CAC_STANDALONE 
		&& theCbCtrl->GetValue() != wxString()
		&& (theCbCtrl->m_flags & MB_SETTINGPOPUP) == 0
	  )
	{
		if( theCbCtrl->Selection_IsAMatch() == true )
			theCbCtrl->m_state = CAC_MATCHING;
		else
			theCbCtrl->m_state = CAC_AUTOCOMPLETE;
	}

	GetParent()->Show();
	theCbCtrl->m_flags |= MB_POPUP_SHOWN;
}

//--------------------------------------------------------------------
void
pListViewPopup::HidePopup()
{
	if( 	   theCbCtrl->m_state == CAC_AUTOCOMPLETE
		&& theCbCtrl->GetValue() != wxString()
		&& (theCbCtrl->m_flags & MB_SETTINGPOPUP) == 0
	  )
	{
		if( theCbCtrl->Selection_IsAMatch() == true )
			theCbCtrl->m_state = CAC_MATCHING;
		else
			theCbCtrl->m_state = CAC_STANDALONE;
	}
	
	GetParent()->Hide();
	theCbCtrl->m_flags &= ~MB_POPUP_SHOWN;
}

//--------------------------------------------------------------------
// Do mouse hot-tracking (which is typical in list popups)
void 
pListViewPopup::OnMouseMove(wxMouseEvent& event)
{
	// TODO: Move selection to cursor
}

//--------------------------------------------------------------------
// On mouse left up, set the value and close the popup
void 
pListViewPopup::OnMouseClick(wxMouseEvent& WXUNUSED(event))
{
	curr_selection = wxListView::GetFirstSelected();
	pValidSelection();
	HidePopup();
}

//--------------------------------------------------------------------
void
pListViewPopup::Evt_FocusIn( wxFocusEvent& evt )
{
	evt.Skip();
#ifdef __WXMSW__
	wxGetTopLevelParent(theCbCtrl)->SetFocus();
#endif
	theCbCtrl->SetFocus();
}

//--------------------------------------------------------------------
BEGIN_EVENT_TABLE(pListViewPopup, wxListView)
    EVT_MOTION(pListViewPopup::OnMouseMove)
    EVT_LEFT_UP(pListViewPopup::OnMouseClick)
    EVT_SET_FOCUS(pListViewPopup::Evt_FocusIn)
END_EVENT_TABLE()
//--------------------------------------------------------------------
//--------------------------------------------------------------------
