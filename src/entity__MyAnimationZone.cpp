/*
 * MyAnimationZone.cpp
 *
 *  Created on: 7 nov. 2008
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/sound.h>
#include <wx/dcbuffer.h>
#include <wx/tooltip.h>

#include "images__MyPalette.h"
#include "entity__enums.h"
#include "entity__globals.h"
#include "entity__MyAnimationZone.h"

extern void PlaySound(char *file);


#define PI 3.14159265
#define JUMP_HALF_TIME 20
#define JUMP_ANIM_STEP 4

using namespace std;

//***************************************
//***************************************
//***************************************
//***************************************

wxColour ob_pink(255,0,255);
wxColour MyAnimationZone::orig_bg(0,0,0);

struct jump_step
{
	int mv_h;
	int delay;
};

struct jump_stair
{
	float mv_x;
	float mv_z;
	int curr_step;
	int nb_step;
	jump_step *steps;
};


void clear_jump_stairs( jump_stair*& fr_jump_stairs, int& nb_stair )
{
	if( fr_jump_stairs != NULL )
		for( int i =0; i < nb_stair; i++ )
		{
			if( fr_jump_stairs[i].nb_step > 0 )
				delete[] fr_jump_stairs[i].steps;
		}

	if( fr_jump_stairs != NULL )
		delete[] fr_jump_stairs;
	fr_jump_stairs = NULL;
	nb_stair = 0;
}


//***************************************
//***************************************
//***************************************

MyAnimationZone::MyAnimationZone(wxWindow *parent)
: wxControl(parent, wxID_ANY
		, wxDefaultPosition, wxDefaultSize
		, wxVSCROLL| wxHSCROLL
		)
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	
	theBitmap = NULL;
	m_flag = 0;

	SetToolTip( wxString() );
	theTip = GetToolTip();
	theTip->SetDelay( 200 );
	theTip->Enable( false );
	
	fr_decal_Xs = NULL;
	fr_decal_Ys = NULL;
	fr_As = NULL;
	fr_Zdecal = NULL;
	fr_total_decal_Xs = NULL;
	fr_total_decal_Ys = NULL;
	fr_offset_Xs = NULL;
	fr_offset_Ys = NULL;
	fr_DM_decal_Xs = NULL;
	fr_DM_decal_Ys = NULL;
	fr_rotate_offset_Xs = NULL;
	fr_rotate_offset_Ys = NULL;
	original_img_w = NULL;
	original_img_h = NULL;
	current_img_w  = NULL;
	current_img_h  = NULL;
	fr_rotate_imgw = NULL;
	fr_rotate_imgh = NULL;
	fr_delays = NULL;
	fr_jumph = NULL;
	fr_jumpx = NULL;
	fr_jumpz = NULL;
	fr_jump_timer = NULL;
	fr_jump_stairs = NULL;
	nb_stair = 0;

	// Bunch of var for animate the jump
	is_in_jump = false;
	jump_decal_x = jump_decal_z = jump_decal_alt = 0;
	jump_alt_step = 0;
	jump_last_delay = jump_time_counter = 0;
	draw_jump_step = false;

	TO_offx = 0;
	TO_offy = 0;
	TO_x = 0;
	TO_a = 0;
	TO_w = 0;
	TO_h = 0;

	mouse_draw = DRAW_NOTHING;
	b_mouseD = false;
	b_rightmouseD = false;
	Clear();
	dc = NULL;

	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
}

MyAnimationZone::~MyAnimationZone()
{
	Clear();
}

//******************************************************

int MyAnimationZone::H()const
{
	int _w, _h;
	GetSize(&_w,&_h);
	return _h;
}

int MyAnimationZone::W()const
{
	int _w, _h;
	GetSize(&_w,&_h);
	return _w;
}

int MyAnimationZone::X()const
{
	int mx,my;
	GetPosition(&mx,&my);
	return mx;
}

int MyAnimationZone::Y()const
{
	int mx,my;
	GetPosition(&mx,&my);
	return my;
}



//******************************************************

void MyAnimationZone::SetSize(int _x, int _y, int _width, int _height, int _sizeFlags )
{
	wxControl::SetSize(_x,_y,_width, _height,_sizeFlags);
}

//-------------------------------------------------------------------------------
void 
MyAnimationZone::pGetClientSize( int* w,int* h)
{
	GetClientSize( w,h);
#ifdef _WXGTK__ 
	if( GetScrollRange(wxHORIZONTAL) > 0 )
		h -= wxSystemSettings::GetMetric( wxSYS_HSCROLL_Y );
	if( GetScrollRange(wxVERTICAL) > 0 )
		w -= wxSystemSettings::GetMetric( wxSYS_HSCROLL_X );
#endif
}

//-------------------------------------------------------------------------------
void 
MyAnimationZone::Zoom(float _factor, bool b_exact_factor, bool b_recalc_size )
{
	if( _factor <= 0 || m_flag & M_ZOOMING )
		return;
	m_flag |= M_ZOOMING;
	
	// Vars...
	int w,h;
	pGetClientSize( &w,&h);
	
	// Get middle point in ob coords
	int sp_x = (w/2+GetScrollPos(wxHORIZONTAL) ) / zoom_factor;
	int sp_y = (h/2+GetScrollPos(wxVERTICAL  ) ) / zoom_factor;
	
	// Set the new zoom factor
	if( b_exact_factor == true )
		zoom_factor = _factor;
	else
		zoom_factor = zoom_factor*_factor;
	
	// If so, recalc size
	if( b_recalc_size == true );
		ReCalcAnimSize();

	// Recompute scrollbars pos
	sp_x = sp_x*zoom_factor - w/2;
	sp_y = sp_y*zoom_factor - h/2;
	
	if( total_W*zoom_factor < w )
		SetScrollbar( wxHORIZONTAL, 0,0,0);
	else
		SetScrollbar( wxHORIZONTAL, sp_x,w,total_W*zoom_factor);
	
	if( total_H*zoom_factor < h )
		SetScrollbar( wxVERTICAL, 0,0,0);
	else
		SetScrollbar( wxVERTICAL, sp_y,h,total_H*zoom_factor);


	// Must rethink the stuff if scroll bar appears
	int new_w,new_h;
	pGetClientSize( &new_w,&new_h);
	if( new_w != w || new_h != h )
	{
		m_flag &= ~M_ZOOMING;
		Zoom( zoom_factor, true, false );
		return;
	}
	
	// Repaint...
	RePaint();
	m_flag &= ~M_ZOOMING;
}

void MyAnimationZone::ResetZoom()
{
	Zoom( 1, true );
}


//******************************************************

extern bool Is_AAirAnim();

void MyAnimationZone::_Recalculate_JumpStuffs()
{
	// ****************************************************************************************
	// Init some jump variables

	ob_object *jumpframe_prop = curr_anim->GetSubObject( wxT("jumpframe") );
	int jump_frame = -1;
	int jump_speedh = 0;
	int jump_speedx = 0;
	int jump_speedz = 0;
	jump_alt_step = 0;

	if( jumpframe_prop != NULL && jumpframe_prop->GetToken(0) != wxString() )
	{
		jump_frame = StrToInt( jumpframe_prop->GetToken(0) );
		jump_speedh = StrToInt( jumpframe_prop->GetToken(1) );
		jump_alt_step = jump_speedh;
		jump_speedx = StrToInt( jumpframe_prop->GetToken(2) );
		jump_speedz = StrToInt( jumpframe_prop->GetToken(3) );
	}
	// Provide some default values for fall frames
	else if( Is_AAirAnim() )
	{
		jump_frame = 0;
		jump_speedh = 3;
		if( curr_anim->GetToken(0).Left(4).Upper() == wxT("FALL") )
			jump_speedx = -2;
		else if( curr_anim->GetToken(0).Left(4).Upper() == wxT("JUMP") )
			jump_speedx = 0;
	}

	ob_object *dropframe_prop = curr_anim->GetSubObject( wxT("dropframe") );
	int drop_frame = -1;
	if( dropframe_prop != NULL && dropframe_prop->GetToken(0) != wxString() )
		drop_frame = StrToInt( dropframe_prop->GetToken(0) );

	ob_object *landframe_prop = curr_anim->GetSubObject( wxT("landframe") );
	int land_frame = -1;
	if( landframe_prop != NULL && landframe_prop->GetToken(0) != wxString() )
		land_frame = StrToInt( landframe_prop->GetToken(0) );


	bool in_jump = false, in_drop = false, in_land = false;
	int know_delay = 0;
	int curr_jumph = 0;;
	int jump_duration = 0;
	int curr_alt = 0;

#define DGET_DELAY( _frame ) \
	(_frame->delay!=NO_CLONED_VAL?_frame->delay:1)

	//******************************************************************
	// Calculate delays and a,x,z decals
	for( int i = 0; i < curr_frames_count; i++ )
	{
		if( curr_frames[i]->jumpState > 0 && ! in_jump )
			in_jump = true;
		if( curr_frames[i]->dropState > 0 && ! in_drop )
			in_drop = true;
		if( curr_frames[i]->landState > 0 && ! in_land )
			in_land = true;

		know_delay = DGET_DELAY(curr_frames[i]);
		if( know_delay <= 0 )
			know_delay = 1;

		curr_frames[i]->end_jump = false;

		bool b_flipped = curr_frames[i]->flipState > 0;

		//***********************************
		// Special Case the drop frame
		if( curr_frames[i]->dropState == 2 && curr_frames[i]->jumpState > 0 )
		{
			if( i == 0 )
			{
				wxMessageBox( wxT("Drop frame at position ZERO !!!"), wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
				fr_delays[i] = know_delay;
				fr_jumph[i] = 0;
				fr_jumpx[i] = 0;
				fr_jumpz[i] = 0;
				continue;
			}

			// Must force the previous frame delay to have this frame to the apex
			if( jump_duration < JUMP_HALF_TIME )
			{
				// simple case : juste force the previous delay so it be at the apex
				fr_delays[i-1] = ( JUMP_HALF_TIME - jump_duration + fr_delays[i-1] );
				fr_jumph[i] = fr_delays[i-1] * jump_speedh;
				fr_jumpx[i] = fr_delays[i-1] * jump_speedx;
				fr_jumpz[i] = fr_delays[i-1] * jump_speedz;
			}

			// Complicated case : the apex is passed
			else
			{
				int over_time = jump_duration - JUMP_HALF_TIME;

				int j;
				for( j = i-1; j >= 0; j--)
				{
					over_time -= fr_delays[j];

					// We got the last frame before the apex
					if( over_time < 0 )
					{
						fr_delays[j] = - over_time;
						break;
					}
				}
				fr_jumph[i] = fr_delays[j] * jump_speedh;
				fr_jumpx[i] = fr_delays[j] * jump_speedx;
				fr_jumpz[i] = fr_delays[j] * jump_speedz;

				// Set a delay of 0 to all the frame between
				for( j = j + 1; j < i; j++ )
				{
					fr_delays[j] = 0;
					fr_jumph[j] = 0;
					fr_jumpx[j] = 0;
					fr_jumpz[j] = 0;
				}
			}

			// Set the drop frame data
			fr_delays[i] = know_delay;
			curr_jumph = jump_speedh * JUMP_HALF_TIME;
			jump_duration = JUMP_HALF_TIME;
			in_land = false;
		}

		//***********************************
		// Special Case the land frame
		else if( curr_frames[i]->landState == 2 && curr_frames[i]->jumpState > 0 )
		{
			// Must force the delay to have this frame (and all remaining previous) to the ground

			//Simple case : just add some time to the land frame
			if( jump_duration < 2 * JUMP_HALF_TIME )
			{
				if( i == 0 )
				{
					wxMessageBox( wxT("Land frame at position ZERO !!!"), wxT("ProPlem"), wxOK | wxICON_INFORMATION, this );
					fr_delays[i] = know_delay;
					fr_jumph[i] = 0;
					fr_jumpx[i] = 0;
					fr_jumpz[i] = 0;
					continue;
				}
				// simple case : juste force the previous delay
				fr_delays[i-1] = ( 2 * JUMP_HALF_TIME - jump_duration + fr_delays[i-1] );

				// Set the drop frame data
				if( fr_delays[i-1] <= JUMP_HALF_TIME )
					fr_jumph[i] = - fr_delays[i-1] * jump_speedh;
				else
				{
					int last_up_delay = fr_delays[i-1] - JUMP_HALF_TIME;
					fr_jumph[i] = - (fr_delays[i-1] -2 * last_up_delay ) * jump_speedh;
				}
				fr_delays[i] = know_delay;
				fr_jumpx[i] = fr_delays[i-1] * jump_speedx;
				fr_jumpz[i] = fr_delays[i-1] * jump_speedz;
			}

			// Complicated case : the ground is passed
			else
			{
				int over_time = jump_duration - 2 * JUMP_HALF_TIME;

				int j;
				for( j = i-1; j >= 0; j--)
				{
					over_time -= fr_delays[j];

					// We got the last frame before the ground
					if( over_time < 0 )
					{
						fr_delays[j] = - over_time;
						break;
					}
				}

				// Set the drop frame data
				if( fr_delays[j] <= JUMP_HALF_TIME )
					fr_jumph[i] = - fr_delays[j] * jump_speedh;
				else
				{
					int last_up_delay = fr_delays[j] - JUMP_HALF_TIME;
					fr_jumph[i] = - (fr_delays[j] -2 * last_up_delay ) * jump_speedh;
				}
				fr_delays[i] = know_delay;
				fr_jumpx[i] = fr_delays[j] * jump_speedx;
				fr_jumpz[i] = fr_delays[j] * jump_speedz;

				// Set a delay of 0 to all the frame between
				for( j = j + 1; j < i; j++ )
				{
					fr_delays[j] = 0;
					fr_jumph[j] = 0;
					fr_jumpx[j] = 0;
					fr_jumpz[j] = 0;
				}
			}

			curr_jumph = 0; //jump_speedh * JUMP_HALF_TIME;
			jump_duration = 2 * JUMP_HALF_TIME;
			curr_frames[i]->end_jump = true;
			in_land = true;
		}

		// at the ground -> no jump influence
		else if( in_land || ! in_jump )
		{
			fr_delays[i] = know_delay;
			fr_jumph[i] = 0;
			fr_jumpx[i] = 0;
			fr_jumpz[i] = 0;
			if( curr_frames[i]->jumpState != 2 )
				curr_frames[i]->end_jump = true;
		}

		// go up case
		else if( in_jump && ! in_drop )
		{
			// Special case : apex passed
			if( jump_duration >= JUMP_HALF_TIME && jump_duration < 2 * JUMP_HALF_TIME )
			{
				fr_delays[i] = know_delay;
				fr_jumpx[i] = 0;
				fr_jumpz[i] = 0;

				fr_jumph[i] = (JUMP_HALF_TIME - ( jump_duration - JUMP_HALF_TIME )) * jump_speedh;
				if( i > 0 && curr_frames[i-1]->jumpState > 0)
				{
					int last_up_delay = fr_delays[i-1] - (jump_duration - JUMP_HALF_TIME);
					fr_jumph[i] = - (fr_delays[i-1] -2 * last_up_delay ) * jump_speedh;
					fr_jumpx[i] = fr_delays[i-1] * jump_speedx;
					fr_jumpz[i] = fr_delays[i-1] * jump_speedz;
				}
				in_drop = true;
			}

			// Special case : apex passed and directly to ground
			else if( jump_duration >= 2 * JUMP_HALF_TIME )
			{
				fr_delays[i] = know_delay;
				fr_jumpx[i] = 0;
				fr_jumpz[i] = 0;
				fr_jumph[i] = 0;

				// Gotta cut some delay and some alt to stay to ground
				if( i > 0 && curr_frames[i-1]->jumpState > 0)
				{
					int over_time = jump_duration - 2 * JUMP_HALF_TIME;
					fr_delays[i-1] -= over_time;
					jump_duration  -= over_time;
					fr_jumph[i] = - curr_alt;
					fr_jumpx[i] = fr_delays[i-1] * jump_speedx;
					fr_jumpz[i] = fr_delays[i-1] * jump_speedz;
				}
				in_land = true;
				curr_frames[i]->end_jump = true;
			}

			// Just moving up
			else
			{
				fr_delays[i] = know_delay;
				fr_jumpx[i] = 0;
				fr_jumpz[i] = 0;
				fr_jumph[i] = 0;

				if( i > 0 && curr_frames[i-1]->jumpState > 0 )
				{
					fr_jumph[i] = fr_delays[i-1] * jump_speedh;
					fr_jumpx[i] = fr_delays[i-1] * jump_speedx;
					fr_jumpz[i] = fr_delays[i-1] * jump_speedz;
				}
			}
		}

		// go down case
		else if( in_jump && in_drop )
		{
			// Special case : ground passed
			if( jump_duration >= 2 * JUMP_HALF_TIME )
			{
				fr_delays[i] = know_delay;
				fr_jumpx[i] = 0;
				fr_jumpz[i] = 0;
				fr_jumph[i] = 0;

				// Gotta cut some alt
				if( i > 0 && curr_frames[i-1]->jumpState > 0)
				{
					int over_time = jump_duration - 2 * JUMP_HALF_TIME;
					jump_duration  -= over_time;
					fr_delays[i-1] -= over_time;
					fr_jumph[i] = - fr_delays[i-1] * jump_speedh;
					fr_jumpx[i] = fr_delays[i-1] * jump_speedx;
					fr_jumpz[i] = fr_delays[i-1] * jump_speedz;
				}
				in_land = true;
				curr_frames[i]->end_jump = true;
			}

			// just going down
			else
			{
				fr_delays[i] = know_delay;
				fr_jumpx[i] = 0;
				fr_jumpz[i] = 0;
				fr_jumph[i] = 0;
				if( i > 0 && curr_frames[i-1]->jumpState > 0)
				{
					fr_jumph[i] = - fr_delays[i-1] * jump_speedh;
					fr_jumpx[i] = fr_delays[i-1] * jump_speedx;
					fr_jumpz[i] = fr_delays[i-1] * jump_speedz;
				}
			}
		}

		if( b_flipped )
			fr_jumpx[i] = - fr_jumpx[i];

		if( curr_frames[i]->jumpState > 0 )
		{
			jump_duration += fr_delays[i];
			curr_alt += fr_jumph[i];
		}
	}


	// Now we can calculate each delay from the start of the jump
	fr_jump_timer[0] = 0;
	for( int _i = 1; _i < curr_frames_count; _i++ )
	{
		if( curr_frames[_i]->jumpState == 2 )
			fr_jump_timer[_i] = 0;
		else if( curr_frames[_i]->jumpState > 0 )
			fr_jump_timer[_i] = fr_jump_timer[_i-1] + fr_delays[_i-1];
		else
			fr_jump_timer[_i] = 0;
	}


	// Now we can calculate each jump step
	for( int i = 0; i < curr_frames_count; i++ )
	{
		fr_jump_stairs[i].nb_step = 0;
		fr_jump_stairs[i].curr_step = 0;

		if( curr_frames[i]->end_jump )
			continue;

		// Number of jump need by this frame
		int nb_step = floor( ((float) fr_delays[i] - (float) JUMP_ANIM_STEP )/ (float) JUMP_ANIM_STEP );
		fr_jump_stairs[i].nb_step = nb_step;
		if( nb_step <= 0 )
		{
			fr_jump_stairs[i].nb_step = 0;
			continue;
		}

		fr_jump_stairs[i].steps = new jump_step[nb_step];

		// Set the delays of each steps
		for( int j = 0; j < nb_step - 1; j++)
			fr_jump_stairs[i].steps[j].delay = JUMP_ANIM_STEP;
		// And the last step
		fr_jump_stairs[i].steps[nb_step - 1].delay = fr_delays[i] - JUMP_ANIM_STEP * nb_step;

		// Setting the move_h of each step
		int time_elapsed = fr_jump_timer[i] + JUMP_ANIM_STEP;
		for( int j = 0; j < nb_step; j++)
		{
			// case, apex passed
			if(    (time_elapsed - JUMP_ANIM_STEP) <= JUMP_HALF_TIME
				&&	time_elapsed  > JUMP_HALF_TIME )
			{
				int _moveh = JUMP_ANIM_STEP * jump_speedh;
				_moveh -= (time_elapsed  - JUMP_HALF_TIME ) * jump_speedh * 2;
				fr_jump_stairs[i].steps[j].mv_h = _moveh;
			}

			// Going up
			else if( time_elapsed < JUMP_HALF_TIME )
				fr_jump_stairs[i].steps[j].mv_h = JUMP_ANIM_STEP * jump_speedh;

			// on the ground
			else if( time_elapsed >= 2 * JUMP_HALF_TIME )
			{
				fr_jump_stairs[i].steps[j].mv_h = 0;
				// Stop calculation, and stop the frame step to this one
				fr_jump_stairs[i].nb_step = j+1;
				fr_jump_stairs[i].steps[j].delay = fr_jump_timer[i] + fr_delays[i] - time_elapsed;
				 j = nb_step;
			}

			// Ground reached
			else if( (time_elapsed + JUMP_ANIM_STEP) >= 2 * JUMP_HALF_TIME )
			{
				fr_jump_stairs[i].steps[j].mv_h = - (2*JUMP_HALF_TIME - time_elapsed ) * jump_speedh;
				// Stop calculation, and stop the frame step to this one
				fr_jump_stairs[i].nb_step = j+1;
				fr_jump_stairs[i].steps[j].delay = fr_jump_timer[i] + fr_delays[i] - time_elapsed;
				 j = nb_step;
			}

			// Going down
			else
				fr_jump_stairs[i].steps[j].mv_h = - JUMP_ANIM_STEP * jump_speedh;

			time_elapsed += JUMP_ANIM_STEP;
		}
	}
}


//******************************************************

void getRotationNewOffset( int off_x, int off_y, int _w, int _h, int angle,
								float& new_offx, float& new_offy,float& new_w, float& new_h );

void MyAnimationZone::ReCalcAnimSize()
{
	// Reinit size datas
	if( fr_decal_Xs != NULL )
		delete[] fr_decal_Xs;
	fr_decal_Xs = NULL;
	if( fr_decal_Ys != NULL )
		delete[] fr_decal_Ys;
	fr_decal_Ys = NULL;
	if( fr_As != NULL )
		delete[] fr_As;
	fr_As = NULL;
	if( fr_Zdecal != NULL )
		delete[] fr_Zdecal;
	fr_Zdecal = NULL;

	if( fr_total_decal_Xs != NULL )
		delete[] fr_total_decal_Xs;
	fr_total_decal_Xs = NULL;
	if( fr_total_decal_Ys != NULL )
		delete[] fr_total_decal_Ys;
	fr_total_decal_Ys = NULL;

	if( fr_offset_Xs != NULL )
		delete[] fr_offset_Xs;
	fr_offset_Xs = NULL;
	if( fr_offset_Ys != NULL )
		delete[] fr_offset_Ys;
	fr_offset_Ys = NULL;
	if( fr_DM_decal_Xs != NULL )
		delete[] fr_DM_decal_Xs;
	fr_DM_decal_Xs = NULL;
	if( fr_DM_decal_Ys != NULL )
		delete[] fr_DM_decal_Ys;
	fr_DM_decal_Ys = NULL;
	if( fr_rotate_offset_Xs != NULL )
		delete[] fr_rotate_offset_Xs;
	fr_rotate_offset_Xs = NULL;
	if( fr_rotate_offset_Ys != NULL )
		delete[] fr_rotate_offset_Ys;
	fr_rotate_offset_Ys = NULL;
	if( original_img_w != NULL )
		delete[] original_img_w;
	original_img_w = NULL;
	if( original_img_h != NULL )
		delete[] original_img_h;
	original_img_h = NULL;
	if( current_img_w != NULL )
		delete[] current_img_w;
	current_img_w = NULL;
	if( current_img_h != NULL )
		delete[] current_img_h;
	current_img_h = NULL;
	if( fr_rotate_imgw != NULL )
		delete[] fr_rotate_imgw;
	fr_rotate_imgw = NULL;
	if( fr_rotate_imgh != NULL )
		delete[] fr_rotate_imgh;
	fr_rotate_imgh = NULL;
	if( fr_delays != NULL )
		delete[] fr_delays;
	fr_delays = NULL;
	if( fr_jumph != NULL )
		delete[] fr_jumph;
	fr_jumph = NULL;
	if( fr_jumpx != NULL )
		delete[] fr_jumpx;
	fr_jumpx = NULL;
	if( fr_jumpz != NULL )
		delete[] fr_jumpz;
	fr_jumpz = NULL;
	if( fr_jump_timer != NULL )
		delete[] fr_jump_timer;
	fr_jump_timer = NULL;

	clear_jump_stairs( fr_jump_stairs, nb_stair );

	if( curr_frames == NULL )
	{
		Clear();
		return;
	}

	// reinit vars
	fr_decal_Xs = new int[curr_frames_count];
	fr_decal_Ys = new int[curr_frames_count];
	fr_total_decal_Xs = new int[curr_frames_count];
	fr_total_decal_Ys = new int[curr_frames_count];
	fr_As = new int[curr_frames_count];
	fr_Zdecal = new int[curr_frames_count];
	fr_offset_Xs = new int[curr_frames_count];
	fr_offset_Ys = new int[curr_frames_count];
	fr_DM_decal_Xs = new int[curr_frames_count];
	fr_DM_decal_Ys = new int[curr_frames_count];
	fr_rotate_offset_Xs = new int[curr_frames_count];
	fr_rotate_offset_Ys = new int[curr_frames_count];
	original_img_w = new int[curr_frames_count];
	original_img_h = new int[curr_frames_count];
	current_img_w = new int[curr_frames_count];
	current_img_h = new int[curr_frames_count];
	fr_rotate_imgw = new int[curr_frames_count];
	fr_rotate_imgh = new int[curr_frames_count];
	fr_delays = new int[curr_frames_count];
	fr_jumph = new int[curr_frames_count];
	fr_jumpx = new int[curr_frames_count];
	fr_jumpz = new int[curr_frames_count];
	fr_jump_timer = new int[curr_frames_count];
	fr_jump_stairs = new jump_stair[curr_frames_count];
	nb_stair = curr_frames_count;

	total_W = 0;
	total_H = 0;

	// The max depassage relative to the first frame left-top corner
	float max_spilling_X = 0;
	float max_spilling_Y = 0;

	// Information on the previous frame
	int prev_offset_X = 0;
	int prev_offset_Y = 0;
	int prev_total_decalx = 0;
	int prev_total_decaly = 0;

	min_decal_X = 0;
	min_decal_Y = 0;

	int prev_fr_time = 0;


	// ****************************************************************************************
	//********************************************
	// first recalculate jumps datas
	_Recalculate_JumpStuffs();


	// ****************************************************************************************
	// Init some jump datas
	ob_object *jumpframe_prop = curr_anim->GetSubObject( wxT("jumpframe") );
	int jump_speedx = 0;
	int jump_speedz = 0;

	if( jumpframe_prop != NULL && jumpframe_prop->GetToken(0) != wxString() )
	{
		jump_speedx = StrToInt( jumpframe_prop->GetToken(2) );
		jump_speedz = StrToInt( jumpframe_prop->GetToken(3) );
	}
	else if( Is_AAirAnim() )
	{
		if( curr_anim->GetToken(0).Left(4).Upper() == wxT("FALL") )
			jump_speedx = -2;
		else if( curr_anim->GetToken(0).Left(4).Upper() == wxT("JUMP") )
			jump_speedx = 0;
		jump_speedz = 0;
	}


	// ****************************************************************************************
	// getting the decal for each frame and the overall decal relative with the first frame
#define DGET_DMVAL(prop,k) \
	(curr_frames[i]->prop[k]!=NO_CLONED_VAL?curr_frames[i]->prop[k]:0)
#define DGET_SVAL(prop) \
	(curr_frames[i]->prop!=NO_CLONED_VAL?curr_frames[i]->prop:0)
	
	for( int i = 0; i < curr_frames_count; i++ )
	{
		// Get size of the img of the frame
		wxImage *fr_img = curr_frames[i]->GetImage();

		if( fr_img == NULL )
			fr_img = noImg;
		float img_w = fr_img->GetWidth();
		float img_h = fr_img->GetHeight();
		original_img_w[i] = img_w;
		original_img_h[i] = img_h;

		int offsetX = DGET_DMVAL(offset,0);
		int original_offsetX = offsetX;
		int offsetY = DGET_DMVAL(offset,1);
		int original_offsetY = offsetY;
		int movex   = DGET_SVAL(move);
		int movea   = DGET_SVAL(moveA);
		int movez   = DGET_SVAL(moveZ);

		bool b_flipped = false;
		if( curr_frames[i]->flipState > 0 )
		{
			movex = -movex;
			fr_jumpx[i] = - fr_jumpx[i];
			original_offsetX = img_w - offsetX;
			offsetX = original_offsetX;
			b_flipped = true;
		}

		// Apply drawmethod transformations
		if( 	curr_frames[i]->GetSubObject( wxT("nodrawmethod") ) == NULL
			&&	curr_frames[i]->GetSubObject( wxT("drawmethod")) != NULL )
		{
			// drawmethod datas
			int _i = 0;
			int _scaleX = DGET_DMVAL(drawMethod,_i);
			_i++;
			int _scaleY = DGET_DMVAL(drawMethod,_i);
			_i++;
			int _flipX = DGET_DMVAL(drawMethod,_i);
			_i++;
			int _flipY = DGET_DMVAL(drawMethod,_i);
			_i += 5;
			int _rotate = DGET_DMVAL(drawMethod,_i);
			_i++;
			bool _fliprotate = (DGET_DMVAL(drawMethod,_i) != 0);

			// scale the offset
			if( _scaleX != 0 )
			{
				float _dm_scaleX = ((float) _scaleX / (float) 256 );
				offsetX *= _dm_scaleX;
				img_w *= _dm_scaleX;
				if( _dm_scaleX < 0 )
				{
					img_w = - img_w;
					offsetX = img_w + offsetX;
				}
			}
			if( _scaleY != 0 )
			{
				float _dm_scaleY = ((float) _scaleY / (float) 256 );
				offsetY *= _dm_scaleY;
				img_h *= _dm_scaleY;
				if( _dm_scaleY < 0 )
				{
					img_h = - img_h;
					offsetY = img_h + offsetY;
				}
			}
			if( _flipX )
			{
				offsetX = img_w - offsetX;
			}
			if( _flipY )
			{
				offsetY = img_h - offsetY;
			}

			fr_rotate_imgw[i] = img_w;
			fr_rotate_imgh[i] = img_h;

			// Rotation stuff
			if( _rotate != 0 )
			{
				if( b_flipped && _fliprotate )
					_rotate = - _rotate;
				fr_rotate_offset_Xs[i] = offsetX;
				fr_rotate_offset_Ys[i] = offsetY;
				float new_img_w, new_img_h, new_off_x,new_off_y;
				getRotationNewOffset( offsetX, offsetY, ceil(img_w), ceil(img_h),
										_rotate, new_off_x, new_off_y, new_img_w, new_img_h );
				offsetX = new_off_x;
				offsetY = new_off_y;
				img_w = new_img_w;
				img_h = new_img_h;
			}
		}

		img_w = ceil(img_w);
		img_h = ceil(img_h);
		current_img_w[i] = img_w;
		current_img_h[i] = img_h;

		fr_offset_Xs[i] = offsetX;
		fr_offset_Ys[i] = offsetY;
		fr_DM_decal_Xs[i] = original_offsetX - offsetX;
		fr_DM_decal_Ys[i] = original_offsetY - offsetY;

		int tot_decal_x = 0, tot_decal_y = 0;
		if( i > 0 )
		{
			int tot_movea = - movea;
			int tot_movez = movez;
			int tot_movex = movex;

			// If not in a jump
			if( curr_frames[i]->jumpState == 0  )
				fr_As[i] =  - movea + fr_As[i-1];

			// In a jump, must compute the new altitude
			else
			{
				// This one have no delay -> no implications
				if( fr_delays[i] == 0 )
				{
					fr_As[i]    = fr_As[i-1];
					fr_decal_Xs[i] = 0;
					fr_Zdecal[i] = 0;
					tot_movea = 0;
					tot_movez = 0;
					tot_movex = 0;
					fr_jump_stairs[i].mv_x = 0;
					fr_jump_stairs[i].mv_z = 0;
				}
				else
				{
					fr_As[i]    = - fr_jumph[i]  + fr_As[i-1];
					fr_decal_Xs[i] += fr_jumpx[i];
					fr_Zdecal[i] -= fr_jumpz[i];
					tot_movea = - fr_jumph[i];
					tot_movez += fr_jumpz[i];
					tot_movex += fr_jumpx[i];
					fr_jump_stairs[i].mv_x = (float)movex / (float)fr_delays[i] + (float) jump_speedx;
					fr_jump_stairs[i].mv_z = (float)movez / (float)fr_delays[i] + (float) jump_speedz;
				}
			}

			int decalx = prev_offset_X + tot_movex - offsetX;
			int decaly = prev_offset_Y  + tot_movez + tot_movea - offsetY;
			tot_decal_x =  decalx + prev_total_decalx;
			tot_decal_y =  decaly + prev_total_decaly;

			fr_decal_Xs[i] = decalx;
			fr_decal_Ys[i] = decaly;
			fr_total_decal_Xs[i] = tot_decal_x;
			fr_total_decal_Ys[i] = tot_decal_y;

			fr_Zdecal[i] = -movez + fr_Zdecal[i-1];

		}
		else
		{
			fr_decal_Xs[i] = 0;
			fr_decal_Ys[i] = 0;
			fr_As[i] = 0;
			fr_Zdecal[i] = 0;
			fr_total_decal_Xs[i] = 0;
			fr_total_decal_Ys[i] = 0;

			// In a jump, must compute the new altitude
			if( curr_frames[i]->jumpState > 0  )
			{
				fr_jump_stairs[i].mv_x = (float)movex / (float)fr_delays[i] + (float) jump_speedx;
				fr_jump_stairs[i].mv_z = (float)movez / (float)fr_delays[i] + (float) jump_speedz;
			}
		}

		if( tot_decal_x < min_decal_X )
			min_decal_X = tot_decal_x;
		if( tot_decal_y < min_decal_Y )
			min_decal_Y = tot_decal_y;

		int curr_spilling_X = tot_decal_x + img_w;
		int curr_spilling_Y = tot_decal_y + img_h;

		if( curr_spilling_X > max_spilling_X )
			max_spilling_X = curr_spilling_X;
		if( curr_spilling_Y > max_spilling_Y )
			max_spilling_Y = curr_spilling_Y;

		// in case of a jump, have to calc all overhead of every jump step
		if( curr_frames[i]->jumpState > 0 && ! curr_frames[i]->end_jump )
		{
			int nb_step = fr_jump_stairs[i].nb_step;
			int x_decal = fr_jump_stairs[i].mv_x;
			int z_decal = fr_jump_stairs[i].mv_z;

			float step_tot_dec_x = 0;
			float step_tot_dec_y = 0;

			for( int j = 0; j < nb_step; j ++ )
			{
				int mv_h  = fr_jump_stairs[i].steps[j].mv_h;
				int delay = JUMP_ANIM_STEP;
				if( j>0 )
					delay = fr_jump_stairs[i].steps[j-1].delay;

				step_tot_dec_x += (float) delay * x_decal;
				step_tot_dec_y += (float) delay * (float) z_decal - (float) mv_h;

				// Update maxs
				if( tot_decal_x + step_tot_dec_x < min_decal_X )
					min_decal_X = tot_decal_x + step_tot_dec_x;
				if( tot_decal_y + step_tot_dec_y < min_decal_Y )
					min_decal_Y = tot_decal_y + step_tot_dec_y;

				if( curr_spilling_X  + step_tot_dec_x> max_spilling_X )
					max_spilling_X = curr_spilling_X + step_tot_dec_x;
				if( curr_spilling_Y  + step_tot_dec_y > max_spilling_Y )
					max_spilling_Y = curr_spilling_Y  + step_tot_dec_y;
			}
		}

		prev_offset_X = offsetX;
		prev_offset_Y = offsetY;
		prev_total_decalx = tot_decal_x;
		prev_total_decaly = tot_decal_y;

		prev_fr_time = DGET_DELAY(curr_frames[i]);
	}

	total_W = ceil( - min_decal_X + max_spilling_X);
	total_H = ceil( - min_decal_Y + max_spilling_Y );
	
	if( 	   GetScrollRange(wxHORIZONTAL) != (total_W * zoom_factor)
		|| GetScrollRange(wxVERTICAL  ) != (total_H * zoom_factor)
		)
		Zoom( zoom_factor, true, false );

	Refresh();
}


//******************************************************
enum { MYALL, MYRIGHT, MYLEFT };

void MyAnimationZone::UpdateFrames()
{
	ReCalcAnimSize();
	RePaint();
}

//******************************************************

wxSize MyAnimationZone::DoGetBestSize() const
{
	return GetContainingSizer()->GetSize();
}


void MyAnimationZone::Clear()
{
	if( theBitmap != NULL )
		delete theBitmap;
	theBitmap = NULL;

	if( fr_decal_Xs != NULL )
		delete[] fr_decal_Xs;
	fr_decal_Xs = NULL;

	if( fr_decal_Ys != NULL )
		delete[] fr_decal_Ys;
	fr_decal_Ys = NULL;

	if( fr_As != NULL )
		delete[] fr_As;
	fr_As = NULL;

	if( fr_total_decal_Xs != NULL )
		delete[] fr_total_decal_Xs;
	fr_total_decal_Xs = NULL;

	if( fr_total_decal_Ys != NULL )
		delete[] fr_total_decal_Ys;
	fr_total_decal_Ys = NULL;

	if( fr_rotate_offset_Xs != NULL )
		delete[] fr_rotate_offset_Xs;
	fr_rotate_offset_Xs = NULL;


	if( fr_rotate_offset_Ys != NULL )
		delete[] fr_rotate_offset_Ys;
	fr_rotate_offset_Ys = NULL;

	if( original_img_w != NULL )
		delete[] original_img_w;
	original_img_w = NULL;

	if( original_img_h != NULL )
		delete[] original_img_h;
	original_img_h = NULL;

	if( current_img_w != NULL )
		delete[] current_img_w;
	current_img_w = NULL;

	if( current_img_h != NULL )
		delete[] current_img_h;
	current_img_h = NULL;

	if( fr_rotate_imgw != NULL )
		delete[] fr_rotate_imgw;
	fr_rotate_imgw = NULL;

	if( fr_rotate_imgh != NULL )
		delete[] fr_rotate_imgh;
	fr_rotate_imgh = NULL;

	if( fr_delays != NULL )
		delete[] fr_delays;
	fr_delays = NULL;

	if( fr_jumph != NULL )
		delete[] fr_jumph;
	fr_jumph = NULL;

	if( fr_jumpx != NULL )
		delete[] fr_jumpx;
	fr_jumpx = NULL;

	if( fr_jumpz != NULL )
		delete[] fr_jumpz;
	fr_jumpz = NULL;

	if( fr_jump_timer != NULL )
		delete[] fr_jump_timer;
	fr_jump_timer = NULL;

	clear_jump_stairs( fr_jump_stairs, nb_stair );

	total_W = 0;
	total_H = 0;
	min_decal_X = 0;
	min_decal_Y = 0;

	m_flag &= ~M_PAINTING;

	Zoom( 3, true );

	Refresh();
}


//******************************************************

void MyAnimationZone::EvtMouseSomething(wxMouseEvent& event)
{
	// No mouse something with no frame avalaible
	if( curr_frames == NULL || curr_anim == NULL || fr_total_decal_Xs == NULL )
		return;

	int mx = event.m_x;
	int my = event.m_y;

	// Gather information about the frame currently drawn
		// Getting the indice of the frame
		int _ind = ind_active_frame;
		if( _ind < 0 )
			_ind = 0;
		if( _ind >= curr_frames_count )
			return;

		wxImage *fr_img = curr_frames[_ind]->GetImage();
		if( fr_img == NULL )
			fr_img = noImg;
		int img_w = (int)((float)fr_img->GetWidth() * zoom_factor);
		int img_h = (int)((float)fr_img->GetHeight() * zoom_factor );
		if( img_w <= 5 && img_h <= 5 )
		{
			fr_img = noImg;
			img_w = (int)((float)fr_img->GetWidth() * zoom_factor);
			img_h = (int)((float)fr_img->GetHeight() * zoom_factor );
		}

		// Gather Infos about the painting zone
		int _w,_h;
		GetSize( &_w, &_h);

// 		decal_x = 0;
// 		decal_y = 0;
// 		if( (int)((float)total_W*zoom_factor) < _w )
// 			decal_x = (_w - (int)((float)total_W*zoom_factor)) /2;
// 		if( (int)((float)total_H*zoom_factor) < _h )
// 			decal_y = (_h - (int)((float)total_H*zoom_factor)) /2;

		// Getting the decals for the current frame
    	int dc_x = fr_total_decal_Xs[_ind];
    	int dc_y = fr_total_decal_Ys[_ind];

	// Translate mouse coord to animation coord
	mx -= decal_x + (int)((float)(dc_x - min_decal_X)*zoom_factor);
	my -= decal_y + (int)((float)(dc_y - min_decal_Y)*zoom_factor);

	mx = (int)((float)mx / zoom_factor);
	my = (int)((float)my / zoom_factor);

	if( event.Moving() == true )
	{
		// Put the tooltip
		wxString tip_txt=
			  IntToStr(mx)
			  + wxT(" , ") 
			+ IntToStr(my)
			;
		theTip->SetTip( tip_txt );
		theTip->Enable(true);
		event.Skip();
		return;
	}
	if( event.IsButton() == true )
		theTip->Enable(false);
	
	// Left click
	if( event.ButtonUp() )
	{
		b_mouseD = false;
		// Simple click
		if( mouse_draw == DRAW_NOTHING || ( abs(r_x - mx) < 2 && abs(r_y - my) < 2 ))
		{
			TO_x = mx - fr_offset_Xs[_ind];
			if( curr_frames[_ind]->flipState > 0 )
				TO_x = -TO_x;
			TO_x +=  + curr_frames[_ind]->move;
			TO_a = fr_offset_Ys[_ind] - my + curr_frames[_ind]->moveA;

			ReverseDMCoord( mx, my);
			ReverseFPCoord(mx);
			TO_offx = mx;
			TO_offy = my;

			TO_w = 0;
			TO_h = 0;
			mouse_draw = DRAW_AIM;
			OnSimpleClick(event);
			RePaint();
			return;
		}
		else
		{
			if( my < r_y)
			{
				int t = r_y;
				r_y = my;
				my = t;
			}
			if( mx < r_x )
			{
				int t = r_x;
				r_x = mx;
				mx = t;
			}

			r_w = mx - r_x;
			r_h = my - r_y;

			TO_x = 0;
			TO_a = 0;
			TO_w = r_w;
			TO_h = r_h;

			// More work with the offsets due To DM
			TO_offx = r_x;
			TO_offy = r_y;

			// Apply the Flipframe mod
			ReverseFPCoord(TO_offx,r_w);

			// Take the Offset decal due to the DM
			TO_offx += fr_DM_decal_Xs[_ind];
			TO_offy += fr_DM_decal_Ys[_ind];

			OnEndLeftDragging();
			RePaint();
			return;
		}
	}

	if( event.ButtonDown() || event.RightDown() )
	{
		TO_offx = 0;
		TO_offy = 0;
		TO_x = 0;
		TO_a = 0;
		TO_w = 0;
		TO_h = 0;
		mouse_draw = DRAW_NOTHING;
		OnAnyButtonDown();
	}

	if( event.ButtonDown() )
	{
		b_mouseD = true;
		b_rightmouseD = false;
		r_x = mx;
		r_y = my;
		OnLeftButtonDown();
		RePaint();
		return;
	}

	/*
	if( event.RightDown() )
	{
		b_mouseD = false;
		b_rightmouseD = true;
		r_right_x = mx;
		r_right_y = my;
		RePaint();
		return;
	}
*/

	if( event.Dragging() && event.m_leftDown )
	{
		if( ! b_mouseD )
			return;
		mouse_draw = DRAW_RECT;
		r_w = mx - r_x;
		r_h = my - r_y;
		OnDragging();
		RePaint();
		return;
	}

	event.Skip();
}

void MyAnimationZone::OnSimpleClick(wxMouseEvent& event)
{
}

void MyAnimationZone::OnEndLeftDragging()
{
}

void MyAnimationZone::OnAnyButtonDown()
{
}

void MyAnimationZone::OnLeftButtonDown()
{
}

void MyAnimationZone::OnDragging()
{
}


//******************************************************

void MyAnimationZone::ResetMouseDraw()
{
	TO_offx = 0;
	TO_offy = 0;
	TO_x = 0;
	TO_a = 0;
	TO_w = 0;
	TO_h = 0;
	mouse_draw = DRAW_NOTHING;
	RePaint();
}

//******************************************************

void MyAnimationZone::OnEraseBackground(wxEraseEvent& event)
{
	event.Skip();
}

//******************************************************

void MyAnimationZone::RePaint()
{
	Refresh();
}

//******************************************************

void MyAnimationZone::Draw_NoImg()
{
	int _w,_h;
	GetSize( &_w, &_h);
	if( _w > 0 && _h > 0 )
	{
		theBitmap = new wxBitmap( noImg->Scale(_w/2, _h/2));
		dc->DrawBitmap( *theBitmap, _w/4, _h/4, true );
	}
}

//******************************************************

void MyAnimationZone::myDrawLine( int _x0, int _y0, int _x1, int _y1 )
{
	dc->DrawLine( 	decal_x + (int)((float)(_x0 - min_decal_X)*zoom_factor),
					decal_y + (int)((float)(_y0 - min_decal_Y)*zoom_factor),
					decal_x + (int)((float)(_x1 - min_decal_X)*zoom_factor),
					decal_y + (int)((float)(_y1 - min_decal_Y)*zoom_factor) );
}

void MyAnimationZone::myDrawRect( int _x, int _y, int _w, int _h, bool b_label_reserve )
{
	if( ! b_label_reserve )
		dc->DrawRectangle( 	decal_x + (int)((float)(_x - min_decal_X)*zoom_factor),
							decal_y + (int)((float)(_y - min_decal_Y)*zoom_factor),
							(int)((float)_w*zoom_factor),
							(int)((float)_h*zoom_factor) );
	else
	{
		myDrawLine( _x + 5, _y, _x+_w, _y );
		myDrawLine( _x, _y + 5, _x, _y+_h );
		myDrawLine( _x+_w, _y, _x+_w, _y+_h );
		myDrawLine( _x, _y+_h, _x+_w,_y+_h );
	}
}

void MyAnimationZone::myDrawCircle( int _x,int  _y, int rayon )
{
	dc->DrawCircle( 	decal_x +(int)((float)(_x - min_decal_X)*zoom_factor),
					decal_y +(int)((float)(_y - min_decal_Y)*zoom_factor),
					(int)((float)rayon * zoom_factor) );
}

void MyAnimationZone::myDrawLabel( const wxString& text, int _x,int  _y )
{
	int _w = text.Len()*5;
	int _h = 10;
	dc->SetTextBackground( wxNullColour );
	dc->SetTextForeground( dc->GetPen().GetColour() );
	dc->DrawText( text
		,decal_x + (int)((float)(_x - min_decal_X -_w/2)*zoom_factor)
		,decal_y + (int)((float)(_y - min_decal_Y -_h/3)*zoom_factor) );
}

//******************************************************

void MyAnimationZone::PaintBackground()
{
	brush.SetColour(orig_bg);
	dc->SetBrush(brush);
	pen.SetColour(orig_bg);
	dc->SetPen(pen);
	myDrawRect( min_decal_X, min_decal_Y, total_W, total_H );
}

int MyAnimationZone::GetIndFrameToPaint()
{
	return ind_active_frame;
}

//******************************************************

void MyAnimationZone::OnPaint(wxPaintEvent& event)
{
	m_flag |= M_PAINTING;
	bool b_setdcNull = false;
	if( dc == NULL )
	{
		dc = new wxAutoBufferedPaintDC( this );
		if( ! dc->IsOk() )
		{
			delete dc;
			m_flag &= ~M_PAINTING;
			return;
		}
		b_setdcNull = true;
	}

	orig_bg= GetBackgroundColour();

	// RePaint All the background SETED
	int _w,_h;
	GetSize( &_w, &_h);

	brush.SetColour(orig_bg);
	if( !brush.IsOk() )
	{
		if( b_setdcNull )
		{
			delete dc;
			dc = NULL;
		}
		return;
	}
//	dc->SetBrush(brush);
	dc->SetBackground(orig_bg);
	dc->Clear();
//	dc->SetPen(pen);
//	dc->DrawRectangle( 0, 0, _w, _h );

	// Paint the frame
	if( curr_frames != NULL && curr_anim != NULL && fr_total_decal_Xs != NULL )
    {
		ind_frPainted = GetIndFrameToPaint();
		if( ind_frPainted < 0 )
			ind_frPainted = 0;
    	if( ind_frPainted >= curr_frames_count )
    	{
    		Draw_NoImg();
    		if( b_setdcNull )
    		{
    			delete dc;
    			dc = NULL;
    		}
		m_flag &= ~M_PAINTING;
    		return;
    	}

    	// Original image size
		int original_img_w;
		int original_img_h;

		// image size after draw method application
		int img_w;
		int img_h;

		// if the frame is a flipped one (with flipframe prop)
		flipped_fr = false;

		wxImage *orig_fr_img = curr_frames[ind_frPainted]->GetImage();
		wxImage fr_img;
		if( orig_fr_img == NULL )
		{
			fr_img = *noImg;
			original_img_w = fr_img.GetWidth();
			original_img_h = fr_img.GetHeight();
		}

		//*****************************************
		// apply transformations to the image
		//*****************************************
		if( orig_fr_img != NULL )
		{
			fr_img = orig_fr_img->Copy();
			PreProcessImage( fr_img );
			original_img_w = fr_img.GetWidth();
			original_img_h = fr_img.GetHeight();

			//*****************************************
			// flipframe transformation
			if( curr_frames[ind_frPainted]->flipState > 0 )
			{
				flipped_fr = true;
				fr_img = fr_img.Mirror();
			}

			//*****************************************
			// draw image transformations
			if( 	curr_frames[ind_frPainted]->GetSubObject( wxT("nodrawmethod") ) == NULL
				&&	curr_frames[ind_frPainted]->GetSubObject( wxT("drawmethod")) != NULL )
			{
#define DGET_DMVAL2(k) \
	(curr_frames[ind_frPainted]->drawMethod[k]!=NO_CLONED_VAL?curr_frames[ind_frPainted]->drawMethod[k]:0)
				// draw image datas
				int _i = 0;
				int _scaleX = DGET_DMVAL2(_i);
				_i++;
				int _scaleY = DGET_DMVAL2(_i);
				_i++;
				bool _flipx = (DGET_DMVAL2(_i)!=0);
				_i++;
				bool _flipy = (DGET_DMVAL2(_i)!=0);
				_i++;
//				int _shiftx = curr_frames[ind_frPainted]->drawMethod[_i];
				_i++;
//				int _alpha = curr_frames[ind_frPainted]->drawMethod[_i]
				_i++;
//				int _remap = curr_frames[ind_frPainted]->drawMethod[_i]
				_i++;
//				int _fillcolor = curr_frames[ind_frPainted]->drawMethod[_i]
				_i++;
				int _rotate = DGET_DMVAL2(_i);
				_i++;
				bool _fliprotate=(DGET_DMVAL2(_i)!=0);

				// draw image FLIPS
				if( _flipx )
					fr_img = fr_img.Mirror( true );
				if( _flipy )
					fr_img = fr_img.Mirror( false );

				// draw image scaling
				if( _scaleX != 0 || _scaleY != 0)
				{
					if( _scaleX == 0 ) _scaleX = 256;
					if( _scaleY == 0 ) _scaleY = 256;

					if( _scaleX < 0 )
					{
						fr_img = fr_img.Mirror( true );
						_scaleX = - _scaleX;
					}

					if( _scaleY < 0 )
					{
						fr_img = fr_img.Mirror( false );
						_scaleY = - _scaleY;
					}

					float dm_scalex = ((float) _scaleX / (float) 256 );
					float dm_scaley = ((float) _scaleY / (float) 256 );

					fr_img.Rescale(  (int)((float) fr_img.GetWidth() * dm_scalex)
									,(int)((float) fr_img.GetHeight()* dm_scaley ));
				}

				// Rotate
				if( _rotate )
				{
					if( flipped_fr && _fliprotate )
						_rotate = - _rotate;
					fr_img.SetMaskColour( 255, 0, 255 );
					fr_img = fr_img.Rotate( (float) (-_rotate) * (float) 2 * PI / (float) 360
										, wxPoint(fr_rotate_offset_Xs[ind_frPainted],fr_rotate_offset_Ys[ind_frPainted])
										, true );
					fr_img.SetMask(false);
				}
			}
		}


		img_w = fr_img.GetWidth();
		img_h = fr_img.GetHeight();
		int real_img_w = (int)((float) img_w * zoom_factor);
		int real_img_h = (int)((float) img_h * zoom_factor );

		if( theBitmap != NULL )
			delete theBitmap;
		theBitmap = new wxBitmap( fr_img.Scale(real_img_w, real_img_h));


	// Gather Infos about the painting zone
		//Check if the anim size is lower than the client size
 		if( (int)((float)total_W*zoom_factor) < _w )
 			decal_x = (_w - (int)((float)total_W*zoom_factor)) /2;
		else
			decal_x = - GetScrollPos(wxHORIZONTAL);
		
		if( (int)((float)total_H*zoom_factor) < _h )
			decal_y = (_h - (int)((float)total_H*zoom_factor)) /2;
		else
			decal_y = - GetScrollPos(wxVERTICAL);


		// Paint the borders of the background
			brush.SetColour(*wxBLACK);
			dc->SetBrush(brush);
			pen.SetColour( *wxBLACK );
			dc->SetPen(pen);
			myDrawRect( min_decal_X - 2, min_decal_Y - 2, total_W + 4, total_H + 4 );

			m_flag &= ~M_REDRAW_ALL;

		// Paint the background
			PaintBackground();


	//******************************************
	// init Some variables
		prev_frame_rect_x = decal_x + (-min_decal_X + fr_total_decal_Xs[ind_frPainted]) * zoom_factor;
		prev_frame_rect_y = decal_y + (-min_decal_Y + fr_total_decal_Ys[ind_frPainted]) * zoom_factor;
		prev_frame_rect_w = theBitmap->GetWidth();
		prev_frame_rect_h = theBitmap->GetHeight();
		dc_x = fr_total_decal_Xs[ind_frPainted];
		dc_y = fr_total_decal_Ys[ind_frPainted];

	//******************************************
	// Draw the bitmap
	//******************************************
		if( ! draw_jump_step )
			dc->DrawBitmap( 	*theBitmap, prev_frame_rect_x, prev_frame_rect_y, true );
		else
		{
			dc->DrawBitmap( 	*theBitmap,
					prev_frame_rect_x + jump_decal_x * zoom_factor,
					prev_frame_rect_y + (-jump_decal_alt+jump_decal_z) * zoom_factor,
					true );
		}
    }
    // curr_frames == NULL
    else
    {
		Draw_NoImg();
    }

	m_flag &= ~M_PAINTING;
	if( b_setdcNull )
	{
		delete dc;
		dc = NULL;
	}
}


//******************************************************
void 
MyAnimationZone::PreProcessImage( wxImage& fr_img )
{
}


//******************************************************

void MyAnimationZone::Refresh()
{
	m_flag |= M_REDRAW_ALL;
	wxControl::Refresh();
}

//******************************************************

void MyAnimationZone::EvtSize( wxSizeEvent& event )
{
	event.Skip();
	Zoom(zoom_factor, true, false );
	Refresh();
}


//******************************************************

void MyAnimationZone::EvtGetFocus(wxFocusEvent&)
{
}


//******************************************************


void _rotate_point( float& _x, float& _y, int _angle );

void MyAnimationZone::ReverseFPCoord( int& _mx, int _mw )
{
	int ind = ind_active_frame;
	if( curr_frames[ind]->flipState > 0 )
	{
		_mx = original_img_w[ind] - _mx;
		_mx = _mx - _mw;
	}
}

//******************************************************


void MyAnimationZone::ReverseDMCoord( int& _mx, int& _my)
{
	int ind = ind_active_frame;
	if( 	curr_frames[ind]->GetSubObject( wxT("nodrawmethod") ) == NULL
		&&	curr_frames[ind]->GetSubObject( wxT("drawmethod")) != NULL )
	{
		// draw image datas
		int _i = 0;
		int _scaleX = curr_frames[ind]->drawMethod[_i];
		_i++;
		int _scaleY = curr_frames[ind]->drawMethod[_i];
		_i++;
		bool _flipx = (curr_frames[ind]->drawMethod[_i]!=0);
		_i++;
		bool _flipy = (curr_frames[ind]->drawMethod[_i]!=0);
		_i+=4;
//		int _fillcolor = curr_frames[ind]->drawMethod[_i];
		_i++;
		int _rotate = curr_frames[ind]->drawMethod[_i];
		_i++;
		bool _fliprotate = (curr_frames[ind]->drawMethod[_i]!=0);


		// DeRotate
		if( _rotate != 0 )
		{
			// Derotate current cursor vector
			float _x = -(fr_offset_Xs[ind] - _mx);
			float _y = fr_offset_Ys[ind] - _my;
			if( curr_frames[ind]->flipState > 0 && _fliprotate )
				_rotate = - _rotate;
			if( _x != 0 || _y != 0 )
				_rotate_point( _x, _y, - _rotate );

			// Rescale the "before rotation" vector to the original image size
			_mx = fr_rotate_offset_Xs[ind] + _x;
			_my = fr_rotate_offset_Ys[ind] - _y;
		}

		// reverse flips
		if( _flipx )
		{
			_mx = fr_rotate_imgw[ind] - _mx;
		}
		if( _flipy )
		{
			_my = fr_rotate_imgh[ind] - _my;
		}
		if( _scaleX < 0 )
		{
			_mx = fr_rotate_imgw[ind] - _mx;
			_scaleX = - _scaleX;
		}
		if( _scaleY < 0 )
		{
			_my = fr_rotate_imgh[ind] - _my;
			_scaleY = - _scaleY;
		}

		// Descale
		if( _scaleX != 0 )
		{
			float _factor = (float)256 / (float)_scaleX;
			_mx *= _factor;
		}
		if( _scaleY != 0 )
		{
			float _factor = (float)256 / (float)_scaleY;
			_my *= _factor;
		}
	}
}

void _rotate_point( float& _x, float& _y, int _angle )
{
	float angle = (float) (-_angle) * (float)2 * (float)PI / (float) 360;
	float mod = sqrt( _x*_x + _y*_y );
	float alpha = acos( _x / mod );
	if( _y < 0 )
		alpha = -alpha;
	_x = cos( angle + alpha ) * mod;
	_y = sin( angle + alpha ) * mod;
}


#define _MIN(x1,x2) ((x1 >x2) ? x2: x1)
#define _MAX(x1,x2) ((x1 >x2) ? x1: x2)

void getRotationNewOffset( int off_x, int off_y, int _w, int _h, int angle,
								float& new_offx, float& new_offy,float& new_w, float& new_h )
{
	float p0_x = - off_x;
	float p0_y = off_y;
	float p1_x = - off_x;
	float p1_y = off_y - _h;
	float p2_x = _w - off_x;
	float p2_y = off_y - _h;
	float p3_x = _w - off_x;
	float p3_y = off_y;
	_rotate_point( p0_x, p0_y, angle );
	_rotate_point( p1_x, p1_y, angle );
	_rotate_point( p2_x, p2_y, angle );
	_rotate_point( p3_x, p3_y, angle );
	float xmin = _MIN( _MIN( p0_x, p1_x) , _MIN(p2_x, p3_x));
	float ymin = _MIN( _MIN( p0_y, p1_y) , _MIN(p2_y, p3_y));

	float xmax = _MAX( _MAX( p0_x, p1_x) , _MAX(p2_x, p3_x));
	float ymax = _MAX( _MAX( p0_y, p1_y) , _MAX(p2_y, p3_y));

	new_w = ceil(xmax) - floor(xmin);
	new_h = ceil(ymax) - floor(ymin);
	new_offx = - xmin;
	new_offy = ymax;
}

//---------------------------------------------------------------
void
MyAnimationZone::Evt_Scroll( wxScrollWinEvent& evt )
{
	SetScrollPos( evt.GetOrientation(), evt.GetPosition() );
	RePaint();
}


//******************************************************
//******************************************************
//******************************************************


BEGIN_EVENT_TABLE(MyAnimationZone, wxControl)
	EVT_ERASE_BACKGROUND(MyAnimationZone::OnEraseBackground)
	EVT_PAINT(MyAnimationZone::OnPaint)
	EVT_MOUSE_EVENTS(MyAnimationZone::EvtMouseSomething)
	EVT_SIZE(MyAnimationZone::EvtSize)
	EVT_SET_FOCUS(MyAnimationZone::EvtGetFocus)
	EVT_SCROLLWIN(MyAnimationZone::Evt_Scroll)
END_EVENT_TABLE()


//******************************************************
//******************************************************
//******************************************************

MyAnimationCtrl::MyAnimationCtrl( wxWindow *_parent )
: MyAnimationZone( _parent ), anim_timer(this,TIMER_ANIM)
{
	b_looping = false;
	b_playing = false;
	b_sound_on = true;
	inStopPlaying = false;
	curr_played = 0;
	first_to_play = 0;
	last_to_play = 0;
}

//******************************************************

MyAnimationCtrl::~MyAnimationCtrl()
{
}

//******************************************************

void MyAnimationCtrl::GetFramesToPlay()
{
	// Get the selected frames from the list of frames
	panel_Anims->wList_frames->GetSelectedFrames( first_to_play, last_to_play );
}

//******************************************************

void MyAnimationCtrl::UpdateFrames()
{
	// No update when playing
	if( b_playing )
		return;

	MyAnimationZone::UpdateFrames();
	UpdateNavigation();
}

//******************************************************

void MyAnimationCtrl::UpdateNavigation()
{
	// Update navigation buttons
	if( curr_frames == NULL )
	{
		panel_Anims->EnableNavigation( MYALL, false );
		Refresh();
		return;
	}

	int ind = ind_active_frame;
	if( curr_frames_count == 1 )
		panel_Anims->EnableNavigation( MYALL, false );
	else if( ind >= curr_frames_count -1 )
		panel_Anims->EnableNavigation( MYRIGHT, false );
	else if( ind <= 0 )
		panel_Anims->EnableNavigation( MYLEFT, false );
	else if( ind > 0 && ind <= curr_frames_count )
		panel_Anims->EnableNavigation( MYALL, true );
}

//******************************************************

void MyAnimationCtrl::StartPlaying()
{
	if( b_playing )
	{
		curr_played = first_to_play;
		return;
	}
	else if( curr_frames_count <= 0 )
		return;

	else
	{
		panel_Anims->butt_Play->SetBitmapLabel( wxBitmap( wxImage( GetRessourceFile_String(wxT("stop.png")))));
		b_playing = true;
		panel_Anims->wList_frames->GetSelectedFrames(first_to_play, last_to_play);
		curr_played = first_to_play -1;

		// Save the current selection of the list of frames
		panel_Anims->wList_frames->GetSelection( sv_ind_active, sv_ind_first, sv_ind_last );
	}

	wxTimerEvent event;
	EvtTimer(event);
}


void MyAnimationCtrl::StopPlaying()
{
	if( inStopPlaying )
		return;
	inStopPlaying = true;

	if( b_playing )
	{
		panel_Anims->butt_Play->SetBitmapLabel( wxBitmap( wxImage( GetRessourceFile_String(wxT("aktion.png")))));

		// restore framelist selection
		panel_Anims->wList_frames->RestoreSelection( sv_ind_active, sv_ind_first, sv_ind_last );
		
		b_playing = false;

		// Reset jump vars
		is_in_jump = false;
		jump_last_delay = jump_time_counter = 0;
		draw_jump_step = false;
	}

	anim_timer.Stop();
	UpdateNavigation();
	Refresh();
	inStopPlaying = false;
}


//******************************************************

void MyAnimationCtrl::Clear()
{
	MyAnimationZone::Clear();
	b_playing = false;
	inStopPlaying = false;
}

//******************************************************

void MyAnimationCtrl::EvtMouseSomething(wxMouseEvent& event)
{
	// No such things during animation
	if( b_playing )
		return;

	MyAnimationZone::EvtMouseSomething(event);
}

//******************************************************

void MyAnimationCtrl::OnSimpleClick(wxMouseEvent& event)
{
	panel_Anims->toBBox->Disable();
	panel_Anims->toAtt->Disable();
	panel_Anims->toOffset->Enable();
	panel_Anims->toXA->Enable();
}

void MyAnimationCtrl::OnEndLeftDragging()
{
	panel_Anims->toBBox->Enable();
	panel_Anims->toAtt->Enable();
	panel_Anims->toOffset->Disable();
	panel_Anims->toXA->Disable();
}

void MyAnimationCtrl::OnAnyButtonDown()
{
	panel_Anims->toBBox->Disable();
	panel_Anims->toAtt->Disable();
	panel_Anims->toOffset->Disable();
	panel_Anims->toXA->Disable();
}

void MyAnimationCtrl::OnLeftButtonDown()
{
}

void MyAnimationCtrl::ResetMouseDraw()
{
	panel_Anims->toBBox->Disable();
	panel_Anims->toAtt->Disable();
	panel_Anims->toOffset->Disable();
	panel_Anims->toXA->Disable();
	MyAnimationZone::ResetMouseDraw();
}

void MyAnimationCtrl::EvtGetFocus(wxFocusEvent&)
{
	// repass the focus to the list of frames
	panel_Anims->wList_frames->SetFocus();
}

//******************************************************

void MyAnimationCtrl::EvtTimer(wxTimerEvent& event)
{
	draw_jump_step = false;
	if( is_in_jump )
	{
		if( curr_played < curr_frames_count &&
			fr_jump_stairs[curr_played].curr_step < fr_jump_stairs[curr_played].nb_step )
		{
			int curr_step = fr_jump_stairs[curr_played].curr_step;
			int curr_delay = JUMP_ANIM_STEP;
			curr_delay = fr_jump_stairs[curr_played].steps[curr_step].delay;
			jump_decal_alt += fr_jump_stairs[curr_played].steps[curr_step].mv_h;
			jump_decal_x += fr_jump_stairs[curr_played].mv_x * (float) curr_delay;
			jump_decal_z += fr_jump_stairs[curr_played].mv_z * (float) curr_delay;

			anim_timer.Start(10 * curr_delay );

			fr_jump_stairs[curr_played].curr_step++;

			// draw the frame
			draw_jump_step = true;

			RePaint();
			return;
		}
		else
			fr_jump_stairs[curr_played].curr_step = 0;
	}

	int k = 0;
	while( k <= curr_frames_count )
	{
		curr_played++;
		if( curr_played > last_to_play )
		{
			if( b_looping == false )
			{
				StopPlaying();
				return;
			}
			else
				curr_played = first_to_play;
		}
		if( fr_delays[curr_played] > 0 )
			break;
		k++;
	}

	if( k > curr_frames_count || curr_played < 0 || curr_played >= curr_frames_count )
	{
		StopPlaying();
		return;
	}

	// End of a jump
	if( curr_frames[curr_played]->end_jump )
		is_in_jump = false;

	// Frame of a jump
	else if( curr_frames[curr_played]->jumpState > 0 )
	{
		// Start of a jump
		if( ! is_in_jump )
		{
			is_in_jump = true;
			jump_time_counter = fr_jump_timer[curr_played];
		}

		// Reset some frame jump datas
		jump_decal_x = jump_decal_z = jump_decal_alt = 0;

		panel_Anims->wList_frames->SetSelected( curr_played, false, false );

		// Set the timer
		jump_last_delay = fr_delays[curr_played];

		// the anim delay is lower than a jump animation step
		if( jump_last_delay >= JUMP_ANIM_STEP )
			jump_last_delay = JUMP_ANIM_STEP;
		anim_timer.Start(10 * jump_last_delay);

		// Play the sound
		PlayFrameSound();

		RePaint();
		return;
	}

	// draw the frame
	PlayUpdate();
}

//******************************************************

void MyAnimationCtrl::PlayUpdate()
{
	if( curr_frames == NULL )
		return;

	// Set the curr_played frame
	panel_Anims->wList_frames->SetSelected( curr_played, false, false );

	// Set the timer
	int delay = fr_delays[curr_played];
	if( delay < 2 ) delay = 2;
	anim_timer.Start(delay*10);

	// draw the frame
	RePaint();

	// Play the sound
	PlayFrameSound();
}


//******************************************************

void MyAnimationCtrl::PlayFrameSound()
{
	ob_frame *frame_to_show = curr_frames[curr_played];
	if( frame_to_show != NULL && b_sound_on)
	{
		wxFileName file_snd = GetObFile(frame_to_show->GetSubObject_Token(wxT("sound")));
		if( file_snd.FileExists() )
		{
			PlaySound( (char*) file_snd.GetFullPath().c_str() );
		}
	}
}


//******************************************************

void MyAnimationCtrl::PaintBackground()
{
	if( ! b_playing )
	{
		brush.SetColour(orig_bg);
		pen.SetColour(orig_bg);
	}
	else
	{
		brush.SetColour(ob_pink);
		pen.SetColour(ob_pink);
	}
	dc->SetBrush(brush);
	dc->SetPen(pen);
	myDrawRect( min_decal_X, min_decal_Y, total_W, total_H );
}

//******************************************************

void MyAnimationCtrl::OnPaint(wxPaintEvent& event)
{
	dc = new wxAutoBufferedPaintDC( this );
	MyAnimationZone::OnPaint( event );

	m_flag |= M_PAINTING;
	if( curr_frames != NULL && curr_anim != NULL && fr_total_decal_Xs != NULL )
    {
		//******************************************
		// Draw the ground lines
		//******************************************

		// The original ground line
		pen.SetColour( wxColour(0,0,0));
		pen.SetWidth( 2 );
		dc->SetPen( pen );
		int _firstframe_ground_y = fr_total_decal_Ys[0] + fr_offset_Ys[0];
		myDrawLine( 	min_decal_X, _firstframe_ground_y, total_W, _firstframe_ground_y );

		// The current ground line
		pen.SetColour( wxColour(255,255,0));
		pen.SetWidth( 2 );
		dc->SetPen( pen );
		int curr_ground_y = dc_y + fr_offset_Ys[ind_frPainted] - fr_As[ind_frPainted];
		if( curr_ground_y != _firstframe_ground_y )
			myDrawLine( 	min_decal_X,curr_ground_y , total_W, curr_ground_y );


		//******************************************
		// In case of not playing the animation, draw the beat-box and all the stuffs
		//******************************************


		if( !b_playing )
		{
			// Draw the Offset cross
			pen.SetColour( wxColour(0,0,0));
			pen.SetWidth( 3 );
			dc->SetPen( pen );
			int cross_w = 7;
			myDrawLine( 	dc_x + fr_offset_Xs[ind_frPainted] - cross_w, dc_y + fr_offset_Ys[ind_frPainted],
						dc_x + fr_offset_Xs[ind_frPainted] + cross_w, dc_y + fr_offset_Ys[ind_frPainted] );
			myDrawLine( 	dc_x + fr_offset_Xs[ind_frPainted], dc_y + fr_offset_Ys[ind_frPainted] - cross_w,
						dc_x + fr_offset_Xs[ind_frPainted], dc_y + fr_offset_Ys[ind_frPainted] + cross_w );

			// Draw the Altitude
			if( fr_As[ind_frPainted] < 0 )
			{
				pen.SetColour( wxColour(255,255,0));
				pen.SetWidth( 3 );
				dc->SetPen( pen );
				myDrawLine(  dc_x+fr_offset_Xs[ind_frPainted],   dc_y+fr_offset_Ys[ind_frPainted],
								dc_x+fr_offset_Xs[ind_frPainted],   dc_y+fr_offset_Ys[ind_frPainted] - fr_As[ind_frPainted] );
				myDrawLine(  dc_x+fr_offset_Xs[ind_frPainted]-3, dc_y+fr_offset_Ys[ind_frPainted] + 3,
								dc_x+fr_offset_Xs[ind_frPainted],   dc_y+fr_offset_Ys[ind_frPainted] );
				myDrawLine(  dc_x+fr_offset_Xs[ind_frPainted]+3, dc_y+fr_offset_Ys[ind_frPainted] + 3,
								dc_x+fr_offset_Xs[ind_frPainted],   dc_y+fr_offset_Ys[ind_frPainted] );
				myDrawLine(  dc_x+fr_offset_Xs[ind_frPainted]-2, dc_y+fr_offset_Ys[ind_frPainted] - fr_As[ind_frPainted],
								dc_x+fr_offset_Xs[ind_frPainted]+2, dc_y+fr_offset_Ys[ind_frPainted] - fr_As[ind_frPainted] );
			}

			// Draw the beat-box
			int bbox_x = curr_frames[ind_frPainted]->bBox[0];
			int bbox_y = curr_frames[ind_frPainted]->bBox[1];
			int bbox_w = curr_frames[ind_frPainted]->bBox[2];
			int bbox_h = curr_frames[ind_frPainted]->bBox[3];

			if( bbox_x != 0 || bbox_y != 0 || bbox_w != 0 || bbox_h != 0 )
			{
				if( flipped_fr )
				{
					bbox_x = original_img_w[ind_frPainted] - bbox_x - bbox_w;
				}
				pen.SetColour( panel_Anims->bbox_color->GetColor() );
				pen.SetWidth( 2 );
				dc->SetPen( pen );
				dc->SetBrush(*wxTRANSPARENT_BRUSH);
				myDrawRect(     dc_x+ bbox_x - fr_DM_decal_Xs[ind_frPainted],
								dc_y+ bbox_y - fr_DM_decal_Ys[ind_frPainted],
								bbox_w, bbox_h, true );
				myDrawLabel(   wxT("BB"), dc_x+bbox_x-fr_DM_decal_Xs[ind_frPainted], dc_y+bbox_y-fr_DM_decal_Ys[ind_frPainted]);
			}

			// Draw the attack-boxes
			pen.SetWidth( 2 );
			pen.SetColour( panel_Anims->att_color->GetColor() );
			dc->SetPen( pen );

			wxString att_name = curr_frames[ind_frPainted]->attName;
			int att_x = curr_frames[ind_frPainted]->attBox[0];
			int att_y = curr_frames[ind_frPainted]->attBox[1];
			int att_w = curr_frames[ind_frPainted]->attBox[2];
			int att_h = curr_frames[ind_frPainted]->attBox[3];

			if( att_x != 0 || att_y != 0 || att_w != 0 || att_h != 0 )
			{
				if( flipped_fr )
				{
					att_x = original_img_w[ind_frPainted] - att_x - att_w;
				}
				dc->SetBrush(*wxTRANSPARENT_BRUSH);
				myDrawRect(    dc_x + att_x - fr_DM_decal_Xs[ind_frPainted]
								, dc_y + att_y - fr_DM_decal_Ys[ind_frPainted]
								, att_w
								, att_h, true );
				if( att_name.Left(6) == wxT("attack") )
					att_name = wxT("att") + att_name.Right(att_name.Len()-6);

				myDrawLabel(  	att_name,
								dc_x + att_x - fr_DM_decal_Xs[ind_frPainted],
								dc_y + att_y - fr_DM_decal_Ys[ind_frPainted]
							);
			}


			// Paint the mouse draw
			pen.SetColour( wxColour(0,255,0));
			dc->SetPen( pen );
				dc->SetBrush(*wxTRANSPARENT_BRUSH);
			if( mouse_draw == DRAW_AIM )
			{
				myDrawCircle(  dc_x+r_x, dc_y+r_y, 5 );
				myDrawLine( dc_x+r_x -2, dc_y+r_y, dc_x+r_x +2, dc_y+r_y );
				myDrawLine( dc_x+r_x , dc_y+r_y-2, dc_x+r_x , dc_y+r_y+2 );
			}
			else if( mouse_draw == DRAW_RECT )
			{
				myDrawRect(  dc_x+r_x, dc_y+r_y, r_w, r_h );
			}
		}
    }
	delete dc;
	dc = NULL;
	m_flag &= ~M_PAINTING;
}


//******************************************************

int MyAnimationCtrl::GetIndFrameToPaint()
{
	if( ! b_playing )
		 return ind_active_frame;
	else
		return curr_played;
}


//******************************************************
//******************************************************
//******************************************************
//******************************************************
BEGIN_EVENT_TABLE(MyAnimationCtrl, MyAnimationZone)
    EVT_PAINT(MyAnimationCtrl::OnPaint)
	EVT_SET_FOCUS(MyAnimationCtrl::EvtGetFocus)
	EVT_TIMER(TIMER_ANIM, MyAnimationCtrl::EvtTimer)
END_EVENT_TABLE()



//******************************************************
//******************************************************
//******************************************************
//******************************************************

MyRemapViewerCtrl::MyRemapViewerCtrl(wxWindow *_parent)
:MyAnimationZone( _parent )
{
#ifdef OSLINUX
wxImage img_pip_plus( GetRessourceFile( wxT("cursor_pipette_plus.png") ).GetFullPath() );
wxImage img_pip_simple( wxImage( GetRessourceFile( wxT("cursor_pipette_simple.png") ).GetFullPath() ) );
#endif
#ifdef OSWINDOW
wxImage img_pip_plus( GetRessourceFile( wxT("cursor_pipette_plus-win.png") ).GetFullPath() );
wxImage img_pip_simple( wxImage( GetRessourceFile( wxT("cursor_pipette_simple-win.png") ).GetFullPath() ) );
#endif
	img_pip_plus.SetMask();
	img_pip_simple.SetMask();

	cursor_pipette_plus = wxCursor( img_pip_plus );
	cursor_pipette_simple= wxCursor( img_pip_simple );

	mouse_icon_state = 0;
}


//******************************************************

MyRemapViewerCtrl::~MyRemapViewerCtrl()
{
	if( dc != NULL )
		delete dc;
}


//******************************************************

void MyRemapViewerCtrl::PreProcessImage( wxImage& fr_img )
{
	if( panel_Remaps->paletteCtrl == NULL || ! panel_Remaps->paletteCtrl->b_init )
		return;

	int palette_count = panel_Remaps->paletteCtrl->thePalette->GetColoursCount();
	if( palette_count <= 0 )
		return;

	unsigned char o_r[palette_count], o_g[palette_count], o_b[palette_count];
	unsigned char n_r[palette_count], n_g[palette_count], n_b[palette_count];

	if( ! panel_Remaps->paletteCtrl->mode8bit )
	{
		for( int i = 0; i < palette_count; i++)
		{
			// Get the original colour
			panel_Remaps->paletteCtrl->thePalette->GetRGB( i, &o_r[i], &o_g[i], &o_b[i] );

			// Get the mapped colour
			panel_Remaps->paletteCtrl->paletteElements[i]->GetRGB( &n_r[i], &n_g[i], &n_b[i] );

		}
	}
	else
	{
		for( int i = 0; i < palette_count; i++)
		{
			// Get the original colour
			panel_Remaps->paletteCtrl->thePalette->GetRGB( i, &o_r[i], &o_g[i], &o_b[i] );

			// Get the mapped colour
			if( panel_Remaps->paletteCtrl->paletteElements[i]->mappedTo >= 0 )
			{
				int ind_remap = panel_Remaps->paletteCtrl->paletteElements[i]->mappedTo;
				panel_Remaps->paletteCtrl->paletteElements[ind_remap]->GetRGB( &n_r[i], &n_g[i], &n_b[i] );
			}
			else
				panel_Remaps->paletteCtrl->paletteElements[i]->GetRGB( &n_r[i], &n_g[i], &n_b[i] );

		}
	}

	// Do the mapping
	int nb_pixs = fr_img.GetWidth() * fr_img.GetHeight();
	unsigned char *_pixs = fr_img.GetData( );

	for( int i = 0; i < nb_pixs; i++ )
	{
		unsigned char _r = _pixs[i*3];
		unsigned char _g = _pixs[i*3+1];
		unsigned char _b = _pixs[i*3+2];

		for (int j=0; j < palette_count; j++ )
		{
			// No map for this colour
			if( n_r[j] == o_r[j] && n_g[j] == o_g[j] && n_b[j] == o_b[j] )
				continue;

			// Mapping found
			if( _r == o_r[j] && _g == o_g[j] && _b == o_b[j] )
			{
				_pixs[i*3]   = n_r[j];
				_pixs[i*3+1] = n_g[j];
				_pixs[i*3+2] = n_b[j];
				break;
			}
		}
	}
}


//******************************************************

void MyRemapViewerCtrl::OnSimpleClick(wxMouseEvent& event)
{
	if( panel_Remaps->paletteCtrl == NULL || curr_frames == NULL || ind_active_frame < 0 )
		return;

	if( curr_frames[ind_active_frame]->GetImage() == noImg )
		return;

	//Get the index of the color under the mouse
	int ind = curr_frames[ind_active_frame]->Get_ColorIndex(TO_offx, TO_offy );

	if( ind < 0 )
		return;

	int add_mode = (event.m_controlDown?1:0)+(event.m_shiftDown?2:0);
	panel_Remaps->paletteCtrl->SelectColour( ind , add_mode);
}


//******************************************************

void MyRemapViewerCtrl::OnPaint(wxPaintEvent& event)
{
	dc = new wxAutoBufferedPaintDC( this );
	MyAnimationZone::OnPaint( event );
	delete dc;
	dc = NULL;
}

//******************************************************
void 
MyRemapViewerCtrl::EvtMouseMove(wxMouseEvent& event)
{
	MyAnimationZone::EvtMouseSomething( event );

	if( event.ControlDown() && mouse_icon_state <= 1 )
	{
		// Set the mouse icon with a pipette plus
		wxSetCursor( cursor_pipette_plus );
	}
	else if( mouse_icon_state != 1 )
	{
		// Set the mouse icon with a simple pipette
		wxSetCursor( cursor_pipette_simple );
	}
	event.Skip();
}


//******************************************************

void MyRemapViewerCtrl::EvtMouseEnter(wxMouseEvent& event)
{
	mouse_icon_state = 0;
	// Set the cursor
	EvtMouseMove(event);
}


//******************************************************

void MyRemapViewerCtrl::EvtMouseLeave(wxMouseEvent& event)
{
	// Restore the previous cursor
	wxSetCursor( wxNullCursor );
}


//******************************************************
//******************************************************
//******************************************************
BEGIN_EVENT_TABLE(MyRemapViewerCtrl, MyAnimationZone)
	EVT_PAINT(MyRemapViewerCtrl::OnPaint)
	EVT_MOTION(MyRemapViewerCtrl::EvtMouseMove)
	EVT_ENTER_WINDOW(MyRemapViewerCtrl::EvtMouseEnter)
	EVT_LEAVE_WINDOW(MyRemapViewerCtrl::EvtMouseLeave)
END_EVENT_TABLE()


//******************************************************
//******************************************************
//******************************************************
//******************************************************

MyPlatformViewerCtrl::MyPlatformViewerCtrl(wxWindow *_parent)
:MyAnimationZone( _parent )
{
#ifdef OSLINUX
wxImage img_move( GetRessourceFile( wxT("cursor_move.png") ).GetFullPath() );
#endif
#ifdef OSWINDOW
wxImage img_move( GetRessourceFile( wxT("cursor_move-win.png") ).GetFullPath() );
#endif

	img_move.SetMaskColour( 130, 130, 130);
	img_move.SetMask();

	cursor_move = wxCursor( img_move );
	elt_dragged = PLAT_NONE;
}


//******************************************************

MyPlatformViewerCtrl::~MyPlatformViewerCtrl()
{
}


//************************************************************************************

void MyPlatformViewerCtrl::OnLeftButtonDown()
{
	//Check if it's near a draggable point
	int poff_x = 	StrToInt( panel_Platform->txtctrl_off_x->GetValue() );
	int poff_y = 	StrToInt( panel_Platform->txtctrl_off_y->GetValue() );
	int plr = 		StrToInt( panel_Platform->txtctrl_l_r->GetValue() );
	int pdl = 		StrToInt( panel_Platform->txtctrl_d_l->GetValue() );
	int pupr = 		StrToInt( panel_Platform->txtctrl_up_r->GetValue() );
	int pupl = 		StrToInt( panel_Platform->txtctrl_up_l->GetValue() );
	int pdepth = 	StrToInt( panel_Platform->txtctrl_depth->GetValue() );
	int palt = 		StrToInt( panel_Platform->txtctrl_alt->GetValue() );

	wxPoint d(  poff_x, poff_y );
	wxPoint dl(  d.x + pdl  , d.y );
	wxPoint dr(  d.x + plr  , d.y );
	wxPoint u(   d.x        , d.y - palt);
	wxPoint ul(  u.x +pupl  , u.y );
	wxPoint ur(  u.x +pupr   , u.y );
	wxPoint depth_decal( pdepth * 0.5, pdepth * 0.866 );
	wxPoint dr_d(  dr.x + depth_decal.x  , dr.y - depth_decal.y);


	int max_dist = 3;

	if( abs(ul.x - r_x ) <= max_dist && abs( ul.y - r_y ) <= max_dist  )
	{
		r_x = StrToInt( panel_Platform->txtctrl_off_x->GetValue() );
		elt_dragged = PLAT_LU;
	}
	else if( abs(ur.x - r_x ) <= max_dist && abs( ur.y - r_y ) <= max_dist  )
	{
		r_x = StrToInt( panel_Platform->txtctrl_off_x->GetValue() );
		elt_dragged = PLAT_RU;
	}
	else if( abs(u.x - r_x ) <= max_dist && abs( u.y - r_y ) <= max_dist  )
	{
		r_y = StrToInt( panel_Platform->txtctrl_off_y->GetValue() );
		elt_dragged = PLAT_U;
	}
	else if( abs(dr_d.x - r_x ) <= max_dist && abs( dr_d.y - r_y ) <= max_dist  )
	{
		r_x = StrToInt( panel_Platform->txtctrl_l_r->GetValue() ) + StrToInt( panel_Platform->txtctrl_off_x->GetValue() );
		elt_dragged = PLAT_DEPTH;
	}
	else if( abs(dl.x - r_x ) <= max_dist && abs( dl.y - r_y ) <= max_dist  )
	{
		r_x = StrToInt( panel_Platform->txtctrl_off_x->GetValue() );
		elt_dragged = PLAT_LD;
	}
	else if( abs(dr.x - r_x ) <= max_dist && abs( dr.y - r_y ) <= max_dist  )
	{
		r_x = StrToInt( panel_Platform->txtctrl_off_x->GetValue() );
		elt_dragged = PLAT_RD;
	}
	else if( abs(d.x - r_x ) <= max_dist && abs(d.y - r_y ) <= max_dist  )
		elt_dragged = PLAT_D;

	else
		elt_dragged = PLAT_NONE;

	if( elt_dragged != PLAT_NONE )
	{
		SetCursor( cursor_move );
	}
	else
		SetCursor( wxNullCursor );
}


//************************************************************************************

void MyPlatformViewerCtrl::OnEndLeftDragging()
{
	elt_dragged = PLAT_NONE;
	SetCursor( wxNullCursor );

	// Make the save into the ob_object
	panel_Platform->Update_entity_platform();
}


//************************************************************************************

void MyPlatformViewerCtrl::OnDragging()
{
	if( elt_dragged == PLAT_NONE )
	{
		SetCursor( wxNullCursor );
		return;
	}

	switch( elt_dragged )
	{
		case PLAT_LD:
		{
			panel_Platform->txtctrl_d_l->SetValue(IntToStr( r_w));
			break;
		}
		case PLAT_RD:
		{
			panel_Platform->txtctrl_l_r->SetValue(IntToStr(r_w));
			break;
		}
		case PLAT_LU:
		{
			panel_Platform->txtctrl_up_l->SetValue(IntToStr(r_w));
			break;
		}
		case PLAT_RU:
		{
			panel_Platform->txtctrl_up_r->SetValue(IntToStr(r_w));
			break;
		}
		case PLAT_DEPTH:
		{
			panel_Platform->txtctrl_depth->SetValue(IntToStr((int)(r_w * 2)));
			break;
		}
		case PLAT_U:
		{
			panel_Platform->txtctrl_alt->SetValue(IntToStr(-r_h));
			break;
		}
		case PLAT_D:
		{
			panel_Platform->txtctrl_off_x->SetValue(IntToStr(r_x+r_w));
			panel_Platform->txtctrl_off_y->SetValue(IntToStr(r_y+r_h));
			break;
		}
	}
	Refresh();
}


//************************************************************************************

void MyPlatformViewerCtrl::OnSimpleClick(wxMouseEvent& event)
{
	elt_dragged = PLAT_NONE;
	SetCursor( wxNullCursor );
}

//************************************************************************************

void MyPlatformViewerCtrl::OnPaint(wxPaintEvent& event)
{
	dc = new wxAutoBufferedPaintDC( this );
	MyAnimationZone::OnPaint( event );

	// If a platform is setted
	if( panel_Platform->chckbx_noplatform->GetValue() == false )
	{
		brush.SetColour(panel_Platform->plat_color->GetColor() );
		dc->SetBrush(brush);
		pen.SetColour(panel_Platform->plat_color->GetColor());
		dc->SetPen(pen);

		int poff_x = 	StrToInt( panel_Platform->txtctrl_off_x->GetValue() );
		int poff_y = 	StrToInt( panel_Platform->txtctrl_off_y->GetValue() );
		int plr = 		StrToInt( panel_Platform->txtctrl_l_r->GetValue() );
		int pdl = 		StrToInt( panel_Platform->txtctrl_d_l->GetValue() );
		int pupr = 		StrToInt( panel_Platform->txtctrl_up_r->GetValue() );
		int pupl = 		StrToInt( panel_Platform->txtctrl_up_l->GetValue() );
		int pdepth = 	StrToInt( panel_Platform->txtctrl_depth->GetValue() );
		int palt = 		StrToInt( panel_Platform->txtctrl_alt->GetValue() );

		wxPoint d( dc_x + poff_x, dc_y + poff_y );
		wxPoint dl(  d.x + pdl  , d.y );
		wxPoint dr(  d.x + plr  , d.y );
		wxPoint u(   d.x        , d.y - palt);
		wxPoint ul(  u.x +pupl  , u.y );
		wxPoint ur(  u.x +pupr   , u.y );
		wxPoint depth_decal( pdepth * 0.5, pdepth * 0.866 );
		wxPoint ul_d(  ul.x + depth_decal.x  , ul.y - depth_decal.y);
		wxPoint ur_d(  ur.x + depth_decal.x  , ur.y - depth_decal.y);
		wxPoint dr_d(  dr.x + depth_decal.x  , dr.y - depth_decal.y);

		// First draw the Draggable Points
		int point_rayon = 2;
		myDrawCircle( dl.x,dl.y, point_rayon );
		myDrawCircle( d.x,d.y, point_rayon );
		myDrawCircle( dr.x,dr.y, point_rayon );
		myDrawCircle( ul.x,ul.y, point_rayon );
		myDrawCircle( u.x,u.y, point_rayon );
		myDrawCircle( ur.x,ur.y, point_rayon );
		myDrawCircle( dr_d.x,dr_d.y, point_rayon );

		// Now draw all these awfull lines
		myDrawLine( dl.x ,	dl.y, d.x , d.y	);
		myDrawLine( d.x ,	dl.y, dr.x , d.y);
		myDrawLine( ul.x ,	ul.y, u.x , u.y	);
		myDrawLine( u.x ,	u.y	, ur.x , ur.y );
		myDrawLine( dl.x,	dl.y , ul.x, ul.y );
		myDrawLine( dr.x,	dr.y , ur.x, ur.y );
		myDrawLine( ul.x,	ul.y , ul_d.x, ul_d.y );
		myDrawLine( ur.x,	ur.y , ur_d.x, ur_d.y );
		myDrawLine( dr.x,	dr.y , dr_d.x, dr_d.y );
		myDrawLine( ul_d.x,	ul_d.y , ur_d.x, ur_d.y );
		myDrawLine( ur_d.x,	ur_d.y , dr_d.x, dr_d.y );

	}

	delete dc;
}


//******************************************************
//******************************************************
//******************************************************
BEGIN_EVENT_TABLE(MyPlatformViewerCtrl, MyAnimationZone)
	EVT_PAINT(MyPlatformViewerCtrl::OnPaint)
END_EVENT_TABLE()


