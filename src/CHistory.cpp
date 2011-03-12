#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__tools.h"
#include "common__ob_object.h"
#include "CHistory.h"

// #define DEBUG_UNDOS

using namespace std;

//--------------------------------------------------------
History_Manager theHistoryManager;
int MAX_MERGE_DELAY = 50; // in ms of course
int MAX_HAND_EDITION_DETECTION_DELAY = 1000; // in ms of course
//--------------------------------------------------------

/**
 ***********************************************************
 *  HISTORY ELT 
 ***********************************************************
 */
size_t debug_hist_elt_count = 0;
void
History_Elt::Zero_Init()
{
	theDate = wxGetLocalTimeMillis();
	is_past = true;
	type  = H_DUMMY;
	datas = NULL;
	o = NULL;
	o_related = NULL;
	token_pos = 0;
	token_val = wxString();
	datas = NULL;
//	cout<<"History_Elt() : NEW ELT !"<<endl;
}

//--------------------------------------------------------
History_Elt::History_Elt()
{
	Zero_Init();
}

//--------------------------------------------------------
// For OBJ_INSERT , OBJ_RM
History_Elt::History_Elt( int _type, ob_object* _o, ob_object* _o_related, bool b_parent )
{
	if( _o_related == NULL )
		wxMessageBox( wxT("BUG!!!!!\n\nHistory_Elt::History_Elt()\n_o_related == NULL !!!\n") );

	debug_hist_elt_count++;
	Zero_Init();
	
	// Gotta keep trace of which object are orphans
	switch( _type )
	{
		case H_OBJ_INSERT:
			type = _type;
			break;
		case H_OBJ_RM:
			type = _type;
			break;
			
		default:
			wxMessageBox( wxT("BUG!!!!!\nHistory_Elt::History_Elt()\nBad type for subobect change !!!\n") );
			return;
	}
	
	o = _o;
	o_related = _o_related;
	token_pos = (b_parent?1:0);
}

//--------------------------------------------------------
// For TOKEN_CHANGE, TOKEN_ADD, TOKEN_RM
History_Elt::History_Elt(  int _type, ob_object* _o
		, const wxString& _token_val, int _token_pos )
{
	debug_hist_elt_count++;
	Zero_Init();
	switch( _type )
	{
		case H_NAME_CHANGE:
		case H_TOKEN_CHANGE:
		case H_TOKEN_ADD:
		case H_TOKEN_RM:
			type = _type;
			token_pos = _token_pos;
			token_val = _token_val;
			break;

		default:
			wxMessageBox( wxT("BUG!!!!!\n\nHistory_Elt::History_Elt()\nBad type for token change !!!\n") );
			return;
	}

	o = _o;
}

//--------------------------------------------------------
void
History_Elt::MayDeleteO()
{
	if( o == NULL || theHistoryManager.l_orphans.count( o ) == 0 )
		return;
#ifdef DEBUG_UNDOS
	cout<<"\tOBJ DELETED : "<<o->ToStr()<<endl;
#endif
	theHistoryManager.l_orphans.erase(o);
	delete o;
	o = NULL;
}

//--------------------------------------------------------
History_Elt::~History_Elt()
{
	debug_hist_elt_count--;
#ifdef DEBUG_UNDOS
	if(    theHistoryManager.IsEnabled() == true 
	    && (theHistoryManager.m_flag & History_Manager::M_IN_DO_OR_UNDO) == 0
	    && o != NULL
	  )
	{
		cout<<"~History_Elt() : history elt count : "<<debug_hist_elt_count<<endl;
		cout<<"\tDeleted : "<<this<<endl;
	}
#endif
	if(theHistoryManager.onDeleteHistory_data != NULL && this->datas != NULL )
		(*theHistoryManager.onDeleteHistory_data)(this->datas);

	if( type == H_TOKEN_RM && is_past == true )
		MayDeleteO();

	if( type == H_TOKEN_ADD && is_past == false )
		MayDeleteO();

	if( type == H_OBJ_INSERT && is_past == false )
		MayDeleteO();

	if( type == H_OBJ_RM  && is_past == false )
		MayDeleteO();
}

//--------------------------------------------------------
// Return true if do or undo process must CONTINUE
wxString
History_Elt::ToStr()
{
	wxString res = IntToStr((size_t)this->o) + wxT(" : ");
	switch( type )
	{
		case H_DUMMY:
			res = wxT("Dummy : ");
			break;
		case H_NAME_CHANGE:
			res = wxT("NAME_CHG : ");
			break;
		case H_TOKEN_CHANGE:
			res = wxT("TOK_CHG : ");
			break;
		case H_TOKEN_ADD:
			res = wxT("TOK_ADD : ");
			break;
		case H_TOKEN_RM:
			res = wxT("TOK_RM : ");
			break;
		case H_OBJ_INSERT:
			res = wxT("OBJ_INSERT : ");
			break;
		case H_OBJ_RM:
			res = wxT("OBJ_RM : ");
			break;
		default:
			res = wxT("BUG_type : ");
			break;
	}
	if( o==NULL)
		res += wxT("NULL");
	else if( theHistoryManager.l_orphans.count( o ) > 0 )
		res += IntToStr((size_t)o) + wxT(" : INVALID");
	else
		res += IntToStr((size_t)o) + wxT(" : ") + o->name + wxT(",") + o->GetToken(0);
	
	return  res;
}

//--------------------------------------------------------
// Return true if do or undo process must CONTINUE
bool
History_Elt::Do(  std::list<History_Group*>::iterator it_grp)
{
#ifdef DEBUG_UNDOS
	if( type != H_OBJ_INSERT )
		cout<<"History_Elt::Do : "<<ToStr()<<endl;
#endif
	
	switch( type )
	{
		case H_NAME_CHANGE:
		{
			wxString prev_val = o->name;
			o->SetName( token_val );
			token_val = prev_val;
			break;
		}
		case H_TOKEN_CHANGE:
		{
			wxString prev_val = o->GetToken( token_pos );
			o->SetToken( token_pos, token_val );
			token_val = prev_val;
			break;
		}
		case H_TOKEN_ADD:
		{
			o->SetToken( o->GetNbTokens(), token_val );
			break;
		}
		case H_TOKEN_RM:
		{
			size_t new_nb_tokens = o->GetNbTokens() -1;
			token_val = o->GetToken( new_nb_tokens );
			o->SetNbTokens( new_nb_tokens );
			break;
		}
		case H_OBJ_INSERT:
			// A Append operation
			if( token_pos == 1 )
				o_related->Append_SubObj( o );
			else
				o_related->InsertObject_Before( o );
			theHistoryManager.l_orphans.erase( o );
			break;
		case H_OBJ_RM:
			o->Detach(false);
			theHistoryManager.l_orphans.insert( o );
			break;
			
		case H_DUMMY:
		default:
			wxMessageBox( wxT("BUG!!!!!\n\nHistory_Elt::Do()\nType Invalid !!!\n"));
			break;
	}
	
#ifdef DEBUG_UNDOS
	if( type == H_OBJ_INSERT )
		cout<<"History_Elt::Do : "<<ToStr()<<endl;
#endif
	// Now elt is the past
	is_past = true;
	
	// If some handler have been set, it's him who decide whether or not to continue
	if( theHistoryManager.onDo != NULL )
		return (*theHistoryManager.onDo)( it_grp, this );
	
	// else by default, only do/undo one group
	return false;
}

//--------------------------------------------------------
// Return true if do or undo process must CONTINUE
bool
History_Elt::UnDo(  std::list<History_Group*>::iterator it_grp)
{
#ifdef DEBUG_UNDOS
	if( type != H_OBJ_RM )
		cout<<"History_Elt::UnDo : "<<ToStr()<<endl;
#endif
	switch( type )
	{
		case H_NAME_CHANGE:
		{
			wxString next_val = o->name;
			o->SetName( token_val );
			token_val = next_val;
			break;
		}
		case H_TOKEN_CHANGE:
		{
			wxString next_val = o->GetToken( token_pos );
			o->SetToken( token_pos, token_val );
			token_val = next_val;
			break;
		}
		case H_TOKEN_ADD:
		{
			size_t new_nb_tokens = o->GetNbTokens() -1;
			token_val = o->GetToken( new_nb_tokens );
			o->SetNbTokens( new_nb_tokens );
			break;
		}
		case H_TOKEN_RM:
			o->SetToken( o->GetNbTokens(), token_val );
			break;
		case H_OBJ_INSERT:
			o->Detach(false);
			theHistoryManager.l_orphans.insert( o );
			break;
		case H_OBJ_RM:
			// A Append operation
			if( token_pos == 1 )
				o_related->Append_SubObj( o );
			else
				o_related->InsertObject_Before( o );
			theHistoryManager.l_orphans.erase( o );
			break;
		case H_DUMMY:
		default:
			wxMessageBox( wxT("BUG!!!!!\n\nHistory_Elt::Do()\nType Invalid !!!\n") );
			break;
	}

#ifdef DEBUG_UNDOS
	if( type == H_OBJ_RM )
		cout<<"History_Elt::UnDo : "<<ToStr()<<endl;
#endif

	// Now elt is the future
	is_past = false;
	
	// If some handler have been set, it's him who decide whether or not to continue
	if( theHistoryManager.onUndo != NULL )
		return (*theHistoryManager.onUndo)( it_grp, this );
	
	// else by default, only do/undo one group
	return false;
}


/**
 ***********************************************************
 *  HISTORY_GROUP
 ***********************************************************
 */

History_Group::History_Group()
{
	b_group_opened = true;
}

//--------------------------------------------------------
History_Group::~History_Group()
{
	while( l_elts.empty() == false )
	{
		delete l_elts.back();
		l_elts.pop_back();
	}
}

//--------------------------------------------------------
bool
History_Group::IsElt__CanBeAdded( History_Elt* _elt )
{
	if( b_group_opened == false )
		return false;
	if( l_elts.empty() == true )
		return true;
	
	if( _elt->theDate < l_elts.back()->theDate + MAX_MERGE_DELAY )
		return true;
	
	return false;
}

//--------------------------------------------------------
void
History_Group::Elt__Add( History_Elt* _elt )
{
	l_elts.push_back( _elt );
}

//--------------------------------------------------------
bool 
History_Group::Do(list<History_Group*>::iterator it_grp)
{
	if( l_elts.empty() == true )
		return true;
	
	list<History_Elt*>::iterator it( l_elts.begin())
					,it_end( l_elts.end());

	bool b_res = true;
	for(;it!=it_end;it++)
		b_res = (*it)->Do(it_grp) && b_res;

	return b_res;
}

//--------------------------------------------------------
bool 
History_Group::UnDo(list<History_Group*>::iterator it_grp)
{
	if( l_elts.empty() == true )
		return true;
	
	list<History_Elt*>::iterator it( l_elts.end())
					,it_begin( l_elts.begin());

	bool b_res = true;
	while(it!=it_begin)
	{
		it--;
		b_res = (*it)->UnDo(it_grp) && b_res;
	}

	return b_res;
}

//--------------------------------------------------------
wxString 
History_Group::GetName()
{
	if( name != wxString())
		return name;
	if( l_elts.empty() == true )
		return wxT("NOTHING !!");
	
	History_Elt* _elt = l_elts.front();
	wxString res;
	switch( _elt->type )
	{
		case History_Elt::H_NAME_CHANGE:
			res = wxT("Prop Rename ") + _elt->token_val;
			break;
		case History_Elt::H_TOKEN_CHANGE:
		case History_Elt::H_TOKEN_ADD:
		case History_Elt::H_TOKEN_RM:
			res = wxT("Prop Change ") + _elt->o->name;
			break;
		case History_Elt::H_OBJ_INSERT:
			res = wxT("Insertion ") + _elt->o->name;
			break;
		case History_Elt::H_OBJ_RM:
			res = wxT("Deletion ") + _elt->o->name;
			break;
		case History_Elt::H_DUMMY:
		default:
			res = wxT("NOTHING ");
			break;
	}
	
	return res;
}


/**
 ***********************************************************
 *  HISTORY_MANAGER 
 ***********************************************************
 */
//--------------------------------------------------------
void 
History_Manager::Reset()
{
	m_flag = 0;
	disable_count = 1;
	group_pack_count = 0;
	initial_datas = NULL;
	Clear_History();
	onUndo = NULL;;
	onDo = NULL;
	onEltAdd = NULL;
	onNoMoreUndo = NULL;
	onNoMoreDo = NULL;
	onDeleteHistory_data = NULL;
	onAfterEltAdded = NULL;
}

//--------------------------------------------------------
History_Manager::History_Manager()
{
	Reset();
}

//--------------------------------------------------------
History_Manager::~History_Manager()
{
	Reset();
}

//--------------------------------------------------------
void 
History_Manager::Set_State(bool b_enable)
{
	if( b_enable == true )
	{
		if( disable_count == 0 )
			wxMessageBox( wxT("BUG!!\n\nHistory_Manager::Set_State()\nToo much Call!\n"));
		else
			disable_count--;
	}
	else
		disable_count++;
}

//--------------------------------------------------------
void 
History_Manager::Force_Disable()
{
	disable_count = 1;
}

//--------------------------------------------------------
void 
History_Manager::GroupStart( const wxString& grp_name )
{
	// Start of a new future !
	Clear_Future();
	
	if( group_pack_count == 0 )
	{
		// Check If the last group is possibly empty
		bool b_must_create = true;
		if( IsHistory_Empty() == false )
		{
			list<History_Group*>::iterator it_t(it_now);
			it_t--;
			if( (*it_t)->l_elts.empty() == true )
			{
				b_must_create = false;
				(*it_t)->b_group_opened = true;
				(*it_t)->name = grp_name;
			}
			else
				(*it_t)->b_group_opened = false;
		}
		if( b_must_create == true )
			Group__Append(grp_name);
	}
	group_pack_count++;
}

//--------------------------------------------------------
void 
History_Manager::GroupEnd()
{
	if( IsHistory_Empty() == true )
		return;
	
	if( it_now != l_history_groups.end() )
		wxMessageBox( wxT("BUG !!!\nGroupEnd()\nit_now is not the end !\n"));
	
	std::list<History_Group*>::iterator it_t(it_now);
	it_t--;
	
	// Close the current group
	if( (*it_t)->l_elts.empty() == false )
		(*it_t)->b_group_opened = false;
	// Delete this empty group
	else
		Group__DeleteLast();

	group_pack_count--;
}

//--------------------------------------------------------
void 
History_Manager::Group__Append( const wxString& grp_name )
{
	History_Group* t = new History_Group();
	t->name = grp_name;
	l_history_groups.push_back( t);
}


//--------------------------------------------------------
void 
History_Manager::Clear_History()
{
	// clear history
	while( l_history_groups.empty() == false )
	{
		delete l_history_groups.back();
		l_history_groups.pop_back();
	}
	it_now = l_history_groups.end();
	
	// clear initial datas
	if( initial_datas != NULL && onDeleteHistory_data != NULL )
		(*onDeleteHistory_data)(initial_datas);
	else if( initial_datas != NULL && onDeleteHistory_data == NULL )
		wxMessageBox( wxT("BUG !!!\nHistory_Manager::Clear_History()\ninitial_datas != NULL && onDeleteHistory_data == NULL !!!\n"));
	initial_datas = NULL;

	// Tell about no more doS and undoS
	if( onNoMoreDo != NULL )
		(*onNoMoreDo)(true);
	if( onNoMoreUndo != NULL )
		(*onNoMoreUndo)(it_now == l_history_groups.begin());
}

//--------------------------------------------------------
void 
History_Manager::Clear_Future()
{
	std::list<History_Group*>::iterator it_t;
	while( it_now != l_history_groups.end() )
	{
		it_t = it_now;
		it_now++;
		delete (*it_t);
		l_history_groups.erase( it_t);
	}
	// it_t here is unused
	if( onNoMoreDo != NULL )
		(*onNoMoreDo)(true);
}

//--------------------------------------------------------
bool
History_Manager::IsHistory_Empty()
{
	return l_history_groups.empty();
}

//--------------------------------------------------------
bool 
History_Manager::IsPresent()
{
	return it_now == l_history_groups.end();
}

//--------------------------------------------------------
bool
History_Manager::Elt__Absorb( History_Elt* _elt )
{
	// History is disabled
	if(      IsEnabled() == false
		|| m_flag & M_IN_DO_OR_UNDO
	  )
	{
		_elt->is_past = false;
		_elt->o = NULL;
		delete _elt;
		return false;
	}

	std::list<History_Group*>::iterator it_t;
	
	// An add always invalidate future events
	Clear_Future();
	
	if( l_history_groups.empty() == true )
		Group__Append();
	

	bool b_added = false;
	if( 	   l_history_groups.back()->IsElt__CanBeAdded( _elt ) == true 
		|| group_pack_count > 0 
		)
	{
		it_t = l_history_groups.end();
		it_t--;
		bool b_can_add = true;
		if( onEltAdd != NULL )
			b_can_add = (*onEltAdd)( it_t, _elt );

		if( 	   b_can_add
			|| group_pack_count > 0 
			)
		{
			l_history_groups.back()->Elt__Add( _elt );
			b_added = true;
		}
	}

	if( b_added == false )
	{
		Group__Append();

		// Try to merge hand edited stuffs
		it_t = l_history_groups.end();
		it_t--;
		// may remove the last empty group if _elt must be merge with previous
		HCB_May_Merge_Hand_Edited_Object( it_t, _elt );

		it_t = l_history_groups.end();
		it_t--;
		if( onEltAdd != NULL )
			(*onEltAdd)( it_t, _elt );
		l_history_groups.back()->Elt__Add( _elt );
	}
	
	it_now = l_history_groups.end();
	if( onNoMoreUndo != NULL )
		(*onNoMoreUndo)( it_now == l_history_groups.begin() );

	// Gotta keep trace of which object are orphans
	switch( _elt->type )
	{
		case History_Elt::H_OBJ_INSERT:
			theHistoryManager.l_orphans.erase( _elt->o );
			break;
		case History_Elt::H_OBJ_RM:
			theHistoryManager.l_orphans.insert( _elt->o );
			break;
	}

	if( onAfterEltAdded != NULL )
		(*onAfterEltAdded)();

#ifdef DEBUG_UNDOS
	cout<<"\nHistory_Elt()"<<endl;
	cout<<"\tAdded : "<<_elt->ToStr()<<endl;
	cout<<"\thistory elt count : "<<debug_hist_elt_count<<endl;
	cout<<"\tHistory GROUPs count : "<<l_history_groups.size()<<endl;
#endif

	return true;
}


//--------------------------------------------------------
void 
History_Manager::Set_CallBacks( History_CallBAck* _onUndo
					, History_CallBAck* _onDo
					, History_CallBAck* _onEltAdd
					, History_CallBAck_Simple* _onNoMoreUndo
					, History_CallBAck_Simple* _onNoMoreDo
					// For clean deletion of <datas>
					, History_CallBAck_Delete* _onDeleteHistory_data
					, History_CallBAck_Basic* _onAfterEltAdded
					)
{
	onUndo = _onUndo;
	onDo = _onDo;
	onEltAdd = _onEltAdd;
	onNoMoreUndo = _onNoMoreUndo;
	onNoMoreDo = _onNoMoreDo;
	onDeleteHistory_data = _onDeleteHistory_data;
	onAfterEltAdded = _onAfterEltAdded;
}

//--------------------------------------------------------
bool 
History_Manager::IsFirstGroup( std::list<History_Group*>::iterator it )
{
	return (it == l_history_groups.begin() );
}

//--------------------------------------------------------
bool 
History_Manager::IsLastGroup(  list<History_Group*>::iterator it )
{
	if( l_history_groups.empty() == true )
		return true;
	list<History_Group*>::iterator it_t(l_history_groups.end());
	it_t--;
	return( it == it_t );
}


//--------------------------------------------------------
bool
History_Manager::Do()
{
	m_flag |= M_IN_DO_OR_UNDO;
	if( l_history_groups.empty() == true || it_now == l_history_groups.end() )
	{
		m_flag &= ~M_IN_DO_OR_UNDO;
		return false;
	}
	
	// While must continue
	while( it_now != l_history_groups.end() )
	{
		bool b_cont = (*it_now)->Do(it_now);
		it_now++;
		if(b_cont != true )
			break;
	}

	if( onNoMoreUndo != NULL )
		(*onNoMoreUndo)( it_now == l_history_groups.begin() );
	if( onNoMoreDo != NULL )
		(*onNoMoreDo)(it_now == l_history_groups.end());

	m_flag &= ~M_IN_DO_OR_UNDO;
	return true;
}


//--------------------------------------------------------
bool
History_Manager::UnDo()
{
	m_flag |= M_IN_DO_OR_UNDO;
	if( 	   	l_history_groups.empty() == true 
		|| 
			it_now == l_history_groups.begin()
		)
	{
		m_flag &= ~M_IN_DO_OR_UNDO;
		return false;
	}
	
	// While must continue
	it_now--;
	while(1)
	{
		// if the undo process tell to stop
		if( (*it_now)->UnDo(it_now) == false  )
			break;
		
		// if no more to undo
		if( it_now == l_history_groups.begin() )
			break;
		it_now--;
	}
	
	if( onNoMoreUndo != NULL )
		(*onNoMoreUndo)( it_now == l_history_groups.begin() );
	if( onNoMoreDo != NULL )
		(*onNoMoreDo)(it_now == l_history_groups.end());

	m_flag &= ~M_IN_DO_OR_UNDO;
	return true;
}


//--------------------------------------------------------
History_Elt* 
History_Manager::Get_Last_HistoryElt()
{
	if( IsHistory_Empty() == true )
		return NULL;
	
	list<History_Group*>::iterator it_t(l_history_groups.end());
	it_t--;
	while( 1 )
	{
		History_Group* h_grp = (*it_t);
		if( h_grp->l_elts.empty() == false )
		{
			list<History_Elt*>::iterator it_tt( h_grp->l_elts.end());
			it_tt--;
			return (*it_tt);
		}
		
		if( it_t != l_history_groups.begin() )
			break;
		it_t--;
	}
	return NULL;
}

//--------------------------------------------------------
void 
History_Manager::Group__DeleteLast()
{
	if( l_history_groups.empty() == true )
		return;
	
	list<History_Group*>::iterator it_last(l_history_groups.end());
	it_last--;
	if( it_now == it_last )
		it_now = l_history_groups.end();
	
	delete( (*it_last) );
	l_history_groups.erase( it_last);
	
 	if( onNoMoreDo != NULL )
		(*onNoMoreDo)(it_now == l_history_groups.end());
}

/**
 ***********************************************************
 *  DETECT AND CORRECT HAND EDITION
 ***********************************************************
 */
int 
HCB_May_Merge_Hand_Edited_Object(
			  list<History_Group*>::iterator it_group
			, History_Elt*  _elt
					)
{
	// If no previous group, or this group is not empty  ==>> get out
	if( 		theHistoryManager.IsFirstGroup( it_group ) == true 
		|| 	(*it_group)->l_elts.empty() == false
		)
		return 1;	// No veto to add elt to current group

	// Not a edition
	if( 	   _elt->type != History_Elt::H_TOKEN_CHANGE 
		&& _elt->type != History_Elt::H_NAME_CHANGE
	  )
		return 1;	// No veto to add elt to current group
	
	// Get the last elt of the prev group
	list<History_Group*>::iterator it_prev_group(it_group);
	it_prev_group--;

	if( (*it_prev_group)->l_elts.empty() == true )
		return 1;	// No veto to add elt to current group

	History_Elt* last_prev_grp_elt = (*it_prev_group)->l_elts.back();

	
/*	// Prev elt is not an edtion
	if( 	   last_prev_grp_elt->type != History_Elt::H_TOKEN_CHANGE 
		&& last_prev_grp_elt->type != History_Elt::H_NAME_CHANGE
	  )
		return 1;	// No veto to add elt to current group
*/	
	
	// Here the hand edition detection
	if( 		last_prev_grp_elt->o == _elt->o
		&&	
			last_prev_grp_elt->theDate + MAX_HAND_EDITION_DETECTION_DELAY
			>
			_elt->theDate
//		&&
//			last_prev_grp_elt->token_pos == _elt->token_pos
		)
	{
		// Have to delete the current group (as it is empty it's ok)
		theHistoryManager.Group__DeleteLast();
		// After that the current history group is the prev one
	}

	return 1;	// No veto to add elt to current group
}



// find the most group-related-recent control states
void*
Get_Prev_Ctrl_States(list<History_Group*>::iterator it_grp,bool b_grp_end_elt_valid )
{
	list<History_Group*>::iterator it_t( it_grp );
	while(
		(    
			(*it_t)->l_elts.empty() == true
			|| 
			(*it_t)->l_elts.back()->datas == NULL
			||
			(
				b_grp_end_elt_valid == false
				&&
				it_t == it_grp
			)
		)
		&& 
		theHistoryManager.IsFirstGroup( it_t) == false )
	{
		it_t--;
	}
	
	if( (*it_t)->l_elts.empty() == true )
	{
		wxMessageBox( wxT("BUG !!\nGet_Prev_Ctrl_States()\nGroup is Empty !!!\n" ));
		return NULL;
	}
	
	// On cherche le précédent history state
	list<History_Elt*>::iterator it_u( (*it_t)->l_elts.end()), it_b( (*it_t)->l_elts.begin());
	do 
	{
		it_u--;
		if( (*it_u)->datas != NULL )
			return (*it_u)->datas;
	}
	while( it_u != it_b );
	
	if( theHistoryManager.IsFirstGroup( it_t) 
		&& theHistoryManager.initial_datas != NULL )
		return theHistoryManager.initial_datas;
	
	else
		wxMessageBox( wxT("BUG !!\nGet_Prev_Ctrl_States()\nNo previous Groups States !!!\n"));
	return NULL;
}
