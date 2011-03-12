#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/hashmap.h>

#include "ob_editor.h"
#include "CHistory.h"
#include "entity__enums.h"
#include "entity__globals.h"

//****************************************************
//***********VARIABLES DECL***********************
int models_count;
ob_models** models;
wxString* models_name;

hash_entName_entObjFile hash_ent_obj;
obFileEntity* entity;
PanelEntity_Overview *panel_EntOverview;
wxString curr_entity_filename;


//****************************************************
//*********** new event

// it may also be convenient to define an event table macro for this event type
#ifndef EVT_ADD_NEW_GRIDLINE
#define EVT_ADD_NEW_GRIDLINE(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_ADD_NEW_GRIDLINE, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif



//****************************************************
//			PANEL Entities overview
//****************************************************

PanelEntity_Overview::PanelEntity_Overview(wxNotebook* onglets)
:wxPanel( onglets, wxID_ANY)
{
	loading = false;

	//******************
	//* SIZERS
	//******************

	//top level sizer
		wxBoxSizer *sizer_main = new wxBoxSizer( wxHORIZONTAL );

	// List Entities Sizer
		wxStaticBoxSizer *sizer_listEntities = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Entities") );
		sizer_main->Add( sizer_listEntities,0,wxEXPAND );

	// Right Panel Sizer
		wxStaticBoxSizer *sizer_Right = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Overview of the entity") );
		sizer_main->Add( sizer_Right,1,wxEXPAND );

	// RightUP Panel Sizer
		wxBoxSizer *sizer_RightUp = new wxBoxSizer( wxHORIZONTAL );
		sizer_Right->Add( sizer_RightUp,0,wxEXPAND | wxFIXED_MINSIZE );

	// Entity path Sizer
		wxStaticBoxSizer *sizer_EntPath = new wxStaticBoxSizer( wxVERTICAL, this, wxT("Entity file" ));
		sizer_RightUp->Add( sizer_EntPath, 1, wxEXPAND );

	// Entity properties Sizer
		wxStaticBoxSizer *sizer_PropEntity = new wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Properties" ));
		sizer_Right->Add( sizer_PropEntity, 1, wxEXPAND | wxALL , 5 );

	//******************
	//* OBJECTS
	//******************

	// Path of the entity file
		wPathEntityFile = new wxTextCtrl(this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize,
				wxTE_READONLY | wxSUNKEN_BORDER );
		wPathEntityFile->SetOwnBackgroundColour( wxColour(220,220,220) );
		sizer_EntPath->Add( wPathEntityFile,0, wxEXPAND );

		fileErrorMessage = new  wxStaticText(this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize,
				wxALIGN_RIGHT );
		fileErrorMessage->SetOwnForegroundColour( wxColour(250,0,0) );
		sizer_EntPath->Add( fileErrorMessage,0 );

/*		wPathEntityFile_Choose = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString,
					"Select the entity file", "*.*", wxDefaultPosition, wxDefaultSize,
						wxFLP_OPEN | wxFLP_FILE_MUST_EXIST );
		wPathEntityFile_Choose->SetPath(  dataDirPath.GetFullPath() );
		sizer_EntPath->Add( wPathEntityFile_Choose,0 );
*/

	// Entity image
		avatarImg = noImg;
		avatarCtrl = new MyImageCtrl(this);
		avatarCtrl->Set_BoundDimensions( -1, 180, -1, 180 );
		avatarCtrl->SetImage( avatarImg );
		sizer_RightUp->Add( avatarCtrl,0, wxALL | wxFIXED_MINSIZE );


	// List of entities
		list_entities = new wListEntities(this, wxID_ANY,wxDefaultPosition ,wxDefaultSize );
		sizer_listEntities->Add( list_entities,1, wxEXPAND );

	// Entity Properties
		list_ent_Props = new GridOb_ObjectProperties( this );
		wxArrayString tt;
		tt.Add( wxT("frame" ));
		tt.Add( wxT("anim" ));
		tt.Add( wxT("remap" ));
		tt.Add( wxT("palette" ));
		tt.Add( wxT("alternatepal" ));
		list_ent_Props->SetFilter_Properties( tt );
		sizer_PropEntity->Add( list_ent_Props, 1, wxEXPAND, 5 );


	// FINALIZE
  	sizer_main->SetSizeHints(this);
  	SetSizer( sizer_main );
	list_entities->Refresh_ListEntities();
	list_ent_Props->RefreshProperties();
//	list_ent_Props->Refresh();
	Layout();
}

PanelEntity_Overview::~PanelEntity_Overview()
{
	// Delete all entity in the hash
	size_t count = hash_ent_obj.size();
	hash_entName_entObjFile::iterator i = hash_ent_obj.begin();
	for( size_t j = 0; j< count; j++)
	{
		obFileEntity *temp = i->second;
		delete temp;
		i++;
	}
	hash_ent_obj.clear();
	entity = NULL;
}

int PanelEntity_Overview::EntityChanged()
{
	if( entity == NULL )
		return 0;

	if( list_ent_Props->changed )
	{
		list_ent_Props->changed = false;
		entity->SetChanged();
	}
	return entity->changed;
}

void PanelEntity_Overview::EvtActivate(wxActivateEvent& event )
{
	list_entities->SetFocus();
	event.Skip();
}


void PanelEntity_Overview::EvtClose( wxCloseEvent& )
{
	// Clear the history
	theHistoryManager.Clear_History();
	theHistoryManager.Force_Disable();
	SaveAll( true );
}

void PanelEntity_Overview::Save_Current(bool b_ask)
{
	if( entity == NULL )
		return;

	bool b_doSave = ! b_ask;
	if( b_ask )
	{
		int res = wxMessageBox( wxT("Change have been made to the entity <")+ entity->Name() +wxT(">\nWrite the changes now ??"), wxT("Question !")
				, wxYES_NO | wxICON_INFORMATION, this );
		if( res == wxYES )
			b_doSave = true;
		else
			entity->changed = 2;
	}

	if( b_doSave )
	{
		if( ! entity->Write() )
		{
			wxMessageBox( wxT("Error, can't write the file !!"), wxT("Error !")
			, wxYES_NO | wxICON_ERROR , this );
			entity->changed = 1;
		}
		else
		{
			entity->changed = 0;
		}
	}
	list_ent_Props->changed = false;
}


void PanelEntity_Overview::SaveAll( bool b_ask )
{
	// Update the state of the current entity
	EntityChanged();

	panel_EntOverview->list_ent_Props->changed = false;

	// Ask for all unsaved objects
	size_t count = hash_ent_obj.size();
	hash_entName_entObjFile::iterator i = hash_ent_obj.begin();
	for( size_t j = 0; j< count; j++)
	{
		obFileEntity *temp = i->second;
		if( temp == NULL )
			continue;

		if( temp->changed > 0 )
		{
			bool b_doSave = ! b_ask;
			if( b_ask )
			{
				int res = wxMessageBox(wxT( "Change have been made to the entity <")+ temp->Name() +wxT(">\nWrite the changes now ??"), wxT("Question !")
						, wxYES_NO | wxICON_INFORMATION, this );
				if( res == wxYES )
					b_doSave = true;
			}
			if( b_doSave )
				if( ! temp->Write() )
				{
					wxMessageBox( wxT("Error, can't write the file !!"), wxT("Error !")
					, wxYES_NO | wxICON_ERROR , this );
				}
		}
		i++;
	}
}

void PanelEntity_Overview::Reload_Current()
{
	if( entity == NULL )
		return;

	theHistoryManager.Force_Disable();
	theHistoryManager.Clear_History();
	entity->Reload();
	
	list_ent_Props->changed = false;
	ChangeEntityOverview( models[list_entities->GetSelection()] );

	theHistoryManager.Set_State( true );
}

void PanelEntity_Overview::ReloadAll()
{
	theHistoryManager.Force_Disable();
	theHistoryManager.Clear_History();
	
	size_t count = hash_ent_obj.size();
	hash_entName_entObjFile::iterator i = hash_ent_obj.begin();
	for( size_t j = 0; j< count; j++)
	{
		obFileEntity *temp = i->second;
		if( temp == NULL )
			continue;
		temp->Reload();
		i++;
	}

	list_ent_Props->changed = false;
	ChangeEntityOverview( models[list_entities->GetSelection()] );

	theHistoryManager.Set_State( true );
}

void PanelEntity_Overview::Reload_EntityList()
{
	wxString old_selection;
	if( list_entities->GetSelection() != wxNOT_FOUND )
		old_selection = list_entities->GetStringSelection();

	loading = true;
	fileModels->Reload();
	list_entities->Refresh_ListEntities();
	loading = false;

	if( old_selection != wxString() )
		list_entities->SetStringSelection( old_selection );
		if( ! list_entities->GetSelection() == wxNOT_FOUND )
			list_entities->SetSelection( 0 );
	else
		list_entities->SetSelection( 0 );

	wxCommandEvent event;
	list_entities->OnSelectionChange(event );
}

void PanelEntity_Overview::EvtGetFocus(wxFocusEvent& event )
{
	list_entities->SetFocus();
	event.Skip();
}

void PanelEntity_Overview::Update_PagesAccessible()
{
	// Desactivate uncoherents panels
	if( entity == NULL && entityFrame != NULL && entityFrame->b_NoAnimPage == false )
	{
		entityFrame->b_NoAnimPage = true;

		//remove Animation page
		entityFrame->onglets->RemovePage( 2 );

		//remove Remaps page
		entityFrame->onglets->RemovePage( 2 );

		//remove Platform page
		entityFrame->onglets->RemovePage( 2 );
	}
	else if( entity != NULL && entityFrame != NULL && entityFrame->b_NoAnimPage == true )
	{
		entityFrame->b_NoAnimPage = false;
		// show Animation page
		entityFrame->onglets->AddPage( panel_Anims, wxT("Animations") );
		// show Remaps page
		entityFrame->onglets->AddPage( panel_Remaps, wxT("Remaps") );
		//remove Platform page
		entityFrame->onglets->AddPage( panel_Platform, wxT("Platform") );
	}
}


void PanelEntity_Overview::ChangeEntityOverview(ob_models* _ent)
{
	if( models_count <= 0 )
	{
		list_ent_Props->Clear();
		list_ent_Props->Disable();
		Update_PagesAccessible();
		curr_entity_filename = wxString();
		avatarCtrl->SetImage( noImg );
		avatarCtrl->Refresh();
		wPathEntityFile->SetValue( wxString() );
		entity = NULL;
		return;
	}

	if( entity != NULL )
	{
		if( EntityChanged() == 1 )
		{
			Save_Current( true );
		}
	}

	//--------------------------------
	// Update ent path
	//--------------------------------
	wxString _subpath = _ent->GetToken(1);
	wPathEntityFile->SetValue( _subpath );
	curr_entity_filename = GetObFile( _subpath).GetFullName();

	//--------------------------------
	// load the entity file
	//--------------------------------
	wxString _name = _ent->GetToken(0);
	obFileEntity* _ent_file = hash_ent_obj[_name];

	// IF Entity not already loaded
	if( _ent_file == NULL )
	{
		wxFileName _ent_path = GetObFile( _subpath);
		if( ! _ent_path.FileExists() )
			fileErrorMessage->SetLabel( wxT("File doesn't exist") );
		else
		{
			_ent_file = new obFileEntity( _ent_path );
			if( _ent_file->nb_lines <= 0 )
			{
				delete _ent_file;
				_ent_file = NULL;
				fileErrorMessage->SetLabel( wxT("File is empty or something") );
			}
		}

		// Couldn't load the entity file
		if( _ent_file == NULL )
		{
			entity = NULL;
			list_ent_Props->Clear();
			list_ent_Props->Disable();
			Update_PagesAccessible();
			curr_entity_filename = wxString();
			return;
		}
		 hash_ent_obj[_name] = _ent_file;
	}

	list_ent_Props->Enable();
	fileErrorMessage->SetLabel( wxString() );
	entity = _ent_file;

	//--------------------------------
	// Reload entity properties
	//--------------------------------
	list_ent_Props->SetObjectHandled( _ent_file->obj_container );


	//--------------------------------
	// Check accessible page of the notebook
	//--------------------------------
	Update_PagesAccessible();

	return;
}

//--------------------------------------------------------------
bool
PanelEntity_Overview::Select_Entity( const wxString& _name )
{
	wxArrayString arr_models = list_entities->GetStrings();
	for( size_t i = 0 ; i < arr_models.GetCount(); i++ )
	{
		if( arr_models[i] == _name )
		{
			list_entities->SetSelection( i );
			wxCommandEvent dummy;
			list_entities->OnSelectionChange( dummy );
			return true;
		}
	}
	return false;
}


// Event table
BEGIN_EVENT_TABLE(PanelEntity_Overview, wxPanel)
	EVT_SET_FOCUS(PanelEntity_Overview::EvtGetFocus)
	EVT_CLOSE(PanelEntity_Overview::EvtClose)
	EVT_ACTIVATE(PanelEntity_Overview::EvtActivate)
END_EVENT_TABLE()



//****************************************************
//		ENTITIES LIST
//****************************************************

wListEntities::wListEntities(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, int n,
		const wxString choices[], long style, const wxValidator& validator, const wxString& name)
:myListBox(parent, id, pos, size, n, choices, style, validator, name)
{
}

wListEntities::~wListEntities()
{
	if( models != NULL )
		delete[] models;
	models = NULL;
	if( models_name != NULL)
		delete[] models_name;
	models_name = NULL;
}

void wListEntities::Refresh_ListEntities()
{
	// Init datas
	if( models != NULL )
		delete[] models;
	models = fileModels->GetModels(models_count);
	if( models_name != NULL)
		delete[] models_name;
	models_name = new wxString[ models_count ];
	for( int i=0;  i < models_count; i++ )
		models_name[i] = models[i]->GetToken(0);

	// Sort the arrays
	for( int i=0;  i < models_count; i++ )
	{
		size_t curr_ind_replacer =i;
		for( int j=i+1;  j < models_count; j++ )
			if( models_name[curr_ind_replacer].Upper() > models_name[j].Upper() )
				curr_ind_replacer = j;

		wxString temp = models_name[i];
		models_name[i] = models_name[curr_ind_replacer];
		models_name[curr_ind_replacer] = temp;

		ob_models * temp2 = models[i];
		models[i] = models[curr_ind_replacer];
		models[curr_ind_replacer] = temp2;
	}

	// List of entities
	Clear();
	InsertItems( models_count, models_name, 0 );
}


void 
wListEntities::OnSelectionChange(wxCommandEvent& event )
{
	if( panel_EntOverview->loading )
	{
		event.Skip();
		return;
	}

	//Purge the last entity images
	if( entity != NULL )
	{
		wListFrames::theFrameImgManager.Invalidate_ALL();
		entity->PurgeImages();
	}

	//Purge the undo history
	theHistoryManager.Clear_History();
	theHistoryManager.Force_Disable();

	// Load the selected entity
	int curr_selection_ind = GetSelection();
	panel_EntOverview->ChangeEntityOverview( models[curr_selection_ind] );


	// Try to Load the ent image
	if( entity != NULL )
	{
		panel_EntOverview->avatarImg = entity->Load_AvatarImg();
		if( panel_EntOverview->avatarImg == NULL )
			panel_EntOverview->avatarImg = noImg;
	}
	else
		panel_EntOverview->avatarImg = noImg;

	panel_EntOverview->avatarCtrl->SetImage( panel_EntOverview->avatarImg );

	// Update the date of the selected entity
	entityFrame->entitySelection_time++;

	// Update Displayed info
	wxString _name = wxT("NO ENTITY");
	if( entity != NULL )
		_name = entity->Name();
	entityFrame->SetTitle( wxT("ObEditor-") + obeditor_version + wxT(" : ") + + _name);
	wxMenu *t = entityFrame->menuBar->GetMenu(0);
	if( t != NULL )
	{
		wxMenuItemList menuItems = t->GetMenuItems();
		wxMenuItemList::compatibility_iterator iter = menuItems.GetFirst();
		while (iter)
		{
			wxMenuItem *temp = iter->GetData();
			if( temp->GetLabel().Right(4) == wxT("(F2)") )
				temp->SetItemLabel( wxT("Save     <")+ _name + wxT(">     (F2)") );
			if( temp->GetLabel().Right(4) == wxT("(F4)") )
				temp->SetItemLabel( wxT("Reload <")+ _name + wxT(">     (F4)") );
			iter = iter->GetNext();
		}
	}

	if( entity != NULL )
		theHistoryManager.Set_State(true);
		
	event.Skip();
}


// Event table
BEGIN_EVENT_TABLE(wListEntities, myListBox)
	EVT_LISTBOX(wxID_ANY,wListEntities::OnSelectionChange)
END_EVENT_TABLE()
