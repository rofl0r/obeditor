/*
 * MyPalette.h
 *
 *  Created on: 9 nov. 2008
 *      Author: pat
 */

#ifndef MYPALETTE_H_
#define MYPALETTE_H_

#include <wx/control.h>
#include <wx/image.h>

//****************************************************
//************ PALETTE CLASS  **************
//****************************************************
class MyPalette : public wxPalette
{
public:
	MyPalette();
	MyPalette( wxPalette& _palette );
	MyPalette( wxImage& _img );
	~MyPalette();

	bool LoadFrom( wxString& _path_palette );
	bool LoadFrom( wxImage _img );
	bool SaveAs( wxString& _fullpath );
};

//****************************************************
//************ PALETTE ELEMENT CONTROL  **************
//****************************************************

// Throw 
//	-  wxPALETTE_ELT_CLICKED when clicked
//	-  wxPALETTE_ELT_COLOR_CHANGE when color change


enum
{ 
	AUTO_ELTCOLOR,	// Standalone colour chooser
	ELTPALETTE,		// the colour chooser is part of a palette Ctrl
};

class MyPaletteElementCtrl : public wxControl
{
public:
	MyPaletteElementCtrl(wxWindow* _parent, int _mode = ELTPALETTE );
	void SetMode( int new_mode );
	~MyPaletteElementCtrl();

	void SetRGB( unsigned char _r, unsigned char _g, unsigned char _b );
	void GetRGB( unsigned char* _r, unsigned char* _g, unsigned char* _b );
	void SetColor(const wxColor& _color );
	wxColour GetColor();
	void ComputeColorTrans( int hue, int sat, int luz );
	void ComputeColorDecal( int red, int green, int blue );
	void RestoreRGBs();
	void CommitNewColours();

	virtual void Update();
	virtual void Refresh();
	void OnPaint(wxPaintEvent& event);
	void DoPainting( wxDC& dc );

	bool Equal( wxColour& _col );
	void EvtDoubleClick(wxMouseEvent& event);
	void EvtClick(wxMouseEvent& event);

	unsigned char r,g,b,a;
	unsigned char origin_r, origin_g, origin_b;
	int hue,sat,lum;
	int selected_state; // 0==NOT ; 1==SELECTED; 2==ASSOCIATED
	int mappedTo; // The index map for this index

	virtual wxSize GetMinSize() const;
protected:
	int eltmode;
    DECLARE_EVENT_TABLE()
};


//****************************************************
//************ PALETTE CONTROL  **************
//****************************************************
class ob_object;
class imgFile;

class MyPaletteCtrl : public wxWindow
{
public:
	static bool mode8bit;
public:
	MyPaletteCtrl(wxWindow* _parent );
	void Constructor();
	virtual ~MyPaletteCtrl();

	virtual void Refresh();
	virtual bool Layout();

	void SetAssociatedObject( ob_object* _obj );
	void Reset(bool b_layout =  true);

	bool TryToInitWithImage( wxString& strpath);
	bool InitRemapping_With( wxString& strpath );
	void UndoRemapping();

	//-----------------------------------------
	// <add_mode> meaning for 16bit mode 
	// 0 => deselect other color and toggle select state for this color
	// 1 => toggle selection state of this colour
	// 2 => add this colour to selection
	// <add_mode> meaning for 8bit mode 
	// >  0 => new remapping or undo remapping 
	// == 0 => choose new source colour for remap
	void SelectColour( int ind, int add_mode = 0 );
	
	void EvtPaletteEltClicked( wxCommandEvent& event );
	void EvtPaletteEltColorChange( wxCommandEvent& event );
	void DeselectEveryThing();
	void DoGuessColors();
	void DoFixSelectedColors();
	void UnDoFixSelectedColors();

	wxString FullPath()const;
	wxString OBPath()const;
	bool IsInit()const;
	wxPalette GetNewPalette();

	ob_object *obj_associated;
	bool isChanged;
	bool b_init;

// 	MyPaletteElementCtrl* PalElt_clicked;
	
	virtual void EvtSize( wxSizeEvent& event );
	void OnPaint(wxPaintEvent& event);

	wxBoxSizer *mainSizer;
	wxGridSizer *gridSizer;
	wxStaticText *error_text;

	MyPalette* thePalette;
	MyPaletteElementCtrl** paletteElements;
	int nb_elts;

	void Reset_HSL();
	void Apply_HSL();
	void ValidateHSLs();
	void InValidateHSLs();

	void Set_Hue( int _hue );
	void Set_Sat( int _sat );
	void Set_Luz( int _luz );

	void Set_Red( int _red );
	void Set_Green( int _green );
	void Set_Blue( int _blue );

	int hue, sat, luz;
	int red, green, blue;

	void SetAssociations();
	imgFile* theSourceImg;

protected:
	void pr_Make_SrcPalette_To_PaletteElts(bool b_update_sizer );
	MyPaletteElementCtrl* curr_selected;
    DECLARE_EVENT_TABLE()
};


//****************************************************
//************ NEW EVENT  **************
//****************************************************

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_EVENT_TYPE(wxPALETTE_ELT_CLICKED, 7777)
	DECLARE_EVENT_TYPE(wxPALETTE_ELT_COLOR_CHANGE, 7777)
END_DECLARE_EVENT_TYPES()

// define an event table macro for this event type
#ifndef PALETTE_ELT_CLICKED
#define PALETTE_ELT_CLICKED(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    		wxPALETTE_ELT_CLICKED, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif


// define an event table macro for this event type
#ifndef PALETTE_ELT_COLOR_CHANGE
#define PALETTE_ELT_COLOR_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    	wxPALETTE_ELT_COLOR_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif



#endif /* MYPALETTE_H_ */
