#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/imaglist.h>

#include "ob_editor.h"
#include "entity__enums.h"
#include "entity__globals.h"

Panel_Errors *panel_Errors;

#define ERR_COL_ICONS_WIDTH 20
#define ERR_COL_FILE_WIDTH  250

enum{
		ID_CLEAR = 303,
};

Panel_Errors::Panel_Errors( wxNotebook *onglets, const wxString& _intitul_panel )
:wxPanel( onglets, wxID_ANY)
{
	intitule_panel = _intitul_panel;
	nb_errors = 0;
	wxBoxSizer *sizer_main = new wxBoxSizer( wxVERTICAL );

	lst_errors = new wxListView( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
				wxLC_REPORT | wxLC_ALIGN_LEFT | wxLC_SINGLE_SEL | wxLC_HRULES );
	Init_LstError();
	sizer_main->Add( lst_errors, 1, wxALL | wxEXPAND );

	wxBoxSizer *sizer_down = new wxBoxSizer( wxHORIZONTAL );
	sizer_main->Add(sizer_down, 0, wxALL | wxEXPAND );

	sizer_down->AddStretchSpacer();
	wxButton *btn_clear = new wxButton( this, ID_CLEAR, wxT("Clear") );
	sizer_down->Add( btn_clear, 0, wxALL | wxALIGN_RIGHT, 8);

  	SetSizer( sizer_main );

}

void Panel_Errors::Init_LstError()
{
	lst_errors->InsertColumn(0, wxT(""), wxLIST_FORMAT_CENTER, ERR_COL_ICONS_WIDTH);
	lst_errors->InsertColumn(1, wxT("Source"),wxLIST_FORMAT_CENTER , ERR_COL_FILE_WIDTH);
	lst_errors->InsertColumn(2, wxT("Message"), wxLIST_FORMAT_LEFT , 60);

    wxImageList *m_imageList = new wxImageList(16, 16, true);
    m_imageList->Add( wxIcon(GetRessourceFile_String(wxT("error_small.png"))) );
    m_imageList->Add( wxIcon(GetRessourceFile_String(wxT("warning_small.png"))) );
//    int img_ind = m_imageList->Add( wxIcon(GetRessourceFile_String("info_small.png")) );
//	wxMessageBox( "Dummy :" + IntToStr(IMG_ERROR), "Debug", wxOK | wxICON_INFORMATION );

	lst_errors->AssignImageList( m_imageList, wxIMAGE_LIST_SMALL );
};


Panel_Errors::~Panel_Errors()
{

}

void Panel_Errors::EvtActivate(wxActivateEvent&)
{
	lst_errors->SetFocus();
}

void Panel_Errors::EvtGetFocus(wxFocusEvent&)
{
	lst_errors->SetFocus();
	if( lst_errors->GetItemCount() > 0 )
	{
		lst_errors->SetItemState(0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED ,
						wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED );
	}
}


void Panel_Errors::EvtSize( wxSizeEvent& event )
{
	int _w,_h;
	lst_errors->GetSize(&_w,&_h);
	lst_errors->SetColumnWidth( 2, _w - ERR_COL_FILE_WIDTH - ERR_COL_ICONS_WIDTH);
	event.Skip();
}

void Panel_Errors::EvtClear( wxCommandEvent& event)
{
	lst_errors->DeleteAllItems();
	nb_errors=0;
	Update_NbErrors();
}

void Panel_Errors::Log( int id_img, const wxString& src, const wxString& message )
{
	wxListItem temp;
	temp.SetImage( id_img );
	int row_num = lst_errors->InsertItem( temp );

	lst_errors->SetItem(row_num, 1, src );
	lst_errors->SetItem(row_num, 2, message );
	nb_errors++;
	Update_NbErrors();
}

void Panel_Errors::Update_NbErrors()
{
	wxString new_intitule = intitule_panel;
	if( nb_errors != 0 )
		new_intitule += wxT(" (") + IntToStr(nb_errors) + wxT(")");

	entityFrame->SetPanelErrorsIntitule( new_intitule );
}

void Panel_Errors::Item_DblClick( wxListEvent& event )
{
	int row = event.GetIndex();
	wxListItem temp;
	temp.SetColumn( 2 );
	temp.SetId( row );
	lst_errors->GetItem( temp );
	wxMessageBox( wxT("<") + temp.GetText() + wxT(">")
	,wxT("Info"), wxOK | wxICON_INFORMATION, this );
}

void Panel_Errors::EvtKeyDown( wxListEvent& event )
{
	if( event.GetKeyCode() == WXK_DELETE )
	{
		int row = event.GetIndex();
		lst_errors->DeleteItem(row);
		nb_errors--;
		Update_NbErrors();
		lst_errors->SetItemState(row, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED ,
						wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED );
		return;
	}
	event.Skip();
}

// Event table
BEGIN_EVENT_TABLE(Panel_Errors, wxPanel)
	EVT_BUTTON  (ID_CLEAR,   Panel_Errors::EvtClear)
	EVT_SIZE(Panel_Errors::EvtSize)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, Panel_Errors::Item_DblClick)
	EVT_LIST_KEY_DOWN(wxID_ANY, Panel_Errors::EvtKeyDown)
	EVT_ACTIVATE(Panel_Errors::EvtActivate)
	EVT_SET_FOCUS(Panel_Errors::EvtGetFocus)
END_EVENT_TABLE()
