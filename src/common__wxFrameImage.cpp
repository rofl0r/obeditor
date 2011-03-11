#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dcbuffer.h>
#include "common__tools.h"
#include "common__wxFrameImage.h"

//****************************************************
//***********VARIABLES DECL***********************

// in the implementation
DEFINE_EVENT_TYPE(wxEVT_IMGFRAME_CLIKED)

//----------------------------------------------------------------------
MyImageCtrl::MyImageCtrl(wxWindow *parent)
: wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize
		, wxFULL_REPAINT_ON_RESIZE | wxBORDER_STATIC )
{
	p_image = noImg;
	theBitmap = NULL;
	min_x = 15;
	min_y = 15;
	max_x = 1500;
	max_y = 1000;
	b_mirror = false;
	nb_rotation_90 = 0;
}

//----------------------------------------------------------------------
MyImageCtrl::~MyImageCtrl()
{
	if( theBitmap != NULL )
		delete theBitmap;
	theBitmap = NULL;
}

//----------------------------------------------------------------------
void 
MyImageCtrl::OnEraseBackground(wxEraseEvent& event)
{
	// do nothing here to be able to see how transparent images are shown
	event.Skip();
}

//------------------------------------------------------------------------
void MyImageCtrl::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc( this );
	if( theBitmap != NULL )
		dc.DrawBitmap( *theBitmap, 0, 0, true /* use mask */ );
}

//------------------------------------------------------------------------
void 
MyImageCtrl::EvtSize(wxSizeEvent& evt )
{
	wxSize _s = evt.GetSize();
	wxSize min_s( GetMinSize() );
	if( _s.x < min_s.x )
		_s.x = min_s.x;
	if( _s.y < min_s.y )
		_s.y = min_s.y;
	mySetSize( _s.x, _s.y );

	if( theBitmap == NULL )
		ReboundSize( _s.x, _s.y );
	
	else if( (theBitmap->GetWidth() < _s.x - 3 && theBitmap->GetHeight() < _s.y - 3)
		||
		(theBitmap->GetWidth() > _s.x + 3 && theBitmap->GetHeight() < _s.y + 3 )
		)
		ReboundSize( _s.x, _s.y );
}


//******************************************************

int MyImageCtrl::Height()const
{
	int _w, _h;
	GetSize(&_w,&_h);
	return _h;
}

int MyImageCtrl::Width()const
{
	int _w, _h;
	GetSize(&_w,&_h);
	return _w;
}

int MyImageCtrl::X()const
{
	int mx,my;
	GetPosition(&mx,&my);
	return mx;
}

int MyImageCtrl::Y()const
{
	int mx,my;
	GetPosition(&mx,&my);
	return my;
}

//******************************************************
void
MyImageCtrl::Get_TransformedImgSize( int& _w, int& _h )const
{
	_w = p_image->GetWidth();
	_h = p_image->GetHeight();
	if( nb_rotation_90 %2 != 0 )
	{
		int t = _w;
		_w = _h;
		_h = t;
	}
}


//******************************************************

void MyImageCtrl::SetImage( wxImage* _newImg)
{
	p_image = _newImg;
	if( theBitmap != NULL )
		delete theBitmap;
	theBitmap = NULL;
	
	int _width,_height;
	Get_TransformedImgSize(_width,_height);
	mySetSize( _width, _height );

	wxSizer *t = GetContainingSizer();
	if( t != NULL )
		t->RecalcSizes();
	
	Refresh();
}

//******************************************************
void 
MyImageCtrl::ScaleImage(double factor )
{
	if( factor <= 0 )
		return;

	int prev_w, prev_h;
	GetSize( &prev_w, &prev_h);
	int w = (int) (prev_w *factor);
	int h = (int) (prev_h *factor);
	SetSize( w, h );
}

//-----------------------------------------------------
void 
MyImageCtrl::Rotate90(int nb_crans)
{
	nb_rotation_90 = (nb_crans%4);
	if( nb_rotation_90 < 0 )
		nb_rotation_90 += 4;
	
	Refresh();
}

//******************************************************
void 
MyImageCtrl::Set_Mirroring( bool b_mirroring )
{
	b_mirror = b_mirroring;
	Refresh();
}

//******************************************************

void MyImageCtrl::SetSize(int _x, int _y, int _width, int _height, int _sizeFlags )
{
	ReboundSize( _width, _height );
	wxControl::SetSize(_x,_y,_width, _height,_sizeFlags);
}

void MyImageCtrl::SetSize(const wxRect& _rect)
{
	return SetSize(_rect.x, _rect.y,  _rect.width, _rect.height );
}

void MyImageCtrl::SetSize(const wxSize& _size)
{
	return SetSize( _size.GetWidth(), _size.GetHeight() );
}

void MyImageCtrl::SetSize(int _width, int _height)
{
	ReboundSize( _width, _height );
	wxControl::SetSize( _width, _height );
}

void MyImageCtrl::mySetSize(int& _width, int& _height)
{
	ReboundSize( _width, _height );
	wxControl::SetSize( _width, _height );
	int _w, _h;
	GetSize(&_w,&_h);
}

//******************************************************
wxSize 
MyImageCtrl::DoGetBestSize() const
{
	wxSize min_s( GetMinSize() );

	if( p_image != NULL)
	{
		int prev_w,prev_h;
		Get_TransformedImgSize( prev_w, prev_h );
		
		float scale_factor = Get_Scale_Factor( prev_w, prev_h );
		wxSize calc_s(prev_w * scale_factor, prev_h * scale_factor);
		if( min_s.x > calc_s.x )
			calc_s.x = min_s.x;
		if( min_s.y > calc_s.y )
			calc_s.y = min_s.y;
		return calc_s;
	}
	wxSize s(wxControl::DoGetBestSize());
	if( min_s.x > s.x )
		s.x = min_s.x;
	if( min_s.y > s.y )
		s.y = min_s.y;
	return s;
}

//******************************************************
void 
MyImageCtrl::Set_BoundDimensions( int _max_x, int _max_y, int _min_x, int _min_y )
{
	max_x = _max_x;
	max_y = _max_y;
	SetMaxSize( wxSize(max_x, max_y) );
	min_x = _min_x;
	min_y = _min_y;
	SetMinSize( wxSize(min_x, min_y) );
}

//--------------------------------------------------------------
float
MyImageCtrl::Get_Scale_Factor( int _x, int _y )const
{
	double scale_factor = 1;
	
	// Min first -> max are stronger constraint
	if( _x < min_x && min_x != wxDefaultCoord )
		scale_factor = ( (double)min_x / (double)_x  );
	if( _y < min_y && min_y != wxDefaultCoord )
		scale_factor = ( ( (double)min_y / (double)_y ) > scale_factor ) ? ( (double)min_y / (double)_y ) : scale_factor;

	_x = (int) (scale_factor * (double)_x );
	_y = (int) (scale_factor * (double)_y );

	scale_factor = 1;
	if( _x > max_x && max_x != wxDefaultCoord )
		scale_factor = ((double)max_x / (double)_x );
	if( _y > max_y && max_y != wxDefaultCoord )
		scale_factor = ( ((double)max_y / (double)_y ) < scale_factor ) ? ((double)max_y / (double)_y ) : scale_factor;

	_x = (int) (scale_factor * (double)_x );
	_y = (int) (scale_factor * (double)_y );
	
	int _w,_h;
	Get_TransformedImgSize( _w, _h );
	double scale_x = (double)_x / (double) _w;
	double scale_y = (double)_y / (double) _h;
	
	scale_factor = (scale_x > scale_y) ? scale_y : scale_x;
	return scale_factor;
}

//--------------------------------------------------------------
void 
MyImageCtrl::ReboundSize( int& _x, int& _y )const
{
	if( _x <= 0 || _y <= 0)
		return;

	if( p_image == NULL )
		return;
	
	int _w,_h;
	Get_TransformedImgSize( _w, _h );
	
	double scale_factor = Get_Scale_Factor( _x, _y );
	
	_x = (int) (scale_factor * (double)_w );
	_y = (int) (scale_factor * (double)_h );

	wxImage t_img ;
	switch( nb_rotation_90 )
	{
		case 1:
			t_img = p_image->Rotate90( false );
			break;
		case 3:
			t_img = p_image->Rotate90( true );
			break;
		case 2:
			t_img = p_image->Rotate90( true );
			t_img = p_image->Rotate90( true );
			break;
		default:
			t_img = *p_image;
	}
	if( b_mirror == true )
		t_img = t_img.Mirror();
	
	// Scale the image
	if( theBitmap != NULL )
		delete theBitmap;
	
	t_img = t_img.Scale(_x, _y );

	theBitmap = new wxBitmap(t_img);
}

void MyImageCtrl::EvtMouseSomething(wxMouseEvent& event)
{
//	wxMessageBox( "Frame Clicked !", "Debug", wxOK | wxICON_INFORMATION );

	// Map the coord of the event for the parent
	event.m_x += X();
	event.m_y += Y();
	wxWindow *parent = GetParent();
	if( parent != NULL )
   		wxPostEvent(parent, event);
   	event.Skip();
}


void MyImageCtrl::EvtContextMenu( wxContextMenuEvent& event )
{
//	wxPostEvent( GetParent(), event );
	event.Skip();
}

BEGIN_EVENT_TABLE(MyImageCtrl, wxControl)
	EVT_ERASE_BACKGROUND(MyImageCtrl::OnEraseBackground)
	EVT_PAINT(MyImageCtrl::OnPaint)
	EVT_SIZE(MyImageCtrl::EvtSize)
	EVT_MOUSE_EVENTS(MyImageCtrl::EvtMouseSomething)
	EVT_CONTEXT_MENU(MyImageCtrl::EvtContextMenu)
END_EVENT_TABLE()


