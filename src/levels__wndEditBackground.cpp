/*
 * levels__myWnd_EditBackground.cpp
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
#include "levels__wndEditBackground.h"
#include "levels__Ctrl_SceneView.h"


const wxFont font_Alphabet( 18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true );
const wxFont font_Label( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxNORMAL, true );

//----------------------------------------------------------------------------
const int MAX_LAYERS = 20;
const int REPEAT_FOREVER = 100;
const int BTN_W = 50;
wxString alphaChoices[] = {wxT("0"),wxT("1"),wxT("2"),wxT("3"),wxT("4"),wxT("5"),wxT("6")};

//****************************************************
myWnd_EditBackground::myWnd_EditBackground( 
			  wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			)
			:wxDialog( _parent, wxID_ANY, wxT("Edit Backgound"), wxDefaultPosition, wxDefaultSize
		,wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE )
{
	stage = _stage;
	stage_declaration = _stage_declaration;
	stage_set = _stage_set;
	
	b_not_update_vals	= false;
	b_updating_vals = false;
	b_initialised = false;
	b_changed = false;

	nb_layers = 0;
	oLayers = NULL;
	oMap_OldNew_Layers__size = 0;
	oMap_OldNew_Layers = NULL;
	
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
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange)
			, NULL, this);
		ctrl_path->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
//		ctrl_path->SetMaxSize( wxSize( BTN_W + 20, wxDefaultCoord));
		t_sizer2->Add( ctrl_path
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );

		// Xratio
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("xRat") )
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Xratio = new wxTextCtrl(this, wxID_ANY
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorFloatPositive() );
		txtCtrl_Xratio->Connect( wxEVT_COMMAND_TEXT_ENTER
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		txtCtrl_Xratio->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Xratio->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Xratio->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Xratio 
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Zratio
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("zRat") )
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Zratio = new wxTextCtrl(this, wxID_ANY
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorFloatPositive() );
		txtCtrl_Zratio->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Zratio->Connect( wxEVT_KILL_FOCUS
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange)
			, NULL, this);
		txtCtrl_Zratio->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Zratio->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Zratio
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Xpos
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("xPos") )
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Xpos = new wxTextCtrl(this, wxID_ANY
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorIntegerRelative() );
		txtCtrl_Xpos->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Xpos->Connect( wxEVT_COMMAND_TEXT_ENTER
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		txtCtrl_Xpos->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Xpos->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Xpos 
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Zpos
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("zPos") )
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Zpos = new wxTextCtrl(this, wxID_ANY 
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorIntegerRelative() );
		txtCtrl_Zpos->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Zpos->Connect( wxEVT_COMMAND_TEXT_ENTER
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		txtCtrl_Zpos->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Zpos->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Zpos
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Xspace
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("xSpace") )
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Xspace = new wxTextCtrl(this, wxID_ANY 
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorIntegerRelative() );
		txtCtrl_Xspace->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Xspace->Connect( wxEVT_COMMAND_TEXT_ENTER
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		txtCtrl_Xspace->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Xspace->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Xspace
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Zspace
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("zSpace") )
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Zspace = new wxTextCtrl(this, wxID_ANY
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorIntegerRelative() );
		txtCtrl_Zspace->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Zspace->Connect( wxEVT_COMMAND_TEXT_ENTER
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		txtCtrl_Zspace->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Zspace->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Zspace
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Xrepeat
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("xRep" ))
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Xrepeat = new wxTextCtrl(this, wxID_ANY
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorIntegerRelative() );
		txtCtrl_Xrepeat->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Xrepeat->Connect( wxEVT_COMMAND_TEXT_ENTER
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		txtCtrl_Xrepeat->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Xrepeat->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Xrepeat
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Zrepeat
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("zRep" ))
					,0,wxALL, BORDERS_SIZES );
		txtCtrl_Zrepeat = new wxTextCtrl(this, wxID_ANY 
			, wxString(), wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER
			, wxValidatorIntegerRelative() );
		txtCtrl_Zrepeat->Connect( wxEVT_KILL_FOCUS
			, wxFocusEventHandler(myWnd_EditBackground::Evt_bgParamFocusOut)
			, NULL, this);
		txtCtrl_Zrepeat->Connect( wxEVT_COMMAND_TEXT_ENTER
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		txtCtrl_Zrepeat->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		txtCtrl_Zrepeat->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( txtCtrl_Zrepeat
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Transparency
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("Trans" ))
					,0,wxALL, BORDERS_SIZES );
		chkCtrl_transparency = new wxCheckBox(this, wxID_ANY, wxString() );
		chkCtrl_transparency->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this);
		chkCtrl_transparency->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		chkCtrl_transparency->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( chkCtrl_transparency
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
		// Alpha
		t_sizer2 = new wxBoxSizer( wxVERTICAL );
		t_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("Alpha" ))
					,0,wxALL, BORDERS_SIZES );
				comboCtrl_alpha = new wxComboBox(this, wxID_ANY, wxT("0")
				, wxDefaultPosition,wxDefaultSize
				, 7, alphaChoices
				, wxCB_READONLY|wxCB_DROPDOWN );
		comboCtrl_alpha->Connect( wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(myWnd_EditBackground::Evt_bgParamChange )
			, NULL, this );
		comboCtrl_alpha->SetMinSize( wxSize( BTN_W, wxDefaultCoord));
		comboCtrl_alpha->SetMaxSize( wxSize( BTN_W, wxDefaultCoord));
		t_sizer2->Add( comboCtrl_alpha 
					,0,wxALL, BORDERS_SIZES );
		t_sizer_top->Add( t_sizer2, 0, wxALL, 2 );
		
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
		, wxCommandEventHandler(myWnd_EditBackground::Evt_MoveLayerUp)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );
	
	t_btn = new wxButton( this, wxID_ANY, wxT("Move Down") );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(myWnd_EditBackground::Evt_MoveLayerDown)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );
	
	t_sizer2 = new wxBoxSizer( wxHORIZONTAL );
	t_sizer->Add( t_sizer2, 0, 0 );
	
	t_btn = new wxButton( this, wxID_ANY, wxT("New Layer" ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(myWnd_EditBackground::Evt_NewLayer)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );

	t_btn = new wxButton( this, wxID_ANY, wxT("Delete Layer") );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(myWnd_EditBackground::Evt_DeleteLayer)
		, NULL, this);
	t_sizer2->Add( t_btn, 0, wxALL, 3 );

	
	
	//-------------------------------------
	// the list of Layers Here
	listCtrl_bg = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
					     0, NULL,
					     wxLB_SINGLE | wxLB_NEEDED_SB );
	listCtrl_bg->Connect( wxEVT_COMMAND_LISTBOX_SELECTED
		, wxCommandEventHandler(myWnd_EditBackground::Evt_SelectedChange)
		, NULL, this);
	listCtrl_bg->Connect( wxEVT_LEFT_DCLICK 
		, wxMouseEventHandler(myWnd_EditBackground::Evt_SelectedDblCick)
		, NULL, this);
	t_sizer->Add( listCtrl_bg, 1, wxEXPAND | wxALL, 3 );
					     

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
		, wxCommandEventHandler(myWnd_EditBackground::Evt_OK)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );

	t_sizer->AddStretchSpacer();
	
	t_btn = new wxButton( this, wxID_ANY, wxT("Cancel" ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(myWnd_EditBackground::Evt_Cancel)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );
	
	t_sizer->AddStretchSpacer(3);

	
	//***********************************
	// Finalisation of the window
	SetMinSize( wxSize( 780,530) );
	SetSizer( sizer_main );
  	Show();
	Frame_RestorePrevCoord( this, wxT("editBackground_frame"));
	
	// Initialisations
	Init();
	b_initialised = true;
}


//****************************************************
void 
myWnd_EditBackground::Init()
{
	if( stage == NULL )
		return;
	
	nb_layers = 0;
	oLayers = new ob_BG_Layer*[MAX_LAYERS];
	
	ob_BG_Layer** t_objs = (ob_BG_Layer**) 
			stage->GetSubObjectS_ofType( OB_TYPE_BG_LAYER, nb_layers );
	
	// On place l'objet background en premier
	bool b_found = false;
	for( size_t i = 0; i < nb_layers; i++)
	{
		if( t_objs[i]->name.Upper() == wxT("BACKGROUND" ))
		{
			if( i > 0 )
			{
				ob_BG_Layer* t  = t_objs[0];
				for( size_t j = i; j > 0; j-- )
					t_objs[j] = t_objs[j-1];
				t_objs[0] = t;
				t_objs[1]->InsertObject_Before( t );
				b_changed = true;
			}
			b_found = true;
			break;
		}
	}

	if( b_found == false )
	{
		wxMessageBox( wxT("No background Found !"), wxT("WARNING")
			, wxICON_EXCLAMATION | wxOK );
	}
	

	// On fait des copies des objets
	for( size_t i = 0; i < nb_layers; i++)
		oLayers[i] = (ob_BG_Layer*)  t_objs[i]->Clone();
	
	if( nb_layers > 0 )
		delete[] t_objs;
	
	for( size_t i = 0; i < nb_layers; i++)
	{
		// On normalise les objets bgLayers
		oLayers[i]->FillMissings();
		
		// On remplit la liste avec les nom des objets
		wxString item_str = oLayers[i]->name;
		if( i > 0 )
			item_str = oLayers[i]->GetName();
		
		listCtrl_bg->Append( item_str );
	}
	
	// Fill the Association Old New array
	oMap_OldNew_Layers__size = nb_layers;
	if( nb_layers > 0 )
	{
		oMap_OldNew_Layers = new ob_BG_Layer*[nb_layers];
		for( size_t i = 0; i < nb_layers; i++ )
			oMap_OldNew_Layers[i] = oLayers[i];
	}
	
	// On selectionne le premier
	listCtrl_bg->Select( 0 );
	
	// On Update les val des champs
	Update_Layer_Vals();
	
	// On assigne puis dessine les layers
	sceneView->Set_Background( oLayers, nb_layers );
	Update_Layers_View();
}


//****************************************************
void 
myWnd_EditBackground::EvtActivate( wxActivateEvent& evt )
{
	evt.Skip();
}

//****************************************************
void 
myWnd_EditBackground::EvtClose( wxCloseEvent& evt )
{
	Frame_SaveCoord( this, wxT("editBackground_frame"));
	evt.Skip();
}


//****************************************************

myWnd_EditBackground::~myWnd_EditBackground()
{
	b_initialised = false;
	
	size_t t_nb_layers = nb_layers;
	nb_layers = 0;
	
	if( oLayers != NULL )
	{
		for( size_t i = 0; i < t_nb_layers; i++ )
			if( oLayers[i] != NULL )
				delete oLayers[i];
		delete[] oLayers;
	}

	if( oMap_OldNew_Layers != NULL )
		delete[] oMap_OldNew_Layers;
}


//****************************************************
void 
myWnd_EditBackground::Zero_Layer_Vals()
{
	ctrl_path->SetObPath(wxT("data"));
	txtCtrl_Xratio->SetValue( wxT("1" ));
	txtCtrl_Zratio->SetValue( wxT("1" ));
	txtCtrl_Xpos->SetValue( wxT("0" ));
	txtCtrl_Zpos->SetValue( wxT("0" ));
	txtCtrl_Xspace->SetValue( wxT("0" ));
	txtCtrl_Zspace->SetValue( wxT("0" ));
	txtCtrl_Xrepeat->SetValue( wxT("5000") );
	txtCtrl_Zrepeat->SetValue( wxT("5000") );;
	chkCtrl_transparency->SetValue( true );;
	comboCtrl_alpha->SetValue( wxT("0") );
}


//****************************************************
void 
myWnd_EditBackground::Update_Layer_Vals()
{
	b_updating_vals = true;
	
	int num = listCtrl_bg->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_layers )
	{
		Zero_Layer_Vals();
		b_updating_vals = false;
		return;
	}

	int i = 0;
	wxString t ;
	
	t = oLayers[num]->GetToken( i );
	ctrl_path->SetObPath(t);
	i++;
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Xratio->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Zratio->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Xpos->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Zpos->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Xspace->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Zspace->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Xrepeat->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	txtCtrl_Zrepeat->SetValue( t );
	
	t = oLayers[num]->GetToken( i );
	i++;
	chkCtrl_transparency->SetValue( t == wxT("1") );
	
	t = oLayers[num]->GetToken( i );
	i++;
	comboCtrl_alpha->SetValue( t );

	b_updating_vals = false;
}

//****************************************************
void
myWnd_EditBackground::Evt_bgParamFocusOut( wxFocusEvent& dummy )
{
	Check_Param_Change();
}

//****************************************************
void
myWnd_EditBackground::Evt_bgParamChange( wxCommandEvent& evt_ctrl )
{
	Check_Param_Change();
}

//****************************************************
void
myWnd_EditBackground::Check_Param_Change()
{
	if( b_initialised == false || b_updating_vals == true )
		return;
	
	int num = listCtrl_bg->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_layers )
	{
		Zero_Layer_Vals();
		return;
	}
	
	bool b_update_view = false;
	wxString t, t2, t3;
	int i = 0;
	
	// Check the values given
	t = ctrl_path->GetObPath();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Xratio->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Zratio->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Xpos->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Zpos->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Xspace->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Zspace->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Xrepeat->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = txtCtrl_Zrepeat->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;

	t = chkCtrl_transparency->GetValue() ? wxT("1") : wxT("0");
	if( t != oLayers[num]->GetToken( i ) )
	{
		b_update_view = true;
		oLayers[num]->SetToken( i, t );
	}
	i++;
	
	t = comboCtrl_alpha->GetValue();
	if( t != oLayers[num]->GetToken( i ) )
		oLayers[num]->SetToken( i, t );
	i++;
	
	if( oLayers[num]->name.Upper() != wxT("BACKGROUND") )
		listCtrl_bg->SetString( num, oLayers[num]->GetName() );
	
	if( b_update_view == true )
		Update_Layers_View();
}


//****************************************************
void 
myWnd_EditBackground::Update_Layers_View()
{
	if( nb_layers == 0 )
		return;
	
	sceneView->Update_View();
	return;
}

	
//****************************************************
void
myWnd_EditBackground::EvtResize( wxSizeEvent& evt )
{
	evt.Skip();
}

//****************************************************
void
myWnd_EditBackground::Evt_OK( wxCommandEvent& evt_menu )
{
	// Have to see if something is changed
	if( stage != NULL && nb_layers > 0 )
	{
		size_t orig_nb_layers = 0;
		ob_BG_Layer** orig_Layers = (ob_BG_Layer**) 
				stage->GetSubObjectS_ofType( OB_TYPE_BG_LAYER, orig_nb_layers );
		
		// Update layers previously in the levels file 
		bool* b_updated = NULL;
		if( nb_layers > 0 )
			b_updated = new bool[nb_layers];
		
		for( size_t i = 0; i < nb_layers; i++)
		{
			b_updated[i] = false;
			for( size_t j = 0; j < oMap_OldNew_Layers__size; j++ )
			{
				if( 		oMap_OldNew_Layers[j] != NULL
					&&	oMap_OldNew_Layers[j] == oLayers[i] )
				{
					b_updated[i] = true;
					b_changed = b_changed ||
						orig_Layers[j]->Update_With( oLayers[i] );
					break;
				}
			}
		}

		// Delete layers previously in the levels file wich have been deleted
		for( size_t j = 0; j < oMap_OldNew_Layers__size; j++ )
		{
			if( oMap_OldNew_Layers[j] == NULL )
			{
				b_changed = true;
				delete orig_Layers[j];
				orig_Layers[j] = NULL;
			}
		}

		// Reordering previously existing layers
		if( orig_nb_layers > 0 )
		{
			// Get the remappings of the indices
			size_t* lay_remaps = new size_t[orig_nb_layers];
			for( size_t j = 0; j < nb_layers; j++ )
			{
				lay_remaps[j] = 666;
				for( size_t i = 0; i < orig_nb_layers; i++ )
				{
					if( oMap_OldNew_Layers[i] == oLayers[j] )
					{
						lay_remaps[j] = i;
						if( lay_remaps[j] != i )
							b_changed = true;
						break;
					}
				}
			}
			
			// 0  ==>  background, already there
			ob_BG_Layer* prev = orig_Layers[0];
			for( size_t i = 0; i < orig_nb_layers; i++)
			{
				if( lay_remaps[i] == 666 )
					continue;
				
				prev->InsertObject_After( orig_Layers[lay_remaps[i]] );
				prev = orig_Layers[lay_remaps[i]];
			}
			
			delete[] lay_remaps;
		}


		ob_BG_Layer* prev_layer = orig_Layers[0]; // The background already exists
		for( size_t i = 1; i < nb_layers; i++)
		{
			// New layer
			if( b_updated[i] == false )
			{
				ob_BG_Layer* new_layer = (ob_BG_Layer*)  oLayers[i]->Clone();
				if( prev_layer->InsertObject_After( new_layer ) == false )
				{
					wxMessageBox( wxT("Unable to insert bgLayer !"), wxT("ERROR !!")
							, wxICON_EXCLAMATION | wxOK );
					delete new_layer;
				}
				else 
				{
					b_changed = true;
					prev_layer = new_layer;
				}
			}
			else
				prev_layer = (ob_BG_Layer*) prev_layer->next;
		}
		
		if( b_updated != NULL )
			delete[] b_updated;
		if( orig_Layers != NULL )
			delete[] orig_Layers;
	}

	Close();
}


//****************************************************
void
myWnd_EditBackground::Evt_Cancel( wxCommandEvent& evt_menu )
{
    Close();
}

//****************************************************
void
myWnd_EditBackground::Evt_SelectedChange( wxCommandEvent& evt_menu )
{
	if( b_not_update_vals == true )
		return;
	
	int num = listCtrl_bg->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_layers )
		return;

	Update_Layer_Vals();
}

//****************************************************
void
myWnd_EditBackground::Evt_SelectedDblCick( wxMouseEvent& evt )
{
	if( b_not_update_vals == true )
		return;
	
	int num = listCtrl_bg->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_layers )
		return;

	myWnd_BGProperties t( this, oLayers[num], false );
	t.ShowModal();
	
	if( t.b_chg == true )
	{
		Update_Layer_Vals();
		Update_Layers_View();
	}
	
}

//****************************************************
void
myWnd_EditBackground::Evt_MoveLayerUp( wxCommandEvent& evt_menu )
{
	int num = listCtrl_bg->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_layers || num <= 1 )
		return;

	if( num == 0 )
		return;

	ob_BG_Layer* t = oLayers[num-1];
	oLayers[num-1] = oLayers[num];
	oLayers[num  ] = t;

	b_not_update_vals = true;
	listCtrl_bg->SetString( num-1, oLayers[num-1]->GetName() );
	listCtrl_bg->SetString( num,   oLayers[num  ]->GetName() );
	listCtrl_bg->Select( num-1 );
	b_not_update_vals = false;
	
	// Update stuffs
	Update_Layers_View();
}

//****************************************************
void
myWnd_EditBackground::Evt_MoveLayerDown( wxCommandEvent& evt_menu )
{
	int num = listCtrl_bg->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_layers - 1 || num == 0 )
		return;
	
	if( num == (int) nb_layers )
		return;

	ob_BG_Layer* t = oLayers[num+1];
	oLayers[num+1] = oLayers[num];
	oLayers[num  ] = t;

	b_not_update_vals = true;
	listCtrl_bg->SetString( num+1, oLayers[num+1]->GetName() );
	listCtrl_bg->SetString( num,   oLayers[num  ]->GetName() );
	listCtrl_bg->Select( num+1 );
	b_not_update_vals = false;
	
	// Update stuffs
	Update_Layers_View();
}

//****************************************************
void
myWnd_EditBackground::Evt_NewLayer( wxCommandEvent& evt_menu )
{
	myWnd_BGProperties t( this, NULL, false );
	t.ShowModal();

	if( t.theObj == NULL )
		return;
	
	if( (int) nb_layers >= MAX_LAYERS )
	{
		delete t.theObj;
		t.theObj = NULL;
		wxMessageBox( wxT("TOO MUCH LAYERS !") );
		return;
	}
	
	// Add it to the layer list
	oLayers[nb_layers] = (ob_BG_Layer*) t.theObj;
	nb_layers++;
	
	// Add it to the Control list
	listCtrl_bg->Append( ((ob_BG_Layer*) t.theObj)->GetName() );
	listCtrl_bg->Select( nb_layers - 1 );
	
	// Update stuffs
	Update_Layer_Vals();
	Update_Layers_View();
}

//****************************************************
void 
myWnd_EditBackground::Evt_DeleteLayer( wxCommandEvent& evt_menu )
{
    if( nb_layers == 0 )
	    return;
    
	int num = listCtrl_bg->GetSelection();
	if( num == wxNOT_FOUND || num >= (int) nb_layers )
		return;
	
	int res = wxMessageBox( wxT("Sure to delete this layer ?"), wxT("Warning !")
					, wxICON_EXCLAMATION | wxYES_NO, this );
	if( res != wxYES )
		return;

	// Update the associtaion memory array
	for( size_t i = 0; i < oMap_OldNew_Layers__size; i++)
		if( oMap_OldNew_Layers[i] == oLayers[num] )
		{
			oMap_OldNew_Layers[i] = NULL;
			break;
		}
			
	delete oLayers[num];
	
	for( int i = num; i < (int)nb_layers-1; i++ )
		oLayers[i] = oLayers[i+1];
	nb_layers--;
    
	listCtrl_bg->Delete( num );
	
	if( nb_layers == 0 )
	{
		Update_Layers_View();
		return;
	}
	
	if( num >= (int) nb_layers )
		num = nb_layers-1;
	listCtrl_bg->Select( num );
	Update_Layer_Vals();
	Update_Layers_View();
}

//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(myWnd_EditBackground, wxDialog)
	EVT_ACTIVATE(myWnd_EditBackground::EvtActivate)
	EVT_CLOSE( myWnd_EditBackground::EvtClose )
	EVT_SIZE(myWnd_EditBackground::EvtResize)
END_EVENT_TABLE()

