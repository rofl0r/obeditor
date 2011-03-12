/*
 * object_game.cpp
 *
 *  Created on: 24 avr. 2009
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <map>

#include "common__tools.h"
#include "common__object_stage.h"


using namespace std;

/*\internal*********************************************
********	OBJECT RELATED TO THE STAGE FILES
******************************************************/

//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a stage of a game
 */

/*
 * STAGE FILE ORGANISATION
 * - HEADERS
 * - Background layer
 * - Front layer
 * - Panels
 * - Walls/Holes
 * - Spawns
 */

wxString stageHeaderTags[] = {
	wxT("loadingbg"), 
	wxT("music"), 
	wxT("allowselect"),
	wxT("direction"), 
	wxT("facing"), 
	wxT("rock"), 
	wxT("bgspeed"), 
	wxT("mirror"), 
	wxT("bossmusic"), 
	wxT("settime"), 
	wxT("setweap"), 
	wxT("notime"), 
	wxT("noreset"), 
	wxT("noslow"), 
	wxT("type"), 
	wxT("nohit"), 
	wxT("gravity"), 
	wxT("maxfallspeed"), 
	wxT("maxtossspeed"), 
	wxT("cameratype"), 
	wxT("stagenumber"), 
	wxT("palette"), 
	wxT("updatescript"), 
	wxT("updatedscript"), 
	wxT("keyscript"), 
	wxT("levelscript"), 
	wxT("endlevelscript"), 
	wxT("blocked"),
	wxT("endhole")
};


bool ob_stage::IsHeaderTag( const wxString& _name )
{
	for( size_t i =0; i< t_size_of(stageHeaderTags); i++ )
	{
		if( _name.Upper() == stageHeaderTags[i].Upper() )
			return true;
	}
	return false;
}


//**********************************************

int stage_at_types[] =  { 
	OB_TYPE_STAGE_AT,
	};

bool ob_stage::Is_Stage_At_Type( ob_object* _obj )
{
	if(  ob_stage::Is_Stage_Spawn_Type( _obj ) )
		return true;
	
	for( size_t i =0; i< t_size_of(stage_at_types); i++ )
	{
		if( _obj->type == stage_at_types[i] )
			return true;
	}
	return false;
}


int stage_spawn_types[] =  { 
	OB_TYPE_SPAWN, 
	OB_TYPE_SPAWN_HEALTHY, 
	OB_TYPE_SPAWN_ITEM, 
	};

bool ob_stage::Is_Stage_Spawn_Type( ob_object* _obj )
{
	for( size_t i =0; i< t_size_of(stage_spawn_types); i++ )
	{
		if( _obj->type == stage_spawn_types[i] )
			return true;
	}
	return false;
}



//**********************************************

ob_stage::ob_stage()
:ob_object_container()
{
	type = OB_TYPE_STAGE;
	direction = STAGE_RIGHT;
}

//-----------------------------------------------------------------
void
ob_stage::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_stage();
	ob_object_container::pClone( res );
	((ob_stage*)res)->direction = direction;
}

//**********************************************

ob_stage::~ob_stage()
{
}

//---------------------------------------------------------------------
bool 
ob_stage::Add_SubObj( ob_object* temp )
{
	if( ob_object_container::Add_SubObj( temp ) == true )
	{
		if( temp->name.Upper() == wxT("DIRECTION") )
			Direction_ComputeAndCache();
		return true;
	}
	return false;
}


//---------------------------------------------------------------------
int 
ob_stage::Direction_ComputeAndCache()
{
	wxString str_dir =  GetSubObject_Token( wxT("direction"), 0 ).Upper();
	
	if( 	   str_dir == wxString()    || str_dir == wxT("RIGHT")
		||str_dir == wxT("BOTH") || str_dir == wxT("RIGHTLEFT")
		)
		direction = STAGE_RIGHT;
	
	else if( str_dir == wxT("LEFT") || str_dir == wxT("LEFTRIGHT") )
		direction = STAGE_LEFT;
	
	else if( str_dir == wxT("UP") )
		direction = STAGE_UP;
	
	else if( str_dir == wxT("DOWN") )
		direction = STAGE_DOWN;
	
	else if( str_dir == wxT("IN")  || str_dir == wxT("INOUT") )
		direction = STAGE_IN;
	
	else if( str_dir == wxT("OUT") || str_dir == wxT("OUTIN") )
		direction = STAGE_OUT;
	else
		direction = STAGE_RIGHT;
	return direction;
}


//---------------------------------------------------------------------
wxSize 
ob_stage::GetDefScrollzBounds( ob_object* st_declar )
{
	if( st_declar == NULL )
		return wxSize(0,0);
	
	if( direction == STAGE_IN || direction == STAGE_OUT )
		return wxSize(0,0);
	
	return ((ob_StageDeclaration*)st_declar)->GetDefScrollzBounds();
}
	
//**********************************************
ob_stage_panel** 
ob_stage::Get_Panels( size_t& nb_panels )
{
	nb_panels = 0;
	
	// Get the panels order
	wxString curr_order = Get_Panels_Order();

	if( curr_order.Len() == 0 )
		return NULL;
	
	// Get the panels refs
	size_t nb_unik_panels = 0;
	ob_stage_panel** oPanels = (ob_stage_panel**) 
			GetSubObjectS_ofType( OB_TYPE_STAGE_PANEL, nb_unik_panels );
	if( nb_unik_panels == 0 )
		return NULL;

	int tay_panels_map = curr_order.Len();
	ob_stage_panel** res = new ob_stage_panel*[tay_panels_map];
	
		
	for( int i = 0; i < tay_panels_map; i++ )
	{
		if( curr_order[i] < 'a' || curr_order[i] >= 'a' + tay_panels_map )
			continue;
		res[nb_panels] = oPanels[curr_order[i] - 'a'];
		nb_panels++;
	}

	delete[] oPanels;
	return res;
}

//**********************************************
ob_object* 
ob_stage::Get_LastHeaderObj()
{
	ob_object * t = first_subobj;

	// Get the first one
	bool b_found = false;
	while( ! b_found )
	{
		if( IsHeaderTag(t->name ) )
			b_found = true;
		t = t->next;
	}

	// Get the last one
	while( t != NULL )
	{
		if( ! IsHeaderTag(t->name ) && ! t->Is_EmptyOrComment() )
			return t->prev;
		t = t->next;
	}
	return NULL;
}


//**********************************************

ob_object* ob_stage::Get_First_AT_Obj()
{
	ob_object * t = first_subobj;
	while( t != NULL )
	{
		if( ! ob_stage::Is_Stage_At_Type( t ) )
			break;
		t = t->next;
	}
	return t;
}

//---------------------------------------------------------------------
list<ob_stage_object*>*
ob_stage::Get_SubObj_With_StageType( const int stage_type )
{
	list<ob_stage_object*>* res = new list<ob_stage_object*>;
	
	size_t tot_subobj_count;
	ob_object** arr_subobj = GetAllSubObjS( tot_subobj_count );
	if( arr_subobj == NULL )
		return res;
	
	for( size_t i = 0; i < tot_subobj_count; i++ )
	{
		if( arr_subobj[i]->Is_Ob_Stage_Object() == true )
		{
			ob_stage_object* t = (ob_stage_object*) arr_subobj[i];
			if( t->object_type == stage_type )
				res->push_back( t );
		}
	}
	return res;
}

//---------------------------------------------------------------------
wxString 
ob_stage::Get_Panels_Order()
{
	wxString res = wxString();
	size_t nb_orders;
	ob_object** orders = GetSubObjectS( wxT("order"),nb_orders );
	if( orders == NULL )
		return res;
	else
		for( size_t i = 0; i < nb_orders; i++ )
			res += orders[i]->GetToken( 0 );
	
	delete[] orders;
	return res;
}


//**********************************************

bool ob_stage::Add_SubObj_Header( ob_object* _obj, bool b_after )
{
	ob_object* t = Get_LastHeaderObj();
	// No non header tag in the object
	if( t == NULL )
		return Prepend_SubObj( _obj );
	else
	{
		if( b_after )
			return t->InsertObject_After( _obj );
		else
			return t->InsertObject_Before( _obj );
	}
}


//**********************************************
//
// BGS
//

bool ob_stage::Add_SubObj_BGs( ob_object* _obj, bool b_after )
{
	size_t nb;
	ob_object** l_obj = GetSubObjectS_ofType( OB_TYPE_BG_LAYER, nb );

	if( nb == 0 )
		return Add_SubObj_Header( _obj, true );
	else
	{
		ob_object* t;
		if( b_after )
		{
			t = l_obj[nb-1];
			delete[] l_obj;
			return t->InsertObject_After( _obj );
		}
		else
		{
			t = l_obj[0];
			delete[] l_obj;
			return t->InsertObject_Before( _obj );
		}
	}

}


//**********************************************

bool ob_stage::Add_SubObj_FrontLayer( ob_object* _obj, bool b_after )
{
	size_t nb;
	ob_object** l_obj = GetSubObjectS_ofType( OB_TYPE_FRONT_PANEL, nb );

	if( nb == 0 )
	{
		return Add_SubObj_BGs( _obj, true );
	}
	else
	{
		ob_object* t;
		if( b_after )
		{
			t = l_obj[nb-1];
			delete[] l_obj;
			return t->InsertObject_After( _obj );
		}
		else
		{
			t = l_obj[0];
			delete[] l_obj;
			return t->InsertObject_Before( _obj );
		}
	}
}


//**********************************************

bool ob_stage::Add_SubObj_Panel( ob_object* _obj, bool b_after )
{
	size_t nb;
	ob_object** l_obj = GetSubObjectS_ofType( OB_TYPE_STAGE_PANEL, nb );

	if( nb == 0 )
	{
		return Add_SubObj_FrontLayer( _obj, true );
	}
	else
	{
		ob_object* t;
		if( b_after )
		{
			t = l_obj[nb-1];
			delete[] l_obj;
			if( t->next->name.Upper() == wxT("ORDER") && _obj->type != OB_TYPE_STAGE_PANEL )
				t = t->next;

			return t->InsertObject_After( _obj );
		}
		else
		{
			t = l_obj[0];
			delete[] l_obj;
			if( t->prev->name.Upper() == wxT("ORDER") && _obj->type != OB_TYPE_STAGE_PANEL )
				t = t->prev;
			return t->InsertObject_Before( _obj );
		}
	}
}


//**********************************************

bool ob_stage::Add_SubObj_WallHole( ob_object* _obj, bool b_after )
{
	size_t nb;
	ob_object** l_obj;
	ob_object* t;

	if( b_after )
	{
		l_obj = GetSubObjectS_ofType( OB_TYPE_WALL, nb );
		if( nb > 0 )
		{
			t = l_obj[nb-1];
			delete[] l_obj;
			return t->InsertObject_After( _obj );
		}
	}

	l_obj = GetSubObjectS_ofType( OB_TYPE_HOLE, nb );
	if( nb > 0 )
	{
		if( b_after )
		{
			t = l_obj[nb-1];
			delete[] l_obj;
			return t->InsertObject_After( _obj );
		}
		else
		{
			t = l_obj[0];
			delete[] l_obj;
			return t->InsertObject_Before( _obj );
		}
	}

	else
	{
		return Add_SubObj_Panel( _obj, true );
	}
}


//**********************************************
bool 
ob_stage::Add_SubObj_AT( ob_object* _obj, bool b_after )
{
	if( b_after )
		return Add_SubObj( _obj );

	ob_object* t = Get_First_AT_Obj();
	if( t == NULL )
		return Add_SubObj_WallHole( _obj, true );
	else
		return t->InsertObject_Before( _obj );
}

//----------------------------------------------------------------
bool 
ob_stage::Insert_SubObj_AT( ob_object* _obj )
{
	ob_object* t = Get_First_AT_Obj();
	if( t == NULL )
		return Add_SubObj_WallHole( _obj, true );
	if( t->InsertObject_Before( _obj ) == false )
		return false;
	
	((ob_stage_at*)_obj)->Reorder_At();
	return true;
}


//**********************************************
bool 
ob_stage::Add_SubObj( ob_object* _obj, bool b_sort_at )
{
	// Add a header tag
	if( IsHeaderTag(_obj->name ) )
		return Add_SubObj_Header( _obj );


	// Add a background layer
	else if( _obj->name.Upper() == wxT("BACKGROUND") || _obj->name.Upper() == wxT("BGLAYER")
				|| _obj->type  == OB_TYPE_BG_LAYER
				)
	{
		return Add_SubObj_BGs( _obj );
	}

	// Add a front panel object
	else if(   _obj->name.Upper() == wxT("FRONTPANEL") || _obj->type  == OB_TYPE_FRONT_PANEL )
	{
		return Add_SubObj_FrontLayer( _obj );
	}

	// Add a panel object
	else if(   _obj->name.Upper() == wxT("PANEL") || _obj->type  == OB_TYPE_STAGE_PANEL
		|| _obj->name.Upper() == wxT("ORDER") )
	{
		return Add_SubObj_Panel( _obj );
	}

	// Add a Wall/Hole object
	else if(   _obj->name.Upper() == wxT("WALL") || _obj->type  == OB_TYPE_WALL
		|| _obj->name.Upper() == wxT("HOLE") || _obj->type  == OB_TYPE_HOLE )
	{
		return Add_SubObj_WallHole( _obj );
	}

	else if( ob_stage::Is_Stage_At_Type( _obj ))
	{
		if( b_sort_at == false )
			return Add_SubObj_AT( _obj );
		else
			return Insert_SubObj_AT( _obj );
	}

	else
	{
		ObjectsLog( MYLOG_WARNING, -1,
			    wxT("ob_stage::Add_SubObj : Unknown object type : ") + _obj->name );
		return Add_SubObj_Header( _obj );
	}
}




//**********************************************

ob_object* ob_stage::Guess_and_ConstructNewObject(MyLine* line, const int _num_line )
{
	// if Empty Line
	if( line == NULL || line->GetTokensCount() <= 0 )
		return new ob_object(line, _num_line);

	wxString _name = *(line->GetToken(0)->data);
	wxString _token;
	if( line->GetToken(1) != NULL && line->GetToken(1)->data != NULL )
		_token = *( line->GetToken(1)->data );

	ob_object *temp;


	if( _name.Upper() == wxT("BACKGROUND") || _name.Upper() == wxT("BGLAYER") )
		temp = new ob_BG_Layer(line,_num_line);

	else if( _name.Upper() == wxT("FRONTPANEL") )
		temp = new ob_front_panel(line,_num_line);

	else if( _name.Upper() == wxT("PANEL") )
		temp = new ob_stage_panel(line,_num_line);

	else if( _name.Upper() == wxT("HOLE") )
		temp = new ob_hole(line,_num_line);

	else if( _name.Upper() == wxT("WALL") )
		temp = new ob_wall(line,_num_line);

	else if( ob_player_spawn::Get_StageObjectType(_name) != STAGETYPE_NONE )
		temp = new ob_player_spawn(line,_num_line);

	else if( ob_stage_at::Get_StageObjectType( _name ) != STAGETYPE_NONE )
		temp = new ob_stage_at(line,_num_line);

	else if( ob_spawn::Get_StageObjectType( _name, _token ) != STAGETYPE_NONE )
		temp = new ob_spawn(line,_num_line);

	else if(ob_spawn_healthy::Get_StageObjectType(_name, _token) != STAGETYPE_NONE )
		temp = new ob_spawn_healthy(line,_num_line);

	else if( ob_spawn_item::Get_StageObjectType( _name, _token ) != STAGETYPE_NONE )
		temp = new ob_spawn_item(line,_num_line);

	else
		temp = new ob_stage_object( line, _num_line);

//	wxLogInfo( "At line " + IntToStr( _num_line +1) + ", object build :" + temp->name );
	return temp;
}

//**********************************************
ob_object* 
ob_stage::Guess_and_ConstructNewObject(const wxString& __name, const wxString& token0 )
{
	ob_object *temp;
	wxString _name = __name;
	wxArrayString _tokens;
	_tokens.Add( token0 );

	if( _name.Upper() == wxT("BACKGROUND") || _name.Upper() == wxT("BGLAYER") )
		temp = new ob_BG_Layer(_name,_tokens);

	else if( _name.Upper() == wxT("FRONTPANEL") )
		temp = new ob_front_panel(_name,_tokens);

	else if( _name.Upper() == wxT("PANEL") )
		temp = new ob_stage_panel(_name,_tokens);

	else if( _name.Upper() == wxT("HOLE"))
		temp = new ob_hole(_name,_tokens);

	else if( _name.Upper() == wxT("WALL") )
		temp = new ob_wall(_name,_tokens);

	else if( ob_player_spawn::Get_StageObjectType(_name) != STAGETYPE_NONE )
		temp = new ob_player_spawn(_name,_tokens);

	else if( ob_stage_at::Get_StageObjectType( _name ) != STAGETYPE_NONE )
		temp = new ob_stage_at(_name,_tokens);

	else if( ob_spawn::Get_StageObjectType( _name, token0 ) != STAGETYPE_NONE )
		temp = new ob_spawn(_name,_tokens);

	else if(ob_spawn_healthy::Get_StageObjectType(_name, token0) != STAGETYPE_NONE )
		temp = new ob_spawn_healthy(_name,_tokens);

	else if( ob_spawn_item::Get_StageObjectType( _name, token0 ) != STAGETYPE_NONE )
		temp = new ob_spawn_item(_name,_tokens);

	else
		temp = new ob_stage_object(_name,_tokens);

//	wxLogInfo( "At line " + IntToStr( _num_line +1) + ", object build :" + temp->name );
	return temp;
}

//**********************************************
//**********************************************
//**********************************************
/**
 * An object holded by a stage object
 */

//**********************************************

ob_BG_Layer::ob_BG_Layer( MyLine* line, const int num_line )
:ob_object( line, num_line ), Image_Holder()
{
	type = OB_TYPE_BG_LAYER;
}


//**********************************************

ob_BG_Layer::ob_BG_Layer( const wxString& _name, wxArrayString& _tokens)
:ob_object( _name, _tokens ), Image_Holder()
{
	type = OB_TYPE_BG_LAYER;
}


//-----------------------------------------------------------------
void
ob_BG_Layer::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_BG_Layer();
	ob_object::pClone( res );
}

//**********************************************

ob_BG_Layer::~ob_BG_Layer()
{
}


//**********************************************

void ob_BG_Layer::Validate()
{
}


//**********************************************

wxString ob_BG_Layer::GetName()
{
	wxString path = GetToken( 0 );
	if( path == wxString() )
		return wxT("NOFILE");

	wxFileName fn = GetObFile( path );
	return fn.GetFullName();

}


//**********************************************

wxFileName ob_BG_Layer::GetFileName()
{
	return GetObFile( GetToken( 0 ));
}


//**********************************************
void 
ob_BG_Layer::SetToDefault()
{
	name = wxT("bglayer");
	SetAllTokenFromStr( wxT("data 0.5 0.5 0 0 0 0 5000 5000 0 0") );
}

//**********************************************
void 
ob_BG_Layer::FillMissings()
{
	size_t i = 0;
	if( nb_tokens < i+1 )
		SetToken( i, wxT("data") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0.5") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0.5") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("5000") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("5000") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0") );
	i++;
	
	if( nb_tokens < i+1 )
		SetToken( i, wxT("0") );
	i++;
}

//------------------------------------------------------------
int  
ob_BG_Layer::Get_RepeatingVal( int direction )
{
	wxString res;
	switch( direction )
	{
		case STAGE_RIGHT:
		case STAGE_LEFT:
			res = GetToken( 7 );
			break;
		case STAGE_UP:
		case STAGE_DOWN:
		case STAGE_IN:
		case STAGE_OUT:
			res = GetToken( 8 );
			break;
	}
	if( res == wxString() )
		res = wxT("5000");
	return StrToInt( res );
}

//**********************************************
bool 
ob_BG_Layer::Update_With( ob_BG_Layer* oLay )
{
	if( oLay == NULL )
		return false;
	
	bool b_res = false;
	ob_BG_Layer* t0 = (ob_BG_Layer*) this->Clone();
	ob_BG_Layer* t1 = (ob_BG_Layer*) oLay->Clone();
	
	t0->FillMissings();
	t1->FillMissings();
	
	if( t0->arr_token_size != t1->arr_token_size )
		b_res = true;
	else
	{
		for( size_t i = 0; i < t0->arr_token_size; i++ )
			if( t0->GetToken( i ) != t1->GetToken( i ) )
			{
				b_res = true;
				break;
			}
	}
	
	delete t0;
	delete t1;
	
	if( b_res == true )
		SetData( oLay );

	return b_res;
}


//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a stage of a game
 */

//**********************************************

ob_front_panel::ob_front_panel( MyLine* line, const int num_line )
:ob_object( line, num_line ), Image_Holder()
{
	type = OB_TYPE_FRONT_PANEL;
}


//**********************************************

ob_front_panel::ob_front_panel( const wxString& _name, wxArrayString& _tokens)
:ob_object( _name, _tokens ), Image_Holder()
{
	type = OB_TYPE_FRONT_PANEL;
}


//-----------------------------------------------------------------
void
ob_front_panel::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_front_panel();
	ob_object::pClone( res );
}

//**********************************************

ob_front_panel::~ob_front_panel()
{
}


//**********************************************

void ob_front_panel::Validate()
{
}


//**********************************************

wxFileName ob_front_panel::GetFileName()
{
	return GetObFile( GetToken( 0 ));
}

//**********************************************
wxString 
ob_front_panel::GetName()
{
	wxString path = GetToken( 0 );
	if( path == wxString() )
		return wxT("NOFILE");

	wxFileName fn = GetObFile( path );
	return fn.GetFullName();

}

//**********************************************
bool 
ob_front_panel::Update_With( ob_front_panel* oPan )
{
	if( oPan == NULL )
		return false;
	
	if( oPan->GetToken( 0 ) != this->GetToken( 0 ) )
	{
		SetToken( 0, oPan->GetToken( 0 ) );
		return true;
	}
	
	return false;
}


//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a stage of a game
 */

//**********************************************

ob_stage_panel::ob_stage_panel( MyLine* line, const int num_line )
:ob_object( line, num_line ), Image_Holder()
{
	type = OB_TYPE_STAGE_PANEL;
}


//**********************************************

ob_stage_panel::ob_stage_panel( const wxString& _name, wxArrayString& _tokens)
:ob_object( _name, _tokens ), Image_Holder()
{
	type = OB_TYPE_STAGE_PANEL;
}


//-----------------------------------------------------------------
void
ob_stage_panel::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_stage_panel();
	ob_object::pClone( res );
}

//**********************************************

ob_stage_panel::~ob_stage_panel()
{
}


//**********************************************

void ob_stage_panel::Validate()
{
}


//**********************************************

wxString ob_stage_panel::GetName()
{
	wxString path = GetToken( 0 );
	if( path == wxString() )
		return wxT("NOFILE");

	wxFileName fn = GetObFile( path );
	return fn.GetFullName();
}


//**********************************************

void ob_stage_panel::SetToDefault()
{
	name = wxT("panel");
	SetToken( 0, wxT("none" ));
	SetToken( 1, wxT("none" ));
	SetToken( 2, wxT("none" ));
}


//**********************************************

wxFileName ob_stage_panel::GetFileName()
{
	wxString ob_path = GetToken( 0 );
	if( ob_path == wxString() )
		return wxFileName();

	return GetObFile( ob_path );
}


//**********************************************
//**********************************************
//**********************************************
/**
 * An object holded by a stage object
 */

ob_stage_object::ob_stage_object( MyLine* line, const int num_line )
:ob_object( line, num_line )
{
	type = OB_STAGE_OBJECT;
	object_type = STAGETYPE_NONE;
}


//**********************************************

ob_stage_object::ob_stage_object(const wxString& _name, wxArrayString& _tokens)
:ob_object( _name, _tokens )
{
	type = OB_STAGE_OBJECT;
	object_type = STAGETYPE_NONE;
}


//-----------------------------------------------------------------
void
ob_stage_object::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_stage_object();
	ob_object::pClone( res );
	((ob_stage_object*)res)->object_type = object_type;
}

//**********************************************

ob_stage_object::~ob_stage_object()
{
}


//**********************************************

void ob_stage_object::Validate()
{
}


//**********************************************
wxString 
ob_stage_object::ToStr()
{
/*	wxString t = "++++++++++++++++\n";
	t += ob_object::ToStr();
	t += "--------------\n";
	return t;
	*/
	return ob_object::ToStr();
}

//-------------------------------------------------------------
bool
ob_stage_object::Get_Coords(int& x, int& y)
{
	x = -1;
	y = -1;
	return false;
}

//-------------------------------------------------------------
// used for object which use coords for screen name
wxString 
ob_stage_object::CoordsToStr()
{
	int x,y;
	if( Get_Coords(x,y) == false )
		return wxT("NO Coords");
	return IntToStr(x) + wxT(" , ") + IntToStr(y);
}


//-------------------------------------------------------------
void 
ob_stage_object::Coords_Decal( const wxSize& _decal )
{
	return;
}

//-------------------------------------------------------------
void 
ob_stage_object::Init_and_Center( wxSize& _here )
{
	return;
}


//**********************************************
//**********************************************
//**********************************************
/**
 * A Wall
 */

//**********************************************

ob_wall::ob_wall( MyLine* line, const int num_line )
:ob_stage_object( line, num_line )
{
	type = OB_TYPE_WALL;
	object_type = SOBJ_WALL;
}


//**********************************************

ob_wall::ob_wall(const wxString& _name, wxArrayString& _tokens)
:ob_stage_object( _name, _tokens )
{
	type = OB_TYPE_WALL;
	object_type = SOBJ_WALL;
}


//-----------------------------------------------------------------
void
ob_wall::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_wall();
	ob_stage_object::pClone( res );
}

//**********************************************

ob_wall::~ob_wall()
{
}


//**********************************************
void 
ob_wall::Validate()
{
}


//-------------------------------------------------------------
bool
ob_wall::Get_Coords(int& x, int& y)
{
	x = -1;
	y = -1;
	wxString t;
	t = GetToken( 0 );
	if( t == wxString() )
		return false;
	x = StrToInt( t );
	
	t = GetToken( 1 );
	if( t == wxString() )
		return false;
	y = StrToInt( t );
	
	return true;
}

//-------------------------------------------------------------
void 
ob_wall::Coords_Decal( const wxSize& _decal )
{
	int x = StrToInt( GetToken(0));
	int z = StrToInt( GetToken(1));
	SetToken(0, IntToStr( x + _decal.x ));
	SetToken(1, IntToStr( z + _decal.y ));
}

//-------------------------------------------------------------
void 
ob_wall::Init_and_Center( wxSize& _here )
{
	SetToken(0,wxT("0"));	//X
	SetToken(1,wxT("0"));	//Z
	SetToken(2,wxT("10"));//ul
	SetToken(3,wxT("0"));//ll
	SetToken(4,wxT("40"));//ur
	SetToken(5,wxT("30"));//lr
	SetToken(6,wxT("30"));//depth
	SetToken(7,wxT("20"));//alt
	
	SetToken(0,IntToStr(_here.x - 20) );	//X
	SetToken(1,IntToStr(_here.y + 25) );	//Z
}



//**********************************************
//**********************************************
//**********************************************
/**
 * A Hole
 */
//**********************************************

ob_hole::ob_hole( MyLine* line, const int num_line )
:ob_stage_object( line, num_line )
{
	type = OB_TYPE_HOLE;
	object_type = SOBJ_HOLE;
}


//**********************************************

ob_hole::ob_hole(const wxString& _name, wxArrayString& _tokens)
:ob_stage_object( _name, _tokens )
{
	type = OB_TYPE_HOLE;
	object_type = SOBJ_HOLE;
}


//-----------------------------------------------------------------
void
ob_hole::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_hole();
	ob_stage_object::pClone( res );
}

//**********************************************

ob_hole::~ob_hole()
{
}


//-------------------------------------------------------------
void 
ob_hole::Validate()
{
}

//-------------------------------------------------------------
bool
ob_hole::Get_Coords(int& x, int& y)
{
	x = -1;
	y = -1;
	wxString t;
	t = GetToken( 0 );
	if( t == wxString() )
		return false;
	x = StrToInt( t );
	
	t = GetToken( 1 );
	if( t == wxString() )
		return false;
	y = StrToInt( t );
	
	return true;
}

//-------------------------------------------------------------
void 
ob_hole::Coords_Decal( const wxSize& _decal )
{
	int x = StrToInt( GetToken(0));
	int z = StrToInt( GetToken(1));
	SetToken(0, IntToStr( x + _decal.x ));
	SetToken(1, IntToStr( z + _decal.y ));
}

//-------------------------------------------------------------
void 
ob_hole::Init_and_Center( wxSize& _here )
{
	SetToken(0,wxT("0"));	//X
	SetToken(1,wxT("0"));	//Z
	SetToken(2,wxT("10"));//ul
	SetToken(3,wxT("0"));//ll
	SetToken(4,wxT("40"));//ur
	SetToken(5,wxT("30"));//lr
	SetToken(6,wxT("30"));//depth
	
	SetToken(0,IntToStr(_here.x - 20) );	//X
	SetToken(1,IntToStr(_here.y + 15) );	//Z
}



//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a stage of a game
 */


int ob_player_spawn::Get_StageObjectType( const wxString& _name )
{
	if( _name.Upper() == wxT("SPAWN1") || _name.Upper() == wxT("SPAWN2")
		|| _name.Upper() == wxT("SPAWN3") || _name.Upper() == wxT("SPAWN4") )
		return SOBJ_PLAYER_SPAWN;
	else
		return STAGETYPE_NONE;
}

//**********************************************

ob_player_spawn::ob_player_spawn( MyLine* line, const int num_line )
:ob_stage_object( line, num_line )
{
	type = OB_TYPE_PLAYER_SPAWN;
	if( line != NULL )
		object_type =  Get_StageObjectType( *(line->GetToken(0)->data) );
}


//**********************************************

ob_player_spawn::ob_player_spawn(const wxString& _name, wxArrayString& _tokens)
:ob_stage_object( _name, _tokens )
{
	type = OB_TYPE_PLAYER_SPAWN;
	object_type =  Get_StageObjectType( _name );
}


//-----------------------------------------------------------------
void
ob_player_spawn::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_player_spawn();
	ob_stage_object::pClone( res );
}

//**********************************************

ob_player_spawn::~ob_player_spawn()
{
}


//-------------------------------------------------------------
void 
ob_player_spawn::Validate()
{
}

//-------------------------------------------------------------
bool
ob_player_spawn::Get_Coords(int& x, int& y)
{
	x = -1;
	y = -1;
	wxString t;
	t = GetToken( 0 );
	if( t == wxString() )
		return false;
	x = StrToInt( t );
	
	t = GetToken( 1 );
	if( t == wxString() )
		return false;
	y = StrToInt( t );
	
	return true;
}

//-------------------------------------------------------------
void 
ob_player_spawn::Coords_Decal( const wxSize& _decal )
{
	SetToken( 0, IntToStr( StrToInt(GetToken(0)) + _decal.x ));
	SetToken( 1, IntToStr( StrToInt(GetToken(1)) + _decal.x ));
}

//-------------------------------------------------------------
void 
ob_player_spawn::Init_and_Center( wxSize& _here )
{
	SetToken( 0, IntToStr( _here.x ));
	SetToken( 1, IntToStr( _here.y ));
}



//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a stage of a game
 */


int ob_stage_at::Get_StageObjectType( const wxString& _name )
{
	if( _name.Upper() == wxT("SCROLLZ") || _name.Upper() == wxT("SCROLLX") )
		return SOBJ_SCROLLZ;

	else if( _name.Upper() == wxT("GROUP") )
		return SOBJ_GROUP;

	else if( _name.Upper() == wxT("LIGHT") )
		return SOBJ_LIGHT;

	else if( _name.Upper() == wxT("SHADOWCOLOR") )
		return SOBJ_SHADOWCOLOR;

	else if( _name.Upper() == wxT("SHADOWALPHA") )
		return SOBJ_SHADOWALPHA;

	else if( _name.Upper() == wxT("SETPALETTE") )
		return SOBJ_SETPALETTE;

	else if( _name.Upper() == wxT("BLOCKADE") )
		return SOBJ_BLOCKADE;

	else if( _name.Upper() == wxT("WAIT") )
		return SOBJ_WAIT;

	else if( _name.Upper() == wxT("NOJOIN") )
		return SOBJ_NOJOIN;

	else if( _name.Upper() == wxT("CANJOIN") )
		return SOBJ_CANJOIN;

	else
		return STAGETYPE_NONE;
}

//**********************************************

ob_stage_at::ob_stage_at( MyLine* line, const int num_line )
:ob_stage_object( line, num_line )
{
	type = OB_TYPE_STAGE_AT;
	if( line != NULL )
		object_type =  Get_StageObjectType( *(line->GetToken(0)->data) );
	ungry = true;
}


//**********************************************

ob_stage_at::ob_stage_at(const wxString& _name, wxArrayString& _tokens)
:ob_stage_object( _name, _tokens )
{
	type = OB_TYPE_STAGE_AT;
	object_type =  Get_StageObjectType(  _name );
}


//-----------------------------------------------------------------
void
ob_stage_at::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_stage_at();
	ob_stage_object::pClone( res );
}

//**********************************************

ob_stage_at::~ob_stage_at()
{
}


//**********************************************

bool ob_stage_at::Eat(  MyLine* line, const int _num_line )
{
	if( ! ungry )
		return false;

	if( line->IsComment() || line->IsEmpty()  )
	{
		Add_SubObj( new ob_object( line, _num_line ) );
		return true;
	}


	wxString _name = *(line->GetToken(0)->data);

	// MUST BE followed by a  <AT> Tag, or there is a problem
	if( _name.Upper() != wxT("AT") )
	{
		ObjectsLog( MYLOG_ERROR, _num_line + 1,
			    wxT("The upper object <") + name + wxT("> require a tag <AT> which is not present !") );
		ungry = false;
		return false;
	}

	// Good, a <AT> tag close the thing !
	Add_SubObj( new ob_object( line, _num_line ) );
	ungry = false;
	return true;
}


//**********************************************
void 
ob_stage_at::Validate()
{
}

//---------------------------------------------------------
bool
ob_stage_at::Get_Coords(int& x, int& y)
{
	x = -1;
	y = -1;
	ob_object* o = GetSubObject( wxT("AT") );
	if( o == NULL )
		return false;
	
	wxString _tok = o->GetToken( 0 );
	if( _tok == wxString() )
		return false;
	
	x = StrToInt( _tok );
	y = 0;
	return true;
}

//-------------------------------------------------------------
int   
ob_stage_at::GetAt()
{
	ob_object* subobj = GetSubObject( wxT("at") );
	if( subobj == NULL )
		return 0;
	return StrToInt( subobj->GetToken(0 ));
}

//-------------------------------------------------------------
void  
ob_stage_at::SetAt(int _here)
{
	ob_object* subobj = GetSubObject( wxT("at") );
	if( subobj == NULL )
	{
		subobj = new ob_object(wxT("at"));
		Add_SubObj( subobj );
	}
	subobj->SetToken( 0, IntToStr(_here));
}

//-------------------------------------------------------------
void 
ob_stage_at::Coords_Decal( const wxSize& _decal )
{
	SetAt( GetAt() + _decal.x );
}

//-------------------------------------------------------------
void 
ob_stage_at::Init_and_Center( wxSize& _here )
{
	SetAt( _here.x );
}

//-------------------------------------------------------------
void 
ob_stage_at::Reorder_At()
{
	int at_val = GetAt();
	
	// Must move the AT object at his new right place
	ob_object* sibling = this->next;
	ob_object* last_obj;
	int found_res = 0;
	
	while( sibling != NULL )
	{
		if( ob_stage::Is_Stage_At_Type( sibling ) == false )
		{
			sibling = sibling->next;
			continue;
		}
		
		ob_object* subobj = sibling->GetSubObject( wxT("AT") );
		if( subobj != NULL )
		{
			if( StrToInt( subobj->GetToken(0)) > at_val )
				break;
			found_res = 1;
			last_obj = sibling;
		}
		sibling = sibling->next;
	}
	
	// If no better place upper  ==>> try lower
	if( sibling == this->next || (sibling == NULL && found_res == 0) )
	{
		sibling = this->prev;
		while( sibling != NULL )
		{
			if( ob_stage::Is_Stage_At_Type( sibling ) == false )
			{
				sibling = sibling->prev;
				continue;
			}
			
			ob_object* subobj = sibling->GetSubObject( wxT("AT") );
			if( subobj != NULL )
			{
				if( StrToInt( subobj->GetToken(0)) < at_val )
					break;
				found_res = 2;
				last_obj = sibling;
			}

			sibling = sibling->prev;
		}
		
		// no better Lower place neither
		if( sibling == this->prev || (sibling == NULL && found_res == 0) )
		{
			sibling = NULL;
		}
	}
	
	// Must replace the object next to the sibling found
	if( found_res > 0 )
	{
		// Front or back case
		if( sibling == NULL )
		{
			// front case  ==>> must be an insert before
			if( found_res == 2 )
				found_res = 1;
			// back case  ==>> must be an insert after
			else
				found_res = 2;
			sibling = last_obj;
		}

		// Must place it before
		if( found_res == 1 )
			sibling->InsertObject_Before( this );
		else
			sibling->InsertObject_After( this );
	}
}

//-------------------------------------------------------------
bool 
ob_stage_at::Add_SubObj( ob_object* temp )
{
	if( temp == NULL )
		return false;
	if( temp->name.Upper() == wxT("AT") )
		return ob_stage_object::Add_SubObj( temp );
	
	if( last_subobj == NULL )
		return ob_stage_object::Add_SubObj( temp );
	
	if( last_subobj->name.Upper() == wxT("AT") )
		return last_subobj->InsertObject_Before( temp );
	
	return ob_stage_object::Add_SubObj( temp );
}


//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a game (mod)
 */

int ob_spawn::Get_StageObjectType( const wxString& _name, const wxString& entity_name )
{
	if( _name.Upper() != wxT("SPAWN") )
		return STAGETYPE_NONE;

	obFileEntity* ent = obFileStage::GetEntity( entity_name );
	if( ent == NULL )
		return STAGETYPE_NONE;
	wxString ent_type = ent->obj_container->GetSubObject_Token( wxT("type") );

	if( ent_type.Upper() == wxT("NONE") )
		return SOBJ_NONE;

	if( ent_type.Upper() == wxT("STEAMER") )
		return SOBJ_STEAMER;

	if( ent_type.Upper() == wxT("PANEL") )
		return SOBJ_PANEL;

	if( ent_type.Upper() == wxT("TEXT") )
		return SOBJ_TEXT;

	if( ent_type.Upper() == wxT("TRAP") )
		return SOBJ_TRAP;

	if( ent_type.Upper() == wxT("PSHOT") || ent_type.Upper() == wxT("SHOT"))
		return SOBJ_SHOT;

	else
		return STAGETYPE_NONE;
}

//**********************************************

wxString obSpawnTags[] = 
	{
		wxT("at"),
		wxT("2pspawn"),
		wxT("3pspawn"),
		wxT("4pspawn"),
		wxT("flip"),
		wxT("alias"),
		wxT("map"),
		wxT("spawnscript"),
		wxT("coords"),
	 };

bool 
ob_spawn::Is_AcceptedTag( const wxString& _name )
{
	return IsInArray( _name, (wxString*) &obSpawnTags, t_size_of( obSpawnTags ) );
}

//------------------------------------------------------------------------------
int
ob_spawn::Get_Entity_StageType( const wxString& ent_name )
{
	int ent_stype = ob_spawn::Get_StageObjectType( wxT("SPAWN"), ent_name );

	if( ent_stype == STAGETYPE_NONE )
		ent_stype = ob_spawn_healthy::Get_StageObjectType( wxT("SPAWN"), ent_name );

	if( ent_stype == STAGETYPE_NONE )
		ent_stype = ob_spawn_item::Get_StageObjectType( wxT("SPAWN"), ent_name );
	
	return ent_stype;
}

//**********************************************

ob_spawn::ob_spawn( MyLine* line, const int num_line )
:ob_stage_at( line, num_line )
{
	type = OB_TYPE_SPAWN;
	if(  line == NULL || line->GetTokensCount() < 2 )
	{
		object_type = STAGETYPE_NONE;
		entity_ref = NULL;
	}
	else
	{
		object_type =  Get_StageObjectType( *(line->GetToken(0)->data), *(line->GetToken(1)->data) );
		entity_ref = obFileStage::GetEntity( *(line->GetToken(1)->data));
	}
}


//**********************************************

ob_spawn::ob_spawn(const wxString& _name, wxArrayString& _tokens)
:ob_stage_at( _name, _tokens )
{
	type = OB_TYPE_SPAWN;
	if(  _tokens.Count() < 1 )
	{
		object_type = STAGETYPE_NONE;
		entity_ref = NULL;
	}
	else
	{
		object_type =  Get_StageObjectType(  _name , _tokens[0] );
		entity_ref = obFileStage::GetEntity(_tokens[0] );
	}
}


//-----------------------------------------------------------------
void
ob_spawn::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_spawn();
	ob_stage_at::pClone( res );
	((ob_spawn*)res)->entity_ref = entity_ref;
}

//**********************************************

ob_spawn::~ob_spawn()
{
}


//**********************************************

bool ob_spawn::Eat(  MyLine* line, const int _num_line )
{
	if( ! ungry )
		return false;

	if( line->IsComment() || line->IsEmpty()  )
	{
		Add_SubObj( new ob_object( line, _num_line ) );
		return true;
	}


	wxString _name = *(line->GetToken(0)->data);

	// Accepted !!
	if( Is_AcceptedTag( _name ) )
	{
		wxString _name = *(line->GetToken(0)->data);
		if( GetSubObject( _name ) != NULL )
			ObjectsLog( MYLOG_ERROR, _num_line + 1,
				    wxT("The object <") + name +wxT( "> contain two <") + _name +wxT("> tag !") );
			
		Add_SubObj( new ob_object( line, _num_line ) );
		
		// An <AT> close the stuff
		if( _name.Upper() == wxT("AT") )
		{
			ungry = false;
			Validate();
		}
		return true;
	}

	// Not a tag for it anymore
	else
	{
		ungry = false;
		Validate();
		return false;
	}
}


//-----------------------------------------------------------------
void 
ob_spawn::Validate()
{
	if( GetSubObject( wxT("AT") ) == NULL )
	{
		ObjectsLog( MYLOG_ERROR, num_line + 1,
			    wxT("The object <") + name + wxT("> require a tag <AT> which is not present !" ));
	}

	if( GetSubObject( wxT("COORDS") ) == NULL )
	{
		ObjectsLog( MYLOG_ERROR, num_line + 1,
			    wxT("The object <") + name + wxT("> require a tag <COORDS> which is not present !") );
	}
}

//-----------------------------------------------------------------
wxString 
ob_spawn::Get_ScreenList_Name()
{
	if( entity_ref == NULL )
		return wxT("NO Entity");
	return entity_ref->Name();
};

//-------------------------------------------------------------
bool
ob_spawn::Get_Coords(int& x, int& y)
{
	if( ob_stage_at::Get_Coords(x,y) == false )
		return false;
	y = -1;

	ob_object* o = GetSubObject(wxT( "coords" ));
	if( o == NULL )
		return false;
	wxString _tok = o->GetToken( 0 );
	if( _tok == wxString() )
		return false;
	x += StrToInt( _tok );

	_tok = o->GetToken( 1 );
	if( _tok == wxString() )
		return false;

	y = StrToInt( _tok );
	return true;
}

//-------------------------------------------------------------
void 
ob_spawn::Coords_Decal( const wxSize& _decal )
{
	SetAt( GetAt() + _decal.x );
	ob_object* coords = GetSubObject(wxT( "coords" ));
	if( coords == NULL )
	{
		coords = new ob_object(wxT("coords"));
		coords->SetToken(0,wxT("0"));
		Add_SubObj( coords );
	}
	coords->SetToken( 1, IntToStr(StrToInt(coords->GetToken(1)) + _decal.y) );
}

//-------------------------------------------------------------
void 
ob_spawn::Init_and_Center( wxSize& _here )
{
	ob_stage_at::Init_and_Center( _here );
	ob_object* coords = GetSubObject( wxT("coords") );
	if( coords == NULL )
	{
		coords = new ob_object(wxT("coords"));
		coords->SetToken(0,wxT("0"));
		Add_SubObj( coords );
	}
	coords->SetToken( 1, IntToStr(_here.y) );
}



//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a game (mod)
 */

int ob_spawn_healthy::Get_StageObjectType( const wxString& _name,  const wxString& entity_name )
{
	if( _name.Upper() != wxT("SPAWN") )
		return STAGETYPE_NONE;

	obFileEntity* ent = obFileStage::GetEntity( entity_name );
	if( ent == NULL )
		return STAGETYPE_NONE;
	wxString ent_type = ent->obj_container->GetSubObject_Token( wxT("type") );

	if( ent_type.Upper() == wxT("OBSTACLE") )
		return SOBJ_OBSTACLE;

	if( ent_type.Upper() == wxT("NPC") )
		return SOBJ_NPC;

	if( ent_type.Upper() == wxT("ENEMY") )
		return SOBJ_ENEMY;
	
	return STAGETYPE_NONE;
}

//**********************************************

wxString obSpawnHealthyTags[] = 
	{ 
		wxT("boss"),
		wxT("health"),
		wxT("2phealth"),
		wxT("3phealth"),
		wxT("4phealth"),
		wxT("score"),
		wxT("nolife"),
		wxT("dying"),

		wxT("weapon"),
		wxT("aggression"),

		wxT("item"),
		wxT("credit"),
		wxT("2pitem"),
		wxT("3pitem"),
		wxT("4pitem"),
		wxT("itemmap"),
		wxT("itemhealth"),
		wxT("itemalias"),
		wxT("itemtrans"),
		wxT("itemalpha"),
		wxT("itemtrans"),
	};

bool ob_spawn_healthy::Is_AcceptedTag( const wxString& _name )
{
	return IsInArray( _name, (wxString*) &obSpawnHealthyTags, t_size_of( obSpawnHealthyTags ) )
		|| ob_spawn::Is_AcceptedTag( _name );
}

//----------------------------------------------------------------------
wxString obItemHoldedTags[] = 
	{
		wxT("item"),
		wxT("credit"),
		wxT("2pitem"),
		wxT("3pitem"),
		wxT("4pitem"),
		wxT("itemmap"),
		wxT("itemhealth"),
		wxT("itemalias"),
		wxT("itemtrans"),
		wxT("itemalpha"),
		wxT("itemtrans"),
	};

bool 
ob_spawn_healthy::IsItemHoldedTag( const wxString& _tag )
{
	return IsInArray( _tag, (wxString*) &obItemHoldedTags, t_size_of( obItemHoldedTags ) );
}

//**********************************************

ob_spawn_healthy::ob_spawn_healthy( MyLine* line, const int num_line )
:ob_spawn( line, num_line )
{
	type = OB_TYPE_SPAWN_HEALTHY;
	if( line != NULL )
		object_type =  Get_StageObjectType( *(line->GetToken(0)->data), *(line->GetToken(1)->data) );
}


//**********************************************

ob_spawn_healthy::ob_spawn_healthy(const wxString& _name, wxArrayString& _tokens)
:ob_spawn( _name, _tokens )
{
	type = OB_TYPE_SPAWN_HEALTHY;
	object_type =  Get_StageObjectType( _name, _tokens[0] );
}


//-----------------------------------------------------------------
void
ob_spawn_healthy::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_spawn_healthy();
	ob_spawn::pClone( res );
}

//**********************************************

ob_spawn_healthy::~ob_spawn_healthy()
{
}


//**********************************************

void ob_spawn_healthy::Validate()
{
	ob_spawn::Validate();
}



//**********************************************
//**********************************************
//**********************************************
/**
 * An item spawned (not dropped)
 */

int ob_spawn_item::Get_StageObjectType( const wxString& _name,  const wxString& entity_name )
{
	if( _name.Upper() != wxT("SPAWN" ))
		return STAGETYPE_NONE;

	obFileEntity* ent = obFileStage::GetEntity( entity_name );
	if( ent == NULL )
		return STAGETYPE_NONE;
	wxString ent_type = ent->obj_container->GetSubObject_Token( wxT("type" ));

	if( ent_type.Upper() == wxT("ITEM") )
		return SOBJ_ITEM;

	if( ent_type.Upper() == wxT("ENDLEVEL") )
		return SOBJ_ENDLEVEL;
	
	return STAGETYPE_NONE;
}

//**********************************************
wxString obSpawnItemTags[] = 
  { 
	wxT("mp"), 
	wxT("credit"), 
  };

bool ob_spawn_item::Is_AcceptedTag(const wxString& _name )
{
	return IsInArray( _name, (wxString*) &obSpawnItemTags, t_size_of( obSpawnItemTags ) )
		|| ob_spawn::Is_AcceptedTag( _name );
}



//**********************************************

ob_spawn_item::ob_spawn_item( MyLine* line, const int num_line )
:ob_spawn( line, num_line )
{
	type = OB_TYPE_SPAWN_ITEM;
	if( line != NULL )
		object_type =  Get_StageObjectType( *(line->GetToken(0)->data), *(line->GetToken(1)->data) );
}


//**********************************************

ob_spawn_item::ob_spawn_item(const wxString& _name, wxArrayString& _tokens)
:ob_spawn( _name, _tokens )
{
	type = OB_TYPE_SPAWN_ITEM;
	object_type =  Get_StageObjectType( _name, _tokens[0] );
}


//-----------------------------------------------------------------
void
ob_spawn_item::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_spawn_item();
	ob_spawn::pClone( res );
}

//**********************************************

ob_spawn_item::~ob_spawn_item()
{
}


//**********************************************

void ob_spawn_item::Validate()
{
	ob_spawn::Validate();
}


/**======================================================================
 * Types and Super Types helpers
 */

int  list_SST_OTHER__count;
int *list_SST_OTHER;
int  list_SST_GEOMETRY__count;
int *list_SST_GEOMETRY;
int  list_SST_CONTROL__count;
int *list_SST_CONTROL;
int  list_SST_VISUAL_FX__count;
int *list_SST_VISUAL_FX;
int  list_SST_STUFFS__count;
int *list_SST_STUFFS;
int  list_SST_DECORATION__count;
int *list_SST_DECORATION;
int  list_SST_LIVINGS__count;
int *list_SST_LIVINGS;

map<int,int> assoc_type_supertype;
void
Init_SuperTypes()
{
	if( assoc_type_supertype.size() > 0 )
		return;
	
	list_SST_OTHER__count = 0;
	list_SST_GEOMETRY__count = 0;
	list_SST_CONTROL__count = 0;
	list_SST_VISUAL_FX__count = 0;
	list_SST_STUFFS__count = 0;
	list_SST_DECORATION__count = 0;
	list_SST_LIVINGS__count = 0;
	
	assoc_type_supertype[SOBJ_TEXT] 		= SST_OTHER;
	assoc_type_supertype[SOBJ_PLAYER_SPAWN] 	= SST_OTHER;
	assoc_type_supertype[SOBJ_SETPALETTE] 	= SST_OTHER;
	

	assoc_type_supertype[SOBJ_SCROLLZ]	= SST_GEOMETRY;
	assoc_type_supertype[SOBJ_WALL] 	= SST_GEOMETRY;
	assoc_type_supertype[SOBJ_HOLE] 	= SST_GEOMETRY;

	assoc_type_supertype[SOBJ_WAIT] 	= SST_CONTROL;
	assoc_type_supertype[SOBJ_NOJOIN] 	= SST_CONTROL;
	assoc_type_supertype[SOBJ_CANJOIN] 	= SST_CONTROL;
	assoc_type_supertype[SOBJ_BLOCKADE] = SST_CONTROL;
	assoc_type_supertype[SOBJ_GROUP] 	= SST_CONTROL;

	assoc_type_supertype[SOBJ_SHADOWCOLOR] 	= SST_VISUAL_FX;
	assoc_type_supertype[SOBJ_SHADOWALPHA] 	= SST_VISUAL_FX;
	assoc_type_supertype[SOBJ_LIGHT] 		= SST_VISUAL_FX;
			
	assoc_type_supertype[SOBJ_TRAP] 	= SST_STUFFS;
	assoc_type_supertype[SOBJ_SHOT] 	= SST_STUFFS;
	assoc_type_supertype[SOBJ_ITEM] 	= SST_STUFFS;
	assoc_type_supertype[SOBJ_ENDLEVEL] = SST_STUFFS;
	assoc_type_supertype[SOBJ_OBSTACLE] = SST_STUFFS;
			
	assoc_type_supertype[SOBJ_STEAMER] 	= SST_DECORATION;
	assoc_type_supertype[SOBJ_NONE] 	= SST_DECORATION;
	assoc_type_supertype[SOBJ_PANEL] 	= SST_DECORATION;
	
	assoc_type_supertype[SOBJ_NPC] 	= SST_LIVINGS;
	assoc_type_supertype[SOBJ_ENEMY] 	= SST_LIVINGS;
	
	// fill the Members Arrays
	map<int,int>:: iterator it(assoc_type_supertype.begin())
				, it_end(assoc_type_supertype.end());

	
	for(;it!=it_end;it++)
	{
		switch( it->second )
		{
			case SST_OTHER:
				list_SST_OTHER__count++;
				break;
			case SST_GEOMETRY:
				list_SST_GEOMETRY__count++;
				break;
			case SST_CONTROL:
				list_SST_CONTROL__count++;
				break;
			case SST_VISUAL_FX:
				list_SST_VISUAL_FX__count++;
				break;
			case SST_STUFFS:
				list_SST_STUFFS__count++;
				break;
			case SST_DECORATION:
				list_SST_DECORATION__count++;
				break;
			case SST_LIVINGS:
				list_SST_LIVINGS__count++;
				break;
		}
	}
	
	if( list_SST_OTHER__count > 0 )
		list_SST_OTHER = new int[list_SST_OTHER__count];
	else
		list_SST_OTHER = NULL;
	list_SST_OTHER__count=0;
		
	if( list_SST_GEOMETRY__count > 0 )
		list_SST_GEOMETRY = new int[list_SST_GEOMETRY__count];
	else
		list_SST_GEOMETRY = NULL;
	list_SST_GEOMETRY__count=0;

	if( list_SST_CONTROL__count > 0 )
		list_SST_CONTROL = new int[list_SST_CONTROL__count];
	else
		list_SST_CONTROL = NULL;
	list_SST_CONTROL__count=0;

	if( list_SST_VISUAL_FX__count > 0 )
		list_SST_VISUAL_FX = new int[list_SST_VISUAL_FX__count];
	else
		list_SST_VISUAL_FX = NULL;
	list_SST_VISUAL_FX__count=0;

	if( list_SST_STUFFS__count > 0 )
		list_SST_STUFFS = new int[list_SST_STUFFS__count];
	else
		list_SST_STUFFS = NULL;
	list_SST_STUFFS__count=0;

	if( list_SST_DECORATION__count > 0 )
		list_SST_DECORATION = new int[list_SST_DECORATION__count];
	else
		list_SST_DECORATION = NULL;
	list_SST_DECORATION__count=0;

	if( list_SST_LIVINGS__count > 0 )
		list_SST_LIVINGS = new int[list_SST_LIVINGS__count];
	else
		list_SST_LIVINGS = NULL;
	list_SST_LIVINGS__count=0;

	it = assoc_type_supertype.begin();
	for(;it!=it_end;it++)
	{
		switch( it->second )
		{
			case SST_OTHER:
				list_SST_OTHER[list_SST_OTHER__count] = it->first;
				list_SST_OTHER__count++;
				break;
			case SST_GEOMETRY:
				list_SST_GEOMETRY[list_SST_GEOMETRY__count] = it->first;
				list_SST_GEOMETRY__count++;
				break;
			case SST_CONTROL:
				list_SST_CONTROL[list_SST_CONTROL__count] = it->first;
				list_SST_CONTROL__count++;
				break;
			case SST_VISUAL_FX:
				list_SST_VISUAL_FX[list_SST_VISUAL_FX__count] = it->first;
				list_SST_VISUAL_FX__count++;
				break;
			case SST_STUFFS:
				list_SST_STUFFS[list_SST_STUFFS__count] = it->first;
				list_SST_STUFFS__count++;
				break;
			case SST_DECORATION:
				list_SST_DECORATION[list_SST_DECORATION__count] = it->first;
				list_SST_DECORATION__count++;
				break;
			case SST_LIVINGS:
				list_SST_LIVINGS[list_SST_LIVINGS__count] = it->first;
				list_SST_LIVINGS__count++;
				break;
		}
	}
	
}


//------------------------------------------------------------------
wxString	
Stage__SuperType_ToStr( const int _sst )
{
	switch( _sst )
	{
		case SST_OTHER:
			return wxT("Other");
		case SST_GEOMETRY:
			return wxT("Geometry");
		case SST_CONTROL:
			return wxT("Control");
		case SST_VISUAL_FX:
			return wxT("Visual Fx");
		case SST_STUFFS:
			return wxT("Stuff");
		case SST_DECORATION:
			return wxT("Decoration");
		case SST_LIVINGS:
			return wxT("Living");
		default:
			return wxT("NONE");
	}
}

//------------------------------------------------------------------
int	
Stage__SuperType_ToInt( const wxString& _sst )
{
	if( _sst == wxT("Other" ))
		return SST_OTHER;
	if( _sst == wxT("Geometry" ))
		return SST_GEOMETRY;
	if( _sst == wxT("Control" ))
		return SST_CONTROL;
	if( _sst == wxT("Visual Fx" ))
		return SST_VISUAL_FX;
	if( _sst == wxT("Stuff" ))
		return SST_STUFFS;
	if( _sst == wxT("Decoration" ))
		return SST_DECORATION;
	if( _sst == wxT("Living" ))
		return SST_LIVINGS;
	return SST_NONE;
}

//------------------------------------------------------------------
const int*
Stage__SuperType_Get_Type_Array( const int _sst, size_t& arr_size )
{
	Init_SuperTypes();
	
	if( _sst == SST_OTHER )
	{
		arr_size = list_SST_OTHER__count;
		return list_SST_OTHER;
	}
	if( _sst == SST_GEOMETRY )
	{
		arr_size = list_SST_GEOMETRY__count;
		return list_SST_GEOMETRY;
	}
	if( _sst == SST_CONTROL )
	{
		arr_size = list_SST_CONTROL__count;
		return list_SST_CONTROL;
	}
	if( _sst == SST_VISUAL_FX )
	{
		arr_size = list_SST_VISUAL_FX__count;
		return list_SST_VISUAL_FX;
	}
	if( _sst == SST_STUFFS )
	{
		arr_size = list_SST_STUFFS__count;
		return list_SST_STUFFS;
	}
	if( _sst == SST_DECORATION )
	{
		arr_size = list_SST_DECORATION__count;
		return list_SST_DECORATION;
	}
	if( _sst == SST_LIVINGS )
	{
		arr_size = list_SST_LIVINGS__count;
		return list_SST_LIVINGS;
	}
	
	arr_size = 0;
	return NULL;
};

//------------------------------------------------------------------
int
Stage__Type__Get_SuperType( const int _sobj_type )
{
	Init_SuperTypes();
	
	map<int,int>::iterator it(assoc_type_supertype.find(_sobj_type));
	if( it == assoc_type_supertype.end() )
		return SST_NONE;
	
	return it->second;
}

//------------------------------------------------------------------
bool	
Stage__Type_Is_in_SuperType( const int _stype, const int _sst )
{
	return (Stage__Type__Get_SuperType( _stype ) == _sst);
}

//------------------------------------------------------------------
int		
Stage__Type_ToInt( const wxString& _sst )
{
	if( _sst == wxT("Text") )
		return SOBJ_TEXT;
	if( _sst == wxT("EndLevel" ))
		return SOBJ_ENDLEVEL;
	if( _sst == wxT("Player Spawn" ))
		return SOBJ_PLAYER_SPAWN;
	if( _sst == wxT("SetPalette" ))
		return SOBJ_SETPALETTE;
	if( _sst == wxT("Wall" ))
		return SOBJ_WALL;
	if( _sst == wxT("Hole" ))
		return SOBJ_HOLE;
	if( _sst == wxT("Wait" ))
		return SOBJ_WAIT;
	if( _sst == wxT("NoJoin" ))
		return SOBJ_NOJOIN;
	if( _sst == wxT("CanJoin" ))
		return SOBJ_CANJOIN;
	if( _sst == wxT("Scroll[zx]" ))
		return SOBJ_SCROLLZ;
	if( _sst == wxT("Blockade" ))
		return SOBJ_BLOCKADE;
	if( _sst == wxT("Group" ))
		return SOBJ_GROUP;
	if( _sst == wxT("ShadowColor" ))
		return SOBJ_SHADOWCOLOR;
	if( _sst == wxT("ShadowAlpha" ))
		return SOBJ_SHADOWALPHA;
	if( _sst == wxT("Light" ))
		return SOBJ_LIGHT;
	if( _sst == wxT("Trap" ))
		return SOBJ_TRAP;
	if( _sst == wxT("Shot" ))
		return SOBJ_SHOT;
	if( _sst == wxT("Item" ))
		return SOBJ_ITEM;
	if( _sst == wxT("Obstacle" ))
		return SOBJ_OBSTACLE;
	if( _sst == wxT("Steamer" ))
		return SOBJ_STEAMER;
	if( _sst == wxT("none" ))
		return SOBJ_NONE;
	if( _sst == wxT("Panel" ))
		return SOBJ_PANEL;
	if( _sst == wxT("Npc" ))
		return SOBJ_NPC;
	if( _sst == wxT("Enemy" ))
		return SOBJ_ENEMY;
	return STAGETYPE_NONE;
}

//------------------------------------------------------------------
wxString	
Stage__Type_ToStr( const int _sst )
{
	switch( _sst )
	{
		case SOBJ_TEXT:
			return wxT("Text");
		case SOBJ_ENDLEVEL:
			return wxT("EndLevel");
		case SOBJ_PLAYER_SPAWN:
			return wxT("Player Spawn");
		case SOBJ_SETPALETTE:
			return wxT("SetPalette");
		case SOBJ_WALL:
			return wxT("Wall");
		case SOBJ_HOLE:
			return wxT("Hole");
		case SOBJ_WAIT:
			return wxT("Wait");
		case SOBJ_NOJOIN:
			return wxT("NoJoin");
		case SOBJ_CANJOIN:
			return wxT("CanJoin");
		case SOBJ_SCROLLZ:
			return wxT("Scroll[zx]");
		case SOBJ_BLOCKADE:
			return wxT("Blockade");
		case SOBJ_GROUP:
			return wxT("Group");
		case SOBJ_SHADOWCOLOR:
			return wxT("ShadowColor");
		case SOBJ_SHADOWALPHA:
			return wxT("ShadowAlpha");
		case SOBJ_LIGHT:
			return wxT("Light");
		case SOBJ_TRAP:
			return wxT("Trap");
		case SOBJ_SHOT:
			return wxT("Shot");
		case SOBJ_ITEM:
			return wxT("Item");
		case SOBJ_OBSTACLE:
			return wxT("Obstacle");
		case SOBJ_STEAMER:
			return wxT("Steamer");
		case SOBJ_NONE:
			return wxT("none");
		case SOBJ_PANEL:
			return wxT("Panel");
		case SOBJ_NPC:
			return wxT("Npc");
		case SOBJ_ENEMY:
			return wxT("Enemy");
		default:
			return wxT("NoType");
	}
}

