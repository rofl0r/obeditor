#ifndef LEVELS__myWnd_EditFrontPanels_H
#define LEVELS__myWnd_EditFrontPanels_H

#include <wx/filepicker.h>
#include <wx/listctrl.h>

#include "common__modalsFrames.h"
#include "common__ob_controls.h"
#include "common__wxFrameImage.h"
#include "common__object_stage.h"


//****************************************************
class Panel_SceneView;
//****************************************************

class wnd_EditFrontPanels: public wxDialog
{
public:
	wnd_EditFrontPanels( wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			);
			
	void Init();
	~wnd_EditFrontPanels();

	ob_stage* stage;
	ob_StageDeclaration* stage_declaration;
 	ob_StagesSet* stage_set;

	prObFileChooserCtrl* ctrl_path;
	
	wxListBox* listCtrl_panels;
	Panel_SceneView* sceneView;
	
	void Zero_Panel_Vals();
	void Update_Panel_Vals();
	void Update_Panels_View();

protected:
	size_t nb_panels;
	ob_front_panel** oPanels;
	size_t oMap_OldNew_Panels__size;
	ob_front_panel**oMap_OldNew_Panels;
	
	bool b_reverted_direction;
	
	bool b_not_update_vals; // Set to say not update vals of the current layer
	bool b_updating_vals;
	bool b_initialised;

	void Check_Param_Change();

public:
	bool b_changed;
protected:
	void Evt_OK( wxCommandEvent& evt_menu );
	void Evt_Cancel( wxCommandEvent& evt_menu );
	
	void Evt_ParamFocusOut( wxFocusEvent& dummy );
	void Evt_SelectedChange( wxCommandEvent& evt_menu );
	void Evt_MoveUp( wxCommandEvent& evt_menu );
	void Evt_MoveDown( wxCommandEvent& evt_menu );
	void Evt_New( wxCommandEvent& evt_menu );
	void Evt_Delete( wxCommandEvent& evt_menu );

	void Evt_ParamChange( wxCommandEvent& evt_menu );
	
	void EvtActivate( wxActivateEvent& evt );
	void EvtClose( wxCloseEvent& evt );
	void EvtResize( wxSizeEvent& evt );

	DECLARE_EVENT_TABLE()
};



#endif //LEVELS__myWnd_EditFrontPanels_H
