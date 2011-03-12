/*
 * common__ob_controls.cpp
 *
 *  Created on: 28 avr. 2009
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dir.h>
#include "common__ob_controls.h"
#include "common__ob_object.h"
#include "common__object_stage.h"

using namespace std;


//****************************************************
std::map<wxString,ob_property*> ob_props;
//****************************************************

//-------------------------------------------------------------
static
atom_path*
ANN__new_atom( const wxString& tag )
{
	atom_path* t = new atom_path;
	t->tag = tag;
	t->num_token = -1;
	return t;
}

//-------------------------------------------------------------
static
atom_path*
ANN__new_atom( int num_token )
{
	atom_path* t = new atom_path;
	t->tag = wxString();
	t->num_token = num_token;
	return t;
}

//-------------------------------------------------------------
static
atom_path*
ANN__new_atom( atom_path* at )
{
	if( at == NULL )
		return NULL;
	
	atom_path* t = new atom_path;
	t->tag = at->tag;
	t->num_token = at->num_token;
	return t;
}

//****************************************************
ob_token_path::ob_token_path( const wxString& tag, int num_token )
{
	l_atoms.push_back( ANN__new_atom(tag) );
	if( num_token >= 0 )
		l_atoms.push_back( ANN__new_atom(num_token) );
}

//-------------------------------------------------------------
ob_token_path&
ob_token_path::operator=( const ob_token_path& obtp )
{
	Clear_l_atoms();
	std::list<atom_path*>::const_iterator it(obtp.l_atoms.begin())
					,  it_end (obtp.l_atoms.end());
	for( ; it != it_end; it++ )
		l_atoms.push_back( ANN__new_atom( *it ));
	return *this;
}

//-------------------------------------------------------------
ob_token_path::ob_token_path(const ob_token_path& ob_tp, const wxString& subtag,int num_sub_token )
{
	*this = ob_tp;
	if( subtag != wxString() )
	{
		l_atoms.push_back( ANN__new_atom( subtag ) );
		if( num_sub_token >= 0 )
			l_atoms.push_back( ANN__new_atom(num_sub_token) );
	}
}

//-------------------------------------------------------------
ob_token_path::ob_token_path( ob_token_path& src )
{
	std::list<atom_path*>::iterator it(src.l_atoms.begin())
					,  it_end (src.l_atoms.end());
	for( ; it != it_end; it++ )
		l_atoms.push_back( ANN__new_atom( *it ));
}

//-------------------------------------------------------------
ob_token_path::~ob_token_path()
{
	Clear_l_atoms();
}

//-------------------------------------------------------------
void
ob_token_path::Clear_l_atoms()
{
	while( l_atoms.empty() == false )
	{
		atom_path* at = l_atoms.front();
		l_atoms.pop_front();
		if( at != NULL )
			delete at;
	}
}

//-------------------------------------------------------------
bool
ob_token_path::IsEmpty()
{
	return l_atoms.empty();
}

//-------------------------------------------------------------
wxString
ob_token_path::GetPath()
{
	wxString res;
	std::list<atom_path*>::iterator it(l_atoms.begin())
					,  it_end (l_atoms.end());
	for( ; it != it_end; it++ )
	{
		if( *it == NULL )
			continue;
		if( (*it)->tag != wxString() )
		{
			if( res.Len() == 0 )
				res += (*it)->tag;
			else
				res += wxT(">") + (*it)->tag;
		}
		else
		{
			res += wxT("=") + IntToStr((*it)->num_token );
			return res;
		}
	}
	return res;
}


//-------------------------------------------------------------
ob_token_path*
ob_token_path::BuildPath_Sibling_Token( int decal )
{
	if( l_atoms.size() < 2 )
		return NULL;
	ob_token_path* res = new ob_token_path;
	std::list<atom_path*>::iterator it(l_atoms.begin())
					,  it_end (l_atoms.end());
	int nb_tokens = 0;
	bool b_ok = false;
	for( ; it != it_end; it++ )
	{
		b_ok = false;
		if( *it == NULL )
			continue;
			
		res->l_atoms.push_back( ANN__new_atom(*it) );
		if( (*it)->tag == wxString() )
		{
			nb_tokens++;
			b_ok = true;
		}
	}
	
	if( nb_tokens != 1 || b_ok == false )
	{
		delete res;
		return NULL;
	}
	atom_path* last_atom = res->l_atoms.back();
	last_atom->num_token += decal;
	if( last_atom->num_token <= 0 )
	{
		delete res;
		return NULL;
	}

	return res;
}

//-------------------------------------------------------------
bool
ob_token_path::Append__SubToken( int num_sub_token )
{
	if( num_sub_token <= 0 )
		return false;
	if( l_atoms.size() < 1 )
		return false;
	if( l_atoms.back()->tag != wxString() )
		return false;
	l_atoms.push_back( ANN__new_atom( num_sub_token ));
	return true;
}

//-------------------------------------------------------------
bool
ob_token_path::Append__SubTag( const wxString& subtag )
{
	if( subtag == wxString() );
		return  false;
	if( l_atoms.back()->tag == wxString() )
		return false;
	l_atoms.push_back( ANN__new_atom( subtag ));
	return true;
}

//-------------------------------------------------------------
ob_object* 
ob_token_path::Resolve_With( ob_object* _o )
{
	ob_object* res = _o;
	std::list<atom_path*>::iterator it(l_atoms.begin())
					,  it_end (l_atoms.end());
	for( ; it != it_end; it++ )
	{
		if( res == NULL )
			return NULL;
		atom_path* at = (*it);
		if( at->tag != wxString() )
			res = res->GetSubObject( at->tag );
		else
			break; 
	}
	return res;
}

//-------------------------------------------------------------
wxString 
ob_token_path::Get_With( ob_object* _o )
{
	if( l_atoms.back()->tag != wxString() )
		return wxString();
	
	ob_object* subobj = Resolve_With( _o );
	if( subobj == NULL )
		return wxString();
	return subobj->GetToken( l_atoms.back()->num_token );
}

//-------------------------------------------------------------
bool     
ob_token_path::Set_To( ob_object* _o, wxString& val )
{
	// FIXME ???
	return false;
}

	
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************

//*********** new event
DEFINE_EVENT_TYPE(wxEVT_OBPROP_CHANGE)

wxColour ob_property::default_values_color( 200,200,255);

//****************************************************
ob_property::ob_property(  const wxString& _tag,const int _num_token
			, int _type_prop
			, const wxString& _def_val, int _do_on_default )
{
	int ind_dblpt = _tag.Find( wxT(":") );
	if( ind_dblpt != wxNOT_FOUND )
	{
		prefix = _tag.Left(ind_dblpt);
		tag = _tag.Right(_tag.Len()-ind_dblpt-1);
	}
	else
	{
		prefix = wxString();
		tag = _tag;
	}
	num_token = _num_token;
	type_prop = _type_prop;
	def_val = _def_val;
	do_on_default = _do_on_default;
}


//-------------------------------------------------------------
ob_property::~ob_property()
{
}

//-------------------------------------------------------------
void 
ob_property::SetRange(wxWindow* _comboBox, int min,int max)
{
	if( max < min )
		return;
	
	wxComboBox* comboBox = (wxComboBox*) _comboBox;
	comboBox->Clear();
	
	for( int i = min; i <= max; i++ )
	{
		wxStringClientData *_data = new wxStringClientData();
		_data->SetData( IntToStr( i ) );
		comboBox->Append( IntToStr( i ), _data );
	}
}

//-------------------------------------------------------------
void 
ob_property::SetEnums(wxWindow* _comboBox, const wxArrayString& _displayed, const wxArrayString& _values)
{
	if( _displayed.Count() != _values.Count() )
	{
		wxMessageBox( wxT("BUG!!!\nob_property::SetEnums\n counts doesn't match !\n") );
		return;
	}
	
	wxComboBox* comboBox = (wxComboBox*) _comboBox;
	comboBox->Clear();
	
	for( size_t i = 0; i < _displayed.Count(); i++ )
	{
		wxStringClientData *_data = new wxStringClientData();
		_data->SetData( _values[i] );
		comboBox->Append( _displayed[i], _data );
	}
}

//-------------------------------------------------------------
wxSizer*
ob_property::BuildControls(
		  ob_object* obj
		, wxWindow* parent
		, wxWindow*& theCtrl
		, const wxString& ctrlName, int ctrl_w
		, const wxString& str_label, int label_w
		, int more_styles 
		)
{
	wxValidator* validator = NULL;
	int _type;
	switch( type_prop )
	{
		case	PROPTYPE_STRING:
			_type = 0;
			validator = (wxValidator*) wxDefaultValidator.Clone();
			break;
		case	PROPTYPE_STRING_NO_WS:
			_type = 0;
			validator = new wxTextValidator_NoWhiteSpace();
			break;
		case	PROPTYPE_NUMBER:
			_type = 0;
			validator = new  wxValidator_Restrict_Range('0','9');
			break;
		case	PROPTYPE_NEGATIVENUMBER:
			_type = 0;
			validator = new wxValidator_Restrict_Range('0','9');
			break;
		case	PROPTYPE_RELATIVENUMBER:
			_type = 0;
			validator = new wxValidatorIntegerRelative();
			break;
		case	PROPTYPE_FLOAT:
			_type = 0;
			validator = new wxValidatorFloat();
			break;
		case	PROPTYPE_FLOAT_POSITIVE:
			_type = 0;
			validator = new wxValidatorFloatPositive();
			break;
			
			
		case	PROPTYPE_OBFILE:
			_type = 1;
			break;
			
		case	PROPTYPE_BOOL:
		case	PROPTYPE_BOOL_EXSISTENCE:
			_type = 2;
			break;
		case	PROPTYPE_ENUMS:
		case	PROPTYPE_RANGE:
			_type = 3;
			break;
			
		default:
			return NULL;
	}

	wxSizer*   sizer_sub = NULL;
	wxWindow*  label = NULL;
	theCtrl = NULL;
	if( _type == 0 )
	{
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		
		if( str_label != wxString() )
		{
			label = new wxStaticText( parent, wxID_ANY, str_label
							,wxDefaultPosition,wxSize(label_w,-1) );
			sizer_sub->Add(label, 0, wxALIGN_CENTER );
		}
		
		theCtrl =  new wxTextCtrl( 
					parent, wxID_ANY, wxString()
					, wxDefaultPosition, wxSize(ctrl_w,-1)
					, wxBORDER_SUNKEN|wxTE_PROCESS_ENTER |more_styles
					, *validator
					, ctrlName );
		theCtrl->Connect(
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(ob_property::Evt_TxtChange)
			, NULL, this );
		sizer_sub->Add( theCtrl );
	}
	
	else if( _type == 1 )
	{
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		if( str_label != wxString() )
		{
			label = new wxStaticText( parent, wxID_ANY, str_label
							,wxDefaultPosition,wxSize(label_w,-1)
							);
			sizer_sub->Add(label, 0, wxALIGN_CENTER );
		}
		
		theCtrl = new prObFileChooserCtrl(
						parent, wxID_ANY
						, wxString()
						, true
						, ctrl_w
						);
		theCtrl->SetName( ctrlName );
		theCtrl->Connect(
			  wxEVT_OBFILECHOOSER_CHANGE
			, wxCommandEventHandler(ob_property::Evt_FileChange)
			, NULL, this );
		sizer_sub->Add( theCtrl );
	}
	else if( _type == 2 )
	{
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		theCtrl =  new wxCheckBox( 
						  parent, wxID_ANY, str_label
						, wxDefaultPosition, wxSize(ctrl_w,-1)
						, wxBORDER_SUNKEN |more_styles
						, wxDefaultValidator
						, ctrlName );
		theCtrl->Connect(
			  wxEVT_COMMAND_CHECKBOX_CLICKED
			, wxCommandEventHandler(ob_property::Evt_ChckChange)
			, NULL, this );
		sizer_sub->Add( theCtrl );
	}
	else if( _type == 3 )
	{
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		label = new wxStaticText( parent, wxID_ANY, str_label
						,wxDefaultPosition,wxSize(label_w,-1) );
		sizer_sub->Add(label, 0, wxALIGN_CENTER );
		
		theCtrl =  new wxComboBox(
						 parent, wxID_ANY, wxString()
						, wxDefaultPosition, wxSize( ctrl_w, -1 )
						, 0, NULL
						, more_styles
						, wxDefaultValidator
						, ctrlName );
		theCtrl->Connect( 
			  wxEVT_COMMAND_COMBOBOX_SELECTED
			, wxCommandEventHandler(ob_property::Evt_Combo)
			, NULL, this );
		sizer_sub->Add( theCtrl );
	}
	
	if( theCtrl != NULL )
		theCtrl->SetClientData( (void*) obj );
	
	if( validator != NULL )
		delete validator;
	return sizer_sub;
}

//-------------------------------------------------------------
wxString 
ob_property::Get_SpecialDefault_Val(ob_object* _o )
{
	if( referer_path.IsEmpty() == true )
		return wxString();
	
	if( ob_stage::Is_Stage_Spawn_Type( _o ) == true )
	{
		ob_spawn* o = (ob_spawn*) _o;
		if( o->entity_ref == NULL || o->entity_ref->obj_container == NULL)
			return wxString();
		
		return referer_path.Get_With( o->entity_ref->obj_container );
	}

	return wxString();
}

//-------------------------------------------------------------
bool     
ob_property::Have_SpecialDefault_Val(ob_object* _o )
{
	if( referer_path.IsEmpty() == true )
		return false;
	if( ob_stage::Is_Stage_Spawn_Type( _o ) == true )
	{
		ob_spawn* o = (ob_spawn*) _o;
		if( o->entity_ref == NULL || o->entity_ref->obj_container == NULL)
			return false;
		
		return referer_path.Resolve_With( o->entity_ref->obj_container ) != NULL;
	}

	return false;
}

//-------------------------------------------------------------
wxString 
ob_property::Get_Default_Val(ob_object* _o )
{
	if( _o == NULL )
		return def_val;
	wxString res = Get_SpecialDefault_Val( _o );
	if( res != wxString() )
		return res;
	return def_val;
}

//-------------------------------------------------------------
wxString 
ob_property::Get_Curr_Value(ob_object* _o)
{
	if( tag.Upper() == _o->name.Upper() || tag == wxString() )
		return _o->GetToken( num_token );
	
	ob_object* subobj = _o->GetSubObject( tag );
	if( subobj == NULL  )
		return wxString();
	
	return subobj->GetToken( num_token );
}

//-------------------------------------------------------------
bool 
ob_property::Update_CtrlVal( wxWindow* ctrl )
{
	// Some sanity checks
	if( ctrl == NULL )
		return false;
	ob_object* obj = (ob_object*) ctrl->GetClientData();
	if( obj == NULL)
		return false;
	wxString w_name = ctrl->GetName();
	wxString typ_Ctrl = w_name.Left(2);
	if( 	   typ_Ctrl != wxT("TC")
		&& typ_Ctrl != wxT("CO")
		&& typ_Ctrl != wxT("CH")
		)
		return false;
	
	// Get the curr val for the object
	wxString val = Get_Curr_Value( obj );
	
	if( typ_Ctrl == wxT("TC") )
	{
		wxTextCtrl* t = ((wxTextCtrl*)ctrl);
		if( t->GetValue() == val )
			return true;
		obj->SetEdited( true );
		t->ChangeValue( val );
		return true;
	}
	
	if( typ_Ctrl == wxT("CO") )
	{
		wxComboBox* t = ((wxComboBox*)ctrl);
		int curr_ind = t->GetCurrentSelection();
		
		int new_ind = wxNOT_FOUND;
		for( size_t i = 0 ; i < t->GetCount() ; i ++)
		{
			wxStringClientData* scd = 
				(wxStringClientData*)t->GetClientObject(i);
			if( scd->GetData().Upper() == val.Upper() )
			{
				new_ind = i;
				break;
			}
		}
		if( new_ind == curr_ind )
			return true;
			
		obj->SetEdited( true );
		t->SetSelection( new_ind );
		return true;
	}
	
	if( typ_Ctrl == wxT("CH") )
	{
		wxCheckBox* t = ((wxCheckBox*)ctrl);
		if( t->GetValue() == (val == wxT("1")) )
			return true;
		
		obj->SetEdited( true );
		t->SetValue( ! t->GetValue() );
		return true;
	}
	
	return false;
}


//-------------------------------------------------------------
ob_object*
ob_property::CreateDefaultObject(ob_object* o)
{
	ob_object* subobj = new ob_object( tag );
	wxString pref = tag;
	if( prefix != wxString() )
		pref = prefix + wxT(":") + pref;
	// Insert default values
	for( int i = 0; i < num_token - 1; i ++ )
	{
		wxString t_tag = pref + wxT("=") + IntToStr(i);
		if( ob_props.find(t_tag) == ob_props.end() ||ob_props[t_tag] == NULL )
		{
			subobj->SetToken( i, wxT("0") );
		}
		else
		{
			subobj->SetToken( i, ob_props[t_tag]->Get_Default_Val(o) );
		}
	}
	subobj->SetToken( num_token, Get_Default_Val(o) );
	return subobj;
}

//-------------------------------------------------------------
void
ob_property::Signal_Modified( ob_object* obj, wxWindow* theCtrl)
{
	if( obj == NULL )
		return;

	obj->edited = true;
	if( obj->parent != NULL )
		obj->parent->edited = true;
	wxCommandEvent evt(wxEVT_OBPROP_CHANGE, theCtrl->GetId());
	theCtrl->ProcessEvent(evt);
}

//-------------------------------------------------------------
void
ob_property::MayTruncate(ob_object* obj, ob_object* subobj, wxWindow* theCtrl )
{
	size_t i = num_token + 1;
	wxString pref = tag;
	if( prefix != wxString() )
		pref = prefix + wxT(":") + pref;
	wxString t_tag = pref + wxT("=") + IntToStr(i);
	while( ob_props.find(t_tag) != ob_props.end() && ob_props[t_tag] != NULL )
	{
		// Subobject deny truncation
		if( ob_props[t_tag]->do_on_default != OBPROP_TRUNCATE )
			return;
		
		// Subobject is not on default val
		if( 	   ob_props[t_tag]->Get_Default_Val(obj)
			!= subobj->GetToken( i ) )
			return;
		
		i++;
		wxString t_tag = pref + wxT("=") + IntToStr(i);
	}

	// truncate the uppers tokens
	if( (int) subobj->nb_tokens > num_token )
	{
		Signal_Modified( obj, theCtrl );
		subobj->nb_tokens = num_token;
	}
	
	// truncate the lowers tokens
	for(  i = num_token;i >= 0; i-- )
	{
		wxString t_tag = pref + wxT("=") + IntToStr(i);
		if( ob_props.find(t_tag) != ob_props.end() && ob_props[t_tag] != NULL )
			return;

		if( ob_props[t_tag]->do_on_default != OBPROP_TRUNCATE )
			return;
		
		// Subobject is not on default val
		if( 	   ob_props[t_tag]->Get_Default_Val(obj)
			!= subobj->GetToken( i )
		  )
			return;
		
		if( subobj->nb_tokens > i-1 )
		{
			Signal_Modified( obj, theCtrl );
			subobj->nb_tokens = i-1;
		}
	}
}

//-------------------------------------------------------------
void
ob_property::pUpdate( ob_object* obj, wxWindow* theCtrl, const wxString& val )
{
	wxString prec_val;
	ob_object* subobj = NULL;
	if( tag.Upper() != obj->name.Upper() && tag != wxString() )
	{
		subobj = obj->GetSubObject( tag );
		if( subobj == NULL && val == Get_Default_Val(obj) )
			return;
	}
	else
		subobj = obj;
	
	if( subobj != NULL && val == Get_Default_Val(obj) )
	{
		switch (do_on_default & OBPROP_DEF_MASK)
		{
			case OBPROP_DEL_TAG:
				Signal_Modified( obj, theCtrl);
				subobj->Rm();
				return;
			case OBPROP_TRUNCATE:
				subobj->SetToken( num_token, val );
				MayTruncate(obj,subobj,theCtrl);
				return;
		}
	}
	
	if( subobj == NULL )
	{
		subobj = CreateDefaultObject(obj);
		obj->Add_SubObj( subobj );
		Signal_Modified( obj, theCtrl);
	}
	else
	{
		prec_val = subobj->GetToken( num_token );
		if( prec_val == val )
			return;
	}
	
	subobj->SetToken( num_token, val );
	Signal_Modified( obj, theCtrl);
}

//-------------------------------------------------------------
void 
ob_property::Evt_TxtChange(wxCommandEvent& evt)
{
	wxTextCtrl* _ctrl = (wxTextCtrl*) evt.GetEventObject();
	if( _ctrl == NULL)
		return;
	ob_object* obj = (ob_object*) _ctrl->GetClientData();
	if( obj == NULL)
		return;

	wxString val = _ctrl->GetValue();
	if( val == wxString() && (do_on_default & OBPROP_SETDEF_IFEMPTY) != 0 )
	{
		val = Get_Default_Val(obj);
		_ctrl->ChangeValue( val );
	}
	
	if( val == Get_SpecialDefault_Val(obj) && Have_SpecialDefault_Val(obj) == true )
		_ctrl->SetBackgroundColour( default_values_color );
	else
		_ctrl->SetBackgroundColour( *wxWHITE );
	
	pUpdate( obj, _ctrl, val );
	evt.Skip();
}


//-------------------------------------------------------------
void 
ob_property::Evt_FileChange(wxCommandEvent& evt)
{
	prObFileChooserCtrl* _ctrl = (prObFileChooserCtrl*) evt.GetEventObject();
	if( _ctrl == NULL)
		return;
	ob_object* obj = (ob_object*) _ctrl->GetClientData();
	if( obj == NULL)
		return;

	pUpdate( obj, _ctrl, _ctrl->GetObPath() );
	evt.Skip();
}

//-------------------------------------------------------------
void 
ob_property::Evt_ChckChange(wxCommandEvent& evt)
{
	wxCheckBox* _ctrl = (wxCheckBox*) evt.GetEventObject();
	if( _ctrl == NULL)
		return;
	ob_object* obj = (ob_object*) _ctrl->GetClientData();
	if( obj == NULL)
		return;

	pUpdate( obj, _ctrl, BoolToStr(_ctrl->GetValue()) );
	evt.Skip();
}


//-------------------------------------------------------------
void 
ob_property::Evt_Combo(wxCommandEvent& evt)
{
	wxComboBox* _ctrl = (wxComboBox*) evt.GetEventObject();
	if( _ctrl == NULL)
		return;
	ob_object* obj = (ob_object*) _ctrl->GetClientData();
	if( obj == NULL)
		return;

	if( _ctrl->GetSelection() == wxNOT_FOUND )
		pUpdate( obj, _ctrl, Get_Default_Val(obj) );
	else
	{
		wxStringClientData* _data = 
			(wxStringClientData*) _ctrl->GetClientObject(_ctrl->GetSelection());
		pUpdate( obj, _ctrl, _data->GetData() );
	}
	evt.Skip();
}


//****************************************************
//****************************************************
//****************************************************
KalachnikofButton::KalachnikofButton(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const size_t _clicks_interval, const wxPoint& pos, const wxSize& size , long style, const wxValidator& validator, const wxString& name )
:wxBitmapButton( parent, id, bitmap, pos,size,style,validator, name )
{
	clicks_interval = _clicks_interval;
	theTimer = new wxTimer( this, 1 );
	b_clickedOn = false;
}

//------------------------------------------------------------------------
KalachnikofButton::~KalachnikofButton()
{
	b_clickedOn = false;
	delete theTimer;
}

//------------------------------------------------------------------------
void 
KalachnikofButton::EvtTimerOut( wxTimerEvent& event )
{
	if( b_clickedOn == true )
	{
		// AutoProcess the event to permit Connect() functions to work
		wxCommandEvent _evt( wxEVT_COMMAND_BUTTON_CLICKED );
		_evt.SetEventObject( this );
		_evt.SetId( GetId() );
		GetEventHandler()->ProcessEvent( _evt );
		return;
	}
	
	theTimer->Stop();
}

//------------------------------------------------------------------------
void 
KalachnikofButton::EvtMouseSomething( wxMouseEvent& event )
{
	if( event.LeftDown() == true )
	{
		if( theTimer->IsRunning() == false )
		{
			b_clickedOn = true;
			theTimer->Start( clicks_interval );
		}
	}
	else if( event.Leaving() == true || event.LeftUp() == true )
	{
		b_clickedOn = false;
		theTimer->Stop();
	}

	event.Skip();
}

//------------------------------------------------------------------------
BEGIN_EVENT_TABLE(KalachnikofButton, wxBitmapButton)
	EVT_MOUSE_EVENTS( KalachnikofButton::EvtMouseSomething )
	EVT_TIMER(1, KalachnikofButton::EvtTimerOut)
END_EVENT_TABLE()


//****************************************************
//****************************************************
//****************************************************
//*********** new event
DEFINE_EVENT_TYPE(wxEVT_OBFILECHOOSER_CHANGE)


//****************************************************

const int fileTextMinW = 150;
const int fileTextMaxH = 50;
const int filePickerW = 30;
const int filePickerH = 30;

//****************************************************
//****************************************************
// Event table

BEGIN_EVENT_TABLE(prObFileChooserCtrl, wxControl)
	EVT_PAINT(prObFileChooserCtrl::EvtPaint)
	EVT_SIZE(prObFileChooserCtrl::EvtSize)
	EVT_SET_FOCUS(prObFileChooserCtrl::EvtGetFocus)
END_EVENT_TABLE()

//****************************************************
//****************************************************
//****************************************************

prObFileChooserCtrl::prObFileChooserCtrl(
			wxWindow *_parent
			, const int _id
			, const wxString& _File_or_Directory
			, const bool _b_file_must_exist
			, const int _width
			, const int _style
			)
:wxControl( _parent, _id, wxDefaultPosition, wxDefaultSize,
				_style | wxBORDER_SIMPLE | wxTE_PROCESS_TAB
				)
{
	b_file_must_exist = _b_file_must_exist;
	File_or_Directory = _File_or_Directory;
	b_init = false;
	b_updating = false;

	//******************************************
	txtCtrl_file = new wxTextCtrl(this, wxID_ANY, wxString(),wxDefaultPosition, wxDefaultSize,
			wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB | wxTE_RIGHT );
	txtCtrl_file->SetInsertionPointEnd();
	txtCtrl_file->Connect( wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(prObFileChooserCtrl::EvtCharPress), NULL, this );

	if( _width == wxDefaultCoord )
		minWidth = fileTextMinW + filePickerW;
	else
		minWidth = _width;

	int txtW = minWidth - filePickerW;

	txtCtrl_file->SetMinSize( wxSize( txtW,  wxDefaultCoord) );
	txtCtrl_file->SetSize( wxSize( txtW,  wxDefaultCoord) );
	txtCtrl_file->Move( 0, 0 );


	//******************************************
	int fp_style = wxFLP_OPEN | wxFLP_CHANGE_DIR;
	if( _b_file_must_exist )
		fp_style |= wxFLP_FILE_MUST_EXIST;
	filePck_file = new wxFilePickerCtrl( this, wxID_ANY,
					     dataDirPath.GetFullPath(), wxT("Select custom selection screen file"), wxT("*.*"),
						wxDefaultPosition, wxDefaultSize, fp_style );
	filePck_file->SetMaxSize( wxSize(30, wxDefaultCoord ));
	filePck_file->SetInitialSize(wxSize(filePickerW, filePickerH ));
	filePck_file->Move( txtW, 0 );

	filePck_file->Connect(wxEVT_COMMAND_FILEPICKER_CHANGED , wxFileDirPickerEventHandler(prObFileChooserCtrl::EvtImgPickerChg), NULL, this );
	filePck_file->Connect(wxEVT_ENTER_WINDOW , wxMouseEventHandler(prObFileChooserCtrl::EvtImgPickerClick), NULL, this );

}


//****************************************************

prObFileChooserCtrl::~prObFileChooserCtrl()
{
}


//****************************************************

void prObFileChooserCtrl::EvtSize(wxSizeEvent& event)
{
//	Init();
	wxSize _s = event.GetSize();
	DoSetSize( wxDefaultCoord, wxDefaultCoord, _s.GetWidth(), _s.GetHeight() );

	event.Skip();
}


//****************************************************

void prObFileChooserCtrl::EvtGetFocus( wxFocusEvent& evt )
{
	txtCtrl_file->SetFocus();
	evt.Skip();
}


//****************************************************

void prObFileChooserCtrl::EvtCharPress( wxKeyEvent& event )
{
	int kc = event.GetKeyCode();
	if( kc == WXK_TAB )
	{
		wxNavigationKeyEvent new_event;
		new_event.SetEventObject( GetParent() );
		new_event.SetDirection( !event.ShiftDown() );
		// CTRL-TAB changes the (parent) window, i.e. switch notebook page
		new_event.SetWindowChange( event.ControlDown() );
		new_event.SetCurrentFocus( this );
		GetParent()->GetEventHandler()->ProcessEvent( new_event );
		return;
	}
	else if( kc == WXK_RETURN || kc == WXK_NUMPAD_ENTER )
	{
		int err = Update(false);
		if( err != 0 )
		{
			if( err == 1 )
				wxMessageBox( wxT("Not a valid OB file !"), wxT("WARNING"), wxOK | wxICON_EXCLAMATION, this );
			else if( err == 2 )
				wxMessageBox( wxT("File doesn't exist !"), wxT("WARNING"), wxOK | wxICON_EXCLAMATION, this );

			txtCtrl_file->SetBackgroundColour( wxColour( 255, 200,200));
		}
		else
			txtCtrl_file->SetBackgroundColour( wxColour( 255, 255,255));
		return;
	}

	event.Skip();
}


//****************************************************

void prObFileChooserCtrl::EvtImgPickerChg(wxFileDirPickerEvent& event )
{
	int err = Update(true);
	if( err != 0 )
	{
		if( err == 1 )
			wxMessageBox( wxT("Not a valid OB file !"), wxT("WARNING"), wxOK | wxICON_EXCLAMATION, this );
		else if( err == 2 )
			wxMessageBox( wxT("File doesn't exist !"), wxT("WARNING"), wxOK | wxICON_EXCLAMATION, this );
	}
}


//****************************************************

void prObFileChooserCtrl::EvtPaint(  wxPaintEvent& event )
{
//	Init();
	event.Skip();
}


//****************************************************

void prObFileChooserCtrl::EvtImgPickerClick( wxMouseEvent& event )
{
	Init();
	event.Skip();
}

//****************************************************
// EVENTs END
//****************************************************

void prObFileChooserCtrl::DoSetSize(int x, int y, int width, int height)
{
	int txtW = width - filePickerW;
	if( txtW <= 0)
		txtW = 1;

	int _th = (height > fileTextMaxH ) ? fileTextMaxH : height;
	txtCtrl_file->SetSize( txtW, _th );
	if( _th < height )
		txtCtrl_file->Move( 0, (height - _th) / 2 );

	int fpY = (height - filePickerH) /2 ;
	if( fpY < 0 )
		fpY = 0;
	filePck_file->Move( txtW, fpY );
	wxControl::DoSetSize( x,y,width,height );

//	wxMessageBox( "W : " + IntToStr( width ) + "\nH" + IntToStr( height ) );
}


//****************************************************

wxSize prObFileChooserCtrl::DoGetBestSize() const
{
	return wxSize( fileTextMaxH, filePickerH );
}

//****************************************************
wxSize 
prObFileChooserCtrl::GetMinSize() const
{
	return wxSize( minWidth,  filePickerH);
}



//****************************************************
int
prObFileChooserCtrl::Update(bool b_filePicker )
{
    if( b_updating == true )
        return true;
    b_updating = true;

	// Get the new file
	wxString fn;
	if( b_filePicker )
	{
		fn = Convert_To_Ob_Path( filePck_file->GetPath() );

		// Not a valid obpath
		if( fn == wxString() )
		{
		    b_updating = false;
			return 1;
		}

		txtCtrl_file->SetValue( fn );
	}
	else
	{
		fn = txtCtrl_file->GetValue();

		wxFileName wfn = GetObFile( fn );
		wxString f_p = wfn.GetFullPath();

		if( b_file_must_exist && wfn.FileExists() == false )
		{
		    b_updating = false;
			return 2;
		}

		filePck_file->SetPath( f_p );
	}

	File_or_Directory = filePck_file->GetPath();
	
	// AutoProcess the event to permit Connect() functions to work
	wxCommandEvent _evt( wxEVT_OBFILECHOOSER_CHANGE );
	_evt.SetEventObject( this );
	_evt.SetId( GetId() );
	GetEventHandler()->ProcessEvent( _evt );
	
	b_updating = false;
	return 0;
}


//****************************************************
bool 
prObFileChooserCtrl::SetFullPath( const wxString& _file )
{
	return SetObPath( Convert_To_Ob_Path( _file ) );
}


//****************************************************

bool prObFileChooserCtrl::SetObPath( const wxString& _file )
{
	wxString prev_fn = txtCtrl_file->GetValue();
	txtCtrl_file->SetValue( _file );
	if( Update(false) != 0 )
	{
		txtCtrl_file->SetValue( prev_fn );
		return false;
	}
	return true;
}

//****************************************************

wxFileName prObFileChooserCtrl::GetFileName()
{
	return GetObFile( txtCtrl_file->GetValue() );
}


//****************************************************

wxString prObFileChooserCtrl::GetFullPath()
{
	return Convert_To_Ob_Path( txtCtrl_file->GetValue() );
}


//****************************************************

wxString prObFileChooserCtrl::GetObPath()
{
	return txtCtrl_file->GetValue();
}

//******************************************

void prObFileChooserCtrl::Init()
{

	if( b_init )
		return;
	b_init = true;

	if( File_or_Directory != wxString() )
		if( SetFullPath( File_or_Directory ) )
			return;

	if( dataDirPath.DirExists() )
	{
		wxString dummy_file = wxDir::FindFirst( dataDirPath.GetFullPath(), wxT("*"), wxDIR_FILES );
		if( wxFileName( dummy_file ).FileExists() )
		{
			filePck_file->SetPath( dummy_file  );
//			wxMessageBox( dummy_file , "WARNING", wxOK | wxICON_EXCLAMATION, this );
		}
	}
	else
		wxMessageBox( wxT("DataDir path invalid : <") + dataDirPath.GetFullPath() + wxT(">"), wxT("WARNING"), wxOK | wxICON_EXCLAMATION, this );

}



//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//****************************************************
//			THE SCROLL PANNEL
//****************************************************

AScrollPanel::AScrollPanel( wxWindow *_parent )
: wxScrolledWindow( _parent, wxID_ANY, wxDefaultPosition,wxDefaultSize,
		wxTAB_TRAVERSAL | wxVSCROLL | wxHSCROLL |wxFULL_REPAINT_ON_RESIZE )
{
	b_fix_V = false;
	b_fix_H = true;
	SetScrollRate( 15, 15 );
	EnableScrolling(!b_fix_H, !b_fix_V);
	SetAutoLayout( true );
	Fit();
}


//************************************************************************************

AScrollPanel::~AScrollPanel()
{
}

//************************************************************************************

void AScrollPanel::EvtSize( wxSizeEvent& event )
{
// 	wxSize size = GetBestVirtualSize();
// //	wxSize s = event.GetSize();
// 
// 	SetVirtualSize( size );

	AdjustScrollbars();
	/*
	Refresh();
	Layout();
	*/
	event.Skip();
}


//************************************************************************************

bool AScrollPanel::Layout()
{
//	AdjustScrollbars();
	return wxScrolledWindow::Layout();
}



//************************************************************************************

wxSize AScrollPanel::GetMinSize()const
{
	wxSizer *m_sizer = GetSizer();
	if( m_sizer == NULL )
		return wxScrolledWindow::GetMinSize();

	wxSize s = m_sizer->GetMinSize();
	if( !b_fix_H ) 
		s.x = -1;
	if( !b_fix_V ) 
		s.y = -1;
	
#ifndef __WXGTK__
	return s;
#else

//	wxMessageBox( "Size : " + IntToStr( s.x ) + "," + IntToStr(s.y ) );
	if( b_fix_V && HasScrollbar( wxHORIZONTAL ))
		s.y = s.y + 20;

	if( b_fix_H && HasScrollbar( wxVERTICAL ))
		s.x = s.x + 20;

	return s;
#endif
}


//************************************************************************************

wxSize AScrollPanel::DoGetBestSize() const
{
	return GetVirtualSize();
}


//************************************************************************************

void AScrollPanel::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
	wxScrolledWindow::DoSetSize( x,y,width,height,sizeFlags );
}


//************************************************************************************

void AScrollPanel::DoGetClientSize( int* width, int* height) const
{
	wxScrolledWindow::DoGetClientSize( width, height );
	return;
}


//************************************************************************************

void AScrollPanel::Set_FixedDirections( bool b_H, bool b_V )
{
	b_fix_V = b_V;
	b_fix_H = b_H;
	EnableScrolling(!b_fix_H, !b_fix_V);
	Refresh();
}


//************************************************************************************

void AScrollPanel::OnPaint( wxPaintEvent &event )
{
//    wxPaintDC dc( this );
    /*
    wxSize s = GetClientSize();
    dc.DrawLine( 0, 0, s.x, s.y );
    */
    event.Skip();
}

//************************************************************************************
//************************************************************************************
//************************************************************************************
//************************************************************************************
// Event table
BEGIN_EVENT_TABLE(AScrollPanel, wxScrolledWindow)
	EVT_SIZE(AScrollPanel::EvtSize)
	EVT_PAINT(AScrollPanel::OnPaint)
END_EVENT_TABLE()
