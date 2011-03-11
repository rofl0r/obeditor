#ifndef LEVELS__PANEL_STAGEVIEW_H
#define LEVELS__PANEL_STAGEVIEW_H

//-----------------------------------------------------------------
class MyPaletteElementCtrl;
class StageView;
class StageMap;
//-----------------------------------------------------------------

//*****************************************************************

// Masks for f_hidden
enum
{
	FM_BACKGROUND = 1,
};


//-----------------------------------------------------------------
class Panel_StageView : public wxPanel
{
public:
	Panel_StageView( wxWindow* parent );
	virtual ~Panel_StageView();
	void Reset();
	
	void UpdateView();
	
	void SaveState();
	void LoadState();
	void Reload__ALL();
	
	void PANELS__Reload();
	
	void OBJECTS__Reload();
	void OBJECTS__Refresh();

	void Delete_GuiObject_of( ob_stage_object* o );
	
	int f_hidden;
public:
	StageView* stageView;
	StageMap*  stageMap;
	MyPaletteElementCtrl* bgColourChooser;
	
	/**
	 * Stage Objects Boundaries
	 */
	wxSize coords_min;
	wxSize coords_max;
	/**
	 * Coords which have to be restored
	 */
	wxSize coords_to_restore;

public:
	virtual void Refresh(bool eraseBackground = true, const wxRect* rect = NULL);
	
public:
	void Evt_ZoomP(  wxCommandEvent& evt );
	void Evt_ZoomM(  wxCommandEvent& evt );
	void Evt_ZoomReset(  wxCommandEvent& evt );
	void Evt_BG_ColourChange(  wxCommandEvent& evt );
	void Evt_BG_VisibleChange(  wxCommandEvent& evt );
	void Evt_DeleteObjects(  wxCommandEvent& evt );
	void Evt_DuplicateObjects(  wxCommandEvent& evt );
	void EvtMouse( wxMouseEvent& evt );

	DECLARE_EVENT_TABLE()
};


//*****************************************************************
//-----------------------------------------------------------------
class StageMap : public wxControl
{
public:
	StageMap(Panel_StageView* _parent );
	virtual ~StageMap();

	void Reload_Panels();
	void Reset();
	virtual void Refresh(bool eraseBackground = true, const wxRect* rect = NULL);
	
	void SCROLLBARS_Update();
	
	wxColour color_view_borders;

	enum
	{
		M_SM_INIT = 1,
	};
	int m_flag;
	bool b_must_reload_panels;

protected:
	Panel_StageView* psv;
	wxBitmap* panelBitmap;
	wxSize coords;
	bool b_refreshing;
	
protected:
	void EvtSize( wxSizeEvent& evt );
	void EvtScroll( wxScrollWinEvent& evt );
	void EvtMouse( wxMouseEvent& evt );
	void EvtPaint( wxPaintEvent& _evt );

	DECLARE_EVENT_TABLE()
};

#endif//LEVELS__PANEL_STAGEVIEW_H
