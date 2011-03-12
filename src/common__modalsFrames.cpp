#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__tools.h"
#include "CHistory.h"
#include "common__modalsFrames.h"
#include "common__ob_controls.h"
#include "common__wxFrameImage.h"


//****************************************************
//****************************************************

const int txtctrl_string_field_w = 300;
const int txtctrl_string_label_w = 170;

const int txtctrl_number_field_w = 50;
const int txtctrl_number_label_w = 170;

const int comboctrl_field_w = wxDefaultCoord;
const int comboctrl_label_w = 170;

const int filectrl_field_w = 280;
const int filectrl_label_w = 170;

//****************************************************
//****************************************************

frameEdit_request::frameEdit_request(
		const wxString& _propname,
		const wxString& _label,
		int _type,
		const wxString& _request,
		bool _b_del_ifempty,
		wxArrayString _enums,
		int _labelw,
		int _fieldw
		)
{
		ctrl = NULL;
		Set( _propname, _label, _type, _request, _b_del_ifempty, _enums, _labelw, _fieldw );
}

frameEdit_request::~frameEdit_request()
{
	if( missing_tokens != NULL )
		delete[] missing_tokens;
}

void frameEdit_request::Set(
						wxString _propname,
						wxString _label,
						int _type,
						wxString _request,
						bool _b_del_ifempty,
						wxArrayString _enums,
						int _labelw,
						int _fieldw
						)
{
	name = _propname;
	label = _label;
	request = _request;
	b_del_ifempty = _b_del_ifempty;
	enums = _enums;
	labelw = _labelw;
	fieldw = _fieldw;
	defVal = wxString();
	missing_tokens = NULL;
	nb_missing_tokens = 0;
	b_optional = false;

	SetPropType( _type );
}


void frameEdit_request::SetPropType( int _proptype )
{
	type = _proptype;
	if( type == PROPTYPE_BOOL_EXSISTENCE )
	{
		b_del_ifempty = true;
	}

	if( type == PROPTYPE_UNDEF )
		return;

	if( labelw == wxDefaultCoord )
	{
		switch( type )
		{
		case PROPTYPE_STRING:
		case PROPTYPE_STRING_NO_WS:
			labelw = txtctrl_string_label_w;
			break;
		case PROPTYPE_OBFILE:
			labelw = filectrl_label_w;
			break;
		case PROPTYPE_NEGATIVENUMBER:
		case PROPTYPE_RELATIVENUMBER:
		case PROPTYPE_NUMBER:
		case PROPTYPE_FLOAT:
		case PROPTYPE_FLOAT_POSITIVE:
			labelw = txtctrl_number_label_w;
			break;
		case PROPTYPE_ENUMS:
		case PROPTYPE_RANGE:
			labelw = comboctrl_label_w;
			break;
		}
	}
	if( fieldw == wxDefaultCoord )
	{
		switch( type )
		{
		case PROPTYPE_STRING:
		case PROPTYPE_STRING_NO_WS:
			fieldw = txtctrl_string_field_w;
			break;
		case PROPTYPE_OBFILE:
			fieldw = filectrl_field_w;
			break;
		case PROPTYPE_NUMBER:
		case PROPTYPE_NEGATIVENUMBER:
		case PROPTYPE_RELATIVENUMBER:
		case PROPTYPE_FLOAT:
		case PROPTYPE_FLOAT_POSITIVE:
			fieldw = txtctrl_number_field_w;
			break;
		case PROPTYPE_ENUMS:
		case PROPTYPE_RANGE:
			fieldw = comboctrl_field_w;
			break;
		}
	}
}


int frameEdit_request::GetPropType()
{
	return type;
}

//****************************************************
//****************************************************

FrameEditProperties::FrameEditProperties(wxWindow *_parent,
					ob_object* _theObj,
					const wxString& _title,
					int more_style )
:wxDialog( _parent, wxID_ANY, _title, wxDefaultPosition, wxDefaultSize,
		wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE | more_style )
{
	nb_props = 0;
	reqs = NULL;

	theObj = _theObj;
	b_newObj = ( theObj  == NULL );
	b_chg = false;
	b_ok = false;
	b_creation = false;
	sizerMain = NULL;
	sizerTop = NULL;

	wndSavingName =wxT("FrameEditProperties");
	theHistoryManager.Set_State(false);
}

//****************************************************

void FrameEditProperties::Init()
{
	InitObj();
	InitProps();
	InitControls();
}


//****************************************************

void FrameEditProperties::InitControls()
{
	//***************
	// SIZERS
	//***************
	wxBoxSizer *sizerWrapper = new wxBoxSizer( wxVERTICAL );
	sizerMain = new wxBoxSizer( wxVERTICAL );
	int mainBordersize = 10;
	sizerWrapper->Add( sizerMain, 0, wxALL, mainBordersize );

	sizerTop = new wxBoxSizer( wxVERTICAL );
	sizerMain->Add( sizerTop, 1, wxEXPAND );

	wxBoxSizer *sizerBottom = new wxBoxSizer( wxHORIZONTAL );
	sizerMain->Add( sizerBottom, 0, wxEXPAND );


	//***************
	// CONTROLS
	//***************

	for( int i= 0; i < nb_props; i++)
	{
		wxBoxSizer *sizer_temp;
		if( reqs[i].GetPropType() == PROPTYPE_SPACER )
		{
			sizerTop->AddSpacer( StrToInt( reqs[i].name ));
			continue;
		}

		wxStaticText* t_stat;
		if( reqs[i].GetPropType() != PROPTYPE_BOOL && reqs[i].GetPropType() != PROPTYPE_BOOL_EXSISTENCE )
		{
			sizer_temp = new wxBoxSizer( wxHORIZONTAL );
			t_stat = new wxStaticText( this, wxID_ANY, reqs[i].label );
			t_stat->SetMinSize( wxSize(reqs[i].labelw,  wxDefaultCoord) );
			sizer_temp->Add( t_stat, 0, wxALIGN_CENTER );
		}

		switch( reqs[i].GetPropType() )
		{
		case PROPTYPE_STRING:
		{
			wxTextCtrl* txtctrl_temp  = new wxTextCtrl( this, wxID_ANY, GetObjValue( i ) );
			txtctrl_temp->SetMinSize( wxSize(reqs[i].fieldw,  wxDefaultCoord) );
			sizer_temp->Add(txtctrl_temp );
			sizerTop->Add(sizer_temp, 0, wxEXPAND, 0 );
			reqs[i].ctrl =txtctrl_temp;
			break;
		}

		case PROPTYPE_STRING_NO_WS:
		{
			wxTextCtrl* txtctrl_temp  = new wxTextCtrl( this, wxID_ANY, GetObjValue( i )
					, wxDefaultPosition, wxDefaultSize, 0,
					wxTextValidator_NoWhiteSpace() );
			txtctrl_temp->SetMinSize( wxSize(reqs[i].fieldw,  wxDefaultCoord) );
			sizer_temp->Add(txtctrl_temp );
			sizerTop->Add(sizer_temp, 0, wxEXPAND, 0 );
			reqs[i].ctrl =txtctrl_temp;
			break;
		}

		case PROPTYPE_NUMBER:
		case PROPTYPE_NEGATIVENUMBER:
		case PROPTYPE_RELATIVENUMBER:
		case PROPTYPE_FLOAT:
		case PROPTYPE_FLOAT_POSITIVE:
		{
			wxValidator* _validator;
			if( reqs[i].GetPropType() == PROPTYPE_NUMBER )
				_validator = new wxValidator_Restrict( wxT("0123456789"));
			else if( reqs[i].GetPropType() == PROPTYPE_NEGATIVENUMBER )
				_validator = new wxValidatorIntegerNegative();
			else if( reqs[i].GetPropType() == PROPTYPE_RELATIVENUMBER )
				_validator = new wxValidatorIntegerRelative();
			else if( reqs[i].GetPropType() == PROPTYPE_FLOAT )
				_validator = new wxValidatorFloat();
			else if( reqs[i].GetPropType() == PROPTYPE_FLOAT_POSITIVE )
				_validator = new wxValidatorFloatPositive();

			wxTextCtrl* txtctrl_temp  = new wxTextCtrl( this, wxID_ANY, GetObjValue( i )
					, wxDefaultPosition, wxDefaultSize, 0
					, *_validator );
			txtctrl_temp->SetMinSize( wxSize(reqs[i].fieldw,  wxDefaultCoord) );
			sizer_temp->Add(txtctrl_temp );
			sizerTop->Add(sizer_temp, 0, wxEXPAND, 0 );
			reqs[i].ctrl =txtctrl_temp;
			break;
		}

		case PROPTYPE_BOOL:
		case PROPTYPE_BOOL_EXSISTENCE:
		{
			wxCheckBox* chckbx_temp = new wxCheckBox(this, wxID_ANY, reqs[i].label );
			wxString _v = GetObjValue( i );
			chckbx_temp->SetValue( StrToBool( _v ));
			sizerTop->Add(chckbx_temp );
			reqs[i].ctrl =chckbx_temp;
			break;
		}

		case PROPTYPE_ENUMS:
		{
			wxComboBox* cbCtrl_temp = new wxComboBox(this, wxID_ANY, wxString()
					, wxDefaultPosition, wxDefaultSize
					, reqs[i].enums,
					wxCB_DROPDOWN | wxCB_READONLY );
			cbCtrl_temp->SetValue( GetObjValue( i ) );
			cbCtrl_temp->SetMinSize( wxSize(reqs[i].fieldw,  wxDefaultCoord) );
			sizer_temp->Add( cbCtrl_temp );
			sizerTop->Add(sizer_temp, 0, wxEXPAND );
			reqs[i].ctrl =cbCtrl_temp;
			break;
		}

		case PROPTYPE_RANGE:
		{
			wxComboBox* cbCtrl_temp = new wxComboBox(this, wxID_ANY, wxString()
					, wxDefaultPosition, wxDefaultSize
					, reqs[i].enums,
					wxCB_DROPDOWN | wxCB_READONLY );
			cbCtrl_temp->SetSelection( StrToInt( GetObjValue( i )) );
			cbCtrl_temp->SetMinSize( wxSize(reqs[i].fieldw,  wxDefaultCoord) );
			sizer_temp->Add( cbCtrl_temp );
			sizerTop->Add(sizer_temp, 0, wxEXPAND );
			reqs[i].ctrl =cbCtrl_temp;
			break;
		}

		case PROPTYPE_OBFILE:
		{
			prObFileChooserCtrl *fileChooser_temp = new prObFileChooserCtrl( this, wxID_ANY, wxString(), true, reqs[i].fieldw );
			fileChooser_temp->SetObPath( GetObjValue( i ) );
			sizer_temp->Add( fileChooser_temp );
			sizerTop->Add(sizer_temp, 0, wxEXPAND );
			reqs[i].ctrl =fileChooser_temp;
			break;
		}
		}
	}

	sizerMain->AddSpacer( 20 );

	int btn_style = 0;
	wxButton *btn_OK = new wxButton( this, wxID_ANY, wxT("OK") );
	sizerBottom->Add(btn_OK, 0, btn_style );
	btn_OK->Connect( wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
					wxCommandEventHandler(FrameEditProperties::EvtClickOK), NULL, this );
					wxButton* btn_CANCEL = new wxButton( this, wxID_ANY, wxT("CANCEL") );
	btn_CANCEL->Connect( wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
					wxCommandEventHandler(FrameEditProperties::EvtClickCancel), NULL, this );
	sizerBottom->Add(btn_CANCEL, 0, btn_style );

    SetSizer( sizerWrapper );

    if( ! Frame_RestorePrevCoord( this, wndSavingName ) )
    {
    	sizerMain->Layout();
    	SetSize( GetBestSize() );
    }

    Layout();
}


//****************************************************

FrameEditProperties::~FrameEditProperties()
{
	if( nb_props > 0 )
		delete[] reqs;
}


//****************************************************

void FrameEditProperties::EvtClose( wxCloseEvent& event )
{
  	Frame_SaveCoord( this, wndSavingName );
  	if( ! b_ok && b_newObj )
  	{
  		if( theObj != NULL )
  			theObj->Rm();
  		theObj = NULL;
  	}
	theHistoryManager.Set_State(true);
	event.Skip();
}


//****************************************************

void FrameEditProperties::EvtClickOK(wxCommandEvent& evt )
{
	b_ok = true;
	Clicked_OK();
}


//-----------------------------------------------------------------
void 
FrameEditProperties::Clicked_OK( bool b_close )
{
	if( IsChanged() == true )
	{
		b_chg = true;
		if( b_newObj == false )
		{
			theHistoryManager.Set_State(true);
			theHistoryManager.GroupStart(GetTitle());
		}
		WriteChangesToObject();
		if( b_newObj == false )
		{
			theHistoryManager.GroupEnd();
			theHistoryManager.Set_State(false);
		}
	}

	if( b_close )
		Close();
}


//****************************************************

void FrameEditProperties::EvtClickCancel(wxCommandEvent& evt )
{
	Clicked_Cancel();
}



//****************************************************

void FrameEditProperties::Clicked_Cancel(bool b_close)
{
	if( b_close )
		Close();
}


//****************************************************

void FrameEditProperties::InitProps()
{
}

//****************************************************

void FrameEditProperties::InitObj()
{
	if( theObj == NULL )
	{
		theObj = new ob_object();
		theObj->SetToDefault();
		b_newObj = true;
	}

}



//****************************************************
// propsNames examples:
//  "_NAME_"    -> the object name
//  "_TOKEN_3"  -> the 3rd token of the object
//  "_ALLTOKENS_"  -> All the tokens of the object
//  "_SUBOBJ_NUM%4%PROP%shape"  -> the shape property of the 4d subobject
//  "_SUBOBJ_NAME%big%ALLTOKENS"  -> all tokens of the "big" subobject
//  "_SUBOBJ_NAME%boule%TOKEN%3"  -> the 3rd token of the subobject "boule"
//  "_SUBOBJ_NAME%boule%NUM%4%PROP%shape"  -> the shape property of the 4d boule subobject
//  "_SUBOBJ_TYPE%balon%NUM%4%TOKEN%3"  -> the 3rd token of the 4d subobject of type balon


wxString FrameEditProperties::GetObjValue( int i )
{
	if( i < 0 || i >= nb_props )
		return wxString();


	wxString res;
	// Object name
	if( reqs[i].name ==  wxT("_NAME_") )
	{
		res = theObj->name;
	}

	// Token request
	else if( reqs[i].name.Mid( 0, wxString::FromAscii("_TOKEN_").Len() ) == wxT("_TOKEN_") )
	{
		int t = wxString::FromAscii("_TOKEN_").Len();
		int num_tok = StrToInt( reqs[i].name.Mid( t ,reqs[i].name.Len() - t ) );
		res = theObj->GetToken( num_tok );
	}

	// All Tokens request
	else if( reqs[i].name.Mid( 0, wxString::FromAscii("_ALLTOKENS_").Len() ) == wxT("_ALLTOKENS_") )
	{
		res = theObj->GetAllTokenToStr();
	}

	// Deep search in subobject
	else if( reqs[i].name.Mid( 0, wxString::FromAscii("_SUBOBJ_").Len() ) == wxT("_SUBOBJ_") )
	{
		int t = wxString::FromAscii("_SUBOBJ_").Len();
		wxString sub_request = reqs[i].name.Mid( t ,reqs[i].name.Len() - t );
		wxArrayString request = StrSplit( sub_request, wxT("%") );

		if( request.Count() >= 3 )
		{
			wxString t0 = request[0];
			wxString t1 = request[1];
			wxString t2 = request[2];
			wxString t3 = t0 + t1;
		}
		if(		(request.Count() != 3 && request.Count() != 4 && request.Count() != 6 )
				||
				(request.Count() == 6 &&
				( request[0] != wxT("NAME") && request[0] != wxT("TYPE")
				&& request[2] != wxT("NUM") && request[4] != wxT("PROP") && request[4] != wxT("TOKEN")))
				||
				(request.Count() == 4 &&
				(	request[0] != wxT("NUM") && request[0] != wxT("NAME") &&
				request[2] != wxT("PROP") && request[0] != wxT("TOKEN") ))
				||
				(request.Count() == 3 &&
				(	request[0] != wxT("NUM") && request[0] != wxT("NAME") &&
				request[2] != wxT("ALLTOKENS") ))
		  )
		{
			wxMessageBox( wxT("FrameEditProperties::GetObjValue : Invalid resquest\n") + reqs[i].name,
				      wxT("BUG"), wxOK | wxICON_INFORMATION, this );
				      return wxString();
		}

		// GET THE SUBOBJECT
		ob_object* subobj = NULL;
		int ind_req = 0;


		if( request[0] == wxT("NAME") )
		{
			wxString subName = request[1];

			if( request[2] == wxT("NUM") )
			{
				int subobj_num = StrToInt(request[3]);
				size_t subobj_count;
				ob_object** subobjs =
						theObj->GetSubObjectS(subName,subobj_count);
				if( subobj_num >= (int) subobj_count)
				{
					if( subobjs != NULL )
						delete[] subobjs;
					return reqs[i].defVal;
				}

				subobj = subobjs[subobj_num];
				if( subobjs != NULL )
					delete[] subobjs;
				ind_req = 4;
			}
			else
			{
				subobj = theObj->GetSubObject( subName );
				ind_req = 2;
			}
		}

		else if( request[0] == wxT("TYPE") )
		{
			int subType = StrToInt( request[1] );
			size_t subobj_count;
			ob_object** subobjs = 
				theObj->GetSubObjectS_ofType(subType , subobj_count );

			if( request[2] == wxT("NUM") )
			{
				size_t subobj_num = StrToInt(request[3]);
				if( subobj_num >= subobj_count)
				{
					if( subobjs != NULL )
						delete[] subobjs;
					return reqs[i].defVal;
				}

				subobj = subobjs[subobj_num];
				ind_req = 4;
			}
			else
			{
				if( subobj_count == 0 )
					return reqs[i].defVal;
				subobj = subobjs[0];
				ind_req = 2;
			}
			if( subobjs != NULL )
				delete[] subobjs;
		}
		else
		{
			size_t nb_subobj;
			ob_object** subObjs = theObj->GetAllSubObjS( nb_subobj );
			
			int num_suboj = StrToInt( request[1] );
			if( (int) nb_subobj < num_suboj )
			{
				if( subObjs != NULL )
					delete[] subObjs;
				return reqs[i].defVal;
			}
			subobj = subObjs[num_suboj];
			delete[] subObjs;
			ind_req = 2;
		}

		if( subobj == NULL)
		{
			if(  reqs[i].GetPropType() != PROPTYPE_BOOL_EXSISTENCE && !reqs[i].b_optional )
				b_creation = true;
//			wxMessageBox( "will create :" + reqs[i].name );
			return reqs[i].defVal;
		}

		// Ok, we've got the subobj, so , get the thing
		wxString req_atom = request[ind_req];
		if( req_atom != wxT("PROP") && req_atom != wxT("TOKEN") && req_atom != wxT("ALLTOKENS") )
		{
			wxMessageBox( wxT("FrameEditProperties::GetObjValue : Invalid resquest\n") + reqs[i].name,
				      wxT("BUG"), wxOK | wxICON_INFORMATION, this );
				      return wxString();
		}

		if( req_atom == wxT("TOKEN") )
		{
			int num_token = StrToInt( request[ind_req+1] );
			res = subobj->GetToken( num_token );
		}
		else if( req_atom == wxT("ALLTOKENS") )
			res = subobj->GetAllTokenToStr();

		else if( req_atom == wxT("PROP") )
		{
			if( reqs[i].GetPropType() == PROPTYPE_BOOL_EXSISTENCE )
				res = wxT("1");
			else
				res = subobj->GetSubObject_Token( request[ind_req+1] );
		}

	}

	// Simple property
	else
	{
		if(theObj->GetSubObject(reqs[i].name ) == NULL && reqs[i].GetPropType() != PROPTYPE_BOOL_EXSISTENCE
				&& !reqs[i].b_optional )
		{
//			wxMessageBox( "will create :" + reqs[i].name );
			b_creation = true;
		}

		if( reqs[i].GetPropType() == PROPTYPE_BOOL_EXSISTENCE )
			res = (theObj->GetSubObject(reqs[i].name ) != NULL ) ? wxT("1") : wxString() ;
		else
			res = theObj->GetSubObject_Token( reqs[i].name );
	}

	if( res == wxString() )
		return reqs[i].defVal;
	else
		return res;
}


//****************************************************

bool FrameEditProperties::IsChanged()
{
	if( b_creation )
	{
		return true;
	}

	for( int i= 0; i < nb_props; i++)
	{
		if( reqs[i].GetPropType() == PROPTYPE_SPACER )
			continue;

		switch( reqs[i].GetPropType() )
		{
		case PROPTYPE_STRING:
		case PROPTYPE_STRING_NO_WS:
		{
			if( ((wxTextCtrl*)reqs[i].ctrl)->GetValue() != GetObjValue( i ) )
				return true;
			break;
		}

		case PROPTYPE_NUMBER:
		case PROPTYPE_NEGATIVENUMBER:
		case PROPTYPE_RELATIVENUMBER:
		{
			if( StrToInt( ((wxTextCtrl*)reqs[i].ctrl)->GetValue()) != StrToInt( GetObjValue( i )) )
				return true;
			break;
		}

		case PROPTYPE_FLOAT_POSITIVE:
		case PROPTYPE_FLOAT:
		{
			bool b_dummy;
			if( 
				StrToFloat( ((wxTextCtrl*)reqs[i].ctrl)->GetValue(),b_dummy)
				!= 
				StrToFloat( GetObjValue( i ),b_dummy)
			  )
				return true;
			break;
		}
		

		case PROPTYPE_BOOL:
		case PROPTYPE_BOOL_EXSISTENCE:
		{
			if( ((wxCheckBox*)reqs[i].ctrl)->GetValue() != StrToBool( GetObjValue( i )) )
				return true;
			break;
		}

		case PROPTYPE_ENUMS:
		{
			if( ((wxComboBox*)reqs[i].ctrl)->GetValue() != GetObjValue( i ) )
				return true;
			break;
		}

		case PROPTYPE_RANGE:
		{
			if( ((wxComboBox*)reqs[i].ctrl)->GetSelection() != StrToInt(GetObjValue( i )) )
				return true;
			break;
		}

		case PROPTYPE_OBFILE:
		{
			if( ((prObFileChooserCtrl*)reqs[i].ctrl)->GetObPath() != GetObjValue( i ) )
				return true;
			break;
		}
		
		default:
		{
			wxMessageBox( wxT("ERROR\nFrameEditProperties::IsChanged()\nUnhandled PROPTYPE : ") + IntToStr(reqs[i].GetPropType()), wxT("BUGG!") );
			break;
		}
		}
	}
	return false;
}


//****************************************************
// Return number of write errors

int FrameEditProperties::WriteChangesToObject()
{
	int err = 0;
	for( int i= 0; i < nb_props; i++ )
	{
		if( reqs[i].GetPropType() == PROPTYPE_SPACER )
			continue;

		switch( reqs[i].GetPropType() )
		{
		case PROPTYPE_STRING:
		case PROPTYPE_STRING_NO_WS:
		{
			wxString val = ((wxTextCtrl*)reqs[i].ctrl)->GetValue();
			if( 	   reqs[i].b_optional == false 
				|| reqs[i].defVal != val
				)
				if( ! SetObjValue(i, val, val ))
					err++;
			break;
		}

		case PROPTYPE_NUMBER:
		case PROPTYPE_NEGATIVENUMBER:
		case PROPTYPE_RELATIVENUMBER:
		{
			wxString val = ((wxTextCtrl*)reqs[i].ctrl)->GetValue();
			wxString int_val = IntToStr( StrToInt(val) );
			if( 	   reqs[i].b_optional == false 
				|| reqs[i].defVal != int_val
				)
				if( ! SetObjValue(i, int_val, val ))
					err++;
			break;
		}
		
		case PROPTYPE_FLOAT_POSITIVE:
		case PROPTYPE_FLOAT:
		{
			wxString val = ((wxTextCtrl*)reqs[i].ctrl)->GetValue();
			bool b_dummy;
			wxString float_val = FloatToStr( StrToFloat(val, b_dummy) );
			if( 	   reqs[i].b_optional == false 
				|| reqs[i].defVal != float_val
				)
				if( ! SetObjValue(i, float_val, val ))
					err++;
			break;
		}
		
			
		case PROPTYPE_BOOL:
		{
			wxString bool_val = BoolToStr(((wxCheckBox*)reqs[i].ctrl)->GetValue());
			if( 	   reqs[i].b_optional == false 
				|| reqs[i].defVal != bool_val
				)
				if( ! SetObjValue(i, bool_val, wxT("__FALSE__") ) )
					err++;
			break;
		}
		case PROPTYPE_BOOL_EXSISTENCE:
		{
			bool _bsetit = ((wxCheckBox*)reqs[i].ctrl)->GetValue();
			if( ! SetObjValue(i, wxString(), _bsetit ? wxT("SETIT") : wxString() ) )
				err++;
			break;
		}
		case PROPTYPE_ENUMS:
		{
			wxString val = ((wxComboBox*)reqs[i].ctrl)->GetValue();
			if( 	   reqs[i].b_optional == false 
				|| reqs[i].defVal != val
				)
				if( ! SetObjValue(i, val, val ))
					err++;
			break;
		}

		case PROPTYPE_RANGE:
		{
			wxString val = IntToStr( ((wxComboBox*)reqs[i].ctrl)->GetSelection() );
			wxString int_val = IntToStr( StrToInt( val ) );
			if( 	   reqs[i].b_optional == false 
				|| reqs[i].defVal != int_val
				)
				if( ! SetObjValue(i, int_val, val ) )
						err++;
			break;
		}

		case PROPTYPE_OBFILE:
		{
			wxString val = ((prObFileChooserCtrl*)reqs[i].ctrl)->GetObPath();
			if( 	   reqs[i].b_optional == false 
				|| reqs[i].defVal != val
				)
				if( ! SetObjValue(i, val, val ) )
					err++;
			break;
		}
		}
	}

	return err;
}

//****************************************************
// propsNames examples:
//  "_NAME_"    -> the object name
//  "_TOKEN_3"  -> the 3rd token of the object
//  "_SUBOBJ_NUM%4%PROP%shape"  -> the shape property of the 4d subobject
//  "_SUBOBJ_NAME%boule%TOKEN%3"  -> the 3rd token of the subobject "boule"
//  "_SUBOBJ_NAME%boule%NUM%4%PROP%shape"  -> the shape property of the 4d boule subobject
//  "_SUBOBJ_TYPE%balon%NUM%4%TOKEN%3"  -> the 3rd token of the 4d subobject of type balon

bool FrameEditProperties::SetObjValue( int i, const wxString& val, const wxString& origin_val )
{
	if( i < 0 || i > nb_props )
		return false;

	bool b_delete = false;
	if( origin_val == wxString() && reqs[i].b_del_ifempty )
		b_delete = true;

	// Object name
	if( reqs[i].name ==  wxT("_NAME_") )
	{
		theObj->SetName( val );
	}

	// Token request
	else if( reqs[i].name.Mid( 0, wxString::FromAscii("_TOKEN_").Len() ) == wxT("_TOKEN_") )
	{
		int t = wxString::FromAscii("_TOKEN_").Len();
		int num_tok = StrToInt( reqs[i].name.Mid( t ,reqs[i].name.Len() - t ) );
		return theObj->SetToken( num_tok, val, reqs[i].missing_tokens, reqs[i].nb_missing_tokens );
	}

	// All Tokens request
	else if( reqs[i].name.Mid( 0, wxString::FromAscii("_ALLTOKENS_").Len() ) == wxT("_ALLTOKENS_") )
	{
		return theObj->SetAllTokenFromStr( val );
	}

	// Deep search in subobject
	else if( reqs[i].name.Mid( 0, wxString::FromAscii("_SUBOBJ_").Len() ) == wxT("_SUBOBJ_") )
	{
		int t = wxString::FromAscii("_SUBOBJ_").Len();
		wxString sub_request = reqs[i].name.Mid( t ,reqs[i].name.Len() - t );
		wxArrayString request = StrSplit( sub_request, wxT("%") );

		if(		(request.Count() != 3 && request.Count() != 4 && request.Count() != 6 )
				||
				(request.Count() == 6 &&
				( request[0] != wxT("NAME") && request[0] != wxT("TYPE")
				&& request[2] != wxT("NUM") && request[4] != wxT("PROP") && request[4] != wxT("TOKEN")))
				||
				(request.Count() == 4 &&
				(	request[0] != wxT("NUM") && request[0] != wxT("NAME") &&
				request[2] != wxT("PROP") && request[0] != wxT("TOKEN") ))
				||
				(request.Count() == 3 &&
				(	request[0] != wxT("NUM") && request[0] != wxT("NAME") &&
				request[2] != wxT("ALLTOKENS") ))
		  )
		{
			return false;
		}

		// GET THE SUBOBJECT
		ob_object* subobj = NULL;
		int ind_req = 0;


		if( request[0] == wxT("NAME") )
		{
			wxString subName = request[1];

			if( request[2] == wxT("NUM") )
			{
				int subobj_num = StrToInt(request[3]);
				size_t subobj_count;
				ob_object** subobjs = 
					theObj->GetSubObjectS( subName, subobj_count );
				
				if( subobj_num >= (int) 	subobj_count)
				{
					if( subobjs != NULL )
						delete[] subobjs;
					return false;
				}
				subobj = subobjs[subobj_num];
				if( subobjs != NULL )
					delete[] subobjs;
				ind_req = 4;
			}
			else
			{
				subobj = theObj->GetSubObject( subName );
				if( subobj == NULL && reqs[i].defVal == val )
					return true;
				else if( subobj == NULL )
				{
					subobj = new ob_object( subName );
					theObj->Add_SubObj( subobj );
				}
				ind_req = 2;
			}
		}

		else if( request[0] ==wxT("TYPE") )
		{
			int subType = StrToInt( request[1] );
			size_t subobj_count;
			ob_object** subobjs = 
				theObj->GetSubObjectS_ofType(subType , subobj_count );

			if( request[2] == wxT("NUM") )
			{
				size_t subobj_num = StrToInt(request[3]);
				if( subobj_num >= subobj_count )
				{
					if( subobjs != NULL )
						delete[] subobjs;
					return false;
				}

				subobj = subobjs[subobj_num];
				ind_req = 4;
			}
			else
			{
				if( subobj_count == 0 )
					return false;
				subobj = subobjs[0];
				ind_req = 2;
			}
			if( subobjs != NULL )
				delete[] subobjs;
		}
		else
		{
			size_t nb_subobj;
			ob_object** subObjs = theObj->GetAllSubObjS( nb_subobj );
			
			int num_suboj = StrToInt( request[1] );
			if( (int) nb_subobj < num_suboj )
			{
				if( subObjs != NULL )
					delete[] subObjs;
				return false;
			}
			subobj = subObjs[num_suboj];
			delete[] subObjs;
			ind_req = 2;
		}

		if( subobj == NULL )
			return false;

		// Ok, we've got the subobj, so , get the thing
		wxString req_atom = request[ind_req];
		if( req_atom != wxT("PROP") && req_atom != wxT("TOKEN") && req_atom != wxT("ALLTOKENS") )
			return false;

		if( req_atom == wxT("TOKEN") )
		{
			wxString _t = wxT(":") + val;
			if( ! b_delete )
			{
				int num_token = StrToInt( request[ind_req+1] );
				return subobj->SetToken( num_token, val, reqs[i].missing_tokens, reqs[i].nb_missing_tokens );
			}
			else
			{
				subobj->Rm();
				return true;
			}
		}
		else if( req_atom == wxT("ALLTOKENS") )
		{
			if( ! b_delete )
				return subobj->SetAllTokenFromStr(val);
			else
			{
				subobj->Rm();
				return true;
			}
		}

		else if( req_atom == wxT("PROP") )
		{
			if( ! b_delete )
			{
				if( subobj->GetSubObject( request[ind_req+1] ) != NULL )
					return subobj->SetProperty( request[ind_req+1], val );
				else
					return subobj->Add_SubObj( new ob_object(request[ind_req+1], val ) );
			}
			else
			{
				subobj->Rm();
				return true;
			}
		}

	}

	// Simple property
	else
	{
		if( ! b_delete )
		{
			if( theObj->GetSubObject( reqs[i].name ) != NULL )
				return theObj->SetProperty( reqs[i].name, val );
			else
				return theObj->Add_SubObj( new ob_object(reqs[i].name, val ) );
		}
		else
		{
			ob_object *t = theObj->GetSubObject( reqs[i].name );
			if( t != NULL )
				t->Rm();
			return true;
		}
	}

	return false;
}


//****************************************************
// Event table
//****************************************************

BEGIN_EVENT_TABLE(FrameEditProperties, wxDialog)
	EVT_CLOSE(FrameEditProperties::EvtClose)
END_EVENT_TABLE()


//****************************************************
//****************************************************
//****************************************************
//****************************************************
//*********** VARIABLES ***********************

#define MAXGRIDPROP_NEWPROPS 100
ob_object* newProps[MAXGRIDPROP_NEWPROPS];
size_t newProps_count;


//****************************************************
//*********** new events
DEFINE_EVENT_TYPE(wxEVT_ADD_NEW_GRIDLINE)

// it may also be convenient to define an event table macro for this event type
#ifndef EVT_ADD_NEW_GRIDLINE
#define EVT_ADD_NEW_GRIDLINE(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_ADD_NEW_GRIDLINE, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif


DEFINE_EVENT_TYPE(wxEVT_GRIDLINE_REPOS_CURSOR)

// it may also be convenient to define an event table macro for this event type
#ifndef EVT_GRIDLINE_REPOS_CURSOR
#define EVT_GRIDLINE_REPOS_CURSOR(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_GRIDLINE_REPOS_CURSOR, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif



//****************************************************
// Event table
BEGIN_EVENT_TABLE(GridPropFrame, wxDialog)
	EVT_CLOSE(GridPropFrame::EvtClose)
	EVT_SIZE(GridPropFrame::EvtSize)
	EVT_CHAR(GridPropFrame::EvtCharPress)
	EVT_KEY_UP(GridPropFrame::EvtCharPress)
	EVT_CHAR_HOOK(GridPropFrame::EvtCharPress)
	EVT_BUTTON( wxOK,GridPropFrame::EvtOkClicked)
END_EVENT_TABLE()



//****************************************************
//*********** A CLASS ***********************
//****************************************************


GridPropFrame::GridPropFrame( wxWindow *_parent, ob_object* _theObj, const wxString& Title, const wxString& _intro, long _style, bool b_sort )
: wxDialog( _parent, wxID_ANY, Title, wxDefaultPosition, wxDefaultSize,
				_style | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE )
{
	// Vars init
	newProps_count=0;

	wxBoxSizer *sizerMain = new wxBoxSizer( wxVERTICAL );

	txtMessage = new wxStaticText(this, wxID_ANY, _intro, wxDefaultPosition,
				wxDefaultSize, wxALIGN_CENTRE );
	sizerMain->Add( txtMessage, 0, wxALL | wxEXPAND, 5 );

	list_Props = new GridOb_ObjectProperties( this );
  	list_Props->SetObjectHandled( _theObj );
	sizerMain->Add( list_Props, 1, wxALL | wxEXPAND, 5 );
  	SetSizer( sizerMain );

  	Frame_RestorePrevCoord( this, Title );
}

GridPropFrame::~GridPropFrame()
{
}

int GridPropFrame::Changed()
{
	return list_Props->changed ? 1 : 0;
}

void GridPropFrame::SetFilter_Properties( wxArrayString& _filter, bool b__exclude)
{
	list_Props->SetFilter_Properties( _filter, b__exclude);
	list_Props->RefreshProperties();
}

void GridPropFrame::SetFilter_Properties( int* arrObjIDs, int nb_ids )
{
	list_Props->SetFilter_Properties( arrObjIDs, nb_ids);
	list_Props->RefreshProperties();
}

void GridPropFrame::EvtClose( wxCloseEvent& event )
{
  	Frame_SaveCoord( this, GetTitle() );
	event.Skip();
}

void GridPropFrame::EvtSize(wxSizeEvent& event)
{
	event.Skip();
}

void GridPropFrame::EvtCharPress(wxKeyEvent& event)
{
//	int kc = event.GetKeyCode();

	event.Skip();
}

void GridPropFrame::EvtOkClicked( wxCommandEvent& event)
{
	Destroy();
	return;
}

//****************************************************
//*********** A CLASS IMPLEMENTATION *****************

GridOb_ObjectProperties::GridOb_ObjectProperties( wxWindow* _parent, bool _b_sort )
: wxGrid( _parent, wxID_ANY ,wxDefaultPosition ,wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
{
	b_Filter_exclude = true;
	b_changing_prop = false;
	changed = false;
	b_sort = _b_sort;
	b_sizing = false;
	b_destroying = false;
	theObj = NULL;
	arr_Props = NULL;
	arr_Props_size = 0;
	nb_Props = 0;
	nb_filter_IDs = 0;
	filter_IDs = NULL;

	// Formating the grid
	SetColMinimalAcceptableWidth( 30 );
	SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_CENTRE );
	CreateGrid( 5, 2 );
	SetRowLabelSize(30);
	AutoSizeColumn(0 );
	SetColLabelValue(0, wxT("Tag") );
	SetColLabelValue(1, wxT("Value") );
	DisableDragColMove();
	RefreshProperties();
}

GridOb_ObjectProperties::~GridOb_ObjectProperties()
{
	b_destroying = true;
	Clear();
}

void GridOb_ObjectProperties::SetFilter_Properties( wxArrayString& _filter, bool b__exclude)
{
	b_Filter_exclude = b__exclude;
	filter = _filter;

	RefreshProperties();
}

void GridOb_ObjectProperties::SetFilter_Properties( int* arrObjIDs, int nb_ids)
{
	filter_IDs = arrObjIDs;
	nb_filter_IDs = nb_ids;
}


void GridOb_ObjectProperties::SetObjectHandled( ob_object* _theObj )
{
	theObj = _theObj;
	RefreshProperties();
}


void GridOb_ObjectProperties::Clear()
{
	theObj = NULL;
	nb_Props = 0;
	if( arr_Props != NULL )
		delete[] arr_Props;
	if( filter_IDs != NULL )
		delete[] filter_IDs;
	filter_IDs = NULL;
	nb_filter_IDs = 0;

	arr_Props = NULL;
	arr_Props_size = 0;
	RefreshProperties();
}


void GridOb_ObjectProperties::RefreshProperties()
{
	if( b_destroying )
		return;
	bool prev_changed = changed;
	ClearGrid();
	DeleteRows(0, GetNumberRows() );

	AppendRows(1); // The Empty Line for additions

	if( theObj == NULL )
	{
		Refresh();
		return;
	}

	// Remove all previous prop
	if( arr_Props != NULL )
		delete[] arr_Props;
	arr_Props = NULL;
	arr_Props_size = 0;
	nb_Props = 0;

	// Get the properties from the object
	size_t temp_arr_Props_size;
	ob_object ** temp_arr_Props = theObj->GetAllSubObjS( temp_arr_Props_size );
	if( temp_arr_Props == NULL )
	{
		Refresh();
		return;
	}

	// Apply the filter
	for( size_t i=0; i < temp_arr_Props_size; i++)
		if( IsToExclude( temp_arr_Props[i]->name, temp_arr_Props[i] ))
			temp_arr_Props[i] = NULL;

	// Resize the temp array
	int curr_decal = 0;
	for( size_t i =0; i < temp_arr_Props_size; i++ )
	{
		if( temp_arr_Props[i] == NULL )
		{
			curr_decal ++;
			continue;
		}

		if( curr_decal == 0 )
			continue;

		temp_arr_Props[i-curr_decal] = temp_arr_Props[i];
	}
	temp_arr_Props_size -= curr_decal;

	// sort the temp array
  	if( b_sort )
  		Sort_ob_Object_ByName( temp_arr_Props, temp_arr_Props_size );


	// Insert the properties
	int curr_row = 1;
	for( size_t i = 0; i< temp_arr_Props_size; i++ )
		_AppendProp( temp_arr_Props[i] , curr_row);

	delete[] temp_arr_Props;

	UpdatePropGridSize();
	changed = prev_changed;

	Refresh();
	return;
}

void GridOb_ObjectProperties::_AppendProp( ob_object* theProp, int& curr_row )
{
	wxString tag = theProp->name;
	if( theProp->name == wxString() )
	{
//		MyLog( MYLOG_ERROR, "GridOb_ObjectProperties::_AppendProp", "A prop with empty name can't be appended" );
		return;
	}

	AppendRows(1);
	SetCellValue( curr_row, 0, tag );
	SetCellValue( curr_row, 1, UnTokenize( theProp ) );
	curr_row++;

	if( nb_Props >= arr_Props_size )
		ExpandArrObj();
	arr_Props[nb_Props] = theProp;
	nb_Props++;
	changed = true;
}

void GridOb_ObjectProperties::UpdatePropGridSize()
{
	wxSizer *sizer_Props = GetContainingSizer();
	if( sizer_Props == NULL )
		return;

	wxSize sizer_size = sizer_Props->GetSize();
	int max_width = sizer_size.GetWidth();
	int col_total_width = max_width - 60; // -60 for not having a scrollbar

	int v0 = col_total_width / 3;
	if( v0 > 200 )
		v0 = 200;
	int v1 = col_total_width - v0;

	SetColSize( 0, v0 );
	SetColSize( 1, v1 );

	SetSize(max_width, wxDefaultCoord );
	SetSize( wxSize(max_width, wxDefaultCoord ));
	sizer_Props->Layout();
	Refresh();
}



//****************************************************

void GridOb_ObjectProperties::EvtSize(wxSizeEvent& event)
{
	if( b_sizing )
		return;
	b_sizing = true;
	UpdatePropGridSize();
	b_sizing = false;
	event.Skip();
}

void GridOb_ObjectProperties::EvtReposCursor( wxCommandEvent& event )
{
	SetGridCursor( new_row_pos, new_col_pos );
}

void GridOb_ObjectProperties::EvtCharPress(wxKeyEvent& event)
{
	int kc = event.GetKeyCode();

	if( kc == WXK_DELETE )
	{
		// check if there is a valid line(s) selection
		wxArrayInt arr_rows = GetSelectedRows();
		for( int j = arr_rows.Count() - 1; j >= 0; j-- )
		{
			if( arr_rows[j] != 0 )
				DeletePropAtRow( arr_rows[j] );
		}
		wxGridCellCoordsArray tl = GetSelectionBlockTopLeft();
		wxGridCellCoordsArray br = GetSelectionBlockBottomRight();
		for( int j = tl.Count() -1; j >= 0; j-- )
		{
			if( tl[j].GetCol() == 0 && br[j].GetCol() == 1 )
			{
				for( int k = br[j].GetRow(); k >= tl[j].GetRow(); k--)
					if( k != 0 )
						DeletePropAtRow( k );
			}
		}
	}
	else
	{
//		wxPostEvent( GetParent(), event );
		event.Skip();
		return;
	}
}


void GridOb_ObjectProperties::EvtPropertyChange( wxGridEvent& event )
{
	if( b_changing_prop )
		return;

	b_changing_prop = true;
    int _col = event.GetCol();
    int _row = event.GetRow();
    wxString newData = GetCellValue(_row, _col);

    if( _col == 0 )
    {
    	bool b_abort = false;

    	//CHeck there is no space in the data
    	if( newData.Find( ' ' ) != wxNOT_FOUND || newData.Find( '\t' ) != wxNOT_FOUND )
    	{
		wxMessageBox( wxT("Tags must not contains blank spaces"),
			      wxT("Error"), wxOK | wxICON_INFORMATION, this );
		     b_abort = true;
    	}

    	if( IsToExclude( newData ) )
    	{
		wxMessageBox( wxT("Tag ") + newData + wxT(" is filtered !!"), wxT("Error"), wxOK | wxICON_EXCLAMATION, GetParent() );
    		b_abort = true;
    	}

    	if( b_abort )
    	{
		    // Restor previous value
		    if( _row > 0 )
		    	SetCellValue(_row, _col, GetPrevPropertyName( _row ));
		    else
		    	SetCellValue(_row, _col, wxString());
		    b_changing_prop = false;
		    return;
    	}
    }
    if( ! SetObjProperty_fromRow( _row ))
    {
	    new_row_pos = _row;
	    new_col_pos = _col;
	    wxCommandEvent EvtRepos(wxEVT_GRIDLINE_REPOS_CURSOR);
		AddPendingEvent( EvtRepos );
    	b_changing_prop = false;
    	return;
    }

	// ADD a new row at the top to permit others addings
	if( _row == 0 && _col == 0 )
	{
	    wxCommandEvent EvtAddRow(wxEVT_ADD_NEW_GRIDLINE);
		AddPendingEvent( EvtAddRow );
	}
    else
    {
    	bool b_repos = true;
	    if( _col == 1 && _row == 1 )
	    {
		    new_row_pos = 0;
		    new_col_pos = 0;
	    }
	    else if( _col == 0 )
	    {
		    new_row_pos = _row;
		    new_col_pos = 1;
	    }
	    else
	    	b_repos = false;

	    if( b_repos )
	    {
		    wxCommandEvent EvtRepos(wxEVT_GRIDLINE_REPOS_CURSOR);
			AddPendingEvent( EvtRepos );
	    }
    }

    b_changing_prop = false;
//	event.Skip();
}


void GridOb_ObjectProperties::EvtAddRow(wxCommandEvent& event)
{
    InsertRows();
	SetGridCursor( 1, 1 );
}


void GridOb_ObjectProperties::EvtClose( wxCloseEvent& event )
{
	event.Skip();
}

void GridOb_ObjectProperties::EvtSelectChg( wxGridEvent& event )
{
//	SetFocus();
	event.Skip();
}

void GridOb_ObjectProperties::EvtRgSelectChg( wxGridRangeSelectEvent& event )
{
	SetFocus();
	event.Skip();
}

bool GridOb_ObjectProperties::ProcessEvent( wxEvent& event )
{
	return wxGrid::ProcessEvent(event);
}


//****************************************************

wxString GridOb_ObjectProperties::GetPrevPropertyName( int _row )
{
	if( _row == 0 )
		return wxString();

	// Out of bound parameter
	if( _row <= 0 || _row > (int) nb_Props )
	{
		wxMessageBox( wxT("GetPrevPropertyName() : out of bound param !"), wxT("Error"), wxOK | wxICON_EXCLAMATION, GetParent() );
		return wxString();
	}

	ob_object* obj = arr_Props[_row -1];
	if( obj == NULL )
		return wxString();
	return obj->name;
}

void GridOb_ObjectProperties::DeletePropAtRow( int j )
{
	// Out of bound parameter
	if( j <= 0 || j > (int) nb_Props )
	{
//	    wxMessageBox( "DeletePropAtRow() : out of bound param !", "Error", wxOK | wxICON_EXCLAMATION, GetParent() );
		return;
	}

	// GetThe prop obj
	int num_prop = j -1;
	arr_Props[num_prop]->Rm();
	for( size_t i = num_prop; i < nb_Props - 1; i++ )
		arr_Props[i] = arr_Props[i+1];
	nb_Props--;

	DeleteRows(j);
	changed = true;
}

bool GridOb_ObjectProperties::SetObjProperty_fromRow( int _row )
{
	if( theObj == NULL )
	{
		wxMessageBox( wxT("BUG\nGridOb_ObjectProperties::SetObjProperty_fromRow()\n\nNo Object associate...\n"));
		return false;
	}

	// Out of bound parameter
	if( _row < 0 || _row > (int) nb_Props )
	{
		wxMessageBox( wxT("SetObjProperty_fromRow() : out of bound param !"), wxT("Error"), wxOK | wxICON_EXCLAMATION, GetParent() );
		return false;
	}

	wxString _name = GetCellValue( _row, 0 );

	// No unamed properties
	if( _name == wxString() )
	{
		wxMessageBox( wxT("Can't set to empty property !"), wxT("Error"), wxOK | wxICON_EXCLAMATION, GetParent() );
	    SetCellValue(_row, 0, GetPrevPropertyName( _row ));
		return false;
	}

	wxString _tokens = GetCellValue( _row, 1 );

	// split the tokens
	wxArrayString t_tokens = Tokenize( _tokens );

	// modify an existing property
	if( _row != 0  )
	{
		ob_object* obj = arr_Props[_row-1];
		if( obj == NULL )
			return false;
		obj->SetProperty( _name, t_tokens );
	}
	// add a new property
	else
	{
		// Check if the property doesn't already exists
		if( theObj->GetSubObject( _name ) != NULL )
		{
			wxMessageBox( wxT("This property allready exists !"), wxT("Error"), wxOK | wxICON_EXCLAMATION, GetParent() );
			return false;
		}

		if( nb_Props >= arr_Props_size )
			ExpandArrObj();

		ob_object *new_prop = theObj->SetProperty( _name, t_tokens );

		// Decal all old rows
		for( int i = nb_Props-1; i >= 0; i-- )
			arr_Props[i+1] = arr_Props[i];
		arr_Props[0] = new_prop;
		nb_Props++;
	}

	changed = true;
	return true;
}


void GridOb_ObjectProperties::ExpandArrObj()
{
	if( arr_Props == NULL )
	{
		arr_Props = new ob_object*[20];
		arr_Props_size = 20;
		return;
	}

	ob_object** temp = new ob_object*[arr_Props_size + 20];
	for( size_t i = 0; i < nb_Props; i++ )
		temp[i] = arr_Props[i];

	ob_object **t2 = arr_Props;
	arr_Props = temp;
	delete[] t2;
	arr_Props_size += 20;
}

bool GridOb_ObjectProperties::IsToExclude( const wxString& _name, ob_object* t )
{
	bool res = false;
	for( size_t i = 0; i < filter.Count(); i++ )
	{
		if( _name.Upper() == filter[i].Upper() )
		{
			res = b_Filter_exclude;
			i = filter.Count();
		}

	if( _name.Upper().Find(wxT("UNSPECIFIED")) != wxNOT_FOUND )
		{
			res = true;
			i = filter.Count();
		}
	}
	if( !res && !b_Filter_exclude )
		return true;
	else if( res )
		return true;

	if( t != NULL && !res )
	{
		for( int i = 0; i < nb_filter_IDs; i++ )
		{
			if( t->type == filter_IDs[i] )
				return true;
		}
	}
	return false;
}


//****************************************************
// Event table
BEGIN_EVENT_TABLE(GridOb_ObjectProperties, wxGrid)
	EVT_GRIDLINE_REPOS_CURSOR(wxID_ANY, GridOb_ObjectProperties::EvtReposCursor)
	EVT_GRID_CELL_CHANGE(GridOb_ObjectProperties::EvtPropertyChange)
	EVT_GRID_SELECT_CELL(GridOb_ObjectProperties::EvtSelectChg)
	EVT_GRID_RANGE_SELECT(GridOb_ObjectProperties::EvtRgSelectChg)
	EVT_ADD_NEW_GRIDLINE(wxID_ANY, GridOb_ObjectProperties::EvtAddRow)
	EVT_SIZE(GridOb_ObjectProperties::EvtSize)
	EVT_CHAR(GridOb_ObjectProperties::EvtCharPress)
	EVT_CHAR_HOOK(GridOb_ObjectProperties::EvtCharPress)
	EVT_KEY_UP(GridOb_ObjectProperties::EvtCharPress)
END_EVENT_TABLE()


//****************************************************
//*********** A CLASS ***********************
//****************************************************


WndFromText::WndFromText( wxWindow* _parent, const wxString& _title, const wxString& path )
:wxDialog( _parent, wxID_ANY, _title, wxDefaultPosition, wxDefaultSize,
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
	wxBoxSizer *sizer_main = new wxBoxSizer( wxVERTICAL );
	wxTextCtrl *theTextCtrl = new wxTextCtrl( this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize,
				wxTE_READONLY | wxTE_BESTWRAP | wxTE_MULTILINE );
	theTextCtrl->LoadFile( path );

	sizer_main->Add( theTextCtrl, 1, wxEXPAND );

	SetSizer( sizer_main );
}

WndFromText::~WndFromText()
{
}



//****************************************************
//*********** A CLASS ***********************
//****************************************************

WndImgShow::WndImgShow( wxWindow* _parent, const wxString& _title
				, wxImage* theImg, const wxString& some_text )
:wxDialog( _parent, wxID_ANY, _title, wxDefaultPosition, wxDefaultSize,
			wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
	wxSizer* sizer_main = new wxBoxSizer( wxHORIZONTAL );
	
	MyImageCtrl* imgCtrl = new MyImageCtrl( this );
	if( theImg == NULL )
		theImg = noImg;
	imgCtrl->SetImage( theImg );
	imgCtrl->SetMinSize( wxSize( theImg->GetWidth(), theImg->GetHeight()));
	imgCtrl->SetMaxSize( wxSize( 500, 500));
	sizer_main->Add( imgCtrl, 1, wxEXPAND );
	
	if( some_text != wxString() )
	{
		wxStaticText* txtCtrl = new wxStaticText( this , wxID_ANY, some_text );
		sizer_main->Add( txtCtrl, 0, wxALL, 3 );
	}
		
			
	SetSizer( sizer_main );
	SetSize(600,500);
}
