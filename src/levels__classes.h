/*
 * le_classes.h
 *
 *  Created on: 27 févr. 2009
 *      Author: pat
 */

#ifndef LE_CLASSES_H_
#define LE_CLASSES_H_

#include <wx/notebook.h>
#include <set>
#include <map>

#include "common__tools.h"
#include "common__modalsFrames.h"
#include "common__prListBox.h"
#include "common__ob_controls.h"
#include "common__object_stage.h"
#include "common__wxFrameImage.h"

//-----------------------------------------------------------------
class Panel_StageView;
class Ob_GAtom;
//-----------------------------------------------------------------

bool FileLevels_Reload( wxWindow* _parent );
//-----------------------------------------------------------------

enum
{
	LF_SETTINGS_TAGS = 0x0001,
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------
class LevelEditorFrame: public wxFrame
{
public:
	LevelEditorFrame(wxWindow *_parent, const wxString& title );
	~LevelEditorFrame();

	void OnAbout(wxCommandEvent& event);

	void Evt_Quit( wxCommandEvent& evt );
	void EvtResize( wxSizeEvent& event );
	void EvtClose( wxCloseEvent& event );
	void EvtActivate( wxActivateEvent& );
	void EvtMenu(wxCommandEvent& event);
	void EvtCharPress(wxKeyEvent& event);

	void Evt_STAGE__Reload( wxCommandEvent& evt_menu );
	void Evt_STAGE__Save( wxCommandEvent& evt_menu );
	void Evt_SAVE_LevelsTxt( wxCommandEvent& evt_menu );
	void Evt_ReloadAll( wxCommandEvent& evt );
	void Evt_SaveAll( wxCommandEvent& evt );

	void Evt_SET__Select( wxCommandEvent& evt_menu );
	void Evt_SET__Edit_Settings( wxCommandEvent& evt_menu );
	void Evt_SET__Edit_ExtendedSettings( wxCommandEvent& evt_menu );

	void Evt_STAGE__Edit_Declare_Settings( wxCommandEvent& evt_menu );
	void Evt_STAGE__Edit_Settings( wxCommandEvent& evt_menu );
	void Evt_STAGE__Edit_ExtendedSettings( wxCommandEvent& evt_menu );
	void Evt_STAGE__Select( wxCommandEvent& evt_menu );

	void Evt_SET__New( wxCommandEvent& evt_menu );
	void Evt_STAGE__New( wxCommandEvent& evt_menu );

	void Evt_LoadedEntities( wxCommandEvent& evt_menu );
	void Evt_OpenLevelsInEditor( wxCommandEvent& evt_menu );
	void Evt_OpenStageInEditor( wxCommandEvent& evt_menu );

	void Evt_BACKGROUND_Click(wxCommandEvent& evt_menu );
	void Evt_BGLayer_Add( wxCommandEvent& evt_menu );
	void Evt_BGLayer_Click( wxCommandEvent& evt_menu );
	void Evt_BGLayer_Order( wxCommandEvent& evt_menu );
	void Evt_BGTest( wxCommandEvent& evt_menu );

	void Evt_Panel_Add( wxCommandEvent& evt_menu );
	void Evt_Panel_Order( wxCommandEvent& evt_menu );
	void Evt_Panel_Click( wxCommandEvent& evt_menu );

	void Evt_FrontPanels_Edit( wxCommandEvent& evt_menu );
	void Evt_FrontPanel_Click( wxCommandEvent& evt_menu );
	
	void Evt_OBJECT__Edit_Settings( wxCommandEvent& evt_menu );
	void Evt_OBJECT__Edit_ExtendedSettings( wxCommandEvent& evt_menu );

	void Evt_EntitiesListTypeChg( wxCommandEvent& evt_menu );
	void Evt_EntitiesListTxtChg( wxKeyEvent& evt );
	void Evt_EntitySelChg( wxCommandEvent& evt );
	void Evt_EntitiesListRightClik( wxMouseEvent& evt );
	void Evt_EntitiesAddOne(wxCommandEvent& evt );

	void Evt_SuperType_Selection_Change( wxCommandEvent& evt_menu );
	void Evt_SuperType_DblClick( wxMouseEvent& evt );
	void Evt_Type_Selection_Change( wxCommandEvent& evt_menu );
	void Evt_Type_DblClick( wxMouseEvent& evt );
	void Evt_TYPEListRightClik( wxMouseEvent& evt );
	void Evt_TYPEListAddOne( wxCommandEvent& evt_menu );
	void Evt_ObjectList_Selection_Change( wxCommandEvent& evt_menu );

	void Evt_ObjectDuplicate(wxCommandEvent& evt );
	void Evt_ObjectDelete(wxCommandEvent& evt );
	void Evt_ObjectProps(wxCommandEvent& evt );
	void Evt_Object_SortAT(wxCommandEvent& evt );

	void EvtRedo(wxCommandEvent&);
	void EvtUndo(wxCommandEvent&);
	
	void Evt_Help(wxCommandEvent& evt_menu );

public:
	void UpdateTitle();
	
	obFileEntity* Entity__Get( const wxString& item_name );
	
	// Disable Elts : Menu, etc...
	void Disable( const wxString& what_to_disable );
	
	wxString GetModifsDone();
	void SAVE_All();
	void ReloadAll( bool b_quiet, bool b_save_states = true );

	void SET__Save();
	bool SET__Select( const wxString& _name, bool b_no_choose_stage = false );
	void SET__New();
	void SET__Edit_ExtendedSettings();
	void SET__Edit_Settings();
	void SET__Save_LastSession( const wxString& _name );
	void SET__Select_LastSession();
	void SET__ClearMenu_List();
	void SET__Load_List( bool b_quiet = false);
	
	bool STAGE__Save( obFileStage* _fStage );
	bool STAGE__Reload( bool b_quiet = false );
	bool STAGE__Select( const wxString& _name, bool b_no_saves = false );
	void STAGE__New();
	void STAGE__Edit_Declare_Settings();
	void STAGE__Edit_Settings();
	void STAGE__Edit_ExtendedSettings();
	void STAGE__Save_LastSession( const wxString& _name );
	void STAGE__Select_LastSession();
	void STAGE__Clear_List();
	void STAGE__Load_List( bool b_quiet = false );

	void BG_ResetMenuList();
	void BG_AddToMenu( const wxString& _name, int ind = -1, bool b_quiet = false);
	void BG_Reload();
	bool BG_Add();
	bool BG_Order();
	bool BG_Configure( ob_BG_Layer* _obj, bool b_background = false );
	void BG_Test();

	void Panel_ResetMenuList();
	void Panel_AddToMenu( const wxString& _name, int ind = -1, bool b_quiet = false);
	void Panel_Reload();
	bool Panel_Add();
	bool Panel_Order();
	void Panel_Edit( int ind_panel, bool b_quiet = false);

	void FrontPanel_ResetMenuList();
	void FrontPanel_AddToMenu( const wxString& _name,int ind = -1, bool b_quiet = false );
	void FrontPanel_Reload();
	void FrontPanel_Edit_a_panel( int ind_panel, bool b_quiet = false);
	void FrontPanel_Edit_All();

	///////////////////////////////////////////////////////////////////////
	void ENTITIES__Load();
	void ENTITIES__UnLoad();
	void ENTITIES__FILTER__Apply();
	
	///////////////////////////////////////////////////////////////////////
	void NOTEBOOK__Init();
	void NOTEBOOK__UnInit();
	void NOTEBOOK__ClearPages();
	void NOTEBOOK__Save_Current_TypePage();
	
	void NOTEBOOK__SetPage_StaticVal( const wxString& wxW_name, const wxString& val);
	void NOTEBOOK__Evt_pspawn( wxCommandEvent& evt );
	void NOTEBOOK__Evt_Item_pitem( wxCommandEvent& evt );
	void NOTEBOOK__Evt_ItemName_Change( wxCommandEvent& evt );
	void NOTEBOOK__Evt_ItemName_Valid( wxCommandEvent& evt );
	void NOTEBOOK__Evt_ObjVisualChange(wxCommandEvent& evt );
	void NOTEBOOK__Evt_AT_Change(wxCommandEvent& evt );
	
	void NOTEBOOK__Set_MultiSelection();
	void NOTEBOOK__Set_NoSelection(bool b_init = false);
	void NOTEBOOK__Set_With( ob_stage_object* obj );
	void NOTEBOOK__Update_Vals();
	
	///////////////////////////////////////////////////////////////////////
	void OBJECT__Edit_ExtendedSettings();
	void OBJECT__Edit_Settings();
	
	///////////////////////////////////////////////////////////////////////
	// Objects Selections bidules
	///////////////////////////////////////////////////////////////////////
		int  OBJECT__Get_SelectionState( ob_stage_object* obj );
		
		// Objects Selected machins
		void OBJECT__ListSelected__Set( ob_stage_object* obj );
		void OBJECT__ListSelected__Add( ob_stage_object* obj );
		void OBJECT__ListSelected__Rm( ob_stage_object* obj );
		void OBJECT__ListSelected__UpdateNoteBook();
		void OBJECT__ListSelected__Clear();
		bool OBJECT__ListSelected__IsIn( ob_stage_object* obj );
		void OBJECT__ListSelected__Delete();
		void OBJECT__ListSelected__Duplicate();
		void OBJECT__ListSelected__Props(Ob_GAtom* obAtom = NULL );

		// Objects Show States things
		void OBJECT__ShowStates__Clear();
		void OBJECT__ShowStates__Toggle_SuperType( int _st );
		void OBJECT__ShowStates__Add_SuperType(int _st );
		void OBJECT__ShowStates__Remove_SuperType( int _st );
		void OBJECT__ShowStates__Toggle_TYPE( int _t );
		void OBJECT__ShowStates__Add_TYPE(int _t );
		void OBJECT__ShowStates__Remove_TYPE( int _t );

		// Objects Highlighted Stuffs
		void OBJECT__Visibilities_Update();
		void OBJECT__Visibilities_Clear();
		void OBJECT__Visibilities_SaveState();
		void OBJECT__Visibilities_LoadState();

		void SuperType__Select( int _sst );
		void TYPE__Select( int _type, bool b_force = false );
		void TYPE__Reselect();
		int  TYPE__Get_Current_Selected();
		void OBJECT__CtrlList_Selection_Change();
		void OBJECT__CtrlList_Clear();

		// Objects Adding Stuffs
		void OBJECT__Add_Entity( obFileEntity* ent );
		void OBJECT__Add_Default_of_Type( int _TYPE );

	///////////////////////////////////////////////////////////////////////
	void Menu_DoRedos__Update_Labels(std::_List_iterator<History_Group*>);
	void Register_ControlsState_inHistory();
	void Set_ControlsState_In(void*);
	void Restore_ControlsState(void*);
		
	///////////////////////////////////////////////////////////////////////
	void VIEW__Update( bool b_quiet = false );
	void VIEW__Reset();

private:
	// Menu Functions
	void Disable_MenuItems( const wxString& what_to_disable );
	void Menu_ResetListItem( int id_submenMenu_ResetListItemu, int id_startlist );
	bool Menu_AddListItem( int id_submenu, int id_startlist
				   , const wxString& name, bool b_unique = false
				   , int ind_insert =-1 );


public:
	// GENERAL PURPOSE FLAG
	int m_flags;
	
	Panel_StageView* stageView;
	wxArrayString modificationList;
	wxString baseTitle;

	wxBoxSizer *sizer_bottom;
	WndFromText* helpWin;
	
	// Entities View and filters
	wxTextCtrl *entities_filter;
	wxComboBox *cbbox_entities_type;
	prListBox *entities_list;
	MyImageCtrl* entity_img;

	// NoteBook related stuffs
	wxNotebook *onglets;
	GridOb_ObjectProperties *     	more_props;
	std::map<int,wxWindow*> 		map_pages;
	std::map<int,wxString*> 		map_pages_names;
	std::map<wxString,wxWindow*> 		map_idPg_page;
	std::map<int,std::list<wxString> > 	map_pages_elts;
	std::map<int,int> 			map_type_last_active_pg;
	int curr_page;
	ob_stage_object* curr_associated_obj;
	
	std::map<int,std::list<int> >       map_type_l_pgs;
	std::map<int,std::list<wxString> >  map_type_l_disable;
	std::map<int,std::list<wxString> >  map_type_l_enable;
	
	// MenuS...
	wxMenuBar *menuBar;
	wxPanel* mainPanel;
	wxMenu *menuSets;
	wxMenu *menuStage;
	wxMenu *menuScene;
	wxMenu *menuObjects;
	wxMenu *menuUnRedo;

	// Popup menus
	wxMenu *menuAdd; // Just add item
	
	// Tree like List of Type Super Type and atoms
	prListBox *object_SuperType_list;
	prListBox *object_Type_list;
	prListBox *objects_List;

protected:
	bool b_started;
	bool b_closing;
	
	wxString last_txt_filter;


public:
	// For highlighted objects
	int TYPE__curr_selected;
	std::set<ob_stage_object*> aSet_Highlighted;
	std::set<ob_stage_object*> aSet_Selected;
	
	// For Show/Hide states
	std::set<int> aSet_Hide_SuperType;
	std::set<int> aSet_Hide_TYPE;

	// Entities of the mod
	size_t arr_entities__size;
	obFileEntity** arr_entities;
	
	DECLARE_EVENT_TABLE()
};

//****************************************************************
//****************************************************************
typedef struct 
{
	std::set<ob_stage_object*> selected;
}editor_level_states;

int level__onUndo(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   );
int level__onDo(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   );
void level__onNoMoreUndo(bool b_state);
void level__onNoMoreDo(bool b_state);
int  level__onEltAdd(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   );
void level__onDeleteHistory_data(void*&  _elt_datas);
void level__onAfterEltAdded();


#endif /* LE_CLASSES_H_ */
