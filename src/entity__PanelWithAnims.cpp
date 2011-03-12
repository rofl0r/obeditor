/*
 * Panel_withAnims.cpp
 *
 *  Created on: 8 nov. 2008
 *      Author: pat
 */
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "ob_editor.h"
#include "entity__globals.h"
#include "entity__enums.h"

//************************************************************************************

int Panel_withAnims::entityAnimation_time = -1;

//************************************************************************************

Panel_withAnims::Panel_withAnims( wxNotebook *onglets )
:wxPanel( onglets, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
	wList_frames = NULL;
	list_Anims = NULL;
	this_entityAnimation_time = -1;
	b_entity_has_change = false;
	m_flag = 0;
}


//************************************************************************************

Panel_withAnims::~Panel_withAnims()
{
}


//************************************************************************************

bool Is_AAirAnim()
{
	if( curr_anim == NULL )
		return false;

	wxString anim_name = curr_anim->GetToken(0);
	if( anim_name.Left(4).Upper() == wxT("FALL") || anim_name.Left(4).Upper() == wxT("JUMP") )
	{
		if(		anim_name.Len() == 4
			||	StrIsInt( anim_name.Right( anim_name.Len() - 4 )
				)
		)
		return true;
	}

	return false;
}


//************************************************************************************

void Panel_withAnims::Frame_CascadeChanges()
{
	if( curr_frames_count == 0 )
		return;
	
	// Check if it's a fall frame
	bool b_jumpState = false;
	if( curr_anim == NULL )
		return;
	
	//-----------------------------------------------------------
	// Update the jumpStates of frames
	ob_object* ob_state = curr_anim->GetSubObject(wxT("jumpframe"));
	if( 	   Is_AAirAnim()
		&& curr_frames_count > 0
		&& ob_state == NULL
	  )
	{
		b_jumpState = true;
		for( int i = 0; i < curr_frames_count; i++ )
			curr_frames[i]->jumpState = 1;
	}
	else if( ob_state != NULL )
		wList_frames->SetJumpFrame( StrToInt( ob_state->GetToken( 0 )));
	else
		wList_frames->SetJumpFrame( -1 );

	//-----------------------------------------------------------
	// Update the dropStates
	ob_state = curr_anim->GetSubObject(wxT("dropframe"));
	if( ob_state != NULL )
		wList_frames->SetDropFrame( StrToInt( ob_state->GetToken( 0 )));
	else
		wList_frames->SetDropFrame( -1 );
	
	//-----------------------------------------------------------
	// Update the landState
	ob_state = curr_anim->GetSubObject(wxT("landframe"));
	if( ob_state != NULL )
		wList_frames->SetLandFrame( StrToInt( ob_state->GetToken( 0 )));
	else
		wList_frames->SetLandFrame( -1 );
	
	//-----------------------------------------------------------
	// Update the flipState
	ob_state = curr_anim->GetSubObject(wxT("flipframe"));
	if( ob_state != NULL )
		wList_frames->SetFlipFrame( StrToInt( ob_state->GetToken( 0 )));
	else
		wList_frames->SetFlipFrame( -1 );
	
	// and clone all other datas
	curr_frames[0]->UpdateClonedDatas(NULL);
}


//************************************************************************************

void Panel_withAnims::EvtClose( wxCloseEvent& )
{
}


//************************************************************************************

void Panel_withAnims::EvtSize( wxSizeEvent& event )
{
	event.Skip();
}


//************************************************************************************

void Panel_withAnims::EvtAnimListChange(wxCommandEvent& event)
{
	entityFrame->Reload_Anims();
	Frame_CascadeChanges();
}



//************************************************************************************

void Panel_withAnims::EvtAnimSelectionChange(wxCommandEvent& event)
{
	m_flag |= M_CHANGING_ANIM;
	
	// Register the history
	entityFrame->map_ent_animHistory[entity->Name()] = 
			list_Anims->GetStringSelection();
	
	// Impact some consequences
	if( wList_frames != NULL )
	{
		int sv_ind_active, sv_ind_first, sv_ind_last;
		wList_frames->GetSelection( sv_ind_active, sv_ind_first, sv_ind_last );
		wList_frames->ReloadFrames();
		wList_frames->SetSelected(sv_ind_active);
		Frame_CascadeChanges();
	}
	m_flag &= ~M_CHANGING_ANIM;
}


//************************************************************************************

void Panel_withAnims::EvtFramesListChange(wxCommandEvent& event)
{
	entityFrame->Reload_Frames();
	Frame_CascadeChanges();
}


//************************************************************************************

void Panel_withAnims::EvtFrameSelectionChange(wxCommandEvent& event)
{
}


//************************************************************************************
void 
Panel_withAnims::OnActivate()
{
	m_flag |= M_ON_ACTIVATE;
	// A entity change have been made
	if( entityAnimation_time < entityFrame->entitySelection_time )
	{
		Refresh();
		wList_frames->SetSelected(0);
		wList_frames->UpdateScrollBar();
		
		// Check if there is some last anim for this entity
		if( 	   entityFrame->map_ent_animHistory.find( entity->Name() ) 
			!= 
			   entityFrame->map_ent_animHistory.end()
		   )
		{
			wxString select_anim = entityFrame->map_ent_animHistory[entity->Name()];
			list_Anims->SetStringSelection( select_anim );
			wxCommandEvent dummy;
			list_Anims->OnSelectionChange( dummy );
		}
		
		entityAnimation_time = entityFrame->entitySelection_time;
	}

	// No entity change, so just refresh current anims and frames selections
	else
	{
		list_Anims->Refresh_List(false);
		wList_frames->ReloadFrames();
		wList_frames->RefreshSelection();
		wList_frames->UpdateScrollBar();
	}

	b_entity_has_change = (this_entityAnimation_time != entityAnimation_time );
	this_entityAnimation_time = entityAnimation_time;
	m_flag &= ~M_ON_ACTIVATE;
}


//*********************************************************************

void Panel_withAnims::Refresh()
{
	list_Anims->Refresh_List();
	wList_frames->ReloadFrames();
}


//*********************************************************************

void Panel_withAnims::Reload()
{
	if( entity == NULL )
		return;
	entity->PurgeImages();
	list_Anims->Refresh_List();
	wList_frames->ReloadFrames();
}


//************************************************************************************

void Panel_withAnims::NoFocus(wxFocusEvent& event)
{
	if( wList_frames != NULL )
		wList_frames->SetFocus();
}


//************************************************************************************

void Panel_withAnims::ThrowFocus(wxCommandEvent& event)
{
	if( wList_frames != NULL )
		wList_frames->SetFocus();
}


//************************************************************************************

void Panel_withAnims::EvtGetFocus(wxFocusEvent& event)
{
	list_Anims->SetFocus();
}


//************************************************************************************

bool Panel_withAnims::EntityChanged()
{
	return entity->changed;
}



//************************************************************************************

void Panel_withAnims::ReloadGifs()
{
	if( wList_frames != NULL )
	{
		wList_frames->ReloadFrames(true);
		wList_frames->RefreshSelection();
	}
}




//************************************************************************************
//************************************************************************************
//************************************************************************************
//************************************************************************************
// Event table
BEGIN_EVENT_TABLE(Panel_withAnims, wxPanel)

	EVT_ANIM_LIST_CHANGE(Panel_withAnims::EvtAnimListChange)
	EVT_ANIM_SELECTED_CHANGE(Panel_withAnims::EvtAnimSelectionChange)
	EVT_FRAME_LIST_CHANGE(Panel_withAnims::EvtFramesListChange)
	EVT_FRAME_SELECTED_CHANGE(Panel_withAnims::EvtFrameSelectionChange)

	EVT_SET_FOCUS(Panel_withAnims::EvtGetFocus)
	EVT_CLOSE(Panel_withAnims::EvtClose)

END_EVENT_TABLE()

