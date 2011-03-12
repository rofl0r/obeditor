#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/textfile.h>
#include "common__tools.h"
#include "CHistory.h"

// #define DEBUG_OB_OBJECT

using namespace std;

//----------------------------------
#ifdef DEBUG_OB_OBJECT
	#include <set>
	set<void*> ob_deleted;
#endif

/*\internal*********************************************
********	OBJECT GENERAL
******************************************************/

int ob_object::id_unspecified = 0;

/** \internal
 * Expand the array of tokens owning by this object
 */
const size_t PAD_TOKEN_ARR = 4;
void ob_object::arr_tokens_expand( size_t newSize = 0 )
 {
	if( newSize == 0 )
		newSize = arr_token_size + PAD_TOKEN_ARR;

 	wxString* res = new wxString[newSize];
 	for( size_t i=0; i< arr_token_size; i++)
 		res[i] = arr_token[i];
 	arr_token_size = newSize;
 	wxString* temp = arr_token;
 	arr_token = res;
 	delete[] temp;
 }

/** Construct default or line based ob_object.
 * If a valid line is passed, the object parse it and fill his datas struct with it.
 * A basic ob_object only eat one line, for more complex object, derive the class.
 */
ob_object::ob_object( MyLine* line, const int _num_line)
{
	theHistoryManager.Set_State( false );
	// Base init
	ZeroInit();

	// if default constructor => Nothing more
	if( line == NULL )
	{
		theHistoryManager.Set_State( true );
		return;
	}

	// valid line passed => init object with it
	SetData( line, _num_line, false );
	nb_line++;
	theHistoryManager.Set_State( true );
}

ob_object::ob_object( const wxString& _name, wxArrayString& _tokens)
{
	theHistoryManager.Set_State( false );
	// Base init
	ZeroInit();

	// valid line passed => init object with it
	wxString* t_arr = new wxString[_tokens.GetCount()+1];
	t_arr[0] = _name;
	size_t _nb_toks = _tokens.GetCount();
	for( size_t i = 0; i < _nb_toks; i++)
		t_arr[i+1]=_tokens[i];
		
	SetData( t_arr, _nb_toks+1, false );
	delete[] t_arr;
	theHistoryManager.Set_State( true );
}

ob_object::ob_object( const wxString& _name, const wxString& _thetoken)
{
	theHistoryManager.Set_State( false );
	// Base init
	ZeroInit();

	// valid line passed => init object with it
	wxArrayString t;
	SetName( _name );
	SetToken( 0, _thetoken );
	theHistoryManager.Set_State( true );
}

//------------------------------------------------
ob_object*
ob_object::ob_object::Clone()
{
	theHistoryManager.Set_State( false );
	ob_object* res = NULL;
	pClone( res );
	theHistoryManager.Set_State( true );
	return res;
}

//------------------------------------------------
void
ob_object::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_object;
	
	// Base init
	res->ZeroInit();
	res->type = type;
	res->dummy = dummy;
	res->ungry = false;

	// copy the name
	res->SetName( name );

	// copie the datas
	if( res->arr_token != NULL )
	{
		delete[] res->arr_token;
		res->arr_token = NULL;
	}
	res->arr_token_size = arr_token_size;
	if( arr_token_size > 0 )
		res->arr_token = new wxString[arr_token_size];

	for( size_t i= 0; i < nb_tokens; i++)
		res->SetToken( i, arr_token[i] );


	// Copie all subobjects
	ob_object *t = first_subobj;
	while( t != NULL)
	{
		res->Add_SubObj( t->Clone() );
		t = t->next;
	}
}

void ob_object::ZeroInit()
{
	referer = NULL;
	prev = NULL;
	next = NULL;
	first_subobj = NULL;
	last_subobj = NULL;
	first_prefix = NULL;
	last_prefix = NULL;
	parent = NULL;

	num_line = -1;
	nb_line = 0;
	SetType( OB_TYPE_EMPTY_LINE );

	arr_token_size = 2;
	arr_token = new wxString[arr_token_size];
	nb_tokens = 0;
	edited = false;
	ungry = true;
	dummy = false;
}

//------------------------------------------------------------
void ob_object::SetToDefault()
{
	SetName(wxT("UNNAMED"));
}

//------------------------------------------------------------
int  
ob_object::GetType()const
{
	return type;
}

//------------------------------------------------------------
void ob_object::SetType( int _id )
{
	if( _id == OB_TYPE_EMPTY_LINE  )
	{
		SetName( wxString() );
		type = OB_TYPE_STD;
		return;
	}

	if( _id == OB_TYPE_COMMENT )
	{
		SetName( wxString() );
		type = OB_TYPE_STD;
		return;
	}

	type = _id;
}

//--------------------------------------------------------
void 
ob_object::SetEdited( bool _b_edited )
{
	if( edited == _b_edited )
		return;

	edited = _b_edited;
	if( _b_edited == true )
	{
		if( parent != NULL )
			parent->SetEdited( true );
	}
	else
	{
		ob_object* temp = first_subobj;
		while( temp != NULL )
		{
			if( temp->edited == true )
				temp->SetEdited( false );
			temp = temp->GetObject_After();
		}
	}
}

wxString ob_object::_nameID( const wxString& _name )const
{
	ob_object::id_unspecified++;
	return _name + wxT("_") + IntToStr(ob_object::id_unspecified);
}


/**
 * Set the tokens and the name of an object
 * @param arr_data : a wxString array (Note: arr_data[0] will be the name of the object );
 * @param arr_data_size : the size of the array
 * @PARAM specifie if the edited flag have to be check
 * @RETURN the number of token own by the obj.
 */
int ob_object::SetData( wxString* arr_data, const size_t arr_data_size, bool b_update )
{
	bool bchange = false;

	// Set data to empty
	if( arr_data == NULL )
	{
		if( nb_tokens > 0 )
			bchange = true;
		SetNbTokens( 0 );
	}

	// Valid datas passed
	else
	{
		SetNbTokens( 0 );
		
		// Check if one of the token will change his value
		if( b_update )
		{
			if( nb_tokens !=  arr_data_size )
				bchange = true;
			else
				for( size_t i = 0; i< arr_data_size; i++)
					if( arr_data[i].Upper() != arr_token[i].Upper() )
					{
						bchange = true;
						break;
					}
		}

		// Set the Datas
		SetName( arr_data[0] );
//		SetType(OB_TYPE_STD);
		for( size_t i = 1; i <arr_data_size; i++)
			SetToken( i-1, arr_data[i] );
	}

	if( b_update && bchange )
		edited = true;

	ungry = false;
	return nb_tokens;
}

/** \internal Extract the data from the line and fill the data struct of the object with it.
 * Note: the old tokens will be REMOVED !
 * @PARAM the line object from which the object will be built
 * @PARAM the number of the line passed
 * @PARAM specifie if the edited flag have to be check
 * @RETURN the number of token own by the obj.
 */
int ob_object::SetData( MyLine* line, const int _num_line, bool b_update )
{
	// Remove old tokens
	SetNbTokens( 0 );
	if( line == NULL )
	{
		if( referer )
			if( referer->GetTokensCount() || referer->Comment() )
				if( b_update )
					edited = true;
		ungry =false;
		SetType(OB_TYPE_EMPTY_LINE);
		return 0;
	}

	// Empty of Comment line
	if( line->GetTokensCount() <= 0 )
	{
		bool b_edited = false;
		// Empty Line
		if( ! line->IsComment() )
		{
			// check if all this change the object data
			if( type != OB_TYPE_COMMENT )
				b_edited = true;
			else if( referer != NULL )
			{
				if( line->Comment() != referer->Comment() )
					b_edited = true;
			}
			else
				b_edited = true;
			SetType(OB_TYPE_COMMENT);
		}
		else
		{
			if( type != OB_TYPE_EMPTY_LINE )
				b_edited = true;
			SetType(OB_TYPE_EMPTY_LINE);
		}

		// if have to Check if edited flag change
		if( b_update && b_edited )
			edited = true;

		ungry =false;
		AssociateToLine( line, _num_line );
		return nb_tokens;
	}

	// Program Error !!
	else if( line->GetToken(0)->data == NULL )
	{
		ObjectsLog( MYLOG_ERROR, _num_line + 1,
			    wxT("ob_object::tokenize(), token data is NULL although line reports nb_tokens > 0" ));
		SetType(OB_TYPE_EMPTY_LINE);
		SetName( _nameID(wxT("TOKENIZE_ERROR")) );
		ungry =false;
		return nb_tokens;
	}

	// Valid line
	ungry =false;
	AssociateToLine( line, _num_line );
	wxString* arr_data = line->GetDatas();
	SetData( arr_data, line->GetTokensCount(), b_update );
	delete[] arr_data;

	return nb_tokens;
}

int ob_object::SetData( ob_object* _src )
{
	SetNbTokens( 0 );
	for( size_t i = 0; i < _src->nb_tokens; i++ )
		SetToken( i,  _src->arr_token[i] );
	return nb_tokens;
}


/** Make the object eat one more line.
 * The object only eat the line if it is ungry.
 * Based object only eat one line. For more complex object, derive the class.
 * @RETURN if the line have been eated
 */
bool ob_object::Eat( MyLine* line, const int _num_line )
{
	// Not ungry anymore
	if( ! ungry )
		return false;

	SetData( line, _num_line );

	// No more than 1 line to a std object
	ungry = false;

	//line eated
	AddNbLine(1);
	return true;
}


void ob_object::EatPrefixLines()
{
	if( Is_EmptyOrComment()
		&& type != OB_TYPE_STAGE
		&& type != OB_TYPE_LEVELS
		&& type != OB_TYPE_FILE_CONTAINER
		)
		return;

	ob_object *temp = prev;
	while( temp != NULL )
	{
		if( ! temp->Is_EmptyOrComment() )
			break;

		ob_object *_next = temp->prev;
		Prefix_Prepend( temp );
		temp = _next;
	}

	//Recursion in subobjects
	temp = first_subobj;
	while( temp != NULL )
	{
		temp->EatPrefixLines();
		temp =temp->next;
	}
}


void ob_object::ThrowTailings_LineOrComments()
{
	if( parent == NULL )
		return;

	// Throw for all subobjects
	ob_object *temp = last_subobj;
	while( temp != NULL )
	{
		temp->ThrowTailings_LineOrComments();
		temp = temp->prev;
	}

	// Throw for the object itself
	temp = last_subobj;
	while( temp != NULL )
	{
		ob_object* p = temp->prev;
		if( temp->Is_EmptyOrComment() )
		{
			this->InsertObject_After(temp);
			temp = p;
		}
		else
			temp = NULL;
	}

}

void ob_object::AssociateToLine( MyLine* line, size_t _num_line )
{
	referer = line;
	num_line = _num_line;
}

/** Do the syntax validation of the object.
 * Base ob_object have no validation done, derive the class to perform a real validation.
 * The warnings and errors will be logged with wxLogXXX functions.
 * Note: validation() only perform on line syntax, advanded validations must be made in the file object.
 */
void ob_object::Validate()
{
	if( name != wxString() )
		ObjectsLog( MYLOG_ERROR, GetLineRef(),
			    wxT("With ") + name + wxT(", token data is NULL although line reports nb_tokens > 0") );
	return;
}


/** Return the num i token of the object.
 */
wxString ob_object::GetToken(const size_t i )
{
	if( i >= nb_tokens )
		return wxString();

	return arr_token[i];
}


wxString 
ob_object::GetAllTokenToStr()
{
	if( nb_tokens == 0 )
		return wxString();
	wxString res;
	for( int i = 0; i < (int)nb_tokens - 1;i++)
		res += arr_token[i] + wxT(" ");
	res += arr_token[nb_tokens - 1];

	return res;
}

bool 
ob_object::SetAllTokenFromStr( const wxString& _tokens )
{
	wxArrayString arrTok = StrSplit( _tokens, wxT(" ") );
	wxString* tabstr = new wxString[arrTok.Count() + 1];
	
	tabstr[0] = name;
	int curr_ind = 1;

	for( size_t i = 0; i < arrTok.Count(); i++ )
	{
		if( arrTok[i] == wxString() )
			continue;
		tabstr[curr_ind] = arrTok[i];
		curr_ind++;
	}

	SetData( tabstr, curr_ind );
	delete[] tabstr;
	return true;
}

//---------------------------------------------------------------
void 
ob_object::SetName( const wxString& _name )
{
	if( theHistoryManager.IsEnabled() )
		theHistoryManager.Elt__Absorb( 
			new History_Elt( History_Elt::H_NAME_CHANGE, this, name ));
	name = _name;
}

//---------------------------------------------------------------
size_t 
ob_object::GetNbTokens()
{
	return nb_tokens;
}

//---------------------------------------------------------------
void 
ob_object::SetNbTokens( size_t _nb_tokens )
{
	if( _nb_tokens >= nb_tokens )
		return;

	if( theHistoryManager.IsEnabled() )
	{
		int nb_del = nb_tokens - _nb_tokens;
		for( int i = 0; i < nb_del; i++ )
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_TOKEN_RM, this, GetToken( nb_tokens-1) ));
	}
	nb_tokens = _nb_tokens;
}

//---------------------------------------------------------------
bool 
ob_object::SetToken( const int i, const wxString& _data, wxString* default_missings, size_t nb_default_missings )
{
	// Can't fill the missings tokens
	if( ( (int)nb_tokens < i && i > (int) nb_default_missings )
		||
		i < 0 )
	{
		cout<<"BUG !! : ob_object::SetToken() : tokens are missing !!"<<endl;
		return false;
	}

	if( _data == wxString() )
	{
		cout<<"BUG !! : ob_object::SetToken() : _data is EMPTY !!"<<endl;
		return false;
	}

	// fill the missings
	if( (int)nb_tokens < i )
	{
		for( size_t j = nb_tokens; (int)j < i; j++ )
			SetToken( j, default_missings[j] );
	}

	// Don't care about if new val is the old val
	if( i < (int) nb_tokens && arr_token[i] == _data )
		return true;

	// Finally, add the wanted token
	if( (int) arr_token_size == i )
		arr_tokens_expand();
	wxString prev_val = arr_token[i];
	arr_token[i] = _data;
	if( i == (int) nb_tokens )
	{
		if( theHistoryManager.IsEnabled() )
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_TOKEN_ADD, this, GetToken( nb_tokens-1) ));
		nb_tokens++;
	}
	else
	{
		if( theHistoryManager.IsEnabled() )
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_TOKEN_CHANGE, this, prev_val, i ));
	}

	return true;
}


/**
 * Set a property tag for an anim.
 * return the object associated.
 */
ob_object* ob_object::SetProperty( const wxString& _name, const wxString& _the_token )
{
	wxString t = _the_token;
	return SetProperty( _name, &t , 1);
}

ob_object* ob_object::SetProperty( const wxString& _name, wxArrayString& _tokens )
{
	if( _tokens.Count() == 0 )
		return SetProperty( _name, NULL, 0);

	wxString * temp = new wxString[_tokens.Count()];
	for( size_t i = 0; i < _tokens.Count(); i++ )
		temp[i] = _tokens[i];

	ob_object *res = SetProperty( _name, temp, _tokens.Count() );
	delete[] temp;
	return res;
}

/**
 * Set a property tag
 * return the object associated.
 */
ob_object* ob_object::SetProperty( const wxString& _name, wxString* _tokens, size_t _tokens_size )
{
	// Check if the property doesn't exists
	ob_object* o = GetSubObject( _name );
	bool b_must_add_it = false;
	if( o == NULL )
	{
		theHistoryManager.Set_State( false );
		o = new ob_object( _name );
		b_must_add_it = true;
	}

	// Set the tokens of the property
	for( size_t i = 0; i < _tokens_size; i++ )
		o->SetToken( i, _tokens[i] );
	
	if( b_must_add_it == true )
	{
		theHistoryManager.Set_State( true );
		Add_SubObj( o );
	}
	return o;
}

//------------------------------------------------------------------------------
void 
ob_object::Rm()
{
	if( theHistoryManager.IsEnabled() )
	{
		if( Detach(true) == true )
			return;
	}
	delete this;
}

//------------------------------------------------------------------------------
ob_object::~ob_object()
{
#ifdef DEBUG_OB_OBJECT
	if( name != wxString() )
		cout<<"~ob_object :"<<this<<" : "<<name<<","<<GetToken(0)<<endl;
	if( ob_deleted.find( this) != ob_deleted.end() )
	{
		cout<<"Multiple Deleted Object !!!"<<endl;
	}
	else
		ob_deleted.insert( this );
#endif
		
	Detach(false);

	if( arr_token != NULL )
		delete[] arr_token;
	ob_object* t;
	while( first_subobj != NULL)
	{
		t = first_subobj;
		first_subobj = first_subobj->next;
		t->Rm();
	}
	while( first_prefix != NULL)
	{
		t = first_prefix;
		first_prefix = first_prefix->next;
		t->Rm();
	}
}

//------------------------------------------------------------------------------
bool 
ob_object::IsEqual( ob_object* o )
{
	if( UnTokenize( o ).Upper() != UnTokenize( this ).Upper() )
		return false;
	
	ob_object* t0 = first_subobj;
	while( t0 != NULL )
	{
		ob_object* t1 = o->GetSubObject( t0->name );
		if( t1 == NULL )
			return false;
		if( t0->IsEqual( t1 ) == false )
			return false;
		
		t0 = t0->next;
	}
	
	t0 = o->first_subobj;
	while( t0 != NULL )
	{
		if(  this->GetSubObject( t0->name ) )
			return false;
		t0 = t0->next;
	}
	return true;
}


//------------------------------------------------------------------------------
// inserted management
bool 
ob_object::InsertObject_Before( ob_object* obj)
{
	if( obj == this )
	{
		MyLog( MYLOG_DEBUG, wxT("ERROR : InsertObject_Before()"), wxT("obj == this") );
		return false;
	}
	// already the previous
	if( prev == obj )
	{
		MyLog( MYLOG_DEBUG, wxT("ERROR : InsertObject_Before()"), wxT("prev == obj") );
		return true;
	}
	
	obj->Detach(true);

	if( prev != NULL)
	{
		prev->next = obj;
		obj->prev = prev;
	}
	prev = obj;
	obj->next = this;
	obj->parent = parent;
	if( parent != NULL )
	{
		if( parent->first_subobj == this )
			parent->first_subobj = obj;
		if( parent->first_prefix == this )
			parent->first_prefix = obj;
		parent->AddNbLine(obj->nb_line);
	}

	if( theHistoryManager.IsEnabled() )
		theHistoryManager.Elt__Absorb( 
			new History_Elt( History_Elt::H_OBJ_INSERT, obj, this, false ));
	return true;
}

//------------------------------------------------------
ob_object* 
ob_object::RemoveObject_Before(bool b_tell_histManager)
{
	if( prev == NULL )
		return NULL;

	if( theHistoryManager.IsEnabled() == true && b_tell_histManager == true )
		theHistoryManager.Elt__Absorb(
			new History_Elt( History_Elt::H_OBJ_RM, prev, this, false ));
	
	ob_object *res = prev;
	if( prev->prev != NULL )
	{
		if( prev->prev == this )
			prev = NULL;
		else
		{
			ob_object * temp = prev->prev;
			prev = temp;
			temp->next = this;
		}
	}
	else
	{
		prev = NULL;
		if( parent != NULL )
		{
			if( parent->first_subobj == res )
				parent->first_subobj = this;
			if( parent->first_prefix == res )
				parent->first_prefix = this;
		}
	}

	res->prev = NULL;
	res->next = NULL;
	res->parent = NULL;

	if( parent != NULL )
		parent->AddNbLine( -res->nb_line);
	return res;
}

//-----------------------------------------------------
ob_object* 
ob_object::GetObject_Before()
{
	return prev;
}

//-----------------------------------------------------
bool 
ob_object::InsertObject_After( ob_object* obj)
{
	if( obj == this )
	{
		MyLog( MYLOG_DEBUG, wxT("ERROR : InsertObject_After()"), wxT("obj == this") );
		return false;
	}
	
	// already the next
	if( next == obj )
	{
		MyLog( MYLOG_DEBUG, wxT("ERROR : InsertObject_After()"), wxT("next == obj") );
		return true;
	}

	
	obj->Detach(true);

	if( next != NULL)
	{
		next->prev = obj;
		obj->next = next;
	}
	next = obj;
	obj->prev = this;
	obj->parent = parent;
	if( parent != NULL )
	{
		if( parent->last_subobj == this )
			parent->last_subobj = obj;
		if( parent->last_prefix == this )
			parent->last_prefix = obj;
		parent->AddNbLine(obj->nb_line);
	}

	if( theHistoryManager.IsEnabled() )
	{
		if( obj->next != NULL )
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_OBJ_INSERT, obj
						, obj->next, false ));
		else
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_OBJ_INSERT, obj
						, parent, true ));
	}
	return true;
}

//------------------------------------------------------------
ob_object* 
ob_object::RemoveObject_After(bool b_tell_histManager)
{
	if( next == NULL )
		return NULL;

	if( theHistoryManager.IsEnabled() && b_tell_histManager == true )
	{
		if( next->next != NULL )
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_OBJ_RM, next, next->next, false ));
		else
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_OBJ_RM, next, parent, true ));
	}
	
	ob_object *res = next;
	if( next->next != NULL )
	{
		if( next->next == this )
			next = NULL;
		else
		{
			ob_object * temp = next->next;
			next = temp;
			temp->prev = this;
		}
	}
	else
	{
		next = NULL;
		if( parent != NULL )
		{
			if( parent->last_subobj == res )
				parent->last_subobj = this;
			if( parent->last_prefix == res )
				parent->last_prefix = this;
		}
	}

	res->prev = NULL;
	res->next = NULL;
	res->parent = NULL;
	if( parent != NULL )
		parent->AddNbLine(-res->nb_line);
	return res;
}

//----------------------------------------------------------------
ob_object* 
ob_object::GetObject_After()
{
	return next;
}


//----------------------------------------------------------------
ob_object*	
ob_object::GetObject_Group_First()
{
	ob_object* temp = prev;
	if( temp == NULL )
		return this;

	while( temp->prev != NULL )
		temp = temp->prev;
	return temp;
}

//----------------------------------------------------------------
ob_object*	
ob_object::GetObject_Group_Last()
{
	ob_object* temp = next;
	if( temp == NULL )
		return this;

	while( temp->next != NULL )
		temp = temp->next;
	return temp;
}

//----------------------------------------------------------------
ob_object* 
ob_object::GetSubObject( const wxString& name_subobj )
{
	wxString to_find = name_subobj.Upper();
	ob_object* res = first_subobj;
	while( res != NULL )
	{
		if( res->name.Upper() == to_find )
			return res;
		res = res->next;
	}
	return res;
}


//----------------------------------------------------------------
wxString 
ob_object::GetSubObject_Token(  const wxString& name_subobj
					, size_t num_tok,const wxString& def_val)
{
	ob_object* temp = GetSubObject(name_subobj);
	if( temp == NULL )
		return def_val;
	return temp->GetToken(num_tok);
}


//-------------------------------------------------------------
ob_object** 
ob_object::GetSubObjectS_ofType( const int _type , size_t& nb_subobj)
{
	ob_object* t = first_subobj;
	nb_subobj = 0;
	while( t != NULL )
	{
		if( t->type == _type )
			nb_subobj++;
		t = t->next;
	}
	if( nb_subobj == 0 )
		return NULL;

	ob_object** res = new ob_object*[nb_subobj];

	t = first_subobj;
	size_t ind = 0;
	while( t != NULL )
	{
		if( t->type == _type )
		{
			res[ind] = t;
			ind++;
		}
		t = t->next;
	}

	return res;
}


//----------------------------------------------------------
ob_object** 
ob_object::GetAllSubObjS( size_t& res_size )
{
	res_size = 0;
	list<ob_object*> l_res;

	ob_object* temp = first_subobj;
	while( temp != NULL )
	{
		l_res.push_back( temp );
		temp = temp->next;
	}

	// No subobj case
	if( l_res.empty() == true )
		return NULL;
	
	// Prepare res
	ob_object** res = new ob_object*[l_res.size()];
	
	// list -> array
	while( l_res.empty() == false )
	{
		res[res_size++] = l_res.front();
		l_res.pop_front();
	}
	return res;
}

//---------------------------------------------------------------------
ob_object** 
ob_object::GetSubObjectS( const wxString& _name, size_t& count )
{
	count = 0;
	wxString t_name = _name.Upper();
	
	list<ob_object*> l_res;
	
	ob_object *temp = first_subobj;
	while( temp != NULL )
	{
		if( temp->name.Upper() == t_name )
			l_res.push_back( temp );
		temp = temp->next;
	}

	// No subobjects
	if( l_res.empty() == true )
		return NULL;

	// Prepare returns vals
	ob_object** res = new ob_object*[l_res.size()];
	
	// List -> array
	while( l_res.empty() == false )
	{
		res[count++] = l_res.front();
		l_res.pop_front();
	}
	
	return res;
}

//------------------------------------------------------------------
bool 
ob_object::Add_SubObj( ob_object* _obj )
{
	return Append_SubObj( _obj );
}


//------------------------------------------------------------------
bool 
ob_object::Append_SubObj( ob_object* _obj )
{
//	wxLogInfo( "Append to : " + this->name + " [" + temp->name + "]");

	if( last_subobj == NULL )
	{
		_obj->Detach(true);
		first_subobj = _obj;
		last_subobj = _obj;
		_obj->parent = this;
		AddNbLine(_obj->nb_line);
		if( theHistoryManager.IsEnabled() )
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_OBJ_INSERT
					, _obj,this, true ));
	}
	else
		last_subobj->InsertObject_After( _obj );

//	wxLogInfo( name + " " + GetToken(0) + " appended : " + _obj->name + " " + _obj->GetToken(0));
	return true;
}

//------------------------------------------------------------------
bool 
ob_object::Prepend_SubObj( ob_object* _obj )
{
	if( first_subobj == NULL )
	{
		_obj->Detach(true);
		first_subobj = _obj;
		last_subobj = _obj;
		_obj->parent = this;
		AddNbLine(_obj->nb_line);
		if( theHistoryManager.IsEnabled() )
			theHistoryManager.Elt__Absorb( 
				new History_Elt( History_Elt::H_OBJ_INSERT
					, _obj,this, true ));
	}
	else
		first_subobj->InsertObject_Before( _obj );

	return true;
}

//------------------------------------------------------------------
bool 
ob_object::Insert_SubObj_After( ob_object* _obj )
{
	if( _obj == NULL )
		return false;

	if( _obj == last_subobj )
		return Append_SubObj( _obj );

	ob_object *temp = first_subobj;
	while( temp != NULL )
	{
		if( temp == _obj )
		{
			temp->InsertObject_After(_obj);
			return true;
		}
		temp = temp->next;
	}
	return false;
}

//----------------------------------------------------------------
bool 
ob_object::Insert_SubObj_Before( ob_object* _obj )
{
	if( _obj == NULL )
		return false;

	if( _obj == first_subobj )
		return Prepend_SubObj( _obj );

	ob_object *temp = last_subobj;
	while( temp != NULL )
	{
		if( temp == _obj )
		{
			temp->InsertObject_Before(_obj);
			return true;
		}
		temp = temp->prev;
	}
	return false;
}

//-----------------------------------------------------
void 
ob_object::Prefix_Append( ob_object* line_prefix )
{
	if( first_prefix == NULL )
	{
		line_prefix->Detach(true);
		first_prefix = line_prefix;
		last_prefix = line_prefix;
		line_prefix->parent = this;
		AddNbLine( line_prefix->nb_line );
	}
	else
		last_prefix->InsertObject_After( line_prefix );
}

void ob_object::Prefix_Prepend( ob_object* line_prefix )
{
	if( first_prefix == NULL )
	{
		line_prefix->Detach(true);
		first_prefix = line_prefix;
		last_prefix = line_prefix;
		line_prefix->parent = this;
		AddNbLine( line_prefix->nb_line );
	}
	else
		first_prefix->InsertObject_Before( line_prefix );

}

//------------------------------------------------------------------------------
void 
ob_object::Absorb_PrefixLines_Of( ob_object * _o, bool b_prepend )
{
	if( b_prepend == true )
	{
		ob_object *_prefix = _o->first_prefix;
		while( _prefix != NULL )
		{
			Prefix_Prepend( _prefix );
			_prefix = _o->first_prefix;
		}
		return;
	}

	else // Must append
	{
		ob_object *_prefix = _o->last_prefix;
		while( _prefix != NULL )
		{
			Prefix_Append( _prefix );
			_prefix = _o->last_prefix;
		}
		return;
	}
}

// Flushing
wxString ob_object::ToStr()const
{
	wxString res;

	// Print prefixes
	ob_object *temp = first_prefix;
	while( temp != NULL )
	{
		res += temp->ToStr();
		temp = temp->next;
	}

	if( type != OB_TYPE_FRAME && type != OB_TYPE_STAGE_DECLARATION && type != OB_TYPE_FILE_CONTAINER && ! dummy )
	{
		res += GetPrefixFor(0) + name;
		for( size_t i=0; i< nb_tokens; i++)
			res += GetPrefixFor(i+1) + arr_token[i];

		res += GetComment();
		res += wxT("\n");
	}

	temp = first_subobj;
	while( temp != NULL )
	{
		res += temp->ToStr();
		temp = temp->next;
	}

	return res;
}


//************************************************************

bool ob_object::SetComment( const wxString& _comment )
{
	mycomment = _comment;
	return true;
}


//************************************************************

void ob_object::PrependCommentLine( const wxString& _comment, bool b_endOfPrefixComments )
{
	ob_object* t = new ob_object();
	t->SetComment( _comment );
	if( b_endOfPrefixComments )
		Prefix_Append( t );
	else
		Prefix_Prepend( t );
}


//************************************************************

wxString ob_object::GetComment()const
{
	if( mycomment != wxString() )
		return mycomment;
	else if( referer != NULL )
		if( referer->Comment() != wxString() )
			return referer->Comment();
	return wxString();
}


//************************************************************
wxString 
ob_object::GetPrefixFor(const size_t i)const
{
	if( referer != NULL && referer->GetToken(i) != NULL && referer->GetToken(i)->prefix != NULL)
		return referer->GetToken(i)->Prefix();

	else if( referer != NULL && referer->GetToken(i) != NULL && referer->GetToken(i)->prefix == NULL)
		return wxString();

	else if( i> 0)
		return wxT("  ");
	else
	{
		if( name ==wxString())
			return wxString();

		wxString pref = wxString();
		ob_object *temp = parent;
		if( temp == NULL )
			return wxString();
		temp = temp->parent;
		while( temp != NULL )
		{
			pref += wxT("\t");
			temp = temp->parent;
		}
		return pref;
	}
}

//--------------------------------------------------------------------
// Types relative
bool 
ob_object::Is_Ob_Stage_Object()
{
	switch( type )
	{
		case OB_STAGE_OBJECT:
		case OB_TYPE_WALL:
		case OB_TYPE_HOLE:
		case OB_TYPE_PLAYER_SPAWN:
		case OB_TYPE_STAGE_AT:
		case OB_TYPE_SPAWN:
		case OB_TYPE_SPAWN_HEALTHY:
		case OB_TYPE_SPAWN_ITEM:
			return true;
	}
	return false;
}

//-----------------------------------------------------------
bool 
ob_object::Absorb( ob_object* tomerge, bool after )
{
	if( tomerge == this )
		return false;

	tomerge->Detach(true);
	if( after )
		GetObject_Group_Last()->InsertObject_After( tomerge );
	else
		GetObject_Group_First()->InsertObject_Before( tomerge );
	return true;
}

//-------------------------------------------------------------
// Return true if the HistManager have absorbed the elt
bool
ob_object::Detach(bool b_tell_hist_manager)
{
	bool b_res = b_tell_hist_manager;
	
	if( prev != NULL )
		prev->RemoveObject_After(b_tell_hist_manager);
	else if( next != NULL )
		next->RemoveObject_Before(b_tell_hist_manager);

	else if( parent != NULL )
	{
		if( b_tell_hist_manager == true && theHistoryManager.IsEnabled() == true )
			b_res = theHistoryManager.Elt__Absorb( 
					new History_Elt( History_Elt::H_OBJ_RM, this
								, parent, true ));
		if( parent->first_subobj == this )
		{
			parent->first_subobj = NULL;
			parent->last_subobj = NULL;
		}
		if( parent->first_prefix == this )
		{
			parent->first_prefix = NULL;
			parent->last_prefix = NULL;
		}
		parent->AddNbLine( -nb_line);
		parent = NULL;
	}
	else
		b_res = false;
	
	return b_res;
}

int ob_object::GetLineRef()const
{
	return num_line + 1;
}

wxString ob_object::GetLineRef_S()const
{
	return IntToStr(GetLineRef());
}

bool ob_object::Is_EmptyOrComment()const
{
	return (nb_tokens <= 0 ) && name == wxString()
		&& type != OB_TYPE_LEVELS && type != OB_TYPE_STAGE_DECLARATION;
}

bool ob_object::Is_EmptyLine()const
{
	if( referer == NULL )
		return ((nb_tokens <= 0 ) && name == wxString()
			&& type != OB_TYPE_LEVELS && type != OB_TYPE_STAGE_DECLARATION );

	wxString t = referer->Comment();
	return ((nb_tokens <= 0 ) && name == wxString() && t == wxString() );
}




void ob_object::AddNbLine(int i)
{
	nb_line+=i;
	ob_object *temp = parent;
	while( temp != NULL )
	{
		temp->nb_line += nb_line;
		temp = temp->parent;
	}
}

/******************************************************
********	OBJECT MODELS
******************************************************/
ob_object_container::ob_object_container()
:ob_object()
{
	type = OB_TYPE_FILE_CONTAINER;
}

//-----------------------------------------------------------------
void
ob_object_container::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_object_container();
	ob_object::pClone( res );
}

//-----------------------------------------------------------------
ob_object_container::~ob_object_container()
{
}

//******************************************************
wxString 
ob_object_container::ToStr()const
{
	ob_object* temp = first_subobj;
	wxString res;
	while( temp != NULL )
	{
		res += temp->ToStr();
		temp = temp->next;
	}

	return res;
}

//-----------------------------------------------------------------
void
ob_entity_container::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_entity_container();
	ob_object_container::pClone( res );
}


//------------------------------------------------------------------------
bool
ob_entity_container::Add_SubObj( ob_object* temp )
{
	if( temp->name.Upper() == wxT("ANIM") || temp->name.Upper() == wxT("ANIMATION") )
		return Append_SubObj( temp );
	
/*	// Property already exists
	ob_object* o = GetSubObject(temp->_name ) != NULL )
		return ob_object::SetProperty( temp );
*/
	// Search the first anim/frame of the file
	ob_object* t = GetSubObject( wxT("anim") );
	if( t == NULL )
	{
		t = GetSubObject( wxT("animation") );
		if( t == NULL )
			t = GetSubObject( wxT("frame") );
		
		// No frame or anim subobjects
		if( t == NULL )
			return Append_SubObj( temp );
	}

	return t->InsertObject_Before( temp );
}

/******************************************************
********	OBJECT MODELS
******************************************************/

ob_models::ob_models( MyLine* line, const int _num_line )
: ob_object( line, _num_line )
{
	type = OB_TYPE_MODELS;
	Validate();
}

ob_models::ob_models( const wxString& _name, wxArrayString& _tokens)
: ob_object( _name, _tokens )
{
	type = OB_TYPE_MODELS;
	Validate();
}

ob_models::~ob_models()
{}

//-----------------------------------------------------------------
void
ob_models::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_models();
	ob_object::pClone( res );
}

//-----------------------------------------------------------------
void 
ob_models::Validate()
{
	// Don"t validate blank lines
	if( name == wxString())
		return;

	ob_validator* validator = models_constraints[name.Lower()];
	if( validator != NULL)
	{
//		wxLogInfo( "Validating " + ToStr() );
		validator->Validate( this, arr_token, nb_tokens );
	}
	else
	{
		ObjectsLog( MYLOG_WARNING, GetLineRef(),
			    wxT("Unknown property : ") + name );
	}
}



/******************************************************
********	OBJECT ENTITY
******************************************************/

wxString animation_tag[] =
{ wxT("anim"), wxT("animation"), wxT("loop"),
wxT("fastattack"), wxT("hitfx"), wxT("hitflash"), 
wxT("custknife"), wxT("custstar"), wxT("custbomb"),
wxT("custpbomb"), wxT("delay"), wxT("offset"),
wxT("bbox"), wxT("frame"), wxT("range"), 
wxT("rangez"), wxT("rangea"), wxT("attack"), wxT("attack1"),
wxT("attack2"), wxT("attack3"), wxT("attack4"),
wxT("attack5"), wxT("attack6"), wxT("attack7"), 
wxT("attack8"), wxT("attack9"),
wxT("blast"), wxT("shock"), wxT("burn"), wxT("freeze"), 
wxT("steal"), wxT("quakeframe"), wxT("move"),
wxT("movea"), wxT("movez"), wxT("seta"), wxT("platform"),
wxT("dive"), wxT("sound"), wxT("pshotframe"), 
wxT("throwframe"), wxT("tossframe"), wxT("pbombframe"),
wxT("jumpframe"), wxT("custpshot"),
wxT("mpcost"), wxT("custfireb"), wxT("shootframe"), 
wxT("flipframe"), wxT("followanim"), wxT("followcond"), wxT("counterframe"),
wxT("spawnframe"), wxT("unsummonframe"), wxT("subentity"),
wxT("custentity"), wxT("weaponframe"), wxT("attackone"), wxT("grabin"),
wxT("forcedirection"), wxT("damageonlanding"), wxT("dropv"),
wxT("dropframe"), wxT("landframe"), wxT("counterattack"),
wxT("fshadow"), wxT("shadowcoords"), wxT("itembox"), wxT("stun"), 
wxT("seal"), wxT("forcemap"), wxT("drain"), wxT("noreflect"),
wxT("drawmethod"), wxT("nodrawmethod"), wxT("bouncefactor"), wxT("mponly"),
wxT("hponly"), wxT("energycost"), wxT("hitflash"), wxT("summonframe"),
wxT("throwframewait"), wxT("resetable"), wxT("@SCRIPT"), wxT("@cmd") };

wxString frame_tag[] =
{ wxT("frame"), wxT("delay"), wxT("offset"), 
wxT("bbox"), wxT("attack"), wxT("attack1"),
wxT("attack2"), wxT("attack3"), wxT("attack4"),
wxT("attack5"), wxT("attack6"), wxT("attack7"),
wxT("attack8"), wxT("attack9"),
wxT("blast"), wxT("shock"), wxT("burn"),
wxT("freeze"), wxT("steal"), wxT("throwframe"), 
wxT("quakeframe"), wxT("move"), wxT("movea"),
wxT("movez"), wxT("seta"), wxT("platform"),
wxT("dive"), wxT("sound"), wxT("grabin"), 
wxT("fshadow"), wxT("shadowcoords"), wxT("itembox"),
wxT("stun"), wxT("seal"), wxT("forcemap"), 
wxT("drain"), wxT("noreflect"),
wxT("drawmethod"), wxT("nodrawmethod"), wxT("@cmd"), 
wxT("hitfx"), wxT("hitflash"), wxT("dropv") };

/**
 * Build the correct object from line and return it.
 * Select the correct ob_entity or derived object to build for the line
 * Build it and return it
 */
ob_entity* ob_entity::Guess_and_ConstructNewObject(MyLine* line, const int _num_line )
{
	// if Empty Line
	if( line->GetTokensCount() <= 0 )
		return new ob_entity(line, _num_line);


	wxString _name = *(line->GetToken(0)->data);
	ob_entity *temp;
	if( _name.Upper() == wxT("ANIM") || _name.Upper() == wxT("ANIMATION") )
		temp = new ob_anim(line,_num_line);
	else
		temp = new ob_entity( line, _num_line);

//	wxLogInfo( "At line " + IntToStr( _num_line +1) + ", object build :" + temp->name );

	return temp;
}

//-----------------------------------------------------------------
ob_entity::ob_entity( MyLine* line, const int _num_line, bool b_validate )
: ob_object( line, _num_line )
{
	type = OB_TYPE_ENTITY;
	if( b_validate )
		Validate();
}

//-----------------------------------------------------------------
ob_entity::ob_entity( const wxString& _name, wxArrayString& _tokens)
: ob_object( _name, _tokens )
{
	type = OB_TYPE_ENTITY;
	Validate();
}

//-----------------------------------------------------------------
void
ob_entity::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_entity();
	ob_object::pClone( res );
}

//-----------------------------------------------------------------
ob_entity::~ob_entity()
{}

void ob_entity::Validate()
{
	// Don"t validate blank lines
	if( name == wxString())
		return;

	ob_validator* validator = entity_constraints[name.Lower()];
	if( validator != NULL)
	{
//		wxLogInfo( "Validating " + ToStr() );
		validator->Validate( this, arr_token, nb_tokens );
	}
	else
		ObjectsLog( MYLOG_WARNING, GetLineRef(),
			    wxT("Unknown property : ") + name );
}

/******************************************************
********	OBJECT ANIMATION
******************************************************/
bool ob_anim::IsAnimTag( const wxString& data )
{
	for( int i= 0; i< t_size_of( animation_tag); i++ )
		if( animation_tag[i].Upper() == data.Upper() )
			return true;
	return false;
}

ob_anim::ob_anim( MyLine* line, const int _num_line )
:ob_entity( line, _num_line )
{
	if( arr_token_size <= 0 )
		SetToken(0, _nameID( wxT("UNSPECIFIED_ANIM")));
	else
		SetToken(0, arr_token[0]);

	ungry = true;
	nb_frame = 0;
	frame_open = false;
	SetType( OB_TYPE_ANIM );
	if( name == wxString() )
		SetName( wxT("anim") );
}

//-----------------------------------------------------------------
void
ob_anim::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_anim();
	ob_entity::pClone( res );

	if( arr_token_size <= 0 )
		SetToken(0, _nameID( wxT("UNSPECIFIED_ANIM")));
	else
		SetToken(0, arr_token[0]);
	((ob_anim*)res)->nb_frame = nb_frame;
	((ob_anim*)res)->frame_open = false;
}

//-----------------------------------------------------------------
wxString ob_anim::animID()
{
	return GetToken(0);
}


ob_anim::~ob_anim()
{
}


void ob_anim::ThrowTailings_LineOrComments()
{
	if( parent == NULL )
		return;

	// find the last meaningfull object
	ob_object *temp = last_subobj;
	while( temp != NULL && temp->Is_EmptyOrComment() )
		temp = temp->prev;

	// no meaningfull object => empty anim
	if( temp == NULL )
		temp = first_subobj;

	// find the first blank line after the last meaningfull object
	while( temp != NULL && ! temp->Is_EmptyLine() )
		temp = temp->next;

	// no blank lines in trailings comment => nothing to throw
	if( temp == NULL )
		return;

	// throw the blank line and everything after
	temp = temp->prev;
	if( temp != NULL )
		while( temp->next != NULL)
			InsertObject_After(temp->next);

}


bool ob_anim::Eat( MyLine* line, const int _num_line )
{

	if( ! ungry )
		return false;

	// If new anim
	if( ! line->IsComment() && ! line->IsEmpty()  )
		if( line->GetToken(0)->data->Upper() == wxT("ANIM") ||
			line->GetToken(0)->data->Upper() == wxT("ANIMATION") )
		{
			if( frame_open )
			{
				wxString __t;
				if( line->GetToken(1) != NULL && line->GetToken(1)->data != NULL )
					__t =  *(line->GetToken(1)->data);
//FIXME??				ObjectsLog( MYLOG_WARNING, 1 + _num_line, "new ANIM <" + __t + "> declared although a frame is still open. Perphaps a previous unknown tag is responsible ??" );
				last_subobj->dummy = true;
			}
			ungry = false;
			return false;
		}

	if( frame_open )
	{
		ob_frame* fr = (ob_frame*) last_subobj;
		bool res = fr->Eat( line, _num_line );

		// ERROR CASE : Frame don't eat the line BUT it had to (cause frame are close with explicit tag "frame"
		if( !res )
		{
			wxString __t;
			if( line->GetToken(1) != NULL && line->GetToken(1)->data != NULL )
				__t ==  *(line->GetToken(1)->data);
			ObjectsLog( MYLOG_WARNING, _num_line + 1,
				    wxT("non anim tag <") + __t + wxT("> found in a frame zone !! Perhaps it's just an non referenced tag ??") );
		}

		// Frame closed
		if( !fr->ungry)
		{
			frame_open = false;
			return true;
		}
		// frame is still ungry
		else // fr->ungry == true
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

	if( last_subobj != NULL && last_subobj->name.Upper() == wxT("@SCRIPT") )
		if( last_subobj->Eat( line, _num_line ) )
			return true;

	// Valid line
	wxString _name = *(line->GetToken(0)->data);

	// Miam !
	if( ob_anim::IsAnimTag( _name ))
	{
		// If a frame is already open -> Why the frame doesn't eat it at the first step of the function ?
		bool is_frametag = ob_frame::IsFrameTag( _name );
		bool b_buildFrame = false;

		if( is_frametag && frame_open )
		{
			ObjectsLog( MYLOG_ERROR, _num_line + 1,
				    wxT("A frame open although the other not closed !") );
			b_buildFrame = true;
		}

		else if( !is_frametag && nb_frame > 0 )
			b_buildFrame = true;

		else if( is_frametag )
			b_buildFrame = true;

		if( b_buildFrame )
		{
			ob_object* temp = new ob_frame();
			temp->Eat( line, _num_line );

			Add_SubObj( temp );
			frame_open = temp->ungry;
		}
		else if( _name.Upper() == wxT("@SCRIPT") )
			Add_SubObj( new ob_script( line, _num_line ));

		else
			Add_SubObj( new ob_object( line, _num_line ));
	}
	// Non anim_tag
	else
	{
		ObjectsLog( MYLOG_WARNING, _num_line + 1,
			    wxT("header property <") + _name + wxT("> found after the first anim had started ! All non anim related header must be before the anims. Or may be it's a non referenced tag ?") );
		//Will not eat that !!
//		Add_SubObj( new ob_object( line, _num_line ));
		ungry = false;
		return false;
	}

	return true;
}


ob_frame*  ob_anim::GetFrame( const int num )
{
	ob_object* temp = first_subobj;
	int i = -1;
	while( temp != NULL )
	{
		if( temp->type == OB_TYPE_FRAME )
			i++;
		if( i == num )
			return (ob_frame*) temp;
		temp = temp->GetObject_After();
	}
	return NULL;
}

//--------------------------------------------------------
ob_frame** 
ob_anim::GetFrames(size_t& nb_frame)
{
	nb_frame = 0;
	list<ob_frame*> l_res;
	
	ob_object* t = GetFrame(0);
	while( t != NULL)
	{
		if( t->type == OB_TYPE_FRAME )
			l_res.push_back( (ob_frame*) t );
		t = t->next;
	}

	if( l_res.empty() == true )
		return NULL;
	
	ob_frame** res = new ob_frame*[l_res.size()];
	while( l_res.empty() == false )
	{
		res[nb_frame++] = l_res.front();
		l_res.pop_front();
	}
	return res;
}


/**
 * Set a property tag for an anim.
 * return the object associated.
 */
bool
ob_anim::Add_SubObj( ob_object* o )
{
	if( o->Is_EmptyOrComment() == true  )
		return Append_SubObj( o );

	// A new prop have been set -> move it before the first frame
	if( o->name.Upper() != wxT("FRAME") )
	{
		ob_object* _first_frame = GetFrame(0);
		if( _first_frame != NULL )
			return _first_frame->InsertObject_Before( o );
	}

	return Append_SubObj( o );
}

//----------------------------------------------------------------
bool 
ob_anim::Absorb( ob_object* tomerge, bool b_after )
{
	if( tomerge == NULL )
		return false;

	if( tomerge == this )
		return false;

	if( tomerge->parent == this )
		return false;

	tomerge->Detach(true);
	ob_frame *first_frame = GetFrame(0);

	if( tomerge->type == OB_TYPE_ANIM  )
	{
//		wxLogInfo( "Merging " + ((ob_anim*)tomerge)->animID() + " to " + animID() );

		// merge all subobj to this
		ob_object *temp = b_after ? tomerge->first_subobj : tomerge->last_subobj;
		while( temp != NULL )
		{
			ob_object *_next = b_after ? temp->next : temp->prev;
			Absorb( temp, b_after );
			temp = _next;
		}
	}
	else if( tomerge->type == OB_TYPE_FRAME  )
	{
//		wxLogInfo( "Absorbing FRAME " + ((ob_frame*)tomerge)->GifPath().GetFullName() + " to " + animID() );
		if( !b_after )
		{
			if( first_frame != NULL )
			{
				first_frame->InsertObject_Before( tomerge );
				first_frame = (ob_frame*)tomerge;
			}
			else
			{
				Add_SubObj( tomerge );
				first_frame = (ob_frame*)tomerge;
			}
		}
		else
			Add_SubObj( tomerge );
	}
	// Basic object
	else
	{
		if( b_after )  // => to append before the first frame
		{
/*
			if( first_frame != NULL)
				first_frame->InsertObject_Before( tomerge );
			else
*/
				Add_SubObj( tomerge );
		}
		else
			Prepend_SubObj( tomerge );
	}

	return true;
}


/***************************
******** FRAME  OBJECT
********************/

// The properties that are copied from the previous frame
wxString _to_check_with_prevframe[] = {wxT("delay"), wxT("offset"), wxT("bbox"),
wxT("move"), wxT("movez"), wxT("movea"),
wxT("nodrawmethod"), wxT("drawmethod"), wxT("platform"), wxT("dropv")	};

bool
ob_frame::IsFrameTag( const wxString& data )
{
	for( size_t i =0; i< t_size_of(frame_tag); i++ )
		if( frame_tag[i].Upper() == data.Upper() )
			return true;
	
	if( ob_frame_attack::IsAttack_Tag( data ) == true )
		return true;

	return false;
}


//-----------------------------------------------------------------------
ob_frame::ob_frame( MyLine* line, const int _num_line )
:ob_entity(line,_num_line)
{
	if( line != NULL )
		ObjectsLog( MYLOG_ERROR, _num_line + 1,
			    wxT("A frame is not supposed to be build with a valid line (it's a last block line created object) !!") );

	type = OB_TYPE_FRAME;
	nb_line=0;
	flipState = 0;
	jumpState = 0;
	dropState = 0;
	landState = 0;
	end_jump = false;

	img = NULL;
	png_or_gif = NULL;

	if( name == wxString() )
		SetName( wxT("frame") );
}

//-----------------------------------------------------------------
void
ob_frame::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_frame();
	ob_entity::pClone( res );
	img = NULL;
	png_or_gif = NULL;
	flipState = 0;
	jumpState = 0;
	dropState = 0;
	landState = 0;
	end_jump = false;
}

//---------------------------------------------------
ob_frame::~ob_frame()
{
	if( img != NULL )
		delete img;

	if( png_or_gif != NULL )
		delete png_or_gif;
}

//---------------------------------------------------
wxFileName 
ob_frame::GifPath()const
{
	if( nb_tokens >= 1 )
		return GetObFile( arr_token[0] );
	else
		return wxFileName(wxString());
}

//---------------------------------------------------
bool 
ob_frame::Eat( MyLine* line, const int _num_line )
{
	if( ! ungry )
		return false;

	// comment line
	if( line->IsComment() || line->IsEmpty() )
	{
		ob_object* temp = new ob_object( line, _num_line );
		Add_SubObj( temp );

		// Still hungry
		return true;
	}

	wxString _name = *(line->GetToken(0)->data);

	if( last_subobj != NULL && last_subobj->name.Upper() == wxT("@SCRIPT") )
		if( last_subobj->Eat( line, _num_line ) )
			return true;

	if( last_subobj != NULL && last_subobj->Eat( line, _num_line) == true )
		return true;
		
	if( _name.Upper() == wxT("ANIM") or _name.Upper() == wxT("ANIMATION") )
	{
		ObjectsLog( MYLOG_WARNING, _num_line + 1,
			    wxT("with <") + _name +wxT("> a start Anim declaration found although the last frame don't close !!") );
	   	dummy=true;
	   	ungry = false;
	    return false;
	}

	// Non Valid Frame line
	else if( !ob_frame::IsFrameTag( _name ))
	{
		// Anim tag in a frame zone
		if( ob_anim::IsAnimTag( _name ))
			ObjectsLog( MYLOG_WARNING, _num_line + 1,
				    wxT("<") + _name + wxT("> is a Non-Frame Tag inside a frame zone !! Or may be an non referenced frame tag ?"));
		// Non anim tag in the zone, let the anim parent object handle the situation
		else
			ObjectsLog( MYLOG_ERROR, _num_line + 1,
				    wxT("<") + _name + wxT("> is a Non-Anim Tag inside a frame zone !! Or may be an non referenced tag ??" ));
	}

	else if( ob_frame_attack::IsAttack_Declare_Tag( _name) == true )
	{
		Add_SubObj( new ob_frame_attack( line, _num_line ));
		return true;
	}

	else if( _name.Upper() == wxT("@SCRIPT") )
		Add_SubObj( new ob_script( line, _num_line ));

	// It's the ending of the frame
	else if( _name.Upper() == wxT("FRAME") )
	{
		SetData( line, _num_line );
		// Have to reset the type
		SetType( OB_TYPE_FRAME );
		ungry = false;
		AddNbLine(1);
		return true;
	}

	// error or normal case -> set the object as a sub-object (may be the validation are wrong ?!)
	ob_entity *temp = new ob_entity(line, _num_line);
	Add_SubObj( temp );
	return true;
}

//-----------------------------------------------------------------
wxString 
ob_frame::ToStr()const
{
	wxString res;
	//--------------------------------------------------------
	// Print prefixes
	ob_object *temp = first_prefix;
	while( temp != NULL )
	{
		res += temp->ToStr();
		temp = temp->next;
	}

	//--------------------------------------------------------
	// Get the previous frame
	ob_frame* prev_frame = NULL;
	ob_object* t = prev;
	while( t != NULL )
	{
		if( t->type == OB_TYPE_FRAME )
		{
			prev_frame = (ob_frame*) t;
			break;
		}
		t = t->prev;
	}

	//--------------------------------------------------------
	// Do not echo props which translate and which are equal with previous frame
	temp = first_subobj;
	while( temp != NULL )
	{
		bool b_pass = false;
		// Skip some redondant property in regard with the previous frame
		if( prev_frame != NULL )
		{
			bool b_found = false;
			
			// Check with some translatable props
			for( int i=0; i < t_size_of( _to_check_with_prevframe ); i++ )
			{
				if( temp->name.Upper() == _to_check_with_prevframe[i].Upper() )
				{
					if( UnTokenize( prev_frame->GetSubObject(_to_check_with_prevframe[i])).Upper()
						==
						UnTokenize( temp ).Upper()
					   )
					{
						b_found = true;
						b_pass = true;
						break;
					}
				}
			}
			
			// Check with the attacks
			if( b_found == false && temp->type == OB_TYPE_FRAME_ATTACK )
			{
				ob_frame_attack* p_att =
					(ob_frame_attack*) prev_frame->GetSubObject( temp->name );
				if( p_att == NULL && temp->name == wxT("ATTACK") )
					p_att = (ob_frame_attack*) prev_frame->GetSubObject( wxT("ATTACK1") );
				if( p_att == NULL && temp->name == wxT("ATTACK1") )
					p_att = (ob_frame_attack*) prev_frame->GetSubObject( wxT("ATTACK") );
					
				if( p_att != NULL && p_att->IsEqual( temp )== true )
					b_pass =true;
			}
		}
		if( ! b_pass )
			res += temp->ToStr();
		temp = temp->next;
	}

	if( !dummy )
	{
		if( nb_tokens > 0 )
			res += GetPrefixFor(0) + name + GetPrefixFor(1) + arr_token[0] + GetComment() + wxT("\n");
		// Do not print empty frame
		else
			res += /*GetPrefixFor(0) + name + */GetComment() + wxT("\n");
	}
	return res;
}

//-------------------------------------------------------------
bool 
ob_frame::Add_SubObj( ob_object* _obj )
{
	if( ob_object::Add_SubObj( _obj ) == false )
		return false;
	
	if( _obj->type == OB_TYPE_FRAME_ATTACK )
	{
		// Have to check that dropv is after the attack
		ob_object* dropv = GetSubObject( wxT("dropv") );
		if( dropv == NULL )
			return true;
		
		//Move the dropv after the attack
		_obj->InsertObject_After( dropv );
	}
	return true;
}

//-------------------------------------------------------------
bool 
ob_frame::Mergeable( ob_object *tomerge, bool b_after )
{
	ob_object* temp = tomerge->GetObject_Group_First();

	while( temp != NULL )
	{
		if( temp->type == OB_TYPE_ANIM )
		{
			ObjectsLog( MYLOG_ERROR, temp->num_line,
				    wxT("Can't merge the ANIM ") + temp->GetToken(0) +
				    wxT(" to the FRAME ") + GifPath().GetFullName() + wxT(" !!") );
			return false;
		}
		else if( temp->type == OB_TYPE_FRAME )
		{
			// Check non mergeable cases
			if( GifPath() != wxString() && ((ob_frame*) temp)->GifPath() != wxString() )
			{
				ObjectsLog( MYLOG_ERROR, temp->num_line,
					    wxT("Can't merge the valid Frame ") + ((ob_frame*) temp)->GifPath().GetFullName() +
					    wxT(" to the other valid FRAME ") + GifPath().GetFullName() + wxT(" !!") );
			    return false;
			}
			else if( b_after && ((ob_frame*) temp)->GifPath() == wxString() )
			{
				ObjectsLog( MYLOG_ERROR, temp->num_line,
					    wxT("Can't merge a non valid Frame after the FRAME <") + GifPath().GetFullName() + wxT("> !!") );
			    return false;
			}
			else if( ! b_after && GifPath() == wxString() )
			{
				ObjectsLog( MYLOG_ERROR, temp->num_line,
					    wxT("Can't merge the Frame ") + ((ob_frame*) temp)->GifPath().GetFullName() +
					    wxT(" before the a valid FRAME !!") );
			    return false;
			}
		}
		temp = temp->next;
	}
	return true;
}


//----------------------------------------------------------------
bool 
ob_frame::Absorb( ob_object* tomerge, bool b_after )
{
	if( tomerge == NULL )
		return false;

	if( tomerge == this )
		return false;

	if( tomerge->parent == this )
		return false;

	if( ! Mergeable( tomerge, b_after ))
		return false;

	tomerge->Detach(true);
	if( tomerge->type == OB_TYPE_FRAME )
	{
		// merge all subobjs
		ob_object* temp = b_after ? tomerge->first_subobj : tomerge->last_subobj;
		while( temp != NULL )
		{
			ob_object *_next = b_after ? temp->next : temp->prev;

			if( temp->type == OB_TYPE_ANIM ||  temp->type == OB_TYPE_FRAME )
				ObjectsLog( MYLOG_ERROR, GetLineRef(),
					    wxT("Frame or Anim <") + temp->name + wxT("> found as subobj of ") + name + wxT(" !!") );

			Absorb( temp, b_after );

			temp = _next;
		}

		edited = tomerge->edited || edited;

		// Overwrite frame data if the merge object is after this
		if( b_after )
		{
			SetName( tomerge->name );
			num_line = tomerge->num_line;
			referer = tomerge->referer;
			SetNbTokens( 0 );
			for( size_t i =0; i < temp->arr_token_size; i++)
				SetToken( i, temp->arr_token[i] );
		}
	}

	// basic case
	else
	{
		if( b_after )
			Add_SubObj(tomerge);
		else
			Prepend_SubObj(tomerge);
	}
	return true;
}

int ob_frame::GetLineRef()const
{
	return num_line - nb_line + 2;
}

wxImage* ob_frame::GetImage()
{
	if( img != NULL )
		return img;

	wxString _path = GifPath().GetFullPath();
	if( ! wxFileName(_path).FileExists() )
		return NULL;

	if( IsFileEmpty( _path))
		return NULL;

	img = new wxImage( _path );
	if( ! img->IsOk() )
	{
		delete img;
		img = NULL;
	}
	return img;
}


void ob_frame::RemoveImage()
{
	if( img != NULL )
		delete img;
	img = NULL;

	if( png_or_gif != NULL )
		delete png_or_gif;
	png_or_gif = NULL;
}


int ob_frame::Get_ColorIndex( const int x, const int y )
{
	if( png_or_gif != NULL )
		return png_or_gif->GetIndex( x, y );

	wxFileName fn = GifPath();
	wxString str_path = fn.GetFullPath();
	if( str_path == wxString() )
		return -1;

	if( ! fn.FileExists() )
		return -1;

	// Check if the file is Zero Size
	if( IsFileEmpty( str_path))
		return -1;

	if( fn.GetExt().Upper() == wxT("PNG") )
		png_or_gif = new wxIndexedPNG( str_path );

	else if( fn.GetExt().Upper() == wxT("GIF") )
		png_or_gif = new wxIndexedGIF( str_path );

	else
		return -1;

	if( ! png_or_gif->IsOk() )
	{
		delete png_or_gif;
		png_or_gif = NULL;
		return -1;
	}

	return png_or_gif->GetIndex( x, y );
}


/**
 * Get the number of avalaible  attacks for this frame
 */
int ob_frame::GetAttacks_Count()
{
	int count = 0;
	ob_object* t = first_subobj;
	while( t != NULL )
	{
		if( t->type == OB_TYPE_FRAME_ATTACK )
			count++;
		t = t->next;
	}
	return count;
}

/**
 * Get the number of avalaible  attacks for this frame
 */
list<ob_frame_attack*>
ob_frame::GetAttacks()
{
	list<ob_frame_attack*> res;
	ob_object* t = first_subobj;
	while( t != NULL )
	{
		if( t->type == OB_TYPE_FRAME_ATTACK )
			res.push_back( (ob_frame_attack*)t );
		t = t->next;
	}
	return res;
}

/**
 * Get the first attacks avalaible, and return its name
 */
wxString
ob_frame::GetFirstAvalaibleAttack_name()
{
	ob_object* t = first_subobj;
	while( t != NULL )
	{
		if( t->type == OB_TYPE_FRAME_ATTACK )
			return t->name;
		t = t->next;
	}
	return wxString();
}

//------------------------------------------------------------
#define FRCL_GET_TOKEN( i )				\
	(	subobj->GetToken(i) != wxString()		\
		? StrToInt( subobj->GetToken(i) )	\
		: NO_CLONED_VAL				\
	)
			
void 
ob_frame::UpdateClonedDatas(ob_frame* prev)
{
	ob_object* subobj;
	// Delay
	subobj = GetSubObject(wxT("DELAY"));
	if( subobj != NULL )
	{
		b_delay_cloned = false;
		delay = FRCL_GET_TOKEN(0);
	}
	else
	{
		if( prev == NULL )
		{
			b_delay_cloned = false;
			delay = NO_CLONED_VAL;
		}
		else
		{
			b_delay_cloned = true;
			delay = prev->delay;
		}
	}			

	// Offset
	subobj = GetSubObject(wxT("Offset"));
	if( subobj != NULL )
	{
		b_offset_cloned = false;
		for( size_t i = 0; i < OFF_TOKS_COUNT; i++ )
			offset[i] = FRCL_GET_TOKEN(i);
	}
	else
	{
		if( prev == NULL )
		{
			b_offset_cloned = false;
			for( size_t i = 0; i < OFF_TOKS_COUNT; i++ )
				offset[i] = NO_CLONED_VAL;
		}
		else
		{
			b_offset_cloned = true;
			for( size_t i = 0; i < OFF_TOKS_COUNT; i++ )
				offset[i] = prev->offset[i];
		}
	}

	// Move
	subobj = GetSubObject(wxT("Move"));
	if( subobj != NULL )
	{
		b_move_cloned = false;
		move = FRCL_GET_TOKEN(0);
	}
	else
	{
		if( prev == NULL )
		{
			b_move_cloned = false;
			move = 0;
		}
		else
		{
			b_move_cloned = true;
			move = prev->move;
		}
	}			

	// MoveZ
	subobj = GetSubObject(wxT("MoveZ"));
	if( subobj != NULL )
	{
		b_moveZ_cloned = false;
		moveZ = FRCL_GET_TOKEN(0);
	}
	else
	{
		if( prev == NULL )
		{
			b_moveZ_cloned = false;
			moveZ = 0;
		}
		else
		{
			b_moveZ_cloned = true;
			moveZ = prev->moveZ;
		}
	}			

	// MoveA
	subobj = GetSubObject(wxT("MoveA"));
	if( subobj != NULL )
	{
		b_moveA_cloned = false;
		moveA = FRCL_GET_TOKEN(0);
	}
	else
	{
		if( prev == NULL )
		{
			b_moveA_cloned = false;
			moveA = 0;
		}
		else
		{
			b_moveA_cloned = true;
			moveA = prev->moveA;
		}
	}			

	// BBox
	subobj = GetSubObject(wxT("BBOX"));
	if( subobj != NULL )
	{
		b_bBox_cloned = false;
		for( size_t i = 0; i < BBOX_TOKS_COUNT; i++)
			bBox[i] = FRCL_GET_TOKEN(i);
	}
	else
	{
		if( prev == NULL )
		{
			b_bBox_cloned = false;
			for( size_t i = 0; i < BBOX_TOKS_COUNT; i++)
				bBox[i] = NO_CLONED_VAL;
		}
		else
		{
			b_bBox_cloned = true;
			for( size_t i = 0; i < BBOX_TOKS_COUNT; i++)
				bBox[i] = prev->bBox[i];
		}
	}

	// attBox
	list<ob_frame_attack*> ob_atts = GetAttacks();
	if( ob_atts.size() > 0 )
	{
		subobj = ob_atts.front();
		b_attBox_cloned = false;
		attName = subobj->name;
		for( size_t i = 0; i < ATT_TOKS_COUNT; i++)
			attBox[i] = FRCL_GET_TOKEN(i);
	}
	else
	{
		if( prev == NULL )
		{
			b_attBox_cloned = false;
			attName = wxT("Attack1");
			for( size_t i = 0; i < ATT_TOKS_COUNT; i++)
				attBox[i] = NO_CLONED_VAL;
		}
		else
		{
			b_attBox_cloned = true;
			attName = prev->attName;
			for( size_t i = 0; i < ATT_TOKS_COUNT; i++)
				attBox[i] = prev->attBox[i];
		}
	}

	// DrawMethod
	subobj = GetSubObject(wxT("DRAWMETHOD"));
	if( subobj != NULL )
	{
		b_drawMethod_cloned = false;
		for( size_t i = 0; i < DM_TOKS_COUNT; i++)
			drawMethod[i] = FRCL_GET_TOKEN(i);
	}
	else
	{
		if( prev == NULL )
		{
			b_drawMethod_cloned = false;
			for( size_t i = 0; i < DM_TOKS_COUNT; i++)
				drawMethod[i] = NO_CLONED_VAL;
		}
		else
		{
			b_drawMethod_cloned = true;
			for( size_t i = 0; i < DM_TOKS_COUNT; i++)
				drawMethod[i] = prev->drawMethod[i];
		}
	}

	// noDrawMethod
	subobj = GetSubObject(wxT("NODRAWMETHOD"));
	if( subobj != NULL )
	{
		b_noDrawMethod_cloned = false;
		noDrawMethod = (FRCL_GET_TOKEN(0)!=0);
	}
	else
	{
		if( prev == NULL )
		{
			b_noDrawMethod_cloned = false;
			noDrawMethod = false;
		}
		else
		{
			b_noDrawMethod_cloned = true;
			noDrawMethod = prev->noDrawMethod;
		}
	}

	// platform
	subobj = GetSubObject(wxT("platform"));
	if( subobj != NULL )
	{
		b_platform_cloned = false;
		for( size_t i = 0; i < PLAT_TOKS_COUNT; i++)
			platform[i] = FRCL_GET_TOKEN(i);
	}
	else
	{
		if( prev == NULL )
		{
			b_platform_cloned = false;
			for( size_t i = 0; i < PLAT_TOKS_COUNT; i++)
				platform[i] = NO_CLONED_VAL;
		}
		else
		{
			b_platform_cloned = true;
			for( size_t i = 0; i < PLAT_TOKS_COUNT; i++)
				platform[i] = prev->platform[i];
		}
	}

	// Repercute in all next frames
	ob_frame* next_fr = Get_NextFrame();
	if( next_fr != NULL )
		next_fr->UpdateClonedDatas(this);
}

/*
//------------------------------------------------------------
void 
ob_frame::Set_Cloned_To_Static()
{
	ob_object* subobj;
	// Delay
	if( b_delay_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("Delay");
		subobj->SetToken( 0, IntToStr(delay) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			

	// Offset
	if( b_offset_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("offset");
		for( size_t i = 0; i < OFF_TOKS_COUNT; i++ )
			subobj->SetToken( i, IntToStr(offset[i] ));
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			

	// Move
	if( b_move_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("move");
		subobj->SetToken( 0, IntToStr(move) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			

	// MoveZ
	if( b_moveZ_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("moveZ");
		subobj->SetToken( 0, IntToStr(moveZ) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			

	// MoveA
	if( b_moveA_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("moveA");
		subobj->SetToken( 0, IntToStr(moveA) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			

	// BBox
	if( b_bBox_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("BBox");
		for( size_t i = 0; i < BBOX_TOKS_COUNT; i++)
			subobj->SetToken( i, IntToStr(bBox[i]) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			


	// AttBox
	if( b_attBox_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_frame_attack();
		subobj->SetName( attName );
		for( size_t i = 0; i < ATT_TOKS_COUNT; i++)
			subobj->SetToken( i, IntToStr(attBox[i]) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}
	
	// DrawMethod
	if( b_drawMethod_cloned == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("DrawMethod");
		for( size_t i = 0; i <DM_TOKS_COUNT; i++)
			subobj->SetToken( i, IntToStr(drawMethod[i]) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			
	
	// noDrawMethod
	if( b_noDrawMethod_cloned == true && noDrawMethod == true )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("NoDrawMethod", "1");
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			
	
	// platform
	if( b_platform_cloned )
	{
		theHistoryManager.Set_State(false);
		subobj = new ob_object("platform");
		for( size_t i = 0; i < PLAT_TOKS_COUNT; i++)
			subobj->SetToken( i, IntToStr(platform[i]) );
		theHistoryManager.Set_State(true);
		Add_SubObj( subobj );
	}			
}
*/

//------------------------------------------------------------
ob_frame* 
ob_frame::Get_PreviousFrame()
{
	ob_object* _prev_frame = prev;
	while( _prev_frame != NULL )
	{
		if( _prev_frame->type == OB_TYPE_FRAME )
			return (ob_frame*) _prev_frame;
		_prev_frame = _prev_frame->prev;
	}
	return NULL;
}

//------------------------------------------------------------
ob_frame* 
ob_frame::Get_NextFrame()
{
	ob_object* next_frame = next;
	while( next_frame != NULL )
	{
		if( next_frame->type == OB_TYPE_FRAME )
			return (ob_frame*) next_frame;
		next_frame = next_frame->next;
	}
	return NULL;
}


/***************************
******** FRAME ATTACK OBJECT
********************/
wxString attacks__declare_tags[] = { wxT("ATTACK"), wxT("BLAST"), wxT("SHOCK"), wxT("BURN"), wxT("FREEZE"), wxT("STEAL") };

bool 
ob_frame_attack::IsAttack_Declare_Tag( const wxString& _prop_name )
{
	for( int i =0; i < t_size_of( attacks__declare_tags); i++ )
		if( attacks__declare_tags[i].Upper() == _prop_name.Upper() )
			return true;


	//Can also be a attack[NUM] tag
	if( _prop_name.Len() <= 6 ) // 6 == "attack".Len()
		return false;
	
	if( _prop_name.Left(6).Upper() != wxT("ATTACK") )
		return false;
	
	if( StrIsUInt( _prop_name.Right( _prop_name.Len() - 6 ) ) == true )
		return true;
	
	return false;
}


//------------------------------------------------------------------------------
bool 
ob_frame_attack::IsAttack_Tag( const wxString& _prop_name )
{
	if( IsAttack_Declare_Tag( _prop_name ) == true )
		return true;
	return IsAttack_SubTag( _prop_name );
}

//------------------------------------------------------------------------------
wxString attacks__sub_tags[] = {};

bool 
ob_frame_attack::IsAttack_SubTag( const wxString& _prop_name )
{
	for( int i =0; i < t_size_of( attacks__sub_tags); i++ )
		if( attacks__sub_tags[i].Upper() == _prop_name.Upper() )
			return true;

	return false;
}


//------------------------------------------------------------------------------
ob_frame_attack::ob_frame_attack( MyLine* line, const int num_line )
: ob_entity( line, num_line )
{
	ungry = true;
	type = OB_TYPE_FRAME_ATTACK;
}

//------------------------------------------------------------------------------
ob_frame_attack::ob_frame_attack( const wxString& _name, wxArrayString& _tokens)
:ob_entity( _name, _tokens )
{
	type = OB_TYPE_FRAME_ATTACK;
}

//-----------------------------------------------------------------
void
ob_frame_attack::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_frame();
	ob_entity::pClone( res );
}

//------------------------------------------------------------------------------
void
ob_frame_attack::SetName( const wxString& _name )
{
	wxString t = _name;
	if( t.Upper() == wxT("ATTACK"))
		t = t + wxT("1");
	return ob_object::SetName(t);
}

//------------------------------------------------------------------------------
ob_frame_attack::~ob_frame_attack()
{
}

//------------------------------------------------------------------------------
bool 
ob_frame_attack::IsEqual( ob_object* o )
{
	
	return UnTokenize(this).Upper() == UnTokenize(o).Upper();
}

//------------------------------------------------------------------------------
bool 
ob_frame_attack::Eat( MyLine* _line, const int _num_line)
{
	if( ! ungry )
		return false;

	// comment line
	if( _line->IsComment() || _line->IsEmpty() )
	{
		ob_object* temp = new ob_object( _line, _num_line );
		Add_SubObj( temp );

		// Still hungry
		return true;
	}

	wxString _name = *(_line->GetToken(0)->data);

	if( last_subobj != NULL && last_subobj->Eat(_line, _num_line) == true )
		return true;
	
	if( IsAttack_SubTag( _name ) == false )
	{
		ungry = false;
		return false;
	}

	// normal Eat case
	ob_entity *temp = new ob_entity(_line, _num_line);
	Add_SubObj( temp );
	return true;
}


/***************************
******** SCRIPT OBJECT
********************/


ob_script::ob_script( MyLine* line, const int _num_line )
:ob_entity(line,_num_line)
{
	ungry = true;
	type = OB_TYPE_SCRIPT;
}

//-----------------------------------------------------------------
void
ob_script::pClone(ob_object*& res )
{
	if( res == NULL )
		res = new ob_script();
	ob_entity::pClone( res );
}

//-----------------------------------------------------------------
ob_script::~ob_script()
{
}

bool ob_script::Eat( MyLine* line, const int _num_line)
{
	if( ! ungry )
		return false;

	// comment line
	if( line->IsComment() || line->IsEmpty() )
	{
		ob_object* temp = new ob_object( line, _num_line );
		Add_SubObj( temp );

		// Still hungry
		return true;
	}


	wxString _name = *(line->GetToken(0)->data);

	if( ob_anim::IsAnimTag(_name) )
	{
		ObjectsLog( MYLOG_WARNING, _num_line + 1,
			    wxT("with <") + _name + wxT(">, a anim tag found although the last scripts didn't close !!") );
	   	dummy=true;
	   	ungry = false;
	    return false;
	}

	//Eat the line
	ob_entity *temp = new ob_entity(line, _num_line, false);
	Add_SubObj( temp );

	// Check if script is finish
	if( _name.Upper() == wxT("@END_SCRIPT") )
		ungry = false;

	return true;
}

