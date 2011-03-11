#ifndef WXFRAMEIMAGE_H_
#define WXFRAMEIMAGE_H_

#include <wx/control.h>


class MyImageCtrl : public wxControl
{
public:
	MyImageCtrl(wxWindow *parent);
	virtual ~MyImageCtrl();

	void OnEraseBackground(wxEraseEvent& WXUNUSED(event));
	virtual void OnPaint(wxPaintEvent& WXUNUSED(event));
	virtual void EvtSize(wxSizeEvent& evt );
	void EvtContextMenu( wxContextMenuEvent& event );

	virtual void mySetSize(int& width, int& height);
	virtual void SetSize(int x, int y, int width, int height, int sizeFlags = wxSIZE_AUTO);
	virtual void SetSize(int width, int height);
	virtual void SetSize(const wxRect& rect);
	virtual void SetSize(const wxSize& size);

	void ScaleImage(double factor );
	void Rotate90(int nb_crans );
	void Set_Mirroring( bool b_mirroring = true );

	void SetImage( wxImage* );

	void Set_BoundDimensions( int max_x, int max_y = wxDefaultCoord, int min_x = wxDefaultCoord, int min_y = wxDefaultCoord );

	int Height()const;
	int Width()const;
	int X()const;
	int Y()const;

protected:
	virtual wxSize DoGetBestSize() const;
	float Get_Scale_Factor( int _x, int _y )const;
	void  ReboundSize( int& _x, int& _y )const;
	void  Get_TransformedImgSize(int& _w, int& _h )const;

	void EvtMouseSomething(wxMouseEvent& event);

	// The bitmap image is not owned by MyImageCtrl
	wxImage* p_image;
	mutable wxBitmap *theBitmap;

	int max_x, max_y, min_x, min_y;
	int nb_rotation_90;
	int b_mirror;

	DECLARE_EVENT_TABLE()
};



#endif /*WXFRAMEIMAGE_H_*/
