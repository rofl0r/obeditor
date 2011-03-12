/*
 * le_main.cpp
 *
 *  Created on: 27 févr. 2009
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/busyinfo.h>

#include "common__mod.h"
#include "common__ob_controls.h"

#include "ob_editor.h"
#include "levels__globals.h"
#include "levels__classes.h"
#include "levels__sidesWindows.h"
#include "common__object_stage.h"
#include "levels__Ctrl_SceneView.h"
#include "levels__wndEditBackground.h"
#include "levels__wndEditFrontPanels.h"
#include "levels__Panel_StageView.h"

extern LevelEditorFrame *leFrame;

using namespace std;

#define D_MAX_INTER_MERGE_DELAY 1000

//****************************************************


bool FileLevels_Reload( wxWindow* _parent );

ob_StagesSet** levels_sets;
size_t nb_set;
ob_StagesSet* curr_set;

ob_StageDeclaration** stage_list;
size_t nb_stage;
ob_StageDeclaration* curr_stage_declaration;
obFileStage* curr_stageFile;

int ob_screen_w = 320;
int ob_screen_h = 240;

//****************************************************
// *** DIMENSIONS ***

#define LISTBOX_MAXWIDTH 180
#define LISTBOX_MINWIDTH 80
#define BOTTOM_PART_HEIGHT 150


//****************************************************
bool 
FileLevels_Reload( wxWindow* _parent )
{
	if( curr_mod == NULL )
		return false;
	
	wxFileName levels_path(dataDirPath);
	levels_path.AppendDir(dataDirPath.GetFullName());
	levels_path.SetFullName(curr_mod->levels_txt);
	wxString t = levels_path.GetFullPath();

	if( fileLevels != NULL )
		delete fileLevels;
	fileLevels = new obFileLevels(levels_path);

	if( fileLevels->obj_container == NULL )
	{
		wxMessageBox( wxT("The selected project does not contain a valid 'levels.txt' file\n\n") +
						levels_path.GetFullPath()
						, wxT("Problem"), wxOK | wxICON_INFORMATION, _parent );
		delete fileLevels;
		fileLevels = NULL;
		return false;
	}
	return true;
}

//****************************************************
void
Init_Filter_Type_List( wxComboBox* comb_ctrl )
{
	comb_ctrl->Append( wxT("ALL") );
	
	int arr_types_filter[] =
			{
				SOBJ_OBSTACLE,
				SOBJ_NPC,
				SOBJ_ENEMY,
				SOBJ_NONE,
				SOBJ_ENDLEVEL,
				SOBJ_STEAMER,
				SOBJ_PANEL,
				SOBJ_TEXT,
				SOBJ_TRAP,
				SOBJ_SHOT,
				SOBJ_ITEM,
			};
	for( size_t i =0; i < sizeof(arr_types_filter)/sizeof(int);i++)
		comb_ctrl->Append( Stage__Type_ToStr(arr_types_filter[i]) );
}

//****************************************************
int 
LEVEL__GetCurrInd()
{
	if( levels_sets == NULL )
		return -1;
	if( curr_set == NULL )
		return -1;
	for( size_t i = 0; i < nb_set; i++ )
		if( levels_sets[i] == curr_set )
			return i;
	return -1;
}


//****************************************************

int STAGE__GetCurrInd()
{
	if( stage_list == NULL )
		return -1;
	if( curr_stage_declaration == NULL )
		return -1;
	for( size_t i = 0; i < nb_stage; i++ )
		if( stage_list[i] == curr_stage_declaration )
			return i;
	return -1;
}


//****************************************************

LevelEditorFrame::LevelEditorFrame(wxWindow *_parent, const wxString& title)
: wxFrame( _parent, -1, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
	//*********************************
	// Init the Handlers of the History manager
	theHistoryManager.Set_CallBacks( 
					  &level__onUndo
					, &level__onDo
					, &level__onEltAdd
					, &level__onNoMoreUndo
					, &level__onNoMoreDo
					, &level__onDeleteHistory_data
					, &level__onAfterEltAdded
				);
	
	//*********************************
	// Init variables

	leFrame = this; // Needed for subWindow thar refer to it
	mainPanel = new wxPanel( this );
	wxBoxSizer *sizer_mainPanel = new wxBoxSizer( wxVERTICAL ); // );
	sizer_mainPanel->Add( mainPanel, 1, wxEXPAND );
	baseTitle = title;

	m_flags = 0;

	helpWin = NULL;
	
	levels_sets = NULL;
	nb_set = 0;
	curr_set = NULL;

	stage_list = NULL;
	nb_stage = 0;
	curr_stage_declaration = NULL;
	curr_stageFile = NULL;
	
	TYPE__curr_selected = STAGETYPE_NONE;
	b_closing = false;

	arr_entities__size = 0;
	arr_entities = NULL;
	last_txt_filter = wxString();

	if( curr_mod != NULL )
	{
		ob_screen_w = curr_mod->video_res.x;
		ob_screen_h = curr_mod->video_res.y;
	}
	else
	{
		ob_screen_w = 320;
		ob_screen_h = 240;
	}
	
	//*********************************
	// SIZERS
	//*********************************

	// Main Sizer
	wxBoxSizer *sizer_main = new wxBoxSizer( wxVERTICAL ); // );

		// Sizer Top
		wxBoxSizer *sizer_top = new wxBoxSizer( wxHORIZONTAL );

			wxBoxSizer *sizer_view = new wxBoxSizer( wxVERTICAL );
			sizer_top->Add( sizer_view, 1, wxEXPAND );

			// Sizer Top-Right
			wxBoxSizer *sizer_topright = new wxBoxSizer( wxVERTICAL );
			sizer_topright->SetMinSize( wxSize(LISTBOX_MINWIDTH, wxDefaultCoord) );
			sizer_top->Add( sizer_topright, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);

		sizer_main->Add( sizer_top, 1, wxEXPAND );

		// Bottom Sizer
		sizer_bottom = new wxBoxSizer( wxHORIZONTAL );
		sizer_bottom->SetMinSize( wxDefaultCoord, BOTTOM_PART_HEIGHT );
		sizer_main->Add( sizer_bottom, 0, wxEXPAND );


	//**************************
	// SET THE MAIN PANELS
	//**************************

		//--------------------------------
		// VIEW PANEL
		stageView = new Panel_StageView( mainPanel );
		sizer_view->Add( stageView, 1, wxEXPAND );

		//--------------------------------
		// Objects Filter and list

		// Label
		sizer_topright->Add( 
		new wxStaticText(mainPanel, wxID_ANY, wxT("Entities"))
					,0 ,wxALIGN_CENTER | wxTOP | wxBOTTOM, 5 
					);

		// Text Entry Filter
		entities_filter = new wxTextCtrl(mainPanel, wxID_ANY
				, wxString(), wxDefaultPosition, wxDefaultSize
				, wxSUNKEN_BORDER );
		entities_filter->Connect(
		    wxEVT_KEY_UP
		  , wxKeyEventHandler(LevelEditorFrame::Evt_EntitiesListTxtChg)
		  , NULL, this);
		sizer_topright->Add( entities_filter, 0, wxEXPAND  | wxBOTTOM, 10 );
		
		// Combo Boxes Filter
		cbbox_entities_type = new wxComboBox(mainPanel, wxID_ANY, wxT("ALL"),
									wxDefaultPosition, wxDefaultSize,
									0, NULL,
									wxCB_DROPDOWN | wxCB_READONLY );
		cbbox_entities_type->Connect( 
		    wxEVT_COMMAND_COMBOBOX_SELECTED
		  , wxCommandEventHandler(LevelEditorFrame::Evt_EntitiesListTypeChg)
		  , NULL, this);
		Init_Filter_Type_List( cbbox_entities_type );
//		cbbox_entities_type->SetMinSize(  wxSize( 80, wxDefaultCoord ));
//		cbbox_entities_type->SetMaxSize(  wxSize( 80, wxDefaultCoord ));
		sizer_topright->Add( cbbox_entities_type );


		// Entities list
		entities_list = new prListBox( mainPanel, wxID_ANY
			, LISTBOX_MAXWIDTH, wxLB_SORT );

		// On selection change -> show the avatar image
		entities_list->Connect( wxEVT_COMMAND_LISTBOX_SELECTED
			, wxCommandEventHandler(LevelEditorFrame::Evt_EntitySelChg)
			, NULL, this);
		// On Mouse Events
		entities_list->Connect( 
		    wxEVT_RIGHT_UP
		  , wxMouseEventHandler(LevelEditorFrame::Evt_EntitiesListRightClik)
		  , NULL, this);
		// On Add object event popmenu
		entities_list->Connect( 
		    wxEVT_COMMAND_MENU_SELECTED
		  , wxCommandEventHandler(LevelEditorFrame::Evt_EntitiesAddOne)
		  , NULL, this);
		

		sizer_topright->Add( entities_list, 1, wxALIGN_CENTER | wxEXPAND);

		// Entity img
		entity_img = new MyImageCtrl( mainPanel );
//		entity_img->SetSize( wxSize(wxDefaultCoord, SCENE_MAP_HEIGHT) );
		entity_img->SetMinSize( wxSize(LISTBOX_MAXWIDTH, SCENE_MAP_HEIGHT) );
		entity_img->Set_BoundDimensions( 
				 LISTBOX_MAXWIDTH, SCENE_MAP_HEIGHT
				,LISTBOX_MAXWIDTH, SCENE_MAP_HEIGHT );
		entity_img->SetImage( noImg );
		sizer_topright->Add( entity_img, 0, wxALIGN_CENTER | wxFIXED_MINSIZE );

		
		//--------------------------------
		// SUPER TYPES LIST
		object_SuperType_list = 
			new prListBox( mainPanel, wxID_ANY, LISTBOX_MAXWIDTH
					, wxLB_SINGLE| wxLB_SORT|wxLB_NEEDED_SB);
		for( size_t i = 0; i <= SST_MAX; i++ )
		{
			object_SuperType_list->Append( wxT("  ") + Stage__SuperType_ToStr(i) , (void*) i );
		}
		object_SuperType_list->Connect( wxEVT_COMMAND_LISTBOX_SELECTED
		  , wxCommandEventHandler(LevelEditorFrame::Evt_SuperType_Selection_Change )
		  , NULL, this);
		object_SuperType_list->Connect( wxEVT_LEFT_DCLICK
		  , wxMouseEventHandler(LevelEditorFrame::Evt_SuperType_DblClick)
		  , NULL, this);
		
		  wxStaticBoxSizer *sizer_object_type_list = new wxStaticBoxSizer( wxVERTICAL, mainPanel, wxT("Super Types" ));
		sizer_object_type_list->SetMinSize( wxSize(LISTBOX_MINWIDTH, wxDefaultCoord) );
		sizer_object_type_list->Add( object_SuperType_list, 1, 0 );
		sizer_bottom->Add( sizer_object_type_list, 0, wxEXPAND );

		//--------------------------------
		// TYPES LIST
		object_Type_list = new prListBox( mainPanel, wxID_ANY, LISTBOX_MAXWIDTH
					, wxLB_SINGLE| wxLB_SORT|wxLB_NEEDED_SB);
		object_Type_list->Connect( wxEVT_COMMAND_LISTBOX_SELECTED
			, wxCommandEventHandler(LevelEditorFrame::Evt_Type_Selection_Change )
			, NULL, this);
		object_Type_list->Connect( wxEVT_LEFT_DCLICK
		  , wxMouseEventHandler(LevelEditorFrame::Evt_Type_DblClick)
		  , NULL, this);

		  sizer_object_type_list = new wxStaticBoxSizer( wxVERTICAL, mainPanel, wxT("Objects") );
		sizer_object_type_list->SetMinSize( wxSize(LISTBOX_MINWIDTH, wxDefaultCoord) );
		sizer_object_type_list->Add( object_Type_list, 1, 0 );
		sizer_bottom->Add( sizer_object_type_list, 0, wxEXPAND );

		// On Mouse Events
		object_Type_list->Connect( 
		    wxEVT_RIGHT_UP
		  , wxMouseEventHandler(LevelEditorFrame::Evt_TYPEListRightClik)
		  , NULL, this);
		// On Add object event popmenu
		object_Type_list->Connect( 
		    wxEVT_COMMAND_MENU_SELECTED
		  , wxCommandEventHandler(LevelEditorFrame::Evt_TYPEListAddOne)
		  , NULL, this);
		
		//--------------------------------
		// OBJECT LIST
		objects_List = new prListBox( mainPanel, wxID_ANY, LISTBOX_MAXWIDTH );

		objects_List->Connect( wxEVT_COMMAND_LISTBOX_SELECTED
			, wxCommandEventHandler(LevelEditorFrame::Evt_ObjectList_Selection_Change )
			, NULL, this);

			wxStaticBoxSizer *sizer_object_list = new wxStaticBoxSizer( wxVERTICAL, mainPanel, wxT("Objects") );
		sizer_object_list->SetMinSize( wxSize(LISTBOX_MINWIDTH, wxDefaultCoord) );
		sizer_object_list->Add( objects_List, 1, wxEXPAND );
		sizer_bottom->Add( sizer_object_list, 0, wxEXPAND );

		//--------------------------------
		// NOTEBOOK PANEL
		onglets = new wxNotebook( mainPanel, wxID_ANY );
		sizer_bottom->Add( onglets, 1, wxEXPAND );



	//**************************
	// SET THE MENU BAR
	//**************************

    menuBar = new wxMenuBar;
    wxMenu *subMenu;

    wxMenu *menuLoadSave = new wxMenu;
    menuLoadSave->Append( ID_STAGE_RELOAD,	   wxT("Reload the stage\tF5") );
    menuLoadSave->Append( ID_STAGE_RELOAD_ALL, wxT("Reload ALL" ));
    menuLoadSave->AppendSeparator();
    menuLoadSave->Append( ID_STAGE_SAVE,	 wxT("Save the stage\tF2" ));
    menuLoadSave->Append( ID_LEVELSTXT_SAVE, wxT("Save LEVELS.TXT\tSHIFT-F2") );
    menuLoadSave->Append( ID_STAGE_SAVE_ALL, wxT("Save ALL" ));
    menuLoadSave->AppendSeparator();
    menuLoadSave->Append( ID_OPEN_IN_EDITOR, wxT("Open Stage in External Editor\tF3" ));
    menuLoadSave->Append( ID_OPEN_LEVELSTXT_IN_EDITOR, wxT("Open levels.txt in External Editor\tSHIFT-F3" ));
    menuLoadSave->AppendSeparator();
    menuLoadSave->Append( ID_BACK_TO_STARTFRAME,	    wxT("Quit to Mods Window" ));
    menuBar->Append( menuLoadSave, wxT("Load/Save" ));

    menuSets = new wxMenu;
    menuSets->Append( ID_SET_NEW,	        wxT("Add new SET" ));
    menuSets->Append( ID_SET_EDIT_SETTINGS, wxT("SET Properties" ));
    menuSets->Append( ID_SET_EDIT_EXTENDED_PROPS, wxT("SET Extended Properties" ));
    menuSets->AppendSeparator();
		subMenu = new wxMenu;
		subMenu->Append( ID_SET_LIST_START, wxT("<SET LIST>" ));
		menuSets->Append( new wxMenuItem(menuSets, ID_SET_LIST, wxT("Change current SET"), wxString(), wxITEM_NORMAL,subMenu ));
		menuBar->Append( menuSets, wxT("| Edit SETs") );

    menuStage = new wxMenu;
    menuStage->Append( ID_STAGE_NEW,	        wxT("Add new STAGE" ));
    menuStage->Append( ID_STAGE_EDIT_DECLARE_SETTINGS, wxT("STAGE Declaration Properties" ));
    menuStage->Append( ID_STAGE_EDIT_SETTINGS, wxT("STAGE Properties" ));

    menuStage->Append( ID_STAGE_EDIT_EXTENDED_PROPS, wxT("STAGE Extended Properties" ));
    menuStage->AppendSeparator();
		subMenu = new wxMenu;
		subMenu->Append( ID_STAGE_LIST_START, wxT("<STAGE LIST>" ));
		menuStage->Append( new wxMenuItem(menuStage, ID_STAGE_LIST, wxT("Change current STAGE"), wxString(), wxITEM_NORMAL,subMenu ));
    menuStage->AppendSeparator();
    menuStage->Append( ID_STAGE_LOADED_ENTITIES, wxT("Loaded Entities" ));
    menuBar->Append( menuStage, wxT("| Edit STAGEs" ));

    menuScene = new wxMenu;
    menuScene->Append( ID_BACKGROUND,	     wxT("Background" ));
    menuScene->Append( ID_SCENE_BG_EDIT_ORDER, wxT("Bg Layers" ));
		subMenu = new wxMenu;
		subMenu->Append( ID_SCENE_PANELS_EDIT_ORDER, wxT("ReOrder" ));
		subMenu->Append( ID_SCENE_PANELS_ADD, wxT("Add a new one" ));
		subMenu->AppendSeparator();
		subMenu->Append( ID_SCENE_PANELS_LIST_START, wxT("<PANELS LIST>" ));
		menuScene->Append( new wxMenuItem(menuScene, ID_SCENE_MENU_PANELS, wxT("Panels"), wxString(), wxITEM_NORMAL,subMenu ));
		subMenu = new wxMenu;
		subMenu->Append( ID_SCENE_FRONTPANEL_ORDER, wxT("Edit" ));
		subMenu->AppendSeparator();
		subMenu->Append( ID_SCENE_FRONTPANEL_LIST_START, wxT("<Front Panel list>" ));
		menuScene->Append( new wxMenuItem(menuScene, ID_SCENE_FRONTPANEL, wxT("Front Panels"), wxString(), wxITEM_NORMAL,subMenu ));

    menuScene->AppendSeparator();
    menuScene->Append( ID_SCENE_TEST, wxT("Test Scene" ));
    menuBar->Append( menuScene, wxT("| Edit Scene" ));

    menuObjects = new wxMenu;
    menuObjects->Append( ID_OBJECT_COPY,	   wxT("Duplicate\tCTRL-D" ));
    menuObjects->Append( ID_OBJECT_SUPP,       wxT("Delete\tCTRL-DEL" ));
    menuObjects->Append( ID_OBJECT_PROPERTIES, wxT("Properties\tALT-P" ));
//    menuObjects->Append( ID_OBJECT_EDIT_EXTENDED_PROPS, "Extended Properties" );
    menuObjects->AppendSeparator();
    menuObjects->Append( ID_OBJECT_SORT_AT_OBJECTS, wxT("Sort Objects with\"AT\"" ));
    menuBar->Append( menuObjects, wxT("| Edit Object" ));

    menuUnRedo = new wxMenu;
    menuUnRedo->Append( ID_UNDO, wxT("Undo\tCTRL-Z" ))->Enable(false);
    menuUnRedo->Append( ID_REDO, wxT("Redo\tCTRL-SHIFT-Z" ))->Enable(false);
    menuBar->Append( menuUnRedo, wxT("| History" ));
    
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append( ID_HELP_PLEASE, wxT("Manual" ));
    menuBar->Append( menuHelp, wxT("| ?" ));

    SetMenuBar( menuBar );

    //**************************
    // Popups Menus
    menuAdd = new wxMenu();
    menuAdd->Append( wxID_NEW, wxT("Add One" ));
    
    //**************************
    // FINISHING THE WIDGETS
    //**************************
    mainPanel->SetSizer( sizer_main );
    SetSizer( sizer_mainPanel );

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
    //**************************
    // INIT STATIC STUFFS
    //**************************
    ENTITIES__Load();
    ENTITIES__FILTER__Apply();
    NOTEBOOK__Init();
    curr_associated_obj = NULL;
    NOTEBOOK__Set_NoSelection(true);
    
    //**************************
    // LOAD THINGS
    //**************************
    ReloadAll(true, false );

    // Select the previous session set and stage
    SET__Select_LastSession();
    STAGE__Select_LastSession();

    Show();
    Frame_RestorePrevCoord( this, wxT("editstage_frame"));
}


//****************************************************

LevelEditorFrame::~LevelEditorFrame()
{
	theHistoryManager.Force_Disable();
 	theHistoryManager.Reset();
	
	// UN-INIT STATIC STUFFS
	ENTITIES__UnLoad();
	obFileStage::ClearEntitiesCache();
	NOTEBOOK__UnInit();
	delete menuAdd;
}


//****************************************************
// END CONSTRUCTOR
//****************************************************

//****************************************************

void LevelEditorFrame::EvtClose( wxCloseEvent& event)
{
	// Clear the TAB view
	NOTEBOOK__Set_NoSelection();
	
	// Make the user sure
	wxString modifs = GetModifsDone();
	if( modifs != wxString() )
	{
		int _reponse = wxMessageBox( wxT("Modification made to this files will be loose :\n")
		+ modifs + wxT("\n\n")
		+ wxT("Do you still want to quit ?")
		, wxT("ATTENTION !")
				, wxICON_EXCLAMATION | wxYES_NO, leFrame );

		if( _reponse != wxYES )
			return;

		modificationList.Clear();
	}
	
	b_closing = true;

	// Save Some params
	OBJECT__Visibilities_SaveState();
	stageView->SaveState();
	
	// Save wnd coords
	Frame_SaveCoord( this, wxT("editstage_frame"));

	// Save to autoload this frame
	config->Write(wxT("/autoload/frame"), wxT("edit_stage") );

	startFrame->b_reset_leFrame = true;
	startFrame->Show();
	event.Skip();
}


//****************************************************
void 
LevelEditorFrame::EvtActivate( wxActivateEvent& evt )
{
	// hide the StartFrame
	startFrame->Hide();
	Layout();
	evt.Skip();
}


//****************************************************

void LevelEditorFrame::EvtResize( wxSizeEvent& event )
{
	event.Skip();
}


//****************************************************

void LevelEditorFrame::Evt_SET__Select( wxCommandEvent& evt_menu )
{
	int ind_menu = evt_menu.GetId() - ID_SET_LIST_START;

	if( levels_sets == NULL )
	{
		wxMessageBox(wxT( "A SET has been selected in the menu although no sets are loaded !!"), wxT("BUG BUG !") );
		return;
	}
	else if( ind_menu < 0 || ind_menu >= (int) nb_set )
	{
		wxMessageBox( wxT("The founded ind for the SET selected in the menu si out of range : <") +IntToStr(ind_menu) + wxT(" !!")
		, wxT("BUG BUG !") );
		return;
	}

	wxString SETName = levels_sets[ind_menu]->GetSetName();
	if( curr_set == NULL || curr_set->name != SETName )
		SET__Select( SETName );

	return;
}


//****************************************************

void LevelEditorFrame::Evt_STAGE__Select( wxCommandEvent& evt_menu )
{
	int ind_menu = evt_menu.GetId() - ID_STAGE_LIST_START;

	if( stage_list == NULL )
	{
		wxMessageBox( wxT("A STAGE has been selected in the menu although no sets are loaded !!"), wxT("BUG BUG !") );
		return;
	}
	else if( ind_menu < 0 || ind_menu >= (int) nb_stage )
	{
		wxMessageBox( wxT("The founded ind for the STAGE selected in the menu si out of range : <") +IntToStr(ind_menu) + wxT(" !!")
		, wxT("BUG BUG !") );
		return;
	}

	wxString StageName = stage_list[ind_menu]->GetStageName();
	if( curr_stage_declaration == NULL || curr_stage_declaration->GetStageName() != StageName )
		STAGE__Select( StageName );

//	wxMessageBox( "Stage Selected : " + StageName , "Debug" );
	return;
}


//****************************************************

void LevelEditorFrame::Evt_ReloadAll( wxCommandEvent& evt )
{
	ReloadAll( false );
}


//****************************************************

void LevelEditorFrame::Evt_SaveAll( wxCommandEvent& evt )
{
	SAVE_All();
}


//****************************************************
void LevelEditorFrame::Evt_SET__Edit_Settings( wxCommandEvent& evt_menu )
{
	if( ! menuBar->FindItem(ID_SET_EDIT_SETTINGS)->IsEnabled() )
		return;

	SET__Edit_Settings();
}


//****************************************************

void LevelEditorFrame::Evt_SET__Edit_ExtendedSettings( wxCommandEvent& evt_menu )
{
	if( ! menuBar->FindItem(ID_SET_EDIT_SETTINGS)->IsEnabled() )
		return;

	SET__Edit_ExtendedSettings();
}


//****************************************************

void LevelEditorFrame::Evt_STAGE__Edit_Declare_Settings( wxCommandEvent& evt_menu )
{
	if( ! menuBar->FindItem(ID_STAGE_EDIT_DECLARE_SETTINGS)->IsEnabled() )
		return;

	STAGE__Edit_Declare_Settings();
}


//****************************************************

void LevelEditorFrame::Evt_STAGE__Edit_Settings( wxCommandEvent& evt_menu )
{
	if( ! menuBar->FindItem(ID_STAGE_EDIT_SETTINGS)->IsEnabled() )
		return;

	STAGE__Edit_Settings();
}


//****************************************************

void LevelEditorFrame::Evt_STAGE__Edit_ExtendedSettings( wxCommandEvent& evt_menu )
{
	if( ! menuBar->FindItem(ID_STAGE_EDIT_SETTINGS)->IsEnabled() )
		return;

	STAGE__Edit_ExtendedSettings();
}


//****************************************************

void LevelEditorFrame::Evt_OBJECT__Edit_Settings( wxCommandEvent& evt_menu )
{
	if( ! menuBar->FindItem(ID_OBJECT_PROPERTIES)->IsEnabled() )
		return;

	OBJECT__Edit_Settings();
}


//****************************************************

void LevelEditorFrame::Evt_OBJECT__Edit_ExtendedSettings( wxCommandEvent& evt_menu )
{
	if( ! menuBar->FindItem(ID_OBJECT_PROPERTIES)->IsEnabled() )
		return;

	OBJECT__Edit_ExtendedSettings();
}


//****************************************************

void LevelEditorFrame::Evt_STAGE__Save( wxCommandEvent& evt_menu )
{
	STAGE__Save(curr_stageFile);
}


//****************************************************

void LevelEditorFrame::Evt_STAGE__Reload( wxCommandEvent& evt_menu )
{
	STAGE__Reload();
}


//****************************************************

void LevelEditorFrame::Evt_OpenLevelsInEditor( wxCommandEvent& evt_menu )
{
	wxFileName levels_path(dataDirPath);
	levels_path.AppendDir(dataDirPath.GetFullName());
	levels_path.SetFullName(curr_mod->levels_txt);
	wxString t = levels_path.GetFullPath();
	OpenInExternalEditor( this, t );
}


//****************************************************

void LevelEditorFrame::Evt_OpenStageInEditor( wxCommandEvent& evt_menu )
{
	if( curr_stage_declaration == NULL )
	{
		wxMessageBox( wxT("No STAGE selected !!!"),
			      wxT("ERROR"), wxOK | wxICON_EXCLAMATION, leFrame );
		return;
	}

	OpenInExternalEditor( this, curr_stage_declaration->GetFileName() );
}

//****************************************************

void LevelEditorFrame::Evt_Quit( wxCommandEvent& evt_menu )
{
	leFrame->Close();
}



//****************************************************

void LevelEditorFrame::Evt_SAVE_LevelsTxt( wxCommandEvent& evt_menu )
{
	SET__Save();
}


//****************************************************

void LevelEditorFrame::Evt_STAGE__New( wxCommandEvent& evt_menu )
{
	STAGE__New();
}


//****************************************************

void LevelEditorFrame::Evt_SET__New( wxCommandEvent& evt_menu )
{
	SET__New();
}


//****************************************************

void LevelEditorFrame::Evt_LoadedEntities( wxCommandEvent& evt_menu )
{
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
	{
		wxMessageBox( wxT("No current stage file loaded !!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
		return;
	}

	myWnd_LoadedEnt t( leFrame, (ob_stage*) curr_stageFile->obj_container );
	t.ShowModal();
}

//----------------------------------------------------------------
void
LevelEditorFrame::Evt_Help(wxCommandEvent& evt_menu )
{
	if( helpWin != NULL )
		delete helpWin;
	helpWin = new WndFromText( this, wxT("Manual"), GetRessourceFile_String( wxT("manual_stage.txt") ));
	helpWin->Show(true);
}

//****************************************************

void LevelEditorFrame::Evt_BACKGROUND_Click(wxCommandEvent& evt_menu )
{
	if( curr_stageFile == NULL )
	{
		wxMessageBox( wxT("Curr_stageFile == NULL !!!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, this );
		return;
	}
	else if( curr_stageFile->obj_container == NULL )
	{
		wxMessageBox( wxT("Curr_stageFile->obj_container == NULL !!!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, this );
		return;
	}
	
	BG_Configure( (ob_BG_Layer*) curr_stageFile->obj_container->GetSubObject( wxT("background")), true );
}


//****************************************************

void LevelEditorFrame::Evt_BGLayer_Add( wxCommandEvent& evt_menu )
{
	BG_Add();
}


//****************************************************
void 
LevelEditorFrame::Evt_BGLayer_Order( wxCommandEvent& evt_menu )
{
	BG_Order();
}

//****************************************************
void 
LevelEditorFrame::Evt_BGTest( wxCommandEvent& evt_menu )
{
	BG_Test();
}

//****************************************************
void 
LevelEditorFrame::Evt_BGLayer_Click( wxCommandEvent& evt_menu )
{
	int ind_menu = evt_menu.GetId() - ID_SCENE_BG_LIST_START;
	wxMessageBox( wxT("BG num ") + IntToStr( ind_menu ) );
}


//****************************************************

void LevelEditorFrame::Evt_Panel_Click( wxCommandEvent& evt_menu )
{
	int ind_menu = evt_menu.GetId() - ID_SCENE_PANELS_LIST_START;
	Panel_Edit(ind_menu);
}



//****************************************************

void LevelEditorFrame::Evt_Panel_Add( wxCommandEvent& evt_menu )
{
	Panel_Add();
}


//****************************************************
void 
LevelEditorFrame::Evt_Panel_Order( wxCommandEvent& evt_menu )
{
	Panel_Order();
}

//****************************************************
void 
LevelEditorFrame::Evt_FrontPanels_Edit( wxCommandEvent& evt_menu )
{
	FrontPanel_Edit_All();
}

//****************************************************
void 
LevelEditorFrame::Evt_FrontPanel_Click( wxCommandEvent& evt_menu )
{
	int ind_menu = evt_menu.GetId() - ID_SCENE_FRONTPANEL_LIST_START;
	FrontPanel_Edit_a_panel( ind_menu, false );
}

//------------------------------------------------------------------------
void 
LevelEditorFrame::Evt_SuperType_Selection_Change( wxCommandEvent& evt_menu )
{
	wxString sel_str = object_SuperType_list->GetStringSelection();
	int _sst = SST_NONE;
	if( sel_str != wxString() )
		_sst = Stage__SuperType_ToInt( sel_str.Right(sel_str.Len() - 2 ) );
	
	SuperType__Select( _sst );
}

//------------------------------------------------------------------------
void 
LevelEditorFrame::Evt_SuperType_DblClick( wxMouseEvent& evt )
{
	wxString sel_str = object_SuperType_list->GetStringSelection();
	int _sst = SST_NONE;
	if( sel_str != wxString() )
		_sst = Stage__SuperType_ToInt( sel_str.Right(sel_str.Len() - 2 ));
	
	if( _sst != SST_NONE )
		OBJECT__ShowStates__Toggle_SuperType(_sst );
	else
	{
		if( aSet_Hide_SuperType.empty() == true && aSet_Hide_TYPE.empty() == true )
		{
			for( size_t i = 0; i <= SST_MAX; i++ )
			{
				if( i != SST_NONE )
					OBJECT__ShowStates__Add_SuperType( i );
			}
		}
		else
			OBJECT__ShowStates__Clear();
	}
}

//------------------------------------------------------------------------
void 
LevelEditorFrame::Evt_Type_Selection_Change( wxCommandEvent& evt_menu )
{
	wxString sel_str = object_Type_list->GetStringSelection();
	if( sel_str == wxString() )
	{
		TYPE__Select(STAGETYPE_NONE);
		return;
	}
	
	int _type = Stage__Type_ToInt( sel_str.Right(sel_str.Len() - 2 ) );
	TYPE__Select( _type );
}

//------------------------------------------------------------------------
void 
LevelEditorFrame::Evt_Type_DblClick( wxMouseEvent& evt )
{
	wxString sel_str = object_Type_list->GetStringSelection();
	int _t = STAGETYPE_NONE;
	if( sel_str != wxString() )
		_t = Stage__Type_ToInt( sel_str.Right(sel_str.Len()-2) );
	
	if( _t != STAGETYPE_NONE )
		OBJECT__ShowStates__Toggle_TYPE(_t );
}


//------------------------------------------------------------------------
void 
LevelEditorFrame::Evt_ObjectList_Selection_Change( wxCommandEvent& evt_menu )
{
	OBJECT__CtrlList_Selection_Change();
}

//------------------------------------------------------------------------
void 
LevelEditorFrame::Evt_EntitiesListTypeChg( wxCommandEvent& evt )
{
	ENTITIES__FILTER__Apply();
	evt.Skip();
}

//------------------------------------------------------------------------
void 
LevelEditorFrame::Evt_EntitiesListTxtChg( wxKeyEvent& evt )
{
	evt.Skip();
	if( entities_filter->GetValue() != last_txt_filter )
	{
		last_txt_filter = entities_filter->GetValue();
		ENTITIES__FILTER__Apply();
	}
}

//------------------------------------------------------------------------
void
LevelEditorFrame::Evt_EntitySelChg( wxCommandEvent& evt )
{
	if( curr_stageFile == NULL || b_closing == true )
		return;
	
	int ind = entities_list->GetSelection();
	if( ind < 0 )
	{
		entity_img->SetImage( noImg );
		return;
	}
	
	obFileEntity* ent = (obFileEntity*) entities_list->GetClientData( ind );
	if( ent == NULL )
	{
		entity_img->SetImage( noImg );
		return;
	}
	
	wxImage* _theImg = ent->Load_AvatarImg();
	if( _theImg == NULL )
	{
		entity_img->SetImage( noImg );
		return;
	}
	
	entity_img->SetImage( _theImg );
}

//------------------------------------------------------------------
void
LevelEditorFrame::Evt_EntitiesListRightClik( wxMouseEvent& evt )
{
	// Simulate a left click
	wxMouseEvent evt2(wxEVT_LEFT_UP);
	evt2.m_x = evt.m_x;
	evt2.m_y = evt.m_y;
	evt.Skip();
	
	entities_list->ProcessEvent( evt2 );
	
	// Now show the PopupMenu
	entities_list->PopupMenu( menuAdd );
}

//------------------------------------------------------------------
void
LevelEditorFrame::Evt_EntitiesAddOne( wxCommandEvent& evt )
{
	evt.Skip();
	int ind = entities_list->GetSelection();
	if( ind == wxNOT_FOUND )
		return;
	
	// Get the entiti to add
	obFileEntity* ent = (obFileEntity*) entities_list->GetClientData( ind );
	if( ent == NULL )
		return;
	
	OBJECT__Add_Entity( ent );
}

//------------------------------------------------------------------
void
LevelEditorFrame::Evt_TYPEListRightClik( wxMouseEvent& evt )
{
	// Simulate a left click
	wxMouseEvent evt2(wxEVT_LEFT_UP);
	evt2.m_x = evt.m_x;
	evt2.m_y = evt.m_y;
	evt.Skip();
	
	object_Type_list->ProcessEvent( evt2 );
	
	int ind = object_Type_list->GetSelection();
	if( ind == wxNOT_FOUND )
		return;

	// Have to get the TYPE of the object
	int _TYPE = (size_t) object_Type_list->GetClientData( ind );
	
	// Only Some type have ADD popup menu
	bool b_continue = false;
	switch( _TYPE )
	{
		case SOBJ_PLAYER_SPAWN:
		case SOBJ_SETPALETTE:
		case SOBJ_WALL:
		case SOBJ_HOLE:
		case SOBJ_WAIT:
		case SOBJ_NOJOIN:
		case SOBJ_CANJOIN:
		case SOBJ_SCROLLZ:
		case SOBJ_BLOCKADE:
		case SOBJ_GROUP:
		case SOBJ_LIGHT:
			b_continue = true;
	}
	
	if( b_continue == false )
		return;

	// Check if the type is invisible
	bool b_filtered = false;
	int _sType=Stage__Type__Get_SuperType(_TYPE);
	if( 		aSet_Hide_SuperType.find(_sType) 
			!= 
			aSet_Hide_SuperType.end()
		  )	
		b_filtered = true;
	else if( 	aSet_Hide_TYPE.find(_TYPE) 
			!= 
			aSet_Hide_TYPE.end()
		  )
		b_filtered = true;

	if( b_filtered == true )
	{
		wxMessageBox( wxT("You have hidden this type of objects !!")
		, wxT("Impossible"), wxOK | wxICON_INFORMATION, this );
		return;
	}
	
	
	// Now show the PopupMenu
	object_Type_list->PopupMenu( menuAdd );
}

//------------------------------------------------------------------
void
LevelEditorFrame::Evt_TYPEListAddOne( wxCommandEvent& evt )
{
	evt.Skip();

	int ind = object_Type_list->GetSelection();
	if( ind == wxNOT_FOUND )
		return;

	// Have to get the TYPE of the object
	int _TYPE = (size_t) object_Type_list->GetClientData( ind );
	
	OBJECT__Add_Default_of_Type( _TYPE );
}


//------------------------------------------------------------------
void
LevelEditorFrame::Evt_ObjectDuplicate(wxCommandEvent& evt )
{
	OBJECT__ListSelected__Duplicate();
}

//------------------------------------------------------------------
void
LevelEditorFrame::Evt_ObjectDelete(wxCommandEvent& evt )
{
	OBJECT__ListSelected__Delete();
}

//------------------------------------------------------------------
void
LevelEditorFrame::Evt_ObjectProps(wxCommandEvent& evt )
{
	if(aSet_Selected.size() != 1 )
	{
		wxMessageBox( wxT("No property with multi or no selection")
		, wxT("Impossible"), wxOK | wxICON_INFORMATION, leFrame );
		return;
	}
	OBJECT__ListSelected__Props();
}

//------------------------------------------------------------------
void
LevelEditorFrame::Evt_Object_SortAT(wxCommandEvent& evt )
{
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
		return;
	ob_stage* stg = (ob_stage*) curr_stageFile->obj_container;
	
	// First make the user sure
	int rep = wxMessageBox( wxT("This will put all the objects with a \"AT\"\nproperty at the end of the file, sorted by the \"AT\" position\nAre you sure that's what you want ?")
	, wxT("Question !!"), wxYES_NO | wxICON_EXCLAMATION, this );
	if( rep != wxYES )
		return;
	
	// Extract all AT objects
	size_t nb_subobj;
	ob_object** arr_obj = stg->GetAllSubObjS( nb_subobj );
	if( nb_subobj == 0 )
	{
		wxMessageBox( wxT("No objects in the stage file !")
		, wxT("ProPlem !"), wxOK | wxICON_EXCLAMATION, this );
		return;
	}
	
	// make a history group
	theHistoryManager.GroupStart(wxT("Sort ATs objects" ));
	
	list<ob_stage_at*> ats_obj;
	for( size_t i = 0; i < nb_subobj; i++ )
	{
		if( ob_stage::Is_Stage_At_Type(arr_obj[i]) == false )
			continue;
		ats_obj.push_back( (ob_stage_at*) arr_obj[i] );
		arr_obj[i]->Detach();
	}
	
	if( ats_obj.empty() == true )
	{
		// close the history group
		theHistoryManager.GroupEnd();
		
		wxMessageBox( wxT("No \"AT\" objects in the stage file !")
		, wxT("ProPlem !"), wxOK | wxICON_EXCLAMATION, this );
		return;
	}

	// Insert the first object at the end of
	// sort the ats_obj
	list<ob_stage_at*>::iterator it_choosen, it_curr, it_end;
	while( ats_obj.empty() == false )
	{
		it_choosen = ats_obj.begin();
		it_curr    = ats_obj.begin();
		it_end     = ats_obj.end();
		for(; it_curr != it_end; it_curr++ )
			if( (*it_curr)->GetAt() < (*it_choosen)->GetAt() )
				it_choosen = it_curr;
		ob_stage_at* o = *it_choosen;
		ats_obj.erase( it_choosen );
		stg->Add_SubObj(o);
	}
	
	curr_stageFile->changed = true;
	// close the history group
	theHistoryManager.GroupEnd();
}

//****************************************************
// END OF EVENTS FUNCTIONS
//****************************************************


//****************************************************
void 
LevelEditorFrame::SAVE_All()
{
	if( fileLevels == NULL )
		return;

	SET__Save();

	size_t nb_sets;
	ob_object** t = fileLevels->obj_container->GetSubObjectS_ofType( OB_TYPE_STAGES_SET, nb_sets );

	if( nb_sets == 0 )
		return;

	for( size_t i = 0; i < nb_sets; i++ )
	{
		size_t nb_stage;
		ob_StageDeclaration** l_stages = (ob_StageDeclaration**) t[i]->GetSubObjectS_ofType( OB_TYPE_STAGE_DECLARATION, nb_stage );

		if( nb_stage == 0 )
			continue;

		for( size_t j = 0; j < nb_stage; j++ )
		{
			
			if( l_stages[j]->stage != NULL && l_stages[j]->stage->changed )
				STAGE__Save( l_stages[j]->stage );
		}

		delete[] l_stages;
	}

	delete[] t;
}


//****************************************************

bool LevelEditorFrame::STAGE__Save( obFileStage* _fStage )
{
	if( _fStage == NULL )
	{
		wxMessageBox( wxT("STAGE__Save() : NULL argument !!")
		, wxT("BUG !!"), wxOK | wxICON_EXCLAMATION, this );
		return false;
	}

	if( ! _fStage->Write() )
	{
		wxMessageBox( wxT("Unable to write the file  :\n\n") + _fStage->filename.GetFullPath()
		, wxT("Error !!"), wxOK | wxICON_EXCLAMATION, this );
		return false;
	}
	return true;
}


//****************************************************

bool LevelEditorFrame::STAGE__Reload( bool b_quiet )
{
	if( curr_stage_declaration == NULL )
	{
		wxMessageBox( wxT("STAGE__Reload() : NULL curr_stage_declaration !!")
		, wxT("BUG !!"), wxOK | wxICON_EXCLAMATION, this );
		return false;
	}

	if( curr_stage_declaration->stage == NULL )
	{
		wxMessageBox( wxT("STAGE__Reload() : NULL curr_stage !!")
		, wxT("BUG !!"), wxOK | wxICON_EXCLAMATION, this );
		return false;
	}

	if( curr_stage_declaration->stage->changed && ! b_quiet )
	{
		int rep = wxMessageBox( wxT("Discard modifications made to the stage ?")
		, wxT("Question !!"), wxYES_NO | wxICON_EXCLAMATION, this );
		if( rep != wxYES )
			return false;
	}

	return STAGE__Select( curr_stage_declaration->GetStageName() );
}


//****************************************************
void 
LevelEditorFrame::SET__Save()
{
	if( fileLevels == NULL )
		return;

	if( fileLevels->changed )
		fileLevels->Write();
}


//****************************************************

void LevelEditorFrame::SET__Edit_Settings()
{
	if( curr_set == NULL )
	{
		wxMessageBox( wxT("No Curr SET defined !!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, this );
		return;
	}
	wxString old_set_name = curr_set->GetSetName();

	// No history for this operation
	theHistoryManager.Set_State( false );
	
	myWnd_SETProperties t( leFrame, curr_set, wxT("Current Set Properties") );
	t.ShowModal();
	
	theHistoryManager.Set_State(true);

	if( t.b_chg )
	{
		fileLevels->changed = true;
		UpdateTitle();

		//Update also the menu elt
		wxMenu* subMenu = menuSets->FindItem( ID_SET_LIST )->GetSubMenu();
		int m_id = LEVEL__GetCurrInd() + ID_SET_LIST_START;
		wxMenuItem* _it = subMenu->FindItem( m_id );
		if( _it == NULL )
		{
			wxMessageBox( wxT("Can't find the menu item with the id returned, for the current set!!")
			, wxT("BUG"), wxOK | wxICON_EXCLAMATION, this );
		}
		else
		{
			_it->SetItemLabel( curr_set->GetSetName() );
		}
	}
}


//****************************************************

void LevelEditorFrame::SET__Edit_ExtendedSettings()
{
	if( curr_set == NULL )
	{
		wxMessageBox( wxT("No curr set selected !!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, this );
		return;
	}

	// fire the modal dialog
	GridPropFrame *temp = new GridPropFrame( this, curr_set, wxT("Set Extended Properties" ));
	wxArrayString filter;
	filter.Add(wxT("select"));
	filter.Add(wxT("nosame"));
	filter.Add(wxT("noshare"));
	filter.Add(wxT("ifcomplete"));
	filter.Add(wxT("lives"));
	filter.Add(wxT("credits"));
	filter.Add(wxT("maxplayers"));
	filter.Add(wxT("typemp"));
	filter.Add(wxT("cansave"));
	filter.Add(wxT("scene"));

	temp->SetFilter_Properties( filter );

	int* filtertypes = new int[1];
	filtertypes[0] = OB_TYPE_STAGE_DECLARATION;
	temp->SetFilter_Properties( filtertypes, 1 );

	// No history for this operation
	theHistoryManager.Set_State( false );

	temp->ShowModal();
	
	theHistoryManager.Set_State( true );
	
	if( temp->Changed() )
		fileLevels->changed = true;
	delete temp;
}


//****************************************************

void LevelEditorFrame::STAGE__Edit_Declare_Settings()
{
	myWnd_LEVELSSTAGEProperties t( leFrame, curr_stage_declaration
	, true, wxT("Stage Declaration Properties" ));
					
	// No history for this operation
	theHistoryManager.Set_State( false );

	t.ShowModal();

	theHistoryManager.Set_State( true );
	
	if( t.b_chg )
	{
		fileLevels->changed = true;

		wxString new_stagefilename = curr_stage_declaration->GetSubObject_Token( wxT("file") );
		wxString old_stagefilename;
		if( curr_stageFile != NULL )
			old_stagefilename = Convert_To_Ob_Path(curr_stageFile->filename.GetFullPath() );

		UpdateTitle();

		//Update also the menu elt
		wxMenu* subMenu = menuStage->FindItem( ID_STAGE_LIST )->GetSubMenu();
		int m_id = STAGE__GetCurrInd() + ID_STAGE_LIST_START;
		wxMenuItem* _it = subMenu->FindItem( m_id );
		if( _it == NULL )
		{
			wxMessageBox( wxT("Can't find the menu item with the current stage !!")
			, wxT("BUG"), wxOK | wxICON_EXCLAMATION, this );
		}
		else
		{
			_it->SetItemLabel( curr_stage_declaration->GetStageName() );
		}

		// The stage file has been changed
		if(		curr_stageFile == NULL
			||
				new_stagefilename != old_stagefilename
			)
		{

			//Ask the user to be sure
			if( curr_stageFile != NULL && curr_stageFile->changed )
			{
				int _reponse = wxMessageBox( wxT("Modification have been made to\n")
				+ old_stagefilename + wxT("\n\n"
				"Still want to change the stageFile in levels.txt ?")
				, wxT("ATTENTION !")
						, wxICON_EXCLAMATION | wxYES_NO );
				if( _reponse != wxYES )
				{
					curr_stage_declaration->SetProperty(wxT( "file"), old_stagefilename );
					return;
				}
			}

			STAGE__Select( curr_stage_declaration->GetStageName() );
		}

	}
}


//****************************************************

void LevelEditorFrame::STAGE__Edit_Settings()
{
	if( !curr_stage_declaration->LoadStage() )
		return;

	myWnd_STAGEProperties t( leFrame, (ob_stage*) curr_stage_declaration->stage->obj_container );
	
	// No history for this operation
	theHistoryManager.Set_State( false );
	
	t.ShowModal();
	
	theHistoryManager.Set_State( true );

	if( t.b_chg )
	{
		ob_stage* t_st = (ob_stage*) curr_stage_declaration->stage->obj_container;
		curr_stage_declaration->stage->changed = true;
		int prev_dir = t_st->direction;
		t_st->Direction_ComputeAndCache();
		
		// Direction change ==>> Have to reload All Graphics
		if( prev_dir != t_st->direction )
			VIEW__Update();
	}
}


//****************************************************

void LevelEditorFrame::STAGE__Edit_ExtendedSettings()
{
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
	{
		wxMessageBox( wxT("No curr stage file avalaible !!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, this );
		return;
	}

	// fire the modal dialog
	GridPropFrame *temp = new GridPropFrame( this, curr_stageFile->obj_container, wxT("Stage Extended Properties") );
	wxArrayString filter;
	filter.Add(wxT("setweap"));
	filter.Add(wxT("noslow"));
	filter.Add(wxT("maxtossspeed"));
	filter.Add(wxT("maxfallspeed"));
	filter.Add(wxT("gravity"));
	filter.Add(wxT("endhole"));
	filter.Add(wxT("blocked"));
	filter.Add(wxT("mirror"));
	filter.Add(wxT("rock"));
	filter.Add(wxT("notime"));
	filter.Add(wxT("noreset"));
	filter.Add(wxT("settime"));
	filter.Add(wxT("bossmusic"));
	filter.Add(wxT("music"));
	filter.Add(wxT("type"));
	filter.Add(wxT("order"));
	filter.Add(wxT("direction"));

	temp->SetFilter_Properties( filter );

	int* filtertypes = new int[30];
	int i= 0;
	filtertypes[i] = OB_TYPE_BG_LAYER;
	i++;
	filtertypes[i] = OB_TYPE_FRONT_PANEL;
	i++;
	filtertypes[i] = OB_TYPE_STAGE_PANEL;
	i++;
	filtertypes[i] = OB_TYPE_WALL;
	i++;
	filtertypes[i] = OB_TYPE_HOLE;
	i++;
	filtertypes[i] = OB_TYPE_PLAYER_SPAWN;
	i++;
	filtertypes[i] = OB_TYPE_STAGE_AT;
	i++;
	filtertypes[i] = OB_TYPE_SPAWN;
	i++;
	filtertypes[i] = OB_TYPE_SPAWN_HEALTHY;
	i++;
	filtertypes[i] = OB_TYPE_SPAWN_ITEM;
	i++;

	temp->SetFilter_Properties( filtertypes, i );

	// No history for this operation
	theHistoryManager.Set_State( false );

	temp->ShowModal();

	theHistoryManager.Set_State( true );
	
	if( temp->Changed() )
		curr_stageFile->changed = true;
	delete temp;
}


//****************************************************

wxString LevelEditorFrame::GetModifsDone()
{
	wxString ModifsDone;
	if( fileLevels->changed )
	{
		ModifsDone += wxT("- levels.txt\n");
	}

	size_t nb_sets;
	ob_object** t = fileLevels->obj_container->GetSubObjectS_ofType( OB_TYPE_STAGES_SET, nb_sets );

	if( nb_sets == 0 )
		return ModifsDone;

	for( size_t i = 0; i < nb_sets; i++ )
	{
		size_t nb_stage;
		ob_StageDeclaration** l_stages = (ob_StageDeclaration**) t[i]->GetSubObjectS_ofType( OB_TYPE_STAGE_DECLARATION, nb_stage );

		if( nb_stage == 0 )
			continue;

		for( size_t j = 0; j < nb_stage; j++ )
		{
			if( l_stages[j]->stage != NULL && l_stages[j]->stage->changed )
				ModifsDone += wxT("- ") + l_stages[j]->stage->filename.GetFullName() + wxT("\n");
		}

		delete[] l_stages;
	}
	delete[] t;

	return ModifsDone;
}


//****************************************************
void 
LevelEditorFrame::ReloadAll( bool b_quiet, bool b_save_states )
{
	wxBusyInfo( wxT("Reloading..."), this );
	//*************************
	// Search for modifications done
	wxString ModifsDone;
	if( ! b_quiet )
		ModifsDone = GetModifsDone();

	//*************************
	// MAKE the user sure
	if( ModifsDone != wxString() )
	{
		int _reponse = wxMessageBox( wxT("Modification made to this files will be loose :\n")
		+ ModifsDone + wxT("\n\n"
		"Do you still want to reload everything ?")
		, wxT("ATTENTION !")
				, wxICON_EXCLAMATION | wxYES_NO );

		if( _reponse != wxYES )
			return;

		modificationList.Clear();
	}

	// Clear the history
	theHistoryManager.Clear_History();
	
	//*************************
	// Preventive
	Disable(wxT( "ALL") );


	//*************************
	// Save current SET and STAGE for further restoration
	wxString curr_SetName;
	if( curr_set != NULL )
		curr_SetName = curr_set->GetSetName();
	wxString curr_StageName;
	if( curr_stage_declaration != NULL )
		curr_StageName = curr_stage_declaration->GetStageName();

	//*************************
	// Save current VIEW state for further restoration
	if( b_save_states == true )
	{
		stageView->SaveState();
	}


	//**************************************
	// FREE ALL STAGES OBJECTS
	for( size_t i = 0; i < nb_set; i++)
	{
		curr_set = levels_sets[i];

		if( stage_list != NULL )
			delete[] stage_list;

		stage_list = curr_set->GetStages( nb_stage );
		for( size_t j = 0; j < nb_stage; j++)
			stage_list[j]->UnLoadStage();
	}


	//**************************************
	// CLEAR STUFFS
	curr_set = NULL;
	curr_stage_declaration = NULL;
	curr_stageFile= NULL;
	SET__ClearMenu_List();
	STAGE__Clear_List();
	NOTEBOOK__Set_NoSelection();

	//**************************************
	// RELOAD LEVELS.TXT
	if( ! FileLevels_Reload( leFrame ) )
	{
		UpdateTitle();
		return;
	}

	//**************************************
	SET__Load_List(b_quiet);
	if( nb_set <= 0 )
	{
		UpdateTitle();
		return;
	}

	bool b_select_ok = false;
	if( curr_SetName  != wxString() )
		if( SET__Select(curr_SetName, true) )
			b_select_ok = true;
	if( ! b_select_ok )
	{
		Disable( wxT("NO_SET_SELECTED") );
		if( ! b_quiet && nb_set > 0 )
			wxMessageBox( wxT("No SET selected !!"), wxT("Information") );
	}

	else if( nb_stage > 0 )
	{
		b_select_ok = false;
		if( curr_StageName  != wxString() )
			if( STAGE__Select(curr_StageName) )
				b_select_ok = true;

		if( ! b_select_ok )
		{
			Disable( wxT("NO_STAGE_SELECTED") );
			if( ! b_quiet && nb_stage >  0 )
				wxMessageBox( wxT("No STAGE selected !!"), wxT("Information") );
		}
	}

	//-------------------------------------------
	UpdateTitle();

}


//****************************************************

void LevelEditorFrame::UpdateTitle()
{
	wxString newTitle = baseTitle;
	if( curr_set == NULL )
		newTitle += wxT(" - NO SET SELECTED");
	else
	{
		newTitle += wxT(" - ") + curr_set->GetSetName();
		if( curr_stage_declaration == NULL )
			newTitle += wxT(" : NO STAGE SELECTED");
		else
			newTitle += wxT(" : ") + curr_stage_declaration->GetStageName();
	}
	SetTitle( newTitle );
}

//****************************************************

void LevelEditorFrame::Disable( const wxString& what_to_disable )
{
	Disable_MenuItems( what_to_disable );
	
	// Disable GUI Object
	//FIXME
}


//****************************************************

void LevelEditorFrame::Disable_MenuItems( const wxString& what_to_disable )
{
	bool b_enable_levels = ! ( 	what_to_disable == wxT("ALL") ||  what_to_disable == wxT("NO_LEVELS_TXT") );

	menuBar->FindItem(ID_LEVELSTXT_SAVE)->Enable( b_enable_levels );

	// SETs MENU
	bool b_enable_sets = b_enable_levels && ! ( what_to_disable == wxT("NO_SET") );
	EnableItemsMenu( menuSets, b_enable_sets );
	menuBar->FindItem(ID_SET_NEW)->Enable( b_enable_levels );
	menuBar->FindItem(ID_SET_EDIT_SETTINGS)->Enable( b_enable_sets && what_to_disable != wxT("NO_SET_SELECTED") );
	menuBar->FindItem(ID_SET_EDIT_EXTENDED_PROPS)->Enable( b_enable_sets && what_to_disable != wxT("NO_SET_SELECTED") );


	// STAGEs options from levels.txt MENU
	bool b_enable_LEVELSTXTstage = b_enable_sets &&
	! ( what_to_disable == wxT("NO_STAGE") || what_to_disable == wxT("NO_SET_SELECTED") );
	EnableItemsMenu( menuStage, b_enable_LEVELSTXTstage );
	menuBar->FindItem(ID_STAGE_NEW)->Enable( b_enable_sets && what_to_disable != wxT("NO_SET_SELECTED" ));
	menuBar->FindItem(ID_STAGE_EDIT_DECLARE_SETTINGS)->Enable( b_enable_LEVELSTXTstage && what_to_disable != wxT("NO_STAGE_SELECTED" ));
	menuBar->FindItem(ID_STAGE_RELOAD)->Enable( b_enable_LEVELSTXTstage && what_to_disable != wxT("NO_STAGE_SELECTED" ));


	bool b_enable_stage = b_enable_LEVELSTXTstage &&
	! ( what_to_disable == wxT("NO_FILESTAGE") || what_to_disable == wxT("NO_STAGE_SELECTED") );
	menuBar->FindItem(ID_STAGE_EDIT_SETTINGS)->Enable( b_enable_stage );
	menuBar->FindItem(ID_STAGE_EDIT_EXTENDED_PROPS)->Enable( b_enable_stage );
	menuBar->FindItem(ID_STAGE_LOADED_ENTITIES)->Enable( b_enable_stage );
	menuBar->FindItem(ID_STAGE_SAVE)->Enable( b_enable_stage );

	// SCENEs MENU
	bool b_enable_scene = b_enable_stage &&
	! ( what_to_disable == wxT("NO_SCENE")
	|| what_to_disable == wxT("NO_SCENE_NO_OBJECT")
	|| what_to_disable == wxT("NO_STAGE_SELECTED")
				);
//	b_enable_scene = true; //FIXME
	EnableItemsMenu( menuScene, b_enable_scene );


	// OBJECTs MENU
	bool b_enable_object = b_enable_stage &&
	! (    what_to_disable == wxT("NO_OBJECT")
	|| what_to_disable == wxT("NO_SCENE_NO_OBJECT")
	|| what_to_disable == wxT("NO_SET_SELECTED")
			  );
	EnableItemsMenu( menuObjects, b_enable_object );

}


//****************************************************

void LevelEditorFrame::SET__ClearMenu_List()
{
	wxMenu* submenu = menuSets->FindItem( ID_SET_LIST )->GetSubMenu();
	ClearMenu( submenu );

	if( levels_sets != NULL )
	{
		delete[] levels_sets;
		levels_sets = NULL;
	}
	nb_set = 0;
}

//****************************************************

void LevelEditorFrame::STAGE__Clear_List()
{
	wxMenu* submenu = menuStage->FindItem( ID_STAGE_LIST )->GetSubMenu();
	ClearMenu( submenu );

	if( stage_list != NULL )
	{
		delete[] stage_list;
		stage_list = NULL;
	}
	nb_stage = 0;

	curr_stage_declaration = NULL;
	curr_stageFile = NULL;

	// reset the VIEW
	VIEW__Reset();
}

//****************************************************

void LevelEditorFrame::SET__Load_List( bool b_quiet )
{
	// Clear the current list of set
	SET__ClearMenu_List();

	if( fileLevels == NULL )
	{
		Disable(wxT("NO_LEVELS_TXT"));
		if( ! b_quiet )
			wxMessageBox(wxT( "The file levels.txt is not loaded !!"), wxT("Problem !") );
	}

	else
	{
		// Get the Sets list
		levels_sets = ((ob_levels*) fileLevels->obj_container)->GetSets(nb_set);
		if( levels_sets == NULL )
		{
			Disable(wxT("NO_SET"));
			if( ! b_quiet )
				wxMessageBox( wxT("The file levels.txt contain no <Set> !!"), wxT("Problem !") );
		}
		else
		{
			// Append the sets to the menu
			wxMenu* submenu = menuSets->FindItem( ID_SET_LIST )->GetSubMenu();
			for( size_t i = 0; i < nb_set; i++ )
				submenu->Append( ID_SET_LIST_START+ i , levels_sets[i]->GetSetName() );
		}
	}
}


//****************************************************

void LevelEditorFrame::STAGE__Load_List( bool b_quiet )
{
	// Clear the current list of set
	STAGE__Clear_List();

	if( nb_set == 0 )
		return;

	if( curr_set == NULL )
	{
		Disable(wxT("NO_SET_SELECTED"));
		if( ! b_quiet )
			wxMessageBox( wxT("Before editing you must select a set and a stage"), wxT("Information") );
	}

	else
	{
		// Get the Stages list
		stage_list = curr_set->GetStages(nb_stage);

		if( stage_list == NULL )
		{
			Disable(wxT("NO_STAGE"));
			if( ! b_quiet )
				wxMessageBox(wxT( "The SET contain no stage !!"), wxT("ProPlem !") );
		}
		else
		{
			// Append the stages to the menu
			wxMenu* submenu = menuStage->FindItem( ID_STAGE_LIST )->GetSubMenu();
			for( size_t i = 0; i < nb_stage; i++ )
				submenu->Append( ID_STAGE_LIST_START+ i , stage_list[i]->GetStageName() );
		}
	}
}


//****************************************************

bool LevelEditorFrame::SET__Select( const wxString& _name, bool b_no_choose_stage )
{
	ob_StagesSet* t = NULL;
	for( size_t i = 0; i < nb_set; i++)
	{
		if( levels_sets[i]->GetSetName().Upper() == _name.Upper() )
		{
			t = levels_sets[i];
			break;
		}
	}

	if( t == NULL )
		return false;

	ob_StagesSet* previous = curr_set;
	curr_set = t;

	STAGE__Load_List(false);

	// Ask the stage to Edit
	if( nb_stage > 0 && ! b_no_choose_stage)
	{
		wxArrayString _stages;
		for( int i= 0; i < (int) nb_stage;i++)
			_stages.Add(stage_list[i]->GetStageName() );

		wxSingleChoiceDialog wChoice( this, wxT("Which Stage you wanna edit ?"), wxT("Select a Stage"), _stages );

		// User changed his mind
		if( wChoice.ShowModal() != wxID_OK )
		{
			curr_set = previous;
			return false;
		}

		// Problem of user change his mind during stage loading
		if( ! STAGE__Select(wChoice.GetStringSelection() ) )
		{
			curr_set = previous;
			return false;
		}
	}

	else
	{
		Disable( wxT("NO_STAGE" ));
	}

	SET__Save_LastSession( curr_set->GetSetName() );
	UpdateTitle();
	return true;
}


//****************************************************
bool 
LevelEditorFrame::STAGE__Select( const wxString& _name, bool b_no_saves )
{
	ob_StageDeclaration* t = NULL;
	for( size_t i = 0; i < nb_stage; i++)
	{
		if( stage_list[i]->GetStageName().Upper() == _name.Upper() )
		{
			t = stage_list[i];
			break;
		}
	}

	if( t == NULL )
		return false;

	// Disable history manager
	theHistoryManager.Clear_History();
	theHistoryManager.Force_Disable();
	
	
	if( b_no_saves == false )
	{
		STAGE__Save_LastSession( t->GetStageName() );
		OBJECT__Visibilities_SaveState();
		stageView->SaveState();
	}
	// Must propagate the edited flag to stage_declaration
	else if( 	   curr_stage_declaration != NULL 
			&& curr_stageFile != NULL
			&& curr_stageFile->obj_container != NULL
			)
	{
		curr_stage_declaration->edited =
			   curr_stage_declaration->edited 
			|| curr_stageFile->obj_container->edited;
	}
	
	// Must avoid TAB with no object
	NOTEBOOK__Set_NoSelection();
	
	// Disengage !!
	curr_stage_declaration = NULL;
	
	// must invalidate View Objects
	VIEW__Reset();
	
	curr_stage_declaration = t;
	UpdateTitle();

	if( ! curr_stage_declaration->LoadStage(true) )
	{
		wxMessageBox( wxT("Unable to load the file for the stage\n\n") +
						curr_stage_declaration->GetFileName()
						, wxT("Problem"), wxOK | wxICON_EXCLAMATION, this );
						Disable( wxT("NO_FILESTAGE") );
		return true;
	}

	curr_stageFile = curr_stage_declaration->stage;

	// ReLoad the BAckground Layers
	BG_Reload();
	Panel_Reload();
	FrontPanel_Reload();

	//-----------------------------------
	// Load GUI objects
	//-----------------------------------
	OBJECT__Visibilities_LoadState();
	stageView->PANELS__Reload();
	stageView->Refresh();
	stageView->LoadState();

	Disable( wxT("NONE") );

	// Re-enable history manager
	theHistoryManager.Set_State( true );
	
	return true;
}


//****************************************************

void LevelEditorFrame::Menu_ResetListItem( int id_submenu, int id_startlist )
{
	wxMenuItem* mit = menuBar->FindItem(id_submenu);
	if( mit == NULL )
	{
		wxMessageBox( wxT("Menu_ResetListItem() : Unable to find the submenu item !!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
		return;
	}
	wxMenu* subMenu = mit->GetSubMenu();
	if( subMenu == NULL )
	{
		wxMessageBox( wxT("Menu_ResetListItem() : the submenu item doesn't have a submenu !!")
		, wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
		return;
	}

	// Get the nb of item in the menu
	int nb_tot_its = subMenu->GetMenuItemCount();
	wxMenuItem* start_it = subMenu->FindItem( id_startlist );

	// List is empty
	if( start_it == NULL )
	{
		// Nothing to remove
		return;
	}

	// Try the insert
	for( int i= nb_tot_its - 1; i > 0; i-- )
	{
		wxMenuItem* t_it = subMenu->FindItemByPosition( i );
		if( t_it == NULL )
		{
			wxMessageBox( wxT("Menu_AddListItem() : FindItemByPosition() misunderstanded  !!")
						  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
			return;
		}

		subMenu->Destroy( t_it );
		if( t_it == start_it )
			return;
	}
}


//****************************************************

bool LevelEditorFrame::Menu_AddListItem( int id_submenu, int id_startlist, const wxString& name, bool b_unique, int ind_insert )
{
	wxMenuItem* mit = menuBar->FindItem(id_submenu);
	if( mit == NULL )
	{
		wxMessageBox( wxT("Menu_AddListItem() : Unable to find the submenu item !!")
					  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
		return false;
	}
	wxMenu* subMenu = mit->GetSubMenu();
	if( subMenu == NULL )
	{
		wxMessageBox( wxT("Menu_AddListItem() : the submenu item doesn't have a submenu !!")
					  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
		return false;
	}

	// Get the nb of item in the menu
	int nb_tot_its = subMenu->GetMenuItemCount();
	wxMenuItem* start_it = subMenu->FindItem( id_startlist );

	// List is empty
	if( start_it == NULL )
	{
		if( ind_insert > 0 )
		{
			wxMessageBox( wxT("Menu_AddListItem() : Try to insert item in an empty list !!")
						  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
			return false;
		}
		subMenu->Append( id_startlist, name );
		return true;
	}

	// Try the insert
	int curr_ind = -3;
	int nb_subit = 0;
	bool b_decal = false;
	for( int i= 0; i < nb_tot_its; i++ )
	{
		wxMenuItem* t_it = subMenu->FindItemByPosition( i );
		if( t_it == NULL )
		{
			wxMessageBox( wxT("Menu_AddListItem() : FindItemByPosition() misunderstanded  !!")
						  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
			return false;
		}

		if( t_it == start_it )
		{
			nb_subit = nb_tot_its - i;
			curr_ind = 0;
			start_it = NULL;
		}

		if(curr_ind == ind_insert)
		{
			subMenu->Insert( i, id_startlist + curr_ind, name );
			b_decal = true;
			nb_tot_its++;
		}
		else if( b_decal )
		{
			wxString str = t_it->GetItemLabel();
			subMenu->Destroy( t_it );
			subMenu->Insert( i, id_startlist + curr_ind, str );
		}

		if( curr_ind >= 0)
			curr_ind++;
	}

	if( ind_insert == -1 || (!b_decal && ind_insert == nb_subit) )
	{
		subMenu->Append( id_startlist + nb_subit, name );
		return true;
	}

	return b_decal;
}


//****************************************************

void LevelEditorFrame::BG_ResetMenuList()
{
//	Menu_ResetListItem( ID_SCENE_MENU_BG, ID_SCENE_BG_LIST_START );
//	menuBar->FindItem(ID_SCENE_BG_EDIT_ORDER)->Enable( false );
}


//****************************************************
/*
void LevelEditorFrame::BG_AddToMenu( const wxString& _name, int ind, bool b_quiet )
{
	if( Menu_AddListItem( ID_SCENE_MENU_BG, ID_SCENE_BG_LIST_START, _name, false, ind ) )
	{
		// Enable background order
		wxMenuItem *_it = menuBar->FindItem(ID_SCENE_BG_EDIT_ORDER);
		if( _it == NULL )
		{
			wxMessageBox( "Can't find MenuItem Background Order  !!"
						  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
			return;
		}
		else
			_it->Enable( true );
	}
}
*/

//****************************************************

void LevelEditorFrame::BG_Reload()
{
	BG_ResetMenuList();

	size_t nb_bg;
	ob_BG_Layer** bgs = (ob_BG_Layer**) 
	curr_stageFile->obj_container->GetSubObjectS( wxT("bglayer"), nb_bg );

/*
	for( size_t i = 0; i < nb_bg; i++ )
	{
		BG_AddToMenu(bgs[i]->GetName() );
	}
*/
	if( bgs != NULL )
		delete[] bgs;

}


//****************************************************
bool 
LevelEditorFrame::BG_Configure( ob_BG_Layer* _obj, bool b_background )
{
	bool b_add_BG = ( _obj == NULL );

	myWnd_BGProperties t( leFrame, _obj, b_background );
	
	// make a history group
	theHistoryManager.GroupStart(wxT("Configure Background" ));

	t.ShowModal();
	
	// close the history group
	theHistoryManager.GroupEnd();
	
	if( b_add_BG == true && t.theObj != NULL )
	{
		curr_stageFile->changed = true;
		curr_stageFile->obj_container->Add_SubObj( t.theObj );
		t.theObj = NULL;
 		VIEW__Update();
	}
	else if(b_add_BG == false && t.b_chg == true )
	{
		curr_stageFile->changed = true;
 		VIEW__Update();
	}

	return true;
}


//****************************************************

bool LevelEditorFrame::BG_Add()
{
	return false;
}


//****************************************************
bool 
LevelEditorFrame::BG_Order()
{
	// Check if there is some background
	if( curr_stageFile->GetProperty( wxT("Background") ) == NULL )
	{
		wxMessageBox(wxT("First configure a Background for this stage !")
		, wxT("Impossible")
				  , wxICON_HAND |wxOK, this );
		return false;
	}
	
	myWnd_EditBackground t( this
			,(ob_stage*) curr_stageFile->obj_container
			, curr_stage_declaration
			, curr_set
			);
			
	// make a history group
	theHistoryManager.GroupStart(wxT("Edit Background" ));

	t.ShowModal();
	
	// close the history group
	theHistoryManager.GroupEnd();
	
	if( t.b_changed == true )
		curr_stageFile->changed = true;
		
	return true;
}


//****************************************************
void
LevelEditorFrame::BG_Test()
{
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
		return;

	ob_stage* _stage = (ob_stage*) curr_stageFile->obj_container;

	//---------------------------------------
	// Create the window
	myWnd_SceneTest *wSceneTest = new myWnd_SceneTest( this
			, _stage, curr_stage_declaration, curr_set );
	
	//---------------------------------------
	// Affect background objects
	size_t nb_layers = 0;
	ob_BG_Layer** oLayers = (ob_BG_Layer**) 
			_stage->GetSubObjectS_ofType( OB_TYPE_BG_LAYER, nb_layers );
	if( oLayers != NULL )
		wSceneTest->Set_Background( oLayers, nb_layers );
	
	size_t nb_panels = 0;
	ob_stage_panel** oPanels = _stage->Get_Panels( nb_panels );
	if( oPanels != NULL )
		wSceneTest->Set_Panels( oPanels, nb_panels );
	
	size_t nb_front_panels = 0;
	ob_front_panel** oFrontPanels = (ob_front_panel**) 
			_stage->GetSubObjectS_ofType( OB_TYPE_FRONT_PANEL, nb_front_panels );
	if( oFrontPanels != NULL )
		wSceneTest->Set_Front_Panels( oFrontPanels, nb_front_panels );
	
	//---------------------------------------
	// Launching Window
	wSceneTest->ShowModal();
	
	//---------------------------------------
	//Cleanings
	delete wSceneTest;
	
	if( oLayers != NULL )
		delete[] oLayers;
	if( oPanels != NULL )
		delete[] oPanels;
	if( oFrontPanels != NULL )
		delete[] oFrontPanels;
	
	return;
}

//****************************************************

void LevelEditorFrame::Panel_ResetMenuList()
{
	Menu_ResetListItem( ID_SCENE_MENU_PANELS, ID_SCENE_PANELS_LIST_START );
	menuBar->FindItem(ID_SCENE_PANELS_EDIT_ORDER)->Enable( false );
}


//****************************************************

void LevelEditorFrame::Panel_AddToMenu( const wxString& _name, int ind, bool b_quiet )
{
	if( Menu_AddListItem( ID_SCENE_MENU_PANELS, ID_SCENE_PANELS_LIST_START, _name, false, ind ) )
	{
		// Enable background order
		wxMenuItem *_it = menuBar->FindItem(ID_SCENE_PANELS_EDIT_ORDER);
		if( _it == NULL )
		{
			wxMessageBox( wxT("Can't find MenuItem Panels Order  !!")
						  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
			return;
		}
		else
			_it->Enable( true );
	}
}


//****************************************************
void 
LevelEditorFrame::Panel_Reload()
{
	Panel_ResetMenuList();

	size_t nb_pans;
	ob_stage_panel** pans = 
			(ob_stage_panel**) curr_stageFile->obj_container->GetSubObjectS_ofType( OB_TYPE_STAGE_PANEL, nb_pans );

	for( size_t i = 0; i < nb_pans; i++ )
		Panel_AddToMenu(pans[i]->GetName() );

	if( pans != NULL )
		delete[] pans;
}


//****************************************************

bool LevelEditorFrame::Panel_Add()
{
	if( curr_stageFile == NULL )
	{
		wxMessageBox( wxT("NO current stage file selected !!"), wxT("BUG BUG !") );
		return false;
	}

	myWnd_PanelProperties t( leFrame, NULL );
	t.SetTitle( wxT("New Panel" ));
	
	// make a history group
	theHistoryManager.GroupStart(wxT("Add new Panel" ));
	
	t.ShowModal();

	// close the history group
	theHistoryManager.GroupEnd();
	
	if( t.theObj != NULL )
	{
		curr_stageFile->obj_container->Add_SubObj( t.theObj );
		curr_stageFile->changed = true;
		Panel_Reload();
		return true;
	}
	return false;
}


//****************************************************
bool 
LevelEditorFrame::Panel_Order()
{
	if( curr_stageFile == NULL )
	{
		wxMessageBox( wxT("NO current stage file selected !!"), wxT("BUG BUG !") );
		return false;
	}

ob_object* obj_order = curr_stageFile->GetProperty( wxT("order") );

	// If no order currently exist
	if( obj_order == NULL )
	{
		// Check if there some panels
		size_t nb_pans;
		ob_stage_panel** pans = (ob_stage_panel**) curr_stageFile->obj_container->GetSubObjectS_ofType( OB_TYPE_STAGE_PANEL, nb_pans );
		if( pans != NULL )
			delete[] pans;

		if( nb_pans == 0 )
		{
			wxMessageBox( wxT("No panel exist in the current Stage File\nCreate some before editing the order"),
					  wxT("Problem"), wxOK | wxICON_EXCLAMATION, leFrame );
			return false;
		}

/*		// Create a trivial "order" object
		wxString str_order;
		for( size_t i = 0; i  < nb_pans; i++ )
		{
			str_order +=  wxChar('a' + i );
		}
		obj_order = new ob_object( "order", str_order );
		curr_stageFile->obj_container->Add_SubObj( obj_order );*/
		curr_stageFile->changed = true;
	}

	// Launch the panel order config window
	myWnd_EditPanelOrder t( leFrame, (ob_stage*) curr_stageFile->obj_container );
	
	// make a history group
	theHistoryManager.GroupStart(wxT("Edit Panels order") );
	
	t.ShowModal();

	// close the history group
	theHistoryManager.GroupEnd();
	
	if( t.b_chg )
	{
		curr_stageFile->changed = true;
		stageView->PANELS__Reload();
		VIEW__Update();
	}
	return true;
}


//****************************************************
void 
LevelEditorFrame::FrontPanel_ResetMenuList()
{
	Menu_ResetListItem( ID_SCENE_FRONTPANEL, ID_SCENE_FRONTPANEL_LIST_START );
//	menuBar->FindItem(ID_SCENE_FRONTPANEL_ORDER)->Enable( false );
}


//****************************************************
void 
LevelEditorFrame::FrontPanel_AddToMenu( const wxString& _name, int ind, bool b_quiet )
{
	if( Menu_AddListItem( ID_SCENE_FRONTPANEL, ID_SCENE_FRONTPANEL_LIST_START, _name, false, ind ) )
	{
		// Enable editing
		wxMenuItem *_it = menuBar->FindItem(ID_SCENE_FRONTPANEL_ORDER);
		if( _it == NULL )
		{
			wxMessageBox( wxT("Can't find MenuItem FrontPanels Edit !!")
						  , wxT("BUG"), wxOK | wxICON_EXCLAMATION, leFrame );
			return;
		}
		else
			_it->Enable( true );
	}
}


//****************************************************
void 
LevelEditorFrame::FrontPanel_Reload()
{
	FrontPanel_ResetMenuList();

	size_t nb_pans;
	ob_stage_panel** pans = (ob_stage_panel**) curr_stageFile->obj_container->GetSubObjectS_ofType( OB_TYPE_FRONT_PANEL, nb_pans );

	for( size_t i = 0; i < nb_pans; i++ )
		FrontPanel_AddToMenu(pans[i]->GetName() );

	if( pans != NULL )
		delete[] pans;
}

//****************************************************
void 
LevelEditorFrame::FrontPanel_Edit_a_panel( int ind_panel, bool b_quiet )
{
	if( curr_stageFile == NULL )
	{
		wxMessageBox( wxT("NO current stage file selected !!"), wxT("BUG BUG !") );
		return;
	}

	size_t nb_panels;
	ob_front_panel** pans = (ob_front_panel**)
				curr_stageFile->obj_container->GetSubObjectS_ofType( OB_TYPE_FRONT_PANEL, nb_panels );

	if( nb_panels == 0 )
	{
		wxMessageBox( wxT("No FrontPanels avalaible in current stage file selected !!"), wxT("BUG BUG !") );
		return;
	}

	if( (int) nb_panels <= ind_panel )
	{
		wxMessageBox( wxT("Ind panel out of range !!"), wxT("BUG BUG !") );
		delete[] pans;
		return;
	}

	ob_front_panel* thePan = pans[ind_panel];
	delete[] pans;

	myWnd_FrontPanelProperties t( leFrame, thePan );

	// make a history group
	theHistoryManager.GroupStart(wxT("Edit a Panel" ));
	
	t.ShowModal();

	// close the history group
	theHistoryManager.GroupEnd();
	
	if( t.b_chg )
	{
		curr_stageFile->changed = true;
		FrontPanel_Reload();
	}
}


//****************************************************
void 
LevelEditorFrame::FrontPanel_Edit_All()
{
	if( curr_stageFile == NULL )
	{
		wxMessageBox( wxT("NO current stage file selected !!"), wxT("BUG BUG !") );
		return;
	}

	// Launch the panel order config window
	wnd_EditFrontPanels t( leFrame
			, (ob_stage*) curr_stageFile->obj_container
			, curr_stage_declaration
			, curr_set );

	// make a history group
	theHistoryManager.GroupStart(wxT("Edit the Frontpanels" ));
	
	t.ShowModal();
	
	// close the history group
	theHistoryManager.GroupEnd();
	
	if( t.b_changed == true )
		curr_stageFile->changed = true;
	FrontPanel_Reload();
	
	return;
}

//****************************************************
void 
LevelEditorFrame::SET__Select_LastSession()
{
	if( nb_set <= 0 )
		return;

	wxString last_set = ConfigEdit_Read( wxT("last_GameSet_selected"), wxString() );
	bool b_select_ok = false;
	if( last_set != wxString() )
		if( SET__Select( last_set, true ) )
			b_select_ok = true;

	if( ! b_select_ok )
		Disable( wxT("NO_SET_SELECTED" ));

}


//****************************************************

void LevelEditorFrame::SET__Save_LastSession( const wxString& _setName )
{
	ConfigEdit_Write( wxT("last_GameSet_selected"), _setName );
}

//****************************************************

void LevelEditorFrame::STAGE__Select_LastSession()
{
	if( nb_stage <= 0 )
		return;

	wxString _last = ConfigEdit_Read( wxT("last_Stage_selected"), wxString() );
	bool b_select_ok = false;
	if( _last != wxString() )
		if( STAGE__Select( _last, true ))
			b_select_ok = true;

	if( ! b_select_ok )
		Disable( wxT("NO_STAGE_SELECTED") );
}


//****************************************************

void LevelEditorFrame::STAGE__Save_LastSession( const wxString& _stageName )
{
	ConfigEdit_Write( wxT("last_Stage_selected"), _stageName );
}

//****************************************************

void LevelEditorFrame::SET__New()
{
	myWnd_SETProperties t( leFrame, NULL, wxT("New Set Properties") );
	
	// No history for these ones
	theHistoryManager.Set_State( false );
	
	t.ShowModal();

	if( t.theObj != NULL && fileLevels != NULL && fileLevels->obj_container != NULL)
	{
		// Check if a set have the same name
		for( size_t i = 0; i < nb_set; i++ )
			if( 	levels_sets[i]->GetSetName().Upper()
				== 
				((ob_StagesSet*)t.theObj)->GetSetName().Upper() 
				)
			{
				wxMessageBox( wxT("A Set with the same name already exists !")
				,wxT("Impossible")
					, wxICON_EXCLAMATION | wxOK, leFrame );
				t.theObj->Rm();
				t.theObj = NULL;
				return;
			}
		
		fileLevels->obj_container->Add_SubObj( t.theObj );
		fileLevels->changed = true;
		SET__Load_List( true );
		SET__Select( ((ob_StagesSet*)t.theObj)->GetSetName() );
		t.theObj = NULL;
	}

	theHistoryManager.Set_State( true );
}


//****************************************************

void LevelEditorFrame::STAGE__New()
{
	// No history for these ones
	theHistoryManager.Set_State( false );
	
	myWnd_STAGE_New t( leFrame );
	t.ShowModal();

	if( t.theObj != NULL )
	{
		fileLevels->changed = true;
		STAGE__Load_List( true );
		STAGE__Select( ((ob_StageDeclaration*)t.theObj)->GetStageName() );
	}

	theHistoryManager.Set_State( true );
}

//****************************************************
void 
LevelEditorFrame::Panel_Edit( int ind_menu, bool b_quiet)
{
	if( curr_stageFile == NULL )
	{
		wxMessageBox(wxT("NO current stage file selected !!"), wxT("BUG BUG !") );
		return;
	}

	size_t nb_panels;
	ob_stage_panel** pans = (ob_stage_panel**)
				curr_stageFile->obj_container->GetSubObjectS_ofType( OB_TYPE_STAGE_PANEL, nb_panels );

	if( nb_panels == 0 )
	{
		wxMessageBox( wxT("No panels avalaible in current stage file selected !!"), wxT("BUG BUG !") );
		return;
	}

	if( (int) nb_panels <= ind_menu )
	{
		wxMessageBox( wxT("Ind panel out of range !!"), wxT("BUG BUG !") );
		delete[] pans;
		return;
	}

	ob_stage_panel* thePan = pans[ind_menu];
	delete[] pans;

	myWnd_PanelProperties t( leFrame, thePan );

	// make a history group
	theHistoryManager.GroupStart(wxT("Edit a panel") );
	
	t.ShowModal();

	// close the history group
	theHistoryManager.GroupEnd();
	
	if( t.b_chg )
	{
		curr_stageFile->changed = true;
		Panel_Reload();
	}
}

//-------------------------------------------------------------------
obFileEntity*
LevelEditorFrame::Entity__Get( const wxString& item_name )
{
	for( size_t i =0; i< arr_entities__size; i++ )
	{
		if( arr_entities[i]->Name().Upper() == item_name.Upper() )
			return arr_entities[i];
	}
	return NULL;
}



//---------------------------------------------------------
void
LevelEditorFrame::EvtRedo(wxCommandEvent& event)
{
	if( theHistoryManager.Do() == false )
	{
		// Beep user
		wxBell();
	}
}

//---------------------------------------------------------
void
LevelEditorFrame::EvtUndo(wxCommandEvent& event)
{
	// Register ctrls states when undoing the present
	if( theHistoryManager.IsPresent() )
		Register_ControlsState_inHistory();
	if( theHistoryManager.UnDo() == false )
	{
		// Beep user
		wxBell();
	}
}

//---------------------------------------------------------
void
LevelEditorFrame::Menu_DoRedos__Update_Labels(list<History_Group*>::iterator it_group)
{
	wxMenuItem* t_redo = menuUnRedo->FindItem(ID_REDO);
	list<History_Group*>::iterator it_end =
				theHistoryManager.l_history_groups.end();
	if( t_redo != NULL )
	{
		if(theHistoryManager.it_now == it_end || it_group == it_end )
		{
			t_redo->SetItemLabel( wxT("Redo\tCTRL-SHIFT-Z"));
			t_redo->Enable( false );
		}
		else
		{
			t_redo->SetItemLabel( wxT("Redo - ") + (*it_group)->GetName() + wxT("\tCTRL-SHIFT-Z"));
			t_redo->Enable( true );
		}
	}
	
	wxMenuItem* t_undo = menuUnRedo->FindItem(ID_UNDO);
	if( t_undo != NULL )
	{
		if( theHistoryManager.IsFirstGroup( it_group )
			|| theHistoryManager.IsHistory_Empty() )
		{
			t_undo->SetItemLabel( wxT("Undo\tCTRL-Z"));
			t_undo->Enable( false );
		}
		else
		{
			list<History_Group*>::iterator it_t(it_group);
			it_t--;
			t_undo->SetItemLabel( wxT("Undo - ") + (*it_t)->GetName() + wxT("\tCTRL-Z"));
			t_undo->Enable( true );
		}
	}
}

//---------------------------------------------------------
void
LevelEditorFrame::Register_ControlsState_inHistory()
{
	if( theHistoryManager.IsUndoing() == true )
		return;

	// Get The Last History Element
	History_Elt* elt = theHistoryManager.Get_Last_HistoryElt();
	
	// no history elt, use the initital_datas of theHistoryManager
	if( elt == NULL )
	{
		if( theHistoryManager.initial_datas == NULL )
			theHistoryManager.initial_datas =(void*) new editor_level_states;
		Set_ControlsState_In( theHistoryManager.initial_datas );
	}
	else
	{
		if( elt->datas == NULL )
			elt->datas = (void*) new editor_level_states;
		Set_ControlsState_In( elt->datas );
	}
}


//---------------------------------------------------------anim_selected
void
LevelEditorFrame::Set_ControlsState_In( void* _elt_datas )
{
	editor_level_states* elt_datas = (editor_level_states*) _elt_datas;
	elt_datas->selected = leFrame->aSet_Selected;
}

//---------------------------------------------------------
void
LevelEditorFrame::Restore_ControlsState( void* _pStates )
{
	editor_level_states* pStates = (editor_level_states*) _pStates;

	aSet_Selected = pStates->selected;
	OBJECT__ListSelected__UpdateNoteBook();
	stageView->OBJECTS__Refresh();
	VIEW__Update();
}

//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(LevelEditorFrame, wxFrame)
	EVT_CLOSE(LevelEditorFrame::EvtClose)
	EVT_ACTIVATE(LevelEditorFrame::EvtActivate)
	EVT_SIZE(LevelEditorFrame::EvtResize)

	EVT_MENU(ID_STAGE_SAVE, LevelEditorFrame::Evt_STAGE__Save )
	EVT_MENU(ID_STAGE_RELOAD, LevelEditorFrame::Evt_STAGE__Reload )
	EVT_MENU(ID_OPEN_IN_EDITOR,  LevelEditorFrame::Evt_OpenStageInEditor )
	EVT_MENU(ID_STAGE_NEW, LevelEditorFrame::Evt_STAGE__New )
	EVT_MENU(ID_SET_NEW, LevelEditorFrame::Evt_SET__New )

	EVT_MENU(ID_LEVELSTXT_SAVE,  LevelEditorFrame::Evt_SAVE_LevelsTxt )
	EVT_MENU(ID_STAGE_RELOAD_ALL,  LevelEditorFrame::Evt_ReloadAll )
	EVT_MENU(ID_STAGE_SAVE_ALL,  LevelEditorFrame::Evt_SaveAll )
	EVT_MENU(ID_OPEN_LEVELSTXT_IN_EDITOR,  LevelEditorFrame::Evt_OpenLevelsInEditor )
	EVT_MENU(ID_BACK_TO_STARTFRAME,  LevelEditorFrame::Evt_Quit )

	EVT_MENU(ID_SET_EDIT_SETTINGS, LevelEditorFrame::Evt_SET__Edit_Settings )
	EVT_MENU(ID_SET_EDIT_EXTENDED_PROPS, LevelEditorFrame::Evt_SET__Edit_ExtendedSettings )
	EVT_MENU_RANGE(ID_SET_LIST_START, ID_SET_LIST_START+ 48, LevelEditorFrame::Evt_SET__Select)

	EVT_MENU(ID_STAGE_EDIT_DECLARE_SETTINGS, LevelEditorFrame::Evt_STAGE__Edit_Declare_Settings )
	EVT_MENU(ID_STAGE_EDIT_SETTINGS, LevelEditorFrame::Evt_STAGE__Edit_Settings )
	EVT_MENU(ID_STAGE_EDIT_EXTENDED_PROPS, LevelEditorFrame::Evt_STAGE__Edit_ExtendedSettings )
	EVT_MENU_RANGE(ID_STAGE_LIST_START, ID_STAGE_LIST_START+ 89, LevelEditorFrame::Evt_STAGE__Select)
	EVT_MENU(ID_STAGE_LOADED_ENTITIES, LevelEditorFrame::Evt_LoadedEntities )

	EVT_MENU(ID_BACKGROUND, LevelEditorFrame::Evt_BACKGROUND_Click )
	EVT_MENU(ID_SCENE_BG_ADD, LevelEditorFrame::Evt_BGLayer_Add )
	EVT_MENU(ID_SCENE_BG_EDIT_ORDER, LevelEditorFrame::Evt_BGLayer_Order )
//	EVT_MENU_RANGE(ID_SCENE_BG_LIST_START, ID_SCENE_BG_LIST_START+ 39, LevelEditorFrame::Evt_BGLayer_Click)
	EVT_MENU(ID_SCENE_TEST, LevelEditorFrame::Evt_BGTest )

	EVT_MENU(ID_SCENE_PANELS_ADD, LevelEditorFrame::Evt_Panel_Add )
	EVT_MENU(ID_SCENE_PANELS_EDIT_ORDER, LevelEditorFrame::Evt_Panel_Order )
	EVT_MENU_RANGE(ID_SCENE_PANELS_LIST_START, ID_SCENE_PANELS_LIST_START+ 39, LevelEditorFrame::Evt_Panel_Click)

	EVT_MENU(ID_SCENE_FRONTPANEL_ORDER, LevelEditorFrame::Evt_FrontPanels_Edit)
	EVT_MENU_RANGE(ID_SCENE_FRONTPANEL_LIST_START, ID_SCENE_FRONTPANEL_LIST_START+ 39, LevelEditorFrame::Evt_FrontPanel_Click)

	EVT_MENU(ID_OBJECT_COPY, LevelEditorFrame::Evt_ObjectDuplicate)
	EVT_MENU(ID_OBJECT_SUPP, LevelEditorFrame::Evt_ObjectDelete)
	EVT_MENU(ID_OBJECT_PROPERTIES, LevelEditorFrame::Evt_ObjectProps)
	EVT_MENU(ID_OBJECT_SORT_AT_OBJECTS, LevelEditorFrame::Evt_Object_SortAT)

	EVT_MENU(ID_UNDO,  LevelEditorFrame::EvtUndo)
	EVT_MENU(ID_REDO,  LevelEditorFrame::EvtRedo)

	EVT_MENU(ID_HELP_PLEASE, LevelEditorFrame::Evt_Help)
	
END_EVENT_TABLE()




//****************************************************
//*********** HISTORY FUNCTIONS **********************
//****************************************************
//-------------------------------------------------------------------
int 
level__onUndo(
			  list<History_Group*>::iterator it_group
			, History_Elt*  _elt
	   )
{
	if( leFrame == NULL )
		return 0;	// By default undo only one group
	
	// Check if it's the first elt of the group
	if( (*it_group)->l_elts.front() == _elt )
	{
		// Check if it's the real first group of the history
		if( theHistoryManager.IsFirstGroup(it_group) == true )
		{
			// So previous controls state are in this elt
			if(theHistoryManager.initial_datas == NULL )
				wxMessageBox( wxT("BUG !!\nlevel__onUndo()\nFirst elt has no state setted !!!\n") );
			else
				leFrame->Restore_ControlsState( theHistoryManager.initial_datas );
		}
		
		// So the states are in the last elt of the previous group
		else
		{
			// Get the group-related-most-recent ctrls states
			void* _datas = Get_Prev_Ctrl_States( it_group, false );
			if( _datas != NULL )
				leFrame->Restore_ControlsState( _datas );
		}
		
		// AnyWay, must update the Do/Redo menu Labels
		leFrame->Menu_DoRedos__Update_Labels( it_group );
	}

	return 0;	// undo only one group
}

//----------------------------------------------------------------
int 
level__onDo(
			  list<History_Group*>::iterator it_group
			, History_Elt*  _elt
	   )
{
	if( leFrame == NULL )
		return 0;	// By default only redo only one group
	
	// Check if it's the last elt of the group
	if( (*it_group)->l_elts.back() == _elt )
	{
		// Get the group-related-most-recent ctrls states
		void* _datas = Get_Prev_Ctrl_States( it_group, true );
		if( _datas != NULL )
			leFrame->Restore_ControlsState( _datas );
		
		// AnyWay, must update the Do/Redo menu Labels
		list<History_Group*>::iterator it_t(it_group);
		it_t++;
		leFrame->Menu_DoRedos__Update_Labels( it_t );

	}
	
	return 0;	// redo only one group
}

//----------------------------------------------------------------
void 
level__onNoMoreUndo(bool b_state)
{
	if( leFrame == NULL )
		return;
	
	leFrame->Menu_DoRedos__Update_Labels( theHistoryManager.it_now );
}

//----------------------------------------------------------------
void 
level__onNoMoreDo(bool b_state)
{
	if( leFrame == NULL )
		return;

	leFrame->Menu_DoRedos__Update_Labels( theHistoryManager.it_now );
}

//----------------------------------------------------------------
void 
level__onDeleteHistory_data(void*&  _elt_datas)
{
	delete (editor_level_states*) _elt_datas;
}


//----------------------------------------------------------------
int  
level__onEltAdd(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   )
{
	return 1; // accept to add in the group
}

static
void
May_RM_Duplicates( list<History_Group*>::iterator it_grp_prev
			,list<History_Group*>::iterator it_grp_last
			,list<History_Elt*>::iterator   it_t_last_elt
			,bool b_move_to_prev_grp )
{
	list<History_Elt*>::iterator it_t0, it_t;

	// try to merge the elt and his successors
	while( it_t_last_elt != (*it_grp_last)->l_elts.end() )
	{
		list<History_Elt*>::iterator it_t1(it_t_last_elt);

		//--------------------------------------------
		// t0 aim the first elt in the prev grp
		it_t0 = (*it_grp_prev)->l_elts.begin();

		//--------------------------------------------
		// Find out if it can be a dragging matching pattern

		// can be a token_change
		if(  (*it_t1)->type == History_Elt::H_TOKEN_CHANGE )
		{
			// must find a match in the prev group
			bool b_merged = false;
			while(   it_t0 != (*it_grp_prev)->l_elts.end()
				&& it_t0 != it_t1
				)
			{
				if(      (*it_t0)->type == History_Elt::H_TOKEN_CHANGE 
					&& (*it_t0)->o == (*it_t1)->o
					&& (*it_t0)->token_pos == (*it_t1)->token_pos
				  )
				{
					// must preserve the previous token val
					(*it_t1)->token_val = (*it_t0 )->token_val;
					
					// must delete prev hist elt
					delete (*it_t0 );
					(*it_grp_prev)->l_elts.erase( it_t0 );
					it_t_last_elt++;
					
					if( b_move_to_prev_grp == true )
					{
						// put this one to the end of the prev grp
						(*it_grp_prev)->l_elts.push_back( (*it_t1 ) );
						// and remove it from the last grp
						(*it_grp_last)->l_elts.erase( it_t1 );
					}
					b_merged = true;
					break;
				}
				
				it_t0++;
			}
			
			if( b_merged == false )
				return;
		}
		
		// can be a RM-INSERT 
		else
		{
			if( (*it_t1)->type != History_Elt::H_OBJ_RM )
				return;
			ob_object* subobj = (*it_t1)->o;

			it_t1++;
			if( 	   it_t1 == (*it_grp_last)->l_elts.end()
				|| (*it_t1)->type != History_Elt::H_OBJ_INSERT
				|| (*it_t1)->o != subobj
				)
				return;
			it_t1--;

			// must find a match in the prev group
			bool b_merged = false;
			while(   it_t0 != (*it_grp_prev)->l_elts.end()
				&& it_t0 != it_t1
				)
			{
				if(      (*it_t0)->type == History_Elt::H_OBJ_RM 
					&& (*it_t0)->o == subobj
				  )
				{
					it_t0++;
					if(      it_t0 != (*it_grp_prev)->l_elts.end()
						&& (*it_t0)->type == History_Elt::H_OBJ_INSERT
						&& (*it_t0)->o == subobj
					)
					{
						// must delete prev hist elt
						it_t = it_t0;
						it_t0++;
						
						// erase the INSERT
						delete (*it_t );
						(*it_grp_prev)->l_elts.erase(it_t);
						
						// erase the RM
						it_t = it_t0;
						it_t--;
						delete (*it_t );
						(*it_grp_prev)->l_elts.erase(it_t);
						
						it_t_last_elt++;
						it_t_last_elt++;
						
						// put this two at the end of the prev grp
						if( b_move_to_prev_grp == true )
						{
							it_t = it_t1;
							it_t1++;
							
							// Move the INSERT
							(*it_grp_prev)->l_elts.push_back( (*it_t ) );
							(*it_grp_last)->l_elts.erase( it_t );
							
							// Move the RM
							(*it_grp_prev)->l_elts.push_back( (*it_t1 ) );
							(*it_grp_last)->l_elts.erase( it_t1 );
						}
						
						b_merged = true;
						break;
					}
					else
						it_t0--;
				}
				
				it_t0++;
			}
			
			if( b_merged == false )
				return;
		}
		
		if( (*it_grp_last)->l_elts.empty() == true )
			break;
	}
		
// 	cout<<"MERGING DONE !!"<<endl;
	
	// here, all elts of the last group have been merged
	if( b_move_to_prev_grp == true )
		theHistoryManager.Group__DeleteLast();
}


//----------------------------------------------------------------
void 
level__onAfterEltAdded()
{
	if( theHistoryManager.l_history_groups.size() < 1 )
		return;

	list<History_Group*>::iterator
			it_grp_last(theHistoryManager.l_history_groups.end());
	it_grp_last--;
	
	if( (*it_grp_last)->l_elts.empty() == true )
		return;
	
	//----------------------------------------------------
	// Rm duplicates in curr grp
	list<History_Elt*>::iterator  it_t1((*it_grp_last)->l_elts.end());
	it_t1--;
	
	if( (*it_t1)->type == History_Elt::H_TOKEN_CHANGE )
	{
		May_RM_Duplicates( it_grp_last
					,it_grp_last
					,it_t1
					, false
					);
	}
	else if( (*it_t1)->type == History_Elt::H_OBJ_INSERT 
		&&   it_t1  != (*it_grp_last)->l_elts.begin())
	{
		it_t1--;
		May_RM_Duplicates( it_grp_last
					,it_grp_last
					,it_t1
					,false
					);
	}

	//----------------------------------------------------
	// Try to merge with prev grp
	if( theHistoryManager.l_history_groups.size() < 2 )
		return;
	
	// First check if the time distance between the 2 last groups is small enough
	list<History_Group*>::iterator it_grp_prev(it_grp_last);
	it_grp_prev--;


	// it Not enough elements in the 2 last groups
	if( 	   (*it_grp_prev)->l_elts.empty() == true
		|| (*it_grp_last)->l_elts.empty() == true
	  )
		return;
	
	it_t1 = (*it_grp_last)->l_elts.begin();
	list<History_Elt*>::iterator  it_t0((*it_grp_prev)->l_elts.end());

	// t0 aim the last elt of the prev grp
	it_t0--;
	
	// Not enough time distance to merge
	if( (*it_t1)->theDate  > (*it_t0)->theDate + D_MAX_INTER_MERGE_DELAY )
		return;

	// t1 aim the next elt to try a merge in the last grp
	it_t1 = (*it_grp_last)->l_elts.begin();

	May_RM_Duplicates( it_grp_prev
				,it_grp_last
				,it_t1
				, true
				);
}
	
