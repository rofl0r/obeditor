/*
 * MyPalette.cpp
 *
 *  Created on: 9 nov. 2008
 *      Author: pat
 */

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/colordlg.h>
#include "common__tools.h"
#include "images__MyPalette.h"

#define ID_FIRST_PALETTE_ELT (wxID_HIGHEST+1)

//****************************************************
//************ PALETTE CLASS  **************
//****************************************************

MyPalette::MyPalette()
:wxPalette()
{
}


//****************************************************

MyPalette::MyPalette( wxPalette& _palette )
:wxPalette( _palette )
{
}


//****************************************************

MyPalette::MyPalette( wxImage& _img )
:wxPalette( _img.GetPalette() )
{
}


//****************************************************

MyPalette::~MyPalette()
{
}


//****************************************************

bool MyPalette::LoadFrom( wxImage _img )
{
	wxPalette::operator=( _img.GetPalette() );
	return true;
}


//****************************************************

bool MyPalette::SaveAs( wxString& _fullpath )
{
	return true;
}


//****************************************************
//****************************************************
//****************************************************
//****************************************************
//************ PALETTE ELEMENT CONTROL  **************
//****************************************************

DEFINE_EVENT_TYPE(wxPALETTE_ELT_CLICKED)
DEFINE_EVENT_TYPE(wxPALETTE_ELT_COLOR_CHANGE)

#define PALELT_SIZE 20
#define PALELT_COLOR_DECAL 4

MyPaletteElementCtrl::MyPaletteElementCtrl(wxWindow* _parent, int _mode )
:wxControl( _parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE )
{
	r=g=b=a=0;
	hue=sat=lum=0;
	selected_state=0;
	mappedTo = -1;
	SetMinSize( wxSize( 15, 15));
	SetMode( _mode );
}


//****************************************************

MyPaletteElementCtrl::~MyPaletteElementCtrl()
{
}


//****************************************************

void MyPaletteElementCtrl::SetMode( int new_mode )
{
	eltmode = new_mode;
}


//****************************************************

void MyPaletteElementCtrl::SetRGB( unsigned char _r, unsigned char _g, unsigned char _b )
{
	origin_r = r = _r;
	origin_g = g = _g;
	origin_b = b = _b;
}


//****************************************************

void MyPaletteElementCtrl::RestoreRGBs()
{
	r = origin_r;
	g = origin_g;
	b = origin_b;
}

//****************************************************

void MyPaletteElementCtrl::CommitNewColours()
{
	origin_r = r;
	origin_g = g;
	origin_b = b;
}

//****************************************************
// Algorithm from Wikipedia

#define MAX(a,b) ((a>b)?a:b)
#define MIN(a,b) ((a<b)?a:b)

void RGB_to_HSL( unsigned char _r, unsigned char _g, unsigned char _b, float& _h, float& _s, float& _l )
{
	float r = (float) _r / 255;
	float g = (float) _g / 255;
	float b = (float) _b / 255;


	float max = MAX( r, MAX( g, b));
	float min = MIN( r, MIN( g, b));

	// Computation of h
	if( max == min )
		_h = 0;
	else if( max == r )
	{
		_h = (g-b) /(max-min)*60;
		if( _h < 0 )
			_h += 360;
	}
	else if( max == g )
	{
		_h = (b-r) /(max-min)*60 + 120;
	}
	else
	{
		_h = (r-g) /(max-min)*60 + 240;
	}

	// Computation of l
	_l = 0.5 * (max+min);


	// Computation of s
	if( max == min )
		_s = 0;
	else if( _l < 0.5 )
		_s = (max-min) / (2*_l);
	else
		_s = (max-min) / (2 - 2*_l);
}


//****************************************************

void HSL_to_RGB( unsigned char& _r, unsigned char& _g, unsigned char& _b, float h, float s, float l )
{
	float q;
	if( l < 0.5 )
		q = l * (1+s );
	else
		q = l + s - l*s;

	float p = 2*l - q;

	float hk = h ;

	float t[3];
	t[0] = hk + (float)1/(float)3;
	t[1] = hk;
	t[2] = hk - (float)1/(float)3;

	for (int i = 0 ; i < 3 ; i++ )
	{
		if( t[i] > 1 )
			t[i] = t[i] - 1;
		if( t[i] <= 0 )
			t[i] = t[i] + 1;
	}

	float col[3];
	for (int i = 0 ; i < 3 ; i++ )
	{
		if( t[i] < (float)1/(float)6 )
			col[i] = p + ((q-p) * 6 * t[i] );
		else if( t[i] < 0.5 )
			col[i] = q;
		else if( t[i] < (float)2/(float)3 )
			col[i] = p + ((q-p) * 6 * ( (float)2/(float)3 - t[i] ));
		else
			col[i] = p;
	}

	_r = col[0] * 255;
	_g = col[1] * 255;
	_b = col[2] * 255;
}


//****************************************************

void MyPaletteElementCtrl::ComputeColorTrans( int hue, int sat, int luz )
{
	int _r = r, _g = g, _b = b;
	float _h, _s, _l;
	RGB_to_HSL( _r, _g, _b, _h, _s, _l );

	_h = _h /360;
	_h += (float)hue / 100;
	if( _h <= 0 )
		_h += 1;
	if( _h > 1 )
		_h -= 1;

	_s += (float)sat / 100;
	if( _s < 0 )
		_s = 0;
	if( _s > 1 )
		_s = 1;

	_l += (float)luz / 100;
	if( _l < 0 )
		_l = 0;
	if( _l > 1 )
		_l = 1;

	HSL_to_RGB( r, g, b, _h, _s, _l );
	Refresh();
}


//****************************************************

void MyPaletteElementCtrl::ComputeColorDecal( int red, int green, int blue )
{
	int _r = origin_r + red;
	if( _r < 0 ) _r = 0;
	if( _r > 255 ) _r = 255;
	r = _r;

	int _g = origin_g + green;
	if( _g < 0 ) _g = 0;
	if( _g > 255 ) _g = 255;
	g = _g;

	int _b = origin_b + blue;
	if( _b < 0 ) _b = 0;
	if( _b > 255 ) _b = 255;
	b = _b;
}


//****************************************************

void MyPaletteElementCtrl::GetRGB( unsigned char* _r, unsigned char* _g, unsigned char* _b )
{
	*_r = r;
	*_g = g;
	*_b = b;
}


//****************************************************
wxColour 
MyPaletteElementCtrl::GetColor()
{
	return wxColour( r,g,b );
}

//----------------------------------------------------
void
MyPaletteElementCtrl::SetColor(const wxColor& _color )
{
	SetRGB( _color.Red(), _color.Green(), _color.Blue() );
}

//****************************************************

void MyPaletteElementCtrl::Refresh()
{
	wxClientDC dc(this);
	DoPainting( dc );
}


//****************************************************

void MyPaletteElementCtrl::Update()
{
	Refresh();
}


//****************************************************

void MyPaletteElementCtrl::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	DoPainting( dc );
}


//****************************************************
#define D_PAL_SEL_BORDER_SIZE 3
void MyPaletteElementCtrl::DoPainting( wxDC& dc )
{
	int _x, _y;
	GetPosition(&_x, &_y);
	wxSizer *t_sizer = GetContainingSizer();
	if( t_sizer == NULL )
		return;

	wxSize _t = t_sizer->GetSize();

	wxBrush brush;
	if( IsEnabled() )
	{
		if( selected_state  == 1 )
		{
			dc.SetBrush( *wxCYAN );
			dc.DrawRectangle(   -D_PAL_SEL_BORDER_SIZE
						, -D_PAL_SEL_BORDER_SIZE
						,  PALELT_SIZE + D_PAL_SEL_BORDER_SIZE*2
						,  PALELT_SIZE + D_PAL_SEL_BORDER_SIZE*2);
		}
		else if( selected_state  == 2 )
		{
			dc.SetBrush( *wxRED );
			dc.DrawRectangle(   -D_PAL_SEL_BORDER_SIZE
						, -D_PAL_SEL_BORDER_SIZE
						,  PALELT_SIZE + D_PAL_SEL_BORDER_SIZE*2
						,  PALELT_SIZE + D_PAL_SEL_BORDER_SIZE*2);
		}
		else
		{
			dc.SetBrush( GetBackgroundColour() );
			dc.DrawRectangle( 0, 0, PALELT_SIZE, PALELT_SIZE );
		}

		brush.SetColour( wxColour(r,g,b) );
		dc.SetBrush( brush );
		dc.DrawRectangle( PALELT_COLOR_DECAL, PALELT_COLOR_DECAL,
				PALELT_SIZE - 2* PALELT_COLOR_DECAL, PALELT_SIZE - 2* PALELT_COLOR_DECAL );
	}
	else
	{
		dc.SetBrush( GetBackgroundColour() );
		dc.DrawRectangle( 0, 0, PALELT_SIZE, PALELT_SIZE );
	}

}


//****************************************************

void MyPaletteElementCtrl::EvtClick(wxMouseEvent& event)
{
	event.Skip();
	wxCommandEvent evt(wxPALETTE_ELT_CLICKED);
	evt.SetId( this->GetId());
	evt.SetEventObject(this);

	// In the ClientData put the adding mode
	int add_mode = (event.m_controlDown?1:0)+(event.m_shiftDown?2:0);
	evt.SetClientData( (void*) add_mode );
	ProcessEvent( evt );
}


//****************************************************

void MyPaletteElementCtrl::EvtDoubleClick(wxMouseEvent& event)
{
	event.Skip();
	
	// No colour select in 8bits mode
	if( MyPaletteCtrl::mode8bit && eltmode != AUTO_ELTCOLOR )
		return;

	wxColour res = wxGetColourFromUser(this, wxColour( origin_r,origin_g,origin_b), wxT("Choose the new colour"));

	if( res.IsOk() )
	{
		r = res.Red();
		origin_r = r;
		g = res.Green();
		origin_g = g;
		b = res.Blue();
		origin_b = b;
		if( eltmode != ELTPALETTE )
			Refresh();

		wxCommandEvent evt(wxPALETTE_ELT_COLOR_CHANGE);
		evt.SetEventObject( this );
		evt.SetId( this->GetId() );
		ProcessEvent( evt );
	}
}


//****************************************************

bool MyPaletteElementCtrl::Equal( wxColour& _col )
{
	return ( _col.Red() == r && _col.Green() == g && _col.Blue() == b );
}


//****************************************************

wxSize MyPaletteElementCtrl::GetMinSize() const
{
	return wxSize(PALELT_SIZE+2,PALELT_SIZE+2);
}



//****************************************************
//****************************************************
//****************************************************
//****************************************************
BEGIN_EVENT_TABLE(MyPaletteElementCtrl, wxControl)
	EVT_LEFT_UP(MyPaletteElementCtrl::EvtClick)
	EVT_LEFT_DCLICK(MyPaletteElementCtrl::EvtDoubleClick)
	EVT_PAINT(MyPaletteElementCtrl::OnPaint)
END_EVENT_TABLE()

//****************************************************
//************ PALETTE CONTROL  **************
//****************************************************

bool 	  MyPaletteCtrl::mode8bit = true;

MyPaletteCtrl::MyPaletteCtrl(wxWindow* _parent )
:wxWindow( _parent, wxID_ANY, wxDefaultPosition, wxDefaultSize )
{
	SetAutoLayout(true);
	Constructor();
}

//---------------------------------------------------------------
void 
MyPaletteCtrl::Constructor()
{
	obj_associated = NULL;
	isChanged = false;
	b_init = false;

	hue = sat = luz = 0;
	red = green = blue = 0;

	theSourceImg = NULL;
	thePalette = NULL;
	paletteElements = new MyPaletteElementCtrl*[256];
	for( int i = 0; i < 256; i++ )
	{
		paletteElements[i] = new MyPaletteElementCtrl( this );
		paletteElements[i]->SetId( ID_FIRST_PALETTE_ELT + i );
		paletteElements[i]->Show( false );
	}
	nb_elts = 0;

	mainSizer = new wxBoxSizer( wxVERTICAL );

	gridSizer = new wxGridSizer( 1,1, 3, 3);
	mainSizer->Add( gridSizer, 1, wxEXPAND );

	error_text = new wxStaticText( this, wxID_ANY, wxT("No Palette Loaded") );
	wxSize _t = error_text->GetSize();
	error_text->SetMinSize( _t );
	gridSizer->Add( error_text );

	SetSizer( mainSizer );
//	Layout();
}

//****************************************************

MyPaletteCtrl::~MyPaletteCtrl()
{
	Reset(false);
	delete[] paletteElements;
}


//****************************************************

void MyPaletteCtrl::SetAssociatedObject( ob_object* _obj )
{
	obj_associated = _obj;
}


//****************************************************

bool MyPaletteCtrl::TryToInitWithImage( wxString& strpath )
{
	if( !wxFileName(strpath).FileExists() || IsFileEmpty( strpath ) )
		return false;

	Reset(false);

	// Load the file and init the palette
	if( wxFileName(strpath).GetExt().Upper() == wxT("GIF") )
		theSourceImg = new wxIndexedGIF( strpath );

	else if( wxFileName(strpath).GetExt().Upper() == wxT("PNG") )
		theSourceImg = new wxIndexedPNG( strpath );

	if( !theSourceImg->IsOk() )
	{
		delete theSourceImg;
		theSourceImg = NULL;
		return false;
	}
	MyPalette *_t_pal = thePalette;
	thePalette = theSourceImg->GetPalette();
	if(thePalette == NULL || !thePalette->IsOk() )
	{
		if( thePalette != NULL )
			delete thePalette;
		thePalette = _t_pal;
		return false;
	}
	if( _t_pal != NULL )
		delete _t_pal;

	nb_elts = thePalette->GetColoursCount();
	pr_Make_SrcPalette_To_PaletteElts(true);

	b_init = true;
	GetParent()->Layout();
	Layout();
	Refresh();
	return true;
}


//****************************************************

void MyPaletteCtrl::pr_Make_SrcPalette_To_PaletteElts(bool b_update_sizer )
{
	if( thePalette == NULL || ! thePalette->IsOk() )
		return;

	Freeze();
	if( nb_elts != 0 )
	{
		if( b_update_sizer )
		{
			error_text->Hide();
			gridSizer->Clear();

			int nb_cols = 16, nb_rows = 1;
			if( nb_elts <= 64 )
				nb_rows = 4;
			else if( nb_elts <= 128 )
				nb_rows = 8;
			else if( nb_elts <= 256 )
				nb_cols = 16;

			gridSizer->SetCols(nb_cols);
			gridSizer->SetRows(nb_rows);
		}

		for( int i = 0; i < nb_elts; i++ )
		{
			unsigned char _r,_g,_b;
			thePalette->GetRGB(i, &_r, &_g, &_b);
			paletteElements[i]->SetRGB( _r,_g,_b);
			if( b_update_sizer )
			{
				paletteElements[i]->Show( true );
				gridSizer->Add( paletteElements[i] );
			}
		}
	}
	Thaw();
}

//****************************************************

void MyPaletteCtrl::UndoRemapping()
{
	// Undo all remappings relations
	for( int i = 0; i < nb_elts; i++ )
		paletteElements[i]->mappedTo = -1;

	// Undo the 16Bits colors mappings
	pr_Make_SrcPalette_To_PaletteElts(false);

}

//****************************************************

bool MyPaletteCtrl::InitRemapping_With( wxString& strpath )
{
	if( ! b_init )
		return false;

	if( !wxFileName(strpath).FileExists() || IsFileEmpty( strpath ) )
		return false;

	// Load the file
	imgFile *t_img;
	if( wxFileName(strpath).GetExt().Upper() == wxT("GIF") )
		t_img = new wxIndexedGIF( strpath );

	else if( wxFileName(strpath).GetExt().Upper() == wxT("PNG") )
		t_img = new wxIndexedPNG( strpath );

	if( ! t_img->IsOk() )
	{
		delete t_img;
		return false;
	}

	if( 	theSourceImg->width  != t_img->width
		||	theSourceImg->height != t_img->height )
	{
		delete t_img;
		return false;
	}

//	int nb_pixs = theSourceImg->width * theSourceImg->height;

	// In 8 Bit mode, just have to make index relations
	if( mode8bit )
	{
		for( int i =0; i < theSourceImg->height; i++ )
		{
			for( int j =0; j < theSourceImg->width; j++ )
			{
				// Get the paletteElt corresponding to the current pix
				int src_ind = theSourceImg->GetIndex(i, j );
				int dest_ind = t_img->GetIndex(i, j );
				if( src_ind < 0 )
					continue;
				paletteElements[src_ind]->mappedTo = dest_ind;
			}
		}

		SetAssociations();
	}
	// In 16 bit mod, just save the new rgb values of each index
	else
	{
		wxPalette *_pal = t_img->GetPalette();
		if( _pal == NULL || ! _pal->IsOk() )
		{
			delete t_img;
			if( _pal != NULL )
				delete _pal;
			return false;
		}

		int pal_size = _pal->GetColoursCount();
		unsigned char r,g,b;
		for( int i=0; i < pal_size; i++ )
		{
			_pal->GetRGB( i, &r,&g,&b );
			paletteElements[i]->SetRGB( r,g,b );
		}
		delete _pal;
	}

	delete t_img;
	Refresh();
	return true;
}


//****************************************************

void MyPaletteCtrl::SetAssociations()
{
	if( ! b_init || thePalette == NULL || nb_elts <= 0 )
		return;

	// Make a first pass to remove all associations
	for( int i = 0; i < nb_elts; i ++ )
	{
		if( paletteElements[i]->selected_state == 2 )
			paletteElements[i]->selected_state = 0;
	}

	// No association in 16Bit mode
	if( !mode8bit )
		return;

	for( int i = 0; i < nb_elts; i ++ )
	{
		if( paletteElements[i]->selected_state == 1 && paletteElements[i]->mappedTo >= 0)
		{
			paletteElements[ paletteElements[i]->mappedTo ]->selected_state = 2;
		}
	}
}


//****************************************************

void MyPaletteCtrl::Reset(bool b_layout)
{
	Freeze();
	
	if( thePalette != NULL )
		delete thePalette;
	thePalette = NULL;

	hue = sat = luz = 0;
	red = green = blue = 0;

	gridSizer->Clear();

	for( int i= 0; i < nb_elts; i++)
	{
		paletteElements[i]->Enable(true);
		paletteElements[i]->Show(false);
	}

	if( theSourceImg != NULL)
		delete theSourceImg;
	theSourceImg = NULL;

	error_text->Show();
	gridSizer->SetCols(1);
	gridSizer->SetRows(1);
	gridSizer->Add( error_text );

	isChanged = false;
	b_init = false;

	Thaw();

	if( b_layout )
	{
		GetParent()->Layout();
		Layout();
		Refresh();
	}
}


//****************************************************

wxPalette MyPaletteCtrl::GetNewPalette()
{
	if( nb_elts <= 0 )
		return wxPalette();

	unsigned char r[nb_elts], g[nb_elts], b[nb_elts];

	for( int i = 0; i < nb_elts; i++)
	{
		r[i] = paletteElements[i]->r;
		g[i] = paletteElements[i]->g;
		b[i] = paletteElements[i]->b;
	}
	return wxPalette( nb_elts, r, g, b );
}



//****************************************************

void MyPaletteCtrl::OnPaint(wxPaintEvent& event)
{
	event.Skip();
}


//****************************************************

void MyPaletteCtrl::EvtSize( wxSizeEvent& event )
{
	event.Skip();
}


//****************************************************

void MyPaletteCtrl::EvtPaletteEltClicked( wxCommandEvent& event )
{
	int ind = event.GetId() - ID_FIRST_PALETTE_ELT;
	if( ind >= 0 && ind < nb_elts)
		SelectColour( ind , (size_t) event.GetClientData() );
	else
		wxMessageBox( wxT("Bug !!!\nMyPaletteCtrl::EvtPaletteEltClicked()\nind invalid !") );

}


//****************************************************

void MyPaletteCtrl::EvtPaletteEltColorChange( wxCommandEvent& event )
{
	// Recompute the elt color with modifs
	if( ! mode8bit )
	{
		MyPaletteElementCtrl *pal_elt = (MyPaletteElementCtrl*) event.GetEventObject();
		if( pal_elt != NULL )
		{
			pal_elt->ComputeColorDecal( red, green, blue );
			pal_elt->ComputeColorTrans( hue, sat, luz );
			pal_elt->Refresh();
			isChanged = true;
		}
	}

	//Send the change to the parents
	if( GetParent() != NULL )
	{
		wxCommandEvent evt(wxPALETTE_ELT_CLICKED);
		evt.SetId( this->GetId());
		evt.SetEventObject( this );
		evt.SetEventObject( this );
		evt.SetClientData( (void*) false );
		GetParent()->ProcessEvent( evt );
	}
}


//****************************************************

void MyPaletteCtrl::SelectColour( int ind, int add_mode )
{
	// Check if the palette is ok
	if( ! IsInit() )
		return;

	bool b_send_to_parent = false;
	// Adding a palette element in 16 bits mode
	if( ! mode8bit && add_mode == 1 )
	{
		if( paletteElements[ind]->selected_state != 0 )
		{
			paletteElements[ind]->selected_state = 0;
			paletteElements[ind]->RestoreRGBs();
		}
		else
		{
			paletteElements[ind]->selected_state = 1;
			paletteElements[ind]->ComputeColorTrans( hue, sat, luz );
		}
		paletteElements[ind]->Refresh();
		b_send_to_parent = true;
	}

	else if( ! mode8bit && add_mode == 2 )
	{
		paletteElements[ind]->selected_state = 1;
		paletteElements[ind]->ComputeColorTrans( hue, sat, luz );
		paletteElements[ind]->Refresh();
		b_send_to_parent = true;
	}
	
	// Choosing an new mapping for the current element selected
	else if( mode8bit && add_mode > 0 )
	{
		if( curr_selected == NULL )
			return;

		isChanged = true;
		
		// Undo Current mapping
		if( curr_selected->mappedTo == ind )
		{
			paletteElements[curr_selected->mappedTo]->selected_state = 0;
			paletteElements[curr_selected->mappedTo ]->Refresh();
			curr_selected->mappedTo = -1;
			b_send_to_parent = true;
		}
		else
		{
			// Remove selected state of the previous mapping
			if( curr_selected->mappedTo >= 0 )
			{
				paletteElements[curr_selected->mappedTo]->selected_state = 0;
				paletteElements[curr_selected->mappedTo ]->Refresh();
			}

			// Avoid the error of auto-mapped element
			if( paletteElements[ind] != curr_selected )
			{
				// Adding the new mapping
				curr_selected->mappedTo = ind;
				paletteElements[curr_selected->mappedTo]->selected_state = 2;
				paletteElements[curr_selected->mappedTo]->Refresh();
				b_send_to_parent = true;
			}
			// Auto-map is obvously forbidden
			else
				isChanged = false;
		}
	}

	// Simple selection in 16 Bits mode
	else if( !mode8bit )
	{
		int last_sel_state = paletteElements[ind]->selected_state;
		DeselectEveryThing();
		if( last_sel_state != 0 )
		{
			paletteElements[ind]->selected_state = 0;
			paletteElements[ind]->RestoreRGBs();
		}
		else
		{
			paletteElements[ind]->selected_state = 1;
			paletteElements[ind]->ComputeColorTrans( hue, sat, luz );
		}
		b_send_to_parent = true;
	}

	// Simple selection in 8 Bits mode
	else
	{
		b_send_to_parent = true;
		DeselectEveryThing();
		if( paletteElements[ind]->selected_state > 0 )
		{
			paletteElements[ind]->selected_state =  0;
			paletteElements[ind]->Refresh();
			if( paletteElements[ind]->mappedTo >= 0 )
			{
				paletteElements[paletteElements[ind]->mappedTo ]->selected_state = 0;
				paletteElements[paletteElements[ind]->mappedTo ]->Refresh();
			}
		}
		else
		{
			paletteElements[ind]->selected_state =  1;
			paletteElements[ind]->Refresh();
			if( paletteElements[ind]->mappedTo >= 0 && paletteElements[ind]->mappedTo != ind )
			{
				paletteElements[paletteElements[ind]->mappedTo ]->selected_state = 2;
				paletteElements[paletteElements[ind]->mappedTo ]->Refresh();
			}
		}
	}

	// Update the current selected element
	if( paletteElements[ind]->selected_state == 1 && mode8bit && add_mode == 0 )
		curr_selected = paletteElements[ind];

	//Send the change to the parent
	if( b_send_to_parent == true && GetParent() != NULL )
	{
		wxCommandEvent event;
		event.SetEventType( wxPALETTE_ELT_CLICKED );
		event.SetEventObject( this );
		event.SetId( this->GetId() );
		GetParent()->ProcessEvent( event );
	}

}


//****************************************************

void MyPaletteCtrl::Refresh()
{
//	GetParent()->Layout();
//	Layout();
	SetSize( GetSize());

	wxWindow::Refresh();
}


//****************************************************

bool MyPaletteCtrl::Layout()
{
	if(  GetParent() != NULL )
		GetParent()->Layout();
	return wxWindow::Layout();
}


//****************************************************

void MyPaletteCtrl::DeselectEveryThing()
{
	for( int i = 0; i < nb_elts; i ++ )
	{
		if( paletteElements[i]->selected_state > 0 )
		{
			paletteElements[i]->selected_state = 0;
			if( !mode8bit )
				paletteElements[i]->RestoreRGBs();
			paletteElements[i]->Refresh();
		}
	}
}


//****************************************************

wxString MyPaletteCtrl::FullPath()const
{
	if( obj_associated == NULL )
		return wxString();

	return GetObFile( OBPath() ).GetFullPath();
}

//****************************************************

wxString MyPaletteCtrl::OBPath()const
{
	if( obj_associated == NULL )
		return wxString();

	return obj_associated->GetToken(2);
}


//****************************************************

bool MyPaletteCtrl::IsInit()const
{
	return nb_elts > 0;
}

//****************************************************

void MyPaletteCtrl::Reset_HSL()
{
	hue = sat = luz = 0;
	red = green = blue = 0;
}

//****************************************************

void MyPaletteCtrl::Set_Hue( int _hue )
{
	hue = _hue;
	Apply_HSL();
	isChanged = true;
}

//****************************************************

void MyPaletteCtrl::Set_Sat( int _sat )
{
	sat = _sat;
	Apply_HSL();
	isChanged = true;
}

//****************************************************

void MyPaletteCtrl::Set_Luz( int _luz )
{
	luz = _luz;
	Apply_HSL();
	isChanged = true;
}

//****************************************************

void MyPaletteCtrl::Set_Red( int _red )
{
	red = _red;
	Apply_HSL();
	isChanged = true;
}


//****************************************************

void MyPaletteCtrl::Set_Green( int _green )
{
	green = _green;
	Apply_HSL();
	isChanged = true;
}


//****************************************************

void MyPaletteCtrl::Set_Blue( int _blue )
{
	blue = _blue;
	Apply_HSL();
	isChanged = true;
}


//****************************************************

void MyPaletteCtrl::Apply_HSL()
{
	for( int i = 0; i < nb_elts; i ++ )
	{
		if( paletteElements[i]->selected_state > 0 )
		{
			paletteElements[i]->ComputeColorDecal( red, green, blue );
			paletteElements[i]->ComputeColorTrans( hue, sat, luz );
			paletteElements[i]->Refresh();
		}
	}
}

//****************************************************

void MyPaletteCtrl::ValidateHSLs()
{
	for( int i = 0; i < nb_elts; i ++ )
	{
		if( paletteElements[i]->selected_state > 0 )
		{
			paletteElements[i]->CommitNewColours();
			paletteElements[i]->Refresh();
		}
	}
	Reset_HSL();
}

//****************************************************

void MyPaletteCtrl::InValidateHSLs()
{
	for( int i = 0; i < nb_elts; i ++ )
	{
		if( paletteElements[i]->selected_state > 0 )
		{
			paletteElements[i]->RestoreRGBs();
			paletteElements[i]->Refresh();
		}
	}
	Reset_HSL();
}

//************************************************************************************

void MyPaletteCtrl::DoGuessColors()
{
	// Check if the src image is ok
	if( ! b_init )
		return;

	for( int i = 0; i < theSourceImg->height; i++ )
	{
		for( int j= 0; j < theSourceImg->width; j++ )
		{
			int pal_ind = theSourceImg->GetIndex( i, j );
			if( pal_ind >= nb_elts || pal_ind <= 0 )
				continue;

			if( paletteElements[pal_ind]->selected_state == 0 )
				paletteElements[pal_ind]->selected_state = 1;
		}
	}
	Apply_HSL();
}


//****************************************************

void MyPaletteCtrl::DoFixSelectedColors()
{
	Freeze();
	// Get the highest line of selected Element
	int highest_line = 0;
	for( int i = nb_elts-1; i >= 0; i-- )
	{
		if( paletteElements[i]->selected_state > 0 )
		{
			highest_line = i /8 + 1;
			break;
		}
	}

	// Remove all unnecessary lines
	for( int i = nb_elts - 1; i >= highest_line * 8; i--)
	{
		gridSizer->Detach( paletteElements[i] );
		paletteElements[i]->Show( false);
	}

	// Disable all unselected remainings
	for( int i = 0; i < nb_elts; i ++ )
	{
		if( paletteElements[i]->selected_state == 0 )
		{
			paletteElements[i]->Enable( false);
		}
	}
	Thaw();
	Refresh();
	GetParent()->Layout();
}


//****************************************************

void MyPaletteCtrl::UnDoFixSelectedColors()
{
	Freeze();
	
	// Get the highest line of selected Element
	int highest_line = 0;
	for( int i = nb_elts-1; i >= 0; i-- )
	{
		if( paletteElements[i]->selected_state > 0 )
		{
			highest_line = i /8 + 1;
			break;
		}
	}

	// ReAdd all previously unnecessary lines
	for( int i = highest_line * 8; i < nb_elts; i++)
	{
		paletteElements[i]->Show( true);
		gridSizer->Add( paletteElements[i], 1, wxEXPAND );
	}

	// Reenable all unselected remainings
	for( int i = 0; i < nb_elts; i ++ )
		paletteElements[i]->Enable( true );

	Thaw();
	Refresh();
	GetParent()->Layout();
}


//****************************************************
//****************************************************
BEGIN_EVENT_TABLE(MyPaletteCtrl, wxWindow)
	PALETTE_ELT_CLICKED(MyPaletteCtrl::EvtPaletteEltClicked)
	PALETTE_ELT_COLOR_CHANGE(MyPaletteCtrl::EvtPaletteEltColorChange)
	EVT_SIZE(MyPaletteCtrl::EvtSize)
	EVT_PAINT(MyPaletteCtrl::OnPaint)
END_EVENT_TABLE()

