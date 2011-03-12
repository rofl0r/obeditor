#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dcmemory.h>
#include <wx/dcbuffer.h>

#include "common__mod.h"
#include "common__ob_controls.h"
#include "common__object_stage.h"
#include "images__MyPalette.h"

#include "ob_editor.h"
#include "levels__globals.h"
#include "levels__classes.h"
#include "levels__sidesWindows.h"
#include "levels__CtrlStageView.h"
#include "levels__Panel_StageView.h"
#include "levels__Ob_GObjects.h"
#include "levels__ObImgs_Manager.h"

using namespace std;

//-------------------------------------------------------
extern LevelEditorFrame *leFrame;
//-------------------------------------------------------
float MAP_ZF = 0.5;
size_t SCENE_MAP_HEIGHT= 150;

//-------------------------------------------------------
wxBitmap* 
Build_All_Panels_Img( ob_stage_panel** oPanels, 
			    size_t oPanels__size,
			    wxColour& background_color,
			    bool  b_transparent
			    )
{
	if( oPanels__size == 0 )
		return new wxBitmap(*noImg);

	//-------------------------------------------------
	// Compute panels_size
	wxSize panels_size(0,0);
	wxSize min_size(6000,6000);
	size_t nb_invalids = 0;

	for( size_t i = 0; i < oPanels__size; i++)
	{
		ob_stage_panel* curr = oPanels[i];
		if( curr == NULL )
		{
			nb_invalids++;
			continue;
		}
		
		wxString imgPath = curr->GetFileName().GetFullPath();
		wxImage theImg( imgPath );
		if( theImg.IsOk() == false )
		{
			nb_invalids++;
			continue;
		}
		
		// total w
		panels_size.x += theImg.GetWidth();
		// total H
		if( theImg.GetHeight() > panels_size.y )
			panels_size.y  = theImg.GetHeight();
		// Min Size
		if( theImg.GetWidth() < min_size.x )
			min_size.x = theImg.GetWidth();
		if( theImg.GetHeight() < min_size.y )
			min_size.y = theImg.GetHeight();
	}
	panels_size.x +=  nb_invalids * min_size.x;
	
	//-----------------------------------
	// Build Invalid Panel Image
	wxImage* img_invalid_panel = NULL;
	if( nb_invalids > 0 )
	{
		img_invalid_panel = new wxImage(*noImg);
		// Only invalid Panels !!
		if( nb_invalids == oPanels__size )
		{
			panels_size = wxSize( 240, 300 );
			img_invalid_panel->Rescale( 240, 300 );
		}
		else
			img_invalid_panel->Rescale( min_size.x, min_size.y );
	}


	//-----------------------------------
	// Init the dc and Bitmap
	wxMemoryDC dc;
	wxBitmap* theBitmap = new wxBitmap(panels_size.x,panels_size.y);
	dc.SelectObject( *theBitmap );
	dc.SetBackground(background_color);
	dc.Clear();
	
	//-----------------------------------
	// Draw the panels in the Bitmap
	int curr_x = 0;
	for( size_t i = 0; i < oPanels__size; i++)
	{
		ob_stage_panel* curr = oPanels[i];
		bool b_invalid = false;
		if( curr == NULL )
			b_invalid = true;
		
		wxImage* theImg = NULL;
		if(b_invalid == false )
		{
			wxString imgPath = curr->GetFileName().GetFullPath();
			theImg = new wxImage( imgPath );
			if( theImg == NULL || theImg->IsOk() == false )
			{
				b_invalid = true;
				if( theImg != NULL )
					delete theImg;
			}
		}
		
		if( b_invalid == true )
			theImg = img_invalid_panel;
		
		wxBitmap t_bmp(*theImg);
		dc.DrawBitmap( t_bmp, curr_x, 0, b_transparent);
		curr_x += theImg->GetWidth();
		if( b_invalid == false )
			delete theImg;
	}
	
	if( img_invalid_panel != NULL )
		delete img_invalid_panel;
	dc.SelectObject( wxNullBitmap );
	return theBitmap;
}


//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
Panel_StageView::Panel_StageView(wxWindow* _parent )
:wxPanel( _parent, wxID_ANY )
{
	wxBoxSizer *sizer_main = new wxBoxSizer( wxVERTICAL );
	
	obImgs_Mgr = new ObImgs_Manager();
	coords_to_restore = wxSize(-100000,-100000);
	f_hidden = 0;

	wxWindow* t_btn;
//	MyPaletteElementCtrl* colourChooser;
	
	//--------------------------------
	// TOOLS
	wxBoxSizer *sizer_tools = new wxBoxSizer( wxHORIZONTAL );
	sizer_main->Add( sizer_tools, 0, wxEXPAND );
	
	t_btn = new wxBitmapButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("zoom-out.png"))) ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_StageView::Evt_ZoomM)
		, NULL, this);
	sizer_tools->Add( t_btn, 0, wxALL|wxCENTER, 3 );
	
	t_btn = new wxBitmapButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("zoom-x1.png"))) ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_StageView::Evt_ZoomReset)
		, NULL, this);
	sizer_tools->Add( t_btn, 0, wxALL|wxCENTER, 3 );
	
	t_btn = new wxBitmapButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("zoom-in.png"))) ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_StageView::Evt_ZoomP)
		, NULL, this);
	sizer_tools->Add( t_btn, 0, wxALL|wxCENTER, 3 );
	
	
	
	sizer_tools->AddSpacer( 5 );
	t_btn = new wxStaticText( this, wxID_ANY, wxT("BG : ") );
	sizer_tools->Add( t_btn, 0, wxALL|wxCENTER, 3 );
	
	bgColourChooser = new MyPaletteElementCtrl( this, AUTO_ELTCOLOR );
	bgColourChooser->Connect( wxPALETTE_ELT_COLOR_CHANGE
		, wxCommandEventHandler(Panel_StageView::Evt_BG_ColourChange)
		, NULL, this);
	sizer_tools->Add( bgColourChooser, 0, wxALL|wxCENTER, 3 );
	
	t_btn = new wxCheckBox( this, wxID_ANY, wxT("Hide") );
	t_btn->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED
		, wxCommandEventHandler(Panel_StageView::Evt_BG_VisibleChange)
		, NULL, this);
	sizer_tools->Add( t_btn, 0, wxALL|wxCENTER, 3 );

	
	
	sizer_tools->AddSpacer( 5 );
	
	t_btn = new wxBitmapButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("delete-small.png"))) ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_StageView::Evt_DeleteObjects)
		, NULL, this);
	sizer_tools->Add( t_btn, 0, wxALL|wxCENTER, 3 );

	t_btn = new wxBitmapButton( this, wxID_ANY
	,wxBitmap(wxImage(GetRessourceFile_String(wxT("copy-small.png"))) ));
	t_btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED
		, wxCommandEventHandler(Panel_StageView::Evt_DuplicateObjects)
		, NULL, this);
	sizer_tools->Add( t_btn, 0, wxALL|wxCENTER, 3 );

	
	//--------------------------------
	// VIEW CTRL
	stageMap = NULL;
	stageView = new StageView( this );
	sizer_main->Add( stageView, 1, wxEXPAND );
	
	//--------------------------------
	// MAP CTRL
	stageMap = new StageMap( this );
	stageMap->SetMinSize( wxSize(wxDefaultCoord, SCENE_MAP_HEIGHT) );
	stageMap->SetSize( wxSize(wxDefaultCoord, SCENE_MAP_HEIGHT) );
	sizer_main->Add( stageMap, 0, wxEXPAND|wxTOP|wxBOTTOM| wxFIXED_MINSIZE, 6 );
	stageMap->m_flag |= stageMap->M_SM_INIT;
	
	//--------------------------------
	// FINALLY
	bgColourChooser->SetColor( stageView->background_color );
	SetSizer( sizer_main );
}

//-------------------------------------------------------
Panel_StageView::~Panel_StageView()
{
	Reset();

	// Free the Panels Images Manager
	ObImgs_Manager* t = obImgs_Mgr;
	obImgs_Mgr = NULL;
	delete t;
}


//-------------------------------------------------------
void 
Panel_StageView::UpdateView()
{
	OBJECTS__Refresh();
}

//-------------------------------------------------------
void 
Panel_StageView::Refresh(bool eraseBackground, const wxRect* rect)
{
	wxPanel::Refresh(eraseBackground, rect);
//	stageMap->Refresh(eraseBackground, rect);
}

//-------------------------------------------------------
void
Panel_StageView::SaveState()
{
	ConfigEdit_Write(wxT("Panel_StageView/zoom_factor")
		, FloatToStr(stageView->zoom_factor ));
		ConfigEdit_Write(wxT("Panel_StageView/coords_x")
		, IntToStr(stageView->coords.x ));
		ConfigEdit_Write(wxT("Panel_StageView/coords_y")
		, IntToStr(stageView->coords.y ));
	
	ConfigEdit_Write(wxT("Panel_StageView/BG_color"),  wxColour_ToStr( stageView->background_color) );
}

//-------------------------------------------------------
void
Panel_StageView::LoadState()
{
	wxString s;
	s = ConfigEdit_Read(wxT("Panel_StageView/zoom_factor"));
	if( s != wxString() )
		stageView->zoom_factor = StrToFloat( s );
	s = ConfigEdit_Read(wxT("Panel_StageView/coords_x") );
	if( s != wxString() )
		coords_to_restore.x = StrToInt( s );
	s = ConfigEdit_Read(wxT("Panel_StageView/coords_y") );
	if( s != wxString() )
		coords_to_restore.y = StrToInt( s );
	
	s = ConfigEdit_Read(wxT("Panel_StageView/BG_color") );
	if( s != wxString() )
	{
		stageView->background_color = wxColour_FromStr(s);
		bgColourChooser->SetColor(stageView->background_color );
	}

	UpdateView();
}

//-------------------------------------------------------
void
Panel_StageView::Reset()
{
	obImgs_Mgr->Reset();
	stageView->Reset();
	stageMap->Reset();
}

//-------------------------------------------------------
void
Panel_StageView::Reload__ALL()
{
	obImgs_Mgr->Reset();
	PANELS__Reload();
	OBJECTS__Reload();
}

//-------------------------------------------------------
void
Panel_StageView::PANELS__Reload()
{
	obImgs_Mgr->Reset();
	stageView->Reload_Panels();
	stageMap->Reload_Panels();
	stageView->Refresh();
}

//-------------------------------------------------------
void
Panel_StageView::OBJECTS__Reload()
{
	obImgs_Mgr->Reset();
	stageView->Reset_Ob_GObjects();
	OBJECTS__Refresh();
}


//-------------------------------------------------------
void 
Panel_StageView::OBJECTS__Refresh()
{
	stageView->Refresh();
	stageMap->Refresh();
}

//-------------------------------------------------------
void 
Panel_StageView::Delete_GuiObject_of( ob_stage_object* o )
{
	stageView->Delete_GuiObject_of( o );
}

//-------------------------------------------------------
void
Panel_StageView::Evt_ZoomP(  wxCommandEvent& evt )
{
	stageView->ZoomP();
}

//-------------------------------------------------------
void
Panel_StageView::Evt_ZoomM(  wxCommandEvent& evt )
{
	stageView->ZoomM();
}

//-------------------------------------------------------
void
Panel_StageView::Evt_ZoomReset(  wxCommandEvent& evt )
{
	stageView->ZoomSet( 1 );
}

//-------------------------------------------------------
void
Panel_StageView::Evt_BG_ColourChange(  wxCommandEvent& evt )
{
	stageView->background_color = bgColourChooser->GetColor();
	OBJECTS__Refresh();
}

//-------------------------------------------------------
void
Panel_StageView::Evt_BG_VisibleChange(  wxCommandEvent& evt )
{
	if( f_hidden & FM_BACKGROUND )
		f_hidden &= ~FM_BACKGROUND;
	else
		f_hidden |= FM_BACKGROUND;
	OBJECTS__Refresh();
}

//-------------------------------------------------------
void
Panel_StageView::Evt_DeleteObjects(  wxCommandEvent& evt )
{
	leFrame->OBJECT__ListSelected__Delete();
}

//-------------------------------------------------------
void
Panel_StageView::Evt_DuplicateObjects(  wxCommandEvent& evt )
{
	leFrame->OBJECT__ListSelected__Duplicate();
}

//-------------------------------------------------------
void
Panel_StageView::EvtMouse( wxMouseEvent& evt )
{
	evt.Skip();
}

//-------------------------------------------------------
BEGIN_EVENT_TABLE(Panel_StageView, wxPanel)
	EVT_MOUSE_EVENTS( Panel_StageView::EvtMouse )
END_EVENT_TABLE()

//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
StageMap::StageMap(Panel_StageView* _parent )
:wxControl(_parent,wxID_ANY, wxDefaultPosition,wxDefaultSize,
	     wxBORDER_SIMPLE | wxHSCROLL | wxVSCROLL
		)
{
	b_must_reload_panels = true;
	m_flag = 0;
	psv = _parent;
	panelBitmap = NULL;
	coords = wxSize(0,0);
	b_refreshing = false;

	color_view_borders = wxColour( 0,255,0);
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	SetBackgroundColour( *wxWHITE );
	SCROLLBARS_Update();
}

//-------------------------------------------------------
StageMap::~StageMap()
{
	if( panelBitmap != NULL )
		delete panelBitmap;
	panelBitmap = NULL;
}

//-------------------------------------------------------
void 
StageMap::Reload_Panels()
{
	b_must_reload_panels = false;
	if( panelBitmap != NULL )
		delete panelBitmap;
	panelBitmap = NULL;

	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
		return;
	
	ob_stage* theStage = (ob_stage*) curr_stageFile->obj_container;
	size_t oPanels__size;
	ob_stage_panel** oPanels = theStage->Get_Panels( oPanels__size);
	if( oPanels == NULL )
		return;

	panelBitmap =
			Build_All_Panels_Img( oPanels, 
						oPanels__size,
						psv->stageView->background_color,
						false
						);
	delete[] oPanels;

	// Some image transformation
	wxImage t_img(  panelBitmap->ConvertToImage() );
	t_img.Rescale( t_img.GetWidth()  * MAP_ZF
			,  t_img.GetHeight() * MAP_ZF );
	delete panelBitmap;
	unsigned char* pixs = t_img.GetData();
	size_t nb_pixels_elts = t_img.GetWidth() * t_img.GetHeight() * 3;
	for( size_t i = 0; i<nb_pixels_elts; i+=3 )
	{
		wxImage::RGBValue _rgb(pixs[i],pixs[i+1],pixs[i+2]);
		wxImage::HSVValue hsv = t_img.RGBtoHSV(_rgb );
		hsv.saturation /= 3;
		hsv.value /= 2;
		_rgb = t_img.HSVtoRGB( hsv);
		pixs[i] = _rgb.red;
		pixs[i+1] = _rgb.green;
		pixs[i+2] = _rgb.blue;
	}

	panelBitmap = new wxBitmap( t_img );
}

//-------------------------------------------------------
void 
StageMap::SCROLLBARS_Update()
{
	if( panelBitmap == NULL )
	{
		coords = wxSize(0,0);
		SetScrollbar( wxHORIZONTAL,0,0,0 );
		SetScrollbar( wxVERTICAL,  0,0,0 );
		return;
	}
	
	wxSize c(GetClientSize());
	c.x /= MAP_ZF;
	c.y /= MAP_ZF;
	wxSize max_size(psv->coords_max.x - psv->coords_min.x
			   ,psv->coords_max.y - psv->coords_min.y );
	
	if( max_size.x <= c.x )
	{
		SetScrollbar( wxHORIZONTAL,0,0,0 );
		coords.x = (max_size.x - c.x)/2 + psv->coords_min.x;
		Refresh();
	}
	else
		SetScrollbar( wxHORIZONTAL, coords.x - psv->coords_min.x
				, c.x, max_size.x );
	
	if( max_size.y <= c.y )
	{
		SetScrollbar( wxVERTICAL,0,0,0 );
		coords.y = (max_size.y - c.y)/2+ psv->coords_min.y;
		Refresh();
	}
	else
		SetScrollbar(  wxVERTICAL, coords.y - psv->coords_min.y
				 , c.y, max_size.y );
}


//-------------------------------------------------------
void 
StageMap::Reset()
{
}

//-------------------------------------------------------
void
StageMap::EvtSize( wxSizeEvent& evt )
{
	evt.Skip();
	SCROLLBARS_Update();
	if( m_flag & M_SM_INIT )
		SCENE_MAP_HEIGHT = GetClientSize().GetHeight();
	Refresh();
}

//-------------------------------------------------------
void
StageMap::EvtScroll( wxScrollWinEvent& evt )
{
	if( evt.GetOrientation() == wxHORIZONTAL )
	{
		SetScrollPos( wxHORIZONTAL, evt.GetPosition() );
		coords.x = GetScrollPos( wxHORIZONTAL ) + psv->coords_min.x;
	}
	else //GetOrientation() == wxVERTICAL
	{
		SetScrollPos( wxVERTICAL, evt.GetPosition() );
		coords.y = GetScrollPos( wxVERTICAL ) + psv->coords_min.y;
	}
	
	Refresh();
}

//-------------------------------------------------------
void
StageMap::Refresh(bool eraseBackground, const wxRect* rect)
{
	if( b_refreshing == true )
		return;

	b_refreshing = true;
	wxControl::Refresh( eraseBackground,rect );
//	GetParent()->Refresh(eraseBackground,rect);
	b_refreshing = false;
}

//-------------------------------------------------------
void
StageMap::EvtMouse( wxMouseEvent& evt )
{
	wxSize mCoords(evt.m_x/MAP_ZF,evt.m_y/MAP_ZF);
	wxSize viewSize(psv->stageView->Get_ObSize());
	mCoords.x += coords.x - viewSize.x/2;
	mCoords.y += coords.y - viewSize.y/2;
	if( 
		(evt.Dragging() == true && evt.m_leftDown == true )
		||
		evt.LeftDown() == true
		
	  )
	{
		psv->stageView->SetViewCoords( mCoords );
	}

	if( evt.GetWheelRotation() != 0 )
	{
		int i = evt.GetWheelRotation() / evt.m_wheelDelta ;
		if( i > 0 )
			psv->stageView->ZoomP(i);
		else
			psv->stageView->ZoomM(-i);
		psv->stageView->SetViewCoords( mCoords );
		return;
	}
}

//-------------------------------------------------------
void
StageMap::EvtPaint( wxPaintEvent& _evt )
{
	wxBufferedPaintDC dc(this);
	wxSize client_size = GetClientSize();

	if( panelBitmap == NULL && b_must_reload_panels )
		Reload_Panels();
	
	//---------------------------------------------------------
	// Clear the background
	dc.SetBackground(psv->stageView->background_color);
	dc.Clear();

	// If Invalid panel img objects
	if( panelBitmap == NULL )
	{
		wxString img_txt = wxT("No Valid Panels !");
		dc.SetBackground( *wxWHITE_BRUSH);
		dc.SetPen( *wxBLACK_PEN );
		dc.SetFont( 
			wxFont( 20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true )
			);
		dc.DrawText( img_txt, client_size.x/3, client_size.y * 2 / 5);
	}

	//---------------------------------------------------------
	// Draw the panels
	else
		dc.DrawBitmap(   *panelBitmap
				, wxPoint(-coords.x*MAP_ZF, -coords.y*MAP_ZF)
				, false );

	// translation of coords to ob_coords
	wxSize c(coords);
	#define T_X( X ) (X-c.x)*MAP_ZF
	#define T_Y( Y ) (Y-c.y)*MAP_ZF

	//---------------------------------------------------------
	// Draw the objects
	list<Ob_GAtom*>::iterator it(psv->stageView->l_atoms.begin())
				  , it_end(psv->stageView->l_atoms.end());
	for(; it != it_end; it++ )
	{
		Ob_GAtom* o = *it;
		if( o == NULL || o->id_atom == GATOM_SHADOW )
			continue;
		wxSize o_coords( o->coords );
		if( o->id_atom == GATOM_CURSOR || o->id_atom == GATOM_AT )
			o_coords.y = coords.y;

		if( o->bitmap_Mini != NULL )
			dc.DrawBitmap( *o->bitmap_Mini
					, wxPoint( T_X( o_coords.x ), T_Y( o_coords.y ))
					, true
						);
	}

	//---------------------------------------------------------
	// Draw also the current Stage view boundaries
	dc.SetPen( wxPen(color_view_borders));
	dc.SetBrush( *wxTRANSPARENT_BRUSH );
	
	wxSize sv_c(psv->stageView->coords);
	wxSize sv_s(psv->stageView->Get_ObSize());
	dc.DrawRectangle(   T_X( sv_c.x )
				, T_Y( sv_c.y )
				, sv_s.x*MAP_ZF
				, sv_s.y*MAP_ZF
				);
				
				
}

//-------------------------------------------------------
//-------------------------------------------------------
BEGIN_EVENT_TABLE(StageMap, wxControl)
	EVT_SIZE( 		StageMap::EvtSize )
	EVT_PAINT( 		StageMap::EvtPaint )
	EVT_SCROLLWIN(	StageMap::EvtScroll )
	EVT_MOUSE_EVENTS( StageMap::EvtMouse )
END_EVENT_TABLE()
