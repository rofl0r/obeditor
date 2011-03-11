/*
 * common__validators.h
 *
 *  Created on: 27 avr. 2009
 *      Author: pat
 */

#ifndef COMMON__VALIDATORS_H_
#define COMMON__VALIDATORS_H_

#include <wx/wxprec.h>
#include <wx/validate.h>


//******************************
// Validator which only allow a set of characters

class wxValidator_Restrict : public wxValidator
{
public:
	wxValidator_Restrict( const wxString& _only_those, wxString* valPtr = NULL );
	wxValidator_Restrict(const wxValidator_Restrict& from);

	DECLARE_EVENT_TABLE();

private:
	wxString only_those;
	wxString* valPtr;

	wxObject* Clone() const;
	bool Validate(wxWindow* parent);
	bool TransferToWindow();
	bool TransferFromWindow();
	bool CharAccepted( int kc );

	void OnChar(wxKeyEvent& event);

};


//******************************
// Validator which only allow a set of characters

class wxValidator_Restrict_Range : public wxValidator
{
public:
	wxValidator_Restrict_Range(wxChar _from_here, wxChar _to_here, wxString* valPtr = NULL );
	wxValidator_Restrict_Range(const wxValidator_Restrict_Range& from);

	DECLARE_EVENT_TABLE();

private:
	wxChar from_here;
	wxChar to_here;
	wxString* valPtr;

	wxObject* Clone() const;
	bool Validate(wxWindow* parent);
	bool TransferToWindow();
	bool TransferFromWindow();
	bool CharAccepted( int kc );

	void OnChar(wxKeyEvent& event);

};







//******************************
// Validator which not allow White spaces

class wxTextValidator_NoWhiteSpace : public wxValidator
{
public:
	wxTextValidator_NoWhiteSpace( wxString* valPtr = NULL );
	wxTextValidator_NoWhiteSpace(const wxTextValidator_NoWhiteSpace& from);

	DECLARE_EVENT_TABLE();

private:
	wxString* valPtr;

	wxObject* Clone() const;
	bool Validate(wxWindow* parent);
	bool TransferToWindow();
	bool TransferFromWindow();

	void OnChar(wxKeyEvent& event);

};


//******************************
//

class wxValidatorIntegerRelative : public wxValidator
{
public:
	wxValidatorIntegerRelative( wxString* valPtr = NULL );
	wxValidatorIntegerRelative(const wxValidatorIntegerRelative& from);

	DECLARE_EVENT_TABLE();

protected:
	wxString* valPtr;

	wxObject* Clone() const;
	bool Validate(wxWindow* parent);
	bool TransferToWindow();
	bool TransferFromWindow();

	void OnChar(wxKeyEvent& event);

};


//******************************
//

class wxValidatorIntegerNegative : public wxValidatorIntegerRelative
{
public:
	wxValidatorIntegerNegative( wxString* valPtr = NULL );
	wxValidatorIntegerNegative(const wxValidatorIntegerNegative& from);

	DECLARE_EVENT_TABLE();
	void OnChar( wxKeyEvent& event );

protected:
	wxObject* Clone() const;
	bool Validate(wxWindow* parent);

};


//******************************
//

class wxValidatorFloat : public wxValidator
{
public:
	wxValidatorFloat( wxString* valPtr = NULL );
	wxValidatorFloat(const wxValidatorFloat& from);

	DECLARE_EVENT_TABLE();

protected:
	wxString* valPtr;

	wxObject* Clone() const;
	bool Validate(wxWindow* parent);
	bool TransferToWindow();
	bool TransferFromWindow();

	void OnChar(wxKeyEvent& event);

};


//******************************
//

class wxValidatorFloatPositive : public wxValidatorFloat
{
public:
	wxValidatorFloatPositive( wxString* valPtr = NULL );
	wxValidatorFloatPositive(const wxValidatorFloatPositive& from);

	DECLARE_EVENT_TABLE();

protected:
	wxString* valPtr;

	wxObject* Clone() const;
	bool Validate(wxWindow* parent);
	bool TransferToWindow();
	bool TransferFromWindow();

	void OnChar(wxKeyEvent& event);

};

#endif /* COMMON__VALIDATORS_H_ */
