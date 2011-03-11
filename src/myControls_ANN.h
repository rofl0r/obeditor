#ifndef MYCONTROLS_ANN_H
#define MYCONTROLS_ANN_H

#include <vector>

#include <wx/combo.h>
#include <wx/listctrl.h>
#include <wx/popupwin.h>

//--------------------------------------------------------------------
class pComboCtrl_AutoComplete;

//********************************************************************
class pListViewPopup :  public wxListView
{
public:
	pListViewPopup( wxWindow* parent, bool b_insensitive, int more_style );
	~pListViewPopup();
	
	virtual void DoSetSize(	int x, int y,
					int width, int height,
					int sizeFlags = wxSIZE_AUTO );
	
	// Get list selection as a string
	virtual wxString	GetStringValue() const;
		  void*	Get_Data( int ind );
		  int 	Get_Index( const wxString& s, void* data = NULL );
		  int		Get_Index_Data( void* data );
	
	void  Clear(bool b_update_size);
	void	Add( const wxString& s, void* _data, bool b_update_size );

	virtual int  GetCount()const;
		  int  FindItem( int i, const wxString& s );

	virtual wxString GetString(int ind);
	virtual void SetStringValue(const wxString& s);
		  void Set_Selection( int ind );
	
	void	 Delete_Choice( int ind, bool b_update_size );
	void   Delete_Choice( void* _data, bool b_update_size );

	int Cmp(const wxString& s1, const wxString& s2 );

	void MoveSelection( int count );
	void pValidSelection();

	//-----------------------------------------------------------------------
	int  curr_selection; // current item index
	bool b_insensitive;
	
	wxControl* theComboBoxAutoComplete;
	pComboCtrl_AutoComplete* theCbCtrl;
	
	//-----------------------------------------------------------------------
	void UpdateSize(bool b_update_pos = true);
	void ShowPopup();
	void HidePopup();
	
public:
	int x,y;
	void ComputePosition( int w, int h, int&x, int& y );

	//-----------------------------------------------------------------------
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseClick(wxMouseEvent& WXUNUSED(event));
	void Evt_FocusIn( wxFocusEvent& evt );

	DECLARE_EVENT_TABLE()
};

//********************************************************************
class pPopupWindow_Combo : public wxPopupWindow
{
public:
	pPopupWindow_Combo(wxWindow* parent,bool _b_insensitive, int more_style = 0 );

	pListViewPopup* theListView;
};


//********************************************************************
//********************************************************************
enum
{
	MB_POPUP_SHOWN	= 0x00001,
	MB_SETTINGPOPUP	= 0x00002,
	MB_CHANGINGTEXT	= 0x00004,
	MB_SETTINGVAL	= 0x00008,
};

//--------------------------------------------------------------------
class pComboCtrl_AutoComplete : public wxTextCtrl
{
public:
	//-----------------------------------------------------------------------
	pComboCtrl_AutoComplete(wxWindow* _parent, bool _b_insensitive = true, int more_style = 0 );
	~pComboCtrl_AutoComplete();

	virtual bool IsShownOnScreen()const{return wxTextCtrl::IsShownOnScreen();};
	
	bool Set_Value( const wxString& that );
	
	//-----------------------------------------------------------------------
	void  Add(const wxString& _choice, void* data = NULL, bool b_update_popup_size = true );
	void  Set_Choices(const wxArrayString& _choices, void** _datas = NULL );
	void* Remove(const wxString& _choice );
	
	bool Selection_IsAMatch();
	
	wxString  Get_Value()const;
	void*     GetSelection_Data();
	
	const std::vector<void*>& Get_All_Datas();

public:
	pListViewPopup* popup;
	int  m_flags;
	int  m_state;
	
	void Set_BackgroundState( bool b_is_a_match );
	void pSetPopup( bool b_show );

	long txt_inspt,txt_from,txt_to;
	void pRestoreTextCtrlState();
	void pSaveTextCtrlState();
	
	
	// -1 : s1 < s2 ...  0 : s1 == s2 : else 1
	int  Cmp(const wxString& s1, const wxString& s2 );
	bool IsFilteredOut(const wxString&);
	void Update_choices(bool b_update_popup_size = true );
	void Update_States();
	
	void Process_Event_Change(wxEvent& orig_evt );
	void Process_Event_Valid(wxEvent& orig_evt );
	
	bool b_insensitive;
	std::vector<wxString> choices;
	std::vector<void*>    datas;

	void Evt_TextChange( wxCommandEvent& evt );
	void Evt_Size(wxSizeEvent& evt);
	void Evt_Move( wxMoveEvent& evt );
	void Evt_KeyUp( wxKeyEvent& evt );
	void Evt_KeyDown( wxKeyEvent& evt );
	void Evt_Activate( wxActivateEvent& evt );
	void Evt_Enter( wxCommandEvent& evt );
	void Evt_FocusOut( wxFocusEvent& evt );
	
	DECLARE_EVENT_TABLE()
};


//--------------------------------------------------------------------

#endif // MYCONTROLS_ANN_H
