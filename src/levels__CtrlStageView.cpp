#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dcmemory.h>
#include <wx/dcbuffer.h>

#include "common__mod.h"
#include "common__ob_controls.h"

#include "ob_editor.h"
#include "common__object_stage.h"
#include "levels__globals.h"
#include "levels__classes.h"
#include "levels__sidesWindows.h"
#include "levels__Ob_GObjects.h"
#include "levels__ObImgs_Manager.h"
#include "levels__CtrlStageView.h"
#include "levels__Panel_StageView.h"

using namespace std;

//-------------------------------------------------------
extern LevelEditorFrame *leFrame;
//-------------------------------------------------------
#define ZOOMFACTOR_STEP 0.3
#define VIEW_OBY_MIN	80
#define VIEW_OBX_MIN	150
#define VIEW_OBY_MAX	2000
#define VIEW_OBX_MAX	3000

//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
StageView::StageView(Panel_StageView* _parent)
:wxControl(_parent,wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxVSCROLL|wxHSCROLL|wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN)
{
	psv = _parent;
	ob_panels__size = 0;
	ob_panels = NULL;
	panels_bitmap = NULL;
	f_mouse = 0;
	pset_obj_in_sel_rect = new set<ob_stage_object*>;
	
	SetToolTip( wxString() );
	theTip = GetToolTip();
	theTip->SetDelay( 200 );
	theTip->Enable( false );
	
	background_color = *wxBLACK;
	coords = wxSize(0,0);
	zoom_factor = 1;
	panels_size = wxSize(0,0);

	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	SetBackgroundColour( *wxWHITE );
	pen_rect_sel = wxPen( wxColour(255,255,25), 1, wxDOT );
}

//-------------------------------------------------------
StageView::~StageView()
{
	if( panels_bitmap != NULL )
		delete panels_bitmap;
	panels_bitmap = NULL;
	delete pset_obj_in_sel_rect;
}

//-------------------------------------------------------
static inline
void
ANN__Clear_l_atoms(list<Ob_GAtom*>& t_l_atoms)
{
	while( t_l_atoms.empty() == false )
	{
		Ob_GAtom* t = t_l_atoms.front();
		t_l_atoms.pop_front();
		if( t != NULL )
			delete t;
	}
}


//-------------------------------------------------------
void
StageView::Reset()
{
	Reset_Panels();
	Reset_Ob_GObjects();
	
	// Throw possibles data on drawed objects
	ANN__Clear_l_atoms(l_atoms);

	SCROLLBARS_Update();
	Refresh();
}

//-------------------------------------------------------
void
StageView::Refresh( bool eraseBackground, const wxRect* rect )
{
	wxControl::Refresh(eraseBackground, rect);
	if( psv->stageMap != NULL )
		psv->stageMap->Refresh(eraseBackground, rect);
}

//-------------------------------------------------------
void 
StageView::SetViewCoords( wxSize new_coords )
{
	wxSize old_coords(coords);
	coords = new_coords;
	Normalize_Coords();
	
	if( coords != old_coords )
		Refresh();
}

//-------------------------------------------------------
void
StageView::Zoom__ReComputeCoords( float old_zoom_factor )
{
	wxSize new_size = GetClientSize();
	if( new_size.x <= 0 || new_size.y <= 0 )
		return;
	
	wxSize old_s(new_size.x / old_zoom_factor,new_size.y / old_zoom_factor);
	wxSize new_s(new_size.x / zoom_factor,new_size.y / zoom_factor);
	wxSize max_size(psv->coords_max.x - psv->coords_min.x
			   ,psv->coords_max.y - psv->coords_min.y );
	
	coords.x += (old_s.x - new_s.x) /2;
	coords.y += (old_s.y - new_s.y) / 2;
	
	Normalize_Coords();
}

//-------------------------------------------------------
void
StageView::Normalize_Coords()
{
	wxSize new_size = GetClientSize();
	if( new_size.x <= 0 || new_size.y <= 0 )
		return;
	
	wxSize s( new_size.x / zoom_factor, new_size.y / zoom_factor );
	wxSize max_size(psv->coords_max.x - psv->coords_min.x
			   ,psv->coords_max.y - psv->coords_min.y );

	//-------------------------------------------------
	// Normalize X dimension
	bool b_set_scrollbar = false;
	
	// More space than needed
	if( s.x > max_size.x )
	{
		coords.x = ((psv->coords_max.x + psv->coords_min.x) - s.x) / 2;
		b_set_scrollbar = true;
	}
	else
	{
		// X too low
		if( coords.x < psv->coords_min.x )
		{
			b_set_scrollbar = true;
			coords.x = psv->coords_min.x;
		}
		// X too high
		else if( coords.x + s.x > psv->coords_max.x )
		{
			b_set_scrollbar = true;
			coords.x = (psv->coords_max.x - s.x);
		}
	}
	
	//-------------------------------------------------
	// Normalize Y dimension
	
	// More space than needed
	if( s.y > max_size.y )
	{
		coords.y = ((psv->coords_max.y + psv->coords_min.y) - s.y) / 2;
		b_set_scrollbar = true;
	}
	else
	{
		// Y too low
		if( coords.y < psv->coords_min.y )
		{
			b_set_scrollbar = true;
			coords.y = psv->coords_min.y;
		}
		// Y too high
		else if( coords.y + s.y > psv->coords_max.y )
		{
			b_set_scrollbar = true;
			coords.y = (psv->coords_max.y - s.y);
		}
	}
	
	if( b_set_scrollbar == true )
		SCROLLBARS_Update();
}
	
//-------------------------------------------------------
void 
StageView::ZoomP(const size_t i )
{
	float old_zoom_factor = zoom_factor;
	zoom_factor += ZOOMFACTOR_STEP * i;
	
	wxSize s( GetClientSize() );
	if( (float)s.x / zoom_factor < VIEW_OBX_MIN )
		zoom_factor = (float)s.x / (float)VIEW_OBX_MIN;

	if( (float)s.y / zoom_factor < VIEW_OBY_MIN )
		zoom_factor =(float) s.y / (float)VIEW_OBY_MIN;
	
	if( zoom_factor ==  old_zoom_factor )
		return;

	Zoom__ReComputeCoords( old_zoom_factor );
	SCROLLBARS_Update();
	Refresh();
}

//-------------------------------------------------------
void 
StageView::ZoomM(const size_t i )
{
	float old_zoom_factor = zoom_factor;
	zoom_factor -= ZOOMFACTOR_STEP * i;
	if( zoom_factor < 0.01 )
		zoom_factor = 0.01;
	
	wxSize s( GetClientSize() );
	if( (float)s.x / zoom_factor > VIEW_OBX_MAX )
		zoom_factor = (float)s.x / (float)VIEW_OBX_MAX;

	if( (float) s.y / zoom_factor > VIEW_OBY_MAX )
		zoom_factor = (float)s.y / (float)VIEW_OBY_MAX;

	if( s.x / zoom_factor <= 0 || s.y / zoom_factor <= 0 )
		zoom_factor = old_zoom_factor;

	if( zoom_factor ==  old_zoom_factor )
		return;
	
	Zoom__ReComputeCoords(old_zoom_factor);
	SCROLLBARS_Update();
	Refresh();
}

//-------------------------------------------------------
void 
StageView::ZoomSet(float new_zoom_factor )
{
	
	float old_zoom_factor = zoom_factor;
	zoom_factor = new_zoom_factor;
	if( zoom_factor < 0.01 )
		zoom_factor = 0.01;
	
	wxSize s( GetClientSize() );
	if( (float)s.x / zoom_factor > VIEW_OBX_MAX )
		zoom_factor = (float)s.x / (float)VIEW_OBX_MAX;

	if( (float) s.y / zoom_factor > VIEW_OBY_MAX )
		zoom_factor = (float)s.y / (float)VIEW_OBY_MAX;

	if( s.x / zoom_factor <= 0 || s.y / zoom_factor <= 0 )
		zoom_factor = old_zoom_factor;

	if( zoom_factor ==  old_zoom_factor )
		return;
	Zoom__ReComputeCoords(old_zoom_factor);
	SCROLLBARS_Update();
	Refresh();
}


//-------------------------------------------------------
wxSize 
StageView::Get_ObSize()
{
	return wxSize( last_clientSize.x / zoom_factor
			,  last_clientSize.y / zoom_factor );
}

//-------------------------------------------------------
void
StageView::Delete_GuiObject_of( ob_stage_object* obj )
{
	// Delete atoms associate
	list<Ob_GAtom*>::iterator it_t;
	list<Ob_GAtom*>::iterator it(   l_atoms.begin())
					,it_end(l_atoms.end());
	while( it != it_end )
	{
		Ob_GAtom* o = *it;
		if( o != NULL && o->referer != NULL && o->referer->obj ==obj )
		{
			delete o;
			it_t = it;
			it++;
			l_atoms.erase(it_t);
			if( l_atoms.size() == 0 )
				break;
		}
		else 
			it++;
	}

	// delete the map_GObjs associate
	map<ob_stage_object*,Ob_GObject*>::iterator l_it =
							map_GObjs.find( obj );
	if( l_it != map_GObjs.end() )
	{
		if( l_it->second != NULL )
			delete l_it->second;
		map_GObjs.erase( l_it );
	}
}


//-------------------------------------------------------
void
StageView::SCROLLBARS_Update()
{
	wxSize new_size = GetClientSize();
	if( new_size.x <= 0 || new_size.y <= 0 )
		return;
	
	wxSize view_size(new_size.x / zoom_factor,new_size.y / zoom_factor);
	wxSize max_size(psv->coords_max.x - psv->coords_min.x
			   ,psv->coords_max.y - psv->coords_min.y );
	
	if( view_size.x < max_size.x )
	{
		SetScrollbar( wxHORIZONTAL, coords.x-psv->coords_min.x
				, view_size.x, max_size.x,true);
	}
	else
		SetScrollbar( wxHORIZONTAL, 0,0,0 );
	
	if( view_size.y < max_size.y )
		SetScrollbar( wxVERTICAL, coords.y-psv->coords_min.y
				, view_size.y, max_size.y );
	else
	{
		SetScrollbar( wxVERTICAL, 0,0,0 );
	}
}

//-------------------------------------------------------
void
StageView::Reset_Panels()
{
	if( panels_bitmap != NULL )
		delete panels_bitmap;
	panels_bitmap = NULL;

	if( ob_panels != NULL )
	{
		for( size_t i = 0; i < ob_panels__size; i++ )
			delete ob_panels[i];
		delete[] ob_panels;
	}
	ob_panels__size = 0;
	ob_panels = NULL;
}

//-------------------------------------------------------
void
StageView::Reset_Ob_GObjects()
{
	map<ob_stage_object*,Ob_GObject*>::iterator it;
	while( map_GObjs.empty() == false )
	{
		it = map_GObjs.begin();
		if( it->second != NULL )
			delete it->second;
		map_GObjs.erase(it);
	}
}

//-------------------------------------------------------
void
StageView::Init_PanelData( size_t curr_ind )
{
	Ob_Panel_Data* t = ob_panels[curr_ind];

	wxImage* srcImg = obImgs_Mgr->GetImageSrc( t->ob_panel, t->img_key );
	
	// Here, no Panel at all is valid !!
	if( srcImg == NULL )
	{
		wxImage* panel_noImg = new wxImage( *noImg );
		panel_noImg->Rescale( curr_mod->video_res.x, 
			curr_mod->video_res.y );
			obImgs_Mgr->AbsorbeImage( wxT("panel_noImg"), NULL, panel_noImg);
		srcImg = panel_noImg;
	}

	if( curr_ind == 0 )
	{
		t->ob_x = 0;
		t->ob_y = 0;
	}
	else
	{
		wxImage* prev_ImgSrc = 
			obImgs_Mgr->GetImageSrc( 
				  ob_panels[curr_ind-1]->ob_panel
				, ob_panels[curr_ind-1]->img_key );

		t->ob_x = ob_panels[curr_ind -1]->ob_x
				+ prev_ImgSrc->GetWidth();
		t->ob_y = ob_panels[curr_ind -1]->ob_y;
	}
	
	t->ob_w = srcImg->GetWidth();
	t->ob_h = srcImg->GetHeight();
}


//-------------------------------------------------------
void
StageView::Init_PanelImage( size_t curr_ind )
{
	Ob_Panel_Data* t = ob_panels[curr_ind];
	
	wxImage* src_img = obImgs_Mgr->GetImageSrc( 
			  t->ob_panel
			, t->ob_panel->GetFileName().GetFullPath()
			);

	bool b_panel_noImg_exists = obImgs_Mgr->IsImageExists( wxT("panel_noImg") );
	
	if( src_img != NULL && b_panel_noImg_exists == false )
	{
		wxImage* panel_noImg = new wxImage( *noImg );
		panel_noImg->Rescale( src_img->GetWidth()
					  , src_img->GetHeight() );
					  obImgs_Mgr->AbsorbeImage( wxT("panel_noImg"), NULL, panel_noImg);
	}
	
	if( src_img != NULL )
		t->img_key = t->ob_panel->GetFileName().GetFullPath();
	else
		t->img_key = wxT("panel_noImg");
	
	return;
}

//-------------------------------------------------------
void
StageView::Reload_Panels()
{
	ANN__Clear_l_atoms(l_atoms);
	Reset_Panels();
	
	// Get all the panels objects from the stage object
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
		return;
	
	ob_stage* theStage = (ob_stage*) curr_stageFile->obj_container;
	ob_stage_panel** arr_panels = theStage->Get_Panels( ob_panels__size);
	
	if( ob_panels__size == 0 )
		return;
	
	ob_panels = new Ob_Panel_Data*[ob_panels__size];
	for( size_t i = 0; i < ob_panels__size; i++ )
	{
		ob_panels[i] = new Ob_Panel_Data;
		ob_panels[i]->ob_panel = arr_panels[i];
		Init_PanelImage(i);
	}

	panels_size.x = 0;
	panels_size.y = 0;
	for( size_t i = 0; i < ob_panels__size; i++ )
	{
		Init_PanelData(i);
		panels_size.x += ob_panels[i]->ob_w;
		panels_size.y  = max(ob_panels[i]->ob_h,panels_size.y);
	}
	
	// Cleanings
	delete[] arr_panels;
	
	// Refreshing Screen
	Refresh();
}

//-------------------------------------------------------
wxBitmap*	
StageView::Get_Panels_Bitmap( 
				const int   ob_x,
				const int   ob_y,
				const int   client_w,
				const int   client_h,
				const float zoom_factor
				)
{
	int _w = 0;
	int _h = 0;

	// If Invalid stage objects
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
		return NULL;
	
	// If no panel bitmap here
	if( ob_panels__size == 0 )
	{
		if( panels_bitmap != NULL )
		{
			delete panels_bitmap;
			panels_bitmap = NULL;
		}
		return panels_bitmap;
	}

	// If Last computed Panel bitmap is up to date
	if( 	   ob_x == last_Coords.x 
		&& ob_y == last_Coords.y
		&& zoom_factor == last_zoom_factor
		&& last_clientSize.x == (int)client_w
		&& last_clientSize.y == (int)client_h
		&& panels_bitmap != NULL )
	{
		return panels_bitmap;
	}

	last_Coords.x = ob_x;
	last_Coords.y = ob_y;
	last_clientSize.x = client_w;
	last_clientSize.y = client_h;
	last_zoom_factor = zoom_factor;
	
	//------------------------------------------------------
	// Calc the Resulting Bitmap offsets and Size
	int ob_w = (float) client_w / zoom_factor;
	int ob_h = (float) client_h / zoom_factor;
	
	wxSize min_offset(ob_w,ob_h);
	for( size_t i = 0; i < ob_panels__size; i++)
	{
		Ob_Panel_Data* t = ob_panels[i];
		t->temp_off_x = 0;
		t->temp_off_y = 0;
		t->temp_w = 0;
		t->temp_h = 0;
		
		//---------------------------------------------
		// Check if the panel appears on screen
		if( 		t->ob_x > ob_x - t->ob_w
			&& 	t->ob_x < ob_x + ob_w
			&&	t->ob_y > ob_y - t->ob_h
			&& 	t->ob_y < ob_y + ob_h
			)
		{
			//------------------------------------
			t->temp_off_x = t->ob_x - ob_x;
			
			if( t->ob_x + t->ob_w > (int)ob_x + ob_w )
				t->temp_w = (ob_x + ob_w) - t->ob_x;
			else
				t->temp_w = t->ob_w;
			
			//------------------------------------
			t->temp_off_y = t->ob_y - ob_y;
			
			if( t->ob_y + t->ob_h > (int)ob_y + ob_h )
				t->temp_h = (ob_y + ob_h) - t->ob_y;
			else
				t->temp_h = t->ob_h;
			
			//------------------------------------
			if( t->temp_off_x + t->temp_w > _w )
				_w = t->temp_off_x + t->temp_w;
			if( t->temp_off_y + t->temp_h > _h )
				_h = t->temp_off_y + t->temp_h;
			
			min_offset.x = min( min_offset.x, t->temp_off_x );
			min_offset.y = min( min_offset.y, t->temp_off_y );
		}
	}
	
	min_offset.x = max(0,min_offset.x);
	min_offset.y = max(0,min_offset.y);

	// Nothing to draw
	if( _w == 0 || _h == 0 )
		return NULL;
	
	// Build the Bitmap
	if( panels_bitmap != NULL )
		delete panels_bitmap;
	
	int bitmap_w = (_w-min_offset.x) *zoom_factor;
	int bitmap_h = (_h-min_offset.y) *zoom_factor;
	panels_bitmap = new wxBitmap(bitmap_w,bitmap_h );
	wxMemoryDC temp_dc;
	temp_dc.SelectObject( *panels_bitmap );
	temp_dc.SetBackground( wxBrush( background_color ));
	temp_dc.Clear();
	
	for( size_t i = 0; i < ob_panels__size; i++)
	{
		Ob_Panel_Data* t = ob_panels[i];
		
		// If this panel appears on screen
		if( ob_panels[i]->temp_w > 0 && ob_panels[i]->temp_h > 0 )
		{
			wxImage* _img = obImgs_Mgr->GetImage(
					  ob_panels[i]->ob_panel
					, ob_panels[i]->img_key
					, zoom_factor );

			// Draw the bitmap in the dc at the correct positions
			wxBitmap t_bitmap( *_img );
			temp_dc.DrawBitmap( t_bitmap
					  , (t->temp_off_x-min_offset.x)*zoom_factor
					  , (t->temp_off_y-min_offset.y)*zoom_factor );
		}
	}
	
	return panels_bitmap;
}

//-------------------------------------------------------
void 
StageView::EvtSize( wxSizeEvent& _evt )
{
	wxSize s(GetParent()->GetClientSize());
	_evt.Skip();
	s =_evt.GetSize();

	Normalize_Coords();
	SCROLLBARS_Update();
}

//-------------------------------------------------------
void
StageView::EvtScroll( wxScrollWinEvent& evt )
{
//	evt.Skip();
	if( evt.GetOrientation() == wxHORIZONTAL )
	{
		SetScrollPos( wxHORIZONTAL, evt.GetPosition() );
		coords.x = GetScrollPos( wxHORIZONTAL ) + psv->coords_min.x;
		Refresh();
	}
		
	else //GetOrientation() == wxVERTICAL
	{
		SetScrollPos( wxVERTICAL, evt.GetPosition() );
		coords.y = GetScrollPos( wxVERTICAL ) + psv->coords_min.y;
		Refresh();
	}
}

//-------------------------------------------------------
static inline
void
ANN__Clear_And_Delete_l_atoms( std::list<Ob_GAtom*>* t_l_atoms )
{
	ANN__Clear_l_atoms( *t_l_atoms );
	delete t_l_atoms;
}

void
StageView::OnPaint( wxPaintEvent& _evt )
{
	wxAutoBufferedPaintDC dc(this);
	wxSize client_size = GetClientSize();

	// If Invalid stage objects
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
	{
		if( l_atoms.empty() == false || panels_bitmap != NULL )
		{
			ANN__Clear_l_atoms(l_atoms);
			Reset_Ob_GObjects();
			Reset_Panels();
			obImgs_Mgr->Reset();
		}
		ClearBackground();
		wxString img_txt = wxT("No Valid Stage Selected !");
		dc.SetBackground( *wxWHITE_BRUSH);
		dc.SetPen( *wxBLACK_PEN );
		dc.Clear();
		dc.SetFont( 
			wxFont( 20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true )
			);
		dc.DrawText( img_txt, 100,100);
		psv->stageMap->Refresh();
		return;
	}

	dc.SetBackground( wxBrush( background_color ));
	dc.Clear();
	
	//-----------------------------------------------------
	// Draw the panels
	wxBitmap* t_bitmap = Get_Panels_Bitmap(
				coords.x,
				coords.y,
				client_size.x,
				client_size.y,
				zoom_factor
				);
	
	// The visible areaSize in ob coords
	wxSize areaSize(client_size.x/zoom_factor,client_size.y/zoom_factor);
		
	if( t_bitmap != NULL )
	{
		wxSize panels_Coords(0,0);
		if( coords.x < 0 )
			panels_Coords.x = - coords.x * zoom_factor;
		if( coords.y < 0 )
			panels_Coords.y = - coords.y * zoom_factor;
		
		if( (psv->f_hidden & FM_BACKGROUND ) == 0 )
		{
			dc.DrawBitmap( *t_bitmap
					, panels_Coords.x
					, panels_Coords.y
					,true
					);
		}
	}
	
	// t_bitmap MUST NOT be delete 


	//-----------------------------------------------------
	// Draw the Objects
	
	// Throw previous ones
	ANN__Clear_l_atoms(l_atoms);

	size_t arr_objs__size;
	ob_object** arr_objs = curr_stageFile->obj_container->GetAllSubObjS(arr_objs__size);
	
	wxSize coords_min(0,0),coords_max(panels_size);
	
	for( size_t i = 0; i < arr_objs__size; i++ )
	{
		//--------------------------------------------------
		// Only draw drawables of course
		bool b_cont = false;
		switch( arr_objs[i]->type )
		{
//			case OB_STAGE_OBJECT:
			case OB_TYPE_WALL:
			case OB_TYPE_HOLE:
			case OB_TYPE_PLAYER_SPAWN:
			case OB_TYPE_STAGE_AT:
			case OB_TYPE_SPAWN:
			case OB_TYPE_SPAWN_HEALTHY:
			case OB_TYPE_SPAWN_ITEM:
				b_cont = true;
				break;
		}
		
		if( b_cont == false )
			continue;

		// remove all possible from this list
		if( arr_objs[i]->type == OB_TYPE_STAGE_AT )
		{
		  switch( ((ob_stage_object*)arr_objs[i])->object_type )
		  {
			// Don't edit These ones
			case SOBJ_SHADOWCOLOR:
			case SOBJ_SHADOWALPHA:
				continue;
		  }
		}
		
		//--------------------------------------------------
		// Get the Ob_GObject associated to the ob_object...
		ob_stage_object* t = (ob_stage_object*) arr_objs[i];
		if( map_GObjs.find(t) == map_GObjs.end() )
		{
			Ob_GObject* gOb = Ob_GObject::GuessAndConstruct( t );
			map_GObjs[t] = gOb;
		}
		
		//--------------------------------------------------
		// Get all the infos for this object
		int selected_state = 
			leFrame->OBJECT__Get_SelectionState( map_GObjs[t]->obj );
		list<Ob_GAtom*>* t_l_atoms = map_GObjs[t]->GetBitmaps(
				coords,
				areaSize,
				zoom_factor,
				selected_state,
				panels_size.x
				);
		size_t s0 = t_l_atoms->size();
		
		//--------------------------------------------------
		// Recalc Stage Boundaries
		list<Ob_GAtom*>::iterator   it(t_l_atoms->begin()),
						it_end(t_l_atoms->end());
		for(; it != it_end; it++ )
		{
			Ob_GAtom* o = (*it);
			switch( o->id_atom )
			{
			case GATOM_SCROLL_LINE:
				coords_min.y = min( coords_min.y, o->coords.y );
				coords_max.y = max( coords_max.y, o->coords.y + o->size.y );
				break;
				
			case GATOM_SHADOW:
			case GATOM_BODY:
			case GATOM_SPAWN_POINT:
			case GATOM_OTHER:
				coords_min.y = min( coords_min.y, o->coords.y );
				coords_max.y = max( coords_max.y, o->coords.y + o->size.y );
			default:
				coords_min.x = min( coords_min.x, o->coords.x );
				coords_max.x = max( coords_max.x, o->coords.x + o->size.x );
			}
		}
		
		//--------------------------------------------------
		// Apply visibilities Filters
		ob_stage_object* _o = (ob_stage_object*) arr_objs[i];
		int _sType=Stage__Type__Get_SuperType(_o->object_type);
		if( 	leFrame->aSet_Hide_SuperType.find(_sType) 
			!= 
			leFrame->aSet_Hide_SuperType.end()
		  )
		{
			ANN__Clear_And_Delete_l_atoms(t_l_atoms);
			continue;
		}
		if( 	leFrame->aSet_Hide_TYPE.find(_o->object_type) 
			!= 
			leFrame->aSet_Hide_TYPE.end()
		  )
		{
			ANN__Clear_And_Delete_l_atoms(t_l_atoms);
			continue;
		}
		
		//--------------------------------------------------
		// This one is not a drawable object
		if( map_GObjs[t] == NULL )
		{
			ANN__Clear_And_Delete_l_atoms(t_l_atoms);
			continue;
		}
		
		//--------------------------------------------------
		// Insert them in the list of TO-DRAW
		while( t_l_atoms->empty() == false) 
		{
			Ob_GAtom* obAtom = t_l_atoms->front();
			t_l_atoms->pop_front();
			if( obAtom == NULL )
				continue;
			
			// Sort the Atoms by Z values
			list<Ob_GAtom*>::iterator it2(l_atoms.begin()),
							it2_end(l_atoms.end());
			
			bool at_the_end = true;
			for(; it2 != it2_end; it2++ )
			{
				if( (*it2)->Z > obAtom->Z )
				{
					l_atoms.insert( it2, obAtom );
					at_the_end = false;
					break;
				}
			}
			
			if( at_the_end == true )
				l_atoms.push_back( obAtom );
		}
		delete t_l_atoms;
		
		s0 = l_atoms.size();
	}

	//---------------------------------------------------
	// Check if stage boundaries have changed
	if( 	   coords_min.x != psv->coords_min.x
		|| coords_min.y != psv->coords_min.y
		|| coords_max.x != psv->coords_max.x
		|| coords_max.y != psv->coords_max.y
		)
	{
		Normalize_Coords();
		SCROLLBARS_Update();
		psv->coords_min = coords_min;
		psv->coords_max = coords_max;
		if( psv->stageMap != NULL )
		{
			psv->stageMap->SCROLLBARS_Update();
			psv->stageMap->Refresh();
		}
		
		// Check if there is some Coords to restore
		if( psv->coords_to_restore.x != -100000 )
		{
			coords = psv->coords_to_restore;
			Normalize_Coords();
			psv->coords_to_restore = wxSize(-100000,-100000);
		}
		
		// have to recompute Everything
		Refresh();
		if( arr_objs != NULL )
			delete[] arr_objs;
		return;
	}


	//--------------------------------------------------
	// Translate coords and DRAW
	list<Ob_GAtom*>::iterator it2(l_atoms.begin()),
					it2_end(l_atoms.end());
	for(; it2 != it2_end; it2++ )
	{
		Ob_GAtom* obAtom = (*it2);
		
		// Translate coords of the object to screen ones
		obAtom->view_coords.x = (obAtom->coords.x-coords.x)*zoom_factor;
		obAtom->view_coords.y = (obAtom->coords.y-coords.y)*zoom_factor;
		
		if( obAtom->b_on_screen == true && obAtom->theBitmap != NULL )
			dc.DrawBitmap( 	 *obAtom->theBitmap
						, obAtom->view_coords.x
						, obAtom->view_coords.y
						, true );
	}

	if( arr_objs != NULL )
		delete[] arr_objs;
	
	
	// Paint the selection rectangle
	if( 	   (f_mouse & M_RECT_SEL) > 0
		&&
			(
				mouse_selRect_ob_coords_end.x != mouse_selRect_ob_coords_start.x
				||
				mouse_selRect_ob_coords_end.y != mouse_selRect_ob_coords_start.y
			)
		)
	{
		dc.SetBrush( *wxTRANSPARENT_BRUSH );
		dc.SetPen( pen_rect_sel );
		wxSize mins( min(mouse_selRect_ob_coords_start.x,
				     mouse_selRect_ob_coords_end.x),
				 min(mouse_selRect_ob_coords_start.y,
				     mouse_selRect_ob_coords_end.y)
				   );
		wxSize s( abs(mouse_selRect_ob_coords_start.x-
				     mouse_selRect_ob_coords_end.x),
				 abs(mouse_selRect_ob_coords_start.y-
				     mouse_selRect_ob_coords_end.y)
				   );
		dc.DrawRectangle(
			 mins.x
			,mins.y
			,s.x
			,s.y
			);
	}

	if( psv->stageMap != NULL )
		psv->stageMap->Refresh();
	return;
}

//-------------------------------------------------------
Ob_GAtom* 
StageView::Get_Aimed_Atom(int x, int y)
{
	list<Ob_GAtom*>::iterator it(l_atoms.begin()),
					it_end(l_atoms.end());

	Ob_GAtom* res = NULL;
	wxBrush obBrush( wxColour(255,0,255));
	for(; it != it_end; it++ )
	{
		Ob_GAtom* at = (*it);
		if( at == NULL || at->referer == NULL || at->theBitmap == NULL )
			continue;
		
		wxSize bSize(at->theBitmap->GetWidth(),at->theBitmap->GetHeight() );
		
		if( 		x >= at->view_coords.x && x < at->view_coords.x + bSize.x 
			&&	y >= at->view_coords.y && y < at->view_coords.y + bSize.y )
		{
			if( at->lastImg == NULL )
				continue;
			int cx = x-at->view_coords.x;
			int cy = y-at->view_coords.y;
			if( 	   at->lastImg->GetRed( cx, cy )==at->lastImg->GetMaskRed()
				&& at->lastImg->GetGreen( cx, cy )==at->lastImg->GetMaskGreen()
				&& at->lastImg->GetBlue( cx, cy )==at->lastImg->GetMaskBlue()
				)
				continue;
			res = at;
		}
	}
	
	return res;
}

//-------------------------------------------------------
bool
StageView::ANN__SendMouseEvent_to_AllSelected( wxSize& _m_coords, wxMouseEvent& evt, bool multi_selection )
{
	bool res = false;
	// Process the list of selecteds
	set<ob_stage_object*>::iterator it(leFrame->aSet_Selected.begin())
						,it_end(leFrame->aSet_Selected.end());
	for(;it!=it_end;it++)
	{
		// check if there is a corresponding atom
		list<Ob_GAtom*>::iterator l_it(	l_atoms.begin())
						,l_it_end(	l_atoms.end());
		for(;l_it!=l_it_end;l_it++)
		{
			if( *l_it != NULL && (*l_it)->referer != NULL 
				&& (*l_it)->referer->obj == *it )
			{
				if( (*l_it)->referer->OnMouseEvent( NULL, _m_coords, evt, multi_selection ) == true )
					res = true;
				break;
			}
		}
	}
	return res;
}

//-------------------------------------------------------
inline
void
StageView::ANN__CheckRectSelection()
{
	set<ob_stage_object*>* t_set = new set<ob_stage_object*>;
	
	wxSize mins( min(mouse_selRect_ob_coords_start.x,
			     mouse_selRect_ob_coords_end.x),
			 min(mouse_selRect_ob_coords_start.y,
			     mouse_selRect_ob_coords_end.y)
			   );
	wxSize maxs( max(mouse_selRect_ob_coords_start.x,
			     mouse_selRect_ob_coords_end.x),
			 max(mouse_selRect_ob_coords_start.y,
			     mouse_selRect_ob_coords_end.y)
			   );
	
	list<Ob_GAtom*>::iterator it(	l_atoms.begin())
					, it_end(	l_atoms.end());
	for(; it != it_end; it++ )
	{
		Ob_GAtom* o = (*it);
		if( 	   o == NULL || o->referer == NULL
			|| o->referer->obj == NULL
			|| o->theBitmap == NULL
			|| o->id_atom == GATOM_AT
			|| o->id_atom == GATOM_SHADOW
			)
			continue;
		
		if(      o->view_coords.x + o->theBitmap->GetWidth() >= mins.x 
			&& o->view_coords.x <= maxs.x 
			&& o->view_coords.y  + o->theBitmap->GetHeight() >= mins.y
			&& o->view_coords.y <= maxs.y
		  )
			{
				// Don't add object which are :
				//  - not in the curr rect sel 
				//  - and in select_set
				ob_stage_object* obj = o->referer->obj;
				if( 		pset_obj_in_sel_rect->find(obj)
						!= 
						pset_obj_in_sel_rect->end()
					|| 
						leFrame->aSet_Selected.find( obj ) 
						== 
						leFrame->aSet_Selected.end()
					)
					t_set->insert( obj );
			}
	}
	
	// Now, deselect obj which are no longer in the rect sel
	set<ob_stage_object*>::iterator l_it(	pset_obj_in_sel_rect->begin())
					   , l_it_end(	pset_obj_in_sel_rect->end());
	for(; l_it != l_it_end; l_it++ )
	{
		if( t_set->find( (*l_it) ) == t_set->end() )
			leFrame->aSet_Selected.erase( *l_it );
	}

	delete pset_obj_in_sel_rect;
	pset_obj_in_sel_rect = t_set;

	// Now add all object in the rect sel
	l_it 	   = pset_obj_in_sel_rect->begin();
	l_it_end = pset_obj_in_sel_rect->end();
	for(; l_it != l_it_end; l_it++ )
	{
		leFrame->aSet_Selected.insert( (*l_it));
	}

	leFrame->OBJECT__ListSelected__UpdateNoteBook();
}

//-------------------------------------------------------
void
StageView::EvtMouse( wxMouseEvent& evt )
{
	wxSize ob_coords(   (int)(evt.m_x / zoom_factor+coords.x)
				, (int)(evt.m_y / zoom_factor+coords.y)
				);
	bool b_multi_selection = ( leFrame->aSet_Selected.size() > 1 );


	//---------------------------------------------
	// Nothing, just a mouse move
	if( evt.Moving() == true )
	{
		// Put the tooltip
		wxString tip_txt=
			  IntToStr((int)(evt.m_x / zoom_factor+coords.x))
			  + wxT(" , ") 
			+ IntToStr((int)(evt.m_y / zoom_factor+coords.y))
			;
		theTip->SetTip( tip_txt );
		theTip->Enable(true);
		evt.Skip();
		return;
	}
	
	//---------------------------------------------
	// Hide the Tip on all other events
	theTip->Enable(false);
	
	//---------------------------------------------
	// Mouse wheel
	if( evt.GetWheelRotation() != 0 )
	{
		if( evt.ShiftDown() == true )
			coords.y += - evt.GetWheelRotation();
		else // Without shift Down
			coords.x += - evt.GetWheelRotation();
		Normalize_Coords();
		SCROLLBARS_Update();
		Refresh();
		return;
	}
	
	
	//---------------------------------------------
	// Dragging evt
	if( evt.Dragging() == true )
	{
		if( 	   evt.m_leftDown == true
			&& f_mouse & M_MLB_OK )
		{
			f_mouse |= M_HAVE_DRAGGED;
			// signal all currently selected the dragging evt
			bool b_must_refresh = 
				ANN__SendMouseEvent_to_AllSelected( 
						ob_coords
						,evt,b_multi_selection
						);
			if( b_must_refresh == true )
			{
				psv->Refresh();
				leFrame->NOTEBOOK__Update_Vals();
			}
		}
		
		// Start or Continue of a rectangle selection move
		else if( evt.m_leftDown == true && f_mouse & (M_RECT_SEL|M_RECT_SEL_START))
		{
			// Start one
			if( f_mouse | M_RECT_SEL_START )
			{
				theTip->Enable(false);
				f_mouse &= ~M_RECT_SEL_START;
				f_mouse |= M_RECT_SEL;
				if( evt.m_shiftDown == false && evt.m_controlDown == false)
					leFrame->aSet_Selected.clear();
				pset_obj_in_sel_rect->clear();
			}
			
			mouse_selRect_ob_coords_end = wxSize(evt.m_x, evt.m_y);
			// Select object in the rect
			ANN__CheckRectSelection();
			Refresh();
		}
		evt.Skip();
		SetFocus();
		return;
	}
	
	//---------------------------------------------
	// End of a rectangle selection
	if( 	   evt.LeftUp() && (f_mouse & M_RECT_SEL) )
	{
		f_mouse &= ~(M_RECT_SEL|M_RECT_SEL_START);
		evt.Skip();
		SetFocus();
		Refresh();
		return;
	}


	//---------------------------------------------
	// No valid object for the next possible events
	Ob_GAtom* obAtom = Get_Aimed_Atom(evt.m_x, evt.m_y);
	if( 	     obAtom == NULL 
		  || obAtom->referer == NULL 
		  || obAtom->referer->obj == NULL
	  )
	{
		f_mouse &= ~M_MLB_OK;
		// Start a rectangle selection 
		if( evt.LeftDown() == true )
		{
			f_mouse |= M_RECT_SEL_START;
			mouse_selRect_ob_coords_start = wxSize(evt.m_x, evt.m_y);
			mouse_selRect_ob_coords_end   = wxSize(evt.m_x, evt.m_y);
		}
		evt.Skip();
		SetFocus();
		return;
	}
	
	// No more rectangle selection from here
	f_mouse &= ~(M_RECT_SEL|M_RECT_SEL_START);

	//---------------------------------------------
	// Dblclick
	if( evt.LeftDClick() )
	{
		leFrame->OBJECT__ListSelected__Props(obAtom);
		evt.Skip();
		return;
	}


	//---------------------------------------------
	// LeftUp
	if( evt.LeftUp() )
	{
		// with no dragging => Simple click
		if( (f_mouse & M_HAVE_DRAGGED) == 0 )
		{
			// With Ctrl pressed
			if( evt.m_controlDown == true || evt.m_shiftDown == true)
			{
				if( f_mouse & M_MUST_RM_ON_UP )
					leFrame->OBJECT__ListSelected__Rm( obAtom->referer->obj );
				else
					leFrame->OBJECT__ListSelected__Add( obAtom->referer->obj );
			}
			else
				leFrame->OBJECT__ListSelected__Set(obAtom->referer->obj);
		}
		f_mouse &= ~M_MUST_RM_ON_UP;
		evt.Skip();
		SetFocus();
		return;
	}
	


	//---------------------------------------------
	// LeftDown   ==>>  Selection
	if( evt.LeftDown() )
	{
		f_mouse &= ~M_HAVE_DRAGGED;
		f_mouse |= M_MLB_OK;

		if( leFrame->OBJECT__ListSelected__IsIn(obAtom->referer->obj)
			== true )
			f_mouse |= M_MUST_RM_ON_UP;
		else if( evt.m_controlDown == true || evt.m_shiftDown == true)
			leFrame->OBJECT__ListSelected__Add( obAtom->referer->obj );
		else
			leFrame->OBJECT__ListSelected__Set(obAtom->referer->obj);
		
		// signal all currently selected the pressing point
		ANN__SendMouseEvent_to_AllSelected( ob_coords,evt,b_multi_selection);
		
		// Must also Select the aimed object and signal it the evt
//		leFrame->OBJECT__ListSelected__Add(obAtom->referer->obj);
		obAtom->referer->OnMouseEvent( obAtom,ob_coords, evt, b_multi_selection);
		evt.Skip();
		SetFocus();
		return;
	}
	
	evt.Skip();
}

//-------------------------------------------------------
bool
StageView::ANN__SendKeyEvent_to_AllSelected( wxKeyEvent& evt, bool multi_selection )
{
	bool res = false;
	// Process the list of selecteds
	set<ob_stage_object*>::iterator it(leFrame->aSet_Selected.begin())
						,it_end(leFrame->aSet_Selected.end());
	for(;it!=it_end;it++)
	{
		// check if there is a corresponding atom
		list<Ob_GAtom*>::iterator l_it(	l_atoms.begin())
						,l_it_end(	l_atoms.end());
		for(;l_it!=l_it_end;l_it++)
		{
			if( *l_it != NULL && (*l_it)->referer != NULL 
				&& (*l_it)->referer->obj == *it )
			{
				if( (*l_it)->referer->OnKeyEvent( evt, multi_selection ) == true )
					res = true;
				break;
			}
		}
	}
	return res;
}


//-------------------------------------------------------
void
StageView::EvtChar( wxKeyEvent& evt )
{
	switch( evt.GetKeyCode() )
	{
		case WXK_LEFT:
		case WXK_NUMPAD_LEFT:
		case WXK_RIGHT:
		case WXK_NUMPAD_RIGHT:
		case WXK_UP:
		case WXK_NUMPAD_UP:
		case WXK_DOWN:
		case WXK_NUMPAD_DOWN:
		{
			bool b_must_refresh = 
				ANN__SendKeyEvent_to_AllSelected(evt,( leFrame->aSet_Selected.size() > 1 ) );
			if( b_must_refresh == true )
			{
				psv->Refresh();
				leFrame->NOTEBOOK__Update_Vals();
			}
			evt.Skip();
			return;
		}
			
		case WXK_DELETE:
		case WXK_NUMPAD_DELETE:
		{
			leFrame->OBJECT__ListSelected__Delete();
			SetFocus();
			evt.Skip();
			return;
		}
	}

	evt.Skip();
}


//-------------------------------------------------------
//-------------------------------------------------------
BEGIN_EVENT_TABLE(StageView, wxControl)
	EVT_PAINT( 		StageView::OnPaint )
	EVT_SIZE(  		StageView::EvtSize )
	EVT_SCROLLWIN(	StageView::EvtScroll )
	EVT_MOUSE_EVENTS( StageView::EvtMouse )
	EVT_CHAR( 		StageView::EvtChar )
END_EVENT_TABLE()
