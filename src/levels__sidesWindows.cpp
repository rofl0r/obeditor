/*
 * levels__sidesWindows.cpp
 *
 *  Created on: 27 avr. 2009
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__tools.h"
#include "common__object_stage.h"
#include "levels__sidesWindows.h"
#include "levels__globals.h"
#include "levels__Ctrl_SceneView.h"


//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_SETProperties::myWnd_SETProperties(wxWindow *_parent, ob_StagesSet* _set, const wxString& wnd_name )
:FrameEditProperties( _parent, _set, wnd_name )
{
	wndSavingName = wnd_name;
	Init();
}


//****************************************************

myWnd_SETProperties::~myWnd_SETProperties()
{
}


//****************************************************

void myWnd_SETProperties::InitObj()
{
	if( theObj == NULL )
	{
		theObj = new ob_StagesSet();
		theObj->SetToDefault();
		b_newObj = true;
	}
}


//****************************************************

void myWnd_SETProperties::InitProps()
{
	int alloc = 30;
	reqs = new frameEdit_request[alloc];

	int i =0;
	reqs[i].name = "_TOKEN_0";
	reqs[i].label = "SET name : ";
	reqs[i].SetPropType( PROPTYPE_STRING_NO_WS);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "20";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "select";
	reqs[i].b_optional = true;
	reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_OBFILE);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "20";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "nosame";
	reqs[i].label = "Clones characters forbidden";
	reqs[i].SetPropType( PROPTYPE_BOOL);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "noshare";
	reqs[i].label = "Players don't share credits";
	reqs[i].SetPropType( PROPTYPE_BOOL);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "ifcomplete";
	reqs[i].label = "Secret level";
	reqs[i].SetPropType( PROPTYPE_BOOL );
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "20";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "lives";
	reqs[i].label = "Nb of lives";
	reqs[i].SetPropType( PROPTYPE_NUMBER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "credits";
	reqs[i].label = "Nb of credits";
	reqs[i].SetPropType( PROPTYPE_NUMBER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "20";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "maxplayers";
	reqs[i].label = "Max players";
	reqs[i].SetPropType( PROPTYPE_ENUMS);
	wxString t0[] = { "1", "2", "3", "4" };
	TabStringToArrayString( t0, 4, reqs[i].enums );

	i++;
	reqs[i].name = "typemp";
	reqs[i].label = "MP recovery mode";
	reqs[i].SetPropType( PROPTYPE_RANGE);
	wxString t1[] = { _T("With time"), _T("Hitting Enemies"), _T("Items and rebirth") };
	TabStringToArrayString( t1, 3, reqs[i].enums );

	i++;
	reqs[i].name = "cansave";
	reqs[i].label = "Saving mode";
	reqs[i].SetPropType( PROPTYPE_RANGE);
	wxString t2[] = { _T("Disable"), _T("Last Level"), _T("Full") };
	TabStringToArrayString( t2, 3, reqs[i].enums );

	i++;
	nb_props = i;
}


//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_LEVELSSTAGEProperties::myWnd_LEVELSSTAGEProperties(wxWindow *_parent, ob_StageDeclaration* _stDeclar, bool b_init, const wxString& wnd_name )
:FrameEditProperties( _parent, NULL, wnd_name )
{
	theObj = _stDeclar;
	b_newObj = (theObj == NULL);

	wndSavingName = "LevelstxtStageProperties";
	if( b_init )
		Init();
}



//****************************************************

myWnd_LEVELSSTAGEProperties::~myWnd_LEVELSSTAGEProperties()
{
}

//****************************************************

void myWnd_LEVELSSTAGEProperties::Clicked_OK(bool b_close )
{
	FrameEditProperties::Clicked_OK(false);

	wxString _newName = theObj->GetSubObject_Token( "temp_name" );

	if( _newName != ((ob_StageDeclaration*)theObj)->GetStageName() )
		b_chg = true;

	// Must update the branch point
	if( b_has_branch && !StrToInt( theObj->GetSubObject_Token( "temp_bhasbranch" ) ) )
	{
		ob_object* branch = theObj->GetSubObject( "branch" );
		if( branch != NULL )
			branch->Rm();
	}

	else if( ! b_has_branch && StrToInt( theObj->GetSubObject_Token( "temp_bhasbranch" ) ) )
	{
		ob_object* t = new ob_object( "branch", _newName );
		theObj->Add_SubObj( t );
	}

	// Must set the name
	((ob_StageDeclaration*)theObj)->SetStageName( _newName );

	Cleanings();
	if( b_close )
		Close();
}


//****************************************************
void 
myWnd_LEVELSSTAGEProperties::Clicked_Cancel(bool b_close)
{
	FrameEditProperties::Clicked_Cancel(false);
	Cleanings();
	if( b_close )
		Close();
}


//-----------------------------------------------------------------
void 
myWnd_LEVELSSTAGEProperties::Cleanings()
{
	ob_object* t = theObj->GetSubObject( "temp_bhasbranch" );
	if( t != NULL )
		t->Rm();
	t = theObj->GetSubObject( "temp_name" );
	if( t != NULL )
		t->Rm();

	// Branch tag must be before the file tag
	if( theObj != NULL )
	{
		ob_object* t = theObj->first_subobj;
		ob_object* _f = NULL;
		ob_object* _b = NULL;
		while( t != NULL )
		{
			if( t->name.Upper() == "BRANCH" )
			{
				if( _f != NULL )
				{
					_b = t;
				}
				else
					t = NULL;
			}
			else if( t->name.Upper() == "FILE" )
			{
				_f = t;
			}

			if( t != NULL )
				t = t->next;
		}

		// Branch found after the file
		if( _b != NULL )
		{
			_f->InsertObject_Before( _b );
		}
	}

}


//****************************************************

void myWnd_LEVELSSTAGEProperties::InitObj()
{
	if( theObj == NULL )
	{
		theObj = new ob_StageDeclaration();
		theObj->SetToDefault();
		b_newObj = true;
	}

	// Handle the stage name problem -> Create a name property
	wxString StageName = ((ob_StageDeclaration*)theObj)->GetStageName();
	b_has_branch = (theObj->GetSubObject( "branch" ) != NULL);
	theObj->SetProperty( "temp_name", StageName );
	theObj->SetProperty( "temp_bhasbranch", BoolToStr(b_has_branch) );
}


//****************************************************

void myWnd_LEVELSSTAGEProperties::InitProps()
{
	int alloc = 30;
	reqs = new frameEdit_request[alloc];

	int i =-1;

	i++;
	reqs[i].name = "temp_name";
	reqs[i].label = "Stage name : ";
	reqs[i].SetPropType( PROPTYPE_STRING_NO_WS);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "20";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "file";
	reqs[i].label = "Stage file : ";
	reqs[i].SetPropType( PROPTYPE_OBFILE);

	i++;
	reqs[i].name = "20";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "_SUBOBJ_NAME%z%ALLTOKENS";
	reqs[i].label = "Z : ";
	reqs[i].labelw = 40;
	reqs[i].SetPropType( PROPTYPE_STRING);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "20";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "temp_bhasbranch";
	reqs[i].label = "Is a branch point";
	reqs[i].SetPropType( PROPTYPE_BOOL);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "next";
	reqs[i].label = "Display 'Stage complete' screen";
	reqs[i].SetPropType( PROPTYPE_BOOL_EXSISTENCE);

	i++;
	reqs[i].name = "end";
	reqs[i].label = "Game End";
	reqs[i].SetPropType( PROPTYPE_BOOL_EXSISTENCE);


	i++;
	nb_props = i;
}


//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_STAGEProperties::myWnd_STAGEProperties(wxWindow *_parent, ob_stage* _stage )
:FrameEditProperties( _parent, NULL, "STAGE properties")
{
	theObj = _stage;

	b_newObj = (theObj == NULL);

	wndSavingName = "StageProperties";
	Init();
}



//****************************************************

myWnd_STAGEProperties::~myWnd_STAGEProperties()
{
}

//****************************************************

void myWnd_STAGEProperties::Clicked_OK(bool b_close )
{
	FrameEditProperties::Clicked_OK(false);

	Cleanings();
	if( b_close )
		Close();
}


//****************************************************

void myWnd_STAGEProperties::Clicked_Cancel(bool b_close)
{
	FrameEditProperties::Clicked_Cancel(false);
	Cleanings();
	if( b_close )
		Close();
}


//****************************************************

void myWnd_STAGEProperties::Cleanings()
{
}


//****************************************************

void myWnd_STAGEProperties::InitObj()
{
	if( theObj == NULL )
	{
		theObj = new ob_stage();
		theObj->SetToDefault();
		b_newObj = true;
	}

}


//****************************************************

void myWnd_STAGEProperties::InitProps()
{
	int alloc = 30;
	reqs = new frameEdit_request[alloc];

	int i =-1;

	i++;
	reqs[i].name = "direction";
	reqs[i].label = "Direction";
	reqs[i].SetPropType( PROPTYPE_ENUMS);
	wxString t0[] = { "right", "both", "rightleft", "left", "leftright"
				, "up", "down", "in", "inout", "out", "outin" };
	TabStringToArrayString( t0, 11, reqs[i].enums );
	reqs[i].defVal = "right";
	reqs[i].b_optional = true;

	 
	i++;
	reqs[i].name = "_SUBOBJ_NAME%type%TOKEN%0";
	reqs[i].label = "Is Bonus Stage";
	reqs[i].SetPropType( PROPTYPE_BOOL );
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "_SUBOBJ_NAME%type%TOKEN%1";
	reqs[i].label = "Specials are forbidden";
	reqs[i].SetPropType( PROPTYPE_BOOL );
	reqs[i].nb_missing_tokens = 1;
	reqs[i].missing_tokens = new wxString[1];
	reqs[i].missing_tokens[0] = "0";
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "_SUBOBJ_NAME%type%TOKEN%2";
	reqs[i].label = "Players are invicible";
	reqs[i].SetPropType( PROPTYPE_BOOL );
	reqs[i].nb_missing_tokens = 2;
	reqs[i].missing_tokens = new wxString[2];
	reqs[i].missing_tokens[0] = "0";
	reqs[i].missing_tokens[1] = "0";
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "music";
	reqs[i].label = "Level music";
	reqs[i].SetPropType( PROPTYPE_OBFILE );
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "bossmusic";
	reqs[i].label = "Bosses music";
	reqs[i].SetPropType( PROPTYPE_OBFILE );
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "settime";
	reqs[i].label = "Level time limit";
	reqs[i].SetPropType( PROPTYPE_NUMBER );

	i++;
	reqs[i].name = "noreset";
	reqs[i].label = "Time reset mod :";
	reqs[i].SetPropType( PROPTYPE_RANGE );
	wxString t1[] = { _T("Respawns and Joins"), _T("Only on time out"), _T("On respawns") };
	TabStringToArrayString( t1, 3, reqs[i].enums );

	i++;
	reqs[i].name = "notime";
	reqs[i].label = "Timer is hidden";
	reqs[i].SetPropType( PROPTYPE_BOOL );

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "rock";
	reqs[i].label = "Background Rocking effect :";
	reqs[i].labelw = 255;
	reqs[i].SetPropType( PROPTYPE_RANGE );
	wxString t2[] = { _T("None"), _T("Boat"), _T("Train"), _T("Hard road") };
	TabStringToArrayString( t2, 4, reqs[i].enums );
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "mirror";
	reqs[i].label = "Mirror in the background";
	reqs[i].SetPropType( PROPTYPE_BOOL );
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "blocked";
	reqs[i].label = "The end of stage is a solid wall";
	reqs[i].SetPropType( PROPTYPE_BOOL );
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "endhole";
	reqs[i].label = "The end of stage is hole";
	reqs[i].SetPropType( PROPTYPE_BOOL );
	reqs[i].b_optional = true;

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "gravity";
	reqs[i].label = "Gravity :";
	reqs[i].labelw = 150;
	reqs[i].SetPropType( PROPTYPE_NEGATIVENUMBER );
	reqs[i].b_optional = true;
	reqs[i].defVal = "-10";

	i++;
	reqs[i].name = "maxfallspeed";
	reqs[i].label = "Maximum falling speed :";
	reqs[i].labelw = 150;
	reqs[i].SetPropType( PROPTYPE_NEGATIVENUMBER );
	reqs[i].defVal = "-10";

	i++;
	reqs[i].name = "maxtossspeed";
	reqs[i].label = "Maximum jumpheight :";
	reqs[i].labelw = 200;
	reqs[i].SetPropType( PROPTYPE_NUMBER );
	reqs[i].defVal = "1000";

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;

	i++;
	reqs[i].name = "noslow";
	reqs[i].label = "Game not slow down when beating a boss";
	reqs[i].SetPropType( PROPTYPE_BOOL );

	i++;
	reqs[i].name = "setweap";
	reqs[i].label = "Nb weapons at start";
	reqs[i].labelw = 200;
	reqs[i].SetPropType( PROPTYPE_NUMBER );


	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;


	i++;
	nb_props = i;
}

//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_STAGE_New::myWnd_STAGE_New(wxWindow *_parent )
:myWnd_LEVELSSTAGEProperties( _parent, NULL,  false, "New Stage Declaration Properties")
{
	wndSavingName = "STAGE_New";
	Init();
}



//****************************************************

myWnd_STAGE_New::~myWnd_STAGE_New()
{
}

//****************************************************

void myWnd_STAGE_New::Clicked_OK(bool b_close )
{
	myWnd_LEVELSSTAGEProperties::Clicked_OK(false);

	// Add the new stage at the correct position
	int ind = cbCtrl_where->GetSelection();
	if( ind == (int) nb_stage && curr_set != NULL )
	{
		curr_set->Add_SubObj( theObj );
	}
	else if( curr_set == NULL )
	{
		delete theObj;
		theObj = NULL;
		wxMessageBox( "No current SET selected !!\n\n", "BUG", wxOK | wxICON_EXCLAMATION, this );
	}
	else
	{
		stage_list[ind]->InsertObject_Before( theObj );
	}

	Cleanings();
	if( b_close )
		Close();
}


//****************************************************

void myWnd_STAGE_New::Clicked_Cancel(bool b_close)
{
	FrameEditProperties::Clicked_Cancel(false);
	Cleanings();
	if( b_close )
		Close();
}


//****************************************************

void myWnd_STAGE_New::Cleanings()
{
	myWnd_LEVELSSTAGEProperties::Cleanings();
}


//****************************************************

void myWnd_STAGE_New::InitControls()
{
	myWnd_LEVELSSTAGEProperties::InitControls();

	// Add the after which one control
	wxBoxSizer * sizer_temp = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText* t_stat = new wxStaticText( this, wxID_ANY, "Add it before the Stage : " );
	t_stat->SetMinSize( wxSize(270,  wxDefaultCoord) );
	sizer_temp->Add( t_stat, 0, wxALIGN_CENTER );

	// Build the list of Stages
	wxArrayString t;
	for( size_t i = 0; i < nb_stage; i++)
	{
		if( stage_list[i] != NULL )
			t.Add( stage_list[i]->GetStageName());
	}

	t.Add( "AT THE END" );

	// Build the list of Stages
	cbCtrl_where = new wxComboBox(this, wxID_ANY, ""
			, wxDefaultPosition, wxDefaultSize
			, t,
			wxCB_DROPDOWN | wxCB_READONLY );
	cbCtrl_where->SetSelection( nb_stage );
//	cbCtrl_temp->SetMinSize( wxSize(reqs[i].fieldw,  wxDefaultCoord) );
	sizer_temp->Add( cbCtrl_where );

	sizerTop->Add( sizer_temp, 0, wxTOP | wxBOTTOM, 20 );
}



//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_LoadedEnt::myWnd_LoadedEnt( wxWindow* _parent, ob_stage* _stage )
:wxDialog( _parent, wxID_ANY, "Loaded Entities for this stage", wxDefaultPosition, wxDefaultSize,
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
	wxBoxSizer *sizerWrapper = new wxBoxSizer( wxVERTICAL );
	wxPanel* thePanel = new wxPanel( this );
	sizerWrapper->Add( thePanel, 0, wxALL, 20 );

	wxBoxSizer *sizer_main = new wxBoxSizer( wxVERTICAL );
	thePanel->SetSizer( sizer_main );

	// Get the loaded
	size_t  nb_loads;
	ob_object** l_loads = _stage->GetSubObjectS( "LOAD", nb_loads );
	if( nb_loads == 0 )
	{
		wxStaticText* t_stat = new wxStaticText( this, wxID_ANY, "NO LOADED ENTITIES" );
		sizer_main->Add( t_stat );
	}
	else
	{
		wxArrayString sorted_list;
		for ( size_t i=0; i< nb_loads; i++)
		{
			sorted_list.Add( l_loads[i]->GetToken( 0 ) );
		}
		sorted_list.Sort();

		for( size_t i = 0; i < sorted_list.Count(); i++)
		{
			wxStaticText* t_stat = new wxStaticText( this, wxID_ANY, sorted_list[i] );
			sizer_main->Add( t_stat );
		}
	}

	if( l_loads != NULL )
		delete[] l_loads;
	SetSizer( sizerWrapper );
}


//****************************************************

myWnd_LoadedEnt::~myWnd_LoadedEnt()
{
}



//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_PanelProperties::myWnd_PanelProperties(wxWindow *_parent, ob_stage_panel* _stage, bool b_init )
:FrameEditProperties( _parent, NULL, "Panel properties" )
{
	theObj = _stage;

	b_newObj = (theObj == NULL);

	wndSavingName = "PanelProperties";
	if( b_init)
		Init();
}



//****************************************************

myWnd_PanelProperties::~myWnd_PanelProperties()
{
}

//****************************************************

void myWnd_PanelProperties::Clicked_OK(bool b_close )
{
	FrameEditProperties::Clicked_OK(false);

	wxString new_path_norm   = theObj->GetToken(0);
	if( new_path_norm == "" )
		new_path_norm = "none";
	wxString new_path_neon   = theObj->GetToken(1);
	if( new_path_neon == "" )
		new_path_neon = "none";
	wxString new_path_screen = theObj->GetToken(2);
	if( new_path_screen == "" )
		new_path_screen = "none";

	if( 	new_path_norm != path_norm
		||  new_path_neon != path_neon
		||  new_path_screen != path_screen )
		b_chg = true;
	else
		b_chg = false;

	theObj->SetToken( 0, new_path_norm );
	theObj->SetToken( 1, new_path_neon );
	theObj->SetToken( 2, new_path_screen );

	if( b_close )
		Close();
}


//****************************************************

void myWnd_PanelProperties::Clicked_Cancel(bool b_close)
{
	FrameEditProperties::Clicked_Cancel(false);
	if( b_close )
		Close();
}


//****************************************************

void myWnd_PanelProperties::InitObj()
{
	if( theObj == NULL )
	{
		theObj = new ob_stage_panel();
		theObj->SetToDefault();
		b_newObj = true;
	}

	path_norm   = theObj->GetToken(0);
	path_neon   = theObj->GetToken(1);
	path_screen = theObj->GetToken(2);
}


//****************************************************

void myWnd_PanelProperties::InitProps()
{
	int alloc = 30;
	reqs = new frameEdit_request[alloc];

	int i =-1;

	i++;
	reqs[i].name = "_TOKEN_0";
	reqs[i].label = "Panel image file";
	reqs[i].SetPropType( PROPTYPE_OBFILE );

	i++;
	reqs[i].name = "_TOKEN_1";
	reqs[i].label = "Neon file";
	reqs[i].SetPropType( PROPTYPE_OBFILE );
	reqs[i].nb_missing_tokens = 1;
	reqs[i].missing_tokens = new wxString[1];
	reqs[i].missing_tokens[0] = "none";

	i++;
	reqs[i].name = "_TOKEN_2";
	reqs[i].label = "Screen file";
	reqs[i].SetPropType( PROPTYPE_OBFILE );
	reqs[i].nb_missing_tokens = 2;
	reqs[i].missing_tokens = new wxString[2];
	reqs[i].missing_tokens[0] = "none";
	reqs[i].missing_tokens[1] = "none";

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;


	i++;
	nb_props = i;
}



//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_FrontPanelProperties::myWnd_FrontPanelProperties(wxWindow *_parent, ob_front_panel* _stage, bool b_init )
:FrameEditProperties( _parent, NULL, "FrontPanel properties" )
{
	theObj = _stage;
	b_newObj = (theObj == NULL);

	wndSavingName = "FrontPanelProperties";
	if( b_init)
		Init();
}



//****************************************************

myWnd_FrontPanelProperties::~myWnd_FrontPanelProperties()
{
}

//****************************************************

void myWnd_FrontPanelProperties::Clicked_OK(bool b_close )
{
	FrameEditProperties::Clicked_OK(false);

	wxString new_path = theObj->GetToken(0);
	if( new_path == "" )
		new_path = "none";

	if( 	new_path != path )
	{
		b_chg = true;
		theObj->SetToken( 0, new_path );
	}
	else
		b_chg = false;


	if( b_close )
		Close();
}


//****************************************************

void myWnd_FrontPanelProperties::Clicked_Cancel(bool b_close)
{
	FrameEditProperties::Clicked_Cancel(false);
	if( b_close )
		Close();
}


//****************************************************

void myWnd_FrontPanelProperties::InitObj()
{
	if( theObj == NULL )
	{
		theObj = new ob_stage_panel();
		theObj->SetName( "frontpanel" );
		b_newObj = true;
	}

	path = theObj->GetToken(0);
}


//****************************************************

void myWnd_FrontPanelProperties::InitProps()
{
	int alloc = 30;
	reqs = new frameEdit_request[alloc];

	int i =-1;

	i++;
	reqs[i].name = "_TOKEN_0";
	reqs[i].label = "FrontPanel image file";
	reqs[i].SetPropType( PROPTYPE_OBFILE );

	i++;
	nb_props = i;
}



//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************


myWnd_BGProperties::myWnd_BGProperties(wxWindow *_parent, ob_BG_Layer* _stage, bool _b_background, bool b_init )
:FrameEditProperties( _parent, NULL, "Background Layer properties" )
{
	theObj = _stage;

	b_newObj = (theObj == NULL);

	b_background = _b_background;
	wndSavingName = "BG_Properties";
	if( b_init)
		Init();
}



//****************************************************

myWnd_BGProperties::~myWnd_BGProperties()
{
}

//****************************************************

void myWnd_BGProperties::Clicked_OK(bool b_close )
{
	FrameEditProperties::Clicked_OK(false);
	if( theObj != NULL )
		if( theObj->GetToken( 0 ) == "" )
			theObj->SetToken( 0, "data" );

	if( b_close )
		Close();
}


//****************************************************

void myWnd_BGProperties::Clicked_Cancel(bool b_close)
{
	FrameEditProperties::Clicked_Cancel(false);
	if( b_close )
		Close();
}


//****************************************************

void myWnd_BGProperties::InitObj()
{
	if( theObj == NULL )
	{
		theObj = new ob_BG_Layer();
		theObj->SetToDefault();
		if( b_background == true )
			theObj->SetName( "background" );
		else
			theObj->SetName( "bglayer" );
		b_newObj = true;
	}
	else
		((ob_BG_Layer*)theObj)->FillMissings();
}


//****************************************************

void myWnd_BGProperties::InitProps()
{
	int alloc = 30;
	reqs = new frameEdit_request[alloc];

	int i =-1;

	i++;
	reqs[i].name = "_TOKEN_0";
	reqs[i].label = "Background image file";
	reqs[i].SetPropType( PROPTYPE_OBFILE );

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;


	i++;
	reqs[i].name = "_TOKEN_1";
	reqs[i].label = "xratio";
	reqs[i].SetPropType( PROPTYPE_FLOAT );
	reqs[i].nb_missing_tokens = 1;
	reqs[i].missing_tokens = new wxString[1];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].defVal = "0.5";

	i++;
	reqs[i].name = "_TOKEN_2";
	reqs[i].label = "zratio";
	reqs[i].SetPropType( PROPTYPE_FLOAT );
	reqs[i].nb_missing_tokens = 2;
	reqs[i].missing_tokens = new wxString[2];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].missing_tokens[1] = "0.5";
	reqs[i].defVal = "0.5";

	i++;
	reqs[i].name = "15";
	//reqs[i].label = "Custom  Selection screen file : ";
	reqs[i].SetPropType( PROPTYPE_SPACER);
	// reqs[i].enums = ;


	i++;
	reqs[i].name = "_TOKEN_3";
	reqs[i].label = "xpos";
	reqs[i].SetPropType( PROPTYPE_NUMBER );
	reqs[i].nb_missing_tokens = 3;
	reqs[i].missing_tokens = new wxString[3];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].missing_tokens[1] = "0.5";
	reqs[i].missing_tokens[2] = "0.5";
	reqs[i].defVal = "0";

	i++;
	reqs[i].name = "_TOKEN_4";
	reqs[i].label = "zpos";
	reqs[i].SetPropType( PROPTYPE_NUMBER );
	reqs[i].nb_missing_tokens = 4;
	reqs[i].missing_tokens = new wxString[4];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].missing_tokens[1] = "0.5";
	reqs[i].missing_tokens[2] = "0.5";
	reqs[i].missing_tokens[3] = "0";
	reqs[i].defVal = "0";


	i++;
	reqs[i].name = "_TOKEN_5";
	reqs[i].label = "xspace";
	reqs[i].SetPropType( PROPTYPE_NUMBER );
	reqs[i].nb_missing_tokens = 5;
	reqs[i].missing_tokens = new wxString[5];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].missing_tokens[1] = "0.5";
	reqs[i].missing_tokens[2] = "0.5";
	reqs[i].missing_tokens[3] = "0";
	reqs[i].missing_tokens[4] = "0";
	reqs[i].defVal = "0";


	i++;
	reqs[i].name = "_TOKEN_6";
	reqs[i].label = "zspace";
	reqs[i].SetPropType( PROPTYPE_NUMBER );
	reqs[i].nb_missing_tokens = 6;
	reqs[i].missing_tokens = new wxString[6];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].missing_tokens[1] = "0.5";
	reqs[i].missing_tokens[2] = "0.5";
	reqs[i].missing_tokens[3] = "0";
	reqs[i].missing_tokens[4] = "0";
	reqs[i].missing_tokens[5] = "0";
	reqs[i].defVal = "0";


	i++;
	reqs[i].name = "_TOKEN_7";
	reqs[i].label = "xrepeat";
	reqs[i].SetPropType( PROPTYPE_NUMBER );
	reqs[i].nb_missing_tokens = 7;
	reqs[i].missing_tokens = new wxString[7];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].missing_tokens[1] = "0.5";
	reqs[i].missing_tokens[2] = "0.5";
	reqs[i].missing_tokens[3] = "0";
	reqs[i].missing_tokens[4] = "0";
	reqs[i].missing_tokens[5] = "0";
	reqs[i].missing_tokens[6] = "0";
	reqs[i].defVal = "5000";

	i++;
	reqs[i].name = "_TOKEN_8";
	reqs[i].label = "zrepeat";
	reqs[i].SetPropType( PROPTYPE_NUMBER );
	reqs[i].nb_missing_tokens = 8;
	reqs[i].missing_tokens = new wxString[8];
	reqs[i].missing_tokens[0] = "data";
	reqs[i].missing_tokens[1] = "0.5";
	reqs[i].missing_tokens[2] = "0.5";
	reqs[i].missing_tokens[3] = "0";
	reqs[i].missing_tokens[4] = "0";
	reqs[i].missing_tokens[5] = "0";
	reqs[i].missing_tokens[6] = "0";
	reqs[i].missing_tokens[7] = "5000";
	reqs[i].defVal = "5000";


	if( ! b_background )
	{
		i++;
		reqs[i].name = "15";
		//reqs[i].label = "Custom  Selection screen file : ";
		reqs[i].SetPropType( PROPTYPE_SPACER);
		// reqs[i].enums = ;


		i++;
		reqs[i].name = "_TOKEN_9";
		reqs[i].label = "use transparency";
		reqs[i].SetPropType( PROPTYPE_BOOL );
		reqs[i].nb_missing_tokens = 9;
		reqs[i].missing_tokens = new wxString[9];
		reqs[i].missing_tokens[0] = "data";
		reqs[i].missing_tokens[1] = "0.5";
		reqs[i].missing_tokens[2] = "0.5";
		reqs[i].missing_tokens[3] = "0";
		reqs[i].missing_tokens[4] = "0";
		reqs[i].missing_tokens[5] = "0";
		reqs[i].missing_tokens[6] = "0";
		reqs[i].missing_tokens[7] = "5000";
		reqs[i].missing_tokens[8] = "5000";
		reqs[i].b_optional = true;

		i++;
		reqs[i].name = "_TOKEN_10";
		reqs[i].label = "alpha";
		reqs[i].SetPropType( PROPTYPE_ENUMS);
		wxString t0[] = { "0", "1", "2", "3", "4", "5", "6" };
		TabStringToArrayString( t0, 7, reqs[i].enums );
		reqs[i].nb_missing_tokens = 10;
		reqs[i].missing_tokens = new wxString[10];
		reqs[i].missing_tokens[0] = "data";
		reqs[i].missing_tokens[1] = "0.5";
		reqs[i].missing_tokens[2] = "0.5";
		reqs[i].missing_tokens[3] = "0";
		reqs[i].missing_tokens[4] = "0";
		reqs[i].missing_tokens[5] = "0";
		reqs[i].missing_tokens[6] = "0";
		reqs[i].missing_tokens[7] = "5000";
		reqs[i].missing_tokens[8] = "5000";
		reqs[i].missing_tokens[9] = "0";
		reqs[i].b_optional = true;

		i++;
		reqs[i].name = "_TOKEN_11";
		reqs[i].label = "watermode";
		reqs[i].SetPropType( PROPTYPE_NUMBER );
		reqs[i].nb_missing_tokens = 11;
		reqs[i].missing_tokens = new wxString[reqs[i].nb_missing_tokens];
		reqs[i].missing_tokens[0] = "data";
		reqs[i].missing_tokens[1] = "0.5";
		reqs[i].missing_tokens[2] = "0.5";
		reqs[i].missing_tokens[3] = "0";
		reqs[i].missing_tokens[4] = "0";
		reqs[i].missing_tokens[5] = "0";
		reqs[i].missing_tokens[6] = "0";
		reqs[i].missing_tokens[7] = "5000";
		reqs[i].missing_tokens[8] = "5000";
		reqs[i].missing_tokens[9] = "0";
		reqs[i].missing_tokens[10] = "0";
		reqs[i].b_optional = true;

		i++;
		reqs[i].name = "_TOKEN_12";
		reqs[i].label = "amplitude";
		reqs[i].SetPropType( PROPTYPE_NUMBER );
		reqs[i].nb_missing_tokens = 12;
		reqs[i].missing_tokens = new wxString[reqs[i].nb_missing_tokens];
		reqs[i].missing_tokens[0] = "data";
		reqs[i].missing_tokens[1] = "0.5";
		reqs[i].missing_tokens[2] = "0.5";
		reqs[i].missing_tokens[3] = "0";
		reqs[i].missing_tokens[4] = "0";
		reqs[i].missing_tokens[5] = "0";
		reqs[i].missing_tokens[6] = "0";
		reqs[i].missing_tokens[7] = "5000";
		reqs[i].missing_tokens[8] = "5000";
		reqs[i].missing_tokens[9] = "0";
		reqs[i].missing_tokens[10] = "0";
		reqs[i].missing_tokens[11] = "0";
		reqs[i].b_optional = true;

		i++;
		reqs[i].name = "_TOKEN_13";
		reqs[i].label = "wavelength";
		reqs[i].SetPropType( PROPTYPE_NUMBER );
		reqs[i].nb_missing_tokens = 13;
		reqs[i].missing_tokens = new wxString[reqs[i].nb_missing_tokens];
		reqs[i].missing_tokens[0] = "data";
		reqs[i].missing_tokens[1] = "0.5";
		reqs[i].missing_tokens[2] = "0.5";
		reqs[i].missing_tokens[3] = "0";
		reqs[i].missing_tokens[4] = "0";
		reqs[i].missing_tokens[5] = "0";
		reqs[i].missing_tokens[6] = "0";
		reqs[i].missing_tokens[7] = "5000";
		reqs[i].missing_tokens[8] = "5000";
		reqs[i].missing_tokens[9] = "0";
		reqs[i].missing_tokens[10] = "0";
		reqs[i].missing_tokens[11] = "0";
		reqs[i].missing_tokens[12] = "0";
		reqs[i].b_optional = true;

		i++;
		reqs[i].name = "_TOKEN_14";
		reqs[i].label = "wavespeed";
		reqs[i].SetPropType( PROPTYPE_NUMBER );
		reqs[i].nb_missing_tokens = 14;
		reqs[i].missing_tokens = new wxString[reqs[i].nb_missing_tokens];
		reqs[i].missing_tokens[0] = "data";
		reqs[i].missing_tokens[1] = "0.5";
		reqs[i].missing_tokens[2] = "0.5";
		reqs[i].missing_tokens[3] = "0";
		reqs[i].missing_tokens[4] = "0";
		reqs[i].missing_tokens[5] = "0";
		reqs[i].missing_tokens[6] = "0";
		reqs[i].missing_tokens[7] = "5000";
		reqs[i].missing_tokens[8] = "5000";
		reqs[i].missing_tokens[9] = "0";
		reqs[i].missing_tokens[10] = "0";
		reqs[i].missing_tokens[11] = "0";
		reqs[i].missing_tokens[12] = "0";
		reqs[i].missing_tokens[13] = "0";
		reqs[i].b_optional = true;
		
		i++;
		reqs[i].name = "_TOKEN_14";
		reqs[i].label = "bgspeedratio";
		reqs[i].SetPropType( PROPTYPE_NUMBER );
		reqs[i].nb_missing_tokens = 15;
		reqs[i].missing_tokens = new wxString[reqs[i].nb_missing_tokens];
		reqs[i].missing_tokens[0] = "data";
		reqs[i].missing_tokens[1] = "0.5";
		reqs[i].missing_tokens[2] = "0.5";
		reqs[i].missing_tokens[3] = "0";
		reqs[i].missing_tokens[4] = "0";
		reqs[i].missing_tokens[5] = "0";
		reqs[i].missing_tokens[6] = "0";
		reqs[i].missing_tokens[7] = "5000";
		reqs[i].missing_tokens[8] = "5000";
		reqs[i].missing_tokens[9] = "0";
		reqs[i].missing_tokens[10] = "0";
		reqs[i].missing_tokens[11] = "0";
		reqs[i].missing_tokens[12] = "0";
		reqs[i].missing_tokens[13] = "0";
		reqs[i].missing_tokens[14] = "0";
		reqs[i].b_optional = true;
	}

	i++;
	nb_props = i;
}






//****************************************************
//****************************************************
//****************************************************

myWnd_SceneTest::myWnd_SceneTest(
			  wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			)
:wxDialog( _parent, wxID_ANY, "Scene Tests" , wxDefaultPosition, wxDefaultSize,
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
	// Launch the Window with Ctrl_SceneView
	sceneView = new Panel_SceneView( this
			, _stage, _stage_declaration, _stage_set );
	
	SetMinSize( wxSize( 250,250) );
  	Show();
	Frame_RestorePrevCoord( this, wxT("testScene"));
}

//****************************************************
myWnd_SceneTest::~myWnd_SceneTest()
{
};

//****************************************************
void 
myWnd_SceneTest::Set_Panels( ob_stage_panel**& p_panels, size_t& p_nb_panels )
{
	sceneView->Set_Panels( p_panels, p_nb_panels );
	sceneView->Update_View();
}

//****************************************************
void 
myWnd_SceneTest::Set_Background( ob_BG_Layer**& p_bg_layers, size_t& p_nb_layers )
{
	sceneView->Set_Background( p_bg_layers, p_nb_layers );
	sceneView->Update_View();
}

//****************************************************
void 
myWnd_SceneTest::Set_Front_Panels(ob_front_panel**& p_front_panels, size_t& p_nb_front_panels )
{
	sceneView->Set_Front_Panels( p_front_panels, p_nb_front_panels );
	sceneView->Update_View();
}

//****************************************************
void
myWnd_SceneTest::EvtClose( wxCloseEvent& event )
{
	Frame_SaveCoord( this, wxT("testScene"));
	event.Skip();
}


// Event table
BEGIN_EVENT_TABLE(myWnd_SceneTest, wxDialog)
	EVT_CLOSE(myWnd_SceneTest::EvtClose)
END_EVENT_TABLE()
