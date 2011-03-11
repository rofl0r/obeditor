#ifndef LEVELS__OBIMGS_MANAGER_H
#define LEVELS__OBIMGS_MANAGER_H

#include <wx/bitmap.h>
#include <wx/image.h>

#include <map>

#include "images__imgFile.h"
#include "common__object_stage.h"
#include "levels__globals.h"


//*****************************************************************
//-----------------------------------------------------------------
enum
{
	MASK_NONE = 0,
	MASK_PALETTE = 1,
	MASK_IN_IMAGE = 2,
};

class Img_Collection
{
public:
	static wxColour color_sel_ON;
	static wxColour color_sel_HIGHLIGHT;
protected:
	typedef struct
	{
		wxImage*  img;
		float     zoom_factor;
		wxBitmap* bmp;
	}Img_state;
		
public:
	Img_Collection(  ob_object* _obj
			   , wxImage* _src
			   , const wxString& path, int _mask_type = MASK_PALETTE );
	virtual ~Img_Collection();
	void Reset_ALL();
	void Reset_States();
	wxImage*  Get_Image(  	size_t img_state_ID, float zoom_factor );
	bool      IsImageExists(size_t img_state_ID, float zoom_factor );
	wxBitmap* Get_Bitmap( wxImage*& pimg, size_t img_state_ID, float zoom_factor );

protected:
	wxImage* Get_Remap( size_t img_state_ID );
	void 	 RecomputeImageState(Img_state* imgState
					  ,size_t img_state_ID,float zoom_factor );
	void RecolorSelectZone( wxImage*& img, size_t img_state_ID );

	Img_Collection* GetCollection(ob_object* obj
					, const wxString& imgPath
					, bool b_quiet = false
					);
	wxImage*	    BuilSelectedImage( wxImage* src
						   , wxColour& selection_colour
						   , size_t border_size
						   );

public:
	ob_object* obj;
	wxImage*  src;
	wxColour  selection_colour;   // The colour which have to be 
						// replace by the selection state color

protected:
	wxImage*  src_selected;
	wxImage*  src_selected_mini;
	imgFile*  src2;
	int mask_type;
	unsigned char mask_r, mask_g, mask_b;
	std::map<size_t,Img_state*> imgs;
	
	std::map<size_t,wxImage*> remaps_src;
};

//*****************************************************************
/**
	 A class to avoid idiot copies and unnecessary processings
*/
class ObImgs_Manager
{
public:
	static wxColour def_mask_colour;
public:
	ObImgs_Manager();
	~ObImgs_Manager();

	void	Reset();				// Clear ENTIRELY the image Data Base
	void 	Invalidate_ImagesStates();	// Clear Only the Computed images
	
	wxImage* GetImage(
			  ob_object* obj
			, const wxString& imgPath
			, float zoom_factor
			, int remap = 0
			, bool b_reversed = false
			, int selected_state = SEL_OFF
			, bool b_miniature = false
			, bool b_quiet = false
			);
	wxBitmap* GetBitmap(
			  wxImage*& pimg
			, ob_object* obj
			, const wxString& imgPath
			, float zoom_factor
			, int remap = 0
			, bool b_reversed = false
			, int selected_state = SEL_OFF
			, bool b_miniature = false
			, bool b_quiet = false
			);

	wxImage* GetImageSrc( ob_object* obj,const wxString& imgPath,bool b_quiet=false );
	bool     IsImageExists( const wxString& key );
			
	void ReleaseImage( const wxString& _key );
	bool AbsorbeImage( const wxString& _key, ob_object* obj
				, wxImage* theSrcImage, int mask_type = MASK_IN_IMAGE );
				
	
protected:
	Img_Collection* GetCollection(ob_object* obj, const wxString& imgPath,bool b_quiet = false );
	
	// the DB of images
	std::map<std::string,Img_Collection*> theImgsMap;
};

extern ObImgs_Manager* obImgs_Mgr;


/**
	 A Function to template AT/JOIN/WAIT/etc... images constructions
*/

wxImage* 
Get_Image_StageCursor(	const wxSize& view_size ,
				float  zoom_factor,
				int    select_state,
				int	 vert_truncate_up,
				int	 vert_truncate_down,
				int	 horiz_width_up,
				int	 horiz_width_down,
				const wxColour& color,
				const wxString& label
				);


/**
	 A Function to template line images constructions
*/

wxImage* 
Get_Image_Rect(		wxString& key_res,
				int ID_line,
				const wxSize& view_size ,
				float  zoom_factor,
				int    select_state,
				int	 width,
				int	 height,
				const wxColour& color
				);

#endif //LEVELS__OBIMGS_MANAGER_H