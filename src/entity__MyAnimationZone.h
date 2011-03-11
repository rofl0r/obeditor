/*
 * MyAnimationZone.h
 *
 *  Created on: 7 nov. 2008
 *      Author: pat
 */

#ifndef MYANIMATIONZONE_H_
#define MYANIMATIONZONE_H_

#include <wx/control.h>
#include <wx/timer.h>
#include <wx/dcbuffer.h>

//****************************************************
//       The Control for the animation
struct jump_stair;

class MyAnimationZone : public wxControl
{
public:
	static wxColour orig_bg;

public:
	MyAnimationZone(wxWindow *parent);
	virtual ~MyAnimationZone();

	virtual void OnEraseBackground(wxEraseEvent& WXUNUSED(event));
	virtual int  GetIndFrameToPaint();
	virtual void PaintBackground();
	virtual void OnPaint( wxPaintEvent& WXUNUSED(event));
	virtual void PreProcessImage( wxImage& fr_img );
	virtual void EvtSize( wxSizeEvent& event );
	virtual void EvtGetFocus(wxFocusEvent&);
	virtual void Evt_Scroll(wxScrollWinEvent&);

	virtual void SetSize(int x, int y, int width, int height, int sizeFlags = wxSIZE_AUTO);
	virtual void UpdateFrames();
	virtual void Clear();
	virtual void RePaint();
	virtual void Refresh();
	virtual void ResetMouseDraw();

	virtual void OnSimpleClick(wxMouseEvent& event);
	virtual void OnEndLeftDragging();
	virtual void OnDragging();
	virtual void OnAnyButtonDown();
	virtual void OnLeftButtonDown();

	void Zoom(float _factor, bool b_exact_factor = false, bool b_recalc_size = true );
	void ResetZoom();

	int H()const;
	int W()const;
	int X()const;
	int Y()const;

	void ReverseFPCoord( int& _mx, int _mw = 0 );
	void ReverseDMCoord( int& mx, int& my);

	// Ob relative calculated mouse draws
	int TO_offx;
	int TO_offy;
	int TO_x;
	int TO_a;
	int TO_w;
	int TO_h;

protected:
	wxToolTip* theTip;
	
	void pGetClientSize( int* w,int* h);
	virtual wxSize DoGetBestSize() const;
	void ReCalcAnimSize();
	void Draw_NoImg();
	void myDrawLine(  int _x0, int _y0, int _x1, int _y1 );
	void myDrawRect(int _x, int _y, int _w, int _h, bool b_label_reserve = false );
	void myDrawLabel( const wxString& text, int _x,int  _y );
	void myDrawCircle(int _x,int  _y, int rayon );

	// Events
	virtual void EvtMouseSomething(wxMouseEvent& event);

	void _Recalculate_JumpStuffs();


	float zoom_factor;

	// Animation data
	int prev_frame_rect_x, prev_frame_rect_y, prev_frame_rect_w, prev_frame_rect_h;

	// The size of the area for the anim
	int total_W;
	int total_H;
	int min_decal_X;
	int min_decal_Y;

	// The offset of the first frame in the calculated area
	int total_decal_X;
	int total_decal_Y;

	// The decalages between the frames
	int* fr_decal_Xs;
	int* fr_decal_Ys;
	int* fr_decal_Zs;

	// The decalages relative to the first frames
	int* fr_total_decal_Xs;
	int* fr_total_decal_Ys;

	// The offset for each frame
	int* fr_offset_Xs;
	int* fr_offset_Ys;

	// The offset for each frame before rotation
	int* fr_rotate_offset_Xs;
	int* fr_rotate_offset_Ys;

	// The decals between the offset and the offset without draw method transformation
	int* fr_DM_decal_Xs;
	int* fr_DM_decal_Ys;

	// The image dimensions
	int *original_img_w;
	int *original_img_h;
	int *current_img_w;
	int *current_img_h;
	int *fr_rotate_imgw;
	int *fr_rotate_imgh;

	// The total altitude for the frames
	int* fr_As;
	// The Z decal for the frames
	int* fr_Zdecal;

	// The recalculated delays between frames
	int *fr_delays;

	// The frames datas of a jump
	int *fr_jumph;
	int *fr_jumpx;
	int *fr_jumpz;
	int *fr_jump_timer;
	jump_stair *fr_jump_stairs;
	int nb_stair;

	// Bunch of var for animate the jump
	bool is_in_jump;
	float jump_decal_x, jump_decal_z;
	int jump_decal_alt;
	int jump_alt_step;
	int jump_last_delay, jump_time_counter;
	bool draw_jump_step;

	enum
	{
		M_ZOOMING  	 = 1,
		M_PAINTING 	 = 2,
		M_REDRAW_ALL = 4,
	};
	int m_flag;

	// Painting vars : decalages to easyliy draw with zoom and offsets
	int decal_x, decal_y;
	int dc_x, dc_y;
	bool flipped_fr;
	int ind_frPainted; // the indice of the current painted frame

	// What the mouse have draw
	bool b_mouseD;
	int mouse_draw;
	int r_x, r_y, r_h, r_w; 	// coord for the mouse draw in anim coord

	// same for right button
	bool b_rightmouseD;
	int r_right_x, r_right_y;

	wxAutoBufferedPaintDC* dc;
	wxBrush brush;
	wxPen pen;

	// The bitmap image is not owned by MyImageCtrl
	wxBitmap *theBitmap;
	DECLARE_EVENT_TABLE()
};


//****************************************************
//       The Control for the animation
class MyAnimationCtrl : public MyAnimationZone
{
public:
	MyAnimationCtrl(wxWindow *parent);
	virtual ~MyAnimationCtrl();

	virtual int GetIndFrameToPaint();
	virtual void PaintBackground();
	virtual void OnPaint( wxPaintEvent& WXUNUSED(event));
	virtual void EvtGetFocus(wxFocusEvent&);
	virtual void EvtTimer(wxTimerEvent& event);
	virtual void UpdateNavigation();

	virtual void StopPlaying();
	virtual void StartPlaying();
			void PlayFrameSound();
	virtual void UpdateFrames();
	virtual void Clear();

	virtual void ResetMouseDraw();
	virtual void OnSimpleClick(wxMouseEvent& event);
	virtual void OnEndLeftDragging();
	virtual void OnAnyButtonDown();
	virtual void OnLeftButtonDown();

	bool b_playing;
	bool b_looping;
	bool b_sound_on;
	int curr_played;
	int first_to_play;
	int last_to_play;

protected:
	void GetFramesToPlay();

	// Animation data
	wxTimer anim_timer;

	bool inStopPlaying;
	int sv_ind_active, sv_ind_first, sv_ind_last;

	// Events
	void EvtMouseSomething(wxMouseEvent& event);
	void PlayUpdate();

	DECLARE_EVENT_TABLE()
};

enum
{
	DRAW_NOTHING,
	DRAW_AIM,
	DRAW_RECT
};

//****************************************************
//       The remap viewer Control
class MyRemapViewerCtrl : public MyAnimationZone
{
public:
	MyRemapViewerCtrl(wxWindow *parent);
	~MyRemapViewerCtrl();
	virtual void PreProcessImage( wxImage& fr_img );

protected:
	int mouse_icon_state;
	wxCursor cursor_pipette_plus, cursor_pipette_simple;
	virtual void OnSimpleClick(wxMouseEvent& event);
	void EvtMouseMove(wxMouseEvent& event);
	void EvtMouseEnter(wxMouseEvent& event);
	void EvtMouseLeave(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);

	DECLARE_EVENT_TABLE()
};


//****************************************************
//       The remap viewer Control
// Enum to identified points of the platform
enum {PLAT_NONE, PLAT_LD, PLAT_D, PLAT_RD, PLAT_LU,PLAT_U, PLAT_RU, PLAT_DEPTH};

class MyPlatformViewerCtrl : public MyAnimationZone
{
public:
	MyPlatformViewerCtrl(wxWindow *parent);
	~MyPlatformViewerCtrl();

protected:
	virtual void OnLeftButtonDown();
	virtual void OnSimpleClick(wxMouseEvent& event);
	virtual void OnEndLeftDragging();
	virtual void OnDragging();

	void OnPaint(wxPaintEvent& event);

	wxCursor cursor_move;
	int elt_dragged;

	DECLARE_EVENT_TABLE()
};



#endif /* MYANIMATIONZONE_H_ */
