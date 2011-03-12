/*
 * common__validators.cpp
 *
 *  Created on: 27 avr. 2009
 *      Author: pat
 */

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "common__validators.h"

int StrToInt( const wxString& str, bool& ok );
int StrToInt( const wxString& str );
wxString IntToStr(const int _v );
wxString FloatToStr( const float f);
float StrToFloat(const wxString& str, bool& ok );

bool IsEditableKeyCode( int kc )
{
	return kc != WXK_BACK
		&& kc != WXK_TAB
		&& kc != WXK_RETURN
		&& kc != WXK_ESCAPE
		&& kc != WXK_DELETE
		&& kc != WXK_END
		&& kc != WXK_HOME
		&& kc != WXK_LEFT
		&& kc != WXK_UP
		&& kc != WXK_RIGHT
		&& kc != WXK_DOWN
		&& kc != WXK_INSERT
		&& kc != WXK_PAGEUP
		&& kc != WXK_PAGEDOWN
		&& ( kc < WXK_F1 || kc < WXK_F24 );
}


//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
// Event table
BEGIN_EVENT_TABLE(wxValidator_Restrict, wxValidator)
    EVT_CHAR(wxValidator_Restrict::OnChar)
END_EVENT_TABLE()


//*********************************************************************
// Constructor
wxValidator_Restrict::wxValidator_Restrict( const wxString& _only_those,  wxString* _valPtr)
{
	only_those = _only_those;
	valPtr = _valPtr;
}


//*********************
// Copy constructor
wxValidator_Restrict::wxValidator_Restrict(const wxValidator_Restrict &from)
: wxValidator()
{
	valPtr = from.valPtr;
	only_those = from.only_those;
}



//*********************
// Clone
wxObject* wxValidator_Restrict::Clone() const
{
	wxValidator_Restrict *clone = new wxValidator_Restrict(only_those, valPtr );
	return clone;
}


////
// Validate
bool wxValidator_Restrict::Validate(wxWindow* parent)
{
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled())
    	return true;

    for( size_t i = 0; i < value.Len(); i ++ )
    	if( ! CharAccepted( value[i] ) )
    		return false;

	return true;
}


//********//////
bool wxValidator_Restrict::CharAccepted( int kc )
{
	bool b_ok = false;
	for( size_t i =0; i< only_those.Len(); i ++)
	{
		if( kc == only_those[i] )
		{
			b_ok = true;
			break;
		}
	}
	return b_ok;
}

//********//////
// Filter keypresses
void wxValidator_Restrict::OnChar(wxKeyEvent& event)
{
	int chr = event.GetKeyCode();

	if( ! IsEditableKeyCode( chr ) )
	{
		event.Skip();
		return;
	}


	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Not OK
	if( ! CharAccepted( chr ) )
	{
		wxBell();
		return;
	}

	// OK
	event.Skip();
	return;
}


//********///////
// Transfer to window
bool wxValidator_Restrict::TransferToWindow()
{
	return true;
}


//*************
// Receive from window
bool wxValidator_Restrict::TransferFromWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	return true;
}




//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
// Event table
BEGIN_EVENT_TABLE(wxValidator_Restrict_Range, wxValidator)
    EVT_CHAR(wxValidator_Restrict_Range::OnChar)
END_EVENT_TABLE()


//********
// Constructor
wxValidator_Restrict_Range::wxValidator_Restrict_Range(wxChar _from_here, wxChar _to_here, wxString* _valPtr)
{
	from_here = _from_here;
	to_here = _to_here;
	valPtr = _valPtr;
}


//*********************
// Copy constructor
wxValidator_Restrict_Range::wxValidator_Restrict_Range(const wxValidator_Restrict_Range &from)
: wxValidator()
{
	valPtr = from.valPtr;
	from_here = from.from_here;
	to_here = from.to_here;
}



//*********************
// Clone
wxObject* wxValidator_Restrict_Range::Clone() const
{
	wxValidator_Restrict_Range *clone = new wxValidator_Restrict_Range(from_here,to_here, valPtr );
	return clone;
}


////
// Validate
bool wxValidator_Restrict_Range::Validate(wxWindow* parent)
{
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled())
    	return true;

    for( size_t i = 0; i < value.Len(); i ++ )
    	if( ! CharAccepted( value[i] ) )
    		return false;

	return true;
}


//********//////
bool wxValidator_Restrict_Range::CharAccepted( int kc )
{
	return ( kc >= from_here && kc <= to_here );
}

//********//////
// Filter keypresses
void wxValidator_Restrict_Range::OnChar(wxKeyEvent& event)
{
	int chr = event.GetKeyCode();

	if( ! IsEditableKeyCode( chr ) )
	{
		event.Skip();
		return;
	}


	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Not OK
	if( ! CharAccepted( chr ) )
	{
		wxBell();
		return;
	}

	// OK
	event.Skip();
	return;
}


//********///////
// Transfer to window
bool wxValidator_Restrict_Range::TransferToWindow()
{
	return true;
}


//*************
// Receive from window
bool wxValidator_Restrict_Range::TransferFromWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	return true;
}




//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(wxTextValidator_NoWhiteSpace, wxValidator)
	EVT_CHAR(wxTextValidator_NoWhiteSpace::OnChar)
END_EVENT_TABLE()

wxTextValidator_NoWhiteSpace::wxTextValidator_NoWhiteSpace( wxString* _valPtr )
{
	valPtr = _valPtr;
}

//*********************
// Copy constructor
wxTextValidator_NoWhiteSpace::wxTextValidator_NoWhiteSpace(const wxTextValidator_NoWhiteSpace &from)
: wxValidator()
{
	valPtr = from.valPtr;
}



//*********************
// Clone
wxObject* wxTextValidator_NoWhiteSpace::Clone() const
{
	wxTextValidator_NoWhiteSpace *clone = new wxTextValidator_NoWhiteSpace(valPtr );
	return clone;
}




//*********************

void wxTextValidator_NoWhiteSpace::OnChar( wxKeyEvent& evt )
{
	int kc = evt.GetKeyCode();

	if( kc == WXK_SPACE )
		wxBell();
	else
		evt.Skip();
}

//*************************
// Validate
bool wxTextValidator_NoWhiteSpace::Validate(wxWindow* parent)
{
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled())
    	return true;

	return true;
}



//*****************
// Transfer to window
bool wxTextValidator_NoWhiteSpace::TransferToWindow()
{
	return true;
}


//*************
// Receive from window
bool wxTextValidator_NoWhiteSpace::TransferFromWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	return true;
}



//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(wxValidatorIntegerRelative, wxValidator)
	EVT_CHAR(wxValidatorIntegerRelative::OnChar)
END_EVENT_TABLE()

wxValidatorIntegerRelative::wxValidatorIntegerRelative( wxString* _valPtr )
{
	valPtr = _valPtr;
}

//*********************
// Copy constructor
wxValidatorIntegerRelative::wxValidatorIntegerRelative(const wxValidatorIntegerRelative &from)
: wxValidator()
{
	valPtr = from.valPtr;
}



//*********************
// Clone
wxObject* wxValidatorIntegerRelative::Clone() const
{
	wxValidatorIntegerRelative *clone = new wxValidatorIntegerRelative(valPtr );
	return clone;
}




//*********************

void wxValidatorIntegerRelative::OnChar( wxKeyEvent& event )
{
	int kc = event.GetKeyCode();

	if( ! IsEditableKeyCode( kc ) )
	{
		event.Skip();
		return;
	}

	if( kc != '0' && kc != '1' && kc != '2' && kc != '3' && kc != '4' && kc != '5'
		&& kc != '6' && kc != '7' && kc != '8' && kc != '9' && kc != '-'   )
	{
		wxBell();
		return;
	}

	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();
	int ind = ctrl->GetInsertionPoint();

	if( kc == '-' && ind > 0 )
	{
		wxBell();
		return;
	}

	event.Skip();
}

//*************************
// Validate
bool wxValidatorIntegerRelative::Validate(wxWindow* parent)
{
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled())
    	return true;

    if( value == wxString() )
    	return true;

    bool ok;
    StrToInt( value, ok );
    return ok;
}



//*****************
// Transfer to window
bool wxValidatorIntegerRelative::TransferToWindow()
{
	return true;
}


//*************
// Receive from window
bool wxValidatorIntegerRelative::TransferFromWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	return true;
}



//*************************
//*************************
//*************************
//*************************
//*************************

BEGIN_EVENT_TABLE(wxValidatorIntegerNegative, wxValidator)
	EVT_CHAR(wxValidatorIntegerNegative::OnChar)
END_EVENT_TABLE()


wxValidatorIntegerNegative::wxValidatorIntegerNegative( wxString* _valPtr )
{
	valPtr = _valPtr;
}

//*********************
// Copy constructor
wxValidatorIntegerNegative::wxValidatorIntegerNegative(const wxValidatorIntegerNegative &from)
: wxValidatorIntegerRelative()
{
	valPtr = from.valPtr;
}



//*********************
// Clone
wxObject* wxValidatorIntegerNegative::Clone() const
{
	wxValidatorIntegerNegative *clone = new wxValidatorIntegerNegative(valPtr );
	return clone;
}

//*************************
// Validate
bool wxValidatorIntegerNegative::Validate(wxWindow* parent)
{
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled())
    	return true;

    if( value == wxString() )
    	return true;

    bool res = (StrToInt( value ) <= 0);

    return res;
}


//*************************

void wxValidatorIntegerNegative::OnChar( wxKeyEvent& event )
{
	int kc = event.GetKeyCode();

	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();
	int ind = ctrl->GetInsertionPoint();

	bool res = true;

	if( kc == WXK_BACK && ind == 1 &&
		value.Len() > 1 && value[0] == '-' )
		res = false;
	if( kc == WXK_DELETE && ind == 0 && value.Len() > 1 && value[0] == '-' )
		res = false;

	if( ! res )
	{
		wxBell();
		return;
	}

	if( ! IsEditableKeyCode( kc ) )
	{
		event.Skip();
		return;
	}

	if( ( kc < '0' || kc > '9') && kc != '-'   )
	{
		wxBell();
		return;
	}

	res = false;
	if( kc == '-' && ind == 0 && ( value.Len() == 0 || value[0] != '-') )
		res = true;

	else if( kc != '-' &&
				( (ind > 0 && value[0] == '-' )
				||
				  ( kc == '0' && value.Len() == 0 )
				)
			)
		res = true;

	if( ! res )
	{
		wxBell();
		return;
	}

	event.Skip();
}


//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(wxValidatorFloat, wxValidator)
	EVT_CHAR(wxValidatorFloat::OnChar)
END_EVENT_TABLE()

wxValidatorFloat::wxValidatorFloat( wxString* _valPtr )
{
	valPtr = _valPtr;
}

//*********************
// Copy constructor
wxValidatorFloat::wxValidatorFloat(const wxValidatorFloat &from)
: wxValidator()
{
	valPtr = from.valPtr;
}



//*********************
// Clone
wxObject* wxValidatorFloat::Clone() const
{
	wxValidatorFloat *clone = new wxValidatorFloat(valPtr );
	return clone;
}




//*********************

void wxValidatorFloat::OnChar( wxKeyEvent& event )
{
	int kc = event.GetKeyCode();

	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();
	int ind = ctrl->GetInsertionPoint();

	if( ! IsEditableKeyCode( kc ) )
	{
		event.Skip();
		return;
	}

	if( ( kc < '0' || kc > '9') && kc != '-'   && kc != '.' )
	{
		wxBell();
		return;
	}

	bool res = true;
	if( kc == '-')
	{
		if( ind != 0 )
			res = false;
		else if( value.Find( '-' ) != wxNOT_FOUND )
			res = false;
	}

	else if( kc == '.' )
	{
		if( value.Find( '.' ) != wxNOT_FOUND )
			res = false;

		else if( ind == 0 && value[0] == '-' )
			res = false;
	}

	if( ! res )
	{
		wxBell();
		return;
	}

	/*
	bool b_ok;
	wxMessageBox( FloatToStr( StrToFloat( value, b_ok )));
*/
	event.Skip();
}

//*************************
// Validate
bool wxValidatorFloat::Validate(wxWindow* parent)
{
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled())
    	return true;

    if( value == wxString() )
    	return true;

    bool ok;
    StrToInt( value, ok );
    return ok;
}



//*****************
// Transfer to window
bool wxValidatorFloat::TransferToWindow()
{
	return true;
}


//*************
// Receive from window
bool wxValidatorFloat::TransferFromWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	return true;
}


//****************************************************
//****************************************************
//****************************************************

BEGIN_EVENT_TABLE(wxValidatorFloatPositive, wxValidator)
	EVT_CHAR(wxValidatorFloatPositive::OnChar)
END_EVENT_TABLE()

wxValidatorFloatPositive::wxValidatorFloatPositive( wxString* _valPtr )
:wxValidatorFloat( _valPtr )
{}


//*********************
// Copy constructor
wxValidatorFloatPositive::wxValidatorFloatPositive(const wxValidatorFloatPositive &from)
{
	valPtr = from.valPtr;
}



//*********************
// Clone
wxObject* wxValidatorFloatPositive::Clone() const
{
	wxValidatorFloatPositive *clone = new wxValidatorFloatPositive(valPtr );
	return clone;
}




//*********************

void wxValidatorFloatPositive::OnChar( wxKeyEvent& event )
{
	int kc = event.GetKeyCode();

	if( kc == '-' )
	{
		wxBell();
		return;
	}

	wxValidatorFloat::OnChar( event );
}

//*************************
// Validate
bool wxValidatorFloatPositive::Validate(wxWindow* parent)
{
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled())
    	return true;

    if( value == wxString() )
    	return true;

    bool ok;
    StrToInt( value, ok );
    return ok;
}



//*****************
// Transfer to window
bool wxValidatorFloatPositive::TransferToWindow()
{
	return true;
}


//*************
// Receive from window
bool wxValidatorFloatPositive::TransferFromWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	return true;
}

