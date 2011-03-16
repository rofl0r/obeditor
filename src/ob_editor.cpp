#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <list>

#ifdef MTRACE
	#include <mcheck.h>
#endif

#include "common__tools.h"
#include "common__mod.h"
#include "common__ob_controls.h"
#include "ob_editor.h"
#include "levels__classes.h"

using namespace std;


// Globals Vars
MyApp* theApp;
StartFrame *startFrame;
EntityFrame *entityFrame;
wxString obeditor_version = _T(PACKAGE_VERSION);
LevelEditorFrame *leFrame;


// Event table
BEGIN_EVENT_TABLE(StartFrame, wxFrame)
    EVT_MENU(ID_Quit,  StartFrame::OnQuit)
    EVT_MENU(ID_About, StartFrame::OnAbout)
	EVT_CLOSE(StartFrame::evtClose)
	EVT_ACTIVATE(StartFrame::EvtActivate)
END_EVENT_TABLE()


IMPLEMENT_APP(MyApp)

//--------------------------------------------------------
extern void closeAudio();
void Init__ObProps();
void UnInit__ObProps();
//--------------------------------------------------------


/** Init of the App.
 *
 * @param s : no param like this !
 * @see StartFrame
 */
bool MyApp::OnInit()
{
	
	theApp = this;
	config = new wxConfig(wxT("obeditor" ));
	theModsList = new ModsList();
	fileModels = NULL;
	entityFrame = NULL;
	leFrame = NULL;
	curr_mod = NULL;
	config->Read(wxT("/auto_load_last_state"), &(theApp->auto_load), true);

	wxString tt(wxT("bingo"));

	// Load wxWidgets image handlers
	wxInitAllImageHandlers();

	// VARIABLES INITIALISATION
	dirRessources.Assign(wxString(argv[0]) );
	dirRessources.AppendDir( wxT("resources" ));
	noImg = new wxImage( GetRessourceFile_String( wxT("noimage.jpg" )) );
	if( ! noImg->IsOk() )
	{
		return false;
	}

	// Props init
	Init__ObProps();

	// Constraints init
	constraints_init();

#ifdef MTRACE
	mtrace();
#endif
	
	startFrame = new StartFrame( wxT("OpenBor Project Editor - ") + obeditor_version
								 , wxPoint(50,50), wxSize(450,340) );
								 Frame_RestorePrevCoord( startFrame, wxT("startFrame"));
    startFrame->Show( true );
    SetTopWindow( startFrame );

    return true;
}

int MyApp::OnExit()
{
#ifdef MTRACE
	muntrace();
#endif
	UnInit__ObProps();
	delete config;
	closeAudio();
	return 0;
}

//---------------------------------------------------------------------
void 
Init__ObProps()
{
	ob_property* t;
	
	// Coords
	ob_props[wxT("SP:coords=0")] =
	new ob_property(  wxT("SP:coords"),0
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	// Coords
	ob_props[wxT("SP:coords=1")] =
	new ob_property(  wxT("SP:coords"),1
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
	// Coords
	ob_props[wxT("SP:coords=2")] =
	new ob_property(  wxT("SP:coords"),2
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_TRUNCATE
			);

	// AT
	ob_props[wxT("SP:at=0")] =
	new ob_property(  wxT("SP:at"),0
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
			
	// spawn map
	ob_props[wxT("SP:map=0")] =
	new ob_property(  wxT("SP:map"),0
			, PROPTYPE_ENUMS
			, wxT("0"), OBPROP_DEL_TAG
			);
			
	// spawn alias
	ob_props[wxT("SP:alias=0")] =
	new ob_property(  wxT("SP:alias"),0
			, PROPTYPE_STRING_NO_WS
			, wxString(), OBPROP_DEL_TAG
			);
			
	// spawn alias
	ob_props[wxT("SP:flip=0")] =
	new ob_property(  wxT("SP:flip"),0
			, PROPTYPE_BOOL
			, wxT("0"), OBPROP_DEL_TAG
			);
			
			
	// spawn health
	t = new ob_property(  wxT("SP:health"),0
		, PROPTYPE_NUMBER
		, wxString(), OBPROP_DEL_TAG | OBPROP_SETDEF_IFEMPTY
		);
	// default value is the one in the reference entity as health tag
	t->referer_path = ob_token_path(wxT("health"), 0 );
	ob_props[wxT("SP:health=0")] = t;
			
	ob_props[wxT("SP:2phealth=0")] =
	new ob_property(  wxT("SP:2phealth"),0
			, PROPTYPE_NUMBER
			, wxString(), OBPROP_DEL_TAG
			);
			
	ob_props[wxT("SP:3phealth=0")] =
	new ob_property(  wxT("SP:3phealth"),0
				, PROPTYPE_NUMBER
				, wxString(), OBPROP_DEL_TAG
				);

	ob_props[wxT("SP:4phealth=0")] =
	new ob_property(  wxT("SP:4phealth"),0
			, PROPTYPE_NUMBER
			, wxString(), OBPROP_DEL_TAG
			);

	// spawn Agression
	t =	new ob_property(  wxT("SP:aggression"),0
			, PROPTYPE_RELATIVENUMBER
			, wxString(), OBPROP_DEL_TAG | OBPROP_SETDEF_IFEMPTY
			);
	// default value is the one in the reference entity as aggression tag
	t->referer_path = ob_token_path(wxT("aggression"), 0 );
	ob_props[wxT("SP:aggression=0")] =t;

	// spawn Score
	t =	new ob_property(  wxT("SP:score"),0
			, PROPTYPE_RELATIVENUMBER
			, wxString(), OBPROP_DEL_TAG | OBPROP_SETDEF_IFEMPTY
			);
	// default value is the one in the reference entity as score tag
	t->referer_path = ob_token_path(wxT("score"), 0 );
	ob_props[wxT("SP:score=0")] = t;

	// spawn mp
	t =	new ob_property(  wxT("SP:mp"),0
			, PROPTYPE_RELATIVENUMBER
			, wxString(), OBPROP_DEL_TAG | OBPROP_SETDEF_IFEMPTY
			);
	// default value is the one in the reference entity as mp tag
	t->referer_path = ob_token_path(wxT("mp"), 0 );
	ob_props[wxT("SP:mp=0")] = t;
	
	// spawn itemhealth
	ob_props[wxT("SP:itemhealth=0")] =
	new ob_property(  wxT("SP:itemhealth"),0
			, PROPTYPE_NUMBER
			, wxString(), OBPROP_DEL_TAG
			);
	
	ob_props[wxT("SP:itemalias=0")] =
	new ob_property(  wxT("SP:itemalias"),0
			, PROPTYPE_STRING_NO_WS
			, wxString(), OBPROP_DEL_TAG
			);
	
	ob_props[wxT("SP:itemmap=0")] =
	new ob_property(  wxT("SP:itemmap"),0
			, PROPTYPE_ENUMS
			, wxT("0"), OBPROP_DEL_TAG
			);
	
	ob_props[wxT("SP:itemalpha=0")] =
	new ob_property(  wxT("SP:itemalpha"),0
			, PROPTYPE_ENUMS
			, wxT("-1"), OBPROP_DEL_TAG
			);
	
	ob_props[wxT("SP:itemalpha=0")] =
	new ob_property(  wxT("SP:itemalpha"),0
			, PROPTYPE_ENUMS
			, wxT("-1"), OBPROP_DEL_TAG
			);
			
	//*************************************************************
	//*************************************************************
	//*************************************************************
	// Blockade pos
	ob_props[wxT("SP:blockade=0")] =
	new ob_property(  wxT("SP:blockade"),0
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
	
	// Group min
	ob_props[wxT("SP:group=0")] =
	new ob_property(  wxT("SP:group"),0
			, PROPTYPE_NUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
			
	// Group max
	ob_props[wxT("SP:group=1")] =
	new ob_property(  wxT("SP:group"),1
			, PROPTYPE_NUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	// scrollz min
	ob_props[wxT("SP:scrollz=0")] =
	new ob_property(  wxT("SP:scrollz"),0
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
	
	// scrollz max
	ob_props[wxT("SP:scrollz=1")] =
	new ob_property(  wxT("SP:scrollz"),1
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
	
	// light x
	ob_props[wxT("SP:light=0")] =
	new ob_property( wxT( "SP:light"),0
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
	
	// light z
	ob_props[wxT("SP:light=1")] =
	new ob_property(  wxT("SP:light"),1
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
	
	// setpalette
	ob_props[wxT("SP:setpalette=0")] =
	new ob_property(  wxT("SP:setpalette"),0
			, PROPTYPE_NUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);
	
	//*************************************************************
	//*************************************************************
	//*************************************************************
	// Player spawn coords
	ob_props[wxT("SP_spawnP:=0")] =
	new ob_property(  wxT("SP_spawnP:"),0
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_spawnP:=1")] =
	new ob_property(  wxT("SP_spawnP:"),1
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_spawnP:=2")] =
	new ob_property(  wxT("SP_spawnP:"),2
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);


	//*************************************************************
	//*************************************************************
	//*************************************************************
	// Wall coords
	ob_props[wxT("SP_wall:=0")] =
	new ob_property(  wxT("SP_wall:"),0
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_wall:=1")] =
	new ob_property(  wxT("SP_wall:"),1
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_wall:=2")] =
	new ob_property(  wxT("SP_wall:"),2
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_wall:=3")] =
	new ob_property(  wxT("SP_wall:"),3
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_wall:=4")] =
	new ob_property( wxT( "SP_wall:"),4
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_wall:=5")] =
	new ob_property( wxT( "SP_wall:"),5
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_wall:=6")] =
	new ob_property( wxT( "SP_wall:"),6
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

	ob_props[wxT("SP_wall:=7")] =
	new ob_property( wxT( "SP_wall:"),7
			, PROPTYPE_RELATIVENUMBER
			, wxT("0"), OBPROP_DONOTHING | OBPROP_SETDEF_IFEMPTY
			);

}

//---------------------------------------------------------------------
void 
UnInit__ObProps()
{
	while( ob_props.empty() == false )
	{
		std::map<wxString,ob_property*>::iterator it( ob_props.begin() );
		delete it->second;
		ob_props.erase(it);
	}
}

//**********************************************************************
StartFrame::StartFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
	startFrame = this;
	b_reset_entityFrame = false;
	b_reset_leFrame = false;
	frame_launched = wxString();

	// The Logger
	guiLog = NULL;
	logger = new myLog();
	myLog::SetActiveTarget(logger);

	//*********************************
	// SIZERS
	//*********************************

	// TOP Sizer
	wxBoxSizer *sizer_top = new wxBoxSizer( wxHORIZONTAL );

	// Project Sizer
	wxStaticBoxSizer *sizer_project = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Projects known") );
	sizer_top->Add( sizer_project,1,wxEXPAND );

	// Sizer Right part
	wxBoxSizer *sizer_right = new wxBoxSizer( wxVERTICAL);
	sizer_top->Add( sizer_right,1,wxEXPAND );

		// Loads options sizer
		wxStaticBoxSizer *sizer_loads = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Editors") );
		sizer_right->Add( sizer_loads,0 );

		// Sizers Tools
		sizer_right->AddStretchSpacer();;
		wxStaticBoxSizer *sizer_staticboxTools = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Tools" ));
		sizer_right->Add( sizer_staticboxTools,1,wxEXPAND );

		wxGridSizer *sizer_tools = new wxGridSizer( 2, 2);
		sizer_staticboxTools->Add( sizer_tools,0 );

	//*********************************
	//  projects panel
	//*********************************
	// Project Select ListBox
	lstBx_projects = new wxListBox(this, ID_LIST_PROJECTS );
	wxArrayString temp = theModsList->GetProjectsNames();
	for( size_t i =0; i< temp.GetCount(); i++)
		lstBx_projects->Append(temp[i]);
	int last_sel;
	config->Read( wxT("/Mod_last_selected"), &last_sel );
	if( last_sel>= 0 && last_sel < (int) temp.GetCount() )
		lstBx_projects->SetSelection( last_sel );
	sizer_project->Add( lstBx_projects,1, wxEXPAND );

	// Buttons Add Delete
	wxBoxSizer *sizer_project_btns = new wxBoxSizer( wxHORIZONTAL );
	sizer_project->Add( sizer_project_btns,0, wxEXPAND, 4 );

	wxButton *btn_add_project = new wxButton(this, ID_ADD_PROJECT, wxT("Add") );
	Connect( ID_ADD_PROJECT, wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(StartFrame::OnAddProject));
	sizer_project_btns->Add( btn_add_project,1, wxALL, 4 );

	wxButton *btn_del_project = new wxButton(this, ID_DEL_PROJECT, wxT("Delete") );
	Connect( ID_DEL_PROJECT, wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(StartFrame::OnDelProject));
	sizer_project_btns->Add( btn_del_project,1, wxALL, 4 );

	// Buttons Rename ChangeDir
	wxBoxSizer *sizer_projectMod_btns = new wxBoxSizer( wxHORIZONTAL );
	sizer_project->Add( sizer_projectMod_btns, 0, wxEXPAND, 4 );

	wxButton *btn_rename_project = new wxButton(this, ID_RENAME_PROJECT, wxT("Rename") );
	Connect( ID_RENAME_PROJECT, wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(StartFrame::OnRenameProject));
	sizer_projectMod_btns->Add( btn_rename_project,1, wxALL, 4 );

	wxButton *btn_chgDir_project = new wxButton(this, ID_CHGDIR_PROJECT, wxT("ChangeDir") );
	Connect( ID_CHGDIR_PROJECT, wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(StartFrame::OnChgDirProject));
	sizer_projectMod_btns->Add( btn_chgDir_project, 1, wxALL, 4 );


	//*********************************
	// Edits / options
	//*********************************

	// Load Stages button
	wxButton *btn_load_stages = new wxButton(this, ID_LOAD_STAGES, wxT("Load Stages") );
	btn_load_stages->SetSize( 150, wxDefaultCoord);
	Connect( ID_LOAD_STAGES, wxEVT_COMMAND_BUTTON_CLICKED,
    	  wxCommandEventHandler(StartFrame::OnLoadStages));
	sizer_loads->Add( btn_load_stages,0, wxALL,4 );

	// Edit Entity button
	wxButton *btn_edit_entity = new wxButton(this, ID_EDIT_ENTITIES, wxT("Edit Entities") );
	btn_edit_entity->SetSize( 150, wxDefaultCoord);
	Connect( ID_EDIT_ENTITIES, wxEVT_COMMAND_BUTTON_CLICKED,
  	    wxCommandEventHandler(StartFrame::OnEditEntities));
	sizer_loads->Add( btn_edit_entity,0, wxALL, 4 );

	// Auto Load check box
	chckbx_autoLoad = new wxCheckBox( this, ID_AUTO_LOAD, wxT("Auto-Load at startup") );
//	chckbx_autoLoad->SetSize( 150, wxDefaultCoord);
	Connect( ID_AUTO_LOAD, wxEVT_COMMAND_CHECKBOX_CLICKED,
  	    wxCommandEventHandler(StartFrame::OnCheckAutoLoad));
	sizer_loads->Add( chckbx_autoLoad,0 , wxALL, 4);

  	chckbx_autoLoad->SetValue( theApp->auto_load );


	//*********************************
	// Tools
	//*********************************
	// Tools Buttons

	wxButton *btn_repair_paths = new wxButton(this, ID_REPAIR_FILEPATHS, wxT("Repair Paths") );
	Connect( ID_REPAIR_FILEPATHS, wxEVT_COMMAND_BUTTON_CLICKED,
  	    wxCommandEventHandler(StartFrame::OnRepairWindaubePath));
	sizer_tools->Add( btn_repair_paths,0 );

	wxButton *btn_set_editor = new wxButton(this, ID_SET_EDITOR, wxT("Set Txt Editor") );
	Connect( ID_SET_EDITOR, wxEVT_COMMAND_BUTTON_CLICKED,
  	    wxCommandEventHandler(StartFrame::OnSetEditor));
	sizer_tools->Add( btn_set_editor,0 );

  	SetSizer( sizer_top );
	btn_load_stages->SetFocus();

	b_init = true;
	if( chckbx_autoLoad->GetValue() )
	{
		//AutoLoad subFrame
		config->Read(wxT("/autoload/frame"), &theApp->autoLoadFrame, wxString() );

		// Auto Start the last frame closed
		if( theApp->autoLoadFrame == wxT("edit_entity"))
		{
			wxCommandEvent dummy;
			OnEditEntities(dummy);
		}
		// Auto Start the last frame closed
		else if( theApp->autoLoadFrame == wxT("edit_stage"))
		{
			wxCommandEvent dummy;
			OnLoadStages(dummy);
		}
	}
	b_init = false;
}

//**********************************************
void StartFrame::EvtActivate( wxActivateEvent& event)
{
	if( b_reset_entityFrame )
	{
		b_reset_entityFrame = false;
		entityFrame = NULL;
		Show();
		frame_launched = wxString();
	}

	if( b_reset_leFrame )
	{
		b_reset_leFrame = false;
		leFrame = NULL;
		Show();
		frame_launched = wxString();
	}

	event.Skip();
}



//**********************************************
void StartFrame::OnCheckAutoLoad(wxCommandEvent& WXUNUSED(event))
{
	theApp->auto_load = chckbx_autoLoad->IsChecked();
	config->Write( wxT("/auto_load_last_state"), theApp->auto_load );
}


//**********************************************
void StartFrame::OnEditEntities(wxCommandEvent& WXUNUSED(event))
{
	if( frame_launched == wxString() )
		if( LoadCurrentProject_Models() )
		{
			frame_launched = wxT("EDIT_ENTITIES");
			ShowEntityFrame();
		}
}


//**********************************************
void StartFrame::ShowEntityFrame()
{
	if( entityFrame == NULL )
	{
		entityFrame = new EntityFrame( this, wxT("ObEditor-") + obeditor_version);
	}
}


//**********************************************
void StartFrame::ShowleFrame()
{
	if( leFrame == NULL )
	{
		leFrame = new LevelEditorFrame( this, wxT("ObEditor-") + obeditor_version );
	}
}


//**********************************************
bool StartFrame::LoadCurrentProject_Models()
{
	//***********************************
	// Get the project path
	wxFileName dataDir = dataDirPath;

	curr_mod = theModsList->GetCurrentMod();
	if( curr_mod == NULL )
	{
		wxMessageBox( wxT("Not a valid project !!")
		, wxT("Problem"), wxOK | wxICON_INFORMATION, this );
		return false;
	}
	
	dataDirPath = theModsList->GetSelectedProject();
	if( dataDirPath.GetFullPath() == wxString() )
	{
		wxMessageBox( wxT("The selected project does not have his Data directory setted !!")
		, wxT("Problem"), wxOK | wxICON_INFORMATION, this );
		dataDirPath = dataDir;
		return false;
	}


	//***********************************
	// Try to load the MODELS.TXT
	if( fileModels != NULL )
	{
		delete fileModels;
		fileModels = NULL;
	}

	wxFileName models_path(dataDirPath);
	models_path.AppendDir(dataDirPath.GetFullName());
	models_path.SetFullName(curr_mod->models_txt);
	fileModels = new obFileModels(models_path);

	if( fileModels->nb_lines <= 0 )
	{
		wxMessageBox( wxT("The selected project does not contain a valid 'models.txt' file") +
	    					models_path.GetFullPath()
	    					, wxT("Problem"), wxOK | wxICON_INFORMATION, this );
	    delete fileModels;
	    fileModels = NULL;
    	dataDirPath = dataDir;
	    return false;
	}


	return true;
}


//**********************************************

bool StartFrame::LoadCurrentProject_Levels()
{
	//***********************************
	// Try to load the MODELS.TXT
	if( ! LoadCurrentProject_Models() )
		return false;

	//***********************************
	// Get the project path
	wxFileName dataDir = dataDirPath;

	dataDirPath = theModsList->GetSelectedProject();
	if( dataDirPath.GetFullPath() == wxString() )
	{
		wxMessageBox( wxT("The selected project does not have his Data directory setted !!")
	                  , wxT("Problem"), wxOK | wxICON_INFORMATION, this );
		dataDirPath = dataDir;
		return false;
	}

	//***********************************
	// Try to load the LEVELS.TXT
	if( fileLevels != NULL )
	{
		delete fileLevels;
		fileLevels = NULL;
	}

	if( ! FileLevels_Reload(this) )
	{
		dataDirPath = dataDir;
		return false;
	}

//	MyLog( MYLOG_INFO, "DEBUG", fileLevels->obj_container->ToStr() );
	return true;
}


//**********************************************
void StartFrame::OnSetEditor(wxCommandEvent& event)
{
	wxFileDialog _tt( this, wxT("Choose your editor program"), wxString(), wxString(), wxT("*.*"), wxFD_FILE_MUST_EXIST );
	_tt.ShowModal();
  	if( ! config->Write( wxT("/startFrame/editor_program"), _tt.GetPath() ) )
		wxMessageDialog( this, wxT("Unable to write to the conf !!"), wxT("ERROR"), wxOK ).ShowModal();
}


//**********************************************
void StartFrame::OnLoadStages(wxCommandEvent& WXUNUSED(event))
{
	if( frame_launched == wxString() )
		if( LoadCurrentProject_Levels() )
		{
			frame_launched = wxT("EDIT_STAGE");
			ShowleFrame();
		}
}


//**********************************************
void StartFrame::OnAddProject(wxCommandEvent& WXUNUSED(event))
{
	wxString last_dir;
  	config->Read( wxT("/startFrame/lastDirProject"), &last_dir );

	wxDirDialog *dirChoose = new wxDirDialog( this, wxT("Choose the data dir of your project"), last_dir, wxDD_DIR_MUST_EXIST );
	if( dirChoose->ShowModal() != wxID_OK )
		return;

	// Save the last_dir choosen to the config
	wxFileName *temp = new wxFileName( dirChoose->GetPath());
  	if( ! config->Write( wxT("/startFrame/lastDirProject"), temp->GetPath() ) )
		wxMessageDialog( this, wxT("Unable to write the last_dir to the conf !!"), wxT("ERROR"), wxOK ).ShowModal();

	// Guess the project name
	wxString project_name = (temp->GetDirs()).Last();
	if( theModsList->ModExist(project_name) )
		wxMessageDialog( this, wxT("A project ") + project_name + wxT(" already exists !"), wxT("ERROR"), wxOK ).ShowModal();
	else
	{
		lstBx_projects->Append( project_name );
		theModsList->Append( project_name, dirChoose->GetPath() );
	}
}


//**********************************************
void StartFrame::OnDelProject(wxCommandEvent& WXUNUSED(event))
{
	wxString project_selected = lstBx_projects->GetStringSelection();
	if( project_selected == wxString() )
		return;
	lstBx_projects->Delete( lstBx_projects->GetSelection() );
	theModsList->Delete( project_selected );

}


//**********************************************
void StartFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close( true );
}


//**********************************************
void StartFrame::evtClose( wxCloseEvent& )
{
	// Save window position
	Frame_SaveCoord( this, wxT("startFrame"));

	// Save projects list
	wxArrayString arr_pjctsNames = lstBx_projects->GetStrings();
	for( size_t i =0; i <arr_pjctsNames.GetCount(); i++)
	{
		wxString t = wxT("/Mod") + IntToStr(i);
		config->Write( t , arr_pjctsNames[i] );
		config->Write( wxT("/Mod/") + theModsList->Get(i)->name +wxT("/dirData") , theModsList->Get(i)->dirData.GetFullPath() );
	}
	int last_sel = lstBx_projects->GetSelection();
	if( last_sel != wxNOT_FOUND )
		config->Write( wxT("/Mod_last_selected"), last_sel );
	else
		config->Write( wxT("/Mod_last_selected"), -1 );

	this->Destroy();
}


//**********************************************
void StartFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox( wxT("This is a wxWidgets' Hello world sample"),
		      wxT("About Hello World"), wxOK | wxICON_INFORMATION, this );
}


//**********************************************
void StartFrame::OnRenameProject(wxCommandEvent& event)
{
	//Chk if a project is selected
	int sel = lstBx_projects->GetSelection();
	if( sel == wxNOT_FOUND )
	{
		wxMessageBox( wxT("No project selected !"),
			      wxT("You're wrong"), wxOK | wxICON_INFORMATION, this );
	     return;
	}

	Mod *the_mod = theModsList->Get(lstBx_projects->GetSelection());
	wxTextEntryDialog *dial = new wxTextEntryDialog( this, wxT("New project name :"), wxT("Input"),
			the_mod->name );

	if( dial->ShowModal() != wxID_OK )
		return;

	wxString new_name = dial->GetValue();
	lstBx_projects->SetString( sel, new_name );
	the_mod->name = new_name;


}


//**********************************************
void StartFrame::OnChgDirProject(wxCommandEvent& event)
{
	//Chk if a project is selected
	int sel = lstBx_projects->GetSelection();
	if( sel == wxNOT_FOUND )
	{
		wxMessageBox( wxT("No project selected !"),
				  wxT("You're wrong"), wxOK | wxICON_INFORMATION, this );
	     return;
	}

	wxString last_dir;
  	config->Read( wxT("/startFrame/lastDirProject"), &last_dir );

	wxDirDialog *dirChoose = new wxDirDialog( this, wxT("Choose the new data dir forf your project"), last_dir, wxDD_DIR_MUST_EXIST );
	if( dirChoose->ShowModal() != wxID_OK )
		return;

	// Save the last_dir choosen to the config
	wxFileName *temp = new wxFileName( dirChoose->GetPath());
  	if( ! config->Write( wxT("/startFrame/lastDirProject"), temp->GetPath() ) )
		wxMessageDialog( this, wxT("Unable to write the last_dir to the conf !!"), wxT("ERROR"), wxOK ).ShowModal();

  	wxString t = dirChoose->GetPath();
	wxMessageDialog( this, wxT("New dir : ") + t, wxT("DEBUG"), wxOK ).ShowModal();
  	theModsList->Get(lstBx_projects->GetSelection())->SetDir( t );
}


//**********************************************
void StartFrame::OnRepairWindaubePath(wxCommandEvent& WXUNUSED(event))
{

	if( ! LoadCurrentProject_Levels() || fileModels == NULL || fileLevels == NULL || curr_mod == NULL )
    	return;

	// Empty the logKeeper
	EmptyLogKeeper();
	int count_repairs = 0;
	bool b_have_repair = false;

	//***********************************************
	// Repair some system files
	//***********************************************
	bool b_sys_files_to_repair = false;
	wxArrayString arr_sysfiles;
	wxArrayString arr_sysfiles_broken;

	arr_sysfiles.Add(wxT("data/" ));
	arr_sysfiles.Add(wxT("data/") + curr_mod->models_txt );
	arr_sysfiles.Add(wxT("data/") + curr_mod->levels_txt );
	arr_sysfiles.Add(wxT("data/") + curr_mod->bgs_dir );
	arr_sysfiles.Add(wxT("data/") + curr_mod->scenes_dir );
	
	arr_sysfiles.Add(wxT("data/sprites/") );
	arr_sysfiles.Add(wxT("data/sprites/shadow1.gif") );
	arr_sysfiles.Add(wxT("data/sprites/shadow2.gif") );
	arr_sysfiles.Add(wxT("data/sprites/shadow3.gif") );
	arr_sysfiles.Add(wxT("data/sprites/shadow4.gif") );
	arr_sysfiles.Add(wxT("data/sprites/shadow5.gif") );
	arr_sysfiles.Add(wxT("data/sprites/shadow6.gif") );
	arr_sysfiles.Add(wxT("data/sprites/arrow.gif") );
	arr_sysfiles.Add(wxT("data/sprites/arrowl.gif") );

	arr_sysfiles.Add(wxT("data/sprites/font.GIF") );
	arr_sysfiles.Add(wxT("data/sprites/font2.GIF") );
	arr_sysfiles.Add(wxT("data/sprites/font3.GIF") );
	arr_sysfiles.Add(wxT("data/sprites/font4.GIF") );
	arr_sysfiles.Add(wxT("data/sprites/font5.gif") );
	arr_sysfiles.Add(wxT("data/sprites/font6.gif") );
	arr_sysfiles.Add(wxT("data/sprites/font7.gif") );
	arr_sysfiles.Add(wxT("data/sprites/font8.gif") );

	arr_sysfiles.Add(wxT("data/sounds/") );
	arr_sysfiles.Add(wxT("data/sounds/go.wav") );
	arr_sysfiles.Add(wxT("data/sounds/beat1.wav") );
	arr_sysfiles.Add(wxT("data/sounds/block.wav") );
	arr_sysfiles.Add(wxT("data/sounds/fall.wav") );
	arr_sysfiles.Add(wxT("data/sounds/get.wav") );
	arr_sysfiles.Add(wxT("data/sounds/money.wav") );
	arr_sysfiles.Add(wxT("data/sounds/jump.wav") );
	arr_sysfiles.Add(wxT("data/sounds/indirect.wav" ));
	arr_sysfiles.Add(wxT("data/sounds/punch.wav" ));
	arr_sysfiles.Add(wxT("data/sounds/1up.wav" ));
	arr_sysfiles.Add(wxT("data/sounds/timeover.wav"));
	arr_sysfiles.Add(wxT("data/sounds/beep.wav" ));
	arr_sysfiles.Add(wxT("data/sounds/beep2.wav"));
	arr_sysfiles.Add(wxT("data/sounds/bike.wav" ));

	for( size_t i = 0; i < arr_sysfiles.Count(); i++ )
	{
		wxString needed_path = GetObFile( arr_sysfiles[i] ).GetFullPath();

		wxFileName _obsysfile( needed_path );
		if( _obsysfile.FileExists() || _obsysfile.DirExists() )
		{
			arr_sysfiles_broken.Add(wxString());
			continue;
		}

		wxString repaired_path = TryRepairObPath( _obsysfile );
		// repaired !!
		if( repaired_path != wxString() )
		{
			wxLogMessage( wxT("Repairable : ") + needed_path );
			wxString _ob_path = Convert_To_Ob_Path( repaired_path);
			arr_sysfiles_broken.Add( repaired_path );
			b_sys_files_to_repair = true;
			count_repairs++;
		}
		else
			arr_sysfiles_broken.Add(wxString());
	}

	if( count_repairs > 0 )
	{
		b_have_repair = true;
		wxArrayString _choices;
		_choices.Add( wxT("Do nothing"));
		_choices.Add( wxT("Rename broken paths"));
		wxSingleChoiceDialog *temp = new wxSingleChoiceDialog( this,
								       IntToStr( count_repairs ) + wxT(" OpenBOR sysfiles paths could be repaired\n"),
								       wxT("What do you want ?"),
							_choices );
		temp->SetSelection(1);
		int res = temp->ShowModal();
		if( res == wxID_OK && temp->GetSelection() != 0 )
		{
			for( size_t i = 0; i < arr_sysfiles.Count(); i++ )
			{
				// if Not a broken one
				if( arr_sysfiles_broken[i] == wxString() )
					continue;
				
				if( wxDirExists( arr_sysfiles_broken[i] ) == true )
				{
					wxRenameFile( arr_sysfiles_broken[i], GetObFile(arr_sysfiles[i]).GetFullPath() );
					continue;
				}

				// Try to move the file to the correct path
				if( ! myMove( arr_sysfiles_broken[i], GetObFile(arr_sysfiles[i]).GetFullPath() ))
					wxMessageBox( wxString( wxT("Unable to move:\n")) + arr_sysfiles_broken[i] +
					wxT("\nto\n") +
									GetObFile(arr_sysfiles[i]).GetFullPath()
									, wxT("ProPlem"), wxOK | wxICON_EXCLAMATION, this );

			}
		}
	}

	//***********************************************
	// Repair models.txt => all entities mainFile paths
	//***********************************************
	int _models_count;
	ob_models** _models = fileModels->GetModels(_models_count);
	count_repairs = 0;

	bool b_models_have_to_be_save = false;
	for( int i = 0; i < _models_count; i++)
	{
		ob_models *_ent = _models[i];
		wxString _ob_path = _ent->GetToken(1);
		wxFileName _ent_filename = GetObFile( _ob_path );
		if( _ent_filename.FileExists() && _ob_path.Find( wxT("\\")) < 0 )
			continue;

		wxString repaired_path = TryRepairObPath( _ent_filename );
		// repaired !!
		if( repaired_path != wxString() )
		{
			wxLogMessage( wxT("Repairable : ") + _ent_filename.GetFullPath() );
			wxString _ob_path = Convert_To_Ob_Path( repaired_path);
			_ent->SetToken( 1,_ob_path );
			b_models_have_to_be_save = true;
			count_repairs++;
		}
	}

	if( count_repairs > 0 )
	{
		b_have_repair = true;
		wxArrayString _choices;
		_choices.Add( wxT("Do nothing"));
		_choices.Add( wxT("Repair"));
		_choices.Add( wxT("Backup models.txt and Repair"));
		wxSingleChoiceDialog *temp = new wxSingleChoiceDialog( this,
								       IntToStr( count_repairs ) +wxT( " file paths in models.txt could been repaired\n"),
												       wxT("What do you want ?"),
							_choices );
		temp->SetSelection(2);
		int res = temp->ShowModal();
		if( res == wxID_OK && temp->GetSelection() != 0 )
		{
			// Backup
			if( temp->GetSelection() == 2 )
				if( ! SimpleBackUpFile( fileModels->filename.GetFullPath() ) )
				{
					wxMessageBox( wxT("Unable to do the backup :("), wxT("ProPlem"), wxOK | wxICON_EXCLAMATION, this );
					return;
				}
			if( ! fileModels->Write() )
			{
				wxMessageBox( wxT("Unable to write the changes to models.txt  :("), wxT("ProPlem"), wxOK | wxICON_EXCLAMATION, this );
				return;
			}
		}
	}

	//***********************************************
	// Repair each entity file
	//***********************************************
	if( guiLog != NULL )
		guiLog->Show(false);
	count_repairs = 0;
	int count_entity_repairable = 0;
	b_models_have_to_be_save = false;
	int MAX_ENTITY_REPAIRABLE = 200;
	obFileEntity** ent_repairable = new obFileEntity*[MAX_ENTITY_REPAIRABLE];
	for( int i = 0; i < _models_count; i++)
	{
		if( count_entity_repairable >= MAX_ENTITY_REPAIRABLE )
			break;

		int count_reperaiblable_this_entity = 0;
		wxString _subpath = _models[i]->GetToken(1);
		wxString curr_entity_filename = GetObFile( _subpath).GetFullName();
		wxString _name = _models[i]->GetToken(0);
		obFileEntity* _ent_file = new obFileEntity(GetObFile( _subpath));
		if( _ent_file->obj_container == NULL )
		{
			delete _ent_file;
			continue;
		}

		//************************************
		// repairable properties
		bool b_repairable = false;
		wxString _arr_str_prop[] = {wxT("animationscript"), wxT("icon"), wxT("icondie"), wxT("iconget"), wxT("iconmphalf"), wxT("iconmphigh"),
			  wxT("iconmplow"), wxT("iconpain"),wxT("iconw"), wxT("parrow"), wxT("parrow2"), wxT("hitfx"), wxT("diesound") };
		for( int i=0; i < t_size_of( _arr_str_prop); i++)
		{
			ob_object *_sub_obj = _ent_file->obj_container->GetSubObject(_arr_str_prop[i]);
			if( _sub_obj == NULL )
				continue;

			wxString __path = _sub_obj->GetToken(0);
			wxFileName _filetorepair = GetObFile( __path );
			if( _filetorepair.FileExists() /*&& __path.Find( wxT("\\")) < 0*/)
				continue;

			wxString repaired_path = TryRepairObPath( _filetorepair );
			// repaired !!
			if( repaired_path != wxString() )
			{
				wxLogMessage( wxT("In  ") + curr_entity_filename +
				wxT("  :  can repair ") + _arr_str_prop[i] + wxT(" <") + repaired_path + wxT(">") );
				wxString _ob_path = Convert_To_Ob_Path( repaired_path);
				_sub_obj->SetToken( 0,_ob_path );
				b_repairable = true;
				count_repairs++;
				count_reperaiblable_this_entity++;
			}
		}

		//************************************
		// search for all remap errors

			//************************************
			// palette
			wxString __prop = wxT("palette");
			size_t _palette_objs_count;
			ob_object** _palette_objs = 
				_ent_file->GetSubObjectS(__prop, _palette_objs_count);
			if( _palette_objs_count > 0 )
			{
				for( size_t l = 0; l < _palette_objs_count; l++)
				{
					wxString __path = _palette_objs[l]->GetToken(0);
					if( __path == wxString() )
						continue;

					wxFileName _filetorepair = GetObFile(__path );
					if( _filetorepair.FileExists() && __path.Find(wxT( "\\")) < 0 )
						continue;

					wxString repaired_path = TryRepairObPath( _filetorepair );
					// repaired !!
					if( repaired_path != wxString() )
					{
						wxLogMessage( wxString(wxT("In  ")) + curr_entity_filename +
						wxT("  :  can repair ") + __prop + wxT(" <") + repaired_path + wxT(">") );
						wxString _ob_path = Convert_To_Ob_Path( repaired_path);
						_palette_objs[l]->SetToken( 0,_ob_path );
						b_repairable = true;
						count_repairs++;
						count_reperaiblable_this_entity++;
					}
				}
				delete[] _palette_objs;
			}

			//************************************
			// alternatepal
			__prop = wxT("alternatepal");
			_palette_objs = 
				_ent_file->GetSubObjectS(__prop, _palette_objs_count);
			if( _palette_objs_count > 0 )
			{
				for( size_t l = 0; l < _palette_objs_count; l++)
				{
					wxString __path = _palette_objs[l]->GetToken(0);
					if( __path == wxString() )
						continue;

					wxFileName _filetorepair = GetObFile(__path );
					if( _filetorepair.FileExists() && __path.Find( wxT("\\")) < 0)
						continue;

					wxString repaired_path = TryRepairObPath( _filetorepair );
					// repaired !!
					if( repaired_path != wxString() )
					{
						wxLogMessage( wxString(wxT("In  ")) + curr_entity_filename +
						wxT("  :  can repair ") + __prop + wxT(" <") + repaired_path + wxT(">") );
						wxString _ob_path = Convert_To_Ob_Path( repaired_path);
						_palette_objs[l]->SetToken( 0,_ob_path );
						b_repairable = true;
						count_repairs++;
						count_reperaiblable_this_entity++;
					}
				}
				delete[] _palette_objs;
			}


			//************************************
			// remaps
			__prop = wxT("remap");
			size_t _remap_objs_count;
			ob_object** _remap_objs = 
				_ent_file->GetSubObjectS(__prop, _remap_objs_count);
			if( _remap_objs_count > 0 )
			{
				for( size_t l = 0; l < _remap_objs_count; l++)
				{
					wxString __path = _remap_objs[l]->GetToken(0);
					if( __path == wxString() )
						continue;

					wxFileName _filetorepair = GetObFile(__path );
					if( ! _filetorepair.FileExists() || __path.Find( wxT("\\")) >= 0 )
					{
						wxString repaired_path = TryRepairObPath( _filetorepair );
						// repaired !!
						if( repaired_path != wxString() )
						{
							wxLogMessage( wxT("In  ") + curr_entity_filename +
							wxT("  :  can repair ") + __prop + wxT(" <") + repaired_path + wxT(">") );
							wxString _ob_path = Convert_To_Ob_Path( repaired_path);
							_remap_objs[l]->SetToken( 0,_ob_path );
							b_repairable = true;
							count_repairs++;
							count_reperaiblable_this_entity++;
						}
					}

					__path = _remap_objs[l]->GetToken(1);
					if( __path == wxString() )
						continue;

					_filetorepair = GetObFile(__path );
					if( ! _filetorepair.FileExists() || __path.Find( wxT("\\")) >= 0 )
					{
						wxString repaired_path = TryRepairObPath( _filetorepair );
						// repaired !!
						if( repaired_path != wxString() )
						{
							wxLogMessage( wxT("In  ") + curr_entity_filename +
							wxT("  :  can repair ") + __prop + wxT(" <") + repaired_path + wxT(">") );
							wxString _ob_path = Convert_To_Ob_Path( repaired_path);
							_remap_objs[l]->SetToken( 1,_ob_path );
							b_repairable = true;
							count_repairs++;
							count_reperaiblable_this_entity++;
						}
					}
				}
				delete[] _remap_objs;
			}


		//************************************
		// search for all Frames errors
		size_t _AnimsCount;
		ob_anim** _anims = _ent_file->GetAnims(_AnimsCount);
		for(size_t j=0; j<_AnimsCount;j++)
		{
			size_t frm_count;
			ob_frame** __frames = _anims[j]->GetFrames(frm_count);
			for( size_t k=0;k<frm_count;k++)
			{
				// Try to repair the frame path
				ob_frame *curr_frame = __frames[k];
				if( curr_frame->GetToken(0) != wxString() )
				{
					wxString __path = curr_frame->GetToken(0);
					wxFileName _curr_gif_path = curr_frame->GifPath();
					wxString __temp = _curr_gif_path.GetFullPath();
					if( ! _curr_gif_path.FileExists() || __path.Find( wxT("\\") ) >= 0 )
					{
						wxString repaired_path = TryRepairObPath( _curr_gif_path );

						// try with a png
						bool b_no_extension = (_curr_gif_path.GetExt() == wxString());
						if( repaired_path == wxString() && b_no_extension )
						{
							_curr_gif_path.SetExt(wxT("png"));
							repaired_path = TryRepairObPath( _curr_gif_path );
						}

						// try with a gif
						if( repaired_path == wxString() && b_no_extension )
						{
							_curr_gif_path.SetExt(wxString());
							_curr_gif_path.SetExt(wxT("gif"));
							repaired_path = TryRepairObPath( _curr_gif_path );
						}

						// repaired !!
						if( repaired_path != wxString() )
						{
							wxLogMessage( wxT("In  ") + curr_entity_filename +
							wxT("  :  can repair frame <") + repaired_path + wxT(">" ));
							wxString _ob_path = Convert_To_Ob_Path( repaired_path);
							curr_frame->SetToken( 0,_ob_path );
							b_repairable = true;
							count_repairs++;
							count_reperaiblable_this_entity++;
						}
					}
				}

				// Try to repair the sound path
				ob_object *_sub_obj = curr_frame->GetSubObject(wxT("sound"));
				if( _sub_obj != NULL )
				{
					wxString str_sound_path = _sub_obj->GetToken( 0 );
					if( str_sound_path != wxString() )
					{
						wxFileName _curr_sound_path= GetObFile( str_sound_path );
						if( ! _curr_sound_path.FileExists() || str_sound_path.Find( wxT("\\")) >= 0 )
						{
							wxString repaired_path = TryRepairObPath( _curr_sound_path );
							// repaired !!
							if( repaired_path != wxString() )
							{
								wxLogMessage(wxT("In  ") + curr_entity_filename +
								wxT("  :  can repair sound <") + repaired_path + wxT(">") );
								wxString _ob_path = Convert_To_Ob_Path( repaired_path);
								_sub_obj->SetToken( 0,_ob_path );
								b_repairable = true;
								count_repairs++;
								count_reperaiblable_this_entity++;
							}
						}
					}
				}
			}
		}
		if( _anims != NULL )
			delete[] _anims;
		
		if( b_repairable )
		{
			wxLogMessage( wxT("In  ") + curr_entity_filename +
			wxT("  :  ") + IntToStr(count_reperaiblable_this_entity) +
			wxT("  repairable Frame ") );
			ent_repairable[count_entity_repairable] = _ent_file;
			count_entity_repairable++;
		}
		else
			delete _ent_file;
		
		
	}
	if( count_repairs > 0 )
	{
		b_have_repair = true;
		wxArrayString _choices;
		_choices.Add( wxT("Do nothing"));
		_choices.Add( wxT("Repair"));
		_choices.Add( wxT("Backup all entities and Repair"));
		wxSingleChoiceDialog *temp = new wxSingleChoiceDialog( this,
								       IntToStr( count_repairs ) + wxT(" file paths in ")
								       + IntToStr(count_entity_repairable) + wxT(" entity files could be repaired\n"),
														 wxT("What do you want ?"),
							_choices );
		temp->SetSelection(1);
		int res = temp->ShowModal();
		if( res == wxID_OK && temp->GetSelection() != 0 )
		{
			int backup_errors = 0;
			int write_errors = 0;
			for( int i = 0; i<count_entity_repairable;i++)
			{
				obFileEntity *_ent_ = ent_repairable[i];
				// Backup
				if( temp->GetSelection() == 2 )
				{
					if( ! SimpleBackUpFile( _ent_->filename.GetFullPath() ) )
					{
						backup_errors++;
						wxLogMessage( wxT("Unable to do the backup :(" ));
					}
				}
				if( ! _ent_->Write() )
				{
					wxLogMessage( wxT("Unable to write the changes to models.txt  :(" ));
				    write_errors++;
				}
				delete _ent_;
			}
			delete[] ent_repairable;
			wxMessageBox( wxT("Repairabables are repaired !\n" )+
			IntToStr( write_errors ) + wxT("  writes errors\n") +
			IntToStr( backup_errors ) + wxT("  backups errors") , wxT("Bingo")
		    				, wxOK | wxICON_EXCLAMATION, this );
		}
	}
	
	if( _models != NULL )
 		delete[] _models;

	//***********************************************
	// Repair levels.txt paths
	//***********************************************
	if( guiLog != NULL )
		guiLog->Show(false);
	
	// Try to load levels.txt
	wxString fp_levels_txt = wxT("data/") + curr_mod->levels_txt;
	obFileLevels levels( GetObFile(fp_levels_txt) );
	if( levels.nb_lines == 0 || fileLevels->obj_container == NULL )
	{
		wxMessageBox( wxT("Unable to load levels.txt : \n") +
				fp_levels_txt
				, wxT("ERROR !")
				, wxOK | wxICON_EXCLAMATION, this );
	}
	
	// Levels loaded
	else
	{
		// Get the sets
		size_t nb_set = 0;
		ob_StagesSet** levels_sets = 
			((ob_levels*) levels.obj_container)->GetSets(nb_set);
		
		size_t count_repairs = 0;
		list<wxString> l_stage_valid;
		
		// There is some sets in the levels.txt
		for( size_t set_num = 0; set_num < nb_set; set_num++ )
		{
			if( levels_sets[set_num] == NULL )
				continue;
			size_t _nb_stages;
			ob_StageDeclaration** stages_declares =
				levels_sets[set_num]->GetStages( _nb_stages);
			
			for( size_t num_stage = 0; num_stage < _nb_stages;num_stage++)
			{
				if( stages_declares[num_stage] == NULL )
					continue;
				
				// Get the stage file
				wxString st_file = 
					stages_declares[num_stage]->GetFileName();
				
				// Try to repair the path
				if( wxFileExists(st_file) == false )
				{
					wxString repaired_path = TryRepairObPath( st_file );
					
					// repaired !!
					if( repaired_path != wxString() )
					{
						wxLogMessage( wxT("In  ") +
							curr_mod->levels_txt +
							wxT("  :  can repair ") + 
							st_file + 
							wxT(" <") + repaired_path + wxT(">")
							);
						wxString _ob_path = Convert_To_Ob_Path( repaired_path);
						stages_declares[num_stage]->SetFileName(_ob_path );
						count_repairs++;
						l_stage_valid.push_back( repaired_path );
					}
				}
				else
					l_stage_valid.push_back( st_file );
			}
			
			// Cleaning
			if( stages_declares != NULL )
				delete[] stages_declares;
		}
		
		// Cleaning
		if( levels_sets != NULL )
			delete[] levels_sets;
		
		// Propose the save of the repairs
		bool b_cont = true;
		if( count_repairs > 0 )
		{
			b_have_repair = true;
			wxArrayString _choices;
			_choices.Add( wxT("Do nothing"));
			_choices.Add( wxT("Repair"));
			wxSingleChoiceDialog *temp = new wxSingleChoiceDialog( this,
									       IntToStr( count_repairs ) + wxT(" file paths in ")
									       + curr_mod->levels_txt + wxT(" stage files could be repaired\n"),
													    wxT("What do you want ?"),
								_choices );
			temp->SetSelection(1);
			int res = temp->ShowModal();
			if( res == wxID_OK && temp->GetSelection() != 0 )
				levels.Write();
			else
				b_cont = false;
		}
		
		if( b_cont == true )
		{
			Repair_Set repair_set;
			repair_set.Add_Repairable( wxT("background") , 0 );
			repair_set.Add_Repairable( wxT("bglayer") , 0 );
			repair_set.Add_Repairable( wxT("frontpanel") , 0 );
			repair_set.Add_Repairable( wxT("panel") , 0 );
			repair_set.Add_Repairable( wxT("panel") , 1 );
			repair_set.Add_Repairable( wxT("panel") , 2 );
			repair_set.Add_Repairable( wxT("loadingbg") , 0 );
			repair_set.Add_Repairable( wxT("palette") , 0 );
			repair_set.Add_Repairable( wxT("bossmusic") , 0 );
			repair_set.Add_Repairable( wxT("music") , 0 );
			
			// For each stage
			while( l_stage_valid.empty() == false )
			{
				wxString stage_fp = l_stage_valid.front();
				l_stage_valid.pop_front();
				obFileStage ofs(stage_fp);
				if( ofs.obj_container == NULL )
					continue;
				
				size_t nb_repairs = 
					repair_set.Try_Repair( ofs.obj_container );
				if( nb_repairs > 0 )
				{
					b_have_repair = true;
					wxArrayString _choices;
					_choices.Add( wxT("Do nothing"));
					_choices.Add( wxT("Repair"));
					wxSingleChoiceDialog *temp = new wxSingleChoiceDialog( this,
											       IntToStr( nb_repairs ) + wxT(" file paths in ")
											       + stage_fp + wxT(" paths could be repaired\n"),
														wxT("What do you want ?"),
										_choices );
					temp->SetSelection(1);
					int res = temp->ShowModal();
					if( res == wxID_OK && temp->GetSelection() != 0 )
						ofs.Write();
				}
			}
		}
	}
	
	if( b_have_repair == false )
	{
		wxMessageBox( wxT("No path found as repairable") , wxT("Bingo")
	    				, wxOK | wxICON_EXCLAMATION, this );
	}


	if( guiLog != NULL )
		guiLog->Show(false);

}



