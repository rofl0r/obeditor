/*
 * levels__wndEditPanelOrder.cpp
 *
 *  Created on: 5 mai 2009
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/dcbuffer.h>

#include "levels__sidesWindows.h"
#include "levels__globals.h"


const int max_img_size = 200;
const int margins = 8;
const int imgs_space = 5;
const int margin_orderCtrl = 25;

//****************************************************

myWnd_EditPanelOrder::myWnd_EditPanelOrder( wxWindow* _parent, ob_stage* _stage )
:wxDialog( _parent, wxID_ANY, wxT("Panel Order"), wxDefaultPosition, wxDefaultSize
		,wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE | wxFULL_REPAINT_ON_RESIZE )
, font_Alphabet( 18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true )
, font_Label( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxNORMAL, true )
{
	ZeroInit();
	stage = _stage;
	
	b_reverted_direction = false;
	b_rotate_imgs = false;

/*
	switch( stage->direction )
	{
		case STAGE_DOWN:
		case STAGE_OUT:
		case STAGE_LEFT:
			b_reverted_direction = true;
	}
	
	switch( stage->direction )
	{
		case STAGE_UP:
		case STAGE_IN:
		case STAGE_DOWN:
		case STAGE_OUT:
			b_rotate_imgs = true;
	}
*/	
	Init();
}


void myWnd_EditPanelOrder::Init()
{
	//*****************************
	// Pre-setup
	wxBoxSizer* sizer_wrapper = new wxBoxSizer( wxHORIZONTAL );
	thePanel = new wxScrolledWindow( this );
	thePanel->EnableScrolling( false, true );
	thePanel->SetScrollRate( 0, 20 );
	sizer_wrapper->Add( thePanel, 1, wxEXPAND | wxALL, 10 );
	wxBoxSizer* sizer_main = new wxBoxSizer( wxVERTICAL );
	thePanel->SetSizer( sizer_main );



	//*****************************
	// SET the variables
	if( stage != NULL )
	{
		l_panels = (ob_stage_panel**) stage->GetSubObjectS_ofType( OB_TYPE_STAGE_PANEL, nb_panels );
		if( nb_panels == 0 )
			ZeroInit();
		else
			curr_order = stage->Get_Panels_Order();
	}

	// Stage == NULL
	if( stage == NULL )
	{
		wxStaticText* t = new wxStaticText( thePanel, wxID_ANY, wxT("STAGE OBJECT == NULL !!"));
		t->SetFont(font_Alphabet);
		sizer_main->Add( t, 0, wxCENTER );
	}


	//*****************************
	// Set up the window
	else if( nb_panels <= 0 )
	{
		wxStaticText* t = new wxStaticText( thePanel, wxID_ANY, wxT("NO PANELS DEFINED !!"));
		t->SetFont(font_Alphabet);
		sizer_main->Add( t, 0, wxCENTER );
	}

	else
	{
		wxBoxSizer* sizer_temp;
		wxStaticText* t_stat;

		//*****************************
		// Panel BGs List

		t_stat = new wxStaticText( thePanel, wxID_ANY, wxT("Avalaible panels"),
				wxDefaultPosition, wxDefaultSize );
		t_stat->SetFont( font_Label );
		sizer_main->Add( t_stat, 0, wxALL, margins );


		wxBoxSizer* sizerWrapper_panel_BGs = new wxBoxSizer( wxVERTICAL );
		AScrollPanel *panel_BGs = new AScrollPanel( thePanel );
//		panel_BGs->SetBackgroundColour( *wxWHITE );
		wxBoxSizer* panel_BGs_sizer = new wxBoxSizer( wxHORIZONTAL );
		panel_BGs->SetSizer( panel_BGs_sizer );
		panel_BGs->SetMinSize( wxSize(60, 60) );
		panel_BGs->Set_FixedDirections( false, true );
		sizerWrapper_panel_BGs->Add(panel_BGs, 1, wxEXPAND );
		sizer_main->Add( sizerWrapper_panel_BGs, 0, wxEXPAND );

		for( size_t i = 0; i < nb_panels; i++ )
		{
			sizer_temp = new wxBoxSizer( wxVERTICAL );

			MyImageCtrl* t_imgctrl = new MyImageCtrl( panel_BGs );
			t_imgctrl->Set_BoundDimensions( max_img_size, max_img_size );
			t_imgctrl->SetImage( l_panels[i]->GetImage() );
			sizer_temp->Add( t_imgctrl, 0, wxALL, imgs_space );

			t_stat = new wxStaticText( panel_BGs, wxID_ANY, l_panels[i]->GetName() );
			sizer_temp->Add( t_stat, 0, wxCENTER |wxALL, imgs_space );
			panel_BGs_sizer->Add( sizer_temp, 0, wxEXPAND |wxALL, imgs_space );

			t_stat = new wxStaticText( panel_BGs, wxID_ANY, wxChar( 'a' + i ),
					wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
			t_stat->SetFont( font_Alphabet );
			t_stat->SetMinSize( wxSize( 40, wxDefaultCoord ));
			sizer_temp->Add( t_stat , 0, wxCENTER |wxALL, 5);

		}


		//*****************************
		// ADD the control Text of the order
		sizer_temp = new wxBoxSizer( wxHORIZONTAL );
		t_stat = new wxStaticText( thePanel, wxID_ANY, wxT("Order\n(Use arrow Keys)") );
		sizer_temp->Add( t_stat, 0, wxCENTER );
		txtCtrl_order = new wxTextCtrl( thePanel, wxID_ANY, wxString(),
				wxDefaultPosition, wxDefaultSize, 0,
				wxValidator_Restrict_Range( 'a', wxChar( 'a' + nb_panels - 1 ) ) );
		txtCtrl_order->Connect( wxEVT_KEY_UP , wxKeyEventHandler(myWnd_EditPanelOrder::Evt_Order_KeyPress), NULL, this);
		txtCtrl_order->Connect( wxEVT_COMMAND_TEXT_UPDATED , wxCommandEventHandler(myWnd_EditPanelOrder::Evt_Order_Changed), NULL, this);
/*		txtCtrl_order->Connect( wxEVT_LEFT_UP , wxMouseEventHandler(myWnd_EditPanelOrder::Evt_Order_Mouse_In), NULL, this);
*/
		sizer_temp->Add( txtCtrl_order, 1, wxCENTER|wxTOP|wxBOTTOM|wxEXPAND, margin_orderCtrl );
		
		sizer_main->Add( sizer_temp, 0, wxEXPAND );

		//Init the order
/*		// No order, make a initial one
		if( curr_order == wxString() )
		{
			for( size_t i =0; i < nb_panels; i++ )
				curr_order += wxChar( 'a' + i );
//			stage->Add_SubObj( new ob_object( "order", curr_order ) );
		}*/
		txtCtrl_order->ChangeValue( curr_order );


		//*****************************
		// ADD the view of the order
		t_stat = new wxStaticText( thePanel, wxID_ANY, wxT("Order view"),
				wxDefaultPosition, wxDefaultSize );
		t_stat->SetFont( font_Label );
		sizer_main->Add( t_stat, 0, wxALL, margins );

		wxBoxSizer* sizerWrapper_stage_view = new wxBoxSizer( wxVERTICAL );
		panel_stage_view = new AScrollPanel( thePanel );
		panel_stage_view->SetMinSize( wxSize(100, 100) );
		panel_stage_view->Set_FixedDirections( false, true );
		panel_stage_view->SetBackgroundStyle( wxBG_STYLE_CUSTOM );
//		panel_stage_view->SetBackgroundColour( *wxWHITE );
		panel_stage_view->Connect( wxEVT_PAINT, wxPaintEventHandler(myWnd_EditPanelOrder::Evt_Paint_PanelView), NULL, this);
		sizerWrapper_stage_view->Add(panel_stage_view, 1, wxEXPAND );
		wxBoxSizer* panel_stage_view_sizer = new wxBoxSizer( wxHORIZONTAL );
		panel_stage_view->SetSizer( panel_stage_view_sizer );
		sizer_main->Add( sizerWrapper_stage_view, 0, wxEXPAND );

		img_ctrl_list = NULL;
		img_ctrl_list_count = 0;
		Update_PanelsView();


		//*****************************
		// ADD the OK/CANCEL buttons
		sizer_temp = new wxBoxSizer( wxHORIZONTAL );

		sizer_temp->AddStretchSpacer();

		wxButton* t_butt = new wxButton( thePanel, wxID_ANY, wxT("OK") );
		t_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED , wxCommandEventHandler(myWnd_EditPanelOrder::Evt_Click_OK), NULL, this);
		sizer_temp->Add( t_butt );

		sizer_temp->AddSpacer( 30 );

		t_butt = new wxButton( thePanel, wxID_ANY, wxT("CANCEL" ));
		t_butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED , wxCommandEventHandler(myWnd_EditPanelOrder::Evt_Click_CANCEL), NULL, this);
		sizer_temp->Add( t_butt );

		sizer_temp->AddStretchSpacer();

		sizer_main->Add( sizer_temp, 0, wxEXPAND | wxTOP, 15);
	}

	SetSizer( sizer_wrapper );
	Show();
	Frame_RestorePrevCoord( this, wxT("Panels_Ordering" ));
}


//****************************************************

void myWnd_EditPanelOrder::ZeroInit()
{
	stage = NULL;
	nb_panels = 0;
	l_panels = NULL;
	orders = NULL;
	b_chg = false;
	panel_cursor_x = -1;
	img_ctrl_list = NULL;
	img_ctrl_list_count = 0;
	tay_panels_map = 0;
	panels_map = NULL;
}

//****************************************************

void myWnd_EditPanelOrder::EvtClose( wxCloseEvent& event )
{
	Frame_SaveCoord( this, wxT("Panels_Ordering" ));
	event.Skip();
}


//****************************************************

myWnd_EditPanelOrder::~myWnd_EditPanelOrder()
{
	if( l_panels != NULL )
		delete[] l_panels;
}


//****************************************************

void myWnd_EditPanelOrder::EvtSize( wxSizeEvent& evt )
{
	wxSize s = thePanel->GetClientSize();
	int max_x = s.x - 20;
	if( max_x <= 10 )
		max_x = 10;
	thePanel->SetVirtualSizeHints(60,60, max_x, -1 );
//	SetSize( wxSize( s.x, wxDefaultCoord ) );
	thePanel->Layout();
	evt.Skip();
}


//****************************************************

void myWnd_EditPanelOrder::Evt_Click_OK( wxCommandEvent& evt )
{
	if( stage != NULL )
	{
		wxString old_order = stage->Get_Panels_Order();
		if( curr_order != old_order )
		{
			b_chg = true;
			orders = stage->GetSubObjectS( wxT("order"), nb_orders );

			if( orders == NULL )
			{
				orders = new ob_object*[1];
				orders[0] = new ob_object( wxT("order") );
				nb_orders = 1;
				stage->Add_SubObj( orders[0] );
			}
			
			size_t curr_ind = 0;
			ob_object* last_obj = orders[nb_orders-1];
			while( curr_order.Len() > 0 )
			{
				ob_object* o;
				if( curr_ind >= nb_orders )
				{
					o = new ob_object(wxT("order" ));
					last_obj->InsertObject_After( o );
				}
				else
					o = orders[curr_ind];
				
				if( curr_order.Len() <= 26 )
				{
					o->SetToken( 0, curr_order );
					curr_ind++;
					break;
				}
				
				o->SetToken( 0, curr_order.Left(26) );
				curr_order = curr_order.Right(curr_order.Len()-26);
				curr_ind++;
			}
			
			// Remove unused orders objects
			for( size_t i = curr_ind; i < nb_orders; i++ )
				delete orders[i];
			
			delete[] orders;
			orders = NULL;


/*			// Debug panel_map
			if( false && panels_map != NULL )
			{
				wxString str_panelmap = "[";
				for( int i = 0; i< tay_panels_map; i++)
				{
					str_panelmap += IntToStr( panels_map[i] ) + ",";
				}
				str_panelmap = str_panelmap.Mid(0, str_panelmap.Len() -1 );
				str_panelmap += "]";

				wxMessageBox( "Panels Map :\n" + str_panelmap );
			}*/
		}
	}
	Close();
}


//****************************************************

void myWnd_EditPanelOrder::Evt_Click_CANCEL( wxCommandEvent& evt )
{
//	Update_PanelsView();
	Close();
}


//****************************************************

void myWnd_EditPanelOrder::Evt_Click_OrderElt( wxMouseEvent& evt )
{
	int id = evt.GetId() - wxID_HIGHEST;
	if( b_reverted_direction )
		id = curr_order.Len() - id - 1;
	txtCtrl_order->SetInsertionPoint( id );
	UpdateInsertPoint();
	evt.Skip();
}


//****************************************************

void myWnd_EditPanelOrder::Evt_Order_Mouse_In( wxMouseEvent& evt )
{
	//int id = txtCtrl_order->GetInsertionPoint();
	txtCtrl_order->SetSelection( curr_ind, curr_ind );
	txtCtrl_order->SetInsertionPoint( curr_ind );
	txtCtrl_order->Update();
	UpdateInsertPoint();
}


//****************************************************

void myWnd_EditPanelOrder::Evt_Order_KeyPress( wxKeyEvent& evt )
{
	UpdateInsertPoint();
	evt.Skip();
}


//****************************************************

void myWnd_EditPanelOrder::Evt_Order_Changed( wxCommandEvent& evt )
{
	wxString new_order = txtCtrl_order->GetValue();
	if( new_order != curr_order )
	{
		bool b_accept_change = true;

		// Find the change
			// Insertion case
			if( curr_order.Len() < new_order.Len() )
			{
				int ind_change = -1;
				for( size_t i =0; i < curr_order.Len();i++)
				{
					if( curr_order[i] != new_order[i] )
					{
						ind_change = i;
						i = curr_order.Len();
					}
				}

				// have to handle the clones char case
				if( ind_change > 0 && new_order[ind_change] == new_order[ind_change -1] )
				{
					// get insertion point
					long i = txtCtrl_order->GetInsertionPoint();
					if( i >= 0 )
						ind_change = (int) i;
				}

				// Found the change
				if( ind_change != -1 )
				{
					// Check that everything is really unchanged after the mod
					bool b_no_change_after = true;
					for( size_t i = ind_change; i < curr_order.Len();i++ )
					{
						if( new_order[i+1] != curr_order[i] )
						{
							b_no_change_after = false;
							i = curr_order.Len();
						}
					}

					if( ! b_no_change_after )
					{
						wxMessageBox( wxT("More than one change at one time can't be handled !!\nChange after case...") );
						b_accept_change = false;
					}
					else
					{
						//Get the ind in map for the lower changed panel
						int ind_in_map = -1;
						for( int i = 0; i < tay_panels_map; i++ )
						{
							if( panels_map[i] >= ind_change )
							{
								ind_in_map = i;
								i = tay_panels_map;
							}
						}

						if( ind_in_map == -1 )
						{
							wxMessageBox( wxT("BUG ??\nNo indice in map found for the insertion !!!") );
							b_accept_change = false;
						}
						else
						{
							//must remap all the upper indices to indices +1
							for( int i= ind_in_map; i < tay_panels_map; i++ )
							{
								// except if the panel have been deleted
								if( panels_map[i] != -1 )
									panels_map[i] = panels_map[i] + 1;
							}
						}
					}
				}

				// Insertion after the old order -> ok
				else
				{
				}
			}

			// Suppression case
			else if( curr_order.Len() > new_order.Len() )
			{
				int ind_change = -1;
				for( size_t i =0; i < new_order.Len(); i++)
				{
					if( curr_order[i] != new_order[i] )
					{
						ind_change = i;
						i = curr_order.Len();
					}
				}

				// Found the change
				if( ind_change != -1 )
				{
					// Check that everything is really unchanged after the mod
					bool b_no_change_after = true;
					for( size_t i = ind_change; i < new_order.Len();i++ )
					{
						if( new_order[i] != curr_order[i+1] )
						{
							b_no_change_after = false;
							i = curr_order.Len();
						}
					}

					if( ! b_no_change_after )
					{
						wxMessageBox( wxT("More than one change at one time can't be handled !!\nChange after case...") );
						b_accept_change = false;
					}
					else
					{
						//Get the ind in map for the lower changed panel
						int ind_in_map = -1;
						bool b_suppress_original = false;
						for( int i = 0; i < tay_panels_map; i++ )
						{
							if( panels_map[i] >= ind_change )
							{
								if( panels_map[i] == ind_change )
									b_suppress_original = true;
								ind_in_map = i;
								i = tay_panels_map;
							}
						}

						// Problem
						if( ind_in_map == -1 )
						{
							wxMessageBox( wxT("BUG ??\nNo indice in map found for the suppression !!!" ));
							b_accept_change = false;
						}

						// normal situation
						else
						{
							//must remap all the upper indices to indices -1
							for( int i= ind_in_map; i < tay_panels_map; i++ )
							{
								// except if the panel have been deleted
								if( panels_map[i] != -1 )
									panels_map[i] = panels_map[i] - 1;
							}

							// Must Set "delete" to the ind_of_change
							if( b_suppress_original )
								panels_map[ind_in_map] = -1;
						}
					}
				}

				// End char Suppressed
				else
				{
					for( int i = tay_panels_map-1; i >= 0; i-- )
					{
						if( (size_t) panels_map[i] >= new_order.Len() )
							panels_map[i] = -1;
					}
				}
			}

			// Replacement case
			else
			{
				// Check that there is only one change
				int ind_change = -1;
				for( size_t i = 0; i < curr_order.Len(); i++)
				{
					if( curr_order[i] != new_order[i] )
					{
						if( ind_change == -1 && ind_change != -2 )
						{
							ind_change = i;
						}
						else
						{
							ind_change = -2;
							i = curr_order.Len();
						}
					}
				}

				// Too much changes
				if( ind_change == -2 )
				{
					wxMessageBox( wxT("More than one change at one time can't be handled !!\nChange after case..." ));
					b_accept_change = false;
				}

				// one change -> must update the map
				else if( ind_change >= 0 )
				{
					//Get the ind in map for the changed panel
					int ind_in_map = -1;
					for( int i = 0; i < tay_panels_map; i++ )
					{
						if( panels_map[i] == ind_change )
						{
							ind_in_map = i;
							i = tay_panels_map;
						}
					}

					if( ind_in_map != -1 )
					{
						// the changed panel implie deletion of the older
						panels_map[ind_in_map] = -1;
					}
				}
			}
			// END update the map

		// Update the panel map
		if( b_accept_change )
		{
			curr_order = new_order;
			Update_PanelsView();
		}
		else
		{
			txtCtrl_order->ChangeValue( curr_order );
		}
	}

	UpdateInsertPoint();
	evt.Skip();
}


//****************************************************

void myWnd_EditPanelOrder::Evt_Paint_PanelView( wxPaintEvent& evt )
{
	wxAutoBufferedPaintDC dc( panel_stage_view );
	panel_stage_view->DoPrepareDC( dc );
//	dc.SetBackground( *wxWHITE_BRUSH );
	dc.Clear();

	if( panel_cursor_x <= 0 )
		return;

	wxSize s = panel_stage_view->GetSize();
	dc.SetBrush( *wxRED_BRUSH );
//	int dummy = 0;
//	panel_stage_view->CalcScrolledPosition( panel_cursor_x, dummy, &panel_cursor_x, &dummy );
		dc.DrawRectangle(  panel_cursor_x, 5, 4, s.y - 10 );
}


//****************************************************

void myWnd_EditPanelOrder::UpdateInsertPoint()
{
	//Get the current insertion point
	curr_ind = txtCtrl_order->GetInsertionPoint();
	MoveImagesCursor( curr_ind );
}


//****************************************************

int myWnd_EditPanelOrder::Get_OrderElt_X( int ind)
{
	if( ind >= img_ctrl_list_count || ind < 0 )
		return -1;

	wxPoint p = img_ctrl_list[ind]->GetPosition();
	panel_stage_view->CalcUnscrolledPosition( p.x, p.y, &p.x, &p.y );

	return p.x;
}


//****************************************************

const int dec_cursor = 3;
void myWnd_EditPanelOrder::MoveImagesCursor( long i )
{
	panel_cursor_x = 0;
	int scrollU_x, scrollU_y;
	int scroll_pos;
	panel_stage_view->GetScrollPixelsPerUnit(&scrollU_x, &scrollU_y);
	wxSize v_s = 	panel_stage_view->GetVirtualSize();
	wxSize r_s = 	panel_stage_view->GetSize();

	if( b_reverted_direction )
		i = curr_order.Len() - i;
	
	if( curr_order.Len() <= 0 || (size_t) i >= curr_order.Len() )
	{
		panel_cursor_x = -30;
		scroll_pos = v_s.x / scrollU_x;
	}
	else if( i == 0 )
	{
		panel_cursor_x = -30;
		scroll_pos = 0;
	}
	else
	{
		panel_cursor_x = Get_OrderElt_X( i ) - dec_cursor * 2.5;
		scroll_pos = panel_cursor_x - r_s.x / 2;
		if( scroll_pos < 0)
			scroll_pos = 0;
	}

	panel_stage_view->Scroll( scroll_pos / scrollU_x, -1 );
	panel_stage_view->Refresh();
//	wxSizeEvent dummy;
//	panel_stage_view->EvtSize( dummy );
}


//****************************************************

void myWnd_EditPanelOrder::Update_PanelsView()
{
	wxSizer* panel_stage_view_sizer = panel_stage_view->GetSizer();
	panel_stage_view->Freeze();

	// Remove old images ctrls
	if( img_ctrl_list != NULL )
	{
		delete[] img_ctrl_list;
		img_ctrl_list = NULL;
		img_ctrl_list_count = 0;
	}

	wxString order;
	size_t max_letter = (size_t) ('a' + nb_panels );
	for( size_t i = 0; i < curr_order.Len(); i++ )
	{
		size_t curr_letter = (size_t) curr_order[i];
		if( curr_letter >= (size_t) 'a' && curr_letter < max_letter )
			order += curr_order[i];
	}

	if( order.Len() > 0)
	{
		img_ctrl_list = new MyImageCtrl*[order.Len()];
		img_ctrl_list_count = order.Len();
	}

	panel_stage_view_sizer->Clear( true );

	for( size_t i = 0; i < order.Len(); i++ )
	{
		wxBoxSizer* sizer_temp = new wxBoxSizer( wxVERTICAL );

		MyImageCtrl* t_imgctrl = new MyImageCtrl( panel_stage_view );
		
		// With vertical Levels  ==>>  rotate the stuff
		if( b_rotate_imgs == true )
			t_imgctrl->Rotate90( 3 );
		int ind = b_reverted_direction ? order.Len() - i - 1 : i;
		
		t_imgctrl->SetId( wxID_HIGHEST + i );
		t_imgctrl->Set_BoundDimensions( max_img_size, max_img_size );
		t_imgctrl->SetImage( l_panels[order[ind]-'a']->GetImage() );
		t_imgctrl->Connect( wxEVT_LEFT_UP, wxMouseEventHandler(myWnd_EditPanelOrder::Evt_Click_OrderElt), NULL, this);
		sizer_temp->Add( t_imgctrl, 0, wxALL, 1 );
		img_ctrl_list[i] = t_imgctrl;

		wxStaticText* t_stat = new wxStaticText( panel_stage_view, wxID_ANY, wxChar( order[ind] ),
				wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		t_stat->SetFont( font_Alphabet );
		t_stat->SetMinSize( wxSize( 40, wxDefaultCoord ));
		sizer_temp->Add( t_stat , 0, wxCENTER |wxALL, 1 );

		panel_stage_view_sizer->Add( sizer_temp, 0, wxEXPAND |wxALL, imgs_space );
	}

	panel_stage_view->Layout();
	wxSizeEvent dummy;
	panel_stage_view->EvtSize(dummy );

	UpdateInsertPoint();
	panel_stage_view->Thaw();
	panel_stage_view->Refresh();
}


//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
// Event table
BEGIN_EVENT_TABLE(myWnd_EditPanelOrder, wxDialog)
	EVT_CLOSE(myWnd_EditPanelOrder::EvtClose)
	EVT_SIZE(myWnd_EditPanelOrder::EvtSize)
//	EVT_PAINT(myWnd_EditPanelOrder::Evt_Paint_PanelView)
END_EVENT_TABLE()
