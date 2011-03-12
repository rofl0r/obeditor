#ifndef MYCONTROLS_H
#define MYCONTROLS_H

#include <vector>
#include <map>

#include <wx/combo.h>
#include <wx/listctrl.h>

//--------------------------------------------------------------------
class pListViewPopup;
class pComboCtrl_AutoComplete;
//--------------------------------------------------------------------

//--------------------------------------------------------------------
class ComboBoxAutoComplete : public wxControl
{
public:
	ComboBoxAutoComplete(wxWindow* _parent, wxWindowID id = wxID_ANY, bool _b_insensitive = true , const wxString& value = wxString(), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int more_style = 0 );
	~ComboBoxAutoComplete();
	
	virtual bool IsShownOnScreen()const;
	virtual wxSize DoGetBestSize()const;

	bool  Set_Value( const wxString& that );
	
	void  Add(const wxString& _choice, void* data = NULL );
	void  Set_Choices(const wxArrayString& _choices, void** _datas = NULL );
	void* Remove(const wxString& _choice );
	
	bool  Selection_IsAMatch();
	
	wxString  Get_Value()const;
	void*     GetSelection_Data();

	const std::vector<void*>& Get_All_Datas();
	
protected:
	pListViewPopup* thePopup;
	pComboCtrl_AutoComplete* theCombo;

	void DoSetSize(int x, int y,
				  int width, int height,
				  int sizeFlags );
	void Evt_Size(wxSizeEvent& evt);
	DECLARE_EVENT_TABLE()
};


//--------------------------------------
BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_COMBOBOXAUTOCOMPLETE_CHANGE, 7778)
    DECLARE_EVENT_TYPE(wxEVT_COMBOBOXAUTOCOMPLETE_VALID,  7878)
END_DECLARE_EVENT_TYPES()


#ifndef EVT_COMBOBOXAUTOCOMPLETE_CHANGE
#define EVT_COMBOBOXAUTOCOMPLETE_CHANGE(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_COMBOBOXAUTOCOMPLETE_CHANGE, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif


#ifndef EVT_COMBOBOXAUTOCOMPLETE_VALID
#define EVT_COMBOBOXAUTOCOMPLETE_VALID(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_COMBOBOXAUTOCOMPLETE_VALID, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif


#endif //MYCONTROLS_H
