#ifndef LEVELS__myWnd_EditBackground_H
#define LEVELS__myWnd_EditBackground_H

#include <wx/filepicker.h>
#include <wx/listctrl.h>

#include "common__modalsFrames.h"
#include "common__ob_controls.h"
#include "common__wxFrameImage.h"
#include "common__object_stage.h"


//****************************************************
class Panel_SceneView;
//****************************************************

class myWnd_EditBackground: public wxDialog
{
public:
	myWnd_EditBackground( wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			);
	void Init();
	~myWnd_EditBackground();

	ob_stage* stage;
	ob_StageDeclaration* stage_declaration;
 	ob_StagesSet* stage_set;

	prObFileChooserCtrl* ctrl_path;
	wxTextCtrl *txtCtrl_Xratio,*txtCtrl_Zratio
			, *txtCtrl_Xpos,  *txtCtrl_Zpos
			, *txtCtrl_Xspace,*txtCtrl_Zspace
			, *txtCtrl_Xrepeat,*txtCtrl_Zrepeat;
	wxCheckBox *chkCtrl_transparency;
	wxComboBox *comboCtrl_alpha;
	
	wxListBox* listCtrl_bg;
	Panel_SceneView* sceneView;
	
	void Zero_Layer_Vals();
	void Update_Layer_Vals();
	void Update_Layers_View();

protected:
	size_t nb_layers;
	ob_BG_Layer** oLayers;
	size_t oMap_OldNew_Layers__size;
	ob_BG_Layer** oMap_OldNew_Layers;
	
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
	
	void Evt_bgParamFocusOut( wxFocusEvent& dummy );
	void Evt_SelectedChange( wxCommandEvent& evt_menu );
	void Evt_SelectedDblCick( wxMouseEvent& evt );
	void Evt_MoveLayerUp( wxCommandEvent& evt_menu );
	void Evt_MoveLayerDown( wxCommandEvent& evt_menu );
	void Evt_NewLayer( wxCommandEvent& evt_menu );
	void Evt_DeleteLayer( wxCommandEvent& evt_menu );

	void Evt_bgParamChange( wxCommandEvent& evt_menu );
	
	void EvtActivate( wxActivateEvent& evt );
	void EvtClose( wxCloseEvent& evt );
	void EvtResize( wxSizeEvent& evt );

	DECLARE_EVENT_TABLE()
};




#endif //LEVELS__myWnd_EditBackground_H
