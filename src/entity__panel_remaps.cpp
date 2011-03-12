/*
 * entity_panel_remaps.cpp
 *
 *  Created on: 7 nov. 2008
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/slider.h>
#include <wx/bmpbuttn.h>

#include "ob_editor.h"
#include "images__MyPalette.h"
#include "entity__enums.h"
#include "entity__globals.h"
#include "entity__MyAnimationZone.h"


//****************************************************
//			PANEL ANIMS
//****************************************************

Panel_Remaps *panel_Remaps;


Panel_Remaps::Panel_Remaps( wxNotebook *onglets )
:Panel_withAnims( onglets )
{
	Constructor();
}


void Panel_Remaps::Constructor()
{

	// Could by use by subobjects
	panel_Remaps = this;
	curr_ind_remap = -1;
	mode8bit = false;

	// Some inits
	entityAnimation_time = -1;
	itState = 0;

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
		sizer_top->Add( sizer_scrollWndFrameProps, 0, wxALL | wxEXPAND);

		wxBoxSizer *sizer_topRight = new wxBoxSizer( wxVERTICAL );
		sizer_top->Add( sizer_topRight, 1, wxALL | wxEXPAND );

	//Bottom part of the panel
		wxSizer *sizer_bottom = new wxBoxSizer( wxHORIZONTAL );
		sizer_main->Add( sizer_bottom, 0, wxALL | wxEXPAND  );

	//******************
	//* OBJECTS
	//******************

		//**************************
		// LIST OF ANIMS
		//**************************

		list_Anims = new wListAnims(this);
		list_Anims->SetMinSize( wxSize(150,50));
		sizer_topLeft->Add( list_Anims, 0, wxALL |wxEXPAND );


		//**************************
		// CENTER PANEL
		//**************************

		scrollPanel = new AScrollPanel( this );
		sizer_scrollWndFrameProps->Add( scrollPanel, 1, wxALL, 0 );

		wxBoxSizer *sizer_topmiddle = new wxBoxSizer( wxVERTICAL );
//		int fix_spacer_height = 12;


		//**************************
		// Remaps drop-down box
		//**************************
		sizer_remaps_choice = new wxBoxSizer( wxHORIZONTAL );
		sizer_topmiddle->Add( sizer_remaps_choice, 0, wxALL | wxEXPAND );

		txt_remaps = new wxStaticText( scrollPanel, wxID_ANY, wxT("Remap :" ));
		sizer_remaps_choice->Add( txt_remaps, 0, wxALIGN_CENTER  | wxALL , 4 );

		wxString lst_choices[] = {};
		choiceBox_remaps = new wxChoice( scrollPanel, REMAP_CHOICE, wxDefaultPosition, wxDefaultSize
						,	t_size_of(lst_choices), lst_choices);
		choiceBox_remaps->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(Panel_Remaps::EvtRemapChange), NULL, this);
		sizer_remaps_choice->Add( choiceBox_remaps, 0, wxALL, 4  );

		sizer_remaps_choice->AddSpacer( 20 );

		chckbx_8bit = new wxCheckBox(scrollPanel,B_8BIT, wxT("8Bit-Mode"),
				wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
		chckbx_8bit->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(Panel_Remaps::Evt8BitModeChange), new wxVariant((long) B_BLOK), this);
		chckbx_8bit->MoveAfterInTabOrder( choiceBox_remaps );
		sizer_remaps_choice->Add( chckbx_8bit, 1, wxALIGN_CENTER | wxALL );

		wxBitmapButton *help_butt = new wxBitmapButton( scrollPanel, ID_HELP_REMAPS,
								wxBitmap( wxImage( GetRessourceFile_String(wxT("help.png")))));
		help_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this);
		sizer_remaps_choice->Add( help_butt, 0, wxALL, 2 );

		//**************************
		// Butttons new/rename/del
		//**************************
		wxGridSizer *sizer_edit_btns = new wxGridSizer( 1, 3, 5, 5 );
		sizer_topmiddle->Add( sizer_edit_btns, 0, wxTOP | wxBOTTOM | wxEXPAND, 10 );

			//---------------------
			// New Remap
			wxButton *btn_new_remap = new wxButton(scrollPanel, ID_NEW_REMAP, wxT("New"), wxDefaultPosition, wxSize( wxDefaultCoord, 30) );
			btn_new_remap->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_edit_btns->Add( btn_new_remap, 0, wxALL | wxEXPAND, 5 );

			//---------------------
			// Rename Remap
			btn_rename_remap = new wxButton(scrollPanel, ID_RENAME_REMAP, wxT("Rename"), wxDefaultPosition, wxSize( wxDefaultCoord, 30) );
			btn_rename_remap->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_edit_btns->Add( btn_rename_remap, 0, wxALL | wxEXPAND, 5 );

			//---------------------
			// Delete Remap
			btn_delete_remap = new wxButton(scrollPanel, ID_DELETE_REMAP, wxT("Delete"), wxDefaultPosition, wxSize( wxDefaultCoord, 30) );
			btn_delete_remap->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_edit_btns->Add( btn_delete_remap, 0, wxALL | wxEXPAND, 5 );


		//**************************
		// Paths
		//**************************
		wxStaticBoxSizer *sizer_PATHS = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("Paths") );
		staticBoxPath = sizer_PATHS->GetStaticBox();
		wxColour _colour;
		sizer_topmiddle->Add( sizer_PATHS, 0, wxRIGHT | wxEXPAND );

			//---------------------
			// BASE IMAGE
			//---------------------
			wxBoxSizer *sizer_baseimg = new wxBoxSizer( wxHORIZONTAL );
			sizer_PATHS->Add( sizer_baseimg, 0 , wxEXPAND );

			txt_base_img = new wxStaticText( scrollPanel, wxID_ANY, wxT("Base"), wxPoint(wxDefaultCoord,8), wxSize( 80, wxDefaultCoord) );
			sizer_baseimg->Add( txt_base_img, 0, wxALL, 4 );

			txtctrl_imgBase = new wxTextCtrl(scrollPanel, IMG_BASE, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
			txtctrl_imgBase->Connect( wxEVT_COMMAND_TEXT_ENTER , wxCommandEventHandler(Panel_Remaps::EvtCommand), NULL, this);
			txtctrl_imgBase->SetInsertionPointEnd();
			txtctrl_imgBase->MoveAfterInTabOrder( chckbx_8bit );
			sizer_baseimg->Add( txtctrl_imgBase, 1, wxEXPAND );

			pickerBaseImg = new wxFilePickerCtrl( scrollPanel, ID_PICKER_BASE_IMG,
							      dataDirPath.GetFullPath(), wxT("Select the image base for the remap"), wxT("*.*"),
								wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST );
			pickerBaseImg->Connect(wxEVT_COMMAND_FILEPICKER_CHANGED , wxFileDirPickerEventHandler(Panel_Remaps::EvtPickerImgBaseChg), NULL, this);
			pickerBaseImg->SetMaxSize( wxSize(30, wxDefaultCoord ));
			pickerBaseImg->SetInitialSize(wxSize(30, 30 ));
			sizer_baseimg->Add( pickerBaseImg, 0, wxALL );

/*
			//---------------------
			// Create base imgage for remaps button
			wxButton *btn_create_base = new wxButton(scrollPanel, ID_CREATE_BASEIMG_REMAP, "Create a base for Remaps" );
			btn_create_base->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_PATHS->Add( btn_create_base, 0, wxALL | wxEXPAND, 5 );
*/
			//---------------------
			// DEST IMG
			//---------------------
			wxBoxSizer *sizer_dest_img = new wxBoxSizer( wxHORIZONTAL );
			sizer_PATHS->Add( sizer_dest_img, 1 , wxEXPAND );

			txt_result = new wxStaticText( scrollPanel, wxID_ANY, wxT("Result"), wxPoint(wxDefaultCoord,8), wxSize( 80, wxDefaultCoord) );
			sizer_dest_img->Add( txt_result, 0, wxALL, 4 );

			txtctrl_destimg = new wxTextCtrl(scrollPanel, IMG_DEST, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
			txtctrl_destimg->Connect( wxEVT_COMMAND_TEXT_ENTER , wxCommandEventHandler(Panel_Remaps::EvtCommand), NULL, this);
			txtctrl_destimg->SetInsertionPointEnd();
			txtctrl_destimg->MoveAfterInTabOrder( txtctrl_imgBase );
			sizer_dest_img->Add( txtctrl_destimg, 1, wxEXPAND );

			pickerDestImg = new wxFilePickerCtrl( scrollPanel, ID_PICKER_DEST_IMG,
							      dataDirPath.GetFullPath(), wxT("Select the sound for this frame"), wxT("*.*"),
						wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_FILE_MUST_EXIST );
			pickerDestImg->Connect(wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler(Panel_Remaps::EvtPickerImgDestChg), NULL, this);
			pickerDestImg->SetMaxSize( wxSize(30, wxDefaultCoord ));
			pickerDestImg->SetInitialSize(wxSize(30, 30 ));
			sizer_dest_img->Add( pickerDestImg, 0, wxALL );

		//**************************
		// TOOLS BUTTONS
		//**************************
		sizer_tools_btns = new wxGridSizer( 2, 2, 0, 0 );
		sizer_topmiddle->Add( sizer_tools_btns, 0, wxTOP | wxBOTTOM | wxEXPAND, 10 );

			//---------------------
			// Guess the colours from the current frame
			btn_guess_cols = new wxButton(scrollPanel, ID_GUESS_COLORS, wxT("Guess Cols"), wxDefaultPosition, wxSize( wxDefaultCoord, 30) );
			btn_guess_cols->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_tools_btns->Add( btn_guess_cols, 0, wxALL | wxEXPAND, 5 );


			//---------------------
			// Save the remap
			btn_save_remap = new wxButton(scrollPanel, ID_SAVE_REMAP, wxT("Save"), wxDefaultPosition, wxSize( wxDefaultCoord, 30) );
			btn_save_remap->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_tools_btns->Add( btn_save_remap, 0, wxALL | wxEXPAND, 5 );


			//---------------------
			// Set the selection to the selected colors
			btn_set_sel = new wxButton(scrollPanel, ID_SET_SEL_COLS, wxT("Set Selects"), wxDefaultPosition, wxSize( wxDefaultCoord, 30) );
			btn_set_sel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_tools_btns->Add( btn_set_sel, 0, wxALL | wxEXPAND, 5 );

			//---------------------
			// UnSet the selection
			btn_unset_sel = new wxButton(scrollPanel, ID_UNSET_SEL_COLS, wxT("UnSet them"), wxDefaultPosition, wxSize( wxDefaultCoord, 30) );
			btn_unset_sel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_tools_btns->Add( btn_unset_sel, 0, wxALL | wxEXPAND, 5 );

		//**************************
		// Palette Object
		//**************************
			paletteCtrl = new MyPaletteCtrl( scrollPanel );
			sizer_topmiddle->Add(paletteCtrl, 0, wxALL | wxEXPAND );


		//**************************
		// MODIF COLORS SLIDERS
		//**************************

sizer_HSV = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("Colours Modifications") );
			sizer_topmiddle->Add( sizer_HSV, 0, wxTOP | wxEXPAND, 10 );

			//---------------------
			// Label RGB
			txt_RGB = new wxStaticText( scrollPanel, wxID_ANY, wxT("Red / Green / Blue" ));
			sizer_HSV->Add( txt_RGB, 0, wxALIGN_CENTER | wxTOP, 10 );

			//---------------------
			// Red
			red_ctrl = new wxSlider( scrollPanel, ID_RED_CTRL, 0, -100, 100 );
			red_ctrl->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			red_ctrl->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			red_ctrl->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			red_ctrl->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			red_ctrl->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			red_ctrl->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sizer_HSV->Add( red_ctrl, 0, wxTOP | wxEXPAND, 10 );

			//---------------------
			// Green
			green_ctrl = new wxSlider( scrollPanel, ID_GREEN_CTRL, 0, -100, 100 );
			green_ctrl->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			green_ctrl->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			green_ctrl->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			green_ctrl->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			green_ctrl->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			green_ctrl->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sizer_HSV->Add( green_ctrl, 0, wxALL | wxEXPAND, 1 );

			//---------------------
			// Blue
			blue_ctrl = new wxSlider( scrollPanel, ID_BLUE_CTRL, 0, -100, 100 );
			blue_ctrl->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			blue_ctrl->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			blue_ctrl->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			blue_ctrl->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			blue_ctrl->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			blue_ctrl->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sizer_HSV->Add( blue_ctrl, 0, wxALL | wxEXPAND, 1 );

			//---------------------
			// Label HSV
			txt_HSL = new wxStaticText( scrollPanel, wxID_ANY, wxT("Hue / Saturation / Brightness") );
			sizer_HSV->Add( txt_HSL, 0, wxALIGN_CENTER | wxTOP, 10 );

			//---------------------
			// Hue
			hue_ctrl = new wxSlider( scrollPanel, ID_HUE_CTRL, 0, -50, 50 );
			hue_ctrl->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			hue_ctrl->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			hue_ctrl->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			hue_ctrl->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			hue_ctrl->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			hue_ctrl->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sizer_HSV->Add( hue_ctrl, 0, wxTOP | wxEXPAND, 10 );

			//---------------------
			// Saturation
			sat_ctrl = new wxSlider( scrollPanel, ID_SAT_CTRL, 0, -50, 50 );
			sat_ctrl->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sat_ctrl->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sat_ctrl->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sat_ctrl->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sat_ctrl->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sat_ctrl->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sizer_HSV->Add( sat_ctrl, 0, wxALL | wxEXPAND, 1 );

			//---------------------
			// Luz
			lum_ctrl = new wxSlider( scrollPanel, ID_LUM_CTRL, 0, -50, 50 );
			lum_ctrl->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			lum_ctrl->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			lum_ctrl->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			lum_ctrl->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			lum_ctrl->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			lum_ctrl->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(Panel_Remaps::EvtSlidersChg), NULL, this );
			sizer_HSV->Add( lum_ctrl, 0, wxALL | wxEXPAND, 1 );

		//**************************
		// BUTTONS APPLY/CANCEL
		//**************************
		wxGridSizer *sizer_btns_AppCanc = new wxGridSizer( 1, 2, 5, 5 );
		sizer_topmiddle->Add( sizer_btns_AppCanc, 0, wxALL |wxEXPAND);

		btn_apply_cols = new wxButton(scrollPanel, ID_APPLY_COLORS, wxT("Apply") );
			btn_apply_cols->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_btns_AppCanc->Add( btn_apply_cols, 1, wxALL | wxEXPAND, 5 );

			btn_cancel_cols = new wxButton(scrollPanel, ID_CANCEL_COLORS, wxT("Cancel") );
			btn_cancel_cols->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this );
			sizer_btns_AppCanc->Add( btn_cancel_cols, 1, wxALL | wxEXPAND, 5 );


		//**************************
		// Frame View
		//**************************
		wxBoxSizer *sizer_btns_Zoom = new wxBoxSizer( wxHORIZONTAL );
		sizer_topRight->Add(sizer_btns_Zoom, 0,wxEXPAND );

		sizer_btns_Zoom->AddStretchSpacer();
		wxBitmapButton *butt_Zoom_M = new wxBitmapButton( this, ID_ZOOM_M,
								  wxBitmap( wxImage( GetRessourceFile_String(wxT("zoom-out.png")))));
		butt_Zoom_M->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this);
		sizer_btns_Zoom->Add( butt_Zoom_M, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

		wxBitmapButton *butt_Zoom_P = new wxBitmapButton( this, ID_ZOOM_P,
								  wxBitmap( wxImage( GetRessourceFile_String(wxT("zoom-in.png")))));
		butt_Zoom_P->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(Panel_Remaps::EvtButtonClick), NULL, this);
		sizer_btns_Zoom->Add( butt_Zoom_P, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

		sizer_btns_Zoom->AddStretchSpacer();

		frameView = new MyRemapViewerCtrl( this );
		sizer_topRight->Add(frameView, 1, wxEXPAND );

		//**************************
		// LIST OF FRAMES
		//**************************
		wList_frames = new wListFrames( this );
		sizer_bottom->Add( wList_frames, 1, wxALL );

	//**************************
	// INITIALISATIONS
	//**************************
	scrollPanel->SetSizer( sizer_topmiddle);
	SetSizer( sizer_main );
	wList_frames->AssociateToListAnims( list_Anims);
	list_Anims->AssociateToListFrame( wList_frames);
	Layout();
}


//*********************************************************************

Panel_Remaps::~Panel_Remaps()
{
}


//*********************************************************************

void Panel_Remaps::Refresh()
{
	SetSize( GetSize() );
	Panel_withAnims::Refresh();
	frameView->UpdateFrames();
}


//*********************************************************************

void Panel_Remaps::ReloadGifs()
{
	Panel_withAnims::ReloadGifs();
	frameView->UpdateFrames();
}



//*********************************************************************

void Panel_Remaps::Reload()
{
	Panel_withAnims::Refresh();
	wxString prev_selection = choiceBox_remaps->GetStringSelection();

	paletteCtrl->Reset(false);

	if( entity == NULL)
		return;

	if( mode8bit )
		SwitchTo8BitMode();
	else
		SwitchTo16BitMode();

	// Try to restore previous selection
	for( size_t i = 0; i < choiceBox_remaps->GetCount(); i++)
	{
		wxString choice_item = choiceBox_remaps->GetString( i );
		if( choice_item.Upper() == prev_selection.Upper() )
		{
			choiceBox_remaps->SetSelection(i);
			DoRemapSelectionChange();
		}
	}

	frameView->UpdateFrames();
}


//*********************************************************************

void Panel_Remaps::OnActivate()
{
	Panel_withAnims::OnActivate();

	if( b_entity_has_change )
	{
		paletteCtrl->Reset();
		GessBitMode();
	}

	Refresh();
}


//*********************************************************************

void Panel_Remaps::GessBitMode()
{
	if( entity == NULL )
		return;

	ob_object* _thePalette = entity->obj_container->GetSubObject( wxT("palette") );
	if( _thePalette != NULL && entity->GetProperty( wxT("alternatepal") ) != NULL )
		SwitchTo16BitMode();
	else
		SwitchTo8BitMode();
}

//*********************************************************************

void Panel_Remaps::InitRemapsChoices()
{
	choiceBox_remaps->Clear();

// Check 16bit mode
	if( !mode8bit )
	{
		ob_object* _thePalette = entity->GetProperty( wxT("palette") );

		size_t nb_alterpal = 0;
		ob_object** _alterpals = NULL;
		if( _thePalette != NULL )
			_alterpals  = entity->GetSubObjectS(wxT("alternatepal"),nb_alterpal);

		// if 16 bits mod exist choose it
		if( nb_alterpal > 0 )
		{
			for( size_t i = 0; i < nb_alterpal; i++ )
			{
				// Get the comment for this remap
				wxString _name = _alterpals[i]->GetComment();
				if( _name == wxString() )
					_name = wxT("alternatepal ") + IntToStr( i );

				_name = _name.Trim().Trim(true);
				if( _name.Left(1) == wxT("#") )
					_name = _name.Right( _name.Len() - 1 );
				_name = _name.Left(15);
				choiceBox_remaps->Append( _name );
			}
			delete[] _alterpals;
		}
	}
	else
	{
		// 8bit mode
		size_t nb_remaps;
		ob_object** _remaps = entity->GetSubObjectS( wxT("remap"), nb_remaps );

		for( size_t i = 0; i < nb_remaps; i++ )
		{
			// Get the comment for this remap
			wxString _name = _remaps[i]->GetComment();
			if( _name == wxString() )
				_name = wxT("remap ") + IntToStr( i );

			_name = _name.Trim().Trim(true);
			if( _name.Left(1) == wxT("#") )
				_name = _name.Right( _name.Len() - 1 );
			_name = _name.Left(15);
			choiceBox_remaps->Append( _name );
		}
		if( _remaps != NULL )
			delete[] _remaps;
	}

	if( choiceBox_remaps->GetCount() > 0 )
	{
		choiceBox_remaps->SetSelection(0);
		curr_ind_remap = 0;
	}
	else
		curr_ind_remap = -1;


	DoRemapSelectionChange();

}


//*********************************************************************

void Panel_Remaps::SwitchTo8BitMode()
{
	// Check the 8bit box
	chckbx_8bit->SetValue( true );
	mode8bit = true;
	paletteCtrl->mode8bit = true;

	// Set some statictext
	txt_base_img->SetLabel( wxT("Base") );
	txt_result->SetLabel(wxT("Result") );
	btn_guess_cols->SetLabel( wxT("Create Base") );

	//Hide some buttons
	EnableHSV( false );

	// Init remap choices
	InitRemapsChoices();

	sizer_tools_btns->RecalcSizes();
/*	wxSizeEvent event;
	scrollPanel->EvtSize( event );*/
	scrollPanel->SetSize(scrollPanel->GetSize());
	scrollPanel->Refresh();
}


//*********************************************************************

void Panel_Remaps::SwitchTo16BitMode()
{
	// Set the 8bit box
	chckbx_8bit->SetValue( false );
	mode8bit = false;
	paletteCtrl->mode8bit = false;

	// Set some statictext
	txt_base_img->SetLabel( wxT("Palette") );
	txt_result->SetLabel( wxT("AlterPal") );
	btn_guess_cols->SetLabel( wxT("Guess Cols") );

	// Show all HSB controls
	EnableHSV( true );
	ResetHSV();

	// Init remap choices
	InitRemapsChoices();

	sizer_tools_btns->Layout();
/*	wxSizeEvent event;
	scrollPanel->EvtSize( event );*/
	scrollPanel->SetSize(scrollPanel->GetSize());
	scrollPanel->Refresh();
}


//*********************************************************************

void Panel_Remaps::EnableHSV( bool b_enable )
{

	// Show/Hide some tools Buttons
	btn_set_sel->Show(b_enable);
	btn_unset_sel->Show(b_enable);

	// Hide all HSV
	sizer_HSV->GetStaticBox()->Show( b_enable );
	txt_HSL->Show( b_enable );
	hue_ctrl->Show( b_enable );
	sat_ctrl->Show( b_enable );
	lum_ctrl->Show( b_enable );
	red_ctrl->Show( b_enable );
	txt_RGB->Show( b_enable );
	green_ctrl->Show( b_enable );
	blue_ctrl->Show( b_enable );
	btn_apply_cols->Show( b_enable );
	btn_cancel_cols->Show( b_enable );

}


//*********************************************************************

void Panel_Remaps::EvtRemapChange( wxCommandEvent& event )
{
	if(	paletteCtrl->isChanged && curr_ind_remap >= 0 )
	{
		//Make the user sure to abort his modifications
		int res = wxMessageBox( wxT("Change have been made in the current palette.\n"
		"Sure to cancel them ?")
		, wxT("Question"), wxYES_NO | wxICON_INFORMATION, this );
		if( res != wxYES )
		{
			// Reselect previous selected
			choiceBox_remaps->SetSelection( curr_ind_remap );
			return;
		}
	}

	DoRemapSelectionChange();
}


//*********************************************************************

void Panel_Remaps::DoRemapSelectionChange( bool b_quiet )
{
	if( entity == NULL )
	{
		wxMessageBox( wxT("No entity selected !!!"), wxT("GROSS PROBLEM"), wxOK | wxICON_INFORMATION, this );
		paletteCtrl->Reset();
		scrollPanel->Enable( false );
		curr_remap = NULL;
		return;
	}

	scrollPanel->Enable( true );
	ResetHSV();

	//******************************
	// Get the new remap object
	int ind_remap = choiceBox_remaps->GetSelection();
	if( ind_remap == wxNOT_FOUND )
	{
		paletteCtrl->Reset();
		curr_remap = NULL;
		EnableEditingTheRemap(2);
		Refresh();
		return;
	}

	size_t nb_remap = 0;
	ob_object** _remaps = NULL;
	if( mode8bit )
		_remaps = entity->GetSubObjectS( wxT("remap"), nb_remap );
	else
		_remaps = entity->GetSubObjectS( wxT("alternatepal"), nb_remap );


	if( ind_remap < 0 || ind_remap >= (int) nb_remap )
	{
		choiceBox_remaps->SetSelection( wxNOT_FOUND );
		curr_remap = NULL;
		wxMessageBox( wxT("This remap does not exist !!!"), wxT("GROSS PROBLEM"), wxOK | wxICON_INFORMATION, this );
		EnableEditingTheRemap(1);
		Refresh();
		if( _remaps != NULL )
			delete[] _remaps;
		return;
	}

	curr_remap = _remaps[ind_remap];
	delete[] _remaps;
	curr_ind_remap = ind_remap;


	//******************************
	// Fill the fields of paths
	wxString _basePath;
	wxString _destPath;
	if( mode8bit )
	{
		_basePath = curr_remap->GetToken(0);
		_destPath = curr_remap->GetToken(1);
	}
	else
	{
		_basePath = entity->obj_container->GetSubObject_Token( wxT("palette") );
		_destPath = curr_remap->GetToken(0);
	}

	txtctrl_imgBase->SetValue( _basePath );
	if( _basePath != wxString() )
		pickerBaseImg->SetPath( GetObFile(_basePath).GetFullPath() );
	_basePath = GetObFile(_basePath).GetFullPath();

	txtctrl_destimg->SetValue( _destPath );
	if( _destPath != wxString() )
		pickerDestImg->SetPath( GetObFile(_destPath).GetFullPath() );
	_destPath = GetObFile(_destPath).GetFullPath();

	//******************************
	// Try to Load the palette
	if( DoChangeImgBase( b_quiet, false ) )
		DoChangeImgDest( b_quiet, false );

	paletteCtrl->DeselectEveryThing();
	Refresh();
}


void Panel_Remaps::DoCreateBase()
{
	// Make the user sure
	int res = wxMessageBox( wxT("This will overwrite the current base image.\n"
						"It will be replaced by a copy image,\n"
						"at which will be added all the missing colors of the palette"
						"Sure to do this ?")
						, wxT("Question"), wxYES_NO | wxICON_INFORMATION, this );
	if( res == wxYES )
	{
		if( paletteCtrl->theSourceImg == NULL || !paletteCtrl->theSourceImg->IsOk() )
		{
			wxMessageBox( wxT("Invalide base image for the operation ")
			, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
			return;
		}

		// Get the base name
		// Ask for the name
		wxFileDialog _dialog(this, wxT("Choose a file for the base img for remaps"),dataDirPath.GetFullPath() );
		int response = _dialog.ShowModal();

		if( response  != wxID_OK )
			return;

		wxString fn_base = _dialog.GetPath();

		// Check if it's a valid ob project pa
		wxString str_path = Convert_To_Ob_Path( fn_base );
		if( str_path == wxString() )
		{
			wxMessageBox( wxT("Can't use this file.\nIs it really in the project data Directory ?"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
			return;
		}

		// Draw each palette element on the base image
		int _y=0;
		int _x=-1;
		for( int i = 0; i < paletteCtrl->nb_elts ; i++)
		{
			_x++;
			// Ok
			if( paletteCtrl->theSourceImg->SetPixel( _x, _y, i ) )
				continue;

			// must go to line
			_y++;
			_x=-1;
			i--;
		}

		//Save the base
		if( ! paletteCtrl->theSourceImg->SaveAs( fn_base ) )
		{
			wxMessageBox( wxT("Cannot save the resulted image ")
			, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
			return;
		}

		// Set the new base to the textcontrol
		txtctrl_imgBase->SetValue( str_path );

		// Refresh the palette
		DoChangeImgBase(true);
		return;
	}
}


//*********************************************************************

void Panel_Remaps::Evt8BitModeChange( wxCommandEvent& event )
{
	if( mode8bit != chckbx_8bit->GetValue() )
	{
		// Make the user sure
		if(	paletteCtrl->isChanged && curr_ind_remap >= 0 )
		{
			//Make the user sure to abort his modifications
			int res = wxMessageBox( wxT("Change have been made in the current palette.\nSure to cancel them ?")
			, wxT("Question"), wxYES_NO | wxICON_INFORMATION, this );
			if( res != wxYES )
			{
				// Restore the previous state
				chckbx_8bit->SetValue( mode8bit );
				return;
			}
		}

		paletteCtrl->DeselectEveryThing();
		if( chckbx_8bit->GetValue() )
			SwitchTo8BitMode();
		else
			SwitchTo16BitMode();

	}
}


//*********************************************************************

void Panel_Remaps::EnableEditingTheRemap( int mode)
{
	wxWindowList l_children = scrollPanel->GetChildren();

	for ( wxWindowListNode *node = l_children.GetFirst(); node; node = node->GetNext() )
	{
		wxWindow *curr_child = node->GetData();
		if(    curr_child->GetId() != ID_NEW_REMAP
			&& curr_child->GetId() != ID_RENAME_REMAP
			&& curr_child->GetId() != ID_DELETE_REMAP
			&& curr_child->GetId() != REMAP_CHOICE
			&& curr_child->GetId() != IMG_BASE
			&& curr_child->GetId() != ID_HELP_REMAPS
			&& curr_child->GetId() != ID_PICKER_BASE_IMG
			&& curr_child->GetId() != IMG_DEST
			&& curr_child->GetId() != ID_PICKER_DEST_IMG
			&& curr_child->GetId() != B_8BIT
			&& curr_child != txt_base_img
			&& curr_child != txt_result
			&& curr_child != txt_remaps
			&& curr_child != staticBoxPath
			)
			curr_child->Enable( mode == 0 );
	}

	bool b_enable_ext = (mode < 2);
	btn_delete_remap->Enable( b_enable_ext);
	btn_rename_remap->Enable( b_enable_ext );
	txt_base_img->Enable( b_enable_ext );
	txt_result->Enable( b_enable_ext );
	txt_remaps->Enable( b_enable_ext );
	choiceBox_remaps->Enable( b_enable_ext );
//	btn_save_remap->Enable( b_enable_ext );
	txt_base_img->Enable( b_enable_ext );;
	txtctrl_imgBase->Enable( b_enable_ext );
	pickerBaseImg->Enable( b_enable_ext );
	txt_result->Enable( b_enable_ext);
	txtctrl_destimg->Enable( b_enable_ext );
	pickerDestImg->Enable( b_enable_ext );
}

//*********************************************************************

bool Panel_Remaps::EntityChanged()
{
	return entity->changed;
}


//*********************************************************************

bool Panel_Remaps::Save_DoChecks()
{
	// Check if there is a valid source img
	if( ! paletteCtrl->b_init || paletteCtrl->theSourceImg == NULL || ! paletteCtrl->theSourceImg->IsOk())
	{
		wxMessageBox( wxT("There is no valide source Image for the remap ")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return false;
	}

	//Get the dest file
	wxFileName dest_file = GetObFile( txtctrl_destimg->GetValue() );

	// Checks Destination not specified
	wxString str_fn = dest_file.GetFullPath();
	if( str_fn == wxString() )
	{
		wxMessageBox( wxT("No Destination Image specified !!")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return false;
	}

	wxString _ext = dest_file.GetExt().Upper();
	if( _ext != wxT("PNG") && _ext != wxT("GIF") )
	{
		wxMessageBox( wxT("The extension of the destination file is not GIF or PNG !!")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return false;
	}

	// Checks mismatch file types
	if(
		(_ext == wxT("PNG") && paletteCtrl->theSourceImg->type != pIMG_PNG )
			||
			(_ext == wxT("GIF") && paletteCtrl->theSourceImg->type != pIMG_GIF )
	   )
	{
		wxMessageBox( wxT("Source and destination Image haven't the same extension")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return false;
	}
	return true;
}


//*********************************************************************

bool Panel_Remaps::Save_16Bit_Remap()
{
	if( ! Save_DoChecks() )
		return false;

	imgFile *srcImg = paletteCtrl->theSourceImg;
	wxFileName dest_file = GetObFile( txtctrl_destimg->GetValue() );

	// Save the previous palette of the source
	MyPalette* prev_palette = srcImg->GetPalette();
	if( prev_palette == NULL || !prev_palette->IsOk() )
	{
		wxMessageBox( wxT("Problem when saving the source palette !!")
		, wxT("GROSS ProPlem"), wxOK | wxICON_INFORMATION, this );
		if( prev_palette != NULL )
			delete prev_palette;
		return false;
	}

	// Apply the new palette to the source
	if( ! srcImg->SetPalette( paletteCtrl->GetNewPalette() ))
	{
		wxMessageBox( wxT("Problem when setting the new palette !!")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		delete prev_palette;
		return false;
	}


	//Save the image
	bool res = srcImg->SaveAs( dest_file.GetFullPath());

	// Restore the palette
	srcImg->SetPalette( *prev_palette );
	delete prev_palette;

	if( ! res )
	{
		wxMessageBox( wxT("Problem when saving the resulting image !!")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return false;
	}

	paletteCtrl->isChanged = false;
	return true;
}

//*********************************************************************

bool Panel_Remaps::Save_8Bit_Remap()
{
	if( ! Save_DoChecks() )
		return false;

	imgFile *srcImg = paletteCtrl->theSourceImg;
	wxFileName dest_file = GetObFile( txtctrl_destimg->GetValue() );

	// Copy image Data
	int srcImg_datas_size;
	unsigned char *srcImg_datas = srcImg->GetDatas( srcImg_datas_size );

	// Remap the image
	int sz_remap = paletteCtrl->nb_elts;
	unsigned char do_remap[sz_remap];

	for(int i = 0; i < sz_remap; i++ )
	{
		int remapTo = paletteCtrl->paletteElements[i]->mappedTo;
		if( remapTo < 0 )
			remapTo = i;

		do_remap[i]   = remapTo;
	}
	bool res = srcImg->Remap( do_remap, sz_remap );

	//Save the image
	if( res )
		res = srcImg->SaveAs( dest_file.GetFullPath());

	// Undo the remap
	srcImg->SetDatas( srcImg_datas, srcImg_datas_size );

	if( ! res )
	{
		wxMessageBox( wxT("Problem when saving the resulting image !!")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return false;
	}

	paletteCtrl->isChanged = false;
	return true;
}

//*********************************************************************

bool Panel_Remaps::DoChangeImgDest( bool b_quiet, bool standalone)
{
	if( !paletteCtrl->b_init )
		return false;

	if( curr_remap == NULL )
		return false;

	//******************************
	// Get the new path
	wxString dest_obPath = txtctrl_destimg->GetValue();
	wxString _destPath = GetObFile( dest_obPath ).GetFullPath();
	if( _destPath == wxString() )
		b_quiet = true;

	//******************************
	// Make the change to the ob_object
	if( mode8bit )
	{
		wxString old_path = curr_remap->GetToken( 1 );
		if( old_path != dest_obPath )
		{
			entity->SetChanged();
			if( curr_remap->GetToken( 0 ) == wxString() )
				curr_remap->SetToken( 0, wxT("UNSETTED") );
			curr_remap->SetToken( 1, dest_obPath );
		}
	}
	else
	{
		// Set the palette tag
		wxString old_path = curr_remap->GetToken( 0 );
		if( old_path != dest_obPath )
		{
			entity->SetChanged();
			curr_remap->SetToken( 0, dest_obPath );
		}
	}

	//******************************
	// Undo the previous remapping
	paletteCtrl->UndoRemapping();

	bool res = false;

	//******************************
	// Check if the destination file exist
	if( ! wxFileName( _destPath ).FileExists() && !b_quiet )
		wxMessageBox( wxT("The Destination image for the remap doesn't exist")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );

	else if( IsFileEmpty(_destPath) && !b_quiet )
		wxMessageBox( wxT("The Destination image for the remap is an empty file")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );

	else if( ! wxImage( _destPath ).IsOk() && !b_quiet)
		wxMessageBox( wxT("Problem with the loading destination image of the remap")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );

	else if( paletteCtrl->b_init )
	{
		res = paletteCtrl->InitRemapping_With( _destPath );
		if( ! res && !b_quiet)
			wxMessageBox(   wxT( "Problem with apply remapping between the two images\nAre they really fit each others ?")
			, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );

	}

	// Force refresh in case remapping didn"t work
	if( ! res )
		paletteCtrl->Refresh();

	if( standalone )
		Refresh();

	return res;
}


//*********************************************************************

bool Panel_Remaps::DoChangeImgBase( bool b_quiet, bool standalone)
{
	paletteCtrl->Reset(false);
	if( curr_remap == NULL )
		return false;

	int i_EnableEditingTheRemap = 1;
	bool res = false;

	wxString ob_basePath = txtctrl_imgBase->GetValue();
	wxString _basePath = GetObFile( ob_basePath ).GetFullPath();

	//******************************
	// Make the change to the ob_object
	if( mode8bit )
	{
		wxString old_path = curr_remap->GetToken( 0 );
		if( old_path != ob_basePath )
		{
			entity->SetChanged();
			curr_remap->SetToken( 0, ob_basePath );
		}
	}
	else
	{
		ob_object *pal_obobj = entity->GetProperty( wxT("palette") );

		// No current palette tag in the current entity
		wxString old_path = wxString();
		if( pal_obobj == NULL )
		{
			pal_obobj = new ob_object();
			pal_obobj->SetName( wxT("palette") );

			//Try to insert one before the first alternatepal
			size_t nb_remap;
			ob_object** _t = entity->GetSubObjectS( wxT("alternatepal"),nb_remap );
			
			if( _t != NULL )
			{
				_t[nb_remap-1]->InsertObject_After( pal_obobj );
				delete[] _t;
			}
			// Try to insert it before the first anim
			else
				entity->AddProperty( pal_obobj );
		}
		else
			old_path = pal_obobj->GetToken( 0 );

		// Set the palette tag
		if( old_path != ob_basePath )
		{
			// Check if the user really want to change the image base for all remaps
			int res = wxMessageBox( wxT("Do your really want to change the image base for all 16Bits remaps ?"), wxT("Question"), wxYES_NO | wxICON_INFORMATION, this );
			if( res != wxYES )
				return false;

			entity->SetChanged();
			pal_obobj->SetToken( 0, ob_basePath );
		}
	}


	//******************************
	// Check if it's a valid path
	if( ! wxFileName( _basePath ).FileExists() && !b_quiet )
	{
		wxMessageBox( wxT("The Base image for this remap doesn't exist")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
	}

	else if( IsFileEmpty(_basePath) && !b_quiet )
	{
		wxMessageBox( wxT("The Base image for this remap is an empty file !!")
		, wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
	}


	//******************************
	// Try to load the palette
	else if( paletteCtrl->TryToInitWithImage( _basePath ) )
	{
		res = true;
		i_EnableEditingTheRemap = 0;

		//******************************
		// Try to quietly to load the dest palette
		if( standalone  )
			DoChangeImgDest( true, false);
	}

	EnableEditingTheRemap(i_EnableEditingTheRemap);
	if( standalone )
		Refresh();

	return res;
}


//*********************************************************************

bool Panel_Remaps::Do_NewRemap()
{
	if( entity == NULL )
		return false;

	// Ask for the name
	wxTextEntryDialog _dialog(this, wxT("New remap"), wxT("Name for the new remap "), wxString(),
				wxOK | wxCANCEL | wxCENTRE );
	int response = _dialog.ShowModal();

	if( response  == wxID_OK )
	{
		wxString remap_name = _dialog.GetValue().Trim().Trim(false);

		// Get current remaps
		size_t nb_remaps = 0;
		ob_object** curr_remaps = NULL;
		if( !mode8bit )
			curr_remaps = entity->GetSubObjectS( wxT("alternatepal"), nb_remaps );
		else
			curr_remaps = entity->GetSubObjectS( wxT("remap"), nb_remaps );

		// Give a default name if none is provided
		if( remap_name == wxString())
		{
			if( mode8bit )
				remap_name = wxT("remap ") + IntToStr( nb_remaps );
			else
				remap_name = wxT("alternatepal ") + IntToStr( nb_remaps );
		}

		ob_object * new_remap;
		// If it's the first remap object
		if( curr_remaps == NULL )
		{
			// No previous remap => Juste add it as a property
			wxString _tag = wxT("remap");
			if( !mode8bit )
				_tag = wxT("alternatepal");

			new_remap = new ob_object();
			new_remap->SetName( _tag );
			entity->AddProperty( new_remap );
		}

		// Insert a new remap object
		else
		{
			if( mode8bit )
			{
				wxString prev_baseImg = curr_remaps[nb_remaps-1]->GetToken( 0);
				new_remap = new ob_object();
				new_remap->SetName( wxT("remap") );
				new_remap->SetToken( 0, prev_baseImg );
			}
			else
			{
				new_remap = new ob_object();
				new_remap->SetName( wxT("alternatepal") );
			}

			curr_remaps[nb_remaps-1]->InsertObject_After( new_remap );
			delete[] curr_remaps;
		}

		wxString _comment = wxT("#") + remap_name;
		new_remap->SetComment( _comment );

		// Append and select the new remap object in the remap control choices
		choiceBox_remaps->Append( remap_name.Left(15) );
		choiceBox_remaps->SetSelection( nb_remaps );
		sizer_remaps_choice->Layout();
		entity->SetChanged();
		DoRemapSelectionChange( true );
		return true;
	}
	return false;
}


//*********************************************************************

bool Panel_Remaps::Do_RenameRemap()
{
	if( entity == NULL || curr_remap == NULL )
		return false;

	// Ask for the name
	wxTextEntryDialog _dialog(this, wxT("Rename remap"), wxT("New name for this remap "), wxString(),
				wxOK | wxCANCEL | wxCENTRE );
	int response = _dialog.ShowModal();

	if( response  == wxID_OK )
	{
		wxString remap_name = _dialog.GetValue().Trim().Trim(false);
		if( remap_name == wxString() )
			return false;

		wxString _comment = wxT("\t#") + remap_name;
		curr_remap->SetComment( _comment );
		choiceBox_remaps->SetString( choiceBox_remaps->GetSelection(), remap_name.Left( 15 ) );
		sizer_remaps_choice->Layout();
		entity->SetChanged();
		return true;
	}
	return false;
}


//*********************************************************************

bool Panel_Remaps::Do_DeleteRemap()
{
	if( entity == NULL || curr_remap == NULL )
		return false;

	// Ask the user to be sure
	int res = wxMessageBox( wxT("Sure to delete it ?"), wxT("Question"), wxYES_NO | wxICON_INFORMATION, this );
	if( res != wxYES )
		return false;

	curr_remap->Rm();
	curr_remap = NULL;

	size_t ind_selected = choiceBox_remaps->GetSelection();
	choiceBox_remaps->Delete( ind_selected );
	if( choiceBox_remaps->GetCount() > 0 )
	{
		if( ind_selected >= choiceBox_remaps->GetCount() )
			ind_selected--;
		choiceBox_remaps->SetSelection( ind_selected );
	}
	sizer_remaps_choice->Layout();
	entity->SetChanged();
	DoRemapSelectionChange( false );
	return true;
}


//*********************************************************************

void Panel_Remaps::EvtCommand( wxCommandEvent& event )
{
	switch( event.GetId() )
	{

		case IMG_BASE:
		{
			DoChangeImgBase();
			return;
		}

		case IMG_DEST:
		{
			DoChangeImgDest(true);
			return;
		}
	}
}


//*********************************************************************

void Panel_Remaps::EvtButtonClick(wxCommandEvent& event)
{
	wxWindow* _src  = (wxWindow*) event.GetEventObject();
	if( _src == NULL )
		return;

	switch( _src->GetId() )
	{
		case ID_SAVE_REMAP:
			if( mode8bit )
				Save_8Bit_Remap();
			else
				Save_16Bit_Remap();
			return;

		case ID_APPLY_COLORS:
		{
			paletteCtrl->ValidateHSLs();
			ResetHSV();
			return;
		}

		case ID_CANCEL_COLORS:
		{
			paletteCtrl->InValidateHSLs();
			ResetHSV();
			frameView->Refresh();
			return;
		}

		case ID_GUESS_COLORS:
		{
			// In real, this button have to usage
			if( ! mode8bit )
			{
				paletteCtrl->DoGuessColors();
				frameView->Refresh();
			}
			else
			{
				DoCreateBase();
			}
			return;
		}

		case ID_SET_SEL_COLS:
		{
			paletteCtrl->DoFixSelectedColors();
			return;
		}

		case ID_UNSET_SEL_COLS:
		{
			paletteCtrl->UnDoFixSelectedColors();
			return;
		}

		case ID_NEW_REMAP :
		{
			Do_NewRemap();
			return;
		}

		case ID_RENAME_REMAP :
		{
			Do_RenameRemap();
			return;
		}

		case ID_DELETE_REMAP :
		{
			Do_DeleteRemap();
			return;
		}

		case ID_ZOOM_P :
		{
			frameView->Zoom(1.3);
			list_Anims->SetFocus();
			return;
		}

		case ID_ZOOM_M :
		{
			frameView->Zoom(0.8);
			list_Anims->SetFocus();
			return;
		}

		case ID_HELP_REMAPS:
		{
			WndFromText( this, wxT("HowTo do remaps"), GetRessourceFile_String( wxT("remaps.txt") )).ShowModal();
			break;
		}
	}
}


//************************************************************************************

void Panel_Remaps::ResetHSV()
{
	hue_ctrl->SetValue(0);
	sat_ctrl->SetValue(0);
	lum_ctrl->SetValue(0);
	red_ctrl->SetValue(0);
	green_ctrl->SetValue(0);
	blue_ctrl->SetValue(0);
}

//************************************************************************************

void Panel_Remaps::EvtAnimSelectionChange(wxCommandEvent& event)
{
	Panel_withAnims::EvtAnimSelectionChange( event );
}


//************************************************************************************

void Panel_Remaps::EvtFrameSelectionChange(wxCommandEvent& event)
{
	Panel_withAnims::EvtFrameSelectionChange( event );
/*
	if( curr_frames != NULL && ind_active_frame >= 0 && ! paletteCtrl->IsInit() )
		paletteCtrl->TryToInitWithImage( curr_frames[ind_active_frame]->GetImage() );
*/
	frameView->UpdateFrames();
}


//************************************************************************************

void Panel_Remaps::EvtClose( wxCloseEvent& )
{
}


//************************************************************************************

void Panel_Remaps::EvtSize( wxSizeEvent& event )
{
/*	wxSize _sizer_size = scrollPanel->GetContainingSizer()->GetSize();
	wxSize _virtual_size = scrollPanel->GetVirtualSize();
	scrollPanel->SetMinSize( wxSize(_virtual_size.GetWidth(), _sizer_size.GetHeight()-20 ));
	scrollPanel->SetScrollRate( 0, 20 );*/
	event.Skip();
}


//*********************************************************************


void Panel_Remaps::EvtPickerImgBaseChg( wxFileDirPickerEvent& )
{
	// Get the path
	wxString str_path = Convert_To_Ob_Path( pickerBaseImg->GetPath() );

	if( str_path == wxString() )
	{
		wxMessageBox( wxT("Can't use this file.\nIs it really in the project data Directory ?"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}
	txtctrl_imgBase->SetValue( str_path );
	DoChangeImgBase();
}


//*********************************************************************

void Panel_Remaps::EvtPickerImgDestChg( wxFileDirPickerEvent& event )
{
	// Get the path
	wxString str_path = Convert_To_Ob_Path( pickerDestImg->GetPath() );

	if( str_path == wxString() )
	{
		wxMessageBox( wxT("Can't use this file.\nIs it really in the project data Directory ?"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}
	txtctrl_destimg->SetValue( str_path );
	DoChangeImgDest();
}


//*********************************************************************

void Panel_Remaps::EvtSlidersChg( wxScrollEvent& event )
{
	// Get the slider id
	wxSlider *sender = (wxSlider*) event.GetEventObject();
	if( sender == NULL )
		return;

	int val = sender->GetValue();

	switch( sender->GetId())
	{
		case ID_HUE_CTRL :
			paletteCtrl->Set_Hue( val );
			break;
		case ID_SAT_CTRL :
			paletteCtrl->Set_Sat( val );
			break;
		case ID_LUM_CTRL :
			paletteCtrl->Set_Luz( val );
			break;
		case ID_RED_CTRL :
			paletteCtrl->Set_Red( val );
			break;
		case ID_GREEN_CTRL :
			paletteCtrl->Set_Green( val );
			break;
		case ID_BLUE_CTRL :
			paletteCtrl->Set_Blue( val );
			break;
		default:
			return;
	}

	frameView->UpdateFrames();
}

void Panel_Remaps::EvtPaletteRemapChange(wxCommandEvent& event )
{
	// Refresh the view
	frameView->Refresh();
}


void Panel_Remaps::EvtKillFocus(wxFocusEvent& event )
{
	if(	paletteCtrl->isChanged && curr_ind_remap >= 0 )
	{
		//Make the user sure to abort his modifications
		int res = wxMessageBox( wxT("Change have been made in the current palette.\n"
							 "If you don't save them now you can loose them\n" 
							 "Do the save ?")
							 , wxT("Question"), wxYES_NO | wxICON_INFORMATION, this );

		if( res == wxYES )
		{
			if( mode8bit )
				Save_8Bit_Remap();
			else
				Save_16Bit_Remap();
		}
	}
	paletteCtrl->isChanged = false;
	event.Skip();
}

//*********************************************************************
//*********************************************************************
//*********************************************************************
BEGIN_EVENT_TABLE(Panel_Remaps, Panel_withAnims )
	EVT_ANIM_SELECTED_CHANGE(Panel_Remaps::EvtAnimSelectionChange)
	EVT_FRAME_SELECTED_CHANGE(Panel_Remaps::EvtFrameSelectionChange)

	PALETTE_ELT_CLICKED( Panel_Remaps::EvtPaletteRemapChange)

	EVT_SIZE(Panel_Remaps::EvtSize)

	EVT_KILL_FOCUS(Panel_Remaps::EvtKillFocus)
	EVT_CLOSE(Panel_Remaps::EvtClose)
END_EVENT_TABLE()
