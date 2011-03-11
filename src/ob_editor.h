#ifndef OB_EDITOR_H_
#define OB_EDITOR_H_

#include <wx/notebook.h>
#include <vector>
#include <map>

#include "common__tools.h"
#include "common__modalsFrames.h"
#include "CHistory.h"

/**
 * The Application.
 *
 * The application central start and end of the program
 *
 */


class MyApp: public wxApp
{
public:
	bool auto_load;
	wxString autoLoadFrame;


private:
    virtual bool OnInit();
   	virtual int OnExit();
};

extern MyApp* theApp;

/**
  * Main Window.
  * The main Window for the Application
  */

class StartFrame: public wxFrame
{
public:
	StartFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	void ShowEntityFrame();
	void ShowleFrame();

	void OnEditEntities(wxCommandEvent& event);
	void OnLoadStages(wxCommandEvent& event);
	void OnCheckAutoLoad(wxCommandEvent& event);

	void evtClose( wxCloseEvent& );
	void EvtActivate( wxActivateEvent& );

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnAddProject(wxCommandEvent& event);
	void OnDelProject(wxCommandEvent& event);
	void OnRenameProject(wxCommandEvent& event);
	void OnChgDirProject(wxCommandEvent& event);
	void OnRepairWindaubePath(wxCommandEvent& WXUNUSED(event));
	void OnSetEditor(wxCommandEvent& event);

	bool LoadCurrentProject_Models();
	bool LoadCurrentProject_Levels();

	wxListBox *lstBx_projects;
	wxCheckBox *chckbx_autoLoad;
	bool b_init;
	bool b_reset_entityFrame;
	bool b_reset_leFrame;
	wxString frame_launched;
    DECLARE_EVENT_TABLE()
};
extern StartFrame *startFrame;

/**
  * Edit Entity window.
  * The Window where to edit entity properties and animations.
  */
class Panel_withAnims;

class EntityFrame: public wxFrame
{
public:
	EntityFrame(wxWindow *_parent, const wxString& title );
	~EntityFrame();

	void OnEditEntities(wxCommandEvent& event);
	void OnLoadStages(wxCommandEvent& event);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	
	void EvtUndo(wxCommandEvent& event);
	void EvtRedo(wxCommandEvent& event);

	void EvtResize( wxSizeEvent& event );
	void EvtClose( wxCloseEvent& event );
	void EvtActivate( wxActivateEvent& );
	void EvtMenu(wxCommandEvent& event);
	void EvtCharPress(wxKeyEvent& event);

	void Reload_Frames();
	void Reload_Anims();
	void PanelPageChanging(wxNotebookEvent& event );
	void SetPanelErrorsIntitule( const wxString& new_intitule );

	Panel_withAnims* GetCurrentPanel();
	
	wxMenu *menuHistory;
	wxMenu *menuUnRedo;
	wxNotebook *onglets;
	bool b_NoAnimPage;
	int entitySelection_time;	//Keep a trace of change of selected entity
	int entityLastSelection_time;

	wxMenuBar *menuBar;
	
	std::map<wxString,wxString> map_ent_animHistory;
	void EntityAnimsHistory__Append(const wxString& _ent_name);
	void EntityAnimsHistory__Clear();

	// Undo/Redo stuffs
	void Menu_DoRedos__Update_Labels(std::list<History_Group*>::iterator it_group);
	void Restore_ControlsState( void* pStates );
	bool May_Register_ControlsState();
	void Register_ControlsState_inHistory( const wxString& f_caller );
	void Set_ControlsState_In( void* elt_datas );

	int m_flag;
	
private:
	std::vector<wxString> vect_ent_history;
	
	bool b_started;
    DECLARE_EVENT_TABLE()
};
extern EntityFrame *entityFrame;



/**
  * Level Editor window.
  */

class LevelEditorFrame;
extern LevelEditorFrame *leFrame;


/**
  * Enumeration of ID for StartFrame
  *
  */

enum
{
    ID_Quit  = 101,
    ID_About = 102,

    ID_AUTO_LOAD = 103,

    ID_LOAD_STAGES = 104,
    ID_EDIT_ENTITIES = 105,

    ID_LIST_PROJECTS = 106,
    ID_ADD_PROJECT = 107,
    ID_DEL_PROJECT = 108,
    ID_RENAME_PROJECT = 109,
    ID_CHGDIR_PROJECT = 110,

    ID_WND_CHG_PJCT_NAME = 111,
    ID_REPAIR_FILEPATHS = 112,

    ID_ENTITIES_CURR_SAVE = 234,
    ID_ENTITIES_CURR_RELOAD = 235,
    ID_ENTITIES_SAVEALL = 236,
    ID_ENTITIES_RELOADALL = 237,
    ID_ENTITIES_RELOAD_MODELSTXT = 238,

    ID_ENTITIES_RELOAD_GIFS = 239,
    ID_HELP_PLEASE = 240,
    ID_SET_EDITOR = 241,
    ID_OPEN_IN_EDITOR = 242,
    ID_BACK_TO_STARTFRAME = 243,

    ID_LEVEL_FIRST_ID = 244,
/*
 * RESERVED To Stages ID in menu
 * 244
 * TO
 * 300
 */

    ID_STAGE_NEW =  301,
    ID_STAGE_EDIT_SETTINGS =  302,

    ID_STAGE_RELOAD =  303,
    ID_STAGE_RELOAD_ALL = 304,

    ID_STAGE_SAVE =  3003,
    ID_STAGE_SAVE_ALL = 3004,

    ID_OBJECT_COPY = 3005,
    ID_OBJECT_SUPP = 3006,
    ID_OBJECT_PROPERTIES = 3007,
    ID_OBJECT_SORT_AT_OBJECTS = 3008,

    ID_STAGE_LOADED_ENTITIES = 308,

    ID_BACKGROUND = 310,
    ID_SCENE_BG_LIST_START = 311,
    /* RESERVED TO 349 */
    ID_SCENE_BG_EDIT_ORDER = 350,

    ID_SCENE_PANELS_LIST_START = 360,
    /* RESERVED TO 399 */
    ID_SCENE_PANELS_EDIT_ORDER = 400,

    ID_SCENE_WATER = 401,
    ID_SCENE_FRONTPANEL = 402,
    ID_SCENE_TEST = 403,
    ID_SCENE_MENU_BG = 404,
    ID_SCENE_MENU_PANELS = 405,

    ID_STAGE_LIST = 409,
    ID_STAGE_LIST_START = 410,
    /* RESERVED TO 499 */

    ID_SET_LIST = 500,
    ID_SET_LIST_START = 501,
    /* RESERVED TO 549 */
    ID_SET_NEW = 550,
    ID_SET_EDIT_SETTINGS = 551,

    ID_LEVELSTXT_SAVE = 552,
    ID_OPEN_LEVELSTXT_IN_EDITOR = 553,

    ID_SET_EDIT_EXTENDED_PROPS =  554,
    ID_STAGE_EDIT_EXTENDED_PROPS =  555,
    ID_OBJECT_EDIT_EXTENDED_PROPS =  556,
    ID_STAGE_EDIT_DECLARE_SETTINGS = 557,

    ID_SCENE_FRONTPANEL_ORDER = 559,
    ID_SCENE_FRONTPANEL_LIST_START = 560,
    /* RESERVED TO 599 */

    ID_SCENE_BG_ADD = 600,
    ID_SCENE_PANELS_ADD = 601,
    
    
    ID_ENTITIES_HISTORY_CLEAR = 610,
    ID_ENTITIES_HISTORY_START = 620,
    /* RESERVED TO 640 */

    ID_UNDO = 641,
    ID_REDO = 642,
};





#endif /*OB_EDITOR_H_*/
