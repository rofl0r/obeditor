/*
 * wListFrames_Base.cpp
 *
 *  Created on: 6 nov. 2008
 *      Author: gringo
 */

//****************************************************
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/stattext.h>
#include <wx/dcbuffer.h>

#include "common__wxFrameImage.h"
#include "ob_editor.h"
#include "entity__globals.h"
#include "entity__enums.h"
#include "entity__wListFrames.h"

using namespace std;

//****************************************************
#define MAXFRAMES_W 150
#define MAXFRAMES_H 200
#define SCROLLBAR_H 15
#define LF_MARGINS 12

//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
wxBitmap* 
FrameImgManager::BmpGet(const wxString& ob_imgPath )
{
	map<wxString,wxBitmap*>::iterator it( map_Path_Bmps.find(ob_imgPath ));
	if( it != map_Path_Bmps.end() )
		return it->second;
	
	wxImage img;
	wxString fp = ObPath_To_FullPath( ob_imgPath );
	if( wxFileExists(fp ) == true )
	{
		img.LoadFile( fp );
		if( img.IsOk() == false )
			img = *noImg;
	}
	else
		img = *noImg;
	
	// Now, must force the img to fit the wanted size
	Image_Rescale_To_Fit( img, MAXFRAMES_W,MAXFRAMES_H);
	
	wxBitmap* theBmp = new wxBitmap( img );
	map_Path_Bmps[ob_imgPath] = theBmp;
	
	return theBmp;
}

//------------------------------------------------------------
void	    
FrameImgManager::Invalidate(const wxString& ob_imgPath )
{
	map<wxString,wxBitmap*>::iterator it( map_Path_Bmps.find(ob_imgPath ));
	if( it != map_Path_Bmps.end() )
	{
		if( it->second != NULL )
			delete it->second;
		map_Path_Bmps.erase( it );
	}
}

//------------------------------------------------------------
void	    
FrameImgManager::Invalidate_ALL()
{
	map<wxString,wxBitmap*>::iterator it;
	while( map_Path_Bmps.empty() == false )
	{
		it = map_Path_Bmps.begin();
		if( it->second != NULL )
			delete it->second;
		map_Path_Bmps.erase( it );
	}
}
	
	

//****************************************************
//****************************************************
//****************************************************
//****************************************************

DEFINE_EVENT_TYPE(wxEVT_FRAME_SELECTED_CHANGE)
DEFINE_EVENT_TYPE(wxEVT_FRAME_LIST_CHANGE)

//****************************************************
//			LISTFRAMES
//****************************************************

FrameImgManager wListFrames::theFrameImgManager;

//----------------------------------------------------------------
wListFrames::wListFrames( wxWindow *parent )
:wxControl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL )
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	wList_anims = NULL;
	total_w = 0;

	m_flag = 0;
	SetMinSize( wxSize( -1, MAXFRAMES_H + 2 * LF_MARGINS + SCROLLBAR_H ) );
	SetSize( GetMinSize() );
	
	v_frames_datas.reserve(50);
	
	select_first = 0;
	select_last = 0;
	last_indframe_mouseDown = -1;
	mouseDragging__last_left_frame = -1;
	
	Clear();
}

//*************************************************************************

void wListFrames::AssociateToListAnims( wxWindow* _wList_anims)
{
	wList_anims = _wList_anims;
}

//*************************************************************************

wListFrames::~wListFrames()
{
	Clear(true);
}

//*************************************************************************
void 
wListFrames::Clear( bool b_really_all )
{
	if( b_really_all )
	{
		select_first = 0;
		select_last = 0;
		ind_active_frame = -1;
		mouseDragging__last_left_frame = -2;
		On_Ind_ActiveFrame_Change();
	}

//	theFrameImgManager.Invalidate_ALL();
	for( size_t i = 0; i < v_frames_datas.size(); i++ )
	{
		if( v_frames_datas[i] != NULL )
		{
			FrameData* t = v_frames_datas[i];
			v_frames_datas[i] = NULL;
			delete t;
		}
	}
	
	v_frames_datas.clear();
}


//*************************************************************************

ob_frame* wListFrames::GetActiveFrame()const
{
	// If No frame selected
	if( ind_active_frame < 0 || ind_active_frame >= (int)v_frames_datas.size() )
		return NULL;

	return v_frames_datas[ind_active_frame]->related_frame;
}

//-------------------------------------------------------------------------
void
wListFrames::On_Ind_ActiveFrame_Change()
{
	if( entityFrame->May_Register_ControlsState() == true )
	{
		entityFrame->Register_ControlsState_inHistory(wxT("On_Ind_ActiveFrame_Change"));
	}

}

//-------------------------------------------------------------------------
void
wListFrames::On_Selection_Bounds_Change()
{
	if( entityFrame->May_Register_ControlsState() == true )
	{
		entityFrame->Register_ControlsState_inHistory(wxT("On_Selection_Bounds_Change"));
	}
}



//*************************************************************************
/* Start the rebuild of curr_frames and curr_frames count, and rebuild object from them.
 * Keep the previous selection.
 *
 * Procedure :
 *  - Start the rebuilding of curr_frames by eventing the parent
 *  - Start the refreshing process by eventing the parent
 *
 *  Does not change current selection.
 */
void wListFrames::ReloadFrames(bool b_invalidate_images)
{
	if( m_flag & M_LOADING )
		return;
	m_flag |= M_LOADING;
	ent_g_flag |= GM_FRAME_RELOAD;
	Freeze();

	// Tell the panel to take mesure
	
	wxCommandEvent _evt(wxEVT_FRAME_LIST_CHANGE);
	ProcessEvent( _evt);

	// Now everything should be uptodate So,
	// Remove all previous frames datas
	// But not the selection datas
	Clear( false );

	// Make the ob_object Load the frames for the anim
	total_w = 0;
	if( curr_anim != NULL )
	{
		FrameData* fd;
		for( int i=0; i< curr_frames_count; i++)
		{
			wxString ob_path = curr_frames[i]->GetToken(0);
			
			// Init a new img data
			fd = new FrameData();
			fd->related_frame = curr_frames[i];
			fd->pos.x = total_w + LF_MARGINS;
			fd->pos.y = LF_MARGINS;

			if( b_invalidate_images )
			{
				theFrameImgManager.Invalidate(ob_path);
				curr_frames[i]->RemoveImage();
			}
			
			// Get the associated img, to get the img size
			wxBitmap* _bmp = theFrameImgManager.BmpGet(ob_path);
			if( _bmp == NULL )
			{
				wxMessageBox( wxT("BUG !!!\nwListFrames::ReloadFrames()\n_bmp == NULL") );
				delete fd;
				continue;
			}
			fd->size.x = _bmp->GetWidth();
			fd->size.y = _bmp->GetHeight();
			
			fd->pos.y += (MAXFRAMES_H - fd->size.y ) / 2;
				
			// Insert in the vector
			if( v_frames_datas.capacity() == v_frames_datas.size() )
				v_frames_datas.reserve(v_frames_datas.size() + 50 );
			v_frames_datas.push_back( fd );
			
			// Update total width
			total_w += fd->size.x + LF_MARGINS;
		}

		// set the flipstates
		wxString str_flipframe = curr_anim->GetSubObject_Token(wxT("flipframe"));
		if( str_flipframe != wxString() )
			SetFlipFrame( StrToInt( str_flipframe ) );

		// set the jumpstate
		wxString str_temp = curr_anim->GetSubObject_Token(wxT("jumpframe"));
		if( str_temp != wxString() )
			SetJumpFrame( StrToInt( str_temp ) );

		str_temp = curr_anim->GetSubObject_Token(wxT("dropframe"));
		if( str_temp != wxString() )
			SetDropFrame( StrToInt( str_temp ) );

		str_temp = curr_anim->GetSubObject_Token(wxT("landframe"));
		if( str_temp != wxString() )
			SetLandFrame( StrToInt( str_temp ) );

	}

	total_w += LF_MARGINS;

	// Refresh the selection
	if( (m_flag & M_REFRESHING) == 0 )
		RefreshSelection();

	// Tell the parent to take in account the new size of the listFrame control
	Thaw();

	// Update some scoll bar
	UpdateScrollBar();

	ent_g_flag &= ~GM_FRAME_RELOAD;
	m_flag &= ~M_LOADING;
}

//*************************************************************************
/* Refresh the current selection on the screen.
 * Intended to be use after a load frames to reactive the frames selected.
 *
 * Procedure :
 *  - Update the selection vars
 *  - Redo the selection
 *  - Update the scroll bar
 *  - Refresh the client zone
 *
 *  Does not change current selection.
 */
void wListFrames::RefreshSelection( bool b_redo )
{
	if( m_flag & M_REFRESH_SELECTION )
		return;
	m_flag |= M_REFRESH_SELECTION;
	ent_g_flag |= GM_REFRESH_SELECTION;

	Freeze();
//	ReloadFrames(false);

	// Take indices up to date
	if( v_frames_datas.size() <= 0 )
	{
		select_first = select_last = 0;
		last_indframe_mouseDown = -1;
		mouseDragging__last_left_frame = -2;
		On_Selection_Bounds_Change();
	}
	else
	{
		if( select_first >= (int) v_frames_datas.size() )
			select_first = v_frames_datas.size() - 1;
		if( select_last >= (int) v_frames_datas.size() )
			select_last = v_frames_datas.size() - 1;
		if( last_indframe_mouseDown >= (int) v_frames_datas.size() )
			last_indframe_mouseDown = (int) v_frames_datas.size() - 1;
		if( mouseDragging__last_left_frame >= (int) v_frames_datas.size() )
			mouseDragging__last_left_frame = v_frames_datas.size() -1;
		On_Selection_Bounds_Change();
	}


	// Redo the selection
	if( b_redo && v_frames_datas.size() > 0)
	{
		m_flag |= M_REFRESHING;
		int _t_ind_active_frame = ind_active_frame;
		int _t_select_last = select_last;
		SetSelected( select_first, 0 );
		for( int i = select_first + 1; i <= _t_select_last; i++ )
			SetSelected( i, 2 );
		ind_active_frame = _t_ind_active_frame;
		On_Ind_ActiveFrame_Change();
		m_flag &= ~M_REFRESHING;
	}


	//move the scroll bar to show the selected frame
	if( 	   ind_active_frame >= 0 
		&& ind_active_frame < (int) v_frames_datas.size() )
	{
		int sc_thumb = GetScrollThumb( wxHORIZONTAL );
		int sc_min = GetScrollPos(wxHORIZONTAL);
		int act_min, act_max;
		
		if(v_frames_datas[ind_active_frame] != NULL )
		{
			act_min = v_frames_datas[ind_active_frame]->pos.x;
			act_max = act_min + v_frames_datas[ind_active_frame]->size.y;
		}
		else
		{
			act_min = sc_min;
			act_max = sc_min + sc_thumb;
		}

		if( act_min < sc_min )
			SetScrollPos( wxHORIZONTAL, act_min );
		else if( act_max > sc_min + sc_thumb )
			SetScrollPos( wxHORIZONTAL, act_max - sc_thumb );
	}

	Thaw();
	Refresh();
	ent_g_flag &= ~GM_REFRESH_SELECTION;
	m_flag &= ~M_REFRESH_SELECTION;
}


//*************************************************************************
/* Get the selection for a futur  restpre selection
 */

void wListFrames::GetSelection( int& sv_ind_active, int& sv_ind_first, int& sv_ind_last )
{
	sv_ind_active = ind_active_frame,
	sv_ind_first = select_first;
	sv_ind_last = select_last;
}


//*************************************************************************
/* Restore a previous selection
 */

void wListFrames::RestoreSelection( int sv_ind_active, int sv_ind_first, int sv_ind_last )
{
	m_flag |= M_REFRESHING;
	select_first = sv_ind_first;
	select_last = sv_ind_last;
	ind_active_frame = sv_ind_active;
	
	On_Ind_ActiveFrame_Change();
	On_Selection_Bounds_Change();
	RefreshSelection();
	if( GetParent() == panel_Anims )
		panel_Anims->Update_FrameDatas();
	m_flag &= ~M_REFRESHING;
}


//*************************************************************************
/* Set the jumpframes properties of all the frames
 */
void wListFrames::SetJumpFrame( int _jumpframe )
{
	if( 	   curr_anim == NULL 
		|| v_frames_datas.size() == 0
		|| (ent_g_flag & GM_FRAME_RELOAD)
		)
		return;

	// Set a jump frame
	if( _jumpframe < (int) v_frames_datas.size() && _jumpframe >= 0)
	{
		for( int i = 0; i<_jumpframe; i++ )
		{
			if( v_frames_datas[i] != NULL )
				v_frames_datas[i]->related_frame->jumpState = 0;
		}
		
		if( v_frames_datas[_jumpframe] != NULL )
			v_frames_datas[_jumpframe]->related_frame->jumpState = 2;

		for( int i = _jumpframe + 1; i < (int) v_frames_datas.size(); i++ )
			if( v_frames_datas[i] != NULL )
				v_frames_datas[i]->related_frame->jumpState = 1;

		ob_object *_t = curr_anim->GetSubObject( wxT("jumpframe") );
		if( _t == NULL )
		{
			curr_anim->SetProperty( wxT("jumpframe"), IntToStr(_jumpframe) );
			_t = curr_anim->GetSubObject( wxT("jumpframe") );
			if( _t != NULL )
			{
				_t->SetToken( 1, wxT("2") );
				_t->SetToken( 2, wxT("0") );
				_t->SetToken( 3, wxT("0") );
			}
		}
		else
			_t->SetToken( 0, IntToStr(_jumpframe) );

	}
	// Set No jumpframe
	else
	{
		for( size_t i = 0; i < v_frames_datas.size(); i++ )
		{
			if( v_frames_datas[i] != NULL )
				v_frames_datas[i]->related_frame->jumpState = 0;
		}

		ob_object* _t = curr_anim->GetSubObject( wxT("jumpframe"));
		if( _t != NULL )
		{
			_t->Rm();
// 			delete _t;
		}
	}
}


//*************************************************************************
//*************************************************************************
#define ANN__SET_FRAMEPROP( IND_FRAME, STATE_ELT, PROP_NAME ) \
	if( 	   curr_anim == NULL 				\
		|| v_frames_datas.size() == 0			\
		|| (ent_g_flag & GM_FRAME_RELOAD)		\
		)							\
		return;	\
				\
	if( IND_FRAME < (int) v_frames_datas.size() && IND_FRAME >= 0)	\
	{	\
		for( int i = 0; i < IND_FRAME; i++ )		\
			if( v_frames_datas[i] != NULL )		\
				v_frames_datas[i]->related_frame->STATE_ELT = 0;\
		\
		if( v_frames_datas[IND_FRAME] != NULL )		\
			v_frames_datas[IND_FRAME]->related_frame->STATE_ELT = 2;	\
		\
		for( int i = IND_FRAME + 1; i < (int) v_frames_datas.size(); i++ )	\
			if( v_frames_datas[i] != NULL )	\
				v_frames_datas[i]->related_frame->STATE_ELT = 1;	\
		\
		curr_anim->SetProperty( PROP_NAME, IntToStr(IND_FRAME) );	\
	}	\
	else				\
	{				\
		for( size_t i = 0; i < v_frames_datas.size(); i++ )			\
			if( v_frames_datas[i] != NULL )	\
				v_frames_datas[i]->related_frame->STATE_ELT = 0;	\
		ob_object* _t = curr_anim->GetSubObject( PROP_NAME);		\
		if( _t != NULL )		\
		{				\
			_t->Rm();		\
		}				\
	}


//*************************************************************************
/* Set the flipframes properties of the curr_frames.
 */
void wListFrames::SetFlipFrame( int _flipframe )
{
	ANN__SET_FRAMEPROP( _flipframe, flipState, wxT("flipframe") );
}


//*************************************************************************
/* Set the dropframes properties of all the frames
 */
void wListFrames::SetDropFrame( int _dropframe )
{
	ANN__SET_FRAMEPROP( _dropframe, dropState, wxT("dropframe") );
}

//*************************************************************************
/* Set the dropframes properties of all the frames
 */
void wListFrames::SetLandFrame( int _landframe )
{
	ANN__SET_FRAMEPROP( _landframe, landState, wxT("landframe") );
}

//*************************************************************************

void wListFrames::EvtSize( wxSizeEvent& event )
{
//	wList_frames->EvtSize( event );
	event.Skip();
}

//*************************************************************************

wxSize wListFrames::DoGetBestSize()const
{
	int h = MAXFRAMES_H + 2 * LF_MARGINS + SCROLLBAR_H;
	int w = wxDefaultCoord;
	return wxSize(w,h);
}

//*************************************************************************

int wListFrames::GetFrameIndFromPos( int m_x, int m_y )const
{
	m_x += GetScrollPos(wxHORIZONTAL);
	
	int last_dist = 2000;
	int ind_frame = -1;

	// Get the nearest anim
	for( size_t i = 0; i < v_frames_datas.size();i++ )
	{
		FrameData* fd = v_frames_datas[i];
		if( fd == NULL )
			continue;

		// If the click is inside the Frame Image
		int _x = fd->pos.x;
		int _w = fd->size.x;
		if( m_x > _x && m_x < _x + _w )
		{
			ind_frame = i;
			break;
		}

		// Not inside the image -> Calc the dist
		_x += _w/2;
		if( m_x > _x )
		{
			ind_frame = i;
			last_dist = m_x - _x;
		}
		else
		{
			if( _x - m_x < last_dist)
				ind_frame = i;
			break;
		}
	}
	return ind_frame;
}

//*************************************************************************

void wListFrames::GetCoupleFramesAssidePos( int m_x, int m_y, int &ind_left, int &ind_right )const
{
	m_x += GetScrollPos( wxHORIZONTAL );
	
	// Get the nearest anims
	ind_left = ind_right = -1;
	for( size_t i = 0; i < v_frames_datas.size();i++ )
	{
		FrameData* fd = v_frames_datas[i];
		if( fd == NULL )
			continue;
		int _x = fd->pos.x + fd->size.x / 2;
		if( m_x < _x )
		{
			ind_left = i -1;
			ind_right = i;
			break;
		}
	}

	// tail case
	if( ind_left == ind_right && curr_frames_count > 0)
		ind_left = curr_frames_count -1;
}


//*************************************************************************

void wListFrames::GetSelectedFrames( int& first_to_play, int& last_to_play )
{
	if( 	   v_frames_datas.empty() == true
		|| select_first > select_last 
		|| select_last >= curr_frames_count
	  )
	{
		first_to_play = -1;
		last_to_play = -1;
		return;
	}

	first_to_play = select_first;
	last_to_play = select_last;

	// If only one frame selected => return all frames
	if( first_to_play == last_to_play )
	{
		first_to_play = 0;
		last_to_play = curr_frames_count - 1;
		return;
	}

	return;
}


//----------------------------------------------------------------------
void 
wListFrames::EvtMouseSomething(wxMouseEvent& event)
{
	if( event.LeftUp())
	{
		if( m_flag & M_REFRESH_SELECTION )
			return;
		m_flag |= M_REFRESH_SELECTION;

//		int frame_to_select = 0;
		int ind_frame = GetFrameIndFromPos( event.m_x, event.m_y );
		int ind_left, ind_right;
		GetCoupleFramesAssidePos( event.m_x, event.m_y, ind_left, ind_right );
		int addModd = 1;
		
		if( event.ShiftDown() || event.ControlDown() )
			addModd = 2;

		else if( event.ButtonDClick() )
			addModd = 0;

		SetSelected( ind_frame, addModd );
		Refresh();
		wList_anims->SetFocus();

		m_flag &= ~M_REFRESH_SELECTION;
	}

	else if( event.GetWheelRotation() != 0 )
	{
		SetScrollPos( wxHORIZONTAL,
				GetScrollPos( wxHORIZONTAL) - event.GetWheelRotation());
		Refresh();
	}

	if( event.Moving() == true )
		m_flag  &= ~M_HAVE_DRAGGED;
	else if( event.Dragging() == true )
		m_flag  |=  M_HAVE_DRAGGED;

	event.Skip();
}


//*************************************************************************

void wListFrames::EvtCharPress(wxKeyEvent& event)
{
	int kc = event.GetKeyCode();
	int add_mode = (event.ShiftDown() || event.ControlDown()) ? 4 : 1;
	if( kc == WXK_LEFT )
	{
		if( ind_active_frame <= 0)
			return;
		SetSelected( ind_active_frame - 1, add_mode );
		return;
	}
	if( kc == WXK_RIGHT )
	{
		if( ind_active_frame >= curr_frames_count -1)
			return;
		SetSelected( ind_active_frame + 1, add_mode );
		return;
	}
	if( kc == WXK_PAGEUP )
	{
		if( ind_active_frame <= 0)
			return;
		int _selected = ind_active_frame - 4;
		if( _selected < 0 ) _selected = 0;
		SetSelected( _selected, add_mode );
		return;
	}
	if( kc == WXK_PAGEDOWN )
	{
		if( ind_active_frame >= curr_frames_count -1)
			return;
		int _selected = ind_active_frame + 4;
		if( _selected >= curr_frames_count -1 ) _selected = curr_frames_count -1;
		SetSelected( _selected, add_mode );
		return;
	}
	if( kc == WXK_HOME )
	{
		if( ind_active_frame <= 0)
			return;
		SetSelected( 0, add_mode);
		return;
	}
	if( kc == WXK_END )
	{
		if( ind_active_frame >= curr_frames_count -1)
			return;
		SetSelected( curr_frames_count -1, add_mode);
		return;
	}

	if( kc == WXK_DELETE )
	{
		wxCommandEvent new_event(wxEVT_COMMAND_MENU_SELECTED, CMD_DEL );
		AddPendingEvent(new_event);
		return;
	}

	// Resend up and down events to the list of anims
	if( kc == WXK_UP || kc == WXK_DOWN )
	{
		wList_anims->ProcessEvent( event );
		return;
	}
	event.Skip();
}


//*************************************************************************

void wListFrames::EvtGetFocus(wxFocusEvent& event )
{
//	event.SetEventType( wxEVT_KILL_FOCUS );
//	GetParent()->ProcessEvent event );
	wList_anims->SetFocus();
//	event.Skip();
}

//*************************************************************************
/* Select a frame in the list.
 * ARGUMENTs:
 *    ind_frame : the indice of the frame to select
 *    b_AddToSelection : true if the above indice have to be ADDED to the current selection
 *
 * Procedure :
 *  - Update selection datas
 *  - Start the refreshing process by eventing the parent
 */
void 
wListFrames::SetSelected( int ind_frame, int mod_AddToSelection, bool b_propagate )
{
	wxCommandEvent _evt(wxEVT_FRAME_SELECTED_CHANGE);

	// No frame to draw or deselect
	if( v_frames_datas.size() <= 0 || ind_frame == -200)
	{
		select_first = 0;
		select_last = 0;
		ind_active_frame = -1;
		last_indframe_mouseDown = -1;
		mouseDragging__last_left_frame = -2;
		frameActive = NULL;

		On_Selection_Bounds_Change();
		On_Ind_ActiveFrame_Change();
		RefreshSelection( false );
		ProcessEvent( _evt);

		return;
	}

	// Some checking of the param
	if( ind_frame < 0 )
		ind_frame = 0;

	else if( ind_frame >= (int) v_frames_datas.size() )
	{
		ind_frame = v_frames_datas.size() - 1;
		if( select_last >= (int) v_frames_datas.size() )
		{
			select_last = v_frames_datas.size() - 1;
			On_Selection_Bounds_Change();
		}
	}


	// Normal selection
	if(     		mod_AddToSelection == 0
               || 
			(   	    mod_AddToSelection == 1 
				&&  (ind_frame < select_first || ind_frame > select_last)
			)
             )
	{
		select_first = ind_frame;
		select_last = ind_frame;
		On_Selection_Bounds_Change();
	}
	
	// Do not change selection bounds when navigating inside
	else if( mod_AddToSelection == 1 
		&& ind_frame >= select_first 
		&& ind_frame <= select_last
		)
	{
		ind_active_frame = ind_frame;
		On_Ind_ActiveFrame_Change();
	}

	// Selection in Adding mode
	else
	{
		if( ind_frame > select_last )
			select_last = ind_frame;
		else if( ind_frame < select_first )
			select_first = ind_frame;
		
		// Keyboard Add
		else if( mod_AddToSelection == 4 ) 
		{
			if( ind_active_frame < ind_frame)
				select_first = ind_frame;
			else
				select_last = ind_frame;
		}
		// Mouse Add
		else if( ind_active_frame != ind_frame )
		{
			select_first = min(ind_frame,ind_active_frame);
			select_last  = max(ind_frame,ind_active_frame);
		}
		On_Selection_Bounds_Change();
	}

	int prev_active_frame = ind_active_frame;
	ind_active_frame = ind_frame;
	On_Ind_ActiveFrame_Change();
	if( v_frames_datas[ind_active_frame] != NULL )
		frameActive = v_frames_datas[ind_active_frame]->related_frame;
	else
		ind_active_frame = prev_active_frame;

	if( (m_flag & M_REFRESHING) == 0 )
	{
		RefreshSelection(false);

		// Tell the parent to take mesure on the new selection
		if( b_propagate )
			ProcessEvent(_evt );
	}
}



//*************************************************************************
void 
wListFrames::EvtPaint(wxPaintEvent& event )
{
	wxAutoBufferedPaintDC dc(this);

	if( curr_frames_count == 0 || m_flag & M_LOADING )
		return;

	// No frame selected
	if( ind_active_frame < 0 || ind_active_frame > (int)v_frames_datas.size() )
		return;

	//---------------------------------------------------------------
	//Some vars
	wxPen pen;
	wxBrush brush;
	int w_w, w_h;
	GetSize( &w_w, &w_h );
	int scroll_pos = GetScrollPos( wxHORIZONTAL );
	int rect_x,rect_y,rect_w,rect_h;
	
	//---------------------------------------------------------------
	//redraw the background
	dc.SetBackground(GetBackgroundColour());
	dc.Clear();

	//---------------------------------------------------------------
	// Paint the selected frames
	if( select_first != select_last )
	{
		pen.SetColour(*wxBLUE);
		dc.SetPen(pen);
		brush.SetColour(*wxBLUE);
		dc.SetBrush(brush);
		
		if( v_frames_datas[select_first] != NULL )
		{
			rect_x =   v_frames_datas[select_first]->pos.x 
						- scroll_pos 
						- LF_MARGINS;
			rect_y = 0;

			rect_w =   	  v_frames_datas[select_last] ->pos.x 
					- v_frames_datas[select_first]->pos.x
					+ v_frames_datas[select_last] ->size.x 
					+ LF_MARGINS*2;
			rect_h = MAXFRAMES_H + 2 * LF_MARGINS;

			dc.DrawRectangle(	rect_x, rect_y, rect_w, rect_h );
		}
	}

	//---------------------------------------------------------------
	// Paint the sourround for the Active frame
	wxColour _colour = wxColour( 255, 240, 0 );
	pen.SetColour(_colour);
	dc.SetPen(pen);
	brush.SetColour(_colour);
	dc.SetBrush(brush);
	
	rect_x =   v_frames_datas[ind_active_frame]->pos.x 
				- scroll_pos 
				- LF_MARGINS / 2;
	rect_y = LF_MARGINS / 2;

	rect_w =   v_frames_datas[ind_active_frame] ->size.x 
		   + LF_MARGINS;
	rect_h = MAXFRAMES_H + LF_MARGINS;;

	dc.DrawRectangle(	rect_x, rect_y, rect_w, rect_h );
	
	//---------------------------------------------------------------
	// have to paint the dragging cursor
	if( mouseDragging__last_left_frame != -2 )
	{
		int cursor_y = LF_MARGINS;
		int cursor_x;

		// The cursor is before the first frame
		if(  mouseDragging__last_left_frame == -1 )
			cursor_x = LF_MARGINS/4;
		else
		{
			cursor_x =   
				  v_frames_datas[mouseDragging__last_left_frame]->pos.x 
				- scroll_pos 
				+ v_frames_datas[mouseDragging__last_left_frame]->size.x 
				+ LF_MARGINS*3/4;
		}
		
		dc.SetPen(*wxBLACK);
		dc.SetBrush(*wxBLACK);
		dc.DrawRectangle( cursor_x , cursor_y
				    , LF_MARGINS/2, MAXFRAMES_H + LF_MARGINS*2 );
	}

	int t_total_w = 0;
	//---------------------------------------------------------------
	// Paint the imgs
	for( size_t i = 0; i < v_frames_datas.size();i++)
	{
		if( v_frames_datas[i] == NULL )
			continue;
		
		wxBitmap* theBmp = theFrameImgManager.BmpGet( v_frames_datas[i]->related_frame->GetToken(0));
		dc.DrawBitmap( 
			 *theBmp
			, v_frames_datas[i]->pos.x - scroll_pos
			, v_frames_datas[i]->pos.y
				);
		
		// Recompute pos and size to check if something have change
		if( i > 0 )
			v_frames_datas[i]->pos.x = 
					  v_frames_datas[i-1]->pos.x
					+ v_frames_datas[i-1]->size.x + LF_MARGINS;
		v_frames_datas[i]->size.x = theBmp->GetWidth();
		t_total_w += v_frames_datas[i]->size.x + LF_MARGINS;
	}
	t_total_w += LF_MARGINS;
	
	// If something has change...
	if( t_total_w != total_w )
	{
		total_w = t_total_w;
		UpdateScrollBar();
		Refresh();
	}
}

//*************************************************************************
void 
wListFrames::UpdateScrollBar()
{
	int sc_pos = GetScrollPos(wxHORIZONTAL);

	int _w,_h;
	GetSize(&_w,&_h);
	if( _w > 0 && total_w > _w )
	{
		SetScrollbar(wxHORIZONTAL , sc_pos, _w, total_w );
	}
	else
		SetScrollbar(wxHORIZONTAL , 0, 0, 0 );
}

//*************************************************************************
void 
wListFrames::EvtScrollUp( wxScrollWinEvent& event )
{
	event.Skip();
	if( event.GetOrientation() == wxVERTICAL )
		return;

	SetScrollPos( wxHORIZONTAL,
			GetScrollPos( wxHORIZONTAL) - GetScrollRange(wxHORIZONTAL) / 5 );
	Refresh();
}

//*************************************************************************
void 
wListFrames::EvtScrollDown( wxScrollWinEvent& event )
{
	event.Skip();
	if( event.GetOrientation() == wxVERTICAL )
		return;
	SetScrollPos( wxHORIZONTAL,
			GetScrollPos( wxHORIZONTAL) + GetScrollRange(wxHORIZONTAL) / 5 );
	Refresh();
}

//*************************************************************************
void 
wListFrames::EvtScroll( wxScrollWinEvent& event )
{
	if( event.GetOrientation() == wxHORIZONTAL )
		SetScrollPos( wxHORIZONTAL, event.GetPosition() );
	event.Skip();
	Refresh();
}

//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
// Event table
BEGIN_EVENT_TABLE(wListFrames, wxControl)
	EVT_PAINT(wListFrames::EvtPaint)
	EVT_MOUSE_EVENTS(wListFrames::EvtMouseSomething)
	EVT_SIZE(wListFrames::EvtSize)
	EVT_SCROLLWIN_LINEUP(wListFrames::EvtScrollUp)
	EVT_SCROLLWIN_PAGEUP(wListFrames::EvtScrollUp)
	EVT_SCROLLWIN_LINEDOWN(wListFrames::EvtScrollDown)
	EVT_SCROLLWIN_PAGEDOWN(wListFrames::EvtScrollDown)
	EVT_SCROLLWIN_THUMBTRACK(wListFrames::EvtScroll)
	EVT_CHAR(wListFrames::EvtCharPress)
	EVT_CHAR_HOOK(wListFrames::EvtCharPress)
	EVT_SET_FOCUS(wListFrames::EvtGetFocus)
END_EVENT_TABLE()


//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************


wListFrames_Editable::wListFrames_Editable( wxWindow *parent )
: wListFrames( parent )
{
	popMenu = new 	wxMenu();
	popMenu->Append( CMD_NEW, wxT("New Frame") );
	popMenu->Append( CMD_DEL, wxT("Delete Frame") );
	popMenu->Append( CMD_COPY, wxT("Copy Frame") );
}


//*************************************************************************

wListFrames_Editable::~wListFrames_Editable()
{
	delete popMenu;
}

//*************************************************************************

void wListFrames_Editable::EvtMouseSomething(wxMouseEvent& event)
{
	// Some vars
	int ind_frame = GetFrameIndFromPos( event.m_x, event.m_y );
	int ind_left, ind_right;
	GetCoupleFramesAssidePos( event.m_x, event.m_y, ind_left, ind_right );

	// Double click on a frame
	if( event.ButtonDClick() )
		SetSelected( ind_frame, 0 );
		
	// Left button change to UP
	else if( event.LeftUp() )
	{
		if( ind_frame != -1 && last_indframe_mouseDown != -1 )
		{
			// Simple click detected
			if(
				( 
					(
						!( event.ShiftDown() || event.ControlDown())
						||
						(m_flag & M_HAVE_DRAGGED) == 0
					)
					&& ind_frame >= select_first 
					&& ind_frame <= select_last
				)
				||
				( 	   ind_right >= select_first
					&& ind_left  <  select_last
					&& (event.ShiftDown() || event.ControlDown())
				)
				)
			{
				int addModd = 1;
				if( event.ShiftDown() || event.ControlDown() )
					addModd = 2;
				SetSelected( ind_frame, addModd );
				mouseDragging__last_left_frame = -2;
				event.Skip();
				return;
			}

			// dragging detected
			else
			{
				// really dragging
				if(
					( event.ShiftDown() || event.ControlDown() )
					||
						(  ind_left  != last_indframe_mouseDown
						&& 	
						ind_right != last_indframe_mouseDown
						)
					)
				{
					if( ! DoDragging(ind_frame, ind_left, ind_right, event) )
					{
						// Redo selection
						RefreshSelection();
					}
				}
				// Back to Simple click event
				else
				{
					SetSelected( ind_frame, 
						(event.ShiftDown()||event.ControlDown())
							? 2 : 1 );
					mouseDragging__last_left_frame = -2;
					event.Skip();
					return;
				}
			}
		}
		mouseDragging__last_left_frame = -2;
		last_indframe_mouseDown = -1;
		SetFocus();
	}

	// Left button change to down
	else if( event.LeftDown() )
	{
		if( ind_frame < select_first || ind_frame > select_last )
		{
			SetSelected( ind_frame, 
					(event.ShiftDown()||event.ControlDown()) 
						? 2 : 1
					);
			mouseDragging__last_left_frame = -2;
		}
		last_indframe_mouseDown = ind_frame;
		SetFocus();
	}

	// Right Button change to Up
	else if( event.RightUp() )
	{
		wxPoint point = event.GetPosition();
		PopupMenu( popMenu, point );
		SetFocus();
	}
	// The wheel have turn
	else if( event.GetWheelRotation() != 0 )
	{
		wListFrames::EvtMouseSomething( event );
	}


	// Dragging event and a frame have been set
	else if( last_indframe_mouseDown >= 0 
		&& event.m_leftDown 
		&& event.Dragging()
		)
	{
		m_flag  |=  M_HAVE_DRAGGED;

		// No big cursor to paint if the drag is not far enough
		if( ( 		ind_left >= select_first - 1
				&& 	ind_left <= select_last
				&& ! event.m_controlDown)
			||
			( 		ind_left >= select_first
				&& 	ind_left < select_last
				&& event.m_controlDown )
		  )
		{
			if( mouseDragging__last_left_frame != - 2)
			{
				mouseDragging__last_left_frame = -2;
				Refresh();
			}
			event.Skip();
			return;
		}
		// if move has stepping a frame
		if( mouseDragging__last_left_frame != ind_left)
		{
			mouseDragging__last_left_frame = ind_left;
			Refresh();
		}
	}
	
	else if ( event.Moving() == true )
		m_flag  &= ~M_HAVE_DRAGGED;

	event.Skip();
}


//*************************************************************************

void wListFrames_Editable::EvtNewFrame(wxCommandEvent& event )
{
	if( curr_anim == NULL )
	{
		wxMessageBox( wxT("No anim selected"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	// Modify the Object
	ob_frame *_frame = new ob_frame();
	int ind_insert = 0;
	if( v_frames_datas.size() > 0 )
	{
		if( 	
			ind_active_frame >= 0 
			&& 
			v_frames_datas[ind_active_frame] != NULL
			)
		{
			v_frames_datas[ind_active_frame]->related_frame->InsertObject_After( _frame );
			ind_insert = ind_active_frame + 1;
		}
		else if( v_frames_datas[0] != NULL )
			v_frames_datas[0]->related_frame->InsertObject_Before( _frame );
	}
	else
		curr_anim->Add_SubObj( _frame );
	
	// Reload Frames and Redo Selection
	ent_g_flag |= GM_FRAME_INVALID;
	ReloadFrames();
	RefreshSelection();
	SetSelected( ind_insert, 0 );
	ent_g_flag &= ~GM_FRAME_INVALID;
	
	// Mark the object as changed
	entity->SetChanged();
}


//*************************************************************************

void wListFrames_Editable::EvtCopyFrame(wxCommandEvent& event )
{
	if( curr_anim == NULL )
	{
		wxMessageBox( wxT("No anim selected"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	if( v_frames_datas.size() == 0 )
	{
		wxMessageBox( wxT("No frame to copy"), wxT("Error"), wxOK | wxICON_INFORMATION,this );
		return;
	}

	if( ind_active_frame < 0 )
	{
		wxMessageBox( wxT("No frame selected"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	if( curr_frames[ind_active_frame] == NULL )
		return;
	ob_frame *_frame = (ob_frame*) curr_frames[ind_active_frame]->Clone();
	v_frames_datas[ind_active_frame]->related_frame->InsertObject_After( _frame );
	
	
	ent_g_flag |= GM_FRAME_INVALID;
	ReloadFrames();
	SetSelected( ind_active_frame + 1, 0 );
	ent_g_flag &= ~GM_FRAME_INVALID;
	
	entity->SetChanged();
}


//*************************************************************************

void wListFrames_Editable::EvtDeleteFrame(wxCommandEvent& event )
{
	if( curr_anim == NULL )
	{
		wxMessageBox( wxT("No anim selected"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	if( v_frames_datas.size() == 0 )
	{
		wxMessageBox( wxT("No frame to copy"), wxT("Error"), wxOK | wxICON_INFORMATION, this );
		return;
	}

	if( ind_active_frame < 0 )
	{
		wxMessageBox( wxT("No frame selected"), wxT("Error"), wxOK | wxICON_INFORMATION,this );
		return;
	}

	// Make the guy to be sure
	int res = wxMessageBox( wxT("Delete selected frames ?"), wxT("Hey !"), wxYES_NO | wxICON_INFORMATION, this );

	if( res == wxYES )
	{
		ent_g_flag |= FRAME_DELETION;
		for( int i = select_last; i >=select_first ; i-- )
		{
			if( i >= 0 && i < curr_frames_count )
			{
				curr_frames[i]->Detach();
				curr_frames[i] = NULL;
			}
		}
		ReloadFrames();
		RefreshSelection();
		((Panel_Anims*)GetParent())->Refresh_FrameData_AnimControl();
		ent_g_flag &= ~FRAME_DELETION;
		entity->SetChanged();
	}
}

//*************************************************************************

void wListFrames_Editable::EvtCharPress(wxKeyEvent& event)
{
	int kc = event.GetKeyCode();

	if( kc == WXK_DELETE )
	{
		wxCommandEvent new_event(wxEVT_COMMAND_MENU_SELECTED, CMD_DEL );
		AddPendingEvent(new_event);
		return;
	}
	wListFrames::EvtCharPress(event);
}


//*************************************************************************
/* Do the mouse draggings.
 * Implement the dragging-copy/moves feature
 * Procedure :
 *  - Move/Insert new frame
 *  - Start the refreshing process by eventing the parent
 *  - Reselect everything
 *  - Refresh the view
 */
bool 
wListFrames_Editable::DoDragging(int ind_frame, int ind_left, int ind_right, wxMouseEvent& event )
{
	if( v_frames_datas.size() <= 0 )
		return false;

	m_flag |= M_REFRESHING;

	//***************************************
	// If only one frame selected
	//***************************************
	if( select_first == select_last )
	{
		// Get the source frame
		ob_frame* _frame = curr_frames[last_indframe_mouseDown];
		if( last_indframe_mouseDown > ind_left )
			ind_active_frame = ind_left + 1;
		else
			ind_active_frame = ind_left;

		// If Make a copy
		if( event.m_controlDown )
		{
			theHistoryManager.GroupStart( wxT("Copy a frame") );
			_frame=(ob_frame*) curr_frames[last_indframe_mouseDown]->Clone();
			if( last_indframe_mouseDown < ind_left )
				ind_active_frame++;
		}
		else
			theHistoryManager.GroupStart( wxT("Move a frame") );

		// if insert after possible
		if( ind_left >= 0 )
		{
			curr_frames[ind_left]->InsertObject_After( _frame );
		}

		// Dragg at the head of the frames
		else
		{
			curr_frames[ind_right]->InsertObject_Before( _frame );
			ind_active_frame = 0;
		}
		theHistoryManager.GroupEnd();
		
		select_first = ind_active_frame;
		select_last = ind_active_frame;
		On_Ind_ActiveFrame_Change();
		On_Selection_Bounds_Change();
	}

	//***************************************
	// Multiple frames to copy/move
	//***************************************
	else
	{
		// Check if draggable possible
		if(
			( !event.m_controlDown && ind_left >= select_first && ind_left <= select_last)
			||
			( event.m_controlDown  && ind_left > select_first && ind_left < select_last) )
			return false;

		// Make the guy to be sure
		wxString str_action = wxT("move");
		if( event.m_controlDown )
			str_action = wxT("copy");
		int res = wxMessageBox( wxT("Are really sure that you want to ")+str_action+wxT(" the selected anims ?"),
					wxT("Hey !"), wxYES_NO | wxICON_INFORMATION, this );

		if( res != wxYES )
			return false;

		//***************************************
		// If Make a copy
		//***************************************
		if( event.m_controlDown )
		{
			theHistoryManager.GroupStart( wxT("Copy some frames") );
			// Heading insert
			if( ind_left == -1 )
			{
				for( int i = select_first; i <= select_last; i++ )
				{
					ob_frame* _frame = 
						(ob_frame*) curr_frames[i]->Clone();
					curr_frames[0]->InsertObject_Before( _frame );
				}
				ind_active_frame = ind_active_frame - select_first;
				select_last  = select_last - select_first;
				select_first = 0;
				On_Ind_ActiveFrame_Change();
				On_Selection_Bounds_Change();
			}

			//normal insert
			else
			{
				for( int i = select_last; i >= select_first; i-- )
				{
					ob_frame* _frame = 
						(ob_frame*) curr_frames[i]->Clone();
					curr_frames[ind_left]->InsertObject_After( _frame );
				}

				// On selectionne la plage copié
				int t_d = ind_active_frame - select_first;
				int _nb_eltl = (select_last - select_first +1);

				select_first = ind_left + 1;
				select_last  = select_first + _nb_eltl-1;
				ind_active_frame = select_first + t_d;
				
				On_Ind_ActiveFrame_Change();
				On_Selection_Bounds_Change();
			}
			theHistoryManager.GroupEnd();
		}

		//***************************************
		// If Make a move
		//***************************************
		else
		{
			theHistoryManager.GroupStart( wxT("Move some frames") );
			// make the move
			if( ind_left == curr_frames_count -1 )
			{
				for( int i = select_last; i >= select_first; i-- )
				{
					ob_frame* _frame = curr_frames[i];
					curr_frames[curr_frames_count - 1]->InsertObject_After( _frame );
				}
			}
			else
			{
				for( int i = select_first; i <= select_last; i++ )
				{
					ob_frame* _frame = curr_frames[i];
					curr_frames[ind_right]->InsertObject_Before( _frame );
				}
			}
			theHistoryManager.GroupEnd();

			// On selectionne la plage copié

			// Heading move
			if( ind_left == -1 )
			{
				ind_active_frame = ind_active_frame - select_first;
				select_last  = select_last - select_first;
				select_first = 0;
			}

			//normal move
			else
			{
				int t_d = ind_active_frame - select_first;
				int _nb_eltl = (select_last - select_first +1);

				if( select_first < ind_left )
				{
					select_first = ind_left + 1 - _nb_eltl;
					select_last  = select_first + _nb_eltl-1;
					ind_active_frame = select_first + t_d;
				}
				else
				{
					select_first = ind_left + 1;
					select_last  = select_first + _nb_eltl-1;
					ind_active_frame = select_first + t_d;
				}
			}
			On_Selection_Bounds_Change();
			On_Ind_ActiveFrame_Change();
		}
	}

	//****************************
	// Reload the list of frames
	//****************************
	ReloadFrames();

	// Redo selection
	RefreshSelection();
	UpdateScrollBar();

	// mark the entity changed
	entity->SetChanged();

	m_flag &= ~M_REFRESHING;

	// Tell the parent to refresh the related views
	wxCommandEvent _evt(wxEVT_FRAME_SELECTED_CHANGE);
	ProcessEvent(_evt );
	return true;
}


//*************************************************************************
//*************************************************************************
//*************************************************************************
//*************************************************************************
// Event table
BEGIN_EVENT_TABLE(wListFrames_Editable, wListFrames)
	EVT_SET_FOCUS(wListFrames::EvtGetFocus)
	EVT_MOUSE_EVENTS(wListFrames_Editable::EvtMouseSomething)
	EVT_CHAR(wListFrames_Editable::EvtCharPress)
	EVT_CHAR_HOOK(wListFrames_Editable::EvtCharPress)
	EVT_MENU( CMD_NEW, wListFrames_Editable::EvtNewFrame)
	EVT_MENU( CMD_COPY, wListFrames_Editable::EvtCopyFrame)
	EVT_MENU( CMD_DEL, wListFrames_Editable::EvtDeleteFrame)
END_EVENT_TABLE()

