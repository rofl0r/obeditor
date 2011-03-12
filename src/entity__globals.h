#ifndef EDIT_ENTITY_H_
#define EDIT_ENTITY_H_

#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/filepicker.h>
#include <wx/listctrl.h>

#include "common__tools.h"
#include "common__wxFrameImage.h"
#include "common__modalsFrames.h"
#include "common__ob_controls.h"
#include "CHistory.h"
#include "entity__wListAnims.h"
#include "entity__wListFrames.h"

extern wxString obeditor_version;

//****************************************************
//************ FRAME OBJECTS  **********************
class PanelEntity_Overview;
extern PanelEntity_Overview *panel_EntOverview;

class Panel_Anims;
extern Panel_Anims *panel_Anims;

class Panel_Errors;
extern Panel_Errors *panel_Errors;

class Panel_Remaps;
extern Panel_Remaps *panel_Remaps;

class Panel_Platform;
extern Panel_Platform *panel_Platform;

//****************************************************
//****************************************************

//************ ERRORS PANEL CLASS  *****************

//****************************************************
//****************************************************



class Panel_Errors : public wxPanel
{
public:
	Panel_Errors( wxNotebook *onglets, const wxString& _intitul_panel );
	~Panel_Errors();

	void EvtSize( wxSizeEvent& event );
	void EvtClear( wxCommandEvent& event);
	void Item_DblClick( wxListEvent& event );
	void EvtKeyDown( wxListEvent& event );
	void EvtActivate(wxActivateEvent&);
	void EvtGetFocus(wxFocusEvent&);

	void Log( int id_img, const wxString& src, const wxString& message );

private:
	int nb_errors;
	wxString intitule_panel;
	void Update_NbErrors();
	void Init_LstError();
	wxListView *lst_errors;
    DECLARE_EVENT_TABLE()
};



//****************************************************
//****************************************************

//************ OVERVIEW PANEL CLASS  *****************

//****************************************************
//****************************************************


class wListEntities : public myListBox
{
public:
	wListEntities(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxT("listBox"));
	~wListEntities();
	void Refresh_ListEntities();
	virtual void OnSelectionChange( wxCommandEvent& event );

private:
    DECLARE_EVENT_TABLE()
};



class PanelEntity_Overview : public wxPanel
{
public:
	PanelEntity_Overview( wxNotebook *onglets );
	~PanelEntity_Overview();
	void EvtActivate(wxActivateEvent&);
	void EvtGetFocus(wxFocusEvent&);
	void EvtClose( wxCloseEvent& );

	void ChangeEntityOverview(ob_models*);
	void OnEntityPropertyChange( wxGridEvent& event );
	void AddNewGridRow(wxCommandEvent& event);
	void UpdatePropGridSize();
	void Update_PagesAccessible();

	wListEntities *list_entities;
	GridOb_ObjectProperties *list_ent_Props;
	wxTextCtrl *wPathEntityFile;
	wxStaticText *fileErrorMessage;
	MyImageCtrl *avatarCtrl;
	wxImage* avatarImg;
	int EntityChanged();

	void SaveAll( bool b_ask = true);
	void Save_Current(bool b_ask = true);
	void Reload_Current();
	void ReloadAll();
	void Reload_EntityList();

	bool Select_Entity( const wxString& _name );
	
	bool loading;
//	wxFilePickerCtrl *wPathEntityFile_Choose;		//FIXME: implement the change of the entity file location

private:
    DECLARE_EVENT_TABLE()
};


//****************************************************
//****************************************************
//************ BASE CLASS PANEL WITH ANIMS ***********
//****************************************************

class Panel_withAnims : public wxPanel
{
public:
	// To know when the entity have change
	static int entityAnimation_time;

public:
	Panel_withAnims( wxNotebook *onglets );
	~Panel_withAnims();

	virtual void Frame_CascadeChanges();

	virtual void EvtClose( wxCloseEvent& );;
	virtual void EvtSize( wxSizeEvent& event );
	virtual void EvtAnimListChange(wxCommandEvent& event);
	virtual void EvtAnimSelectionChange(wxCommandEvent& event);
	virtual void EvtFramesListChange(wxCommandEvent& event);
	virtual void EvtFrameSelectionChange(wxCommandEvent& event);
	virtual void EvtGetFocus(wxFocusEvent& event );

	virtual void OnActivate();
	virtual void Refresh();
	virtual void Reload();
	virtual void ReloadGifs();

	virtual void NoFocus(wxFocusEvent& event);
	virtual void ThrowFocus(wxCommandEvent& event);
	virtual bool EntityChanged();

	wListAnims *list_Anims;
	wListFrames *wList_frames;

	int this_entityAnimation_time;
	bool b_entity_has_change;

protected:
	enum
	{ 
		M_CHANGING_ANIM = 1,
		NO_PROCESS_ATT_CHANGES = 2,
		M_ON_ACTIVATE	= 4,
	};
	int m_flag;
	
	DECLARE_EVENT_TABLE()
};


//****************************************************
//****************************************************
//************ ANIMATION PANEL CLASSES  **************
//****************************************************

class TheScrollPanel : public AScrollPanel
{
public :
	TheScrollPanel( wxWindow *_parent );
	~TheScrollPanel();

	void EvtSndPickerChg( wxFileDirPickerEvent& event );
	void EvtImgPickerChg( wxFileDirPickerEvent& event );
	void EvtCommand( wxCommandEvent& event );
	void EvtChildFocus_Kill(wxFocusEvent& event );
	void EvtButtonClick(wxCommandEvent& event);

};

//****************************************************
class MyAnimationCtrl; class MyAnimationZone; class MyPaletteElementCtrl;


class Panel_Anims : public Panel_withAnims
{
public:
	Panel_Anims( wxNotebook *onglets );
	~Panel_Anims();
	void OnActivate();
	void Refresh();
	virtual void Reload();
	virtual void ReloadGifs();
	void ResetFrameDatas();
	void EnableFrameDatas( bool b_enable = true);
	void Refresh_FrameData_AnimControl(int refreshtype=3);

	void EvtCommand( wxCommandEvent& event );
	void DoCommand( int _ctlID, int force_updateMode = 0 );
	void EvtChildFocus_Kill(wxFocusEvent& event );
	void EvtButtonClick(wxCommandEvent& event);
	void EvtKillFocus(wxFocusEvent& event );
	void EvtClose( wxCloseEvent& );
	void EvtCharPress(wxKeyEvent& event);
	void EvtSize( wxSizeEvent& event );
	void EvtImgPickerChg( wxFileDirPickerEvent& );
	void EvtSndPickerChg( wxFileDirPickerEvent& event );
	void EvtAnimSelectionChange(wxCommandEvent& event);

	void EvtFrameSelectionChange(wxCommandEvent& event);
	void EvtAnimListChange(wxCommandEvent& event);
	void EvtFramesListChange(wxCommandEvent& event);

	void Evt_Attack_BaseName_Change(wxCommandEvent& event);
	void Evt_Attack_NumChange( wxCommandEvent& event );
	void Evt_Attack_ValChange( wxCommandEvent& event );

	wxString ATTACK_GetName();
	void ATTACK__SetActive( const wxString& att_name );
	void ATTACK__OnUpdate(ob_frame_attack* att);
	void ATTACK__Set_Clone(wxCommandEvent& event);
	void ATTACK__Set_Zero(wxCommandEvent& event);

	void Evt_TxtCtrlCommand( wxCommandEvent& event );
	void Evt_TxtCtrl_KeyUp(wxKeyEvent& event );
	void Evt_Ctrl_FrameNavig(wxKeyEvent& event );
	
	void Update_FrameDatas();
	void EnableNavigation( const int code, bool enable = true);
	void SetBigFrameNumber( int _num );
	void DoDeleteOffset( wxCommandEvent& event);
	void DoDeleteBBox(wxCommandEvent& event);
	void DoDeleteDrawmethod(wxCommandEvent& event);

	void Rescale_Boxes();
	
	// All Control of the Anim/Frame
	TheScrollPanel *scrollPanel;

	wxButton *animation_ctrlprop;
	wxButton *butt_ExtFramProp;
	wxTextCtrl *txtctrl_imgFile;
	wxFilePickerCtrl *pickerFrame;
	wxTextCtrl *txtctrl_soundFile;
	wxFilePickerCtrl *pickerSound;
	wxTextCtrl *txtctrl_delay;
	wxTextCtrl *txtctrl_offset_X;
	wxTextCtrl *txtctrl_offset_Y;
	wxTextCtrl *txtctrl_move_X;
	wxTextCtrl *txtctrl_move_A;
	wxTextCtrl *txtctrl_move_Z;
	MyPaletteElementCtrl *bbox_color;
	wxTextCtrl *txtctrl_bbox_X;
	wxTextCtrl *txtctrl_bbox_Y;
	wxTextCtrl *txtctrl_bbox_W;
	wxTextCtrl *txtctrl_bbox_H;
	wxTextCtrl *txtctrl_bbox_Z;
	wxBitmapButton *del_bbox_butt;
	wxChoice* choiceBox_attacks;
	wxStaticText *txt_attacks;
	wxTextCtrl* txtctrl_att_num;
	wxBitmapButton *del_attbox_butt;
	MyPaletteElementCtrl *att_color;
	wxTextCtrl *txtctrl_attbox_X;
	wxTextCtrl *txtctrl_attbox_Y;
	wxTextCtrl *txtctrl_attbox_W;
	wxTextCtrl *txtctrl_attbox_H;
	wxCheckBox *chckbx_block;
	wxCheckBox *chckbx_flash;
	wxTextCtrl *txtctrl_dmgs;
	wxStaticText *txt_pow;
	wxTextCtrl *txtctrl_pow;
	wxTextCtrl *txtctrl_pause;
	wxTextCtrl *txtctrl_zrange;

	wxCheckBox *chckbx_flipframe;

	wxCheckBox *chckbx_nodrawmethod;
	wxTextCtrl *txtctrl_scaleX;
	wxTextCtrl *txtctrl_scaleY;
	wxCheckBox *chckbx_flipx;
	wxCheckBox *chckbx_flipy;
	wxTextCtrl *txtctrl_shiftX;
	wxTextCtrl *txtctrl_alpha;
	wxTextCtrl *txtctrl_remap;
	wxTextCtrl *txtctrl_fillcolor;
	wxTextCtrl *txtctrl_rotate;
	wxCheckBox *chckbx_fliprotate;

	wxStaticBoxSizer *sizer_Jump;
	wxCheckBox *chckbx_jumpframe;
	wxStaticBox *staticBox_jump_datas;
	wxStaticText *txt_jump_h;
	wxTextCtrl *txtctrl_jump_h;
	wxStaticText *txt_jump_x;
	wxTextCtrl *txtctrl_jump_x;
	wxStaticText *txt_jump_z;
	wxTextCtrl *txtctrl_jump_z;
	wxCheckBox *chckbx_dropframe;
	wxCheckBox *chckbx_landframe;

	// The big frame number
	wxStaticText *txt_FrameNumber;

	// Animation navigation
	wxButton* butt_Zoom_M;
	wxButton* butt_Zoom_P;
	wxButton* butt_ToStart;
	wxBitmapButton* butt_Play;
	wxBitmapButton* butt_Loop;
	wxBitmapButton* butt_Sound;
	wxButton* butt_ToEnd;

	// Set boxes ctrls
	wxButton *toOffset;
	wxButton *toXA;
	wxButton *toAtt;
	wxButton *toBBox;

	// The Animation control
	MyAnimationCtrl* animation_ctrl;
private:
	int itState;
	void ResetAttacksFrameDatas();
	void EnableFrameAttacksData( bool b_enable);
	void EnableDrawMethod( bool b_enable );
	void DoEditAnimProps();
	void DoEditFrameProps();
//	void NoFocus(wxFocusEvent& event);

    DECLARE_EVENT_TABLE()
};



//****************************************************
//****************************************************
//************ REMAPS PANEL CLASSES  **************
//****************************************************
class MyPaletteCtrl;class MyRemapViewerCtrl;


class Panel_Remaps : public Panel_withAnims
{
public:
	Panel_Remaps( wxNotebook *onglets );
	void Constructor();
	~Panel_Remaps();
	void Refresh();
	virtual void Reload();
	virtual void ReloadGifs();

	bool EntityChanged();
	void OnActivate();

	void EvtClose( wxCloseEvent& );
	void EvtSize( wxSizeEvent& event );

	void EvtAnimSelectionChange(wxCommandEvent& event);
	void EvtFrameSelectionChange(wxCommandEvent& event);
	void EvtFramesReload(wxCommandEvent& event);

	void EvtRemapChange( wxCommandEvent& event );
	void Evt8BitModeChange( wxCommandEvent& event );
	void EvtCommand( wxCommandEvent& event );
	void EvtButtonClick(wxCommandEvent& event);
	void EvtPickerImgBaseChg( wxFileDirPickerEvent& );
	void EvtPickerImgDestChg( wxFileDirPickerEvent& event );
	void EvtSlidersChg( wxScrollEvent& event );
	void EvtPaletteRemapChange(wxCommandEvent& event );
	void EvtKillFocus(wxFocusEvent& event );
	void DoRemapSelectionChange( bool b_quiet = false );
	void EnableEditingTheRemap(int mode=0);
	void InitRemapsChoices();

	bool Save_DoChecks();
	bool Save_16Bit_Remap();
	bool Save_8Bit_Remap();

	void GessBitMode();
	void SwitchTo8BitMode();
	void SwitchTo16BitMode();
	void EnableHSV( bool b_enable );
	void ResetHSV();

	bool DoChangeImgBase( bool b_quiet = false, bool standalone = true );
	bool DoChangeImgDest( bool b_quiet = false, bool standalone = true);
	bool Do_NewRemap();
	bool Do_RenameRemap();
	bool Do_DeleteRemap();
	void DoCreateBase();

	int curr_ind_remap;
	ob_object* curr_remap;
	bool mode8bit;

	AScrollPanel *scrollPanel;

	wxBoxSizer *sizer_remaps_choice;
	wxStaticText *txt_remaps;
	wxChoice* choiceBox_remaps;
	wxCheckBox* chckbx_8bit;

	wxButton *btn_delete_remap;
	wxButton *btn_rename_remap;

	wxStaticBox *staticBoxPath;
	wxStaticText *txt_base_img;
	wxTextCtrl *txtctrl_imgBase;
	wxFilePickerCtrl *pickerBaseImg;
	wxStaticText *txt_result;
	wxTextCtrl *txtctrl_destimg;
	wxFilePickerCtrl *pickerDestImg;

	wxGridSizer *sizer_tools_btns;
	wxButton *btn_guess_cols;
	wxButton *btn_save_remap;
	wxButton *btn_set_sel;
	wxButton *btn_unset_sel;

	wxStaticBoxSizer *sizer_HSV;

	wxStaticText *txt_HSL;
	wxSlider *hue_ctrl;
	wxSlider *sat_ctrl;
	wxSlider *lum_ctrl;

	wxStaticText *txt_RGB;
	wxSlider *red_ctrl;
	wxSlider *green_ctrl;
	wxSlider *blue_ctrl;

	wxButton *btn_apply_cols;
	wxButton *btn_cancel_cols;

	MyPaletteCtrl *paletteCtrl;

	MyRemapViewerCtrl *frameView;

	int itState;
    DECLARE_EVENT_TABLE()
};


//****************************************************
//****************************************************
//************ PLATFORM PANEL CLASSES  **************
//****************************************************
class MyPlatformViewerCtrl;


class Panel_Platform : public Panel_withAnims
{
public:
	Panel_Platform(wxNotebook* _parent );
	virtual void OnActivate();
	virtual ~Panel_Platform();
	void Update_entity_platform();
	virtual void Reload();
	virtual void ReloadGifs();

	AScrollPanel *scrollPanel;

	wxCheckBox* chckbx_noplatform;

	wxTextCtrl *txtctrl_off_x;
	wxTextCtrl *txtctrl_off_y;
	wxTextCtrl *txtctrl_up_l;
	wxTextCtrl *txtctrl_d_l;
	wxTextCtrl *txtctrl_up_r;
	wxTextCtrl *txtctrl_l_r;
	wxTextCtrl *txtctrl_depth;
	wxTextCtrl *txtctrl_alt;

	MyPaletteElementCtrl *plat_color;

	MyPlatformViewerCtrl *platformView;

protected:
	void EvtCommand(wxCommandEvent& event );
	void EvtButtonClick(wxCommandEvent& event);
	void EvtFrameSelectionChange(wxCommandEvent& event);
	void EvtAnimSelectionChange(wxCommandEvent& event);

	void UpdateBG_colours( ob_object* platform );
	void Update_PlaftformDatas();
	void EnablePlatform( bool b_enable );
	int off_x,off_y;
	int up_l, up_r;
	int d_l, d_r;
	int depth, alt;

    DECLARE_EVENT_TABLE()
};


//****************************************************************
//****************************************************************
typedef struct 
{
	wxString anim_selected;
	int frame_sel_start;
	int frame_sel_end;
	int frame_selected;
}editor_entities_states;

int ent__onUndo(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   );
int ent__onDo(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   );
int ent__onEltAdd(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
		   );
void ent__onNoMoreUndo(bool b_state);
void ent__onNoMoreDo(bool b_state);
void ent__onDeleteHistory_data(void*&  _elt_datas);

#endif /*EDIT_ENTITY_H_*/
