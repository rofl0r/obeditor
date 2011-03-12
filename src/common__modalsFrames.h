#ifndef MYMODALSFRAMES_H_
#define MYMODALSFRAMES_H_

#include <wx/event.h>
#include <wx/grid.h>

#include "common__globals.h"

//*****************************************************************
//*****************************************************************
class prObFileChooserCtrl;
class ob_object;

//*****************************************************************
// propsNames examples:
//  "_NAME_"    -> the object name
//  "_TOKEN_3"  -> the 3rd token of the object
//  "_SUBOBJ_NUM%4%PROP%shape"  -> the shape property of the 4d subobject
//  "_SUBOBJ_NAME%boule%TOKEN%3"  -> the 3rd token of the subobject "boule"
//  "_SUBOBJ_NAME%boule%NUM%4%PROP%shape"  -> the shape property of the 4d boule subobject
//  "_SUBOBJ_TYPE%balon%NUM%4%TOKEN%3"  -> the 3rd token of the 4d subobject of type balon
//*****************************************************************

const wxArrayString arrStr_empty;

class frameEdit_request
{
public:
	frameEdit_request(
		const wxString& _propname = wxT("UNNAMED"),
		const wxString& _label = wxT("NO-LABEL"),
		int _type = PROPTYPE_UNDEF,
		const wxString& _request = wxT("_TOKEN_0"),
		bool _b_del_ifempty = false,
		wxArrayString _enums = arrStr_empty,
		int _labelw = wxDefaultCoord,
		int _fieldw = wxDefaultCoord
	);
	void Set(
		const wxString _propname = wxT("UNNAMED"),
		const wxString _label = wxT("NO-LABEL"),
		int _type = PROPTYPE_STRING_NO_WS,
		const wxString _request = wxT("_TOKEN_0"),
		bool _b_del_ifempty = true,
		wxArrayString _enums = arrStr_empty,
		int _labelw = wxDefaultCoord,
		int _fieldw = wxDefaultCoord
	);

	~frameEdit_request();

	void SetPropType( int _proptype );
	int  GetPropType();

	wxString request;
	wxString label;
	bool b_del_ifempty;
	wxString name;
	wxArrayString enums;
	wxWindow* ctrl;
	int labelw;
	int fieldw;
	wxString defVal;
	bool b_optional;
	wxString* missing_tokens;
	size_t nb_missing_tokens;

protected:
	int type;
};

class FrameEditProperties : public wxDialog
{
public:
	FrameEditProperties(wxWindow *_parent, ob_object* _theObj = NULL,
			const wxString& _title = wxT("NO-TITLE"),
			int more_style = 0 );
	~FrameEditProperties();

	void EvtClose( wxCloseEvent& event );
	void EvtClickOK(wxCommandEvent& evt );
	void EvtClickCancel(wxCommandEvent& evt );

	ob_object* theObj;
	bool b_newObj;
	bool b_chg;

protected:
	virtual wxString GetObjValue( int ind );
	virtual bool SetObjValue( int ind, const wxString& val, const wxString& origin_val );

	virtual bool IsChanged();
	virtual int  WriteChangesToObject();

	virtual void Clicked_OK(bool b_close = true);
	virtual void Clicked_Cancel(bool b_close = true);

	void Init();
	virtual void InitControls();
	virtual void InitProps();
	virtual void InitObj();
	void SetDatas();


	wxSizer *sizerMain;
	wxBoxSizer *sizerTop;

	int nb_props;
	frameEdit_request* reqs;
	wxString wndSavingName; // For save and restore coords

	bool b_ok;
	bool b_creation;  //True -> an subobject will be create on write

    DECLARE_EVENT_TABLE()
};






//*****************************************************************
// Init these var before create the GridPropFrame
#define MAXGRIDPROPFRAME_PROPSSIZE 200

class wxGridEvent;
class GridOb_ObjectProperties;

class GridPropFrame : public wxDialog
{
public:
	GridPropFrame( wxWindow *parent,
					ob_object* _theObj,
					const wxString& Title =wxT("Set some properties"),
					const wxString& _intro =wxT("You can change/add/del properties here"),
					long style = wxDEFAULT_DIALOG_STYLE, bool b_sort = true );
	~GridPropFrame();

	void SetFilter_Properties( wxArrayString& _filter, bool b__exclude = true);
	void SetFilter_Properties( int* arrObjIDs, int nb_ids);

	void EvtClose( wxCloseEvent& event );
	void EvtSize(wxSizeEvent& event);
	void EvtCharPress(wxKeyEvent& event);
	void EvtOkClicked( wxCommandEvent& event);
	int Changed();

protected:
	wxStaticText *txtMessage;
	GridOb_ObjectProperties *list_Props;

    DECLARE_EVENT_TABLE()
};


class GridOb_ObjectProperties : public wxGrid
{
public:
	GridOb_ObjectProperties( wxWindow *parent, bool b_sort = true);
	~GridOb_ObjectProperties();

	void SetFilter_Properties( wxArrayString& _filter, bool b__exclude = true);
	void SetFilter_Properties( int* arrObjIDs, int nb_ids);
	void SetObjectHandled( ob_object* _theObj );
	void Clear();
	void RefreshProperties();

	bool changed;

protected:
	void _AppendProp( ob_object* theProp, int& row_num  );
	void UpdatePropGridSize();
	void Init( bool b_sort = true);

	void EvtSize(wxSizeEvent& event);
	void EvtPropertyChange( wxGridEvent& event );
	void EvtAddRow(wxCommandEvent& event);
	void EvtClose( wxCloseEvent& event );
	void EvtReposCursor( wxCommandEvent& event );
	void EvtCharPress(wxKeyEvent& event);
	void EvtSelectChg( wxGridEvent& event );
	void EvtRgSelectChg( wxGridRangeSelectEvent& event );

	void DeletePropAtRow( int j );
	bool SetObjProperty_fromRow( int _row );
	wxString GetPrevPropertyName( int _row );
	void ExpandArrObj();
	bool IsToExclude( const wxString& _name, ob_object* t  = NULL );

	int new_row_pos;
	int new_col_pos;

	bool b_sort;
	bool b_sizing;
	bool b_changing_prop;
	bool b_destroying;
	wxArrayString filter;
	int nb_filter_IDs;
	int*  filter_IDs;

	bool b_Filter_exclude;

	ob_object* theObj;
	ob_object** arr_Props;
	size_t arr_Props_size;
	size_t nb_Props;

	virtual bool ProcessEvent( wxEvent& event );

    DECLARE_EVENT_TABLE()
};

class WndFromText : public wxDialog
{
public:
	WndFromText( wxWindow* _parent, const wxString& _title, const wxString& path );
	~WndFromText();
};


//-----------------------------------------------------------
class WndImgShow : public wxDialog
{
public:
	WndImgShow( wxWindow* parent, const wxString& _title
	, wxImage* _theImg, const wxString& some_text = wxString() );
};

#endif /*MYMODALSFRAMES_H_*/
