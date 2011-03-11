/*
 * wListFrames_Base.h
 *
 *  Created on: 6 nov. 2008
 *      Author: pat
 */

#ifndef WLISTFRAMES_H_
#define WLISTFRAMES_H_


//****************************************************

#include <list>
#include <map>
#include <vector>

#include "common__tools.h"


//****************************************************
//****************************************************
class FrameImgManager
{
public:
	wxBitmap* BmpGet(const wxString& ob_imgPath );
	void	    Invalidate(const wxString& ob_imgPath );
	void	    Invalidate_ALL();

protected:
	std::map<wxString,wxBitmap*> map_Path_Bmps;
};


//****************************************************

class wListFrames : public wxControl
{
public:
	static FrameImgManager theFrameImgManager;
	
public:
	wListFrames( wxWindow *parent );
	void AssociateToListAnims( wxWindow* _wList_anims);
	virtual ~wListFrames();
	
	//------------------------------------------------------
	void ReloadFrames(bool b_invalidate_images = false);
	
	void On_Selection_Bounds_Change();
	void On_Ind_ActiveFrame_Change();
	
	//------------------------------------------------------
	void EvtPaint(wxPaintEvent& event );
	void EvtSize( wxSizeEvent& event );

	//------------------------------------------------------
	void EvtScrollDown( wxScrollWinEvent& event );
	void EvtScrollUp( wxScrollWinEvent& event );
	void EvtScroll( wxScrollWinEvent& event );
	void UpdateScrollBar();

	//------------------------------------------------------
	virtual void EvtMouseSomething(wxMouseEvent& event);
	virtual void EvtCharPress(wxKeyEvent& event);
	void EvtGetFocus(wxFocusEvent& event );

	//------------------------------------------------------
	void RefreshSelection(bool b_redo = true );
	void RestoreSelection( int sv_ind_active, int sv_ind_first, int sv_ind_last );
	void GetSelection( int& sv_ind_active, int& sv_ind_first, int& sv_ind_last );
	// -200 to Deselect
	void SetSelected( int ind_frame = -200
			    , int mod_AddToSelection = 0
			    , bool b_propagate = true ); 
	void GetSelectedFrames( int& first_to_play, int& last_to_play );
	ob_frame* GetActiveFrame()const;

	//------------------------------------------------------
	void SetFlipFrame( int _flip_frame );
	void SetLandFrame( int _landframe );
	void SetDropFrame( int _dropframe );
	void SetJumpFrame( int _jumpframe );

	int GetSelectedFirst(){return select_first;};
	int GetSelectedLast(){return select_last;};
protected:
	//---------------------------------------------------------
	// The associated list of anims
	wxWindow* wList_anims;
	int total_w;

	//---------------------------------------------------------
	class FrameData
	{
		public:
		FrameData():pos(0,0),size(0,0),related_frame(NULL){};
		wxSize pos;
		wxSize size;
		ob_frame* related_frame;
	};
	
	std::vector<FrameData*> v_frames_datas;

	//---------------------------------------------------------
	// ind of the first and last frames selected
	int select_first, select_last;
	// the frame that get the mouseDown event
	int last_indframe_mouseDown;			
	// the frame that have the dragging cursor on his right
	int mouseDragging__last_left_frame;		

	int  GetFrameIndFromPos( int _x, int _y )const;
	void GetCoupleFramesAssidePos( int m_x, int m_y, int &ind_left, int &ind_right )const ;

	//---------------------------------------------------------
	enum
	{
		M_LOADING = 1,
		M_REFRESHING = 2,
		M_REFRESH_SELECTION = 4,
		M_HAVE_DRAGGED	= 8,
	};
	int m_flag;
	
	//---------------------------------------------------------
	void Clear(bool b_really_all = true);
	
public:
	//---------------------------------------------------------
	wxSize DoGetBestSize() const;
	
	DECLARE_EVENT_TABLE()
};


//****************************************************

class wListFrames_Editable : public wListFrames
{
public:
	wListFrames_Editable( wxWindow *parent );
	virtual ~wListFrames_Editable();

protected:
	virtual void EvtMouseSomething(wxMouseEvent& event);
	virtual void EvtCharPress(wxKeyEvent& event);
	void EvtGetFocus(wxFocusEvent& event );
	void EvtNewFrame(wxCommandEvent& event );
	void EvtDeleteFrame(wxCommandEvent& event );
	void EvtCopyFrame(wxCommandEvent& event );
	void EvtContextMenu( wxContextMenuEvent& event );

	bool DoDragging(int ind_frame, int ind_left, int ind_right, wxMouseEvent& event );
	wxMenu *popMenu;

	DECLARE_EVENT_TABLE()
};

#endif /* WLISTFRAMES_BASE_H_ */
