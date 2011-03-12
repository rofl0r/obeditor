#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__mod.h"
#include "ob_editor.h"
#include "entity__enums.h"
#include "entity__globals.h"
#include "entity__MyAnimationZone.h"

using namespace std;

extern void initaudio();

//****************************************************
//			Some Globals
//****************************************************
ob_anim** arr_anims;
int arr_anims_count;

// Currently selected anim (Note: -1 => no anim selected )
ob_anim* curr_anim;
wxString last_anim_selected;

// array and array size of Frames loaded
int curr_frames_count;
ob_frame** curr_frames;
int ind_active_frame;

// current active frame
ob_frame* frameActive;

// the original bg colour
wxColour default_BG_colour;

int ent_g_flag;

//****************************************************
//			Tools or fucking constructors annex functions
//****************************************************

void SetLastSessionSelectedEntity()
{
	// Set the current selected object
	wxString last_ent = ConfigEdit_Read( wxT("last_entity_selected"), wxString() );
	if( models_count > 0 )
	{
		int last_ind = 0;
		if( last_ent != wxString() )
		{
			for( int i =0; i < models_count; i++)
				if( models_name[i] == last_ent )
				{
					last_ind = i;
					break;
				}
		}

		// Set the default selected entity
		panel_EntOverview->list_entities->SetSelection(last_ind);
		// Update some widgets and the avatar
		wxCommandEvent event;
		panel_EntOverview->list_entities->OnSelectionChange(event);
	}
}


//****************************************************
//			Entity Frame
//****************************************************
enum {
	PG_ENTITIES,
	PG_ANIMATION,
	PG_ERRORS,
	TOOL_ADD,
	TOOL_DEL,
	TOOL_DUPLICATE,
};


//****************************************************

EntityFrame::EntityFrame(wxWindow *_parent, const wxString& title)
       : wxFrame( _parent, -1, title )
{
	m_flag = 0;
	
	// Audio
	initaudio();

	// VARIABLES INITIALISATION
	entityFrame = this; // Needed for subWindow thar refer to it
	models = NULL;
	models_name = NULL;
	b_NoAnimPage = false;
	entitySelection_time = 0;
	entityLastSelection_time = -1;
	curr_entity_filename = wxString();
	ent_g_flag = 0;
	b_started = false;

	// Pre-Init some var to inform others
	panel_Anims = NULL;
	
	// Init the Handlers of the History manager
	theHistoryManager.Set_CallBacks( 
					  &ent__onUndo
					, &ent__onDo
					, &ent__onEltAdd
					, &ent__onNoMoreUndo
					, &ent__onNoMoreDo
					, &ent__onDeleteHistory_data
				);

	// The onglets
	onglets = new wxNotebook(this, wxID_ANY );

	//**************************
	// The globals vars
	arr_anims = NULL;
	arr_anims_count = 0;

	// Currently selected anim (Note: -1 => no anim selected )
	curr_anim = NULL;
	last_anim_selected = wxString();

	// array and array size of Frames loaded
	curr_frames_count = 0;
	curr_frames = NULL;
	ind_active_frame = -1;

	// current active frame
	frameActive = NULL;

	
	vect_ent_history.reserve(MAX_ENTITIES_HISTORY );
	
	//**************************
	// PANELS
	//**************************
	panel_Errors = new Panel_Errors( onglets, wxT("Errors") );
	onglets->AddPage(panel_Errors, wxT("Errors") );

	panel_EntOverview =  new PanelEntity_Overview(onglets);
	onglets->AddPage(panel_EntOverview, wxT("Overview") );

	panel_Anims =  new Panel_Anims(onglets);
	onglets->AddPage(panel_Anims, wxT("Animations") );

	panel_Remaps =  new Panel_Remaps(onglets);
	onglets->AddPage(panel_Remaps, wxT("Remaps") );

	panel_Platform =  new Panel_Platform(onglets);
	onglets->AddPage(panel_Platform, wxT("Platform") );

	//**************************
	// MENU
	//**************************
    wxMenu *menuLoadSave = new wxMenu;

    menuLoadSave->Append( ID_OPEN_IN_EDITOR,	    wxT("Open External Editor  (F3)") );
    menuLoadSave->AppendSeparator();
    menuLoadSave->Append( ID_ENTITIES_CURR_RELOAD,	wxT("Reload Current Entity  (F4)") );
    menuLoadSave->Append( ID_ENTITIES_CURR_SAVE,	wxT("Save Current Entity     (F2)" ));
    menuLoadSave->AppendSeparator();
    menuLoadSave->Append( ID_ENTITIES_RELOADALL, 	wxT("Reload all entities   (Shift + F2)") );
    menuLoadSave->Append( ID_ENTITIES_SAVEALL,		wxT("Save   all entities    (Shift + F4)" ));
    menuLoadSave->AppendSeparator();
    menuLoadSave->Append( ID_ENTITIES_RELOAD_MODELSTXT, wxT("Reload entities list" ));

    menuHistory = new wxMenu;
    menuHistory->Append( ID_ENTITIES_HISTORY_CLEAR, wxT("Clear this list") );
    menuHistory->AppendSeparator();

    menuUnRedo = new wxMenu;
    menuUnRedo->Append( ID_UNDO, wxT("Undo\tCTRL-Z" ))->Enable(false);
    menuUnRedo->Append( ID_REDO, wxT("Redo\tCTRL-SHIFT-Z") )->Enable(false);

    wxMenu *menuAnims = new wxMenu;
    menuAnims->Append( ID_ENTITIES_RELOAD_GIFS, wxT("Reload Frames (F5)") );
    menuAnims->Append( ID_ENTITIES_RESCALE_BOXES, wxT("Rescale Boxes and Offsets (F6)") );

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append( ID_HELP_PLEASE, wxT("Manual") );


    menuBar = new wxMenuBar;
    menuBar->Append( menuLoadSave, wxT("Load/Save") );
    menuBar->Append( menuHistory, wxT("Last Entities") );
    menuBar->Append( menuAnims, wxT("Animation") );
    menuBar->Append( menuUnRedo, wxT("History") );
    menuBar->Append( menuHelp, wxT("?") );

    SetMenuBar( menuBar );


    //**************************
	// FINISHING THE START
	//**************************
	SetLastSessionSelectedEntity();
	Frame_RestorePrevCoord( this, wxT("entity_frame"));
	SetSizeHints( 400, 400 );
	onglets->SetSelection(1);
	panel_EntOverview->list_entities->SetFocus();
  	Show();
  	Center();
	b_started = true;
}

//****************************************************

void EntityFrame::EvtClose( wxCloseEvent& event)
{
	m_flag |= M_CLOSING;
	// Stop playing
	if( panel_Anims != NULL )
		panel_Anims->animation_ctrl->StopPlaying();
	
	// Invalidate everything
	wListFrames::theFrameImgManager.Invalidate_ALL();
	
	// Save wnd coords
	Frame_SaveCoord( this, wxT("entity_frame"));

	// Save last entity selected
	wxString last_entity_edited = panel_EntOverview->list_entities->GetStringSelection();
	ConfigEdit_Write( wxT("last_entity_selected"), last_entity_edited );

	// Save to autoload this frame
	config->Write(wxT("/autoload/frame"), wxT("edit_entity" ));

	// Save the last anim edited
	wxString temp = panel_Anims->list_Anims->GetStringSelection();
	ConfigEdit_Write( wxT("last_anim_edited"), temp );

	panel_Anims->EvtClose( event);
	panel_EntOverview->EvtClose( event);

	startFrame->b_reset_entityFrame = true;
	startFrame->Show();
	event.Skip();
}

//****************************************************

EntityFrame::~EntityFrame()
{
	b_started = false;
	
	panel_Errors=NULL;
	panel_EntOverview=NULL;
	panel_Anims=NULL;
	panel_Remaps = NULL;
}


//****************************************************

void EntityFrame::EvtActivate( wxActivateEvent& evt )
{
	// hide the StartFrame
	startFrame->Hide();
	Refresh();
	evt.Skip();
}

//****************************************************

void EntityFrame::EvtResize( wxSizeEvent& event )
{
	Refresh();
	event.Skip();
}

//****************************************************

void EntityFrame::EvtMenu(wxCommandEvent& event)
{
	switch( event.GetId() )
	{
		case ID_ENTITIES_CURR_SAVE:
			panel_EntOverview->Save_Current();
			break;

		case ID_ENTITIES_SAVEALL:
			panel_EntOverview->SaveAll(false);
			break;

		case ID_ENTITIES_CURR_RELOAD:
			ent_g_flag |= GF_RELOAD;
			wListFrames::theFrameImgManager.Invalidate_ALL();
			panel_EntOverview->Reload_Current();
			Reload_Anims();
			Reload_Frames();
			panel_Anims->Reload();
			panel_Remaps->Reload();
			panel_Platform->Reload();
			ent_g_flag &= ~GF_RELOAD;
			break;

		case ID_ENTITIES_RELOADALL:
			ent_g_flag |= GF_RELOAD;
			wListFrames::theFrameImgManager.Invalidate_ALL();			
			panel_EntOverview->ReloadAll();
			Reload_Anims();
			Reload_Frames();
			panel_Anims->Reload();
			panel_Remaps->Reload();
			panel_Platform->Reload();
			ent_g_flag &= ~GF_RELOAD;
			break;

		case ID_ENTITIES_RELOAD_MODELSTXT:
			ent_g_flag |= GF_RELOAD;
			wListFrames::theFrameImgManager.Invalidate_ALL();			
			panel_EntOverview->Reload_EntityList();
			ent_g_flag &= ~GF_RELOAD;
			break;

		case ID_ENTITIES_RELOAD_GIFS:
			wListFrames::theFrameImgManager.Invalidate_ALL();			
			panel_Anims->ReloadGifs();
			panel_Remaps->ReloadGifs();
			panel_Platform->ReloadGifs();
			break;

		case ID_ENTITIES_HISTORY_CLEAR:
			EntityAnimsHistory__Clear();
			break;

		case ID_ENTITIES_RESCALE_BOXES:
			panel_Anims->Rescale_Boxes();
			panel_Anims->ReloadGifs();
			break;

		case ID_HELP_PLEASE:
			WndFromText( this, wxT("Manual"), GetRessourceFile_String( wxT("manual.txt") )).ShowModal();
			break;

		case ID_OPEN_IN_EDITOR:
		{
			// Get the editor from the config
			wxString editor_path;
			if( ! config->Read( wxT("/startFrame/editor_program"), &editor_path ) )
			{
				wxMessageBox( wxT("Did you correctly set the editor in the start window ??"),
					      wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
				break;
			}

			if( ! wxFileName(editor_path).FileExists() )
			{
				wxMessageBox( wxT("Unable to find ") + editor_path + wxT(" ??"),
					      wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
				break;
			}

			// Get the current file to edit
			if( entity == NULL )
			{
				wxMessageBox( wxT("No current entity to edit ??"),
					      wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
				break;
			}

			wxString __path = entity->filename.GetFullPath();

				// Launch the editor
				wxExecute( wxT("\"") + editor_path +wxT("\" \"") + __path + wxT("\"") );
		}

		default:
			if( 	   event.GetId() >=ID_ENTITIES_HISTORY_START
				&& event.GetId() < ID_ENTITIES_HISTORY_START + (int) vect_ent_history.size()
				&& entity != NULL 
			  )
			{
				wxString ent_name = 
					vect_ent_history[event.GetId() -ID_ENTITIES_HISTORY_START];
				
				if( ent_name != entity->Name() )
				{
					if( panel_EntOverview->Select_Entity( ent_name ) == true )
					{
						wxNotebookEvent event;
						event.SetOldSelection( onglets->GetSelection() );
						event.SetSelection( onglets->GetSelection() );
						PanelPageChanging( event );
					}
				}
				break;
			}

			break;
	}
}

//-----------------------------------------------------------
void 
EntityFrame::EntityAnimsHistory__Clear()
{
	// Delete all menu item
	for( int i = 0; i < (int) vect_ent_history.size(); i++)
		menuHistory->Delete( ID_ENTITIES_HISTORY_START + i );
	
	// Clear the structs
	vect_ent_history.clear();
}

//-----------------------------------------------------------
void 
EntityFrame::EntityAnimsHistory__Append(const wxString& _ent_name)
{
	// Check if it's not already here
	for( size_t i = 0; i < vect_ent_history.size() ; i++)
		if( vect_ent_history[i] == _ent_name )
			return;
	
	// Create a non-empty label
	wxString pent_name = _ent_name;
	if( IsEmpty( pent_name ) == true )
		pent_name = wxT("EMPTY");
		
	// Append the stuff
	menuHistory->Append(vect_ent_history.size()+ID_ENTITIES_HISTORY_START,pent_name );
	vect_ent_history.push_back( pent_name );
}


//****************************************************
void 
EntityFrame::EvtCharPress(wxKeyEvent& event)
{
	wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
	bool b_shortcut = true;

	int kc = event.GetKeyCode();

	switch( kc )
	{
		case WXK_F2:
			if( event.ShiftDown() )
				evt.SetId( ID_ENTITIES_SAVEALL );
			else
				evt.SetId( ID_ENTITIES_CURR_SAVE );
			break;

		case WXK_F3:
			evt.SetId( ID_OPEN_IN_EDITOR );
			break;

		case WXK_F4:
			if( event.ShiftDown() )
				evt.SetId( ID_ENTITIES_RELOADALL );
			else
				evt.SetId( ID_ENTITIES_CURR_RELOAD );
			break;
		case WXK_F5:
//			wxMessageBox( "Panel_Anims keypress", "Error", wxOK | wxICON_INFORMATION );
			evt.SetId( ID_ENTITIES_RELOAD_GIFS );
			break;
		default:
			b_shortcut = false;
			break;
	}
	if( b_shortcut )
	{
		AddPendingEvent( evt );
		return;
	}
	event.Skip();
}

//--------------------------------------------------------
Panel_withAnims* 
EntityFrame::GetCurrentPanel()
{
	if( onglets->GetCurrentPage() == panel_Anims )
		return panel_Anims;

	if( onglets->GetCurrentPage() == panel_Remaps )
		return panel_Remaps;

	if( onglets->GetCurrentPage() == panel_Platform )
		return panel_Platform;

	return NULL;
}

//****************************************************
void 
EntityFrame::PanelPageChanging(wxNotebookEvent& event )
{
	ent_g_flag |= GF_ACTIVATE;
	// Stop playing
	if( panel_Anims != NULL )
		panel_Anims->animation_ctrl->StopPlaying();
	
	wxWindow* thepanel = panel_EntOverview;

	if( event.GetSelection() == 2 )
		thepanel = panel_Anims;

	else if( event.GetSelection() == 3 )
		thepanel = panel_Remaps;

	else if( event.GetSelection() == 4 )
		thepanel = panel_Platform;

	else if( event.GetSelection() == 0 )
		thepanel = panel_Errors;
	
	// Possibly needs to refresh frames and anims
	if( event.GetSelection() >= 2 && 	entitySelection_time > entityLastSelection_time )
	{
		EntityAnimsHistory__Append( entity->Name() );
		Reload_Anims();
		Reload_Frames();
		entityLastSelection_time = entitySelection_time;
	}
	
	// Load animations if new page is anim page
	if( event.GetSelection() == 2 && panel_Anims != NULL )
		panel_Anims->OnActivate();
	if( event.GetSelection() == 3 && panel_Remaps != NULL )
		panel_Remaps->OnActivate();
	if( event.GetSelection() == 4 && panel_Platform != NULL )
		panel_Platform->OnActivate();

	ent_g_flag &= ~GF_ACTIVATE;
	
	// Check if changes have to be written
	if( entity != NULL )
	{
		if( panel_EntOverview->EntityChanged() == 1 )
		{
			int res = wxMessageDialog( thepanel,
				wxT("Change have been made to the current entity\nWrite the changes now ??"),
						   wxT("Question !"), wxYES_NO | wxSTAY_ON_TOP).ShowModal();

			if( res == wxID_YES )
			{
				if( ! entity->Write() )
					wxMessageBox( wxT("Error, can't write the file !!"), wxT("Error !")
					, wxYES_NO | wxICON_ERROR, thepanel  );
			}
			else
				// Do not ask again
				entity->changed = 2;
		}
	}

}


//****************************************************

void EntityFrame::SetPanelErrorsIntitule( const wxString& new_intitule )
{
	onglets->SetPageText( 0, new_intitule );
}


//****************************************************

void EntityFrame::Reload_Frames()
{
	if( entity == NULL )
	{
		if( curr_frames != NULL )
			delete[] curr_frames;
		curr_frames = NULL;
		curr_frames_count = 0;
		ind_active_frame = -1;
		frameActive = NULL;
		return;
	}

	// Free the old frames
	if( curr_frames != NULL )
	{
		curr_frames_count = 0;
		delete[] curr_frames;
		curr_frames = NULL;
	}

	// Reload the frame list
	if( curr_anim == NULL )
	{
		ind_active_frame = -1;
		frameActive = NULL;
	}
	else
	{
		size_t nb_frames;
		curr_frames = curr_anim->GetFrames(nb_frames);
		curr_frames_count = nb_frames;
		if( ind_active_frame >= curr_frames_count )
			ind_active_frame = curr_frames_count -1;
	}
}


//****************************************************

void EntityFrame::Reload_Anims()
{
	if( entity == NULL )
	{
		if( arr_anims != NULL )
			delete[] arr_anims;
		arr_anims = NULL;
		arr_anims_count = 0;
		curr_anim = NULL;
		return;
	}

	// Reload the anim list
	if( arr_anims != NULL )
		delete[] arr_anims;
	size_t _t_nb_anims;
	arr_anims = entity->GetAnims(_t_nb_anims);
	arr_anims_count = _t_nb_anims;

	curr_anim = NULL;
	if( arr_anims != NULL )
	{
		Sort_ob_Object_ByTag( arr_anims, arr_anims_count, 0 );
		if( last_anim_selected != wxString() )
			for( int i  = 0; i < arr_anims_count; i++)
				if( arr_anims[i]->GetToken(0).Upper() == last_anim_selected.Upper() )
				{
					curr_anim = arr_anims[i];
					break;
				}

		if( curr_anim == NULL )
		{
			curr_anim = arr_anims[0];
			last_anim_selected = curr_anim->GetToken( 0 );
		}
	}

}

//---------------------------------------------------------
void
EntityFrame::EvtRedo(wxCommandEvent& event)
{
	if( theHistoryManager.Do() == false )
	{
		// Beep user
		wxBell();
	}
}

//---------------------------------------------------------
void
EntityFrame::EvtUndo(wxCommandEvent& event)
{
	// Register ctrls states when undoing the present
	if( theHistoryManager.IsPresent() )
		Register_ControlsState_inHistory(wxT("EvtUndo"));
	if( theHistoryManager.UnDo() == false )
	{
		// Beep user
		wxBell();
	}
}

//---------------------------------------------------------
void
EntityFrame::Menu_DoRedos__Update_Labels(list<History_Group*>::iterator it_group)
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
bool
EntityFrame::May_Register_ControlsState()
{
/*	// Register nothing if no history or in the past
	if( 	   theHistoryManager.IsHistory_Empty() == true 
		|| theHistoryManager.IsPresent() == false
	  )
		return false;*/
	return ( entityFrame != NULL && (m_flag & M_CLOSING) == 0 );
}
	

//---------------------------------------------------------
void
EntityFrame::Register_ControlsState_inHistory(const wxString& f_caller )
{
//	MyLog( MYLOG_DEBUG, "Register_ControlsState_inHistory()", "CALLED by" + f_caller );
	if( theHistoryManager.IsUndoing() == true || b_started == false )
		return;

	// Get The Last History Element
	History_Elt* elt = theHistoryManager.Get_Last_HistoryElt();
	
	// no history elt, use the initital_datas of theHistoryManager
	if( elt == NULL )
	{
		if( theHistoryManager.initial_datas == NULL )
			theHistoryManager.initial_datas =(void*) new editor_entities_states;
		Set_ControlsState_In( theHistoryManager.initial_datas );
	}
	else
	{
		if( elt->datas == NULL )
			elt->datas = (void*) new editor_entities_states;
		Set_ControlsState_In( elt->datas );
	}
}


//---------------------------------------------------------
void
EntityFrame::Set_ControlsState_In( void* _elt_datas )
{
	editor_entities_states* elt_datas = (editor_entities_states*) _elt_datas;
	Panel_withAnims* thePanel = GetCurrentPanel();
	if( thePanel != NULL )
	{
		elt_datas->anim_selected   = thePanel->list_Anims->GetStringSelection();
		elt_datas->frame_sel_start = thePanel->wList_frames->GetSelectedFirst();
		elt_datas->frame_sel_end   = thePanel->wList_frames->GetSelectedLast();
		elt_datas->frame_selected  = ind_active_frame;
	}
}

//---------------------------------------------------------
static
void
Restore_Panels_Anims_with_States( Panel_withAnims* thePanel, editor_entities_states* pStates )
{
	if( entity != NULL )
		entity->PurgeImages();
	
	// Restore the anim list selection
	wxCommandEvent dummy;
	thePanel->EvtAnimListChange( dummy );
	thePanel->list_Anims->Refresh_List();
	if( thePanel->list_Anims->pSetSelection( pStates->anim_selected ) == true )
	{
		wxCommandEvent dummy;
		thePanel->list_Anims->OnSelectionChange(dummy);
	
		// Restore the frames list selection
		thePanel->wList_frames->RefreshSelection( false );
		thePanel->wList_frames->RestoreSelection( 
				pStates->frame_selected
				, pStates->frame_sel_start
				, pStates->frame_sel_end );
	}

	thePanel->OnActivate();
}


//---------------------------------------------------------
void
EntityFrame::Restore_ControlsState( void* _pStates )
{
	if( entity == NULL )
		return;

	editor_entities_states* pStates = (editor_entities_states*) _pStates;
	
	// Current view is a valid panel anims
	Panel_withAnims* thePanel = GetCurrentPanel();
	if( thePanel != NULL )
		Restore_Panels_Anims_with_States( thePanel, pStates );
	
	// Current view is either the overview or the errors
	else
	{
		// have to check if a panel have load the current entity
		list<Panel_withAnims*> l_panels;
		l_panels.push_back( panel_Anims );
		l_panels.push_back( panel_Remaps );
		l_panels.push_back( panel_Platform );

		while( l_panels.empty() == false )
		{
			thePanel = l_panels.front();
			l_panels.pop_front();
			if( thePanel->this_entityAnimation_time
				>= entityFrame->entitySelection_time
			  )
			{
				Restore_Panels_Anims_with_States( thePanel, pStates );
			}
		}
	}
	
	// Must also refresh the entity prop ctrl of overview panel
	panel_EntOverview->list_ent_Props->RefreshProperties();
}


//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(EntityFrame, wxFrame)

	EVT_MENU(ID_ENTITIES_RELOADALL,  EntityFrame::EvtMenu)
	EVT_MENU(ID_ENTITIES_CURR_RELOAD,  EntityFrame::EvtMenu)
	EVT_MENU(ID_ENTITIES_SAVEALL,  EntityFrame::EvtMenu)
	EVT_MENU(ID_ENTITIES_CURR_SAVE,  EntityFrame::EvtMenu)
	EVT_MENU(ID_ENTITIES_RELOAD_MODELSTXT,  EntityFrame::EvtMenu)
	EVT_MENU(ID_ENTITIES_RELOAD_GIFS,  EntityFrame::EvtMenu)
	EVT_MENU(ID_ENTITIES_RESCALE_BOXES,  EntityFrame::EvtMenu)
	EVT_MENU(ID_HELP_PLEASE,  EntityFrame::EvtMenu)
	EVT_MENU(ID_OPEN_IN_EDITOR,  EntityFrame::EvtMenu)
	EVT_MENU(ID_ENTITIES_HISTORY_CLEAR,  EntityFrame::EvtMenu)
	EVT_MENU_RANGE(ID_ENTITIES_HISTORY_START, ID_ENTITIES_HISTORY_START+ MAX_ENTITIES_HISTORY , EntityFrame::EvtMenu)
	EVT_MENU(ID_UNDO,  EntityFrame::EvtUndo)
	EVT_MENU(ID_REDO,  EntityFrame::EvtRedo)

	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, EntityFrame::PanelPageChanging)
	EVT_CLOSE(EntityFrame::EvtClose)
	EVT_ACTIVATE(EntityFrame::EvtActivate)
	EVT_SIZE(EntityFrame::EvtResize)

	EVT_CHAR(EntityFrame::EvtCharPress)
	EVT_CHAR_HOOK(EntityFrame::EvtCharPress)

END_EVENT_TABLE()



//****************************************************
//*********** TOOLS FUNCTIONS ***********************
//****************************************************
wxString
ConfigEdit_Read( const wxString& subpath, const wxString& def_val)
{
	wxString res;
	config->Read(wxT("/editFrame/") + theModsList->GetSelectedProjectName() + wxT("/") + subpath,
				&res, def_val);
	return res;
}

void
ConfigEdit_Write( const wxString& subpath, const wxString& val )
{
	config->Write(wxT("/editFrame/") + theModsList->GetSelectedProjectName() + wxT("/") + subpath,
				val);
	return;
}

//****************************************************
//*********** HISTORY FUNCTIONS **********************
//****************************************************
//-------------------------------------------------------------------
int 
ent__onUndo(
			  list<History_Group*>::iterator it_group
			, History_Elt*  _elt
	   )
{
	if( entityFrame == NULL )
		return 0;	// By default undo only one group
	
	// Check if it's the first elt of the group
	if( (*it_group)->l_elts.front() == _elt )
	{
		// Check if it's the real first group of the history
		if( theHistoryManager.IsFirstGroup(it_group) == true )
		{
			// So previous controls state are in this elt
			if(theHistoryManager.initial_datas == NULL )
				wxMessageBox( wxT("BUG !!\nent__onUndo()\nFirst elt has no state setted !!!\n") );
			else
				entityFrame->Restore_ControlsState( theHistoryManager.initial_datas );
		}
		
		// So the states are in the last elt of the previous group
		else
		{
			// Get the group-related-most-recent ctrls states
			void* _datas = Get_Prev_Ctrl_States( it_group, false );
			if( _datas != NULL )
				entityFrame->Restore_ControlsState( _datas );
		}
		
		// AnyWay, must update the Do/Redo menu Labels
		entityFrame->Menu_DoRedos__Update_Labels( it_group );
	}

	return 0;	// undo only one group
}

//----------------------------------------------------------------
int 
ent__onDo(
			  list<History_Group*>::iterator it_group
			, History_Elt*  _elt
	   )
{
	if( entityFrame == NULL )
		return 0;	// By default only redo only one group
	
	// Check if it's the last elt of the group
	if( (*it_group)->l_elts.back() == _elt )
	{
		// Get the group-related-most-recent ctrls states
		void* _datas = Get_Prev_Ctrl_States( it_group, true );
		if( _datas != NULL )
			entityFrame->Restore_ControlsState( _datas );
		
		// AnyWay, must update the Do/Redo menu Labels
		list<History_Group*>::iterator it_t(it_group);
		it_t++;
		entityFrame->Menu_DoRedos__Update_Labels( it_t );

	}
	
	return 0;	// redo only one group
}

//----------------------------------------------------------------
int 
ent__onEltAdd(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   )
{
	return 1;	// accepts adds to curr group
}


//----------------------------------------------------------------
void 
ent__onNoMoreUndo(bool b_state)
{
	if( entityFrame == NULL )
		return;
	
	entityFrame->Menu_DoRedos__Update_Labels( theHistoryManager.it_now );
}

//----------------------------------------------------------------
void 
ent__onNoMoreDo(bool b_state)
{
	if( entityFrame == NULL )
		return;

	entityFrame->Menu_DoRedos__Update_Labels( theHistoryManager.it_now );
}

//----------------------------------------------------------------
void 
ent__onDeleteHistory_data(void*&  _elt_datas)
{
	delete (editor_entities_states*) _elt_datas;
	_elt_datas = NULL;
}

