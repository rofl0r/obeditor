#ifndef LEVELS__CTRLSTAGEVIEW_H
#define LEVELS__CTRLSTAGEVIEW_H

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/tooltip.h>

#include <list>
#include <map>

#include "common__object_stage.h"


//-----------------------------------------------------------------
class Ob_GObject;
class Ob_GAtom;
//*****************************************************************
//-----------------------------------------------------------------
class StageView : public wxControl
{
public:
	StageView(Panel_StageView* _parent );
	virtual ~StageView();
	void Reset();
	void Reset_Ob_GObjects();
	void Reset_Panels();
	
	void Reload_Panels();
	virtual void Refresh(bool eraseBackground = true, const wxRect* rect = NULL);
	
	void SetViewCoords( wxSize new_coords );	// OB Coords
	void ZoomP(const size_t i = 1 );
	void ZoomM(const size_t i = 1);
	void ZoomSet(float new_zoom_factor );
	
	wxSize Get_ObSize();

	void Delete_GuiObject_of( ob_stage_object* o );
	
protected:
	wxBitmap*	Get_Panels_Bitmap( 
				const int   ob_x,
				const int   ob_y,
				const int   client_w,
				const int   client_h,
				const float zoom_factor
				);

public:
	/**
	 * The parent
	 */
	Panel_StageView* psv;
	
	/**
	 * Paint General Datas
	 */
	wxSize   coords;
	wxSize   panels_size;
	float    zoom_factor;

	wxColour background_color;

protected:
	/**
	 * Some last Painting Infos
	 */
	wxSize  last_clientSize;	// last GetClientSize()
	wxSize  last_Coords;		// last coords
	float	  last_zoom_factor;	// last zoom_factor
	
	/**
	 * Miscelanious Stuffs
	 */
	void SCROLLBARS_Update();
	void Zoom__ReComputeCoords( float old_zoom_factor );
	void Normalize_Coords();
	// The Pen for drawing selection rectangle
	wxPen pen_rect_sel;

	/**
	 * Panels Datas
	 */
	typedef struct
	{
		ob_stage_panel* ob_panel;
		wxString img_key;
		int ob_x;
		int ob_y;
		int ob_w;
		int ob_h;
		int temp_off_x;
		int temp_off_y;
		int temp_w;
		int temp_h;
	}
	Ob_Panel_Data;
	
	size_t ob_panels__size;
	Ob_Panel_Data** ob_panels;
	
	wxBitmap* panels_bitmap;
	
	wxToolTip* theTip;
	enum
	{
		// If the last Mouse Left Button down apply to a valid object
		M_MLB_OK 		= 1,
		M_HAVE_DRAGGED 	= 2,
		M_RECT_SEL 		= 4,
		M_RECT_SEL_START = 8,
		M_MUST_RM_ON_UP  = 16,
//		M_MUST_SET_ON_UP = 32,
	};
	int f_mouse;
	
	// Rectangular selection vars and functions
	void   ANN__CheckRectSelection();
	wxSize mouse_selRect_ob_coords_start;
	wxSize mouse_selRect_ob_coords_end;
	// Hold the object which have been included by the rectangular selection
	std::set<ob_stage_object*>* pset_obj_in_sel_rect;
	
public:
	/**
	 * Objects Datas
	 */
	std::map<ob_stage_object*,Ob_GObject*> map_GObjs;
	std::list<Ob_GAtom*> l_atoms;	// List of atoms currently drawn
	
protected:
	void Init_PanelImage( size_t curr_ind );
	void Init_PanelData( size_t curr_ind );

	Ob_GAtom* Get_Aimed_Atom(int x, int y);
	bool ANN__SendMouseEvent_to_AllSelected( 
				wxSize& _m_coords, wxMouseEvent& evt, bool multi_selection );
	bool ANN__SendKeyEvent_to_AllSelected( wxKeyEvent& evt, bool multi_selection );

protected:
	void OnPaint( wxPaintEvent& _evt );
	void EvtSize( wxSizeEvent& _evt );
	void EvtScroll( wxScrollWinEvent& evt );
	void EvtMouse( wxMouseEvent& evt );
	void EvtChar( wxKeyEvent& evt );
	
	DECLARE_EVENT_TABLE()
};


#endif //LEVELS__CTRLSTAGEVIEW_H
