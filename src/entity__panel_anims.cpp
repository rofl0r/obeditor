#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "ob_editor.h"
#include "images__MyPalette.h"
#include "entity__globals.h"
#include "entity__MyAnimationZone.h"
#include "entity__enums.h"


using namespace std;

//****************************************************
//***********VARIABLES DECL***********************

WX_DECLARE_HASH_MAP( wxString, wxTextCtrl*, wxStringHash, wxStringEqual, hash_name_textCtrl );
hash_name_textCtrl hash_framePropName_textCtrl;

Panel_Anims *panel_Anims;

//****************************************************
// Utils functions
//****************************************************

#define __ARR_DATA_CTRLS_MAX 30
size_t __arr_data_ctrls__curr_ind;
void ***__arr_data_ctrls;

enum {
		_MY_CHKBOX,
		_MY_TEXT,
		_MY_CHOICE,
	};

void __PushCtrlData( const wxString& _propName, const wxString& def_val, int _ctrlType, wxObject* ctrl)
{
	void **_tt = new void*[4];
	_tt[0] = (void*) new wxString(_propName);
	_tt[1] = (void*) new wxString(def_val);
	_tt[2] = new int(_ctrlType);
	_tt[3] = (void*) ctrl;
	__arr_data_ctrls[__arr_data_ctrls__curr_ind] = _tt;
	__arr_data_ctrls__curr_ind++;
}

void __delete_arr_data_ctrls()
{
	for( size_t i =0; i < __arr_data_ctrls__curr_ind; i++)
	{
		wxString *t = (wxString*) __arr_data_ctrls[i][0];
		delete t;
		t = (wxString*) __arr_data_ctrls[i][1];
		delete t;

		int *t2 = (int*) __arr_data_ctrls[i][2];
		delete t2;
		delete[] __arr_data_ctrls[i];
	}
	delete[] __arr_data_ctrls;
	__arr_data_ctrls__curr_ind = 0;
	__arr_data_ctrls = NULL;
}


//****************************************************
//			PANEL ANIMS
//****************************************************

#define ATT_CTRL_CONNECT( TXTCTRL, EVT_TYPE, EVT_FUNCTION ) \
	TXTCTRL->Connect( EVT_TYPE , wxCommandEventHandler(Panel_Anims::EVT_FUNCTION), NULL, this);

Panel_Anims::Panel_Anims( wxNotebook *onglets )
:Panel_withAnims(onglets)
{
	// Needed by subobjects
	panel_Anims = this;
	m_flag = 0;

	// Some inits
	entityAnimation_time = -1;
	itState = 0;
	animation_ctrl = NULL;

	__arr_data_ctrls = new void**[__ARR_DATA_CTRLS_MAX];
	__arr_data_ctrls__curr_ind = 0;

	default_BG_colour = GetBackgroundColour();

	//******************
	//* MAIN SIZERS
	//******************

	//top level sizer
		wxBoxSizer *sizer_main = new wxBoxSizer( wxVERTICAL );

	//Top part of the panel
		wxBoxSizer *sizer_top = new wxBoxSizer( wxHORIZONTAL );
		sizer_main->Add( sizer_top, 1, wxALL  | wxEXPAND);

		wxBoxSizer *sizer_topLeft = new wxBoxSizer( wxHORIZONTAL );
		sizer_top->Add( sizer_topLeft, 0, wxALL | wxEXPAND);

		wxBoxSizer *sizer_scrollWndFrameProps = new wxBoxSizer( wxVERTICAL );
		sizer_top->Add( sizer_scrollWndFrameProps, 0, wxALL | wxEXPAND, 6 );

		wxBoxSizer *sizer_topRight = new wxBoxSizer( wxVERTICAL );
		sizer_top->Add( sizer_topRight, 1, wxALL | wxEXPAND );

		wxBoxSizer *sizer_topRightUp = new wxBoxSizer( wxHORIZONTAL );
		sizer_topRight->Add( sizer_topRightUp, 0, wxALL | wxEXPAND );

		wxBoxSizer *sizer_topRightDown = new wxBoxSizer( wxVERTICAL );
		sizer_topRight->Add( sizer_topRightDown, 1, wxALL | wxEXPAND );


	//Bottom part of the panel
		wxSizer *sizer_bottom = new wxBoxSizer( wxHORIZONTAL );
		sizer_main->Add( sizer_bottom, 0, wxALL | wxEXPAND  );

	//******************
	//* OBJECTS
	//******************

		//**************************
		// LIST OF ANIMS
		//**************************

		list_Anims = new wListAnims_Editable(this);
		list_Anims->SetMinSize( wxSize(150,50));
		sizer_topLeft->Add( list_Anims, 0, wxALL |wxEXPAND );


		//**************************
		// CENTER PANEL
		//**************************

		scrollPanel = new TheScrollPanel( this );
		sizer_scrollWndFrameProps->Add( scrollPanel, 1, wxALL, 0 );

		wxBoxSizer *sizer_topCenter = new wxBoxSizer( wxVERTICAL );
		int fix_spacer_height = 12;


		//**************************
		// PROPS BUTTONS
		//**************************
		int btns_width = 170;
		wxBoxSizer *sizer_topCentertop = new wxBoxSizer( wxVERTICAL );
		sizer_topCenter->Add( sizer_topCentertop, 0, wxRIGHT | wxEXPAND, 20 );

		animation_ctrlprop = new wxButton(scrollPanel, ID_ANIM_PROPS, wxT("Animation Data") );
		animation_ctrlprop->SetMinSize( wxSize( btns_width, wxDefaultCoord ));
		animation_ctrlprop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TheScrollPanel::EvtButtonClick), new wxVariant((long) ID_ANIM_PROPS), scrollPanel );
		animation_ctrlprop->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
					NULL, this );
		sizer_topCentertop->Add( animation_ctrlprop, 1, wxALL | wxEXPAND, 1 );

		butt_ExtFramProp = new wxButton(scrollPanel, ID_EXT_FRAM_PROPS, wxT("Extended Frame Data") );
		butt_ExtFramProp->SetMinSize( wxSize( btns_width, wxDefaultCoord ));
		butt_ExtFramProp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TheScrollPanel::EvtButtonClick), new wxVariant((long) ID_EXT_FRAM_PROPS), scrollPanel);
		butt_ExtFramProp->MoveAfterInTabOrder( animation_ctrlprop );
		sizer_topCentertop->Add( butt_ExtFramProp, 1, wxALL | wxEXPAND, 1 );
		butt_ExtFramProp->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
					NULL, this );


		//**************************
		// FILES PATHS
		//**************************
		sizer_topCenter->AddSpacer(fix_spacer_height);

		wxStaticBoxSizer *sizer_PATHS = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("Paths") );
		wxStaticBox *_sb;
		wxColour _colour;
		sizer_topCenter->Add( sizer_PATHS, 0, wxALL | wxEXPAND, 2 );

			//---------------------
			// IMAGE FILE
			//---------------------
			wxBoxSizer *sizer_img = new wxBoxSizer( wxHORIZONTAL );
			sizer_PATHS->Add( sizer_img, 0 , wxEXPAND );

			wxStaticText *txt_img_file = new wxStaticText( scrollPanel, wxID_ANY, wxT("Img"), wxPoint(wxDefaultCoord,8) );
			sizer_img->Add( txt_img_file, 0, wxALL, 4 );

			txtctrl_imgFile = new wxTextCtrl(scrollPanel, IMG_FILE, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
			txtctrl_imgFile->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_imgFile->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_imgFile->SetInsertionPointEnd();
			txtctrl_imgFile->MoveAfterInTabOrder( butt_ExtFramProp );
			sizer_img->Add( txtctrl_imgFile, 1, wxEXPAND );

			pickerFrame = new wxFilePickerCtrl( scrollPanel, ID_PICKER_FRAME_PATH,
							    dataDirPath.GetFullPath(), wxT("Select the frame image"), wxT("*.*"),
								wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST );
			pickerFrame->Connect(wxEVT_COMMAND_FILEPICKER_CHANGED , wxFileDirPickerEventHandler(TheScrollPanel::EvtImgPickerChg), new wxVariant((long) ID_PICKER_FRAME_PATH), scrollPanel);
			pickerFrame->SetMaxSize( wxSize(30, wxDefaultCoord ));
			pickerFrame->SetInitialSize(wxSize(30, 30 ));
			sizer_img->Add( pickerFrame, 0, wxALL );

			//---------------------
			// SOUND FILE
			//---------------------
			wxBoxSizer *sizer_snd = new wxBoxSizer( wxHORIZONTAL );
			sizer_PATHS->Add( sizer_snd, 0 , wxEXPAND );

			wxStaticText *txt_sound = new wxStaticText( scrollPanel, wxID_ANY, wxT("Snd"), wxPoint(wxDefaultCoord,8) );
			sizer_snd->Add( txt_sound, 0, wxALL, 4 );

			txtctrl_soundFile = new wxTextCtrl(scrollPanel, SND_FILE, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
			__PushCtrlData( wxT("sound"), wxString(), _MY_TEXT, txtctrl_soundFile );
			txtctrl_soundFile->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_soundFile->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_soundFile->SetInsertionPointEnd();
			txtctrl_soundFile->MoveAfterInTabOrder( txtctrl_imgFile );
			sizer_snd->Add( txtctrl_soundFile, 1, wxEXPAND );

			pickerSound = new wxFilePickerCtrl( scrollPanel, ID_PICKER_SOUND_PATH,
							    dataDirPath.GetFullPath(), wxT("Select the sound for this frame"), wxT("*.*"),
						wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST );
			pickerSound->Connect(wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler(TheScrollPanel::EvtSndPickerChg), new wxVariant((long) ID_PICKER_SOUND_PATH), scrollPanel);
			pickerSound->SetMaxSize( wxSize(30, wxDefaultCoord ));
			pickerSound->SetInitialSize(wxSize(30, 30 ));
			sizer_snd->Add( pickerSound, 0, wxALL );



		//**************************
		// DELAY
		//**************************
		sizer_topCenter->AddSpacer(fix_spacer_height);

		wxBoxSizer *sizer_Delay = new wxBoxSizer( wxHORIZONTAL );
		sizer_topCenter->Add( sizer_Delay, 0, wxRIGHT | wxEXPAND, 20 );

		// DELAY
		wxStaticText *txt_delay = new wxStaticText( scrollPanel, wxID_ANY, wxT("Delay"), wxPoint(wxDefaultCoord,8),
			wxDefaultSize, wxALIGN_RIGHT );
		sizer_Delay->Add( txt_delay, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT, 4 );

		txtctrl_delay = new wxTextCtrl(scrollPanel,ID_DELAY,wxString(),wxDefaultPosition,wxSize(60, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_delay->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_delay->MoveAfterInTabOrder( txtctrl_soundFile );
		__PushCtrlData( wxT("delay"), wxString(), _MY_TEXT, txtctrl_delay );
		txtctrl_delay->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		
		sizer_Delay->Add( txtctrl_delay, 0, wxALL | wxALIGN_LEFT, 2 );

		sizer_Delay->AddStretchSpacer();

		//**************************
		// OFFSET
		//**************************
		sizer_topCenter->AddSpacer(fix_spacer_height);

		wxStaticBoxSizer *sizer_Offset = new wxStaticBoxSizer( wxHORIZONTAL, scrollPanel, wxT("OffSet") );
		_sb = sizer_Offset->GetStaticBox();
		_colour = _sb->GetBackgroundColour();
		_sb->SetBackgroundColour(MeltColor(_colour,wxColour(0,120,0)));
		sizer_topCenter->Add( sizer_Offset, 0, wxRIGHT | wxEXPAND, 20 );

		sizer_Offset->AddStretchSpacer();

		wxStaticText *txt_offsetX = new wxStaticText( scrollPanel, wxID_ANY, wxT("X"), wxPoint(wxDefaultCoord,8),
			wxDefaultSize, wxALIGN_RIGHT );
		sizer_Offset->Add( txt_offsetX, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4 );

		txtctrl_offset_X = new wxTextCtrl(scrollPanel,OFFSET_X,wxString(),wxDefaultPosition,wxSize(60, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_offset_X->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_offset_X->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_offset_X->MoveAfterInTabOrder( txtctrl_delay );
		sizer_Offset->Add( txtctrl_offset_X, 0, wxALL, 2 );

		wxStaticText *txt_offsetY = new wxStaticText( scrollPanel, wxID_ANY, wxT("y"), wxPoint(wxDefaultCoord,8),
			wxDefaultSize, wxALIGN_RIGHT );
		sizer_Offset->Add( txt_offsetY, 1, wxALL | wxALIGN_CENTER_VERTICAL, 4 );

		txtctrl_offset_Y = new wxTextCtrl(scrollPanel,OFFSET_Y,wxString(),wxDefaultPosition, wxSize(60, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_offset_Y->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_offset_Y->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_offset_Y->MoveAfterInTabOrder( txtctrl_offset_X );
		sizer_Offset->Add( txtctrl_offset_Y, 0, wxALL, 2 );

		wxBitmapButton *clone_offset_butt = new wxBitmapButton( scrollPanel, wxID_ANY,
			wxBitmap( wxImage( GetRessourceFile_String(wxT("copy-small.png")))));
		clone_offset_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Anims::DoDeleteOffset), NULL, this);
		sizer_Offset->Add( clone_offset_butt, 0, wxALL, 2 );


		sizer_Offset->AddStretchSpacer();

		//**************************
		// FRAMES MOVES
		//**************************
		sizer_topCenter->AddSpacer(fix_spacer_height);

		wxStaticBoxSizer *sizer_FrameMoves = new wxStaticBoxSizer( wxHORIZONTAL, scrollPanel, wxT("Moves Axies") );
		sizer_topCenter->Add( sizer_FrameMoves, 0, wxRIGHT | wxEXPAND, 20 );

		wxStaticText *txt_moveX = new wxStaticText( scrollPanel, wxID_ANY, wxT("X"), wxPoint(wxDefaultCoord,8),
			wxDefaultSize, wxALIGN_RIGHT );
		sizer_FrameMoves->Add( txt_moveX, 0, wxALL | wxALIGN_CENTER_VERTICAL, 4 );

		txtctrl_move_X = new wxTextCtrl(scrollPanel,MOVE_X,wxString(),wxDefaultPosition,wxSize(25, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_move_X->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_move_X->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_move_X->MoveAfterInTabOrder( txtctrl_offset_Y );
		__PushCtrlData( wxT("move"), wxT("0"), _MY_TEXT, txtctrl_move_X );
		sizer_FrameMoves->Add( txtctrl_move_X, 1, wxALL, 2 );

		wxStaticText *txt_moveA = new wxStaticText( scrollPanel, wxID_ANY, wxT("A"), wxPoint(wxDefaultCoord,8),
			wxDefaultSize, wxALIGN_RIGHT );
		sizer_FrameMoves->Add( txt_moveA, 1, wxALL | wxALIGN_CENTER_VERTICAL, 4 );

		txtctrl_move_A = new wxTextCtrl(scrollPanel,MOVE_A,wxString(),wxDefaultPosition, wxSize(25, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_move_A->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_move_A->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_move_A->MoveAfterInTabOrder( txtctrl_move_X );
		__PushCtrlData( wxT("movea"), wxT("0"), _MY_TEXT, txtctrl_move_A );
		sizer_FrameMoves->Add( txtctrl_move_A, 1, wxALL, 2 );

		wxStaticText *txt_moveZ = new wxStaticText( scrollPanel, wxID_ANY, wxT("Z"), wxPoint(wxDefaultCoord,8),
			wxDefaultSize, wxALIGN_RIGHT );
		sizer_FrameMoves->Add( txt_moveZ, 1, wxALL | wxALIGN_CENTER_VERTICAL, 4 );

		txtctrl_move_Z = new wxTextCtrl(scrollPanel,MOVE_Z,wxString(),wxDefaultPosition, wxSize(25, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_move_Z->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_move_Z->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_move_Z->MoveAfterInTabOrder( txtctrl_move_A );
		__PushCtrlData( wxT("movez"), wxT("0"), _MY_TEXT, txtctrl_move_Z );
		sizer_FrameMoves->Add( txtctrl_move_Z, 1, wxALL, 2 );




		//**************************
		// BEAT BOXE
		//**************************
		sizer_topCenter->AddSpacer(fix_spacer_height);

		wxStaticBoxSizer *sizer_bbox = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("BeatBox") );
		sizer_topCenter->Add( sizer_bbox, 0, wxRIGHT | wxEXPAND, 20 );

		wxBoxSizer *sizer_temp_bb = new wxBoxSizer( wxHORIZONTAL );
		sizer_bbox->Add( sizer_temp_bb, 0, wxALL, 2 );

		txtctrl_bbox_X = new wxTextCtrl(scrollPanel, BBOX_X, wxString(),wxDefaultPosition, wxSize(40, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_bbox_X->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_bbox_X->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_bbox_X->MoveAfterInTabOrder( txtctrl_move_Z );
		sizer_temp_bb->Add( txtctrl_bbox_X, 0, wxALL, 2 );

		txtctrl_bbox_Y = new wxTextCtrl(scrollPanel,BBOX_Y,wxString(),wxDefaultPosition, wxSize(40, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_bbox_Y->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_bbox_Y->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_bbox_Y->MoveAfterInTabOrder( txtctrl_bbox_X );
		sizer_temp_bb->Add( txtctrl_bbox_Y, 0, wxALL, 2 );

		txtctrl_bbox_W = new wxTextCtrl(scrollPanel,BBOX_W,wxString(),wxDefaultPosition, wxSize(40, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_bbox_W->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_bbox_W->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_bbox_W->MoveAfterInTabOrder( txtctrl_bbox_Y );
		sizer_temp_bb->Add( txtctrl_bbox_W, 0, wxALL, 2 );

		txtctrl_bbox_H = new wxTextCtrl(scrollPanel,BBOX_H,wxString(),wxDefaultPosition, wxSize(40, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_bbox_H->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_bbox_H->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_bbox_H->MoveAfterInTabOrder( txtctrl_bbox_W );
		sizer_temp_bb->Add( txtctrl_bbox_H, 0, wxALL, 2 );

		txtctrl_bbox_Z = new wxTextCtrl(scrollPanel,BBOX_Z,wxString(),wxDefaultPosition, wxSize(40, wxDefaultCoord), wxTE_PROCESS_ENTER );
		txtctrl_bbox_Z->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
		txtctrl_bbox_Z->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
		txtctrl_bbox_Z->MoveAfterInTabOrder( txtctrl_bbox_H );
		sizer_temp_bb->Add( txtctrl_bbox_Z, 0, wxALL, 2 );

		sizer_temp_bb = new wxBoxSizer( wxHORIZONTAL );
		sizer_bbox->Add( sizer_temp_bb, 0, wxALL, 2 );

		del_bbox_butt = new wxBitmapButton( scrollPanel, ID_DEL_BBOX,
						    wxBitmap( wxImage( GetRessourceFile_String(wxT("delete-small.png")))));
		del_bbox_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TheScrollPanel::EvtButtonClick), new wxVariant((long) ID_DEL_BBOX), scrollPanel);
		del_bbox_butt->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
					NULL, this );
		sizer_temp_bb->Add( del_bbox_butt, 0, wxALL, 2 );

		wxBitmapButton *clone_bbox_butt = new wxBitmapButton( scrollPanel, wxID_ANY,
								      wxBitmap( wxImage( GetRessourceFile_String(wxT("copy-small.png")))));
		clone_bbox_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Anims::DoDeleteBBox), NULL, this);
		clone_bbox_butt->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
					NULL, this );
		sizer_temp_bb->Add( clone_bbox_butt, 0, wxALL, 2 );

		bbox_color = new MyPaletteElementCtrl(scrollPanel);
		bbox_color->SetId( ID_BBOX_COLOR );
		bbox_color->SetRGB( 0,0,0 );
		bbox_color->SetMode( AUTO_ELTCOLOR );
		bbox_color->Connect(wxEVT_SET_FOCUS, wxCommandEventHandler(Panel_withAnims::ThrowFocus), NULL, this);
		bbox_color->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
					NULL, this );
		sizer_temp_bb->Add( bbox_color, 0, wxCENTER | wxALL, 2 );

		//**************************
		// ATTACK BOXES
		//**************************
		sizer_topCenter->AddSpacer(fix_spacer_height);

		wxStaticBoxSizer *sizer_attacks = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("Attacks") );
		_sb = sizer_attacks->GetStaticBox();
		_colour = _sb->GetBackgroundColour();
		_sb->SetBackgroundColour(MeltColor(_colour,wxColour(120,0,0)));
		sizer_topCenter->Add( sizer_attacks, 0, wxRIGHT | wxEXPAND, 20 );

			//---------------------
			// LINE : choice attacks
			//---------------------
			wxBoxSizer *sizer_attacks_choice = new wxBoxSizer( wxHORIZONTAL );
			sizer_attacks->Add( sizer_attacks_choice, 0, wxALL );

			wxString lst_choices[] = { wxT("attack"), wxT("blast"), wxT("shock"),
				wxT("burn"), wxT("freeze"), wxT("steal") };
			choiceBox_attacks = new wxChoice( scrollPanel, ATTACK_CHOICE, wxDefaultPosition, wxDefaultSize
							,	t_size_of(lst_choices), lst_choices);
			choiceBox_attacks->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(Panel_Anims::Evt_Attack_BaseName_Change), NULL, this);
			choiceBox_attacks->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			choiceBox_attacks->MoveAfterInTabOrder( txtctrl_bbox_H );
			sizer_attacks_choice->Add( choiceBox_attacks, 0, wxCENTER |wxALL, 4  );

			txt_attacks = new wxStaticText( scrollPanel, wxID_ANY, wxT("num:") );
			sizer_attacks_choice->Add( txt_attacks, 0, wxCENTER | wxALL, 4 );

			txtctrl_att_num	= new wxTextCtrl( scrollPanel, ID_ATT_NUM, wxString()
							,wxDefaultPosition, wxSize(40, wxDefaultCoord) );
			txtctrl_att_num->Connect( wxEVT_COMMAND_TEXT_UPDATED,
					wxCommandEventHandler(Panel_Anims::Evt_Attack_NumChange),
					NULL, this );
			txtctrl_att_num->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			sizer_attacks_choice->Add( txtctrl_att_num, 0, wxCENTER |wxALL, 4  );

			
			del_attbox_butt = new wxBitmapButton( scrollPanel, ID_DEL_ATTBOX,
							      wxBitmap( wxImage( GetRessourceFile_String(wxT("delete-small.png")))));
			del_attbox_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Anims::ATTACK__Set_Zero), NULL, this);
			del_attbox_butt->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			sizer_attacks_choice->Add( del_attbox_butt, 0, wxCENTER |wxALL, 2 );

			wxBitmapButton *clone_attbox_butt = new wxBitmapButton( scrollPanel, wxID_ANY,
					wxBitmap( wxImage( GetRessourceFile_String(wxT("copy-small.png")))));
			clone_attbox_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Anims::ATTACK__Set_Clone), NULL, this);
			clone_attbox_butt->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			sizer_attacks_choice->Add( clone_attbox_butt, 0, wxCENTER |wxALL, 2 );

			att_color = new MyPaletteElementCtrl(scrollPanel);
			att_color->SetId( ID_ATTACK_COLOR );
			att_color->SetRGB( 0,255,255 );
			att_color->SetMode( AUTO_ELTCOLOR );
			att_color->Connect(wxEVT_SET_FOCUS, wxCommandEventHandler(Panel_withAnims::ThrowFocus), NULL, this);
			att_color->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			sizer_attacks_choice->Add( att_color, 0, wxCENTER | wxALL, 2 );

			//---------------------
			// LINE : ATTACKS-BOX
			//---------------------
			wxBoxSizer *sizer_attacks_box = new wxBoxSizer( wxHORIZONTAL );
			sizer_attacks->Add( sizer_attacks_box, 0, wxALL );

			wxStaticText *txt_attbox = new wxStaticText( scrollPanel, wxID_ANY, wxT("Att-Box"), wxPoint(wxDefaultCoord,8) );
			sizer_attacks_box->Add( txt_attbox, 0, wxALL, 4 );

			txtctrl_attbox_X = new wxTextCtrl( scrollPanel, ATTBOX_X, wxString()
							,wxDefaultPosition, wxSize(40, wxDefaultCoord) );
			ATT_CTRL_CONNECT( txtctrl_attbox_X,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_attbox_X->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
			txtctrl_attbox_X->MoveAfterInTabOrder( choiceBox_attacks );
			sizer_attacks_box->Add( txtctrl_attbox_X, 0, wxALL, 2 );

			txtctrl_attbox_Y = new wxTextCtrl( scrollPanel, ATTBOX_Y, wxString()
							,wxDefaultPosition, wxSize(40, wxDefaultCoord) );
			ATT_CTRL_CONNECT( txtctrl_attbox_Y,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_attbox_Y->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
			txtctrl_attbox_Y->MoveAfterInTabOrder( txtctrl_attbox_X );
			sizer_attacks_box->Add( txtctrl_attbox_Y, 0, wxALL, 2 );

			txtctrl_attbox_W = new wxTextCtrl( scrollPanel, ATTBOX_W, wxString()
							,wxDefaultPosition, wxSize(40, wxDefaultCoord) );
			ATT_CTRL_CONNECT( txtctrl_attbox_W,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_attbox_W->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
			txtctrl_attbox_W->MoveAfterInTabOrder( txtctrl_attbox_Y );
			sizer_attacks_box->Add( txtctrl_attbox_W, 0, wxALL, 2 );

			txtctrl_attbox_H = new wxTextCtrl( scrollPanel, ATTBOX_H, wxString()
							,wxDefaultPosition, wxSize(40, wxDefaultCoord) );
			ATT_CTRL_CONNECT( txtctrl_attbox_H,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_attbox_H->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_TxtCtrl_KeyUp),
					NULL, this );
			txtctrl_attbox_H->MoveAfterInTabOrder( txtctrl_attbox_W );
			sizer_attacks_box->Add( txtctrl_attbox_H, 0, wxALL, 2 );

			//---------------------
			// LINE : FLASH BLOCK
			//---------------------
			wxBoxSizer *sizer_attacks_chkbx = new wxBoxSizer( wxHORIZONTAL );
			sizer_attacks->Add( sizer_attacks_chkbx, 0, wxALL | wxEXPAND );

			chckbx_block = new wxCheckBox(scrollPanel,B_BLOK,	wxT("Unblockable"),
					wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
			ATT_CTRL_CONNECT(    chckbx_block,
						   wxEVT_COMMAND_CHECKBOX_CLICKED,
						   Evt_Attack_ValChange )
			chckbx_block->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			chckbx_block->MoveAfterInTabOrder( txtctrl_attbox_H );
			sizer_attacks_chkbx->Add( chckbx_block, 1, wxALL | wxALIGN_CENTRE, 2 );

			chckbx_flash = new wxCheckBox(scrollPanel,B_FLASH,	wxT("No-Flash"),
					wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
			ATT_CTRL_CONNECT(    chckbx_flash,
						   wxEVT_COMMAND_CHECKBOX_CLICKED,
						   Evt_Attack_ValChange )
			chckbx_flash->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			chckbx_flash->MoveAfterInTabOrder( chckbx_block );
			sizer_attacks_chkbx->Add( chckbx_flash, 1, wxALL | wxALIGN_CENTRE, 2 );

			//---------------------
			// GRID : DMG POW PAUSE ZRANG
			//---------------------

			wxGridSizer *sizer_attacks_someProps = new wxGridSizer( 2, 4, 0, 0 );
			sizer_attacks->Add( sizer_attacks_someProps, 0, wxALL );

			wxStaticText *txt_dmgs = new wxStaticText( scrollPanel, wxID_ANY, wxT("Dmgs"), wxPoint(wxDefaultCoord,8) );
			sizer_attacks_someProps->Add( txt_dmgs, 0, wxALL, 4 );

			txtctrl_dmgs = new wxTextCtrl(scrollPanel,DMGS_VAL,wxString(),wxDefaultPosition,wxSize(40, wxDefaultCoord));
			ATT_CTRL_CONNECT(    txtctrl_dmgs,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_dmgs->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_dmgs->MoveAfterInTabOrder( chckbx_flash );
			sizer_attacks_someProps->Add( txtctrl_dmgs, 0, wxALL, 2 );

			txt_pow = new wxStaticText( scrollPanel, wxID_ANY, wxT("KnockD"), wxPoint(wxDefaultCoord,8) );
			sizer_attacks_someProps->Add( txt_pow, 0, wxALL, 4 );

			txtctrl_pow = new wxTextCtrl( scrollPanel, PWD_VAL,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord));
			ATT_CTRL_CONNECT(    txtctrl_pow,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_pow->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_pow->MoveAfterInTabOrder( txtctrl_dmgs );
			sizer_attacks_someProps->Add( txtctrl_pow, 0, wxALL, 2 );

			wxStaticText *txt_Pause = new wxStaticText( scrollPanel, wxID_ANY, wxT("Pause"), wxPoint(wxDefaultCoord,8) );
			sizer_attacks_someProps->Add( txt_Pause, 0, wxALL, 4 );

			txtctrl_pause = new wxTextCtrl( scrollPanel, CTRL_PAUSE_VAL,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord));
			ATT_CTRL_CONNECT(    txtctrl_pause,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_pause->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_pause->MoveAfterInTabOrder( txtctrl_pow );
			sizer_attacks_someProps->Add( txtctrl_pause, 0, wxALL, 2 );

			wxStaticText *txt_zRang = new wxStaticText( scrollPanel, wxID_ANY, wxT("zRang"), wxPoint(wxDefaultCoord,8) );
			sizer_attacks_someProps->Add( txt_zRang, 0, wxALL, 4 );

			txtctrl_zrange = new wxTextCtrl( scrollPanel, ZRANGE,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
			ATT_CTRL_CONNECT(    txtctrl_zrange,
						   wxEVT_COMMAND_TEXT_UPDATED,
						   Evt_Attack_ValChange )
			txtctrl_zrange->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_zrange->MoveAfterInTabOrder( txtctrl_pause );
			sizer_attacks_someProps->Add( txtctrl_zrange, 0, wxALL, 2 );

		//**************************
		// fLIP frame
		//**************************
		sizer_topCenter->AddSpacer(fix_spacer_height);

		chckbx_flipframe = new wxCheckBox(scrollPanel, ID_FLIPFRAME,	wxT("Is the Flipframe")
					, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
		chckbx_flipframe->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(TheScrollPanel::EvtButtonClick), new wxVariant((long) ID_FLIPFRAME), scrollPanel);
		chckbx_flipframe->Connect( wxEVT_CHAR,
					wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
					NULL, this );
		chckbx_flipframe->MoveAfterInTabOrder( txtctrl_zrange );
		sizer_topCenter->Add( chckbx_flipframe, 0, wxRIGHT | wxALIGN_LEFT, 20 );


		//**************************
		// GRID : DRAWMETHDO
		//**************************

			sizer_topCenter->AddSpacer(fix_spacer_height);

			wxStaticBoxSizer *sizer_DrawMethods = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("DrawMethod") );
			_sb = sizer_DrawMethods->GetStaticBox();
			_colour = _sb->GetBackgroundColour();
			_sb->SetBackgroundColour(MeltColor(_colour,wxColour(0,0,120)));
			sizer_topCenter->Add( sizer_DrawMethods, 0, wxRIGHT | wxEXPAND, 20 );

			wxFlexGridSizer *sizer_grid_drawmethod00 = new wxFlexGridSizer( 1, 2, 0, 0 );
			sizer_DrawMethods->Add( sizer_grid_drawmethod00, 0, wxALL );

			chckbx_nodrawmethod = new wxCheckBox(scrollPanel,ID_NODRAWMETHOD,	wxT("No DrawMethod")
						, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
			chckbx_nodrawmethod->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(TheScrollPanel::EvtButtonClick), NULL, scrollPanel);
			chckbx_nodrawmethod->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			chckbx_nodrawmethod->MoveAfterInTabOrder( chckbx_flipframe );
			sizer_grid_drawmethod00->Add( chckbx_nodrawmethod, 0, wxALIGN_LEFT | wxALL, 10 );

			wxBitmapButton *clone_DM_butt = new wxBitmapButton( scrollPanel, wxID_ANY,
									    wxBitmap( wxImage( GetRessourceFile_String(wxT("copy-small.png")))));
			clone_DM_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Anims::DoDeleteDrawmethod), NULL, this);
			sizer_grid_drawmethod00->Add( clone_DM_butt, 0, wxALL, 2 );
			clone_DM_butt->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );


			wxGridSizer *sizer_grid_drawmethod0 = new wxGridSizer( 1, 4, 0, 0 );
			sizer_DrawMethods->Add( sizer_grid_drawmethod0, 0, wxALL );

			wxStaticText *txt_scX = new wxStaticText( scrollPanel, wxID_ANY, wxT("ScaleX"), wxPoint(wxDefaultCoord,8) );
			sizer_grid_drawmethod0->Add( txt_scX, 0, wxALL, 4 );

			txtctrl_scaleX = new wxTextCtrl(scrollPanel, ID_SCALEX,wxString(),wxDefaultPosition,wxSize(40, wxDefaultCoord), wxTE_PROCESS_ENTER);
			txtctrl_scaleX->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_scaleX->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_scaleX->MoveAfterInTabOrder( chckbx_nodrawmethod );
			sizer_grid_drawmethod0->Add( txtctrl_scaleX, 0, wxALL, 2 );

			wxStaticText *txt_xcY = new wxStaticText( scrollPanel, wxID_ANY, wxT("ScaleY"), wxPoint(wxDefaultCoord,8) );
			sizer_grid_drawmethod0->Add( txt_xcY, 0, wxALL, 4 );

			txtctrl_scaleY = new wxTextCtrl( scrollPanel, ID_SCALEY,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
			txtctrl_scaleY->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_scaleY->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_scaleY->MoveAfterInTabOrder( txtctrl_scaleX );
			sizer_grid_drawmethod0->Add( txtctrl_scaleY, 0, wxALL, 2 );


			wxGridSizer *sizer_grid_drawmethod1 = new wxGridSizer( 1, 2, 0, 0 );
			sizer_DrawMethods->Add( sizer_grid_drawmethod1, 0, wxALL );

			chckbx_flipx = new wxCheckBox(scrollPanel,ID_FLIPX,	wxT("FlipX")
						, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
			chckbx_flipx->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			chckbx_flipx->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			chckbx_flipx->MoveAfterInTabOrder( txtctrl_scaleY );
			sizer_grid_drawmethod1->Add( chckbx_flipx, 0, wxALL, 2 );

			chckbx_flipy = new wxCheckBox(scrollPanel,ID_FLIPY,	wxT("FlipY")
						, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
			chckbx_flipy->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			chckbx_flipy->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			chckbx_flipy->MoveAfterInTabOrder( chckbx_flipx );
			sizer_grid_drawmethod1->Add( chckbx_flipy, 0, wxALL, 2 );


			wxGridSizer *sizer_grid_drawmethod2 = new wxGridSizer( 2, 4, 0, 0 );
			sizer_DrawMethods->Add( sizer_grid_drawmethod2, 0, wxALL );

			wxStaticText *txt_shiftX = new wxStaticText( scrollPanel, wxID_ANY, wxT("ShiftX"), wxPoint(wxDefaultCoord,8) );
			sizer_grid_drawmethod2->Add( txt_shiftX, 0, wxALL, 4 );

			txtctrl_shiftX = new wxTextCtrl( scrollPanel, ID_SHIFTX,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
			txtctrl_shiftX->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_shiftX->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_shiftX->MoveAfterInTabOrder( chckbx_flipy );
			sizer_grid_drawmethod2->Add( txtctrl_shiftX, 0, wxALL, 2 );

			wxStaticText *txt_alpha = new wxStaticText( scrollPanel, wxID_ANY, wxT("alpha"), wxPoint(wxDefaultCoord,8) );
			sizer_grid_drawmethod2->Add( txt_alpha, 0, wxALL, 4 );

			txtctrl_alpha = new wxTextCtrl( scrollPanel, ID_ALPHA,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
			txtctrl_alpha->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_alpha->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_alpha->MoveAfterInTabOrder( txtctrl_shiftX );
			sizer_grid_drawmethod2->Add( txtctrl_alpha, 0, wxALL, 2 );



			wxStaticText *txt_remap = new wxStaticText( scrollPanel, wxID_ANY, wxT("Remap"), wxPoint(wxDefaultCoord,8) );
			sizer_grid_drawmethod2->Add( txt_remap, 0, wxALL, 4 );

			txtctrl_remap = new wxTextCtrl( scrollPanel, ID_REMAP,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
			txtctrl_remap->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_remap->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_remap->MoveAfterInTabOrder( txtctrl_alpha );
			sizer_grid_drawmethod2->Add( txtctrl_remap, 0, wxALL, 2 );

			wxStaticText *txt_fillcolor = new wxStaticText( scrollPanel, wxID_ANY, wxT("fillcolor"), wxPoint(wxDefaultCoord,8) );
			sizer_grid_drawmethod2->Add( txt_fillcolor, 0, wxALL, 4 );

			txtctrl_fillcolor = new wxTextCtrl( scrollPanel, ID_FILLCOLOR,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
			txtctrl_fillcolor->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_fillcolor->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_fillcolor->MoveAfterInTabOrder( txtctrl_remap );
			sizer_grid_drawmethod2->Add( txtctrl_fillcolor, 0, wxALL, 2 );


			wxFlexGridSizer *sizer_grid_drawmethod3 = new wxFlexGridSizer( 1, 3, 0, 0 );
			sizer_DrawMethods->Add( sizer_grid_drawmethod3, 0, wxALL );

			wxStaticText *txt_rotate = new wxStaticText( scrollPanel, wxID_ANY, wxT("rotate"), wxPoint(wxDefaultCoord,8) );
			sizer_grid_drawmethod3->Add( txt_rotate, 0, wxALL, 4 );

			txtctrl_rotate = new wxTextCtrl( scrollPanel, ID_ROTATE,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
			txtctrl_rotate->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			txtctrl_rotate->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			txtctrl_rotate->MoveAfterInTabOrder( txtctrl_fillcolor );
			sizer_grid_drawmethod3->Add( txtctrl_rotate, 0, wxALL, 2 );

			chckbx_fliprotate = new wxCheckBox(scrollPanel,ID_FLIPROTATE,	wxT("FlipRotate")
						, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
			chckbx_fliprotate->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(Panel_Anims::EvtCommand), NULL, this);
			chckbx_fliprotate->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
			chckbx_fliprotate->MoveAfterInTabOrder( txtctrl_rotate );
			sizer_grid_drawmethod3->Add( chckbx_fliprotate, 0, wxALL, 2 );


			//**************************
			// JUMP STUFFS
			//**************************

				sizer_topCenter->AddSpacer(fix_spacer_height);

				sizer_Jump = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("Jump stuffs") );
				_sb = sizer_Jump->GetStaticBox();
				_colour = _sb->GetBackgroundColour();
				_sb->SetBackgroundColour(MeltColor(_colour,wxColour(0,120,120)));
				sizer_topCenter->Add( sizer_Jump, 0, wxRIGHT | wxEXPAND, 20 );

				chckbx_jumpframe = new wxCheckBox(scrollPanel,ID_JUMPFRAME,	wxT("Is the JumpFrame")
							, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
				chckbx_jumpframe->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(TheScrollPanel::EvtButtonClick), NULL, scrollPanel);
				chckbx_jumpframe->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
				chckbx_jumpframe->MoveAfterInTabOrder( chckbx_fliprotate );
				sizer_Jump->Add( chckbx_jumpframe, 0, wxALIGN_LEFT | wxALL, 10 );

				{
					wxStaticBoxSizer *sizer_jump_datas = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("Datas") );
					staticBox_jump_datas = sizer_jump_datas->GetStaticBox();
					sizer_Jump->Add( sizer_jump_datas, 0, wxRIGHT | wxEXPAND, 20 );


					wxBoxSizer *sizer_temp = new wxBoxSizer( wxHORIZONTAL );
					sizer_jump_datas->Add( sizer_temp, 0, wxALL, 4 );

					txt_jump_h = new wxStaticText( scrollPanel, wxID_ANY, wxT("height"), wxPoint(wxDefaultCoord,8) );
					sizer_temp->Add( txt_jump_h, 0, wxALL, 4 );

					txtctrl_jump_h = new wxTextCtrl( scrollPanel, ID_JUMPH,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
					txtctrl_jump_h->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(TheScrollPanel::EvtCommand), NULL, scrollPanel);
					txtctrl_jump_h->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
					sizer_temp->Add( txtctrl_jump_h, 0, wxALL, 2 );


					sizer_temp = new wxBoxSizer( wxHORIZONTAL );
					sizer_jump_datas->Add( sizer_temp, 0, wxALL, 4 );

					txt_jump_x = new wxStaticText( scrollPanel, wxID_ANY, wxT("speedx"), wxPoint(wxDefaultCoord,8) );
					sizer_temp->Add( txt_jump_x, 0, wxALL, 4 );

					txtctrl_jump_x = new wxTextCtrl( scrollPanel, ID_JUMPX,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
					txtctrl_jump_x->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(TheScrollPanel::EvtCommand), NULL, this);
					txtctrl_jump_x->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
					sizer_temp->Add( txtctrl_jump_x, 0, wxALL, 2 );


					sizer_temp = new wxBoxSizer( wxHORIZONTAL );
					sizer_jump_datas->Add( sizer_temp, 0, wxALL, 4 );

					txt_jump_z = new wxStaticText( scrollPanel, wxID_ANY, wxT("speedz"), wxPoint(wxDefaultCoord,8) );
					sizer_temp->Add( txt_jump_z, 0, wxALL, 4 );

					txtctrl_jump_z = new wxTextCtrl( scrollPanel, ID_JUMPZ,wxString(),wxDefaultPosition, wxSize(40,wxDefaultCoord), wxTE_PROCESS_ENTER);
					txtctrl_jump_z->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(TheScrollPanel::EvtCommand), NULL, this);
					txtctrl_jump_z->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
					sizer_temp->Add( txtctrl_jump_z, 0, wxALL, 2 );
				}

				chckbx_dropframe = new wxCheckBox(scrollPanel,ID_DROPFRAME,	wxT("Is the DropFrame")
							, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
				chckbx_dropframe->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(TheScrollPanel::EvtButtonClick), NULL, scrollPanel);
				chckbx_dropframe->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
				chckbx_dropframe->MoveAfterInTabOrder( chckbx_dropframe );
				sizer_Jump->Add( chckbx_dropframe, 0, wxALIGN_LEFT | wxALL, 10 );

				chckbx_landframe = new wxCheckBox(scrollPanel,ID_LANDFRAME,	wxT("Is the LandFrame")
							, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTAB_TRAVERSAL);
				chckbx_landframe->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(TheScrollPanel::EvtButtonClick), NULL, scrollPanel);
				chckbx_landframe->Connect( wxEVT_CHAR,
						wxKeyEventHandler(Panel_Anims::Evt_Ctrl_FrameNavig),
						NULL, this );
				chckbx_landframe->MoveAfterInTabOrder( chckbx_dropframe );
				sizer_Jump->Add( chckbx_landframe, 0, wxALIGN_LEFT | wxALL, 10 );


		scrollPanel->SetSizer( sizer_topCenter );
//		scrollPanel->SetMaxSize( sizer_scrollWndFrameProps->GetSize());
//		sizer_scrollWndFrameProps->FitInside(scrollPanel);

		//**************************
		// ANIMATION CONTROL
		//**************************

		sizer_topRightUp->AddStretchSpacer();

		//**************************
		//Animation controls
		wxStaticBoxSizer *sizer_anim_ctrls = new wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Anim Controls") );
		sizer_topRightUp->Add( sizer_anim_ctrls, 0, wxALL, 6 );

		wxGridSizer *sizer_grid_temp = new wxGridSizer( 2, 2, 3, 3 );
		sizer_anim_ctrls->Add( sizer_grid_temp, 0, wxALL );

		butt_Zoom_M = new wxBitmapButton( this, ID_ZOOM_M,
						  wxBitmap( wxImage( GetRessourceFile_String(wxT("zoom-out.png")))));
		sizer_grid_temp->Add( butt_Zoom_M, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

		butt_Zoom_P = new wxBitmapButton( this, ID_ZOOM_P,
						  wxBitmap( wxImage( GetRessourceFile_String(wxT("zoom-in.png")))));
		sizer_grid_temp->Add( butt_Zoom_P, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

		butt_Loop = new wxBitmapButton( this, ID_LOOP_BTN,
						wxBitmap( wxImage( GetRessourceFile_String(wxT("noloop.png")))));
		sizer_grid_temp->Add( butt_Loop, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

		butt_Sound = new wxBitmapButton( this, ID_SOUND_MUTE,
						 wxBitmap( wxImage( GetRessourceFile_String(wxT("sound-on.png")))));
		sizer_grid_temp->Add( butt_Sound, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );


		//**************************
		// Big Frame number
		txt_FrameNumber = new wxStaticText( this, wxID_ANY, wxT("0"), wxDefaultPosition
							, wxDefaultSize, wxALIGN_CENTRE);
		txt_FrameNumber->SetFont(	wxFont(40, wxFONTFAMILY_DEFAULT,
									wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true ));
		txt_FrameNumber->SetForegroundColour( *wxBLUE);
		sizer_topRightUp->Add( txt_FrameNumber, 0, wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 6 );

		//**************************
		// Play/Stop Animation
		butt_Play = new wxBitmapButton( this, ID_PLAY_STOP,
						wxBitmap( wxImage( GetRessourceFile_String(wxT("aktion.png")))));
		sizer_topRightUp->Add( butt_Play, 0, wxTOP|wxALIGN_CENTER_VERTICAL, 10 );


		//**************************
		// Assigns buttons
		wxStaticBoxSizer *sizerStatic_SetBoxes = new wxStaticBoxSizer( wxHORIZONTAL, this, wxT("Assign boxes") );
		sizer_topRightUp->Add( sizerStatic_SetBoxes, 0, wxALL|wxEXPAND, 2 );

		wxFlexGridSizer *sizer_SetBoxes  = new wxFlexGridSizer( 2, 3, 4, 4 );
		sizerStatic_SetBoxes->Add( sizer_SetBoxes, 0, wxALL|wxEXPAND, 6 );

		toBBox = new wxButton(this, ID_To_BBOX, wxT("->BBox"), wxDefaultPosition, wxSize(75,wxDefaultCoord)  );
			toBBox->Disable();
			sizer_SetBoxes->Add( toBBox, 0, wxALL, 2 );

			toAtt = new wxButton(this, ID_To_ATTBOX, wxT("->Att"), wxDefaultPosition, wxSize(75,wxDefaultCoord)  );
			toAtt->Disable();
			sizer_SetBoxes->Add( toAtt, 0, wxALL, 2 );

			wxButton *toClear = new wxButton(this, ID_To_CLEAR, wxT("Clear"), wxDefaultPosition, wxSize(75,wxDefaultCoord)  );;
			sizer_SetBoxes->Add( toClear, 0, wxALL, 2 );

			toOffset = new wxButton(this, ID_To_OFFSET, wxT("->Offs"), wxDefaultPosition, wxSize(75,wxDefaultCoord)  );
			toOffset->Disable();
			sizer_SetBoxes->Add( toOffset, 0, wxALL, 2 );

			toXA = new wxButton(this, ID_To_XA, wxT("->X:A"), wxDefaultPosition, wxSize(75,wxDefaultCoord)  );
			toXA->Disable();
			sizer_SetBoxes->Add( toXA, 0, wxALL, 2 );

		//**************************
		// THE ANIMATION ZONE
		//**************************

		animation_ctrl = new MyAnimationCtrl(this );
		sizer_topRightDown->Add( animation_ctrl, 1, wxALL | wxEXPAND, 2 );

		//**************************
		// LIST OF FRAMES
		//**************************

		wList_frames = new wListFrames_Editable( this );
		sizer_bottom->Add( wList_frames, 1, wxFIXED_MINSIZE );


	//**************************
	// INITIALISATIONS
	//**************************
  	SetSizer( sizer_main );
  	wList_frames->AssociateToListAnims( list_Anims);
  	list_Anims->AssociateToListFrame( wList_frames);
	list_Anims->ReloadLastSessionAnim();
	list_Anims->Refresh_List();
	ResetFrameDatas();
	ATTACK__SetActive(wxT("DEF"));
	Layout();
}


//************************************************************************************

Panel_Anims::~Panel_Anims()
{
	SetEvtHandlerEnabled(false );


	__delete_arr_data_ctrls();
}

//************************************************************************************

void Panel_Anims::OnActivate()
{
	Panel_withAnims::OnActivate();

	if( b_entity_has_change )
	{
		// Reload Frame data
		ATTACK__SetActive(wxT( "DEF" ));
		Update_FrameDatas();
	}
	animation_ctrl->UpdateNavigation();
	animation_ctrl->UpdateFrames();
	wxSizeEvent event;
	scrollPanel->EvtSize( event );
	Layout();
}


//************************************************************************************

enum { MYALL, MYRIGHT, MYLEFT };


//************************************************************************************

void Panel_Anims::EnableNavigation( const int code, bool enable )
{
}


//************************************************************************************

void Panel_Anims::Refresh_FrameData_AnimControl( int refreshtype )
{
	if( refreshtype > 2 )
	{
		Frame_CascadeChanges();
		Update_FrameDatas();
	}

	if( panel_Anims->animation_ctrl != NULL && refreshtype >= 1 )
	{
		animation_ctrl->StopPlaying();
		if( refreshtype >= 2)
			animation_ctrl->UpdateFrames();
	}
}

//************************************************************************************

void Panel_Anims::DoDeleteOffset(wxCommandEvent& event)
{
	if( frameActive == NULL )
		return;

	ob_object *_tt = frameActive->GetSubObject( wxT("offset") );
	if( _tt != NULL )
	{
		_tt->Rm();
//		delete _tt;
		panel_Anims->Refresh_FrameData_AnimControl();
	}
}



//************************************************************************************

void Panel_Anims::DoDeleteBBox(wxCommandEvent& event)
{
	if( frameActive == NULL )
		return;

	ob_object *_tt = frameActive->GetSubObject( wxT("bbox") );
	if( _tt != NULL )
	{
		_tt->Rm();
// 		delete _tt;
		panel_Anims->Refresh_FrameData_AnimControl();
	}
}


//************************************************************************************
// Delete all attacks and Clone from previous frame
void 
Panel_Anims::ATTACK__Set_Clone(wxCommandEvent& event)
{
	if( frameActive == NULL )
		return;

	list<ob_frame_attack*> atts = frameActive->GetAttacks();
	while( atts.empty() == false )
	{
		atts.front()->Rm();
		atts.pop_front();
		panel_Anims->Refresh_FrameData_AnimControl();
	}
	
	ATTACK__OnUpdate(NULL);
}

//--------------------------------------------------------------
void 
Panel_Anims::ATTACK__Set_Zero(wxCommandEvent& event)
{
	if( frameActive != NULL )
	{
		list<ob_frame_attack*> l_atts = frameActive->GetAttacks();
		while( l_atts.empty() == false ) 
		{
			l_atts.front()->Rm();
			l_atts.pop_front();
		}
		
		ob_frame_attack *t = new ob_frame_attack();
		frameActive->Add_SubObj( t );

		t->SetName( wxT("Attack1"));
		t->SetToken( 0, wxT("0") );

		panel_Anims->Refresh_FrameData_AnimControl();
		ATTACK__OnUpdate(NULL);
	}
}

//************************************************************************************

void Panel_Anims::DoDeleteDrawmethod(wxCommandEvent& event)
{
	if( frameActive == NULL )
		return;

	bool _b_change = false;
	ob_object *_tt = frameActive->GetSubObject( wxT("nodrawmethod") );
	if( _tt != NULL )
	{
		_tt->Rm();
// 		delete _tt;
		_b_change = true;
	}

	_tt = frameActive->GetSubObject( wxT("drawmethod") );
	if( _tt != NULL )
	{
		_tt->Rm();
// 		delete _tt;
		_b_change = true;
	}

	if( _b_change )
	{
		panel_Anims->Refresh_FrameData_AnimControl();
	}
}


//************************************************************************************

wxColour cloneColour( 255, 255, 160 );


//***********************************************************************************
#define DFR_TOK_TOSTR( prop, i )			\
	frameActive->prop[i] == NO_CLONED_VAL	\
	? wxString() 							\
	: IntToStr(frameActive->prop[i])
#define DFR_TOK_TOSTR_S( prop )			\
	frameActive->prop == NO_CLONED_VAL	\
	? wxString() 							\
	: IntToStr(frameActive->prop)
#define DFR_TOK_TOBOOL( prop, i )			\
	(frameActive->prop[i] != NO_CLONED_VAL	\
	&& frameActive->prop[i] != 0)
#define DFR_TOK_TOBOOL_S( prop, i )			\
	(frameActive->prop != NO_CLONED_VAL		\
	&& frameActive->prop != 0)
	
void Panel_Anims::Update_FrameDatas()
{
	if( curr_anim == NULL )
	{
		animation_ctrlprop->Disable();
	}

	// No frame object avalaible
	if( curr_anim == NULL || frameActive == NULL )
	{
		EnableFrameDatas(false);
		if( animation_ctrl != NULL )
			animation_ctrl->StopPlaying();
		return;
	}

	animation_ctrlprop->Enable();
	EnableFrameDatas(true);

	//------------------------
	// IMG
	txtctrl_imgFile->ChangeValue(frameActive->GetToken(0));
	wxFileName __path = GetObFile( frameActive->GetToken(0) );
	if( __path.FileExists() )
		pickerFrame->SetPath( __path.GetFullPath() );

	//------------------------
	// SOUND
	txtctrl_soundFile->ChangeValue(frameActive->GetSubObject_Token(wxT("sound"), 0));
	__path = GetObFile( frameActive->GetSubObject_Token(wxT("sound"), 0) );
	if( __path.FileExists() )
		pickerSound->SetPath( __path.GetFullPath() );

	//------------------------
	// DELAY
	txtctrl_delay->ChangeValue(DFR_TOK_TOSTR_S(delay));
	if( frameActive->b_delay_cloned == true )
		txtctrl_delay->SetBackgroundColour( cloneColour );
	else
		txtctrl_delay->SetBackgroundColour( *wxWHITE  );

	//------------------------
	// OFFSET
	txtctrl_offset_X->ChangeValue(DFR_TOK_TOSTR(offset,0));
	txtctrl_offset_Y->ChangeValue(DFR_TOK_TOSTR(offset,1));
	if( frameActive->b_offset_cloned == true )
	{
		txtctrl_offset_X->SetBackgroundColour( cloneColour );
		txtctrl_offset_Y->SetBackgroundColour( cloneColour );
	}
	else
	{
		txtctrl_offset_X->SetBackgroundColour( *wxWHITE );
		txtctrl_offset_Y->SetBackgroundColour( *wxWHITE );
	}

	//------------------------
	// MOVEX
	txtctrl_move_X->ChangeValue(DFR_TOK_TOSTR_S(move));
	if( frameActive->b_move_cloned == true )
		txtctrl_move_X->SetBackgroundColour( cloneColour );
	else
		txtctrl_move_X->SetBackgroundColour( *wxWHITE );

	//------------------------
	// MOVEA
	txtctrl_move_A->ChangeValue(DFR_TOK_TOSTR_S(moveA));
	if( frameActive->b_moveA_cloned == true )
		txtctrl_move_A->SetBackgroundColour( cloneColour );
	else
		txtctrl_move_A->SetBackgroundColour( *wxWHITE );

	//------------------------
	// MOVEZ
	txtctrl_move_Z->ChangeValue(DFR_TOK_TOSTR_S(moveZ));
	if( frameActive->b_moveZ_cloned == true )
		txtctrl_move_Z->SetBackgroundColour( cloneColour );
	else
		txtctrl_move_Z->SetBackgroundColour( *wxWHITE );

	//*********************
	// BBOX
	txtctrl_bbox_X->ChangeValue(DFR_TOK_TOSTR(bBox,0));
	txtctrl_bbox_Y->ChangeValue(DFR_TOK_TOSTR(bBox,1));
	txtctrl_bbox_W->ChangeValue(DFR_TOK_TOSTR(bBox,2));
	txtctrl_bbox_H->ChangeValue(DFR_TOK_TOSTR(bBox,3));

	wxColour tColour = *wxWHITE;
	if( frameActive->b_bBox_cloned )
		tColour = cloneColour;
	txtctrl_bbox_X->SetBackgroundColour( tColour );
	txtctrl_bbox_Y->SetBackgroundColour( tColour );
	txtctrl_bbox_W->SetBackgroundColour( tColour );
	txtctrl_bbox_H->SetBackgroundColour( tColour );


	//*********************
	// ATTACKS
	ATTACK__SetActive(wxString());

	//*********************
	// FLIPFRAME
	chckbx_flipframe->SetValue( frameActive->flipState == 2 );
	if( frameActive->flipState == 1 )
		chckbx_flipframe->SetBackgroundColour( cloneColour );
	else
	{
#ifdef OSLINUX
		chckbx_flipframe->SetBackgroundColour( *wxWHITE );
#endif
#ifdef OSWINDOW
		chckbx_flipframe->SetBackgroundColour( default_BG_colour );
#endif
	}

	//*********************
	// DRAWMETHOD
	int _i = 0;
	txtctrl_scaleX->ChangeValue(DFR_TOK_TOSTR(drawMethod,_i));
	_i++;
	txtctrl_scaleY->ChangeValue(DFR_TOK_TOSTR(drawMethod,_i));
	_i++;
	chckbx_flipx->SetValue(DFR_TOK_TOBOOL(drawMethod,_i) );
	_i++;
	chckbx_flipy->SetValue(DFR_TOK_TOBOOL(drawMethod,_i));
	_i++;
	txtctrl_shiftX->ChangeValue(DFR_TOK_TOSTR(drawMethod,_i));
	_i++;
	txtctrl_alpha->ChangeValue(DFR_TOK_TOSTR(drawMethod,_i));
	_i++;
	txtctrl_remap->ChangeValue(DFR_TOK_TOSTR(drawMethod,_i));
	_i++;
	txtctrl_fillcolor->ChangeValue(DFR_TOK_TOSTR(drawMethod,_i));
	_i++;
	txtctrl_rotate->ChangeValue(DFR_TOK_TOSTR(drawMethod,_i));
	_i++;
	chckbx_fliprotate->SetValue(DFR_TOK_TOBOOL(drawMethod,_i));
	_i++;

	tColour = *wxWHITE;
	if( frameActive->noDrawMethod )
	{
		chckbx_nodrawmethod->SetValue( true );
		if( frameActive->b_noDrawMethod_cloned )
			chckbx_nodrawmethod->SetBackgroundColour( cloneColour );
		else
			// Set nodrawmethod to no state colors
#ifdef OSLINUX
			chckbx_nodrawmethod->SetBackgroundColour( *wxWHITE );
#endif
#ifdef OSWINDOW
			chckbx_nodrawmethod->SetBackgroundColour( default_BG_colour );
#endif

		tColour = wxColour( 200,200,200);
		EnableDrawMethod( false );
	}
	else
	{
		// Set nodrawmethod to no state colors
		chckbx_nodrawmethod->SetValue( false );
#ifdef OSLINUX
		chckbx_nodrawmethod->SetBackgroundColour( *wxWHITE );
#endif
#ifdef OSWINDOW
		chckbx_nodrawmethod->SetBackgroundColour( default_BG_colour );
#endif

		EnableDrawMethod( true );
		if( frameActive->b_noDrawMethod_cloned )
		{
			tColour = cloneColour;
		}
		else
		{
#ifdef OSLINUX
			chckbx_nodrawmethod->SetBackgroundColour( *wxWHITE );
#endif
#ifdef OSWINDOW
			chckbx_nodrawmethod->SetBackgroundColour( default_BG_colour );
#endif
		}
	}

	txtctrl_scaleX->SetBackgroundColour( tColour );
	txtctrl_scaleY->SetBackgroundColour( tColour );
#ifdef OSLINUX
	chckbx_flipx->SetBackgroundColour( tColour );
	chckbx_flipy->SetBackgroundColour( tColour );
#endif
	txtctrl_shiftX->SetBackgroundColour( tColour );
	txtctrl_alpha->SetBackgroundColour( tColour );
	txtctrl_remap->SetBackgroundColour( tColour );
	txtctrl_fillcolor->SetBackgroundColour( tColour );
	txtctrl_rotate->SetBackgroundColour( tColour );
#ifdef OSLINUX
	chckbx_fliprotate->SetBackgroundColour( tColour );
#endif

	//*********************
	// JUMPFRAME
	int _jumpState = frameActive->jumpState;
	chckbx_jumpframe->SetValue( _jumpState == 2 );
	if( _jumpState == 1 )
		chckbx_jumpframe->SetBackgroundColour( cloneColour );
	else
	{
#ifdef OSLINUX
		chckbx_jumpframe->SetBackgroundColour( *wxWHITE );
#endif
#ifdef OSWINDOW
		chckbx_jumpframe->SetBackgroundColour( default_BG_colour );
#endif

		if( _jumpState == 2 && curr_anim != NULL )
		{
			ob_object *jumpframe_prop = curr_anim->GetSubObject( wxT("jumpframe") );
			if( jumpframe_prop != NULL )
			{
				txtctrl_jump_h->ChangeValue( jumpframe_prop->GetToken( 1 ));
				txtctrl_jump_x->ChangeValue( jumpframe_prop->GetToken( 2 ));
				txtctrl_jump_z->ChangeValue( jumpframe_prop->GetToken( 3 ));
			}
		}
	}
	chckbx_jumpframe->Refresh();

	bool b_show_jumpframedatas = (_jumpState ==2 );
	staticBox_jump_datas->Enable(b_show_jumpframedatas) ;
	txt_jump_h->Enable(b_show_jumpframedatas);
	txtctrl_jump_h->Enable(b_show_jumpframedatas);
	txt_jump_x->Enable(b_show_jumpframedatas);
	txtctrl_jump_x->Enable(b_show_jumpframedatas);
	txt_jump_z->Enable(b_show_jumpframedatas);
	txtctrl_jump_z->Enable(b_show_jumpframedatas);
	sizer_Jump->Layout();

	//*********************
	// DROPFRAME
	int _dropState = frameActive->dropState;
	chckbx_dropframe->SetValue( _dropState == 2 );
	if( _dropState == 1 )
		chckbx_dropframe->SetBackgroundColour( cloneColour );
	else
	{

#ifdef OSLINUX
		chckbx_dropframe->SetBackgroundColour( *wxWHITE );
#endif
#ifdef OSWINDOW
		chckbx_dropframe->SetBackgroundColour( default_BG_colour );
#endif
	}
	chckbx_dropframe->Refresh();


	//*********************
	// LANDFRAME
	int _landState = frameActive->landState;
	chckbx_landframe->SetValue( _landState == 2 );
	if( _landState == 1 )
		chckbx_landframe->SetBackgroundColour( cloneColour );
	else
	{

#ifdef OSLINUX
		chckbx_landframe->SetBackgroundColour( *wxWHITE );
#endif
#ifdef OSWINDOW
		chckbx_landframe->SetBackgroundColour( default_BG_colour );
#endif
	}
	chckbx_landframe->Refresh();


	//*********************
	// update animation zone
	if( animation_ctrl != NULL )
		animation_ctrl->StopPlaying();

	scrollPanel->SetSize(scrollPanel->GetSize());
	scrollPanel->Refresh();
}


//************************************************************************************

void Panel_Anims::EnableDrawMethod( bool b_enable )
{
	txtctrl_scaleX->Enable( b_enable );
	txtctrl_scaleY->Enable( b_enable );
	chckbx_flipx->Enable( b_enable );
	chckbx_flipy->Enable( b_enable );
	txtctrl_shiftX->Enable( b_enable );
	txtctrl_alpha->Enable( b_enable );
	txtctrl_remap->Enable( b_enable );
	txtctrl_fillcolor->Enable( b_enable );
	txtctrl_rotate->Enable( b_enable );
	chckbx_fliprotate->Enable( b_enable );
}

//-------------------------------------------------------------------
wxString 
Panel_Anims::ATTACK_GetName()
{
	int ind = panel_Anims->choiceBox_attacks->GetSelection();
	if( ind == wxNOT_FOUND )
		ind = 0;
	wxString att_name = panel_Anims->choiceBox_attacks->GetStringSelection();

	if( ind == 0 )
		att_name += panel_Anims->txtctrl_att_num->GetValue();
	return att_name;
}

/**
 * Depending upon the att_name given, set :
 *   - the attack combo box
 *   - the attack textCTrl num 
 */

void 
Panel_Anims::ATTACK__SetActive( const wxString& _att_name )
{
	wxString att_name = _att_name;
	m_flag |= NO_PROCESS_ATT_CHANGES;
	
	//----------------------------------------------
	// INVALID FRAME
	if( frameActive == NULL )
	{
		choiceBox_attacks->SetSelection(0);
		txtctrl_att_num->ChangeValue( wxT("1") );
	}

	//----------------------------------------------
	// VALID FRAME
	else
	{
		// Ask the default one
		if( att_name == wxT("DEF") )
		{
			att_name = frameActive->GetFirstAvalaibleAttack_name();
			if( att_name == wxString() )
				att_name = wxT("ATTACK1");
		}
		
		// If there is one => the one, else do nothing
		if( att_name == wxString() )
		{
			att_name = frameActive->GetFirstAvalaibleAttack_name();
		}
		
		// MAJ GUI elements of the attack name
		// simple Attack case
		if(  att_name.Left(6).Upper() == wxT("ATTACK") )
		{
			choiceBox_attacks->SetSelection(0);
			// Update textCtrl of the attack num
			txtctrl_att_num->ChangeValue( att_name.Right(att_name.Len()-6));
		}
		else if( att_name != wxString() )
		{
			wxString _att_name = att_name.Lower();
			wxArrayString _arr_attacks = choiceBox_attacks->GetStrings();
			for( size_t i = 1; i < _arr_attacks.Count(); i++)
				if( _arr_attacks[i].Lower() == att_name )
					choiceBox_attacks->SetSelection(i);
		}
	}
	
	//----------------------------------------------
	// FINALLY
	if( frameActive == NULL )
	{
		EnableFrameAttacksData( false );
		m_flag &= ~NO_PROCESS_ATT_CHANGES;
		return;
	}

	//----------------------------------------------
	// Enable all the stuffs
	EnableFrameAttacksData( true);

	
	// Check if attack num have to be showned
	wxString curr_choice = choiceBox_attacks->GetStringSelection();
	bool b_enable_num = ( curr_choice.Upper() ==wxT( "ATTACK" ));
//	txt_attacks->Show(b_show);
	txtctrl_att_num->Enable(b_enable_num);
	txtctrl_att_num->Refresh();
	
	// special case everywhere
	if( att_name == wxT("blast") )
	{
		txtctrl_pow->ChangeValue(wxString());
		txtctrl_pow->Enable(false);
		txt_pow->SetLabel(wxString());
		txt_pow->Enable(false);
	}
	else
	{
		txtctrl_pow->Enable();
		txt_pow->Enable();
		if( att_name == wxT("freeze" ))
			txt_pow->SetLabel(wxT("Time"));
		else
			txt_pow->SetLabel(wxT("KnockD"));
	}

	if( frameActive != NULL )
	{
		int i =0;
		txtctrl_attbox_X->ChangeValue(DFR_TOK_TOSTR(attBox,i));
		i++;
		txtctrl_attbox_Y->ChangeValue(DFR_TOK_TOSTR(attBox,i));
		i++;
		txtctrl_attbox_W->ChangeValue(DFR_TOK_TOSTR(attBox,i));
		i++;
		txtctrl_attbox_H->ChangeValue(DFR_TOK_TOSTR(attBox,i));
		i++;
		txtctrl_dmgs->ChangeValue(DFR_TOK_TOSTR(attBox,i));
		i++;
		if( att_name != wxT("blast") )
		{
			txtctrl_pow->ChangeValue(DFR_TOK_TOSTR(attBox,i));
			i++;
		}
		chckbx_block->SetValue(DFR_TOK_TOBOOL(attBox,i));
		i++;
		chckbx_flash->SetValue(DFR_TOK_TOBOOL(attBox,i));
		i++;
		txtctrl_pause->ChangeValue(DFR_TOK_TOSTR(attBox,i));
		i++;
		txtctrl_zrange->ChangeValue(DFR_TOK_TOSTR(attBox,i));
		i++;

		wxColour tColour;
		if( frameActive->b_attBox_cloned )
			tColour = cloneColour ;
		else
			tColour = *wxWHITE;
		txtctrl_attbox_X->SetBackgroundColour( tColour );
		txtctrl_attbox_Y->SetBackgroundColour( tColour );
		txtctrl_attbox_W->SetBackgroundColour( tColour );
		txtctrl_attbox_H->SetBackgroundColour( tColour );
		txtctrl_dmgs->SetBackgroundColour( tColour );
		txtctrl_pow->SetBackgroundColour( tColour );
#ifdef OSLINUX
		chckbx_block->SetBackgroundColour( tColour );
		chckbx_flash->SetBackgroundColour( tColour );
#endif
		txtctrl_pause->SetBackgroundColour( tColour );
		txtctrl_zrange->SetBackgroundColour( tColour );

		if( animation_ctrl != NULL )
			animation_ctrl->StopPlaying();
	}

	m_flag &= ~NO_PROCESS_ATT_CHANGES;
}


//************************************************************************************

void Panel_Anims::ResetFrameDatas()
{
	txtctrl_imgFile->ChangeValue(wxString());
	txtctrl_soundFile->ChangeValue(wxString());
	txtctrl_delay->ChangeValue(wxString());
	txtctrl_offset_X->ChangeValue(wxString());
	txtctrl_offset_Y->ChangeValue(wxString());
	txtctrl_move_X->ChangeValue(wxString());
	txtctrl_move_A->ChangeValue(wxString());
	txtctrl_move_Z->ChangeValue(wxString());
	txtctrl_bbox_X->ChangeValue(wxString());
	txtctrl_bbox_Y->ChangeValue(wxString());
	txtctrl_bbox_W->ChangeValue(wxString());
	txtctrl_bbox_H->ChangeValue(wxString());
	ResetAttacksFrameDatas();
	if( animation_ctrl != NULL )
		animation_ctrl->StopPlaying();
}


//************************************************************************************

void Panel_Anims::ResetAttacksFrameDatas()
{
	txtctrl_attbox_X->ChangeValue(wxString());
	txtctrl_attbox_Y->ChangeValue(wxString());
	txtctrl_attbox_W->ChangeValue(wxString());
	txtctrl_attbox_H->ChangeValue(wxString());
	chckbx_block->SetValue(false);
	chckbx_flash->SetValue(false);
	txtctrl_dmgs->ChangeValue(wxString());
	txtctrl_pow->ChangeValue(wxString());
	txtctrl_pause->ChangeValue(wxString());
	txtctrl_zrange->ChangeValue(wxString());
	if( animation_ctrl != NULL )
		animation_ctrl->StopPlaying();
}


//************************************************************************************

void Panel_Anims::EnableFrameDatas(bool b_enable)
{
	scrollPanel->Enable( b_enable );
}


//************************************************************************************

void Panel_Anims::EnableFrameAttacksData( bool b_enable)
{
	choiceBox_attacks->Enable(b_enable);
	txtctrl_att_num->Enable(b_enable);
	del_attbox_butt->Enable(b_enable);
	txtctrl_attbox_X->Enable(b_enable);
	txtctrl_attbox_Y->Enable(b_enable);
	txtctrl_attbox_W->Enable(b_enable);
	txtctrl_attbox_H->Enable(b_enable);
	chckbx_block->Enable(b_enable);
	chckbx_flash->Enable(b_enable);
	txtctrl_dmgs->Enable(b_enable);
	txtctrl_pow->Enable(b_enable);
	txtctrl_pause->Enable(b_enable);
	txtctrl_zrange->Enable(b_enable);
	if( animation_ctrl != NULL )
		animation_ctrl->StopPlaying();

//	toAtt->Enable( b_enable );
}


//************************************************************************************

void Panel_Anims::Refresh()
{
	Panel_withAnims::Refresh();
	Update_FrameDatas();
	SetSize( GetSize() );
	scrollPanel->GetContainingSizer()->Layout();
}


//*********************************************************************

void Panel_Anims::Reload()
{
	Panel_withAnims::Refresh();
	Update_FrameDatas();
}


//*********************************************************************

void Panel_Anims::ReloadGifs()
{
	Panel_withAnims::ReloadGifs();
	animation_ctrl->UpdateFrames();
}


//************************************************************************************

void Panel_Anims::EvtButtonClick(wxCommandEvent& event)
{
	int id_wind;
	wxWindow *winSrc = (wxWindow*) event.GetEventObject();
	if( winSrc != NULL )
	{
		id_wind = winSrc->GetId();
	}
	else
	{
		id_wind = event.GetId();
	}

	bool focus_to_list_anims = true;
	switch( id_wind )
	{
		case ID_ANIM_PROPS:
			panel_Anims->DoEditAnimProps();
			break;

		case ID_EXT_FRAM_PROPS:
			panel_Anims->DoEditFrameProps();
			break;

		case ID_DEL_BBOX:
			if( frameActive != NULL )
			{
				wxArrayString __p_bbox;
				__p_bbox.Add(wxT("0"));
				frameActive->SetProperty( wxT("bbox"), __p_bbox);
				panel_Anims->Frame_CascadeChanges();
				panel_Anims->Update_FrameDatas();
				entity->SetChanged();
			}
			break;

		case ID_ZOOM_M:
			panel_Anims->animation_ctrl->Zoom(0.8);
			break;

		case ID_ZOOM_P:
			panel_Anims->animation_ctrl->Zoom(1.3);
			break;

		case ID_PLAY_STOP:
			if( ! panel_Anims->animation_ctrl->b_playing )
				panel_Anims->animation_ctrl->StartPlaying();
			else
				panel_Anims->animation_ctrl->StopPlaying();
			break;

		case ID_LOOP_BTN:
			if( panel_Anims->animation_ctrl->b_looping )
				panel_Anims->butt_Loop->SetBitmapLabel(wxBitmap( wxImage( GetRessourceFile_String(wxT("noloop.png")))));
			else
				panel_Anims->butt_Loop->SetBitmapLabel(wxBitmap( wxImage( GetRessourceFile_String(wxT("loop.png")))));

			panel_Anims->animation_ctrl->b_looping = ( ! panel_Anims->animation_ctrl->b_looping);
			break;

		case ID_SOUND_MUTE:
			if( panel_Anims->animation_ctrl->b_sound_on )
				panel_Anims->butt_Sound->SetBitmapLabel(wxBitmap( wxImage( GetRessourceFile_String(wxT("sound-off.png")))));
			else
				panel_Anims->butt_Sound->SetBitmapLabel(wxBitmap( wxImage( GetRessourceFile_String(wxT("sound-on.png")))));

			panel_Anims->animation_ctrl->b_sound_on = ( ! panel_Anims->animation_ctrl->b_sound_on);
			break;


		case ID_To_BBOX:
		{
			panel_Anims->txtctrl_bbox_X->ChangeValue( IntToStr( animation_ctrl->TO_offx) );
			panel_Anims->txtctrl_bbox_Y->ChangeValue( IntToStr( animation_ctrl->TO_offy) );
			panel_Anims->txtctrl_bbox_W->ChangeValue( IntToStr( animation_ctrl->TO_w) );
			panel_Anims->txtctrl_bbox_H->SetValue( IntToStr( animation_ctrl->TO_h) );
			panel_Anims->animation_ctrl->ResetMouseDraw();
			break;
		}

		case ID_To_ATTBOX:
		{
			if( frameActive != NULL )
			{
				panel_Anims->txtctrl_attbox_X->ChangeValue( IntToStr( animation_ctrl->TO_offx) );
				panel_Anims->txtctrl_attbox_Y->ChangeValue( IntToStr( animation_ctrl->TO_offy) );
				panel_Anims->txtctrl_attbox_W->ChangeValue( IntToStr( animation_ctrl->TO_w) );
				panel_Anims->txtctrl_attbox_H->ChangeValue( IntToStr( animation_ctrl->TO_h) );
				wxCommandEvent evt;
				panel_Anims->Evt_Attack_ValChange( evt );
				panel_Anims->animation_ctrl->ResetMouseDraw();
			}
			break;
		}

		case ID_To_OFFSET:
		{
			panel_Anims->txtctrl_offset_X->SetValue( IntToStr( animation_ctrl->TO_offx) );
			panel_Anims->txtctrl_offset_Y->SetValue( IntToStr( animation_ctrl->TO_offy) );
			panel_Anims->animation_ctrl->ResetMouseDraw();
			break;
		}
		case ID_To_XA:
		{
			panel_Anims->txtctrl_move_X->SetValue( IntToStr( animation_ctrl->TO_x) );
			panel_Anims->txtctrl_move_A->SetValue( IntToStr( animation_ctrl->TO_a) );
/*			DoCommand( MOVE_X, -1 );
			DoCommand( MOVE_A, 2 );*/
			panel_Anims->animation_ctrl->ResetMouseDraw();
			break;
		}

		case ID_To_CLEAR:
			panel_Anims->animation_ctrl->ResetMouseDraw();
			break;

		case ID_NODRAWMETHOD:
			if( frameActive != NULL )
			{
				bool b_update = false;
				ob_object * _tt = frameActive->GetSubObject( wxT("nodrawmethod") );
				if( chckbx_nodrawmethod->GetValue() == true )
				{
					if( _tt == NULL )
					{
						frameActive->SetProperty( wxT("nodrawmethod"), NULL, 0 );
						_tt = frameActive->GetSubObject( wxT("drawmethod"));
						if( _tt != NULL )
						{
							_tt->Rm();
// 							delete _tt;
						}
						b_update = true;
					}
				}
				else
				{
					if( _tt != NULL )
					{
						_tt->Rm();
// 						delete _tt;
						b_update = true;
					}
				}

				if( b_update )
				{
					panel_Anims->Frame_CascadeChanges();
					Update_FrameDatas();
					entity->SetChanged();
					if( panel_Anims->animation_ctrl != NULL )
					{
						panel_Anims->animation_ctrl->StopPlaying();
						panel_Anims->animation_ctrl->UpdateFrames();
					}
				}
			}
			focus_to_list_anims = false;
			break;

		case ID_FLIPFRAME:
			if( panel_Anims->chckbx_flipframe->GetValue() == true )
				panel_Anims->wList_frames->SetFlipFrame( ind_active_frame );
			else
				panel_Anims->wList_frames->SetFlipFrame( -1 );
			panel_Anims->animation_ctrl->StopPlaying();
			panel_Anims->animation_ctrl->UpdateFrames();
			focus_to_list_anims = false;
			entity->SetChanged();
			break;

		case ID_JUMPFRAME:
			if( panel_Anims->chckbx_jumpframe->GetValue() == true )
				panel_Anims->wList_frames->SetJumpFrame( ind_active_frame );
			else
				panel_Anims->wList_frames->SetJumpFrame( -1 );

			entity->SetChanged();
			Frame_CascadeChanges();
			Update_FrameDatas();
			panel_Anims->animation_ctrl->StopPlaying();
			panel_Anims->animation_ctrl->UpdateFrames();
			focus_to_list_anims = false;
			break;

		case ID_DROPFRAME:
			if( panel_Anims->chckbx_dropframe->GetValue() == true )
				panel_Anims->wList_frames->SetDropFrame( ind_active_frame );
			else
				panel_Anims->wList_frames->SetDropFrame( -1 );
			entity->SetChanged();
			panel_Anims->animation_ctrl->StopPlaying();
			panel_Anims->animation_ctrl->UpdateFrames();
			focus_to_list_anims = false;
			break;

		case ID_LANDFRAME:
			if( panel_Anims->chckbx_landframe->GetValue() == true )
				panel_Anims->wList_frames->SetLandFrame( ind_active_frame );
			else
				panel_Anims->wList_frames->SetLandFrame( -1 );
			panel_Anims->animation_ctrl->StopPlaying();
			panel_Anims->animation_ctrl->UpdateFrames();
			focus_to_list_anims = false;
			entity->SetChanged();
			break;

		default:
			break;

	}
	if( panel_Anims->list_Anims != NULL && focus_to_list_anims )
		panel_Anims->list_Anims->SetFocus();

	event.Skip();
}


//************************************************************************************

void Panel_Anims::SetBigFrameNumber( int _num )
{
	txt_FrameNumber->SetLabel( IntToStr(_num) );
	txt_FrameNumber->GetContainingSizer()->Layout();
}


//************************************************************************************

void Panel_Anims::DoEditAnimProps()
{
	if( curr_anim == NULL )
		return;

	// fire the modal dialog
	GridPropFrame *temp = new GridPropFrame( this, curr_anim,
						 wxT("Animation Properties") );
	wxArrayString filter;
	filter.Add(wxT("frame"));
	filter.Add(wxT("flipframe"));
	filter.Add(wxT("jumpframe"));
	filter.Add(wxT("dropframe"));
	filter.Add(wxT("landframe"));
	temp->SetFilter_Properties( filter );
	
	theHistoryManager.GroupStart( wxT("Edit Anim Props") );
	temp->ShowModal();
	theHistoryManager.GroupEnd();
	
	if( entity->changed == 0 )
		entity->changed = temp->Changed();
	delete temp;
}


//************************************************************************************

void Panel_Anims::DoEditFrameProps()
{
	if( frameActive == NULL )
		return;

	// fire the modal dialog
	GridPropFrame *temp = new GridPropFrame( this, frameActive,
						 wxT("More frame Properties") );

	wxString arr_filter[] = {
		wxT("delay"), wxT("offset"), wxT("bbox"), wxT("attack"), wxT("attack1"),
		wxT("attack2"), wxT("attack3"), wxT("attack4"), wxT("attack5"), wxT("attack6"), 
		wxT("attack7"), wxT("attack8"), wxT("attack9"), wxT("attack10"), wxT("attack11"),
		wxT("attack12"), wxT("attack13"), wxT("attack14"), wxT("attack15"), wxT("attack16"), 
		wxT("attack17"), wxT("attack18"), wxT("attack19"), wxT("blast"), wxT("shock"), 
		wxT("burn"), wxT("freeze"), wxT("steal"), wxT("move"), wxT("movea"),
		wxT("movez"), wxT("sound"), wxT("nodrawmethod"), wxT("drawmethod"), 
		wxT("flipframe"), wxT("platform") };
	wxArrayString filter( t_size_of( arr_filter ), arr_filter );
	temp->SetFilter_Properties( filter );
	
	theHistoryManager.GroupStart( wxT("Edit Anim Props") );
	temp->ShowModal();
	theHistoryManager.GroupEnd();
	
	if( entity->changed == 0 )
		entity->changed = temp->Changed();
	delete temp;
}


//************************************************************************************

void Panel_Anims::EvtCommand( wxCommandEvent& event )
{
	if( itState == 1 ) // Closing
		return;

	if( curr_anim == NULL || frameActive == NULL || __arr_data_ctrls == NULL )
		return;

	int id_wind;
	wxWindow *winSrc = (wxWindow*) event.GetEventObject();
	if( winSrc != NULL )
	{
		id_wind = winSrc->GetId();
	}
	else
	{
		id_wind = event.GetId();
	}

	wxWindow *_ctrlInCause = panel_Anims->FindWindow( id_wind );
	if( _ctrlInCause == NULL )
		return;

	int _ctlID = _ctrlInCause->GetId();

	if( _ctlID == ID_ATTACK_COLOR || _ctlID == ID_BBOX_COLOR )
		animation_ctrl->Refresh();

	else
		DoCommand( _ctlID );
}

//-------------------------------------------------------------
void 
Panel_Anims::ATTACK__OnUpdate(ob_frame_attack* att )
{
	if( att != NULL )
		frameActive->UpdateClonedDatas(frameActive->Get_PreviousFrame());
	else
		panel_Anims->Frame_CascadeChanges();
// 	panel_Anims->ATTACK__SetActive(wxString());
	entity->SetChanged();
	animation_ctrl->UpdateFrames();
}

//---------------------------------------------------------------------
void 
Panel_Anims::Evt_Attack_NumChange( wxCommandEvent& event )
{
	if( frameActive == NULL  || m_flag & NO_PROCESS_ATT_CHANGES )
		return;

	int ind = panel_Anims->choiceBox_attacks->GetSelection();
	if( ind != 0 )
		return;
	
	list<ob_frame_attack*> l_att = frameActive->GetAttacks();
	
	//-----------------------------------------------
	// Only one attack per frame
	while( l_att.size() > 1 )
	{
		l_att.front()->Rm();
		l_att.pop_front();
	}
	
	if( l_att.size() > 0 )
	{
		wxString att_name = 
			panel_Anims->choiceBox_attacks->GetStringSelection()
			+ panel_Anims->txtctrl_att_num->GetValue();
		if( l_att.front()->name.Upper() != att_name.Upper() )
		{
			l_att.front()->SetName(att_name);
			ATTACK__OnUpdate(l_att.front());
		}
	}

}


//****************************
// ATTACK CHOICE
//****************************
void 
Panel_Anims::Evt_Attack_BaseName_Change( wxCommandEvent& event )
{
	if( frameActive == NULL || m_flag & NO_PROCESS_ATT_CHANGES )
		return;
	
	int new_ind = panel_Anims->choiceBox_attacks->GetSelection();
	if( new_ind == wxNOT_FOUND )
		new_ind = 0;

	wxString att_name = ATTACK_GetName();
	list<ob_frame_attack*> l_att = frameActive->GetAttacks();
	if( l_att.size() > 0 )
	{
		if( l_att.front()->name.Upper() != att_name.Upper() )
		{
			l_att.front()->SetName( ATTACK_GetName() );
			ATTACK__OnUpdate(l_att.front());
		}
	}

	wxString visible_att_name = 
		panel_Anims->choiceBox_attacks->GetStringSelection();
		bool b_enable_num_att = (visible_att_name.Upper() == wxT("ATTACK"));
	txtctrl_att_num->Enable( b_enable_num_att );
}


//--------------------------------------------------------------
void 
Panel_Anims::Evt_Attack_ValChange( wxCommandEvent& event )
{
	if( frameActive == NULL  || m_flag & NO_PROCESS_ATT_CHANGES )
		return;
	
	//-----------------------------------------------
	// Only one attack per frame
	list<ob_frame_attack*> l_attacks = frameActive->GetAttacks();
	while( l_attacks.size() > 1 )
	{
		l_attacks.front()->Rm();
		l_attacks.pop_front();
	}
	

	//--------------------
	// Get the attack name
	wxString _prop_name = ATTACK_GetName();

	//--------------------
	// The associate object (possibly NULL)
	ob_frame_attack* associate_ob  =  NULL;
	if( l_attacks.size() > 0 )
		associate_ob = l_attacks.front();

	//--------------------
	// GET the values
	wxString curr_val_X = panel_Anims->txtctrl_attbox_X->GetValue();
	curr_val_X.Trim(false).Trim(true);
	wxString curr_val_Y = panel_Anims->txtctrl_attbox_Y->GetValue();
	curr_val_Y.Trim(false).Trim(true);
	wxString curr_val_W = panel_Anims->txtctrl_attbox_W->GetValue();
	curr_val_W.Trim(false).Trim(true);
	wxString curr_val_H = panel_Anims->txtctrl_attbox_H->GetValue();
	curr_val_H.Trim(false).Trim(true);

	bool b_block   = panel_Anims->chckbx_block->GetValue();
	bool b_noflash = panel_Anims->chckbx_flash->GetValue();

	wxString dmgs_val = panel_Anims->txtctrl_dmgs->GetValue();
	dmgs_val.Trim(false).Trim(true);
	wxString _pow_val = panel_Anims->txtctrl_pow->GetValue();
	_pow_val.Trim(false).Trim(true);
	wxString _pause_val = panel_Anims->txtctrl_pause->GetValue();
	_pause_val.Trim(false).Trim(true);
	wxString _zrang = panel_Anims->txtctrl_zrange->GetValue();
	_zrang.Trim(false).Trim(true);

	//--------------------
	// Evacuate the delete case
	if( curr_val_X == wxString() && curr_val_Y == wxString() && curr_val_W == wxString() && curr_val_H == wxString()
		&& dmgs_val == wxString() && _pow_val == wxString() && _pause_val == wxString() && ! b_block && ! b_noflash
		&& _zrang == wxString() )
	{
		if( associate_ob != NULL )
		{
			associate_ob->Rm();
// 			delete associate_ob;
			associate_ob = NULL;
			ATTACK__OnUpdate(NULL);
			panel_Anims->ATTACK__SetActive(wxString());
		}
	}

	//--------------------
	// Creation or update case
	else
	{
		// flag for the no more attack box case
		bool b_no_more_attack_box = false;
		int last_token_set = 0;

		//--------------------
		// Set some default val in case of empty fields
		//		Note : take care of the "no-more attack box case"
		if(	curr_val_X != wxT("0") || curr_val_Y != wxString() || curr_val_W != wxString() || curr_val_H != wxString()
			||  b_block || b_noflash || dmgs_val != wxString() || _pow_val != wxString() || _pause_val != wxString()
			|| _zrang != wxString() )
		{
			if( curr_val_X == wxString() ) 	{ curr_val_X = wxT("0");}
			if( curr_val_Y == wxString() ) 	{ curr_val_Y = wxT("0");}
			if( curr_val_W == wxString() ) 	{ curr_val_W = wxT("0");}
			if( curr_val_H == wxString() ) 	{ curr_val_H = wxT("0");}

			if( _zrang != wxString() ) 		{ last_token_set = 6;}

			if( _pause_val == wxString() && last_token_set > 0)
			{ 
				_pause_val = wxT("0");
				panel_Anims->txtctrl_pause->ChangeValue( dmgs_val );
			}
			else if( _pause_val != wxString() )
				last_token_set = last_token_set > 0 ? last_token_set : 5;

			if( b_noflash == true )
				last_token_set = last_token_set > 0 ? last_token_set : 4;
			if( b_block == true )
				last_token_set = last_token_set > 0 ? last_token_set : 3;

			if( _prop_name != wxT("blast") )
			{
				if( _pow_val == wxString() && last_token_set > 0)
				{ 
					_pow_val = wxT("0");
					panel_Anims->txtctrl_pow->ChangeValue( dmgs_val );
				}
				else if( _pow_val != wxString() )
					last_token_set = last_token_set > 0 ? last_token_set : 2;
			}

			if( dmgs_val == wxString() && last_token_set > 0)
			{ 
				dmgs_val = wxT("0");
				panel_Anims->txtctrl_dmgs->ChangeValue( dmgs_val );
			}
			else if( dmgs_val != wxString() )
				last_token_set = last_token_set > 0 ? last_token_set : 1;
		}
		else
			b_no_more_attack_box = true;

		// Used for later SetTokens() call
		wxArrayString t;

		//--------------------
		// creation case
		if( associate_ob == NULL )
		{
			t.Add( curr_val_X );
			if( ! b_no_more_attack_box )
			{
				t.Add( curr_val_Y );
				t.Add( curr_val_W );
				t.Add( curr_val_H );
				if( dmgs_val != wxString() )
					t.Add( dmgs_val );
				if( _prop_name != wxT("blast") )
				{
					if( _pow_val != wxString() )
						t.Add( _pow_val );
				}
				if( last_token_set >= 3 )
					t.Add( BoolToStr(b_block) );
				if( last_token_set >= 4 )
					t.Add( BoolToStr(b_noflash) );
				if( _pause_val != wxString() )
					t.Add( _pause_val );
				if( _zrang != wxString() )
					t.Add( _zrang );
			}
			theHistoryManager.Set_State( false );
			associate_ob = new ob_frame_attack();
			associate_ob->SetName( _prop_name );
			for( size_t i = 0; i < t.GetCount(); i++)
				associate_ob->SetToken( i, t[i] );
			theHistoryManager.Set_State( true );
			frameActive->Add_SubObj( associate_ob );
			ATTACK__OnUpdate(associate_ob);
			ATTACK__SetActive(wxString());
		}

		//--------------------
		// Update Case
		else
		{
			int i=0;
			wxString old_val_X  =  associate_ob->GetToken(i);
			i++;
			wxString old_val_Y  =  associate_ob->GetToken(i);
			i++;
			wxString old_val_W  =  associate_ob->GetToken(i);
			i++;
			wxString old_val_H  =  associate_ob->GetToken(i);
			i++;
			wxString old_dmgs_val  =  associate_ob->GetToken(i);
			i++;
			wxString old_pow_val;
			if( _prop_name != wxT("blast") )
			{
				old_pow_val  =  associate_ob->GetToken(i);
				i++;
			}
			else
				old_pow_val  =  wxString();
			bool 	 old_b_block  =  StrToBool( associate_ob->GetToken(i));
			i++;
			bool 	 old_b_noflash  =  StrToBool( associate_ob->GetToken(i));
			i++;
			wxString old_pause_val  =  associate_ob->GetToken(i);
			i++;
			wxString old_zrang  =  associate_ob->GetToken(i);

			// If No change
			if( old_val_X == curr_val_X && old_val_Y == curr_val_Y && old_val_W == curr_val_W && old_val_H == curr_val_H
				&& old_dmgs_val == dmgs_val && old_pow_val == _pow_val && old_b_noflash == b_noflash && old_b_block == b_block
				&& old_pause_val == _pause_val && old_zrang == _zrang )
				return;

			t.Add( curr_val_X );
			if( ! b_no_more_attack_box )
			{
				t.Add( curr_val_Y );
				t.Add( curr_val_W );
				t.Add( curr_val_H );
				if( dmgs_val != wxString() )
					t.Add( dmgs_val );
				if( _pow_val != wxString() 	&& _prop_name != wxT("blast") )
					t.Add( _pow_val );
				if( last_token_set >= 3 )
					t.Add( BoolToStr(b_block) );
				if( last_token_set >= 4 )
					t.Add( BoolToStr(b_noflash) );
				if( _pause_val != wxString() )
					t.Add( _pause_val );
				if( _zrang != wxString() )
					t.Add( _zrang );
			}

//			theHistoryManager.GroupStart("Modif " + _prop_name );
			for( size_t i = 0; i < t.GetCount();i++ )
				associate_ob->SetToken( i, t[i] );
			associate_ob->SetNbTokens(last_token_set+4);
// 			theHistoryManager.GroupEnd();
			ATTACK__OnUpdate(associate_ob);
		}
	}
}


//--------------------------------------------------------------
void 
Panel_Anims::Evt_TxtCtrlCommand( wxCommandEvent& event )
{
	if( frameActive == NULL )
		return;
	wxWindow* w = (wxWindow*)event.GetEventObject();
	DoCommand(w->GetId() );
}

/**
 * Take in account a change in the frame data from the _ctlID
 * If
 * 		updateMode == -1 => No update will be donne
 * 		updateMode ==  0 => The default, let the control know what to update
 * 		updateMode ==  1 => Force to Simply update the frame Datas
 * 		updateMode ==  2 => Force to Update everything
 */
void Panel_Anims::DoCommand( int _ctlID, int force_updateMode )
{
	int update_showedFrame = 0;	// if the change imply to refresh the showed Frame
	bool b_changeMade = false;	// if the change imply to refresh the showed Frame

	wxWindow *_ctrlInCause = panel_Anims->FindWindow( _ctlID );


	//****************************
	// JUMP DATAS
	//****************************
	if( _ctlID == ID_JUMPH || _ctlID == ID_JUMPX || _ctlID == ID_JUMPZ )
	{
		// Refresh current frame data object
		if( curr_anim == NULL || frameActive == NULL || ind_active_frame < 0 )
			return;

		ob_object *jumpframe_prop = curr_anim->GetSubObject( wxT("jumpframe") );
		if( jumpframe_prop == NULL )
			return;

// 		theHistoryManager.GroupStart( "Change Jump Prop" );
		jumpframe_prop->SetToken( 1, txtctrl_jump_h->GetValue() );
		jumpframe_prop->SetToken( 2, txtctrl_jump_x->GetValue() );
		jumpframe_prop->SetToken( 3, txtctrl_jump_z->GetValue() );
// 		theHistoryManager.GroupEnd();

		b_changeMade = true;
		update_showedFrame = 2;
	}

	//****************************
	// FRAME IMAGE
	//****************************
	else if( _ctlID == IMG_FILE )
	{
		wxString curr_path = panel_Anims->txtctrl_imgFile->GetValue();
		if( curr_path == frameActive->GetToken(0) )
			return;

		b_changeMade = true;
		update_showedFrame = 2;

		// Check if the new path exist
		wxFileName _wxpath = GetObFile( curr_path);
		if( ! _wxpath.FileExists())
			ObjectsLog( MYLOG_WARNING, -1, wxT("Frame file <") + _wxpath.GetFullName() + wxT("> doesn't exists") );

// 		theHistoryManager.GroupStart( "Change Frame Image" );
		frameActive->SetToken( 0, curr_path);
// 		theHistoryManager.GroupEnd();
		
		frameActive->RemoveImage();
//		wListFrames::theFrameImgManager.Invalidate(curr_path);
		panel_Anims->wList_frames->RefreshSelection();
	}

	//****************************
	// OFFSET
	//****************************
	else if( _ctlID == OFFSET_X || _ctlID == OFFSET_Y )
	{
		wxString _prop_name = wxT("offset");
		ob_object* associate_ob  =  frameActive->GetSubObject(_prop_name);

		wxString curr_val_X = panel_Anims->txtctrl_offset_X->GetValue();
		curr_val_X.Trim(false).Trim(true);
		wxString curr_val_Y = panel_Anims->txtctrl_offset_Y->GetValue();
		curr_val_Y.Trim(false).Trim(true);

		if( curr_val_X == wxString() )
		{ 
			curr_val_X = wxT("0");
			panel_Anims->txtctrl_offset_X->ChangeValue(wxT("0"));
		}
		if( curr_val_Y == wxString() )
		{ 
			curr_val_Y = wxT("0");
			panel_Anims->txtctrl_offset_Y->ChangeValue(wxT("0"));
		}

		// avoid the NULL prop case
		if( associate_ob != NULL )
		{
			wxString old_val_X  =  associate_ob->GetToken(0);
			wxString old_val_Y  =  associate_ob->GetToken(1);

			// If No change
			if( old_val_X == curr_val_X && old_val_Y == curr_val_Y )
				return;
		}
		b_changeMade = true;
		update_showedFrame = 2;
		wxArrayString t;
		t.Add( curr_val_X );
		t.Add( curr_val_Y );
// 		theHistoryManager.GroupStart( "Change Offset" );
		frameActive->SetProperty( _prop_name,t );
// 		theHistoryManager.GroupEnd();
		panel_Anims->animation_ctrl->UpdateFrames();
	}

	//****************************
	// BEAT BOX
	//****************************
	else if( _ctlID == BBOX_X || _ctlID == BBOX_Y || _ctlID == BBOX_W
			|| _ctlID == BBOX_H || _ctlID == BBOX_Z )
	{
		wxString _prop_name = wxT("bbox");
		ob_object* associate_ob  =  frameActive->GetSubObject(_prop_name);

		wxString curr_val_X = panel_Anims->txtctrl_bbox_X->GetValue();
		curr_val_X.Trim(false).Trim(true);
		wxString curr_val_Y = panel_Anims->txtctrl_bbox_Y->GetValue();
		curr_val_Y.Trim(false).Trim(true);
		wxString curr_val_W = panel_Anims->txtctrl_bbox_W->GetValue();
		curr_val_W.Trim(false).Trim(true);
		wxString curr_val_H = panel_Anims->txtctrl_bbox_H->GetValue();
		curr_val_H.Trim(false).Trim(true);
		wxString curr_val_Z = panel_Anims->txtctrl_bbox_Z->GetValue();
		curr_val_Z.Trim(false).Trim(true);

		//--------------------
		// Evacuate the delete case
		if(	curr_val_X == wxString() && curr_val_Y == wxString() && curr_val_W == wxString()
			&& curr_val_H == wxString()  && curr_val_Z == wxString())
		{
			if( associate_ob != NULL )
			{
				b_changeMade = true;
				update_showedFrame = 1;
				theHistoryManager.GroupStart( wxT("Del BBox") );
				associate_ob->Rm();
				theHistoryManager.GroupEnd();
// 				delete associate_ob;
				associate_ob = NULL;
			}
		}

		//--------------------
		// Creation or update case
		else
		{
			// Set some default val in case of empty fields
			if( curr_val_X == wxString() )
			{ 
				curr_val_X = wxT("0");
				panel_Anims->txtctrl_bbox_X->ChangeValue( curr_val_X );
			}
			if( curr_val_Y == wxString() )
			{ 
				curr_val_Y = wxT("0");
				panel_Anims->txtctrl_bbox_Y->ChangeValue( curr_val_X );
			}
			if( curr_val_W == wxString() )
			{ 
				curr_val_W = wxT("0");
				panel_Anims->txtctrl_bbox_W->ChangeValue( curr_val_X );
			}
			if( curr_val_H == wxString() )
			{ 
				curr_val_H = wxT("0");
				panel_Anims->txtctrl_bbox_H->ChangeValue( curr_val_X );
			}

			// For setTokens call
			wxArrayString t;

			//--------------------
			// Creation case
			if( associate_ob == NULL )
			{
// 				theHistoryManager.GroupStart( "Create BBox" );
				b_changeMade = true;
				update_showedFrame = 1;
				t.Add( curr_val_X );
				t.Add( curr_val_Y );
				t.Add( curr_val_W );
				t.Add( curr_val_H );
				if( curr_val_Z != wxT("0") && curr_val_Z != wxString() )
					t.Add( curr_val_Z );
			}

			//--------------------
			// update case
			else
			{
// 				theHistoryManager.GroupStart( "Change BBox" );
				wxString old_val_X  =  associate_ob->GetToken(0);
				wxString old_val_Y  =  associate_ob->GetToken(1);
				wxString old_val_W  =  associate_ob->GetToken(2);
				wxString old_val_H  =  associate_ob->GetToken(3);
				wxString old_val_Z  =  associate_ob->GetToken(4);

				// If No change
				if( old_val_X == curr_val_X && old_val_Y == curr_val_Y
					 && old_val_W == curr_val_W && old_val_H == curr_val_H
					 && old_val_Z == curr_val_Z )
					return;

				// Set that things are changed
				b_changeMade = true;
				update_showedFrame = 1;

				// Update object
				t.Add( curr_val_X );
				t.Add( curr_val_Y );
				t.Add( curr_val_W );
				t.Add( curr_val_H );
				if( curr_val_Z != wxT("0") )
					t.Add( curr_val_Z );
			}
			
			ob_object* subobj = frameActive->SetProperty( _prop_name,t );
			subobj->SetNbTokens( t.GetCount() );
			
// 			theHistoryManager.GroupEnd();
		}
	}




	//****************************
	// DRAW METHOD
	//****************************
	else if( 	_ctlID == ID_SCALEX || _ctlID == ID_SCALEY || _ctlID == ID_FLIPX || _ctlID == ID_FLIPY
			|| 	_ctlID == ID_SHIFTX || _ctlID == ID_ALPHA || _ctlID == ID_REMAP || _ctlID == ID_FILLCOLOR
			|| 	_ctlID == ID_ROTATE || _ctlID == ID_FLIPROTATE )
	{

		//--------------------
		// Get the attack name
		wxString _prop_name = wxT("drawmethod");

		//--------------------
		// The associate object (possibly NULL)
		ob_object* associate_ob  =  frameActive->GetSubObject(_prop_name);

		//--------------------
		// GET the values
		wxString scaleX_val = panel_Anims->txtctrl_scaleX->GetValue();
		scaleX_val.Trim(false).Trim(true);
		if( scaleX_val == wxString() ) scaleX_val = wxT("256");
		wxString scaleY_val = panel_Anims->txtctrl_scaleY->GetValue();
		scaleY_val.Trim(false).Trim(true);
		if( scaleY_val == wxString() ) scaleY_val = wxT("256");

		bool b_flipx = panel_Anims->chckbx_flipx->GetValue();
		bool b_flipy = panel_Anims->chckbx_flipy->GetValue();

		wxString shiftx_val = panel_Anims->txtctrl_shiftX->GetValue();
		shiftx_val.Trim(false).Trim(true);
		wxString alpha_val = panel_Anims->txtctrl_alpha->GetValue();
		alpha_val.Trim(false).Trim(true);
		if( alpha_val == wxString() ) alpha_val = wxT("-1");
		wxString remap_val = panel_Anims->txtctrl_remap->GetValue();
		remap_val.Trim(false).Trim(true);
		if( remap_val == wxString() ) remap_val = wxT("-1");
		wxString fillcolor_val = panel_Anims->txtctrl_fillcolor->GetValue();
		fillcolor_val.Trim(false).Trim(true);
		if( fillcolor_val == wxString() ) fillcolor_val = wxT("0");
		wxString rotate_val = panel_Anims->txtctrl_rotate->GetValue();
		rotate_val.Trim(false).Trim(true);
		if( rotate_val == wxString() ) rotate_val = wxT("0");

		bool b_fliprotate = panel_Anims->chckbx_fliprotate->GetValue();

		//--------------------
		// Check the values
		int _scaleX = StrToInt( scaleX_val );
		int _scaleY = StrToInt( scaleY_val );
		int _shiftx = StrToInt( shiftx_val );
		int _alpha = StrToInt( alpha_val );
		if( _alpha < -1 || _alpha > 6 )
		{
			wxMessageBox( wxT("Alpha val must be between -1 and 6"), wxT("Warning"), wxOK | wxICON_INFORMATION, this );
		}
		int _remap = StrToInt( remap_val );
		int _fillcolor = StrToInt( fillcolor_val );
		if( _fillcolor < 0 || _fillcolor > 255 )
		{
			wxMessageBox( wxT("Fillcolor val must be between 0 and 255"), wxT("Warning"), wxOK | wxICON_INFORMATION, this );
		}
		int _rotate = StrToInt( rotate_val );


		//--------------------
		// Evacuate the delete case
		if(    _scaleX == 256 && _scaleY == 256 && b_flipx == false && b_flipy == false
			&& _shiftx == 0 && _alpha == -1 && _remap == 0 && _fillcolor == 0 && _rotate == 0 && b_fliprotate == false )
		{
			if( associate_ob != NULL )
			{
				b_changeMade = true;
				update_showedFrame = 2;
				theHistoryManager.GroupStart( wxT("Del DrawMethod") );
				associate_ob->Rm();
				theHistoryManager.GroupEnd();
// 				delete associate_ob;
				associate_ob = NULL;
			}
			panel_Anims->txtctrl_scaleX->ChangeValue(wxString());
			panel_Anims->txtctrl_scaleY->ChangeValue(wxString());
			panel_Anims->txtctrl_shiftX->ChangeValue(wxString());
			panel_Anims->txtctrl_alpha->ChangeValue(wxString());
			panel_Anims->txtctrl_remap->ChangeValue(wxString());
			panel_Anims->txtctrl_fillcolor->ChangeValue(wxString());
			panel_Anims->txtctrl_rotate->ChangeValue(wxString());
		}

		//--------------------
		// Creation or update case
		else
		{
			// flag for the no more attack box case
//			bool b_no_more_attack_box = false;
			int last_token_set = 0;

			//--------------------
			// Get the last token valid
			if( _scaleX != 256 ) 	{ last_token_set = 0;}
			if( _scaleY != 256 ) 	{ last_token_set = 1;}
			if( b_flipx != false ) 		{ last_token_set = 2;}
			if( b_flipy != false ) 		{ last_token_set = 3;}
			if( _shiftx != 0 )	{ last_token_set = 4;}
			if( _alpha != -1 ) 	{ last_token_set = 5;}
			if( _remap != 0 ) 	{ last_token_set = 6;}
			if( _fillcolor != 0 ) 	{ last_token_set = 7;}
			if( _rotate != 0 ) 	{ last_token_set = 8;}
			if( b_fliprotate != false ) 	{ last_token_set = 9;}
			int nb_tokens = last_token_set + 1;


			// Used for later SetTokens() call
			wxArrayString t;
			t.Add( IntToStr(_scaleX) );
			if( _ctlID != ID_SCALEX )
				panel_Anims->txtctrl_scaleX->ChangeValue(IntToStr(_scaleX));
			last_token_set--;
			if( last_token_set >= 0 )
			{
				wxString val = IntToStr(_scaleY);
				t.Add( val );
				if( _ctlID != ID_SCALEY )
					panel_Anims->txtctrl_scaleY->ChangeValue(val);
				last_token_set--;
			}
			else
				panel_Anims->txtctrl_scaleY->ChangeValue(wxString());
			
			if( last_token_set >= 0 )
			{
				t.Add( BoolToStr(b_flipx) );
				panel_Anims->chckbx_flipx->SetValue(b_flipx);
				last_token_set--;
			}
			else
				panel_Anims->chckbx_flipx->SetValue(false);
			
			if( last_token_set >= 0 )
			{
				t.Add( BoolToStr(b_flipy) );
				panel_Anims->chckbx_flipy->SetValue(b_flipy);
				last_token_set--;
			}
			else
				panel_Anims->chckbx_flipy->SetValue(false);
			
			if( last_token_set >= 0 )
			{
				wxString val = IntToStr(_shiftx);
				t.Add( val );
				if( _ctlID != ID_SHIFTX )
					panel_Anims->txtctrl_shiftX->ChangeValue(val);
				last_token_set--;
			}
			else
				panel_Anims->txtctrl_shiftX->ChangeValue(wxString());
			
			if( last_token_set >= 0 )
			{
				wxString val = IntToStr(_alpha);
				t.Add( val );
				if( _ctlID != ID_ALPHA )
					panel_Anims->txtctrl_alpha->ChangeValue(val);
				last_token_set--;
			}
			else
				panel_Anims->txtctrl_alpha->ChangeValue(wxString());
			
			if( last_token_set >= 0 )
			{
				wxString val = IntToStr(_remap);
				t.Add( val );
				if( _ctlID != ID_REMAP )
					panel_Anims->txtctrl_remap->ChangeValue(val);
				last_token_set--;
			}
			else
				panel_Anims->txtctrl_remap->ChangeValue(wxString());
			
			if( last_token_set >= 0 )
			{
				wxString val = IntToStr(_fillcolor);
				t.Add( val );
				if( _ctlID != ID_FILLCOLOR )
					panel_Anims->txtctrl_fillcolor->ChangeValue(val);
				last_token_set--;
			}
			else
				panel_Anims->txtctrl_fillcolor->ChangeValue(wxString());
			
			if( last_token_set >= 0 )
			{
				wxString val = IntToStr(_rotate);
				t.Add( val );
				if( _ctlID != ID_ROTATE )
					panel_Anims->txtctrl_rotate->ChangeValue(val);
				last_token_set--;
			}
			else
				panel_Anims->txtctrl_rotate->ChangeValue(wxString());
			
			if( last_token_set >= 0 )
			{
				t.Add( BoolToStr(b_fliprotate) );
				panel_Anims->chckbx_fliprotate->SetValue(b_fliprotate);
				last_token_set--;
			}
			else
				panel_Anims->chckbx_fliprotate->SetValue(false);
			
			//--------------------
			// creation case
			if( associate_ob == NULL )
			{
				b_changeMade = true;
				update_showedFrame = 2;
				theHistoryManager.GroupStart( wxT("Create DrawMethod") );
				frameActive->SetProperty( _prop_name,t );
				theHistoryManager.GroupEnd();
			}

			//--------------------
			// Update Case
			else
			{
				int _nb_token = associate_ob->nb_tokens;
				int i=0;
				int old_scaleX = 256;
				if( _nb_token > i )
					old_scaleX  =  StrToInt( associate_ob->GetToken(i));
				i++;
				int old_scaleY = 256;
				if( _nb_token > i )
					old_scaleY  =  StrToInt( associate_ob->GetToken(i));
				i++;
				bool old_b_flipx = false;
				if( _nb_token > i )
					old_b_flipx  =  StrToBool( associate_ob->GetToken(i) );
				i++;
				bool old_b_flipy = false;
				if( _nb_token > i )
					old_b_flipy  =  StrToBool( associate_ob->GetToken(i) );
				i++;
				int old_shiftx = -1;
				if( _nb_token > i )
					old_shiftx  =  StrToInt( associate_ob->GetToken(i));
				i++;
				int old_alpha = -1;
				if( _nb_token > i )
					old_alpha  =  StrToInt( associate_ob->GetToken(i));
				i++;
				int old_remap = 0;
				if( _nb_token > i )
					old_remap  =  StrToInt( associate_ob->GetToken(i));
				i++;
				int old_fillcolor = 0;
				if( _nb_token > i )
					old_fillcolor  =  StrToInt( associate_ob->GetToken(i));
				i++;
				int old_rotate = 0;
				if( _nb_token > i )
					old_rotate  =  StrToInt( associate_ob->GetToken(i));
				i++;
				bool old_b_fliprotate = false;
				if( _nb_token > i )
					old_b_fliprotate  =  StrToBool( associate_ob->GetToken(i));
				i++;

				// If No change
				if( 	old_scaleX == _scaleX && old_scaleY == _scaleY
					&& 	old_b_flipx == b_flipx && old_b_flipy == b_flipy
					&&  old_shiftx == _shiftx
					&&	old_alpha == _alpha && old_remap == _remap
					&& 	old_fillcolor == _fillcolor && old_rotate == _rotate
					&&	old_b_fliprotate == b_fliprotate )
					return;

				b_changeMade = true;
				update_showedFrame = 2;
// 				theHistoryManager.GroupStart( "Change DrawMethod" );
				ob_object* subobj = frameActive->SetProperty( _prop_name,t );
				subobj->SetNbTokens( nb_tokens );
// 				theHistoryManager.GroupEnd();
			}
		}
	}


	//****************************
	//Single token properties
	//****************************
	else
	{
		for( size_t i = 0; i < __arr_data_ctrls__curr_ind; i++)
		{
			wxString *_prop_name = (wxString*) 	__arr_data_ctrls[i][0];
			wxString *_defVal	 = (wxString*) 	__arr_data_ctrls[i][1];
			int _ctrlType  		 = *((int*) 	__arr_data_ctrls[i][2]);
			wxWindow *t_ctrl_ 	 = (wxWindow*) 	__arr_data_ctrls[i][3];

			// if Not this one
			if( t_ctrl_ != _ctrlInCause )
				continue;

			switch( _ctrlType )
			{
				case _MY_CHKBOX:
				{
					wxCheckBox *_ctrl_ = (wxCheckBox*) t_ctrl_;

					// If nothing change
					if( BoolToStr( _ctrl_->GetValue()) ==  frameActive->GetSubObject_Token(*_prop_name) )
						return;

					b_changeMade = true;
					update_showedFrame = 0;
					if( _ctrl_->GetValue() == *_defVal )
					{
						ob_object *temp = frameActive->GetSubObject(*_prop_name);
						if( temp != NULL )
							temp->Rm();
					}
					else
					{
						frameActive->SetProperty( *_prop_name, BoolToStr(_ctrl_->GetValue()) );
					}
					break;
				}
				case _MY_TEXT:
				{
					wxTextCtrl *_ctrl_ 	= (wxTextCtrl*) t_ctrl_;
					wxString curr_val = _ctrl_->GetValue();
					curr_val.Trim(false).Trim(true);

					// Deletion case
					if( curr_val == wxString() )
					{
						ob_object *temp = frameActive->GetSubObject( *_prop_name );
						
						// Nothing to delete
						if( temp == NULL )
							return;
						
						temp->Rm();
					}

					// If nothing change
					else if( curr_val ==  frameActive->GetSubObject_Token(*_prop_name) )
						return;

					else
					{
						frameActive->SetProperty( *_prop_name, curr_val );
					}

					b_changeMade = true;
					update_showedFrame = 1;
					break;
				}
				case _MY_CHOICE:
				default:
					return;
			}

			if( _prop_name->Upper() == wxT("SOUND") )
			{
				wxString curr_path = frameActive->GetSubObject_Token( wxT("sound" ));
				// Check if the new path exist
				wxFileName _wxpath = GetObFile( curr_path );
				if( ! _wxpath.FileExists())
					ObjectsLog( MYLOG_WARNING, -1, wxT("Sound file <") + _wxpath.GetFullName() + wxT("> doesn't exist") );
				update_showedFrame = 0;
			}
			else if( _prop_name->Upper() == wxT("MOVE") || _prop_name->Upper() == wxT("MOVEA")
				|| _prop_name->Upper() == wxT("MOVEZ") || _prop_name->Upper() == wxT("DELAY"))
			{
				update_showedFrame = 2;
			}

			// Found the control -> stop looping
			break;
		}
	}

	// Cascade the changes to frames that follow
	panel_Anims->Frame_CascadeChanges();
	if( b_changeMade )
		entity->SetChanged();

	if( (update_showedFrame == 1  && force_updateMode == 0)
		||
		force_updateMode == 1
		)
	{
		panel_Anims->Refresh_FrameData_AnimControl( 1 );
	}
	else if( (update_showedFrame == 2  && force_updateMode == 0)
		||
		force_updateMode == 2
		)
	{
		panel_Anims->Refresh_FrameData_AnimControl(2);
	}

	return;
}

//---------------------------------------------------------
void 
Panel_Anims::Rescale_Boxes()
{
	if( frameActive == NULL || arr_anims_count <= 0 )
	{
		wxMessageBox( wxT("No animations to rescale !"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	// Ask For the percentage
	wxTextEntryDialog* ctrl_percent = new wxTextEntryDialog( this, wxT("Percentage of the rescaling ?"), wxT("As you want"),wxT("100") );
	ctrl_percent->ShowModal();
	;
	bool is_a_number = false;
	int percent = StrToInt( ctrl_percent->GetValue(), is_a_number );
	delete ctrl_percent;

	if( ! is_a_number )
	{
		wxMessageBox( wxT("Not a valid integer !"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	float ratio = (float) percent / (float) 100;

	// For each animation
	for( int i=0; i < arr_anims_count; i++)
	{
		size_t nb_frames;
		ob_frame **_frames = arr_anims[i]->GetFrames( nb_frames );

		if( _frames == NULL )
			continue;

		for( size_t j=0; j < nb_frames; j++)
		{
			ob_frame *_fr = _frames[j];

			//----------------------
			// Resize the offset
			ob_object *_offset = _fr->GetSubObject(wxT("offset"));
			if( _offset != NULL )
			{
				_offset->SetToken(0, IntToStr( (int) ( StrToInt(_offset->arr_token[0]) * ratio )));
				_offset->SetToken(1, IntToStr( (int) ( StrToInt(_offset->arr_token[1]) * ratio )));
			}

			//----------------------
			// Resize the beat-box
			ob_object *_bbox = _fr->GetSubObject(wxT("bbox"));
			if( _bbox != NULL )
			{
				for( size_t k=0;k< _bbox->nb_tokens; k++)
					_bbox->SetToken(k,IntToStr( (int) ( StrToInt(_bbox->arr_token[k]) * ratio )));
			}

			//----------------------
			// Resize the attack-boxes
			wxString lst_attacks[] = { wxT("attack"), wxT("attack1"), wxT("attack2"), 
			wxT("attack3"), wxT("attack4"), wxT("attack5"), wxT("attack6"),
			wxT("attack7"), wxT("attack8"), wxT("attack9"),
			wxT("blast"), wxT("shock"), wxT("burn"), wxT("freeze"), wxT("steal") };
			for( int k = 0; k < 15; k++)
			{
				wxString _att = lst_attacks[k];
				ob_object *_attbox = _fr->GetSubObject(_att);
				if( _attbox != NULL )
				{
					int _m = (4 < _attbox->nb_tokens) ? 4 : _attbox->nb_tokens;
					for( int l=0;l<_m;l++)
					{
						int _new_val = StrToInt(_attbox->arr_token[l]) * ratio;
						_attbox->SetToken(l, IntToStr( _new_val ) );
					}
				}
			}

		}

		delete[] _frames;
	}
}


//************************************************************************************

void Panel_Anims::EvtClose( wxCloseEvent& event )
{
	// Set the closing flag
	itState = 1;
	event.StopPropagation();
	event.Skip();
}


//************************************************************************************

void Panel_Anims::EvtCharPress(wxKeyEvent& event)
{
//	int kc = event.GetKeyCode();
//	wxMessageBox( "Panel_Anims keypress", "Error", wxOK | wxICON_INFORMATION, this );
	event.Skip();
}

//--------------------------------------------------------------------
static 
bool
ANN__MyProceed_Frame_Navigation( wxKeyEvent& event, wListFrames* wList_frames )
{
	int kc = event.GetKeyCode();
	// Have to check if UP or Down have been pressed
	switch( kc )
	{
		case WXK_LEFT:
		case WXK_RIGHT:
			if( event.GetModifiers() == wxMOD_ALT )
			{
				event.m_altDown = false;
				wList_frames->ProcessEvent( event );
				return true;
			}
	}
	return false;
}

//--------------------------------------------------------------------
void
Panel_Anims::Evt_Ctrl_FrameNavig(wxKeyEvent& event )
{
	if( ANN__MyProceed_Frame_Navigation( event, wList_frames ) == true )
		return;
	event.Skip();
}

//--------------------------------------------------------------------
void
Panel_Anims::Evt_TxtCtrl_KeyUp(wxKeyEvent& event )
{
	if( ANN__MyProceed_Frame_Navigation( event, wList_frames ) == true )
		return;

	wxTextCtrl* theCtrl = (wxTextCtrl*) event.GetEventObject();
	int prev_val = StrToInt( theCtrl->GetValue() );
	bool b_skip = true;

	int kc = event.GetKeyCode();
	// Have to check if UP or Down have been pressed
	switch( kc )
	{
		case WXK_PAGEUP:
		case WXK_NUMPAD_PAGEUP:
			prev_val += 10;
			b_skip = false;
			break;

		case WXK_PAGEDOWN:
		case WXK_NUMPAD_PAGEDOWN:
			prev_val -= 10;
			b_skip = false;
			break;

		case WXK_UP:
		case WXK_NUMPAD_UP:
			prev_val += 1;
			b_skip = false;
			break;

		case WXK_DOWN:
		case WXK_NUMPAD_DOWN:
			prev_val -= 1;
			b_skip = false;
			break;
		
	}
	
	if( b_skip == true )
	{
		event.Skip();
		return;
	}
	
	theCtrl->SetValue(IntToStr(prev_val));
}

//************************************************************************************

void Panel_Anims::EvtImgPickerChg( wxFileDirPickerEvent& event )
{
	// Get the path relatively to dataDir
	wxString ob_path = Convert_To_Ob_Path( event.GetPath() );

	if( ob_path == wxString() )
	{
		wxMessageBox( wxT("The following file doesn't belong to the project dir :\n")+ event.GetPath(), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

 	panel_Anims->txtctrl_imgFile->ChangeValue( ob_path );
 	panel_Anims->txtctrl_imgFile->ShowPosition( ob_path.Len() - 1 );
 	wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED, IMG_FILE );
	panel_Anims->EvtCommand( evt );
}


//************************************************************************************

void Panel_Anims::EvtSndPickerChg( wxFileDirPickerEvent& event )
{
	// Get the path relatively to dataDir
	wxString ob_path = Convert_To_Ob_Path( event.GetPath() );

	if( ob_path == wxString() )
	{
		wxMessageBox( wxT("The following file doesn't belong to the project dir :\n")+ event.GetPath(), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

 	panel_Anims->txtctrl_soundFile->ChangeValue( ob_path );
 	panel_Anims->txtctrl_soundFile->ShowPosition( ob_path.Len() - 1 );
 	wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED, SND_FILE );
	panel_Anims->EvtCommand( evt );
}


//************************************************************************************

void Panel_Anims::EvtSize( wxSizeEvent& event )
{
	if( itState == 1 ) // Closing
		return;

	wList_frames->RefreshSelection();
	wList_frames->UpdateScrollBar();

	event.Skip();
}


//************************************************************************************

void Panel_Anims::EvtAnimListChange(wxCommandEvent& event)
{
	Panel_withAnims::EvtAnimListChange(event);
}


//************************************************************************************

void Panel_Anims::EvtAnimSelectionChange(wxCommandEvent& event)
{
	Panel_withAnims::EvtAnimSelectionChange(event);
	ATTACK__SetActive( wxT("DEF") );
}


//************************************************************************************

void Panel_Anims::EvtFramesListChange(wxCommandEvent& event)
{
	Panel_withAnims::EvtFramesListChange(event);
}

//************************************************************************************
void 
Panel_Anims::EvtFrameSelectionChange(wxCommandEvent& event)
{
	//Before everything, checks if the path of Image and sound have changed
	Panel_withAnims::EvtFrameSelectionChange(event);

	if( curr_frames_count <= 0 )
	{
		ResetFrameDatas();
		EnableFrameDatas( false );
	}
	else
	{
		ATTACK__SetActive(wxString());
		Update_FrameDatas();
	}

	if( animation_ctrl != NULL )
	{
		animation_ctrl->StopPlaying();
		animation_ctrl->UpdateFrames();
	}

	// Set the big frame number
	SetBigFrameNumber( ind_active_frame );
}



//************************************************************************************
//************************************************************************************
//************************************************************************************
//************************************************************************************
//************************************************************************************
// Event table
BEGIN_EVENT_TABLE(Panel_Anims, Panel_withAnims)
	EVT_BUTTON  (ID_NODRAWMETHOD, Panel_Anims::EvtButtonClick)

	EVT_BUTTON  (ID_NEW_ANIM,   	Panel_Anims::EvtButtonClick)
	EVT_BUTTON  (ID_NEW_FRAME,   	Panel_Anims::EvtButtonClick)

	EVT_BUTTON  (ID_ZOOM_M, Panel_Anims::EvtButtonClick)
	EVT_BUTTON  (ID_ZOOM_P, Panel_Anims::EvtButtonClick)

 	EVT_BUTTON  (ID_To_BBOX, Panel_Anims::EvtButtonClick)
 	EVT_BUTTON  (ID_To_XA, Panel_Anims::EvtButtonClick)
	EVT_BUTTON  (ID_To_ATTBOX, Panel_Anims::EvtButtonClick)
	EVT_BUTTON  (ID_To_OFFSET, Panel_Anims::EvtButtonClick)
	EVT_BUTTON  (ID_To_CLEAR, Panel_Anims::EvtButtonClick)

	EVT_BUTTON  (ID_PLAY_STOP, Panel_Anims::EvtButtonClick)
	EVT_BUTTON  (ID_LOOP_BTN, Panel_Anims::EvtButtonClick)
	EVT_BUTTON  (ID_SOUND_MUTE, Panel_Anims::EvtButtonClick)

	EVT_CLOSE(Panel_Anims::EvtClose)

	EVT_SIZE(Panel_Anims::EvtSize)
END_EVENT_TABLE()

//****************************************************
//			THE SCROLL PANNEL
//****************************************************

TheScrollPanel::TheScrollPanel( wxWindow *_parent )
: AScrollPanel( _parent  )
{
}


//************************************************************************************

TheScrollPanel::~TheScrollPanel()
{
	wxWindowList l_children = GetChildren();

	for ( wxWindowListNode *node = l_children.GetFirst(); node; node = node->GetNext() )
	{
		node->GetData()->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED);
		node->GetData()->Disconnect(wxEVT_COMMAND_TEXT_UPDATED);
		node->GetData()->Disconnect(wxEVT_KILL_FOCUS);
		node->GetData()->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED);
		node->GetData()->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED);
		node->GetData()->Disconnect(wxEVT_COMMAND_FILEPICKER_CHANGED);
	}
}


//************************************************************************************

void TheScrollPanel::EvtSndPickerChg( wxFileDirPickerEvent& event )
{
	if( panel_Anims != NULL )
		panel_Anims->EvtSndPickerChg( event );
}


//************************************************************************************

void TheScrollPanel::EvtImgPickerChg( wxFileDirPickerEvent& event )
{
	if( panel_Anims != NULL )
		panel_Anims->EvtImgPickerChg( event );
}


//************************************************************************************

void TheScrollPanel::EvtCommand( wxCommandEvent& event )
{
	if( panel_Anims != NULL )
		panel_Anims->EvtCommand( event );
}


//************************************************************************************

void TheScrollPanel::EvtButtonClick(wxCommandEvent& event)
{
	if( panel_Anims != NULL )
		panel_Anims->EvtButtonClick( event );
}

