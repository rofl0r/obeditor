/*
 * common__ob_controls.h
 *
 *  Created on: 28 avr. 2009
 *      Author: pat
 */

#ifndef COMMON__OB_CONTROLS_H_
#define COMMON__OB_CONTROLS_H_

#include <wx/event.h>
#include <wx/filepicker.h>
#include <vector>
#include <map>

#include "common__tools.h"
#include "common__globals.h"

//****************************************************
//****************************************************
typedef struct
{
	int num_token;
	wxString tag;
}
atom_path;

class ob_token_path
{
public:
	ob_token_path(){};
	ob_token_path( const wxString& tag, int num_token = -1 );
	ob_token_path(const ob_token_path& ob_tp
	, const wxString& subtag = wxString()
			, int	num_sub_token = -1 );
	ob_token_path( ob_token_path& src );
	ob_token_path& operator=( const ob_token_path& obtp );
	~ob_token_path();

	bool IsEmpty();
	wxString GetPath();

	bool Append__SubTag( const wxString& subtag );
	bool Append__SubToken( int num_sub_token );
	
	ob_token_path* BuildPath_Sibling_Token( int decal );


	ob_object* Resolve_With( ob_object* _o );
	wxString Get_With( ob_object* _o );
	bool     Set_To( ob_object* _o, wxString& val );
	
protected:
	void Clear_l_atoms();
	std::list<atom_path*> l_atoms;
};

//****************************************************
//****************************************************
enum
{
	  OBPROP_DONOTHING	= 0x0001
	, OBPROP_DEL_TAG		= 0x0002
	, OBPROP_TRUNCATE		= 0x0003

	, OBPROP_DEF_MASK		= 0x00FF

	, OBPROP_SETDEF_IFEMPTY = 0x0100
};


class ob_property : public wxEvtHandler
{
public:
	static wxColour default_values_color;
public:
	// Facilities for ComboBoxes
	static void SetRange(wxWindow* comboBox, int min,int max);
	static void SetEnums(	  wxWindow* comboBox
					, const wxArrayString& _displayed
					, const wxArrayString& _values
					);

public:
	ob_property(  const wxString& tag,const int num_token = 0
			, int type_prop = PROPTYPE_STRING
			, const wxString& def_val = wxString(), int do_on_default = OBPROP_DEL_TAG);
	~ob_property();

	wxSizer* BuildControls( 
		  ob_object* obj
		, wxWindow* parent
		, wxWindow*& res_Ctrl
		, const wxString& ctrlName, int ctrl_w = wxDefaultCoord
		, const wxString& str_label = wxString(), int label_w = wxDefaultCoord
		, int ctrl_more_styles = 0 );

	//-------------------------------------------------------------
	// Getting the special default val for this prop 
	// ex : return the pointed entity prop if referer_path is set
	wxString Get_SpecialDefault_Val(ob_object* o );
	bool     Have_SpecialDefault_Val(ob_object* o );
	
	// Return special val if there is one, else def_val
	wxString Get_Default_Val(ob_object* _o );

	wxString Get_Curr_Value(ob_object* _o);
	
	bool Update_CtrlVal( wxWindow* ctrl );

public:
	wxString prefix;
	wxString tag;
	int num_token;
	int type_prop;
	wxString def_val;
	int do_on_default;
	
	// If the prop have a referer tag for his default value
	//  (ex: a spawn have some referers to entity associated for health, score, etc...)
	ob_token_path referer_path;


protected:
	ob_object* CreateDefaultObject(ob_object* o);
	void pUpdate( ob_object* obj, wxWindow* theCtrl, const wxString& val );
	void MayTruncate(ob_object* obj, ob_object* subobj, wxWindow* theCtrl );
	void Signal_Modified( ob_object* obj, wxWindow* theCtrl);

protected:
	void Evt_TxtChange(wxCommandEvent& evt);
	void Evt_FileChange(wxCommandEvent& evt);
	void Evt_ChckChange(wxCommandEvent& evt);
	void Evt_Combo(wxCommandEvent& evt);
};

extern std::map<wxString,ob_property*> ob_props;

#ifndef EVT_OBPROP_CHANGE
#define EVT_OBPROP_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_OBPROP_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif


//****************************************************
//****************************************************
class KalachnikofButton: public wxBitmapButton
{
public:
	KalachnikofButton(wxWindow* parent, wxWindowID id, const wxBitmap& bitmap, const size_t _clicks_interval_ms = 200, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxBU_AUTODRAW, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxT("button") );
	~KalachnikofButton();

	size_t clicks_interval;
protected:
	wxTimer* theTimer;
	bool b_clickedOn;

protected:
	void EvtTimerOut( wxTimerEvent& event );
	void EvtMouseSomething( wxMouseEvent& event );
	
	DECLARE_EVENT_TABLE()
};

//****************************************************

class prObFileChooserCtrl : public wxControl
{
public:
	prObFileChooserCtrl(
			  wxWindow *_parent
			, const int _id = wxID_ANY
			, const wxString& _File_or_Directory = wxString()
			, const bool _b_file_must_exist = true
			, const int _width =  wxDefaultCoord
			, const int style = 0
			);
	~prObFileChooserCtrl();

	bool SetFullPath( const wxString& _File );
	bool SetObPath( const wxString& _File );
	wxFileName GetFileName();
	wxString   GetFullPath();
	wxString   GetObPath();

	virtual int Update(bool b_filePicker );

protected:
	int minWidth;
	wxString File_or_Directory;

	wxTextCtrl *txtCtrl_file;
	wxFilePickerCtrl *filePck_file;

	bool b_updating;
	bool b_file_must_exist;
	bool b_init;

	virtual wxSize DoGetBestSize() const;
	virtual void DoSetSize(int x, int y, int width, int height);
	virtual wxSize GetMinSize() const;

	void Init();
	void EvtSize(wxSizeEvent& event);
	void EvtGetFocus( wxFocusEvent& evt );
	void EvtCharPress( wxKeyEvent& event);
	void EvtImgPickerChg(wxFileDirPickerEvent& event );
	void EvtPaint(  wxPaintEvent& WXUNUSED(event));
	void EvtImgPickerClick( wxMouseEvent& event );


    DECLARE_EVENT_TABLE()

};


//****************************************************

#ifndef EVT_OBFILECHOOSER_CHANGE
#define EVT_OBFILECHOOSER_CHANGE(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_OBFILECHOOSER_CHANGE, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif




//****************************************************
//************ BASE SCROLL PANNEL CLASSES  ***********
//****************************************************
class AScrollPanel : public wxScrolledWindow
{
public :
	AScrollPanel( wxWindow *_parent );
	~AScrollPanel();
	void EvtSize( wxSizeEvent& event );
	virtual bool Layout();
	virtual wxSize GetMinSize()const;

	void Set_FixedDirections( bool b_H, bool b_V );  // Horizontable length fix ?  Verticable length fix ?

protected:
	virtual wxSize DoGetBestSize() const;
	virtual void DoGetClientSize( int* width, int* height) const;
	virtual void DoSetSize(int x, int y, int width, int height, int sizeFlags);

	void OnPaint( wxPaintEvent &WXUNUSED(event) );

	bool b_fix_V;
	bool b_fix_H;

	int h;
	DECLARE_EVENT_TABLE()
};




#endif /* COMMON__OB_CONTROLS_H_ */


