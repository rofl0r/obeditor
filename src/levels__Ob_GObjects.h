#ifndef LEVELS__OB_GOBJECTS_H
#define LEVELS__OB_GOBJECTS_H

#include <wx/bitmap.h>
#include <wx/image.h>
#include <list>

#include "common__object_stage.h"

//*****************************************************************
class Ob_GObject;
//-----------------------------------------------------------------
enum
{
	GATOM_AT,
	GATOM_SHADOW,
	GATOM_BODY,
	GATOM_CURSOR,
	GATOM_SCROLL_LINE,
	GATOM_SPAWN_POINT,
	GATOM_OTHER, // Wall Hole
};

class Ob_GAtom
{
public:
	Ob_GAtom(int atom_id);
	virtual ~Ob_GAtom(){};
	wxString ToStr();
	
	Ob_GObject* referer;
	
	int id_atom;
	wxString key;
	wxSize coords;
	wxSize size;
	wxSize view_coords;
	bool b_on_screen;
	wxBitmap* theBitmap;
	wxImage*  lastImg;
	wxBitmap* bitmap_Mini;
	wxImage*  img_Mini;
	int Z;
	int A;
};


//-----------------------------------------------------------------
class Ob_GObject
{
public:
	static Ob_GObject* GuessAndConstruct( ob_stage_object* src );
	
public:
	Ob_GObject( ob_stage_object* src ){obj = src;};
	virtual ~Ob_GObject(){};

	std::list<Ob_GAtom*>*
			GetBitmaps(
				wxSize view_coords ,
				wxSize view_size ,
				float  zoom_factor,
				int    select_state,
			      int 	 stage_w
				);
	
	// Return true if the TAB controls must be updated
	virtual bool OnMouseEvent( Ob_GAtom* obAtom, wxSize _m_coords, wxMouseEvent& evt, bool multi_selection );
	
	// Return true if the TAB controls must be updated
	virtual bool OnKeyEvent( wxKeyEvent& evt, bool multi_selection );

	// Return true if the TAB controls must be updated
	virtual bool ApplyMove( wxSize& diffs, bool multi_selection ){return false;};

	ob_stage_object* obj;
protected:
	virtual std::list<Ob_GAtom*>* Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						) = 0;
	int m_flag;
	wxSize m_coords;
};

//-----------------------------------------------------------------
class Ob_GObject__Spawn : public Ob_GObject
{
public:
	Ob_GObject__Spawn( ob_stage_object* src );
	virtual ~Ob_GObject__Spawn();

	virtual bool ApplyMove( wxSize& diffs, bool multi_selection );

protected:
	std::list<Ob_GAtom*>* Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						);
};

//-----------------------------------------------------------------
class Ob_GObject__AT : public Ob_GObject
{
public:
	Ob_GObject__AT( ob_stage_object* src );
	virtual ~Ob_GObject__AT();

	virtual bool ApplyMove( wxSize& diffs, bool multi_selection );

protected:
	wxColour barColour;
	wxString barLabel;
	int	 vert_truncate_up;
	int	 vert_truncate_down;
	int	 horiz_width_up;
	int	 horiz_width_down;
	std::list<Ob_GAtom*>* Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						);
};


//-----------------------------------------------------------------
class Ob_GObject__ScrollZ : public Ob_GObject
{
public:
	Ob_GObject__ScrollZ( ob_stage_object* src );
	virtual ~Ob_GObject__ScrollZ();

	virtual bool OnMouseEvent( Ob_GAtom* obAtom, wxSize _m_coords, wxMouseEvent& evt, bool multi_selection );
	virtual bool ApplyMove( wxSize& diffs, bool multi_selection );

protected:
	std::list<Ob_GAtom*>* Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						);
};


//-----------------------------------------------------------------
class Ob_GObject__PlayerSpawn : public Ob_GObject
{
public:
	Ob_GObject__PlayerSpawn( ob_stage_object* src );
	virtual ~Ob_GObject__PlayerSpawn();

	virtual bool ApplyMove( wxSize& diffs, bool multi_selection );

protected:
	std::list<Ob_GAtom*>* Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						);
};



//-----------------------------------------------------------------
class Ob_GObject__Wall : public Ob_GObject
{
public:
	static wxColour theColor;
	
public:
	Ob_GObject__Wall( ob_stage_object* src );
	virtual ~Ob_GObject__Wall();

	virtual bool OnMouseEvent( Ob_GAtom* obAtom, wxSize _m_coords, wxMouseEvent& evt, bool multi_selection );
	virtual bool ApplyMove( wxSize& diffs, bool multi_selection );

protected:
	std::list<Ob_GAtom*>* Get_GAtoms(
							wxSize view_coords ,
							wxSize view_size ,
							float  zoom_factor,
							int    select_state,
							int 	 stage_w
						);
	// Only the first 4 are draguables
	wxSize pts[8]; 
	wxSize pt_alt;
	wxSize pt_depth;
	wxString last_wall_state;
	int min_x;
	int max_x;
	int spaces;
	int total_h;
	
	enum{
		LL = 0,
		LR = 1,
		UL = 2,
		UR = 3,
		TLL = 4,
		TLR = 5,
		TUL = 6,
		TUR = 7,
	};
	
	enum{
		OBG_WALL,
		OBG_HOLE,
	}
	m_type;
};

/*
//-----------------------------------------------------------------
class Ob_GObject__Hole : public Ob_GObject
{
};
*/




//-----------------------------------------------------------------
#endif //LEVELS__OB_GOBJECTS_H
