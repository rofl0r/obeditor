/*
 * entity_panel_platform.cpp
 *
 *  Created on: 17 nov. 2008
 *      Author: pat
 */
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif


#include "ob_editor.h"
#include "images__MyPalette.h"
#include "entity__enums.h"
#include "entity__globals.h"
#include "entity__MyAnimationZone.h"


Panel_Platform *panel_Platform;


//************************************************************************************

Panel_Platform::Panel_Platform(wxNotebook* _parent)
:Panel_withAnims(_parent)
{
	panel_Platform = this;
	off_x = off_y = 0;
	up_l = up_r = 0;
	d_l = d_r = 0;
	depth = alt = 0;

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

		list_Anims = new wListAnims(this);
		list_Anims->SetMinSize( wxSize(150,50));
		sizer_topLeft->Add( list_Anims, 0, wxALL |wxEXPAND );


		//**************************
		// CENTER PANEL
		//**************************

		scrollPanel = new AScrollPanel( this );
		sizer_scrollWndFrameProps->Add( scrollPanel, 1, wxALL, 0 );

		wxBoxSizer *sizer_topCenter = new wxBoxSizer( wxVERTICAL );

		scrollPanel->SetSizer( sizer_topCenter );
		int border_size = 4;


			//**************************
			// PLATFORM DATAS
			//**************************

			chckbx_noplatform = new wxCheckBox(scrollPanel, ID_NO_PLATFORM,	wxT("No/Clone Platform"), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
			chckbx_noplatform->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED , wxCommandEventHandler(Panel_Platform::EvtButtonClick), NULL, this );
			sizer_topCenter->Add( chckbx_noplatform, 0, wxALL | wxALIGN_LEFT, border_size );

			wxStaticBoxSizer *sizer_PlatformDatas = new wxStaticBoxSizer( wxVERTICAL, scrollPanel, wxT("Platform Datas") );
			sizer_topCenter->Add( sizer_PlatformDatas, 0, wxALL, border_size );

			wxGridSizer *sizer_temp = new wxGridSizer( 8, 2, border_size, border_size );
			sizer_PlatformDatas->Add( sizer_temp, 0 , wxEXPAND );

			// OFFSET X
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("xPos"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_off_x = new wxTextCtrl(scrollPanel, ID_PLAT_OFF_X, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_off_x->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				sizer_temp->Add( txtctrl_off_x, 0, wxALL | wxCENTER, border_size );
			}


			// OFFSET Y
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("yPos"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_off_y = new wxTextCtrl(scrollPanel, ID_PLAT_OFF_Y, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_off_y->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				txtctrl_off_y->MoveAfterInTabOrder( txtctrl_off_x );
				sizer_temp->Add( txtctrl_off_y, 0, wxALL | wxCENTER, border_size );
			}


			// Uper left
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("upperleft"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_up_l = new wxTextCtrl(scrollPanel, ID_PLAT_UPL, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_up_l->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				txtctrl_up_l->MoveAfterInTabOrder( txtctrl_off_y );
				sizer_temp->Add( txtctrl_up_l, 0, wxALL | wxCENTER, border_size );
			}


			// lower left
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("lowerleft"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_d_l = new wxTextCtrl(scrollPanel, ID_PLAT_DL, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_d_l->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				txtctrl_d_l->MoveAfterInTabOrder( txtctrl_up_l );
				sizer_temp->Add( txtctrl_d_l, 0, wxALL | wxCENTER, border_size );
			}


			// Upper right
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("upperright"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_up_r = new wxTextCtrl(scrollPanel, ID_PLAT_UPR, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_up_r->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				txtctrl_up_r->MoveAfterInTabOrder( txtctrl_d_l );
				sizer_temp->Add( txtctrl_up_r, 0, wxALL | wxCENTER, border_size );
			}


			// Lower right
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("lowerright"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_l_r = new wxTextCtrl(scrollPanel, ID_PLAT_LR, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_l_r->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				txtctrl_l_r->MoveAfterInTabOrder( txtctrl_up_r );
				sizer_temp->Add( txtctrl_l_r, 0, wxALL | wxCENTER, border_size );
			}


			// depth
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("depth"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_depth = new wxTextCtrl(scrollPanel, ID_PLAT_DEPTH, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_depth->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				txtctrl_depth->MoveAfterInTabOrder( txtctrl_l_r );
				sizer_temp->Add( txtctrl_depth, 0, wxALL | wxCENTER, border_size );
			}


			// altitude
			{
				wxStaticText *txt_temp = new wxStaticText( scrollPanel, wxID_ANY, wxT("alt"), wxPoint(wxDefaultCoord, 8) );
				sizer_temp->Add( txt_temp, 0, wxALL | wxCENTER, border_size );

				txtctrl_alt = new wxTextCtrl(scrollPanel, ID_PLAT_ALT, wxString(),wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT );
				txtctrl_alt->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(Panel_Platform::EvtCommand), NULL, this);
				txtctrl_alt->MoveAfterInTabOrder( txtctrl_depth );
				sizer_temp->Add( txtctrl_alt, 0, wxALL | wxCENTER, border_size );
			}

			// The color chooser of the platform
			wxSizer *sizer_color = new wxBoxSizer( wxHORIZONTAL );
			sizer_topCenter->Add( sizer_color, 0 , wxALL );

			wxStaticText *txt_color = new wxStaticText( scrollPanel, wxID_ANY, wxT("Color :"), wxPoint(wxDefaultCoord, 8) );
			sizer_color->Add( txt_color, 0, wxALL | wxCENTER, border_size );

			plat_color = new MyPaletteElementCtrl(scrollPanel);
			plat_color->SetId( ID_BBOX_COLOR );
			plat_color->SetRGB( 0,0,0 );
			plat_color->SetMode( AUTO_ELTCOLOR );
			sizer_color->Add( plat_color, 0, wxCENTER | wxALL, 2 );

		//**************************
		// ANIMATION CONTROL
		//**************************

		sizer_topRightUp->AddStretchSpacer();

		//**************************
		//Animation controls
		wxBoxSizer *sizer_anim_ctrls = new wxBoxSizer( wxHORIZONTAL );
		sizer_topRightUp->Add( sizer_anim_ctrls, 0, wxALL|wxEXPAND, 6 );

		wxBitmapButton* butt_Zoom_M = new wxBitmapButton( this, ID_ZOOM_M,
				wxBitmap( wxImage( GetRessourceFile_String(wxT("zoom-out.png")))));
		sizer_anim_ctrls->Add( butt_Zoom_M, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

		wxBitmapButton* butt_Zoom_P = new wxBitmapButton( this, ID_ZOOM_P,
				wxBitmap( wxImage( GetRessourceFile_String(wxT("zoom-in.png")))));
		sizer_anim_ctrls->Add( butt_Zoom_P, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

		sizer_topRightUp->AddStretchSpacer();


		//**************************
		// THE ANIMATION ZONE
		//**************************

		platformView = new MyPlatformViewerCtrl(this );
		sizer_topRightDown->Add( platformView, 1, wxALL | wxEXPAND, 2 );

		//**************************
		// LIST OF FRAMES
		//**************************

		wList_frames = new wListFrames( this );
		sizer_bottom->Add( wList_frames, 1, wxALL );


	//**************************
	// INITIALISATIONS
	//**************************
  	SetSizer( sizer_main );
  	wList_frames->AssociateToListAnims( list_Anims);
  	list_Anims->AssociateToListFrame( wList_frames);
	list_Anims->ReloadLastSessionAnim();
	list_Anims->Refresh_List();
	Layout();

}


//************************************************************************************

Panel_Platform::~Panel_Platform()
{
}


//************************************************************************************

void Panel_Platform::OnActivate()
{
	Panel_withAnims::OnActivate();

	if( b_entity_has_change )
	{
		// Check for new platform datas
		Update_PlaftformDatas();
	}

	platformView->UpdateFrames();
	wxSizeEvent event;
	scrollPanel->EvtSize( event );
	Layout();
}


//*********************************************************************

void Panel_Platform::Reload()
{
	Panel_withAnims::Refresh();
	Update_PlaftformDatas();
	platformView->UpdateFrames();
}


//*********************************************************************

void Panel_Platform::ReloadGifs()
{
	Panel_withAnims::ReloadGifs();
	platformView->UpdateFrames();
}


//************************************************************************************

extern wxColour cloneColour;

void Panel_Platform::Update_PlaftformDatas()
{
	if( entity == NULL || curr_anim == NULL )
	{
		scrollPanel->Enable( false );
		return;
	}

	if( frameActive == NULL || frameActive->GetImage() == noImg )
	{
		scrollPanel->Enable( false );
		wxMessageBox( wxT("Not a valid frame for making a platform"), wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	scrollPanel->Enable( true );

	ob_object* platform_prop = frameActive->GetSubObject( wxT("platform") );

	if( platform_prop == NULL )
		EnablePlatform( false );
	else
	{
		int nb_tokens = platform_prop->nb_tokens;
		if( nb_tokens != 6 && nb_tokens != 8 )
		{
			scrollPanel->Enable( false );
			wxMessageBox( wxT("Malformed platform property in entity header"), wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
			return;
		}

		EnablePlatform( true );

		// In case of old platform format, convert to new format
		if( nb_tokens == 6 )
		{
			// set some default value to offset x and y
			ob_object* fr_offsets = frameActive->GetSubObject( wxT("offset") );
			platform_prop->SetName( wxT("platform" ));
			platform_prop->SetToken( 0, fr_offsets->GetToken( 0 ) );
			platform_prop->SetToken( 1, fr_offsets->GetToken( 1 ) );

			for( int i =0; i < 6; i ++ )
				platform_prop->SetToken( i+2, platform_prop->GetToken(i) );

			entity->SetChanged();
		}

		txtctrl_off_x->ChangeValue( platform_prop->GetToken(0));
		txtctrl_off_y->ChangeValue( platform_prop->GetToken(1));
		txtctrl_up_l->ChangeValue(  platform_prop->GetToken(2));
		txtctrl_d_l->ChangeValue(   platform_prop->GetToken(3));
		txtctrl_up_r->ChangeValue(  platform_prop->GetToken(4));
		txtctrl_l_r->ChangeValue(   platform_prop->GetToken(5));
		txtctrl_depth->ChangeValue( platform_prop->GetToken(6));
		txtctrl_alt->ChangeValue(   platform_prop->GetToken(7));

		UpdateBG_colours( platform_prop );
		return;
	}
}


//************************************************************************************

void Panel_Platform::EnablePlatform( bool b_enable )
{
	chckbx_noplatform->SetValue( ! b_enable );

	txtctrl_off_x->Enable( b_enable );
	txtctrl_off_y->Enable( b_enable );
	txtctrl_up_l->Enable( b_enable );
	txtctrl_d_l->Enable( b_enable );
	txtctrl_up_r->Enable( b_enable );
	txtctrl_l_r->Enable( b_enable );
	txtctrl_depth->Enable( b_enable );
	txtctrl_alt->Enable( b_enable );
}


//************************************************************************************

void Panel_Platform::EvtCommand(wxCommandEvent& event )
{
	Update_entity_platform();
}


//************************************************************************************

void Panel_Platform::Update_entity_platform()
{
	if( curr_anim == NULL || frameActive == NULL )
	{
		wxMessageBox( wxT("No animation selected !!"), wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	ob_object* platform_prop = frameActive->GetSubObject( wxT("platform" ));

	if( platform_prop == NULL )
		return;
	else
	{
		// If no change
		if(    platform_prop->GetToken(0) == txtctrl_off_x->GetValue()
			&& platform_prop->GetToken(1) == txtctrl_off_y->GetValue()
			&& platform_prop->GetToken(2) == txtctrl_up_l->GetValue()
			&& platform_prop->GetToken(3) == txtctrl_d_l->GetValue()
			&& platform_prop->GetToken(4) == txtctrl_up_r->GetValue()
			&& platform_prop->GetToken(5) == txtctrl_l_r->GetValue()
			&& platform_prop->GetToken(6) == txtctrl_depth->GetValue()
			&& platform_prop->GetToken(7) == txtctrl_alt->GetValue()
			)
			return;

		platform_prop->SetToken(0, txtctrl_off_x->GetValue() );
		platform_prop->SetToken(1, txtctrl_off_y->GetValue() );
		platform_prop->SetToken(2, txtctrl_up_l->GetValue() );
		platform_prop->SetToken(3, txtctrl_d_l->GetValue() );
		platform_prop->SetToken(4, txtctrl_up_r->GetValue() );
		platform_prop->SetToken(5, txtctrl_l_r->GetValue() );
		platform_prop->SetToken(6, txtctrl_depth->GetValue() );
		platform_prop->SetToken(7, txtctrl_alt->GetValue() );

		entity->SetChanged();

		UpdateBG_colours( platform_prop );
		platformView->UpdateFrames();
	}
}


//************************************************************************************

void Panel_Platform::UpdateBG_colours( ob_object* platform_prop )
{
	wxColour t_col( *wxWHITE );
	if( 	   platform_prop->parent != NULL 
		&& ((ob_frame*) platform_prop->parent)->b_platform_cloned == true )
		t_col = cloneColour;

	txtctrl_off_x->SetBackgroundColour( t_col );
	txtctrl_off_y->SetBackgroundColour( t_col );
	txtctrl_up_l->SetBackgroundColour( t_col );
	txtctrl_d_l->SetBackgroundColour( t_col );
	txtctrl_up_r->SetBackgroundColour( t_col );
	txtctrl_l_r->SetBackgroundColour( t_col );
	txtctrl_depth->SetBackgroundColour( t_col );
	txtctrl_alt->SetBackgroundColour( t_col );

}


//************************************************************************************

void Panel_Platform::EvtAnimSelectionChange(wxCommandEvent& event)
{
	Panel_withAnims::EvtAnimSelectionChange(event);
}


//************************************************************************************

void Panel_Platform::EvtButtonClick(wxCommandEvent& event)
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

	switch( id_wind )
	{
		case ID_ZOOM_M:
			platformView->Zoom(0.8);
			list_Anims->SetFocus();
			return;

		case ID_ZOOM_P:
			platformView->Zoom(1.3);
			list_Anims->SetFocus();
			return;

		case ID_NO_PLATFORM:
			if( curr_anim == NULL || frameActive == NULL )
				return;

			// Deletion case
			if( chckbx_noplatform->GetValue() == true )
			{
				ob_object* platform_prop = frameActive->GetSubObject( wxT("platform" ));
				if( platform_prop != NULL )
				{
					platform_prop->Rm();
					entity->SetChanged();
				}
				Frame_CascadeChanges();
				Update_PlaftformDatas();
				// If a cloned value has appear, decheck the box
				if( frameActive->GetSubObject( wxT("platform") ) != NULL )
					chckbx_noplatform->SetValue( false );
				platformView->UpdateFrames();
			}

			// Creation case
			else
			{
				if( frameActive == NULL || frameActive->GetImage() == NULL || frameActive->GetImage() == noImg )
				{
					scrollPanel->Enable( false );
					wxMessageBox( wxT("Not a valid frame for making a platform"), wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
					return;
				}

				entity->SetChanged();

				// set some default value
				ob_object* fr_offsets = frameActive->GetSubObject( wxT("offset") );
				wxString new_tokens[8];
				new_tokens[0] = fr_offsets->GetToken( 0 );
				new_tokens[1] = fr_offsets->GetToken( 1 );

				wxImage* curr_img = frameActive->GetImage();
				int w = curr_img->GetWidth();
				int h = curr_img->GetHeight();
				new_tokens[2] = IntToStr( -w/4 );
				new_tokens[3] = IntToStr( -w/4 );
				new_tokens[4] = IntToStr( w/4 );
				new_tokens[5] = IntToStr( w/4 );
				new_tokens[6] = IntToStr( w/4 );
				new_tokens[7] = IntToStr( 2*h/3 );

				frameActive->SetProperty( wxT("platform"), new_tokens, 8 );

				Update_PlaftformDatas();
				platformView->UpdateFrames();
			}
			return;
	}
	return;
}

//************************************************************************************

void Panel_Platform::EvtFrameSelectionChange(wxCommandEvent& event)
{
	Panel_withAnims::EvtFrameSelectionChange(event);

	Frame_CascadeChanges();
	Update_PlaftformDatas();

	if( platformView != NULL )
	{
		platformView->UpdateFrames();
	}

}



//************************************************************************************
//************************************************************************************
//************************************************************************************
//************************************************************************************
//************************************************************************************
// Event table
BEGIN_EVENT_TABLE(Panel_Platform, Panel_withAnims)

	EVT_BUTTON  (ID_ZOOM_M, Panel_Platform::EvtButtonClick)
	EVT_BUTTON  (ID_ZOOM_P, Panel_Platform::EvtButtonClick)

END_EVENT_TABLE()
