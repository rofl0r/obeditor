/*
 * levels__sidesWindows.h
 *
 *  Created on: 27 avr. 2009
 *      Author: pat
 */

#ifndef LEVELS__SIDESWINDOWS_H_
#define LEVELS__SIDESWINDOWS_H_

#include <wx/filepicker.h>
#include <wx/listctrl.h>

#include "common__modalsFrames.h"
#include "common__ob_controls.h"
#include "common__wxFrameImage.h"
#include "common__object_stage.h"

//****************************************************
class ob_StagesSet;

class myWnd_SETProperties : public FrameEditProperties
{
public:
	myWnd_SETProperties(wxWindow *parent, ob_StagesSet* _set = NULL, const wxString& wnd_name = wxT("NoName..."));
	~myWnd_SETProperties();

	virtual void InitProps();
	virtual void InitObj();

};



//****************************************************
class ob_StageDeclaration;

class myWnd_LEVELSSTAGEProperties : public FrameEditProperties
{
public:
	myWnd_LEVELSSTAGEProperties(wxWindow *parent, ob_StageDeclaration* _stageDeclar, bool b_init = true, const wxString& wnd_name = wxT("NoName...") );
	~myWnd_LEVELSSTAGEProperties();

	virtual void InitProps();
	virtual void InitObj();


protected:
	virtual void Clicked_Cancel(bool b_close = true);
	virtual void Clicked_OK(bool b_close = true);
	virtual void Cleanings();

	bool b_has_branch;
};



//****************************************************
class ob_stage;

class myWnd_STAGEProperties : public FrameEditProperties
{
public:
	myWnd_STAGEProperties(wxWindow *parent, ob_stage* _stage );
	~myWnd_STAGEProperties();

	virtual void InitProps();
	virtual void InitObj();


protected:
	virtual void Clicked_Cancel(bool b_close = true);
	virtual void Clicked_OK(bool b_close = true);
	virtual void Cleanings();

	bool b_has_branch;
};



//****************************************************

class myWnd_STAGE_New : public myWnd_LEVELSSTAGEProperties
{
public:
	myWnd_STAGE_New(wxWindow *parent );
	~myWnd_STAGE_New();

	virtual void InitControls();

protected:
	virtual void Clicked_Cancel(bool b_close = true);
	virtual void Clicked_OK(bool b_close = true);
	virtual void Cleanings();

	bool b_has_branch;
	wxComboBox* cbCtrl_where;
};


//****************************************************

class myWnd_PanelProperties : public FrameEditProperties
{
public:
	myWnd_PanelProperties( wxWindow* _parent, ob_stage_panel* _stage, bool b_init = true );
	~myWnd_PanelProperties();

	virtual void InitProps();
	virtual void InitObj();

protected:

	wxString path_norm;
	wxString path_neon;
	wxString path_screen;

	virtual void Clicked_Cancel(bool b_close = true);
	virtual void Clicked_OK(bool b_close = true);

};

//****************************************************

class myWnd_FrontPanelProperties : public FrameEditProperties
{
public:
	myWnd_FrontPanelProperties( wxWindow* _parent, ob_front_panel* _stage, bool b_init = true );
	~myWnd_FrontPanelProperties();

	virtual void InitProps();
	virtual void InitObj();

protected:
	wxString path;
	
	virtual void Clicked_Cancel(bool b_close = true);
	virtual void Clicked_OK(bool b_close = true);

};


//****************************************************

class myWnd_BGProperties : public FrameEditProperties
{
public:
	myWnd_BGProperties( wxWindow* _parent, ob_BG_Layer* _bgl, bool _b_background = false, bool b_init = true  );
	~myWnd_BGProperties();

	virtual void InitProps();
	virtual void InitObj();

protected:

	bool b_background;
	virtual void Clicked_Cancel(bool b_close = true);
	virtual void Clicked_OK(bool b_close = true);

};




//****************************************************

class myWnd_LoadedEnt : public wxDialog
{
public:
	myWnd_LoadedEnt( wxWindow* _parent, ob_stage* _stage );
	~myWnd_LoadedEnt();

};


//****************************************************

class myWnd_EditPanelOrder: public wxDialog
{
public:
	myWnd_EditPanelOrder( wxWindow* _parent, ob_stage* _stage );
	void ZeroInit();
	void Init();
	~myWnd_EditPanelOrder();

	ob_stage* stage;
	bool b_reverted_direction;
	bool b_rotate_imgs;

	size_t nb_panels;
	ob_stage_panel** l_panels;
	size_t nb_orders;
	ob_object** orders;

	bool b_chg;
	int tay_panels_map;
	int* panels_map;

protected:
	void EvtClose( wxCloseEvent& event );
	void EvtSize( wxSizeEvent& evt );
	void Evt_Click_OK( wxCommandEvent& evt );
	void Evt_Click_CANCEL( wxCommandEvent& evt );
	void Evt_Click_OrderElt( wxMouseEvent& evt );
	void Evt_Order_KeyPress( wxKeyEvent& evt );
	void Evt_Order_Mouse_In( wxMouseEvent& evt );
	void Evt_Order_Changed( wxCommandEvent& evt );
	void Evt_Paint_PanelView( wxPaintEvent& evt );

	int Get_OrderElt_X( int ind);
	void Update_PanelsView();
	void UpdateInsertPoint();
	void MoveImagesCursor( long i );

	AScrollPanel *panel_stage_view;
	wxString curr_order;
	wxTextCtrl* txtCtrl_order;
	wxScrolledWindow* thePanel;
	wxFont font_Alphabet;
	wxFont font_Label;

	MyImageCtrl** img_ctrl_list;
	int img_ctrl_list_count;
	int curr_ind;
	int panel_cursor_x;

	DECLARE_EVENT_TABLE()
};

//****************************************************
class Panel_SceneView;
//****************************************************

class myWnd_SceneTest: public wxDialog
{
public:
	myWnd_SceneTest( wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			);
	~myWnd_SceneTest();

	Panel_SceneView* sceneView;
	
	void Set_Panels( ob_stage_panel**& p_panels, size_t& p_nb_panels );
	void Set_Background( ob_BG_Layer**& p_bg_layers, size_t& p_nb_layers );
	void Set_Front_Panels(ob_front_panel**& p_front_panels, size_t& p_nb_front_panels );

protected:
	void EvtClose( wxCloseEvent& event );
	DECLARE_EVENT_TABLE()
};


//****************************************************
#endif /* LEVELS__SIDESWINDOWS_H_ */
