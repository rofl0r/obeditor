/*
 * common__object_levels.cpp
 *
 *  Created on: 24 avr. 2009
 *      Author: pat
 */

#include "common__tools.h"
#include "common__object_stage.h"


/*\internal*********************************************
********	OBJECT RELATED TO THE LEVELS.TXT FILE
******************************************************/



//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a game (mod)
 */


//**********************************************
ob_object* 
ob_levels::Guess_and_ConstructNewObject(MyLine* line, const int _num_line )
{
	// if Empty Line
	if( line->GetTokensCount() <= 0 )
		return new ob_object(line, _num_line);

	wxString _name = *(line->GetToken(0)->data);
	ob_object *temp;
	if( _name.Upper() == wxT("SET") )
		temp = new ob_StagesSet(line,_num_line);
	else
		temp = new ob_object( line, _num_line);

//	wxLogInfo( "At line " + IntToStr( _num_line +1) + ", object build :" + temp->name );

	return temp;
}


//**********************************************
ob_levels::ob_levels()
: ob_object_container()
{
	SetType( OB_TYPE_LEVELS );
}


//-----------------------------------------------------------------
void
ob_levels::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_levels();
	ob_object_container::pClone( res );
}

//**********************************************
ob_levels::~ob_levels()
{
}


//**********************************************

ob_StagesSet**	 ob_levels::GetSets( size_t& _nb_set)
{
	return ((ob_StagesSet**) GetSubObjectS_ofType( OB_TYPE_STAGES_SET, _nb_set ));
}


//**********************************************

bool ob_levels::Set_Insert( ob_StagesSet* ta, int ind )
{
	size_t _nb_set;
	ob_StagesSet**	t = ob_levels::GetSets( _nb_set);

	// No set
	if( t == NULL )
	{
		return Add_SubObj( ta );
	}

	bool res;
	if( ind == -1 || ind == (int)_nb_set )
	{
		res = t[_nb_set-1]->InsertObject_After( ta );
	}
	else if( ind >= 0 && ind < (int) _nb_set )
	{
		res = t[ind]->InsertObject_Before( ta );
	}
	else
		res = false;

	delete[] t;
	return res;
}


//**********************************************

bool ob_levels::Set_Tag(const wxString& _tag, const wxString& _token )
{
	wxArrayString t;
	t.Add( _token );
	return Set_Tag( _tag, t );
}

//**********************************************

bool ob_levels::Set_Tag(const wxString& _tag, wxArrayString _tokens )
{
	// If the tag is already here
	ob_object* temp = GetSubObject( _tag );
	if( temp != NULL )
	{
		temp->SetProperty( _tag, _tokens );
		return true;
	}

	//-----------------
	ob_object* t = new ob_object( _tag, _tokens );

	// Get the first set object
	temp = first_subobj;
	while( temp != NULL )
	{
		if( temp->type == OB_TYPE_STAGES_SET)
			break;
	}

	// No Set -> Append the new tag at the end
	if( temp == NULL )
		return Add_SubObj( t );

	// Insert the new tag just before the first set
	else
		return temp->InsertObject_Before( t );
}


//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a game (mod)
 */

wxString stagedeclaration_tag[] = { wxT("Z"), wxT("FILE"), wxT("BRANCH"),  };

bool IsStageDeclarationTag( wxString& t )
{
	for( size_t i =0; i< t_size_of(stagedeclaration_tag); i++ )
		if( stagedeclaration_tag[i].Upper() == t.Upper() )
			return true;
	return false;
}

//**********************************************
ob_StagesSet::ob_StagesSet( MyLine* line, const int _num_line )
: ob_object( line, _num_line )
{
	SetType( OB_TYPE_STAGES_SET );
	ungry = true;
}


//-----------------------------------------------------------------
void
ob_StagesSet::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_StagesSet();
	ob_object::pClone( res );
	res->ungry = ungry;
}

//**********************************************

bool ob_StagesSet::Eat(  MyLine* line, const int _num_line )
{
	if( ! ungry )
		return false;

	// If new set
	if( ! line->IsComment() && ! line->IsEmpty()  )
		if( line->GetToken(0)->data->Upper() == wxT("SET")  )
		{
			if(     last_subobj->type == OB_TYPE_STAGE_DECLARATION
				&&  ! ((ob_StageDeclaration*) last_subobj )->b_completed )
			{
				wxString __t;
				if( line->GetToken(1) != NULL && line->GetToken(1)->data != NULL )
					__t =  *(line->GetToken(1)->data);
				ObjectsLog( MYLOG_WARNING, 1 + _num_line,
					    wxT("new SET of Stage <") + __t + wxT("> declared although a stage declaration is still uncomplete.") );
			}
			ungry = false;
			return false;
		}

	// A stage declaration is in progress
	if( last_subobj != NULL && last_subobj->type == OB_TYPE_STAGE_DECLARATION && last_subobj->ungry )
	{
		ob_StageDeclaration* st = (ob_StageDeclaration*) last_subobj;
		bool res = st->Eat( line, _num_line );

		// ERROR CASE : Stage declaration don't eat the line BUT it had to (cause declaration are close with explicit tag "file" )
		if( !res )
		{
			wxString __t;
			if( line->GetToken(1) != NULL && line->GetToken(1)->data != NULL )
				__t ==  *(line->GetToken(1)->data);
			ObjectsLog( MYLOG_WARNING, _num_line + 1,
				    wxT("Tag <") + __t + wxT("> pop in a middle of a stage declaration !!") );
		}
		else
			return true;
	}

	// comment line
	if( line->IsComment() || line->IsEmpty() )
	{
		ob_object* temp = new ob_object( line, _num_line );
		Add_SubObj( temp );
		// Still hungry
		return true;
	}

	//*************************
	// Valid line
	wxString _name = *(line->GetToken(0)->data);


	//-------------------------
	// Annoying "next" tag case :(
	if( _name.Upper() == wxT("NEXT") )
	{
		// Gotta merge all from here up to the previous stage_declaration
		ob_object* obj_next = new ob_object( line, _num_line );
		ob_object* temp = last_subobj;
		while( temp != NULL && temp->type != OB_TYPE_STAGE_DECLARATION && temp->type != OB_TYPE_STAGES_SET )
			temp = temp->prev;

		// Problem, having a "next" tag without previous Stage declaration ...
		// No merging will be done
		if( temp == NULL || temp->type != OB_TYPE_STAGE_DECLARATION )
		{
			ObjectsLog( MYLOG_WARNING, _num_line + 1,wxT("Tag <NEXT> found without previous stage declaration !!!") );
			Add_SubObj( obj_next );
		}

		// Normal case, do the merging
		else
		{
			ob_StageDeclaration* sd = (ob_StageDeclaration*) temp;
			temp = sd->next;
			while( temp != NULL )
			{
				sd->Add_SubObj( temp );
				temp = sd->next;
			}

			// Append also the "NEXT" tag
			sd->Add_SubObj( obj_next  );
			return true;
		}
	}

	//-------------------------
	// Stage declaration start here
	if( IsStageDeclarationTag( _name ) )
	{
		Add_SubObj( new ob_StageDeclaration( line, _num_line ) );
		return true;
	}

	// Basic object here
	else
	{
		Add_SubObj( new ob_object( line, _num_line ) );
		return true;
	}
}


//**********************************************

ob_StagesSet::~ob_StagesSet()
{
}


//**********************************************

void ob_StagesSet::Validate()
{
}


//**********************************************

bool ob_StagesSet::Add_SubObj( ob_object* _obj )
{
	// Add the next at the end
	if( _obj->type == OB_TYPE_STAGE_DECLARATION )
	{
		return Append_SubObj( _obj );
	}

	// Add the obj just before the first stage declaration
	else if( ! _obj->Is_EmptyOrComment() )
	{
		size_t dummy;
		ob_object** arr_o = GetSubObjectS_ofType( OB_TYPE_STAGE_DECLARATION, dummy );
		if( arr_o == NULL )
			return Append_SubObj( _obj );
		else
		{
			bool res = arr_o[0]->InsertObject_Before( _obj );
			delete[] arr_o;
			return res;
		}
	}
	else
		return Append_SubObj( _obj );
}

//**********************************************
bool ob_StagesSet::SetSetName(const wxString& _sn)
{
	return SetToken( 0, _sn );
}


//**********************************************

wxString ob_StagesSet::GetSetName()
{
	return GetToken( 0 );
}


//**********************************************

ob_StageDeclaration**	 ob_StagesSet::GetStages( size_t& _nb_stages)
{
	return (ob_StageDeclaration**) GetSubObjectS_ofType( OB_TYPE_STAGE_DECLARATION, _nb_stages );
}


//**********************************************

bool ob_StagesSet::Stage_Insert( ob_StageDeclaration* t, int ind )
{
	size_t nb_stages;
	bool res = false;
	ob_StageDeclaration**	arr_stages = GetStages( nb_stages );

	// case : there's already some stages in
	if( arr_stages != NULL )
	{
		// Append case
		if( ind == -1 || ind == (int) nb_stages )
		{
			res = arr_stages[nb_stages-1]->InsertObject_After( t );
		}

		// Valid indice case
		else if( ind >= 0 && ind < (int) nb_stages )
		{
			res = arr_stages[ind]->InsertObject_Before( t );
		}

		// INValid indice case
		else
		{
			res = false;
		}
		// Cleaning
		delete[] arr_stages;
	}

	// First stage to add -> Juste append it at the end of the object
	else
	{
		res = Add_SubObj( t );
	}
	return res;
}


//**********************************************

bool ob_StagesSet::Set_Tag(const wxString& _tag, const wxString& _token )
{
	wxArrayString t;
	t.Add( _token );
	return Set_Tag( _tag, t );
}

//**********************************************

bool ob_StagesSet::Set_Tag(const wxString& _tag, wxArrayString _tokens )
{
	// If the tag is already here
	ob_object* temp = GetSubObject( _tag );
	if( temp != NULL )
	{
		temp->SetProperty( _tag, _tokens );
		return true;
	}

	//-----------------
	ob_object* t = new ob_object( _tag, _tokens );

	// Get the first stage object
	temp = first_subobj;
	while( temp != NULL )
	{
		if( temp->type == OB_TYPE_STAGE_DECLARATION)
			break;
	}

	// No Stage -> Append the new tag at the end
	if( temp == NULL )
		return Add_SubObj( t );

	// Insert the new tag just before the first stage
	else
		return temp->InsertObject_Before( t );
}


//**********************************************

void ob_StagesSet::SetToDefault()
{
	name = wxT("SET");
	SetToken( 0, wxT("UNAMED" ));
	Add_SubObj( new ob_object(wxT("nosame"), wxT("1" )) );
	Add_SubObj( new ob_object(wxT("noshare"), wxT("0" )) );
	Add_SubObj( new ob_object(wxT("ifcomplete"), wxT("0") ) );
	Add_SubObj( new ob_object(wxT("lives"), wxT("3") ) );
	Add_SubObj( new ob_object(wxT("credits"), wxT("3") ) );
	Add_SubObj( new ob_object(wxT("maxplayers"), wxT("2") ) );
	Add_SubObj( new ob_object(wxT("typemp"), wxT("0") ) );
	Add_SubObj( new ob_object(wxT("cansave"), wxT("0") ) );
}


//**********************************************
//**********************************************
//**********************************************
/**
 * The object which hold the datas about a game (mod)
 */

//**********************************************
ob_StageDeclaration::ob_StageDeclaration( MyLine* line, const int _num_line )
: ob_object()
{
	SetType( OB_TYPE_STAGE_DECLARATION );
	b_completed = false;
	stage = NULL;

	// if default constructor => Nothing more
	if( line == NULL )
		return;

	Eat( line, _num_line );
}


//**********************************************

ob_StageDeclaration::ob_StageDeclaration( const wxString& _name, wxArrayString& _tokens)
: ob_object()
{
	SetType( OB_TYPE_STAGE_DECLARATION );
	b_completed = false;
	stage = NULL;
	if( _name.Upper() == wxT("FILE") )
		b_completed = true;

	ob_object* temp = new ob_object( _name, _tokens );
	Add_SubObj( temp );
}

//-----------------------------------------------------------------
void
ob_StageDeclaration::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_StageDeclaration();
	ob_object::pClone( res );
	((ob_StageDeclaration*)res)->stage = NULL;
	((ob_StageDeclaration*)res)->b_completed = b_completed;
}

//**********************************************

ob_StageDeclaration::~ob_StageDeclaration()
{
	UnLoadStage();
}


//**********************************************

bool ob_StageDeclaration::Eat(  MyLine* line, const int _num_line )
{
	if( ! ungry )
		return false;

	// comment line
	if( line->IsComment() || line->IsEmpty() )
	{
		ob_object* temp = new ob_object( line, _num_line );
		Add_SubObj( temp );
		return true;
	}


	wxString _name = *(line->GetToken(0)->data);
	if( _name.Upper() == wxT("FILE") )
	{
		b_completed = true;
		ungry = false;
	}

	ob_object* temp = new ob_object( line, _num_line );
	Add_SubObj( temp );
	return true;
}

//**********************************************

bool ob_StageDeclaration::Add_SubObj( ob_object* _obj )
{
	// Add the next at the end
	if( _obj->name.Upper() == wxT("NEXT") )
	{
		return Append_SubObj( _obj );
	}
	// Add the branch just after "FILE"
	else if( _obj->name.Upper() == wxT("BRANCH") )
	{
		ob_object * t = GetSubObject( wxT("BRANCH") );
		if( t == NULL )
			return Append_SubObj( _obj );
		else
			return t->InsertObject_After( _obj );
	}
	else
	{
		return Append_SubObj( _obj );
	}
}


//**********************************************

wxString ob_StageDeclaration::ToStr()const
{
	wxString res;

	// Print prefixes
	ob_object *temp = first_prefix;
	while( temp != NULL )
	{
		res += temp->ToStr();
		temp = temp->next;
	}

	temp = first_subobj;
	while( temp != NULL )
	{
		res += temp->ToStr();
		temp = temp->next;
	}

	return res;
}

//**********************************************

void ob_StageDeclaration::Validate()
{
}


//**********************************************

void ob_StageDeclaration::SetStageName(const wxString& _sn)
{
	// Branch Name
	ob_object* temp = GetSubObject( wxT("BRANCH") );
	if( temp != NULL)
		temp->SetToken( 0, _sn );

	// Else, the name is set in the prefixs comments
	ob_object* t = first_prefix;
	while( t != NULL )
	{
		wxString c = t->GetComment();
		int _ind = c.Find( wxT("# STAGENAME : ") );
		if( _ind != wxNOT_FOUND )
		{
			_ind += wxString::FromAscii("# STAGENAME : ").Len();
			wxString _pref = c.Mid(0, _ind);
			t->SetComment( _pref + wxT(" ") + _sn);
			return;
		}
		t = t->next;
	}

	// Have to add a prefix
	PrependCommentLine( wxT("\t# STAGENAME : ") + _sn );
}


//**********************************************

wxString ob_StageDeclaration::GetStageName()
{
	// Branch Name is mandatory
	ob_object* temp = GetSubObject( wxT("BRANCH"));
	if( temp != NULL)
		return temp->GetToken( 0 );

	// Else, the name is set in the prefixs comments
	ob_object* t = first_prefix;
	while( t != NULL )
	{
		wxString c = t->GetComment();
		int _ind = c.Find( wxT("# STAGENAME : ") );
		if( _ind != wxNOT_FOUND )
		{
			_ind += wxString::FromAscii("# STAGENAME : ").Len();
			wxString res = c.Mid( _ind, c.Len() - _ind);
			res.Trim().Trim(false );
			return res;
		}
		t = t->next;
	}

	wxString res = wxFileName(GetFileName()).GetFullName();
	if( res == wxString() )
		res = wxT("UNNAMED");
	return res;
}


//**********************************************

bool ob_StageDeclaration::SetFileName(const wxString& _fn)
{
	ob_object* temp = GetSubObject( wxT("FILE") );
	if( temp != NULL)
		return temp->SetToken( 0, _fn );

	temp = new ob_object( wxT("FILE"), _fn );
	return Add_SubObj( temp );
}


//**********************************************

wxString ob_StageDeclaration::GetFileName()
{
	return GetObFile( GetSubObject_Token( wxT("FILE"), 0 )).GetFullPath();
}


//**********************************************

// bool ob_StageDeclaration::Set_NextTag_State(bool b_next)
// {
// 	// If a "NEXT" tag is already setted
// 	ob_object* temp = GetSubObject( "NEXT" );
// 	if( temp != NULL )
// 	{
// 		// Already setted as wanted
// 		if( b_next == true )
// 			return true;
// 
// 		// Remove the next TAG
// 		delete temp;
// 		return true;
// 	}
// 
// 	// No Next Tag and want to be none ? -> Everythings fine
// 	if( b_next == false )
// 		return true;
// 
// 	//try to find the FILE tag
// 	temp = GetSubObject( "FILE" );
// 
// 	// No FILE Elt -> Cancel the operation
// 	if( temp != NULL)
// 		return false;
// 
// 	// Ok File Elt is here, so just add the tag after it
// 	ob_object* _next = new ob_object( "next" );
// 	temp->InsertObject_After( _next );
// 
// 	return true;
// }

//**********************************************

bool ob_StageDeclaration::Set_BranchTag( const wxString& _bn)
{
	// If a "Branch" tag is already setted
	ob_object* temp = GetSubObject(wxT("BRANCH") );
	if( temp != NULL )
	{
		// Just Change the name
		SetStageName( _bn );
		return true;
	}

	// No branch tag ? -> Create one at the start of the stage declaration
	ob_object* _branch = new ob_object( wxT("BRANCH"), _bn );
	Prepend_SubObj( _branch );
	SetStageName( _bn );
	return true;
}


//**********************************************

bool ob_StageDeclaration::Set_Tag(const wxString& _tag, const wxString& _token )
{
	wxArrayString t;
	t.Add( _token );
	return Set_Tag( _tag, t );
}

//**********************************************

bool ob_StageDeclaration::Set_Tag(const wxString& _tag, wxArrayString _tokens )
{
	// If the tag is already here
	ob_object* temp = GetSubObject( _tag );
	if( temp != NULL )
	{
		temp->SetProperty( _tag, _tokens );
		return true;
	}

	//try to find the FILE tag
	temp = GetSubObject( wxT("FILE") );

	// No FILE Elt -> Cancel the operation
	if( temp != NULL)
		return false;

	// Ok File Elt is here, so just add the tag before it
	ob_object* _new = new ob_object( _tag, _tokens );
	temp->InsertObject_Before( _new );

	return true;
}

//---------------------------------------------------------------------
wxSize 
ob_StageDeclaration::GetDefScrollzBounds()
{
	ob_object* subobj = GetSubObject( wxT("Z") );
	if( subobj == NULL )
		return wxSize(160,232);
	return wxSize( StrToInt(subobj->GetToken(0)), StrToInt(subobj->GetToken(1)) );
}
	
	

//**********************************************

void ob_StageDeclaration::UnLoadStage()
{
	if( stage != NULL )
	{
		delete stage;
		stage = NULL;
	}
}

//**********************************************

bool ob_StageDeclaration::LoadStage(bool b_reload)
{
	if( ! b_reload && stage != NULL )
		return true;

	UnLoadStage();

	wxFileName fn = GetFileName();
	if( ! fn.FileExists() )
		return false;

	stage = new obFileStage( fn );
	if( stage->textFile == NULL )
	{
		delete stage;
		stage = NULL;
		return false;
	}
	return true;
}


//**********************************************

bool ob_StageDeclaration::Is_EmptyOrComment()const
{
	return false;
}

