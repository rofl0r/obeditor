#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dcmemory.h>

#include "common__mod.h"
#include "common__ob_controls.h"

#include "ob_editor.h"
#include "common__object_stage.h"
#include "levels__globals.h"
#include "levels__classes.h"
#include "levels__sidesWindows.h"
#include "levels__ObImgs_Manager.h"
#include "levels__Ob_GObjects.h"

using namespace std;

// For Wall/Hole, the radius of point of the elt in ob_coord
size_t pt_radius = 4;

#define M_DIST( x0, x1 ) abs(x0-x1)
#define DIST_DETECT 10


//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
Ob_GAtom::Ob_GAtom(int atom_id)
: referer(NULL),id_atom(atom_id),key(wxString())
 ,coords(0,0),size(0,0),view_coords(0,0)
 ,b_on_screen(false),theBitmap(NULL),lastImg(NULL),Z(0),A(0)
{
	bitmap_Mini = NULL,
	img_Mini = NULL;
};


//-------------------------------------------------------
wxString 
Ob_GAtom::ToStr()
{
	wxString res;
	if( referer == NULL )
		res = wxT("referer == NULL\n");
	else if( referer->obj == NULL )
		res = wxT("referer->obj == NULL\n");
	else
		res = referer->obj->ToStr();
	res += wxT("---------------\n");
	res += wxT("id_atom = ") + IntToStr(id_atom) + wxT("\n");
	wxString s_key = Convert_To_Ob_Path(key);
	if( s_key == wxString() )
		s_key = key;
	res += wxT("key = ") + s_key + wxT("\n");
	res += wxT("coords = ") + IntToStr(coords.x) + wxT(",") + IntToStr(coords.y) + wxT("\n");
	res += wxT("Z = ") + IntToStr(Z) + wxT("\n");
	res += wxT("A = ") + IntToStr(A) + wxT("\n");
	res += wxT("size   = ") + IntToStr(size.x) + wxT(",") + IntToStr(size.y) + wxT("\n");
	res += wxT("Bitmap   = ") + IntToStr((size_t)theBitmap) + wxT("\n");
	res += wxT("lastImg  = ") + IntToStr((size_t)lastImg) + wxT("\n");
	
	return res;
}


//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
Ob_GObject* 
Ob_GObject::GuessAndConstruct( ob_stage_object* src )
{
	if( 	   src->type == OB_TYPE_SPAWN 
		|| src->type == OB_TYPE_SPAWN_HEALTHY 
		|| src->type == OB_TYPE_SPAWN_ITEM
		)
		return new Ob_GObject__Spawn( src );
	
	else if( 	src->type == OB_TYPE_STAGE_AT
		   && ((ob_stage_object*)src)->object_type != SOBJ_SCROLLZ
		   )
		return new Ob_GObject__AT( src );

	else if( 	src->type == OB_TYPE_STAGE_AT
		   && ((ob_stage_object*)src)->object_type == SOBJ_SCROLLZ
		   )
		return new Ob_GObject__ScrollZ( src );
	
	else if( src->type == OB_TYPE_PLAYER_SPAWN )
		return new Ob_GObject__PlayerSpawn( src );
	
	else if( src->type == OB_TYPE_WALL || src->type == OB_TYPE_HOLE )
		return new Ob_GObject__Wall( src );
	
	return NULL;
}


//-------------------------------------------------------
std::list<Ob_GAtom*>* 
Ob_GObject::GetBitmaps(
				wxSize view_coords ,
				wxSize view_size ,
				float  zoom_factor,
				int    select_state,
			      int 	 stage_w
				)
{
	// If Invalid stage objects
	if( curr_stageFile == NULL || curr_stageFile->obj_container == NULL )
		return NULL;
//	int direction = ((ob_stage*)curr_stageFile->obj_container)->direction;

	wxSize clientSize( view_size.x/zoom_factor, view_size.y/zoom_factor);
	
	list<Ob_GAtom*>* l_atoms = Get_GAtoms(
							view_coords ,
							view_size ,
							zoom_factor,
							select_state,
							stage_w
						);
	list<Ob_GAtom*>::iterator it(l_atoms->begin()),
					it_end(l_atoms->end());
	
	int dbg_i = -1;
	for( ; it != it_end; it++ )
	{
		dbg_i++;
		Ob_GAtom* t = (*it);
		
		ob_object* related_obj = NULL;
		int remap = 0;
		bool b_flip = false;
		if( t->id_atom == GATOM_BODY )
		{
			b_flip = true;//(direction==STAGE_RIGHT);
			related_obj = obj;
			if( obj->GetSubObject_Token(wxT("flip")) == wxT("1"))
				b_flip = !b_flip;

			ob_object* map_obj = obj->GetSubObject(wxT("map"));
			if( map_obj != NULL )
				remap = StrToInt(map_obj->GetToken(0));
		}


		// Zoom_factor don't apply to all images
		float t_zoom_factor = zoom_factor;
		if( t->id_atom != GATOM_SHADOW 
			&& t->id_atom != GATOM_BODY
			&& t->id_atom != GATOM_SPAWN_POINT
			)
 			t_zoom_factor = 1;

		//------------------------------------------------
		// If the Bitmap will not appear on screen
		// Don't compute the image
		if( 	t->coords.x + t->size.x < view_coords.x
			||	t->coords.x > view_coords.x + view_size.x
			||	t->coords.y + t->size.y < view_coords.y
			||	t->coords.y > view_coords.y + view_size.y
			)
		{
			t->b_on_screen = false;
			t->theBitmap = NULL;
		}
	
		//------------------------------------------------
		// Image will appears on view screen
		else
		{
			wxBitmap* theBitmap = obImgs_Mgr->GetBitmap(
				  t->lastImg
				, related_obj
				, t->key
				, t_zoom_factor
				, remap
				, b_flip
				, select_state
				);

			if( theBitmap == NULL )
			{
				t->key = wxT("noImg");
				b_flip = false;
				remap = 0;
				theBitmap = obImgs_Mgr->GetBitmap(
					  t->lastImg
					, related_obj
					, t->key
					, t_zoom_factor
					, remap
					, b_flip
					, select_state
					);
			}
				
			if( theBitmap == NULL )
			{
				cerr<<"BUG : Ob_GObject::GetBitmaps() : theImg (theBitmap == NULL)"<<endl;
				t->b_on_screen = false;
				t->theBitmap = NULL;
			}
			else
			{
				t->b_on_screen = true;
				t->theBitmap = theBitmap;
			}
		}
		
		//-------------------------------------------------
		// Build the Miniature
		float m_zoom_factor = MAP_ZF;
		if( t->id_atom == GATOM_CURSOR || t->id_atom == GATOM_AT )
			m_zoom_factor = (float) SCENE_MAP_HEIGHT
					  / ( (float) view_size.y * zoom_factor );
		else if( t->id_atom != GATOM_SHADOW 
			&& t->id_atom != GATOM_BODY
			&& t->id_atom != GATOM_SPAWN_POINT
			)
			m_zoom_factor = MAP_ZF / zoom_factor;
					 
		wxBitmap* theBitmap = obImgs_Mgr->GetBitmap(
			  t->img_Mini
			, related_obj
			, t->key
			, m_zoom_factor
			, remap
			, b_flip
			, select_state
			, true
			);

		if( theBitmap == NULL )
		{
			t->key = wxT("noImg");
			b_flip = false;
			remap = 0;
			theBitmap = obImgs_Mgr->GetBitmap(
				  t->img_Mini
				, related_obj
				, t->key
				, m_zoom_factor
				, remap
				, b_flip
				, select_state
				, true
				);
		}
		
		if( theBitmap == NULL )
		{
			cerr<<"BUG : Ob_GObject::GetBitmaps() : img_Mini (theBitmap == NULL)"<<endl;
			t->b_on_screen = false;
			t->bitmap_Mini = NULL;
		}
		else
			t->bitmap_Mini = theBitmap;
	}
	
	return l_atoms;
}


//-------------------------------------------------------
bool 
Ob_GObject::OnMouseEvent( Ob_GAtom* obAtom, wxSize _m_coords, wxMouseEvent& evt, bool multi_selection )
{
	// Dragging evt
	if( evt.Dragging() == true )
	{
		// compute the coords diff
		wxSize diffs( _m_coords.x - m_coords.x, _m_coords.y-m_coords.y );
		
		// Update coords
		m_coords = _m_coords;
		return ApplyMove( diffs, multi_selection );
	}
	
	
	// Not a dragging evt
	if( evt.Dragging() == false )
	{
		// LeftDown on the atom
		if( evt.LeftDown() ==  true )
		{
			if( obAtom != NULL )
				m_flag = obAtom->id_atom;
			m_coords = _m_coords;
			return false;
		}
	}
	return false;
}	

//-------------------------------------------------------
// Return true if the TAB controls must be updated
bool 
Ob_GObject::OnKeyEvent( wxKeyEvent& evt, bool multi_selection )
{
	// Do default move on keyEvent
	int kc = evt.GetKeyCode();
	wxSize diff(0,0);
	
	switch( kc )
	{
		case WXK_LEFT:
		case WXK_NUMPAD_LEFT:
			diff.x = -5;
			break;
			
		case WXK_RIGHT:
		case WXK_NUMPAD_RIGHT:
			diff.x = 5;
			break;
			
		case WXK_UP:
		case WXK_NUMPAD_UP:
			diff.y = -5;
			break;
			
		case WXK_DOWN:
		case WXK_NUMPAD_DOWN:
			diff.y = 5;
			break;
		
		default:
			return false;
	}
	
	if( evt.m_controlDown == true )
	{
		diff.x *= 3; 
		diff.y *= 3; 
	}
	else if( evt.m_shiftDown == true )
	{
		diff.x /= 3; 
		diff.y /= 3; 
	}

	return ApplyMove( diff, multi_selection );
}

	
//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
Ob_GObject__Spawn::Ob_GObject__Spawn( ob_stage_object* src )
:Ob_GObject(src)
{
}

//-------------------------------------------------------
Ob_GObject__Spawn::~Ob_GObject__Spawn()
{
}

//-------------------------------------------------------
list<Ob_GAtom*>*
Ob_GObject__Spawn::Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						)
{
	int direction = ((ob_stage*)curr_stageFile->obj_container)->direction;
	
	list<Ob_GAtom*>* res = new list<Ob_GAtom*>;

	ob_spawn* ent = (ob_spawn*) obj;
	ob_object* o_coords = obj->GetSubObject( wxT("coords") );
	ob_object* o_AT = obj->GetSubObject( wxT("at") );

	wxSize _coords = wxSize(0,0);
	int _a = 0;
	if( o_coords != NULL )
	{
		_coords = wxSize(StrToInt( o_coords->GetToken(0)),StrToInt( o_coords->GetToken(1)));
		_a = StrToInt( o_coords->GetToken(2));
	}
	int _at = 0;
	if( o_AT != NULL )
		_at = StrToInt( o_AT->GetToken(0) );

	//------------------------------------------
	// The Body
	Ob_GAtom* atom_body = new Ob_GAtom(GATOM_BODY);
	atom_body->referer = this;
	res->push_back( atom_body );
	int ent_y = 0;
	wxSize offsets; // Image Frame offset

	//------------------------------------------
	// The Image Size
	bool b_noImg = false;
	if( ent->entity_ref == NULL )
		b_noImg = true;
	
	wxString body_img_path = wxString();
	if( b_noImg == false )
	{
		body_img_path = ent->entity_ref->Get_AvatarImg_Path();
		if( body_img_path == wxString() )
			b_noImg = true;
	}
	
	wxImage* bodyImg = NULL;
	if( b_noImg == false )
	{
		bodyImg = obImgs_Mgr->GetImageSrc( obj, body_img_path );
		if( bodyImg == NULL || bodyImg->IsOk() == false )
			b_noImg = true;
	}

	if( b_noImg == true )
	{
		bodyImg = noImg;
		atom_body->key = wxT("noImg");
	}
	else
		atom_body->key = body_img_path;

	atom_body->size = wxSize(bodyImg->GetWidth(),bodyImg->GetHeight());

	//-------------------------------------------
	// compute flip state
	bool b_flip = true;//(direction==STAGE_RIGHT);
	if(  obj != NULL )
	{
		if( obj->GetSubObject_Token(wxT("flip")) == wxT("1") )
			b_flip = !b_flip;
	}


	//-------------------------------------------
	// The Coords
// 	offsets = wxSize(0, atom_body->size.y );
	if( ent->entity_ref == NULL ) // || b_noImg == true )
		offsets = wxSize(atom_body->size.x / 2, atom_body->size.y );
	else
	{
		offsets = ent->entity_ref->avatar_img_path_offset;
		if( b_flip == true )
			offsets.x = atom_body->size.x - offsets.x;
	}
	
	ent_y =  _coords.y - offsets.y; 
	// In a direction LEFT stage , things change...
	if( direction == STAGE_LEFT )
	{
		atom_body->coords.x = stage_w - _at - ob_screen_w
				- offsets.x + _coords.x
				;
	}
	else
		atom_body->coords.x = _coords.x + _at
				- offsets.x
				;

	atom_body->coords.y = ent_y - _a;
	atom_body->Z = _coords.y;
	atom_body->A = _a;
	


	//------------------------------------------
	// The Shadow
	wxImage* shadowImg = obImgs_Mgr->GetImageSrc( NULL, wxT("ent_shadow") );
	if( shadowImg != NULL )
	{
		Ob_GAtom* atom_shadow = new Ob_GAtom(GATOM_SHADOW);
		atom_shadow->referer = this;
		res->push_back( atom_shadow );

		atom_shadow->key = wxT("ent_shadow");
		
		// The Size
		atom_shadow->size = wxSize( shadowImg->GetWidth(), shadowImg->GetHeight());
		
		atom_shadow->coords.x =
			  atom_body->coords.x
			+ offsets.x
//  			+ atom_body->size.x / 2
			- shadowImg->GetWidth()/ 2;
			
		atom_shadow->coords.y =
			  ent_y 
 			+ offsets.y 
			- shadowImg->GetHeight() / 2;
		atom_shadow->Z = ent_y;
	}

	//------------------------------------------
	// The AT
	wxImage* atImg = 
		Get_Image_StageCursor(	view_size ,
						zoom_factor,
						select_state,
						15,
						40,
						0,
						40,
						wxColour( 0,0,255),
						wxT("AT")
						);

	if( atImg != NULL && select_state == SEL_ON && o_AT != NULL  )
	{
		Ob_GAtom* atom_AT = new Ob_GAtom(GATOM_AT);
		atom_AT->referer = this;
		res->push_back( atom_AT );
		
		atom_AT->key = wxT("Cursor_AT");
		atom_AT->size = wxSize(   floor((float)atImg->GetWidth() / zoom_factor)
						, view_size.y );
		
		atom_AT->coords = wxSize( _at - atom_AT->size.x / 2, view_coords.y );
		
		// In a direction LEFT stage , things change...
		if( direction == STAGE_LEFT )
			atom_AT->coords.x = stage_w - atom_AT->coords.x - atom_AT->size.x;
		
		atom_AT->Z = view_coords.x + view_size.x;
	}


	return res;
}



//-------------------------------------------------------
// Return true if the TAB controls must be updated
bool 
Ob_GObject__Spawn::ApplyMove( wxSize& diffs, bool multi_selection )
{
	if( diffs.x == 0 && diffs.y == 0 )
		return false;

	// Some variables
	ob_object* soCoords = obj->GetSubObject( wxT("coords") );
	if( soCoords == NULL )
	{
		soCoords = new ob_object( wxT("coords") );
		soCoords->SetToken( 0, wxT("0") );
		soCoords->SetToken( 1, wxT("0") );
		obj->Add_SubObj( soCoords );
	}
	else
	{
		if( soCoords->GetToken( 1 ) == wxString() )
			soCoords->SetToken( 1, wxT("0") );
	}

	ob_object* soAt = obj->GetSubObject( wxT("at") );
	if( soAt == NULL )
	{
		soAt = new ob_object( wxT("at") );
		soAt->SetToken( 0, wxT("0") );
		obj->Add_SubObj( soAt );
	}

	int x   = StrToInt( soCoords->GetToken(0));
	int y   = StrToInt( soCoords->GetToken(1));
	int alt = StrToInt( soCoords->GetToken(2));
	int at  = StrToInt( soAt->GetToken(0));

	// Relay changes in the object
	if( multi_selection == false )
	{
		if( m_flag == GATOM_BODY )
		{
			x  += diffs.x;
			alt-= diffs.y;
			if( alt < 0 )
				alt = 0;
			soCoords->SetToken(0, IntToStr(x));
			soCoords->SetToken(2, IntToStr(alt));
			((ob_stage_at*)obj)->Reorder_At();
			return true;
		}
		
		if( m_flag == GATOM_SHADOW )
		{
			x  += diffs.x;
			y  += diffs.y;
			soCoords->SetToken(0, IntToStr(x));
			soCoords->SetToken(1, IntToStr(y));
			return true;
		}
	}
	
	// IF( multi_selection == true || m_flag == GATOM_AT )
	int direction = ((ob_stage*)curr_stageFile->obj_container)->direction;
	if( direction == STAGE_LEFT )
		at -= diffs.x;
	else
		at += diffs.x;
	y  += diffs.y;
	soCoords->SetToken(1, IntToStr(y));
	soAt->SetToken(0, IntToStr(at));
	((ob_stage_at*)obj)->Reorder_At();
	return true;
}


//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
Ob_GObject__AT::Ob_GObject__AT( ob_stage_object* _src )
:Ob_GObject( _src )
{
	vert_truncate_up = 15;
	horiz_width_up = 0;
	switch( _src->object_type )
	{
		case SOBJ_WAIT:
		case SOBJ_BLOCKADE:
			barColour = wxColour(255,255,0);
			vert_truncate_down = 60;
			if( _src->object_type == SOBJ_WAIT )
			{
				horiz_width_down = 60;
				barLabel = wxT("WAIT");
			}
			else
			{
				horiz_width_down = 110;
				barLabel = wxT("BLOCKADE");
			}
			break;
		case SOBJ_CANJOIN:
		case SOBJ_NOJOIN:
			barColour = wxColour(120,255,0);
			vert_truncate_down = 50;
			horiz_width_down = 80;
			if( _src->object_type == SOBJ_CANJOIN )
				barLabel = wxT("Can-Join");
			else
				barLabel = wxT("No-Join");
			break;
		case SOBJ_SHADOWCOLOR:
		case SOBJ_SHADOWALPHA:
			barColour = wxColour(255,120,0);
			vert_truncate_down = 70;
			horiz_width_down = 80;
			if( _src->object_type == SOBJ_SHADOWCOLOR )
				barLabel = wxT("Sh-Color");
			else
				barLabel = wxT("Sh-Alpha");
			break;
		case SOBJ_LIGHT:
			barColour = wxColour(255,255,255);
			barLabel = wxT("LIGHT");
			vert_truncate_down = 55;
			horiz_width_down = 60;
			break;
		case SOBJ_SETPALETTE:
			barColour = wxColour(50,80,255);
			barLabel = wxT("Palette");
			vert_truncate_down = 45;
			horiz_width_down = 70;
			break;
		case SOBJ_GROUP:
			barColour = wxColour(0,160,255);
			barLabel = wxT("GROUP");
			vert_truncate_down = 45;
			horiz_width_down = 80;
			break;
	}
}

//-------------------------------------------------------
Ob_GObject__AT::~Ob_GObject__AT()
{
}

//-------------------------------------------------------
list<Ob_GAtom*>* 
Ob_GObject__AT::Get_GAtoms(
					wxSize view_coords ,
					wxSize view_size ,
					float  zoom_factor,
					int    select_state,
					int 	 stage_w
				)
{
	//------------------------------------------
	wxImage* theImg = 
		Get_Image_StageCursor(	view_size ,
						zoom_factor,
						select_state,
						vert_truncate_up,
						vert_truncate_down,
						horiz_width_up,
						horiz_width_down,
						barColour,
						barLabel
						);

	list<Ob_GAtom*>* res = new list<Ob_GAtom*>;
	if( theImg != NULL )
	{
		Ob_GAtom* theAtom = new Ob_GAtom(GATOM_CURSOR);
		theAtom->referer = this;
		res->push_back( theAtom );

		ob_object* o_AT = obj->GetSubObject( wxT("at") );
		int _at = 0;
		if( o_AT != NULL )
			_at = StrToInt( o_AT->GetToken(0) );
		int direction = ((ob_stage*)curr_stageFile->obj_container)->direction;
		
		theAtom->key = wxT("Cursor_") + barLabel;
		theAtom->size = wxSize(   floor((float)theImg->GetWidth() / zoom_factor)
						, view_size.y );
		
		theAtom->coords = wxSize( _at - theAtom->size.x / 2, view_coords.y );
		
		// In a direction LEFT stage , things change...
		if( direction == STAGE_LEFT )
			theAtom->coords.x = stage_w - theAtom->coords.x - theAtom->size.x;
		
		theAtom->Z = view_coords.x + view_size.x;
	}


	return res;
}


//-------------------------------------------------------
// Return true if the TAB controls must be updated
bool 
Ob_GObject__AT::ApplyMove( wxSize& diffs, bool multi_selection )
{
	if( diffs.x == 0 )
		return false;
	
	ob_object* soAt = obj->GetSubObject( wxT("at") );
	if( soAt == NULL )
	{
		soAt = new ob_object( wxT("at") );
		soAt->SetToken( 0, wxT("0") );
		obj->Add_SubObj( soAt );
	}

	int x   = StrToInt( soAt->GetToken(0));
	soAt->SetToken( 0, IntToStr(x + diffs.x) );
	((ob_stage_at*)obj)->Reorder_At();
	return true;
}



//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
Ob_GObject__ScrollZ::Ob_GObject__ScrollZ( ob_stage_object* _src )
:Ob_GObject( _src )
{
}

//-------------------------------------------------------
Ob_GObject__ScrollZ::~Ob_GObject__ScrollZ()
{
	wxString imgRectKey = wxT("Rect_") + IntToStr( (size_t) this );
	obImgs_Mgr->ReleaseImage( imgRectKey );
}

//-------------------------------------------------------
list<Ob_GAtom*>* 
Ob_GObject__ScrollZ::Get_GAtoms(
					wxSize view_coords ,
					wxSize view_size ,
					float  zoom_factor,
					int    select_state,
					int 	 stage_w
				)
{
	wxColour scroll_color( 50,255,50);
	
	int direction = ((ob_stage*)curr_stageFile->obj_container)->direction;
	
	ob_object* o_AT = obj->GetSubObject( wxT("at") );
	int _at = 0;
	if( o_AT != NULL )
		_at = StrToInt( o_AT->GetToken(0) );
	
	//------------------------------------------
	// The Cursor
	wxImage* theImg = 
		Get_Image_StageCursor(	view_size ,
						zoom_factor,
						select_state,
						60,
						75,
						0,
						60,
						scroll_color,
						wxT("ScrollZ")
						);

	list<Ob_GAtom*>* res = new list<Ob_GAtom*>;
	Ob_GAtom* atAtom = NULL;
	if( theImg != NULL )
	{
		atAtom = new Ob_GAtom(GATOM_CURSOR);
		atAtom->referer = this;
		res->push_back( atAtom );

		atAtom->key = wxT("Cursor_ScrollZ");
		atAtom->size = wxSize(   floor((float)theImg->GetWidth() / zoom_factor)
						, view_size.y );
		
		atAtom->coords = wxSize( _at - atAtom->size.x / 2, view_coords.y );
		
		// In a direction LEFT stage , things change...
		if( direction == STAGE_LEFT )
			atAtom->coords.x = stage_w - atAtom->coords.x - atAtom->size.x;
		
		atAtom->Z = view_coords.x + view_size.x;
	}

	//------------------------------------------
	// Compute the width of the scroll line
	// Find the next scroll_z object
	ob_object* sibling = obj->prev;
	ob_object *sibling_at;
	while( sibling != NULL )
	{
		if( sibling->type == OB_TYPE_STAGE_AT
			&& ((ob_stage_object*) sibling)->object_type == SOBJ_SCROLLZ
		  )
		{
			sibling_at = sibling->GetSubObject( wxT("AT") );
			if( 	   sibling_at != NULL 
				&& StrToInt( sibling_at->GetToken(0) ) > _at 
			  )
				break;
			sibling_at = NULL;
		}
		sibling = sibling->next;
	}
	
	// Compute the width 
	int line_width = stage_w - _at;
	if( sibling_at != NULL )
		line_width = StrToInt( sibling_at->GetToken(0) ) - _at;
	if( line_width < 0 )
		line_width = -line_width;

	//------------------------------------------
	// Get the line image
	wxString key_res;
	theImg = 
		Get_Image_Rect(   key_res,
					(size_t) this,
					view_size ,
					zoom_factor,
					select_state,
					line_width,
					4,
					scroll_color
				  );

	// Some img's ob_size needs
	int img_ob_H_on2 = (theImg->GetHeight()/2)/zoom_factor;
	// transparent spaces for selections borders
	int space_dec = 	- (4/MAP_ZF) / zoom_factor;
	
	//------------------------------------------
	// The Upper line
	Ob_GAtom* lineUpAtom = new Ob_GAtom(GATOM_SCROLL_LINE);
	lineUpAtom->referer = this;
	res->push_back( lineUpAtom );

	lineUpAtom->key = key_res;
	lineUpAtom->size = wxSize(line_width , 2 );
	
	if( direction == STAGE_LEFT )
		lineUpAtom->coords = wxSize( 
				  atAtom->coords.x + atAtom->size.x/2 + space_dec
					- line_width 
				, StrToInt(obj->GetToken(0))-img_ob_H_on2 + space_dec
						);
	else
		lineUpAtom->coords = wxSize(  
				  atAtom->coords.x + atAtom->size.x/2 + space_dec
				, StrToInt(obj->GetToken(0))-img_ob_H_on2 + space_dec
						   );
	lineUpAtom->Z = -(view_coords.x + view_size.x);

	//------------------------------------------
	// The Lower line
	Ob_GAtom* lineDownAtom = new Ob_GAtom(GATOM_SCROLL_LINE);
	lineDownAtom->referer = this;
	res->push_back( lineDownAtom );

	lineDownAtom->key = key_res;
	lineDownAtom->size = lineUpAtom->size;
	
	lineDownAtom->coords = wxSize( lineUpAtom->coords.x 
				, StrToInt( obj->GetToken(1))-img_ob_H_on2 + space_dec
					     );
	lineDownAtom->Z = -(view_coords.x + view_size.x);

	return res;
}


//-------------------------------------------------------------------------
// Return true if the TAB controls must be updated
bool 
Ob_GObject__ScrollZ::OnMouseEvent( Ob_GAtom* obAtom, wxSize _m_coords, wxMouseEvent& evt, bool multi_selection )
{
	bool res=Ob_GObject::OnMouseEvent( obAtom, _m_coords, evt, multi_selection );
	
	// Event have been done by UpperClass
	if( res == true )
		return true;
	
	if( multi_selection == true )
		return false;
	
	// Must guess which part of the scrollZ have been touched
	ob_object* soAt = obj->GetSubObject( wxT("at") );
	if( soAt == NULL )
	{
		soAt = new ob_object( wxT("at") );
		soAt->SetToken( 0, wxT("0") );
		obj->Add_SubObj( soAt );
	}
	int at    = StrToInt( soAt->GetToken(0));
	int z_min = StrToInt( obj->GetToken(0));
	int z_max = StrToInt( obj->GetToken(1));
	
	int at_dist    = M_DIST( _m_coords.x, at );
	if( at_dist < 3 )
	{
		m_flag = GATOM_AT;
		return false;
	}
	
	int z_min_dist = M_DIST( _m_coords.y, z_min );
	int z_max_dist = M_DIST( _m_coords.y, z_max );
	if( at_dist < z_min_dist && at_dist & z_max_dist )
		m_flag = GATOM_AT;
	else if( z_min_dist < z_max_dist )
		m_flag = GATOM_SHADOW;
	else
		m_flag = GATOM_BODY;
	return false;
}


//-------------------------------------------------------
// Return true if the TAB controls must be updated
bool 
Ob_GObject__ScrollZ::ApplyMove( wxSize& diffs, bool multi_selection )
{
	if( diffs.x == 0 && diffs.y == 0 )
		return false;
	
	ob_object* soAt = obj->GetSubObject( wxT("at") );
	if( soAt == NULL )
	{
		soAt = new ob_object( wxT("at") );
		soAt->SetToken( 0, wxT("0") );
		obj->Add_SubObj( soAt );
	}
	
	//---------------------------------------------------
	// AT move case
	int at    = StrToInt( soAt->GetToken(0));
	if( multi_selection == true || m_flag == GATOM_AT )
	{
		if( diffs.x == 0 )
			return false;
		soAt->SetToken( 0, IntToStr( at + diffs.x ));
		((ob_stage_at*)obj)->Reorder_At();
		return true;
	}
	
	//---------------------------------------------------
	// BOUNDS move case
	if( diffs.y == 0 )
		return false;
		
	if( m_flag == GATOM_SHADOW )
	{
		int z_min = StrToInt( obj->GetToken(0));
		obj->SetToken( 0, IntToStr( z_min + diffs.y ));
		return true;
	}
	
	int z_max = StrToInt( obj->GetToken(1));
	obj->SetToken( 1, IntToStr( z_max + diffs.y ));
	return true;
}



//*************************************************************************
//*************************************************************************
//*************************************************************************
//-------------------------------------------------------------------------
Ob_GObject__PlayerSpawn::Ob_GObject__PlayerSpawn( ob_stage_object* _src )
:Ob_GObject( _src )
{
}

//-------------------------------------------------------

Ob_GObject__PlayerSpawn::~Ob_GObject__PlayerSpawn()
{
}


//-------------------------------------------------------
list<Ob_GAtom*>* 
Ob_GObject__PlayerSpawn::Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						)
{
	// The result
	list<Ob_GAtom*>* res = new list<Ob_GAtom*>;
	
	// Some vars...
	int num_player = StrToInt( obj->name.Right(obj->name.Len() - 5 ) );
	wxString imgKey = wxT("pSpawn_") + IntToStr(num_player) + wxT(".png");

	int direction = ((ob_stage*)curr_stageFile->obj_container)->direction;
	wxSize s_bounds( 
		((ob_stage*)curr_stageFile->obj_container)
			->GetDefScrollzBounds( curr_stage_declaration ) );
	wxSize _coords( StrToInt(obj->GetToken(0)), StrToInt(obj->GetToken(1)) );
	_coords.x = _coords.x%320;
	_coords.y += s_bounds.x;
	int alt = StrToInt(obj->GetToken(2));
	

	//------------------------------------------
	// The aim img
	Ob_GAtom* atom_aim = NULL;
	wxImage* aimImg = obImgs_Mgr->GetImageSrc( NULL, imgKey );
	if( aimImg != NULL )
	{
		atom_aim = new Ob_GAtom(GATOM_SPAWN_POINT);
		atom_aim->referer = this;
		res->push_back( atom_aim );

		atom_aim->key = imgKey;
		atom_aim->size = wxSize( aimImg->GetWidth(), aimImg->GetHeight());
		atom_aim->coords = wxSize( _coords.x - atom_aim->size.x/2
					    , _coords.y - alt - atom_aim->size.y/2);
		atom_aim->Z = _coords.y - atom_aim->size.y/2;
		
		// In a direction LEFT stage , things change...
		if( direction == STAGE_LEFT )
			atom_aim->coords.x = stage_w - atom_aim->coords.x - atom_aim->size.x;
	}

	
	//------------------------------------------
	// The Shadow
	wxImage* shadowImg = obImgs_Mgr->GetImageSrc( NULL, wxT("ent_shadow") );
	if( shadowImg != NULL )
	{
		Ob_GAtom* atom_shadow = new Ob_GAtom(GATOM_SHADOW);
		atom_shadow->referer = this;
		res->push_back( atom_shadow );

		atom_shadow->key = wxT("ent_shadow");
		
		// The Size
		atom_shadow->size = wxSize( shadowImg->GetWidth(), shadowImg->GetHeight());
		
		atom_shadow->coords.x =
			  _coords.x
			- shadowImg->GetWidth()/ 2;
			
		atom_shadow->coords.y =
			  _coords.y
			- shadowImg->GetHeight() / 2;
		atom_shadow->Z = _coords.y - shadowImg->GetHeight();
		// In a direction LEFT stage , things change...
		if( direction == STAGE_LEFT )
			atom_shadow->coords.x = stage_w - atom_shadow->coords.x - atom_shadow->size.x;
	}

	return res;
}

//-------------------------------------------------------
// Return true if the TAB controls must be updated
bool 
Ob_GObject__PlayerSpawn::ApplyMove( wxSize& diffs, bool multi_selection )
{
	if( diffs.x == 0 && diffs.y == 0 )
		return false;
	
	int x = StrToInt(obj->GetToken( 0 )) + diffs.x;
	obj->SetToken( 0, IntToStr(x) );
	int y = StrToInt(obj->GetToken( 1 )) + diffs.y;
	obj->SetToken( 1, IntToStr(y) );
	
	return true;
}


//*************************************************************************
//*************************************************************************
//*************************************************************************

int OB_LINE_PEN_WIDTH = 2;

static inline
void
ANN__Draw_Draggable_Point( wxDC& theDC, wxSize& pt, float& zoom_factor )
{
	theDC.DrawRectangle( 	(pt.x - pt_radius)*zoom_factor,
					(pt.y - pt_radius)*zoom_factor,
					pt_radius*2*zoom_factor,
					pt_radius*2*zoom_factor
					);
}

//-------------------------------------------------------------------------
static inline
void
ANN__Draw_Line( wxDC& theDC, wxSize& pt0, wxSize& pt1, float& zoom_factor )
{
	theDC.DrawLine( pt0.x*zoom_factor,
			    pt0.y*zoom_factor,
			    pt1.x*zoom_factor,
			    pt1.y*zoom_factor
			    );
}


//*************************************************************************
//*************************************************************************
//*************************************************************************
wxColour Ob_GObject__Wall::theColor( 255,200,255 );

//-------------------------------------------------------------------------
Ob_GObject__Wall::Ob_GObject__Wall( ob_stage_object* _src )
:Ob_GObject( _src )
{
	if( _src->name.Upper() == wxT("WALL") )
		m_type = OBG_WALL;
	else if( _src->name.Upper() == wxT("HOLE") )
		m_type = OBG_HOLE;
	else
	{
		cerr<<"BUG!!! : Ob_GObject__Wall() : name unhandled : "<<_src->name<<endl;
		m_type = OBG_WALL;
	}
	
	min_x = 0;
	max_x = 0;
	spaces = 0;
	total_h = 0;
}

//-------------------------------------------------------

Ob_GObject__Wall::~Ob_GObject__Wall()
{
	obImgs_Mgr->ReleaseImage( obj->name.Upper() + wxT("_") + IntToStr((size_t)this) );
}


//-------------------------------------------------------
list<Ob_GAtom*>* 
Ob_GObject__Wall::Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						)
{
	// The result
	list<Ob_GAtom*>* res = new list<Ob_GAtom*>;
	
	// Some vars...
	wxString imgKey = obj->name.Upper() + wxT("_")+IntToStr((size_t)this);

/*	int direction = ((ob_stage*)curr_stageFile->obj_container)->direction;*/

	//---------------------------------------------
	// Getting Wall coords
	int X = StrToInt( obj->GetToken(0));
	int Z = StrToInt( obj->GetToken(1));
	int ul = StrToInt( obj->GetToken(2));
	int ll = StrToInt( obj->GetToken(3));
	int ur = StrToInt( obj->GetToken(4));
	int lr = StrToInt( obj->GetToken(5));
	int depth = StrToInt( obj->GetToken(6));
	int alt   = 0;
	if( m_type == OBG_WALL )
		alt = StrToInt( obj->GetToken(7));


	//---------------------------------------------
	// Compute the resulting img size
	
	// Spaces for letting spaces to selection pixels
	spaces = (4/MAP_ZF);
	
	// Normalize Wall values
	if( lr < ll )
		lr = ll;
	if( ur < ul )
		ur = ul;
	if( depth <= 0 )
		depth = 1;
	if( alt <= 0 && m_type == OBG_WALL)
		alt = 1;
	
	// Get bounds
	min_x = min( ll, ul );
	max_x = max( lr, ur );
	total_h = depth + alt;
	
	wxSize img_size_ob( max_x - min_x + spaces*2, total_h + spaces * 2 );
	wxSize img_size_real( img_size_ob.x * zoom_factor
				  , img_size_ob.y * zoom_factor );
	
	//---------------------------------------------
	// Check if last computed img is already valid
	wxImage* theImg = NULL;
	wxString wall_state = IntToStr(ul)
	+wxT(",")+IntToStr(ul)
	+wxT(",")+IntToStr(ll)
	+wxT(",")+IntToStr(ur)
	+wxT(",")+IntToStr(lr)
	+wxT(",")+IntToStr(depth)
	+wxT(",")+IntToStr(alt);
	if( last_wall_state != wall_state )
	{
		last_wall_state = wall_state;
		obImgs_Mgr->ReleaseImage( imgKey );
	}
	else
	{
		theImg = obImgs_Mgr->GetImageSrc( NULL, imgKey );
		if( theImg != NULL )
		{
			if( 	   M_DIST(theImg->GetHeight(),img_size_real.y) > 1
				|| M_DIST(theImg->GetWidth() ,img_size_real.x) > 1
			)
			{
				obImgs_Mgr->ReleaseImage( imgKey );
				theImg = NULL;
			}
		}
	}

	//------------------------------------------
	// Must build/reBuild the img
	if( theImg == NULL )
	{
		//-----------------------------
		// Prepare DC / Bitmap
		//-----------------------------
		wxBitmap btmp( img_size_real.x, img_size_real.y );
		wxMemoryDC theDC( btmp );
		
		theDC.SetBackground( ObImgs_Manager::def_mask_colour );
		theDC.Clear();
		theDC.SetPen( wxPen( theColor, OB_LINE_PEN_WIDTH ) ); 

		//-----------------------------
		// First Place points
		//-----------------------------
		theDC.SetBrush( wxBrush( theColor ) ); 
		
		// Compute points coords
		// LL
		pts[LL] = wxSize( ll - min_x+ spaces, total_h + spaces );
		// LR
		pts[LR] = wxSize( lr - min_x+ spaces, total_h + spaces );
		// UL
		pts[UL] = wxSize( ul - min_x+ spaces, total_h - depth + spaces );
		// UR
		pts[UR] = wxSize( ur - min_x+ spaces, total_h - depth + spaces );

		// TOP point for Computation
		for( int i = TLL; i <= TUR; i++ )
			pts[i] = wxSize( pts[i-4].x, pts[i-4].y - alt );

		// DEPTH
		pt_depth = wxSize( (pts[UL].x + pts[UR].x)/2
				,  (pts[UL].y + pts[UR].y)/2 );
		// ALT
		if( m_type == OBG_WALL )
			pt_alt = wxSize( (pts[TUL].x + pts[TUR].x)/2
					,  (pts[TUL].y + pts[TUR].y)/2 );
		
		
		// Draw the bottom points
		for( int i = 0; i <= UR; i++ )
			ANN__Draw_Draggable_Point( theDC, pts[i], zoom_factor );
		
		//Draw the ALT point
		if( m_type == OBG_WALL )
			ANN__Draw_Draggable_Point( theDC,pt_alt, zoom_factor );
		
		//Draw the DEPTH point
		ANN__Draw_Draggable_Point( theDC,pt_depth, zoom_factor );
		
		//-----------------------------
		// Second Trace Lines
		//-----------------------------
		theDC.SetBrush( wxBrush( theColor ) ); 
		// LL-LR
		ANN__Draw_Line( theDC,pts[LL],pts[LR], zoom_factor );
		// LL-UL
		ANN__Draw_Line( theDC,pts[LL],pts[UL], zoom_factor );
		// LR-UR
		ANN__Draw_Line( theDC,pts[LR],pts[UR], zoom_factor );
		// UL-UR
		ANN__Draw_Line( theDC,pts[UL],pts[UR], zoom_factor );
		
		// WALL vertices
		if( m_type == OBG_WALL )
		{
			// UL-TUL
			ANN__Draw_Line( theDC,pts[UL],pts[TUL], zoom_factor );
			// LL-TLL
			ANN__Draw_Line( theDC,pts[LL],pts[TLL], zoom_factor );
			// LR-TLR
			ANN__Draw_Line( theDC,pts[LR],pts[TLR], zoom_factor );
			// UR-TUR
			ANN__Draw_Line( theDC,pts[UR],pts[TUR], zoom_factor );
			
			// TLL-TLR
			ANN__Draw_Line( theDC,pts[TLL],pts[TLR], zoom_factor );
			// TLL-TUL
			ANN__Draw_Line( theDC,pts[TLL],pts[TUL], zoom_factor );
			
			// TLR-TUR
			ANN__Draw_Line( theDC,pts[TLR],pts[TUR], zoom_factor );
			
			// TUL-TUR
			ANN__Draw_Line( theDC,pts[TUL],pts[TUR], zoom_factor );
		}
		
		//-----------------------------
		// Convert to img, and insert in DB
		//-----------------------------
		// Remove the Bitmap from the DC
		theDC.SelectObject( wxNullBitmap );
		
		// Convert the Bitmap to an image
		wxImage* res = new wxImage( btmp.ConvertToImage() );
		if( res->IsOk() == false )
		{
			delete res;
			res = new wxImage( *noImg );
			res->Rescale(img_size_real.x, img_size_real.y );
		}
		else
			res->SetMaskColour( 
				  ObImgs_Manager::def_mask_colour.Red()
				, ObImgs_Manager::def_mask_colour.Green()
				, ObImgs_Manager::def_mask_colour.Blue()
				);
		
	//	res->Resize( wxSize(btmp_w, clientSize.y), wxPoint(0,0)  );
		obImgs_Mgr->AbsorbeImage( imgKey, NULL, res, MASK_IN_IMAGE );
	}

	//--------------------------------------------
	// build the resulting atom
	Ob_GAtom* theAtom = NULL;
	theAtom = new Ob_GAtom(GATOM_OTHER);
	theAtom->referer = this;
	res->push_back( theAtom );

	theAtom->key = imgKey;
	theAtom->size = img_size_ob;
	
	theAtom->coords = wxSize( X + min_x - spaces
					  , Z - total_h - spaces
					  );
	theAtom->Z = 0;
	
/*	// In a direction LEFT stage , things change...
	if( direction == STAGE_LEFT )
		theAtom->coords.x = stage_w - theAtom->coords.x - theAtom->size.x;
*/
	return res;
}


//-------------------------------------------------------
bool 
Ob_GObject__Wall::OnMouseEvent( Ob_GAtom* obAtom, wxSize _m_coords, wxMouseEvent& evt, bool multi_selection )
{
	bool res=Ob_GObject::OnMouseEvent( obAtom, _m_coords, evt, multi_selection );
	
	// Event have been done by UpperClass
	if( res == true )
		return true;
	
	if( multi_selection == true )
		return false;
	
	//---------------------------------------------
	// Must figure out wich part of the Wall/Hole is being targeted
	m_flag = -1;
	
	int X = StrToInt( obj->GetToken(0));
	int Z = StrToInt( obj->GetToken(1));
	_m_coords.x = _m_coords.x - X - min_x+ spaces;
	_m_coords.y = total_h - (Z - _m_coords.y) + spaces;
	
	// Try the bottom points
	for( int i = 0; i <= UR; i++ )
	{
		if( 	   M_DIST(pts[i].x,_m_coords.x) < DIST_DETECT
			&& M_DIST(pts[i].y,_m_coords.y) < DIST_DETECT
		  )
		{
			m_flag = i;
			return false;
		}
	}

	//Try the DEPTH point
	if( 	   M_DIST(pt_depth.x,_m_coords.x) < DIST_DETECT
		&& M_DIST(pt_depth.y,_m_coords.y) < DIST_DETECT
		)
	{
		m_flag = UR+1;
		return false;
	}
	

	//Perhaps the ALT point ?
	if( m_type == OBG_WALL )
	{
		if( 	   M_DIST(pt_alt.x,_m_coords.x) < DIST_DETECT
			&& M_DIST(pt_alt.y,_m_coords.y) < DIST_DETECT
		  )
		{
			m_flag = UR+2;
			return false;
		}
	}
	
	return false;
}


//-------------------------------------------------------
bool 
Ob_GObject__Wall::ApplyMove( wxSize& diffs, bool multi_selection )
{
	if( diffs.x == 0 && diffs.y == 0 )
		return false;
	
	//---------------------------------------------
	// Getting Wall coords
	int X = StrToInt( obj->GetToken(0));
	int Z = StrToInt( obj->GetToken(1));
	int ul = StrToInt( obj->GetToken(2));
//	int ll = StrToInt( obj->GetToken(3));
	int ur = StrToInt( obj->GetToken(4));
	int lr = StrToInt( obj->GetToken(5));
	int depth = StrToInt( obj->GetToken(6));
	int alt   = 0;
	if( m_type == OBG_WALL )
		alt = StrToInt( obj->GetToken(7));

	//---------------------------------------------------
	// Global move case
	if( multi_selection == true || m_flag <= 0  )
	{
		obj->SetToken( 0, IntToStr( X + diffs.x ));
		obj->SetToken( 1, IntToStr( Z + diffs.y ));
	}
	
	else if( m_flag == LR  )
	{
		if( diffs.x == 0 )
			return false;
		obj->SetToken( 5, IntToStr( lr + diffs.x ));
	}
	else if( m_flag == UL  )
	{
		if( diffs.x == 0 )
			return false;
		obj->SetToken( 2, IntToStr( ul + diffs.x ));
	}
	else if( m_flag == UR  )
	{
		if( diffs.x == 0 )
			return false;
		obj->SetToken( 4, IntToStr( ur + diffs.x ));
	}
	else if( m_flag == UR+1  )
	{
		if( diffs.y == 0 )
			return false;
		obj->SetToken( 6, IntToStr( depth - diffs.y ));
	}
	else if( m_flag == UR+2 && m_type == OBG_WALL )
	{
		if( diffs.y == 0 )
			return false;
		obj->SetToken( 7, IntToStr( alt - diffs.y ));
	}
		
	return true;
}

