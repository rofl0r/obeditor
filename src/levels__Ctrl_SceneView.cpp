#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include <wx/dcbuffer.h>

#include "common__mod.h"
#include "common__ob_controls.h"

#include "levels__globals.h"
#include "levels__Ctrl_SceneView.h"

using namespace std;
//****************************************************************************
const float FRONT_LAYER_RATIO = 1.4;
float MAX_ZOOM_FACTOR = 6;
float MIN_ZOOM_FACTOR = 0.2;
int	DECAL_UNIT = 10;
int	KBUTS_TIMES = 50;


//****************************************************************************
//----------------------------------------------------------------------------
Panel_SceneView::Panel_SceneView( 
			  wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			)
:wxPanel( _parent, wxID_ANY )
{
	// Now, the Controls
	wxSizer* main_sizer = new wxBoxSizer( wxVERTICAL );
	
	ctrlSceneView = new Ctrl_SceneView( this );
	main_sizer->Add( ctrlSceneView, 1, wxEXPAND | wxALL, 4 );
	
	wxSizer* t_sizer  = new wxBoxSizer( wxHORIZONTAL );
	main_sizer->Add( t_sizer, 0, wxEXPAND );
	
	t_sizer->AddStretchSpacer();
	
	wxButton* t_btn = new wxBitmapButton( this, wxID_ANY
	,wxBitmap(GetRessourceFile_String(wxT("zoom-out.png"))) );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_SceneView::Evt_ZoomM)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );
	
	t_btn = new wxBitmapButton( this, wxID_ANY
	,wxBitmap(GetRessourceFile_String(wxT("zoom-in.png"))) );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_SceneView::Evt_ZoomP)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );

	t_sizer->AddSpacer( 30 );

	t_btn = new KalachnikofButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("ArrowUp.png")))), KBUTS_TIMES );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_SceneView::Evt_MoveUp)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );

	t_btn = new KalachnikofButton( this, wxID_ANY 
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("ArrowDown.png")))), KBUTS_TIMES );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_SceneView::Evt_MoveDown)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );

	t_btn = new KalachnikofButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("ArrowLeft.png")))), KBUTS_TIMES );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_SceneView::Evt_MoveLeft)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );

	t_btn = new KalachnikofButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("ArrowRight.png")))), KBUTS_TIMES );
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_SceneView::Evt_MoveRight)
		, NULL, this);
	t_sizer->Add( t_btn, 0, wxALL, 3 );

	t_sizer->AddStretchSpacer();
	
	this->SetSizer( main_sizer );

	// Init view Control 's variables
	ctrlSceneView->stage = _stage;
	ctrlSceneView->stage_declaration = _stage_declaration;
	ctrlSceneView->stage_set = _stage_set;

	ctrlSceneView->p_nb_layers = NULL;
	ctrlSceneView->p_bg_layers = NULL;
	ctrlSceneView->p_nb_panels = NULL;
	ctrlSceneView->p_panels = NULL;
	ctrlSceneView->p_nb_front_panels = NULL;
	ctrlSceneView->p_front_panels = NULL;
	
}
			

//----------------------------------------------------------------------------
Panel_SceneView::~Panel_SceneView()
{
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void 
Panel_SceneView::Evt_ZoomM( wxCommandEvent& evt )
{
	ctrlSceneView->Zoom_More();
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Evt_ZoomP( wxCommandEvent& evt )
{
	ctrlSceneView->Zoom_Less();
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Evt_MoveUp(   wxCommandEvent& evt )
{
	ctrlSceneView->MoveUp();
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Evt_MoveDown( wxCommandEvent& evt )
{
	ctrlSceneView->MoveDown();
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Evt_MoveLeft(   wxCommandEvent& evt )
{
	ctrlSceneView->MoveLeft();
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Evt_MoveRight( wxCommandEvent& evt )
{
	ctrlSceneView->MoveRight();
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Update_View()
{
	ctrlSceneView->Update_View();
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Set_Panels( ob_stage_panel**& p_panels, size_t& p_nb_panels )
{
	ctrlSceneView->p_panels = &p_panels;
	ctrlSceneView->p_nb_panels = &p_nb_panels;
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Set_Background( ob_BG_Layer**& p_bg_layers, size_t& p_nb_layers )
{
	ctrlSceneView->p_nb_layers = &p_nb_layers;
	ctrlSceneView->p_bg_layers = &p_bg_layers;
}

//----------------------------------------------------------------------------
void 
Panel_SceneView::Set_Front_Panels(ob_front_panel**& p_front_panels, size_t& p_nb_front_panels )
{
	ctrlSceneView->p_front_panels = &p_front_panels;
	ctrlSceneView->p_nb_front_panels = &p_nb_front_panels;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

Ctrl_SceneView::Ctrl_SceneView( Panel_SceneView* _parent)
:wxControl( _parent, wxID_ANY )
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	stage = NULL;
	stage_declaration = NULL;
	stage_set = NULL;

	// Regular Panels
	p_nb_panels = NULL;
	p_panels = NULL;
	
	// Background layers
	p_nb_layers = NULL;
	p_bg_layers = NULL;
	
	// Front panels
	p_nb_front_panels = NULL;
	p_front_panels = NULL;

	bool b_dummy;
	zoom_factor = StrToFloat( ConfigEdit_Read(wxT("Ctrl_SceneView/zoom_factor"), wxT("1") ) , b_dummy);
	x_decal = 0;
	y_decal = 0;
	level_start_x = 0;
	last_panel_w = 0;
}

//----------------------------------------------------------------------------
Ctrl_SceneView::~Ctrl_SceneView()
{
	map<string,wxImage*>::iterator it;
	while( h_imgs.empty() == false)
	{
		it = h_imgs.begin();
		delete it->second;
		h_imgs.erase( it );
	}
}

//----------------------------------------------------------------------------
void
Ctrl_SceneView::RezoomUpdate(float old_zoom_factor)
{
//	float rezoom_factor = zoom_factor / old_zoom_factor;
//	x_decal *= rezoom_factor;
//	y_decal *= rezoom_factor;
}

//----------------------------------------------------------------------------
void 
Ctrl_SceneView::Zoom_Less()
{
	if( zoom_factor >= MAX_ZOOM_FACTOR )
		return;
	float old_zoom_factor = zoom_factor;
	
	zoom_factor *= 1.3;
	if( zoom_factor >= MAX_ZOOM_FACTOR )
		zoom_factor = MAX_ZOOM_FACTOR;
	
	RezoomUpdate( old_zoom_factor );
	
	// Rescale every images
	Rescale_All_Images();
	
	// Refresh the stuff
	Refresh();
}

//----------------------------------------------------------------------------
void 
Ctrl_SceneView::Zoom_More()
{
	if( zoom_factor <= MIN_ZOOM_FACTOR )
		return;
	float old_zoom_factor = zoom_factor;
	
	zoom_factor *= 0.7;
	if( zoom_factor <= MIN_ZOOM_FACTOR )
		zoom_factor = MIN_ZOOM_FACTOR;

	RezoomUpdate( old_zoom_factor );
	
	// Rescale every images
	Rescale_All_Images();
	
	// Refresh the stuff
	Refresh();
}

//----------------------------------------------------------------------------
void 
Ctrl_SceneView::MoveDown()
{
	y_decal -= DECAL_UNIT / zoom_factor;
	Refresh();
}

//----------------------------------------------------------------------------
void 
Ctrl_SceneView::MoveUp()
{
	y_decal += DECAL_UNIT / zoom_factor;
	Refresh();
}

//----------------------------------------------------------------------------
void 
Ctrl_SceneView::MoveLeft()
{
	x_decal += DECAL_UNIT / zoom_factor;
	Refresh();
}

//----------------------------------------------------------------------------
void 
Ctrl_SceneView::MoveRight()
{
	x_decal -= DECAL_UNIT / zoom_factor;
	Refresh();
}

//----------------------------------------------------------------------------
void 
Ctrl_SceneView::Update_View()
{
	bool b_show_panels = (p_nb_panels != NULL);
	bool b_show_background = (p_nb_layers != NULL);
	bool b_show_front_panels = (p_nb_front_panels != NULL);
	
	if(  ( !b_show_panels && !b_show_background 
		&& !b_show_front_panels )
		|| stage == NULL 
	  )
	{
		// Nothing to show
		Refresh();
	}
	
	Update_Panels_Images();
	Update_Bgs_Images();
	Update_FrontPanels_Images();
	Refresh();
}

//----------------------------------------------------------------------------
wxImage*
Ctrl_SceneView::Get_Img( const wxString& imgPath )
{
	map<string,wxImage*>::iterator it = h_imgs.find( (char*)imgPath.c_str() );
	if( it == h_imgs.end() )
		return NULL;
	return it->second;
}

//----------------------------------------------------------------------------
bool
Ctrl_SceneView::ResizeImg_with_ZoomFactor( wxImage*& theImg )
{
	if( theImg == NULL || theImg->IsOk() == false )
		return false;
	theImg->Rescale(	  theImg->GetWidth() * zoom_factor
				, theImg->GetHeight()* zoom_factor );
	return true;
}

//----------------------------------------------------------------------------
// Return True if the image is new
bool
Ctrl_SceneView::Check_ImgExists( const wxString& imgPath )
{
	map<string,wxImage*>::iterator it = h_imgs.find( string((char*)imgPath.c_str()) );
	if( it != h_imgs.end() )
		return false;
	
	wxImage* img = new wxImage( imgPath );
	if( img->IsOk() == false )
	{
		delete img;
		return false;
	}
	
	unsigned long int _rgbs = 0;
	unsigned char* p_rgbs = (unsigned char*) &_rgbs;
	wxPalette palette = img->GetPalette();
	palette.GetRGB(0, p_rgbs+1,p_rgbs+2,p_rgbs+3);
	
	ResizeImg_with_ZoomFactor( img );
	h_imgs[string((char*)imgPath.c_str())] = img;
	h_masques[(char*)imgPath.c_str()] = _rgbs;
	return true;
}

//----------------------------------------------------------------------------
// Return True if the image is new
bool
Ctrl_SceneView::Rescale_All_Images()
{
	map<string,wxImage*>::iterator it(h_imgs.begin()),
						it_end(h_imgs.end());
	for(; it != it_end; it++ )
	{
		if( it->second == NULL )
			continue;
		delete it->second;
		
		wxImage* img = new wxImage( wxString::FromAscii(it->first.c_str()) );
		if( img->IsOk() == false )
		{
			delete img;
			h_imgs[it->first] = NULL;
			continue;
		}
		ResizeImg_with_ZoomFactor( img );
		it->second = img;
	}
	
	return true;
}


//----------------------------------------------------------------------------
bool 
Ctrl_SceneView::Update_Panels_Images()
{
	if( p_nb_panels == NULL || *p_nb_panels <= 0) 
		return false;
	
	int old_level_start_x = level_start_x;
	level_start_x = 0;
	int direction = stage->direction;
	
	
	for( size_t i = 0; i < *p_nb_panels; i++ )
	{
		// Update the images DB
		Check_ImgExists( (*p_panels)[i]->GetFileName().GetFullPath() );
		
		// Update the start offset
		if( direction == STAGE_LEFT )
		{
			wxImage* _img = Get_Img( (*p_panels)[i]->GetFileName().GetFullPath() );
			if( _img != NULL && _img->IsOk() )
			{
				if( i + 1 < *p_nb_panels )
				{
						level_start_x += _img->GetWidth();
				}
				else
				{
					if( last_panel_w == 0 )
						x_decal += x_decal;
					last_panel_w = _img->GetWidth();
				}
			}
		}
	}
	if( direction == STAGE_LEFT )
	{
		x_decal -= level_start_x - old_level_start_x;
	}
	
	return true;
}

//----------------------------------------------------------------------------
bool 
Ctrl_SceneView::Update_Bgs_Images()
{
	if( p_nb_layers == NULL || *p_nb_layers <= 0) 
		return false;
	
	for( size_t i = 0; i < *p_nb_layers; i++ )
		Check_ImgExists( (*p_bg_layers)[i]->GetFileName().GetFullPath() );
	return true;
}

//----------------------------------------------------------------------------
bool 
Ctrl_SceneView::Update_FrontPanels_Images()
{
	if( p_nb_front_panels == NULL || *p_nb_front_panels <= 0
	  )
		return false;
	
	for( size_t i = 0; i < *p_nb_front_panels; i++ )
		Check_ImgExists( (*p_front_panels)[i]->GetFileName().GetFullPath() );
	return true;
}

//----------------------------------------------------------------------------
void
Ctrl_SceneView::pDrawImg( wxDC& theDC, const string& imgPath, wxImage* theImg, int x, int y, bool b_trans )
{
	if( b_trans == true )
	{
		if( theImg->HasMask() == false )
		{
			unsigned long int masque = h_masques[imgPath];
			unsigned char* p_m = (unsigned char*) &masque;
			theImg->SetMaskColour( p_m[1],p_m[2],p_m[3] );
			theImg->SetMask( true );
		}
	}
	else
		theImg->SetMask( false );
	
	theDC.DrawBitmap( wxBitmap(*theImg), x*zoom_factor,y*zoom_factor, b_trans );
}


//----------------------------------------------------------------------------
void 
Ctrl_SceneView::Evt_Paint( wxPaintEvent& evt )
{
	wxAutoBufferedPaintDC theDC( this );
		
	bool b_show_panels = (p_nb_panels != NULL && *p_nb_panels>0);
	bool b_show_background = (p_nb_layers != NULL&& *p_nb_layers > 0);
	bool b_show_front_panels =(p_nb_front_panels != NULL && *p_nb_front_panels >0);
	
	if(  ( !b_show_panels && !b_show_background && !b_show_front_panels )
		|| stage == NULL 
		|| stage_declaration == NULL
		|| stage_set == NULL
	  )
	{
		// Nothing to show
		wxString img_txt = wxT("Nothing to show !");
		theDC.SetBrush( *wxWHITE_BRUSH );
		theDC.SetPen( *wxBLACK_PEN );
		theDC.Clear();
		theDC.SetFont( 
			wxFont( 20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true )
			);
		theDC.DrawText( img_txt, 100,100);
		return;
	}

	//----------------------
	// Fill Background
	theDC.SetBrush( *wxBLACK_BRUSH );
	theDC.Clear();

	//***********************************************************************
	// Compute the current regular pannels view boundary
	int win_W, win_H;
	GetSize( &win_W, &win_H );
	
	int client_w = win_W / zoom_factor;
	int client_h = client_w * win_H / win_W;
	int direction = stage->direction;

	//***********************************************************************
	int first_panel_end_x = client_w;
	// Backgrounds
	if( b_show_background == true )
	{
		for( size_t i = 0; i < *p_nb_layers; i++)
		{
			ob_BG_Layer* curr = (*p_bg_layers)[i];
			if( curr == NULL )
				continue;
			
			int xrepeat  =  curr->Get_RepeatingVal( STAGE_RIGHT );
			int zrepeat  =  curr->Get_RepeatingVal( STAGE_UP );

			// If the layer have to NOT be displayed
			if( 	   xrepeat == 0
				|| zrepeat == 0 )
				continue;
			
			wxString imgPath = curr->GetFileName().GetFullPath();
			wxImage* theImg = Get_Img( imgPath );
			if( theImg == NULL )
				continue;
			
			// Get all the var from the object
			int p =1;
			float xratio = StrToFloat( curr->GetToken(p));
			p++;
			float zratio = StrToFloat( curr->GetToken(p));
			p++;
			int   xpos   = StrToInt(   curr->GetToken(p));
			p++;
			int   ypos   = StrToInt(   curr->GetToken(p));
			p++;
			int   xspace = StrToInt(   curr->GetToken(p));
			p++;
			int   zspace = StrToInt(   curr->GetToken(p));
			p++;
			p++; // xrepeat
			p++; // zrepeat
			bool b_trans =  curr->GetToken(p) == wxT("1");
			p++;
//			int  alpha   =  StrToInt( curr->GetToken(p));
			p++;
//			int  wMode   =  StrToInt( curr->GetToken(p));
			p++;
//			int  wAmp    =  StrToInt( curr->GetToken(p));
			p++;
//			int  wLen    =  StrToInt( curr->GetToken(p));
			p++;
//			int  wSpeed  =  StrToInt( curr->GetToken(p));

			if( curr->name.Upper() == wxT("BACKGROUND") )
				xratio = zratio = 0.5;

			int img_w = theImg->GetWidth();
			int img_h = theImg->GetHeight();
			int bg_w = img_w / zoom_factor + xspace;
			int bg_h = img_h / zoom_factor + zspace;
			
			int bg_x = xpos + x_decal * xratio;
			int bg_y = ypos + y_decal * zratio;
			
			bool b_start_drawing = false;
			
			if( i == 0 )
				first_panel_end_x = bg_w;
			
			// Compute the indice repeats intervals of drawable imgs
			int start_k = 0;
			int end_k = 0;
			int start_j = 0;
			int end_j = 0;
			
			if( direction == STAGE_UP )
			{
				if( bg_x > client_w || bg_x + img_w < 0 )
					continue;
				start_j = bg_y / bg_h;
				if( (bg_y % bg_h) > zspace )
					start_j++;
				if( start_j >= zrepeat )
					start_j = zrepeat - 1;
				start_j = -start_j;
				end_j = 0;
			}
			else if( direction == STAGE_DOWN )
			{
				if( bg_x > client_w || bg_x + img_w < 0 )
					continue;
				start_j = 0;
				end_j = (-bg_y + client_h) / bg_h + 1;
				if( end_j >= zrepeat )
					end_j = zrepeat - 1;
			}
/*
			else if( direction == STAGE_LEFT )
			{
				if( bg_y > client_h || bg_y + img_h < 0 )
					continue;
				start_k = bg_x / bg_w;
				if( (bg_x % bg_w) > xspace )
					start_k++;
				if( start_k >=  xrepeat  )
					start_k =  xrepeat - 1;
				start_k = -start_k;
				end_k = 0;
			}
*/
			else// if( direction == STAGE_RIGHT || IN || OUT || LEFT
			{
				if( bg_y > client_h || bg_y + img_h < 0 )
					continue;
				start_k = 0;
				end_k = (-bg_x + client_w)/ bg_w +1;
				if( end_k >= xrepeat )
					end_k =  xrepeat - 1;
			}


			bg_x += start_k * bg_w;
			bg_y += start_j * bg_h;
			
			for( int j = start_j; j <= end_j; j++ )
			{
				if( bg_y > client_h )
					break;

				int t_bg_x = bg_x;
				for( int k = start_k; k <= end_k; k++ )
				{
					if(  t_bg_x > client_w )
						break;
					
					pDrawImg(theDC, string((char*)imgPath.c_str()), theImg, t_bg_x, bg_y, b_trans );
					b_start_drawing = true;
					
					t_bg_x += bg_w;
				}
				bg_y += bg_h;
			}
		}
	}
	
	int old_y_decal = y_decal;
	if( direction == STAGE_UP || direction == STAGE_DOWN )
		y_decal = 0;

	//***********************************************************************
	// Panels
	if( b_show_panels == true )
	{
		int x_offset = x_decal;
		int y_offset = y_decal;
		bool b_draw_start = false;
		for( size_t ind_p = 0; ind_p < *p_nb_panels; ind_p++)
		{
			ob_stage_panel* curr = (*p_panels)[ind_p];
			if( curr == NULL )
				continue;
			
			wxString imgPath = curr->GetFileName().GetFullPath();
			wxImage* theImg = Get_Img( imgPath );
			if( theImg == NULL )
				continue;
			int curr_img_w = theImg->GetWidth();
			int curr_img_h = theImg->GetHeight();
			int img_w = curr_img_w / zoom_factor;
			int img_h = curr_img_h / zoom_factor;

/*
			// Some fix in LEFT direction stage
			if( ind_p == 0 && direction == STAGE_LEFT )
				x_offset += first_panel_end_x- img_w;
*/

			if( x_offset < client_w && x_offset > -img_w
				&&
				y_offset < client_h && y_offset > -img_h
				)
			{
				pDrawImg(theDC, string((char*)imgPath.c_str()), theImg, x_offset, y_offset, true );
				b_draw_start = true;
			}
			// Everything have been drawn
			else if( b_draw_start == true )
				break;
			
/*			
			if( direction == STAGE_UP )
			{
				y_offset -= img_h;
			}
			else if( direction == STAGE_DOWN )
			{
				y_offset += img_h;
			}
			else if( direction == STAGE_LEFT )
			{
				x_offset -= img_w;
			}
			else// if( direction == STAGE_RIGHT || IN || OUT || LEFT
			{
*/
				x_offset += img_w;
//			}
		}
	}
	
	//***********************************************************************
	// Front Panels
	if( b_show_front_panels == true )
	{
		int x_offset = x_decal * FRONT_LAYER_RATIO;
		int draw_state = 0;
		
		while( draw_state != 2 )
		{
			bool b_something_drawn = false;
			
			for( size_t ind_p = 0; ind_p < *p_nb_front_panels; ind_p++)
			{
				ob_front_panel* curr = (*p_front_panels)[ind_p];
				if( curr == NULL )
					continue;
				
				wxString imgPath = curr->GetFileName().GetFullPath();
				wxImage* theImg = Get_Img( imgPath );
				if( theImg == NULL )
					continue;
				
				int curr_img_w = theImg->GetWidth();
//				int curr_img_h = theImg->GetHeight();
				int img_w = curr_img_w / zoom_factor;
//				int img_h = curr_img_h / zoom_factor;

/*
				// Some fix in LEFT direction stages
				if( 	   direction == STAGE_LEFT 
					&& ind_p == 0 
					&& draw_state == 0
					)
					x_offset += first_panel_end_x - img_w;
*/				
				if( x_offset > -img_w && x_offset < client_w )
				{
					b_something_drawn = true;
					pDrawImg(theDC, string((char*)imgPath.c_str()), theImg, x_offset, y_decal, true );
					draw_state = 1;
				}
					
				// Everything have been drawn
				else if( draw_state == 1 )
				{
					draw_state = 2;
					break;
				}
/*				
				if( direction == STAGE_LEFT )
				{
					x_offset -= img_w;
				}
				
				else// if( direction == STAGE_RIGHT || IN || OUT || STAGE_LEFT
				{
*/					
					x_offset += img_w;
//				}
			}
			
			if( 	(
					b_something_drawn == false 
					&& (direction == STAGE_DOWN || direction == STAGE_UP)
				)
				||
				(
					b_something_drawn == false 
					&& x_offset > 0
				)
			  )
				draw_state = 2;
		}
	}

	//***********************************************************************
	// Borders of the current view
	// FIXME
// 	theDC.SetPen( wxPen( *wxBlUE, 2 ));
// 	theDC.SetBrush( *wxTRANSPARENT_BRUSH );
// 	theDC.DrawRectangle( 

	y_decal = old_y_decal;
}


//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(Ctrl_SceneView, wxControl)
	EVT_PAINT(Ctrl_SceneView::Evt_Paint)
END_EVENT_TABLE()
