/*
 * levels__wnd_EditFrontPanels.cpp
 *
 *  Created on: 8 mai 2009
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__mod.h"
#include "common__ob_controls.h"

#include "levels__globals.h"
#include "levels__sidesWindows.h"
#include "levels__Ctrl_SceneView.h"
#include "levels__wndEditFrontPanels.h"


const wxFont font_Alphabet( 18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true );
const wxFont font_Label( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxNORMAL, true );

//----------------------------------------------------------------------------
const int BTN_W = 50;
const int MAX_PANELS = 50;

//****************************************************
wnd_EditFrontPanels::wnd_EditFrontPanels( 
			  wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			)
			:wxDialog( _parent, wxID_ANY, wxT("Edit FrontPanels"), wxDefaultPosition, wxDefaultSize
		,wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE )
{
	stage = _stage;
	stage_declaration = _stage_declaration;
	stage_set = _stage_set;
	
	nb_panels = 0;
	oPanels = NULL;
	oMap_OldNew_Panels__size = 0;
	oMap_OldNew_Panels = NULL;
	
	b_not_update_vals	= false;
	b_updating_vals = false;
	b_initialised = false;
	
	b_changed = false;
	
	wxButton* t_btn;
	wxSizer *t_sizer, *t_sizer2;

	//-------------------------------------
	wxSizer* sizer_main = new wxBoxSizer( wxVERTICAL );
	wxSizer* sizer_middle = new wxBoxSizer( wxHORIZONTAL );
	
	//-------------------------------------
	// Top
	wxSizer* t_sizer_top = new wxBoxSizer( wxHORIZONTAL );
	sizer_main->Add( t_sizer_top, 0, wxEXPAND );
	int BORDERS_SIZES = 1;
	
	// Control with current layer values
		// PATH
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("Path") )
					,0,wxALL, BORDERS_SIZES );
		
		ctrl_path = new prObFileChooserCtrl( this );
		ctrl_path->Connect( wxEVT_OBFILECHOOSER_CHANGE
			, wxCommandEventHandler(wnd_EditFrontPanels::Evt_ParamChange)
			, NULL, this);
		ctrl_path->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(wnd_EditFrontPanels::Evt_ParamFocusOut)
			, NULL, this);
		ctrl_path->SetMinSize( wxSize( 150, wxDefaultCoord));
		t_sizer2->Add( ctrl_path,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 1, wxALL|wxEXPAND, 2 );

		
	t_sizer_top->AddStretchSpacer();

	//-------------------------------------
	sizer_main->Add( sizer_middle, 1, wxEXPAND );
	
	//-------------------------------------
	//Left
	t_sizer = new wxBoxSizer( wxVERTICAL );
	sizer_middle->Add( t_sizer, 0, wxEXPAND );
	
	// List of layers avalaile + Button Add
	t_sizer2 = new wxBoxSizer( wxHORIZONTAL );
	t_sizer->Add( t_sizer2, 0, 0 );
	
	t_btn = new wxButton( this, wxID_ANY, wxT("Move Up") );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(wnd_EditFrontPanels::Evt_MoveUp)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );
	
	t_btn = new wxButton( this, wxID_ANY, wxT("Move Down") );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(wnd_EditFrontPanels::Evt_MoveDown)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );
	
	t_sizer2 = new wxBoxSizer( wxHORIZONTAL );
	t_sizer->Add( t_sizer2, 0, 0 );
	
	t_btn = new wxButton( this, wxID_ANY, wxT("New" ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(wnd_EditFrontPanels::Evt_New)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );

	t_btn = new wxButton( this, wxID_ANY, wxT("Delete" ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(wnd_EditFrontPanels::Evt_Delete)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );

	
	
	//-------------------------------------
	// the list of Layers Here
	listCtrl_panels = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
					     0, NULL,
					     wxLB_SINGLE | wxLB_NEEDED_SB );
	listCtrl_panels->Connect( wxEVT_COMMAND_LISTBOX_SELECTED
		, wxCommandEventHandler(wnd_EditFrontPanels::Evt_SelectedChange)
		, NULL, this);
	t_sizer->Add( listCtrl_panels, 1, wxEXPAND | wxALL, 3 );
					     

	//-------------------------------------
	//-------------------------------------
	// Right
	// View of the layers
	sceneView = new Panel_SceneView( this
				, stage, stage_declaration, stage_set
				);
	sizer_middle->Add( sceneView, 1, wxEXPAND );

	//-------------------------------------
	// Down
	t_sizer = new wxBoxSizer( wxHORIZONTAL );
	sizer_main->Add( t_sizer, 0, wxEXPAND );

	t_sizer->AddStretchSpacer(3);
	
	// Btns OK CANCEL
	t_btn = new wxButton( this, wxID_ANY, wxT("OK") );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(wnd_EditFrontPanels::Evt_OK)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );

	t_sizer->AddStretchSpacer();
	
	t_btn = new wxButton( this, wxID_ANY, wxT("Cancel" ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(wnd_EditFrontPanels::Evt_Cancel)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );
	
	t_sizer->AddStretchSpacer(3);

	
	//***********************************
	// Finalisation of the window
	SetMinSize( wxSize( 780,530) );
	SetSizer( sizer_main );
  	Show();
	Frame_RestorePrevCoord( this, wxT("editFrontPanels_frame"));
	
	// Initialisations
	Init();
	b_initialised = true;
}


//****************************************************
void 
wnd_EditFrontPanels::Init()
{
	nb_panels = 0;
	oPanels = new ob_front_panel*[MAX_PANELS];
	
	if( stage == NULL )
		return;
	
	ob_front_panel** t_objs = (ob_front_panel**) 
			stage->GetSubObjectS_ofType( OB_TYPE_FRONT_PANEL, nb_panels );
	
	// On fait des copies des objets
	for( size_t i = 0; i < nb_panels; i++)
		oPanels[i] = (ob_front_panel*) t_objs[i]->Clone();
	
	if( nb_panels > 0 )
		delete[] t_objs;
	
	// On remplit la liste avec les nom des objets
	for( size_t i = 0; i < nb_panels; i++)
		listCtrl_panels->Append( oPanels[i]->GetName() );
	
	// Fill the Association Old New array
	oMap_OldNew_Panels__size = nb_panels;
	if( nb_panels > 0 )
	{
		oMap_OldNew_Panels = new ob_front_panel*[nb_panels];
		for( size_t i = 0; i < nb_panels; i++ )
			oMap_OldNew_Panels[i] = oPanels[i];
	}
	
	// On selectionne le premier
	if( nb_panels > 0 )
		listCtrl_panels->Select( 0 );
	
	// On Update les val des champs
	Update_Panel_Vals();
	
	// On assigne puis dessine les panels
	sceneView->Set_Front_Panels( oPanels, nb_panels );
	Update_Panels_View();
}


//****************************************************
void 
wnd_EditFrontPanels::EvtActivate( wxActivateEvent& evt )
{
	evt.Skip();
}

//****************************************************
void 
wnd_EditFrontPanels::EvtClose( wxCloseEvent& evt )
{
	Frame_SaveCoord( this, wxT("editFrontPanels_frame"));
	evt.Skip();
}


//****************************************************

wnd_EditFrontPanels::~wnd_EditFrontPanels()
{
	b_initialised = false;
	size_t t_nb_panels = nb_panels;
	nb_panels = 0;
	
	if( oPanels != NULL )
	{
		for( size_t i = 0; i < t_nb_panels; i++ )
			if( oPanels[i] != NULL )
 				delete oPanels[i];
		delete[] oPanels;
	}

	if( oMap_OldNew_Panels != NULL )
		delete[] oMap_OldNew_Panels;
}


//****************************************************
void 
wnd_EditFrontPanels::Zero_Panel_Vals()
{
	ctrl_path->SetObPath(wxT("data"));
}


//****************************************************
void 
wnd_EditFrontPanels::Update_Panel_Vals()
{
	b_updating_vals = true;
	
	int num = listCtrl_panels->GetSelection();
	if( num == wxNOT_FOUND || num > (int) nb_panels )
	{
		Zero_Panel_Vals();
		b_updating_vals = false;
		return;
	}

	int i = 0;
	wxString t ;
	
	t = oPanels[num]->GetToken( i );
	ctrl_path->SetObPath(t);
	i++;
	
	b_updating_vals = false;
}

//****************************************************
void
wnd_EditFrontPanels::Evt_ParamFocusOut( wxFocusEvent& dummy )
{
	Check_Param_Change();
}

//****************************************************
void
wnd_EditFrontPanels::Evt_ParamChange( wxCommandEvent& evt_ctrl )
{
	Check_Param_Change();
}

//****************************************************
void
wnd_EditFrontPanels::Check_Param_Change()
{
	if( b_initialised == false || b_updating_vals == true )
		return;
	
	int num = listCtrl_panels->GetSelection();
	if( num == wxNOT_FOUND || num > (int) nb_panels )
	{
		Zero_Panel_Vals();
		return;
	}
	
	bool b_update_view = false;
	wxString t, t2, t3;
	int i = 0;
	
	// Check the values given
	t = ctrl_path->GetObPath();
	if( t != oPanels[num]->GetToken( i ) )
	{
		b_update_view = true;
		oPanels[num]->SetToken( i, t );
	}
	i++;
	
	listCtrl_panels->SetString( num, oPanels[num]->GetName() );
	
	if( b_update_view == true )
		Update_Panels_View();
}


//****************************************************
void 
wnd_EditFrontPanels::Update_Panels_View()
{
	if( nb_panels == 0 )
		return;
	
	sceneView->Update_View();
	return;
}

	
//****************************************************
void
wnd_EditFrontPanels::EvtResize( wxSizeEvent& evt )
{
	evt.Skip();
}

//****************************************************
void
wnd_EditFrontPanels::Evt_OK( wxCommandEvent& evt_menu )
{
	// Have to see if something is changed
	if( stage != NULL && nb_panels > 0 )
	{
		size_t orig_nb_panels = 0;
		ob_front_panel** orig_panels = (ob_front_panel**) 
				stage->GetSubObjectS_ofType( OB_TYPE_FRONT_PANEL, orig_nb_panels );

		// Update layers previously in the levels file 
		bool* b_updated = new bool[nb_panels];
		
		// Vars for reordering old panels
		size_t* panels_remaps = NULL;
		if( orig_nb_panels > 0 )
		{
			panels_remaps = new size_t[orig_nb_panels];
		}
		size_t curr_ind_remaps = 0;
		ob_front_panel* first_old_panel = NULL;
		
		// Get datas on old panels deletion and remaps 
		for( size_t i = 0; i < nb_panels; i++)
		{
			b_updated[i] = false;
			for( size_t j = 0; j < oMap_OldNew_Panels__size; j++ )
			{
				if( 		oMap_OldNew_Panels[j] != NULL 
					&&	oMap_OldNew_Panels[j] == oPanels[i]
				  )
				{
					b_updated[i] = true;
					b_changed = b_changed ||
						orig_panels[j]->Update_With( oPanels[i] );
					panels_remaps[curr_ind_remaps] = j;
					if( curr_ind_remaps == 0 )
						first_old_panel = orig_panels[j];
					curr_ind_remaps++;
					break;
				}
			}
		}
		
		// Delete layers previously in the levels file wich have been deleted
		for( size_t j = 0; j < oMap_OldNew_Panels__size; j++ )
		{
			if( oMap_OldNew_Panels[j] == NULL )
				delete orig_panels[j];
		}
		
		// Reorder the old existings panels
		ob_front_panel* prev = first_old_panel;
		for( size_t i = 1; i < curr_ind_remaps; i++ )
		{
			ob_front_panel* next = orig_panels[panels_remaps[i]];
			if( prev->next != next )
			{
				b_changed = true;
				prev->InsertObject_After(next);
			}
		}

		// Insert new layers
		prev = NULL;
		for( size_t i = 0; i < nb_panels; i++)
		{
			// New layer
			if( b_updated[i] == false )
			{
				ob_front_panel* new_panel=(ob_front_panel*)oPanels[i]->Clone();
				if( prev != NULL )
				{
					if( prev->InsertObject_After( new_panel ) == false )
					{
						wxMessageBox( wxT("Unable to insert frontpanel !"), wxT("ERROR !!")
								, wxICON_EXCLAMATION | wxOK );
						delete new_panel;
					}
				}
				else if( stage->Add_SubObj_FrontLayer( new_panel, false )
					   == false )
				{
					wxMessageBox( wxT("Unable to insert frontpanel !"), wxT("ERROR !!")
							, wxICON_EXCLAMATION | wxOK );
					delete new_panel;
				}
				else
					prev = new_panel;
			}
			else if( i == 0 )
				prev = orig_panels[panels_remaps[0]];
			else
			{
				ob_object* t = prev->next;
				while( t != NULL && t->type != OB_TYPE_FRONT_PANEL )
				{
					t = t->next;
				}
				prev = (ob_front_panel*) t;
			}
		}

		if( panels_remaps != NULL )
			delete[] panels_remaps;
		delete[] b_updated;
		if( orig_panels != NULL )
			delete[] orig_panels;
	}
	Close();
}


//****************************************************
void
wnd_EditFrontPanels::Evt_Cancel( wxCommandEvent& evt_menu )
{
    Close();
}

//****************************************************
void
wnd_EditFrontPanels::Evt_SelectedChange( wxCommandEvent& evt_menu )
{
	if( b_not_update_vals == true )
		return;
	
	int num = listCtrl_panels->GetSelection();
	if( num == wxNOT_FOUND || num > (int) nb_panels )
		return;

	Update_Panel_Vals();
}

//****************************************************
void
wnd_EditFrontPanels::Evt_MoveUp( wxCommandEvent& evt_menu )
{
	int num = listCtrl_panels->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_panels || num < 1 )
		return;

	ob_front_panel* t = oPanels[num-1];
	oPanels[num-1] = oPanels[num];
	oPanels[num  ] = t;

	b_not_update_vals = true;
	listCtrl_panels->SetString( num-1, oPanels[num-1]->GetName() );
	listCtrl_panels->SetString( num,   oPanels[num  ]->GetName() );
	listCtrl_panels->Select( num-1 );
	b_not_update_vals = false;
	
	// Update stuffs
	Update_Panels_View();
}

//****************************************************
void
wnd_EditFrontPanels::Evt_MoveDown( wxCommandEvent& evt_menu )
{
	int num = listCtrl_panels->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_panels - 1 )
		return;
	
	ob_front_panel* t = oPanels[num+1];
	oPanels[num+1] = oPanels[num];
	oPanels[num  ] = t;

	b_not_update_vals = true;
	listCtrl_panels->SetString( num+1, oPanels[num+1]->GetName() );
	listCtrl_panels->SetString( num,   oPanels[num  ]->GetName() );
	listCtrl_panels->Select( num+1 );
	b_not_update_vals = false;
	
	// Update stuffs
	Update_Panels_View();
}

//****************************************************
void
wnd_EditFrontPanels::Evt_New( wxCommandEvent& evt_menu )
{
	myWnd_FrontPanelProperties t( this, NULL, true );
	t.ShowModal();

	if( t.theObj == NULL )
		return;
	
	if( (int) nb_panels >= MAX_PANELS )
	{
		delete t.theObj;
		t.theObj = NULL;
		wxMessageBox( wxT("TOO MUCH PANELS !") );
		return;
	}
	
	// Add it to the panels list
	oPanels[nb_panels] = (ob_front_panel*) t.theObj;
	nb_panels++;
	
	// Add it to the Control list
	listCtrl_panels->Append( ((ob_front_panel*) t.theObj)->GetName() );
	listCtrl_panels->Select( nb_panels - 1 );
	
	// Update stuffs
	Update_Panel_Vals();
	Update_Panels_View();
}

//****************************************************
void 
wnd_EditFrontPanels::Evt_Delete( wxCommandEvent& evt_menu )
{
    if( nb_panels == 0 )
	    return;
    
	int num = listCtrl_panels->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_panels )
		return;
	
	int res = wxMessageBox( wxT("Sure to delete this panel ?"), wxT("Warning !")
					, wxICON_EXCLAMATION | wxYES_NO, this );
	if( res != wxYES )
		return;

	// Update the association memory array
	for( size_t i = 0; i < oMap_OldNew_Panels__size; i++)
		if( oMap_OldNew_Panels[i] == oPanels[num] )
		{
			oMap_OldNew_Panels[i] = NULL;
			break;
		}

	delete oPanels[num];
	
	for( int i = num; i < (int)nb_panels-1; i++ )
		oPanels[i] = oPanels[i+1];
	nb_panels--;
    
	listCtrl_panels->Delete( num );
	
	if( nb_panels == 0 )
	{
		Update_Panels_View();
		return;
	}
	
	if( num >= (int) nb_panels )
		num = nb_panels-1;
	listCtrl_panels->Select( num );
	Update_Panel_Vals();
	Update_Panels_View();
}

//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(wnd_EditFrontPanels, wxDialog)
	EVT_ACTIVATE(wnd_EditFrontPanels::EvtActivate)
	EVT_CLOSE( wnd_EditFrontPanels::EvtClose )
	EVT_SIZE(wnd_EditFrontPanels::EvtResize)
END_EVENT_TABLE()

