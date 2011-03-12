#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dcmemory.h>
#include <set>

#include "common__mod.h"
#include "common__ob_controls.h"

#include "ob_editor.h"
#include "common__object_stage.h"
#include "levels__globals.h"
#include "levels__classes.h"
#include "levels__sidesWindows.h"
#include "levels__ObImgs_Manager.h"

using namespace std;
#define D_PEN_LINE_WIDTH 1
#define D_PEN_CURS_WIDTH 4

//-------------------------------------------------------
extern LevelEditorFrame *leFrame;
ObImgs_Manager* obImgs_Mgr;
//-------------------------------------------------------

//*****************************************************************
// Img states stuffs
size_t IMGSTT_MASK_REMAP     = 0xFF;
size_t IMGSTT_MASK_REVERSE   = 0;
size_t IMGSTT_MASK_MINIATURE = 0;
size_t IMGSTT_MASK_SEL_START = 0;
size_t IMGSTT_MASK_SELS      = 0;

//-------------------------------------------------------
inline
void
ANN_Init_ImgStates_Masks()
{
	// Init the masks
	if( IMGSTT_MASK_REVERSE == 0 )
	{
		IMGSTT_MASK_REVERSE   = 1 << 8;
		IMGSTT_MASK_MINIATURE = 1 << 9;
		size_t s_mask  = 1 << (sizeof(size_t) * 8 - 1);
		IMGSTT_MASK_SEL_START = s_mask;
		for( size_t i = 0; i < SEL_MAX; i++ )
		{
			IMGSTT_MASK_SELS |= s_mask;
			s_mask = s_mask >> 1;
		}
		
	}
}

//-------------------------------------------------------
inline
size_t
Get_Remap_Num( size_t& img_state_ID )
{
	return (img_state_ID & IMGSTT_MASK_REMAP);
}

//-------------------------------------------------------
inline
bool
Is_Reversed( size_t& img_state_ID )
{
	return (img_state_ID & IMGSTT_MASK_REVERSE) != 0;
}

//-------------------------------------------------------
inline
bool
Is_Miniature( size_t& img_state_ID )
{
	return (img_state_ID & IMGSTT_MASK_MINIATURE) != 0;
}


//-------------------------------------------------------
inline
size_t
Get_Selection_ID( size_t& img_state_ID )
{
	size_t selstate = IMGSTT_MASK_SELS & img_state_ID;
	size_t s_mask = IMGSTT_MASK_SEL_START;
	for( int i = 0; i < SEL_MAX; i++)
	{
		if( s_mask & selstate )
			return i+1;
		s_mask = s_mask >> 1;
	}
	return 0;
}

//*****************************************************************
//*****************************************************************
wxColour Img_Collection::color_sel_ON(255,0,0);
wxColour Img_Collection::color_sel_HIGHLIGHT(0,0,255);
//-------------------------------------------------------
Img_Collection::Img_Collection(ob_object* _obj, wxImage* _src
			, const wxString& path, int _mask_type )
{
	mask_type = _mask_type;
	obj = _obj;
	src = _src;
	if( mask_type == MASK_PALETTE )
		src2 = imgFile::LoadImage( path );
	else 
		src2 = NULL;

	if( _mask_type != 0 && src != NULL && src->IsOk() )
	{
		if( _mask_type == MASK_PALETTE && src->GetPalette().IsOk() )
			src->GetPalette().GetRGB( 0, &mask_r,&mask_g,&mask_b);
		else if( _mask_type == MASK_IN_IMAGE )
			src->GetOrFindMaskColour( &mask_r,&mask_g,&mask_b );
	}
	else
	{
		mask_r = 10;
		mask_g = 10;
		mask_b = 10;
	}

	src_selected = BuilSelectedImage(src,selection_colour, 1);
	if( src_selected != NULL && src_selected->IsOk() == false )
	{
		delete src_selected;
		src_selected = NULL;
	}
	
	src_selected_mini = BuilSelectedImage(src,selection_colour, 5 );
	if( src_selected_mini != NULL && src_selected_mini->IsOk() == false )
	{
		delete src_selected_mini;
		src_selected_mini = NULL;
	}
}

//-------------------------------------------------------
Img_Collection::~Img_Collection()
{
	Reset_ALL();
	if( src != NULL )
		delete src;
	if( src2 != NULL )
		delete src2;

	// Remove Selected Images
	if( src_selected != NULL )
	{
		delete src_selected;
		src_selected = NULL;
	}

	// Remove Selected Images
	if( src_selected_mini != NULL )
	{
		delete src_selected_mini;
		src_selected_mini = NULL;
	}
}

//-------------------------------------------------------
void
Img_Collection::Reset_ALL()
{
	// Delete states
	Reset_States();
	
	// Remove Remaps
	map<size_t,wxImage*>::iterator it;
	while( remaps_src.empty() == false )
	{
		it = remaps_src.begin();
		if( it->second != NULL )
			delete it->second;
		remaps_src.erase(it);
	}
}

//-------------------------------------------------------
void
Img_Collection::Reset_States()
{
	map<size_t,Img_state*>::iterator it;
	
	while( imgs.empty() == false )
	{
		it = imgs.begin();
		if( it->second->img != NULL )
			delete it->second->img;
		if( it->second->bmp != NULL )
			delete it->second->bmp;
		delete it->second;
		imgs.erase(it);
	}
}

//-------------------------------------------------------
wxImage* 
Img_Collection::Get_Image( size_t img_state_ID, float zoom_factor )
{
	if( src == NULL || src->IsOk() == false )
	{
		if( src != NULL )
			delete src;
		src = new wxImage(*noImg);
	}
	
	map<size_t,Img_state*>::iterator it(imgs.find(img_state_ID));
	
	Img_state* imgState = NULL;
	if( it != imgs.end() )
		imgState = it->second;
	
	else // No such state in the base
	{
		imgState = new Img_state;
		imgState->zoom_factor = 0;
		imgState->img = NULL;
		imgState->bmp = NULL;
		imgs[img_state_ID] = imgState;
	}
	
	if( imgState->zoom_factor != zoom_factor || imgState->img == NULL )
		RecomputeImageState( imgState, img_state_ID, zoom_factor );
	
	return  imgState->img;
}

//-------------------------------------------------------
bool
Img_Collection::IsImageExists(  size_t img_state_ID, float zoom_factor )
{
	if( src == NULL || src->IsOk() == false )
		return false;

	map<size_t,Img_state*>::iterator it(imgs.find(img_state_ID));
	if( it == imgs.end() )
		return false;
	Img_state* imgState = it->second;
	
	if( imgState->zoom_factor != zoom_factor || imgState->img == NULL )
		return false;
	return true;
}

//-------------------------------------------------------
wxBitmap* 
Img_Collection::Get_Bitmap(wxImage*& pimg, size_t img_state_ID, float zoom_factor )
{
	pimg = Get_Image( img_state_ID, zoom_factor );
	if( pimg == NULL )
		return NULL;
	
	map<size_t,Img_state*>::iterator it(imgs.find(img_state_ID));
	if( it == imgs.end() )
	{
		pimg = NULL;
		return NULL;
	}
	Img_state* imgState = it->second;
	
	if( imgState->bmp == NULL )
		imgState->bmp = new wxBitmap(*pimg);
	if( imgState->bmp->IsOk() == false )
	{
		cerr<<"BUG !! : Img_Collection::Get_Bitmap() : imgState->bmp->IsOk() == false"<<endl;
		imgState->bmp = NULL;
		return NULL;
	}
	return imgState->bmp;
}

//-------------------------------------------------------
inline
void
Img_Collection::RecolorSelectZone( wxImage*& img, size_t img_state_ID )
{
	size_t ind_sel = Get_Selection_ID(img_state_ID);
	wxColour o(selection_colour);
	wxColour c(color_sel_HIGHLIGHT);
	if( ind_sel == SEL_ON )
		c = color_sel_ON;
	img->Replace( o.Red(),o.Green(),o.Blue()
			, c.Red(),c.Green(),c.Blue() );
}

//-------------------------------------------------------

inline
unsigned long int
Ann__PixsToLong( unsigned char* pixs )
{
	unsigned long int res = pixs[0];
	res = res << 8;
	res += pixs[1];
	res = res << 8;
	res += pixs[2];
	return res;
}

#define RET_IMG_DATAS__GET_REMAP__ERROR \
	{	\
		remaps_src[img_state_ID] = NULL;	\
		return NULL;				\
	}

wxImage*
Img_Collection::Get_Remap( size_t img_state_ID )
{
	if( src == NULL )
		return NULL;
	if( src->IsOk() == false )
	{
		delete src;
		src = NULL;
		return NULL;
	}
	if( src2 == NULL )
		return NULL;
	if( src2->IsOk() == false )
	{
		delete src2;
		src2 = NULL;
		return NULL;
	}
	if( obj == NULL )
		return NULL;
	
	//------------------------------------------------------
	// Check if the remap have already been computed
	map<size_t,wxImage*>::iterator it( remaps_src.find( img_state_ID) );
	if( it != remaps_src.end() )
		return remaps_src[img_state_ID];
	
	//------------------------------------------------------
	// Check if it's a remapped object type
	bool b_cont = false;
	switch( obj->type )
	{
		case OB_TYPE_SPAWN:
		case OB_TYPE_SPAWN_HEALTHY:
		case OB_TYPE_SPAWN_ITEM:
			b_cont = true;
	}
	if( b_cont == false )
		return NULL;
	
	//------------------------------------------------------
	// Get the associated entity
	obFileEntity* entity_ref = ((ob_spawn*) obj)->entity_ref;
	if( entity_ref == NULL )
		return NULL;
	
	//------------------------------------------------------
	// Get the wanted remap num
	size_t num_remap = Get_Remap_Num(img_state_ID);
	
	//------------------------------------------------------
	// Get the remaps objects
	size_t nb_remaps_8b,nb_remaps_16b;
	ob_object** _remaps_8b = 
	entity_ref->obj_container->GetSubObjectS(wxT("remap"), nb_remaps_8b );
	ob_object** _remaps_16b = 
	entity_ref->obj_container->GetSubObjectS(wxT("alternatepal"),nb_remaps_16b );

	//------------------------------------------------------
	// Get the wanted remap
	bool b_8bits = (nb_remaps_8b > 0);
	size_t nb_remaps;
	ob_object* oRemap = NULL;
	if( nb_remaps_8b > 0 )
	{
		nb_remaps = nb_remaps_8b;
		if( nb_remaps > num_remap)
			oRemap = _remaps_8b[num_remap-1];
	}
	else
	{
		nb_remaps = nb_remaps_16b;
		if( nb_remaps > num_remap)
			oRemap = _remaps_16b[num_remap-1];
	}
	
	if( _remaps_8b != NULL )
		delete[] _remaps_8b;
	if( _remaps_16b != NULL )
		delete[] _remaps_16b;

	
	//------------------------------------------------------
	// Check the num_remap queried is avalaible for the entity
	if( oRemap == NULL )
	{	RET_IMG_DATAS__GET_REMAP__ERROR	}
	
	//------------------------------------------------------
	// Check wich img_src to take
	wxImage* _src = src;
	if( Get_Selection_ID( img_state_ID ) > 0 )
	{
		_src = src_selected;
		if( Is_Miniature( img_state_ID ) == true )
			_src = src_selected_mini;
	}

	if( _src == NULL )
		return NULL;
	
	//**********************************************************
	// 8 BIT remap
	wxImage* remappedImg = NULL;
	if( b_8bits == true )
	{
		wxString s_remap_src  = GetObFile(oRemap->GetToken(0) ).GetFullPath();
		imgFile* remap_src = imgFile::LoadImage( s_remap_src );
		if( remap_src->IsOk() == false )
		{
			delete remap_src;
			remaps_src[img_state_ID] = NULL;
			return NULL;
		}
		
		wxString s_remap_dest = GetObFile(oRemap->GetToken(1) ).GetFullPath();
		imgFile* remap_dest = imgFile::LoadImage( s_remap_dest );
		if( remap_dest == NULL || remap_dest->IsOk() == false )
		{
			delete remap_src;
			if( remap_dest != NULL )
			{
				delete remap_dest;
				remap_dest = NULL;
			}
			remaps_src[img_state_ID] = NULL;
			return NULL;
		}
		
		remappedImg = Build_Remap( _src, src2, remap_src, remap_dest );
		delete remap_src;
		delete remap_dest;
	}
	
	//**********************************************************
	//  16 Bits mod remap
	else
	{
		wxImage imgRemap( GetObFile(oRemap->GetToken(0)).GetFullPath());
		if( imgRemap.IsOk() == false )
		{	RET_IMG_DATAS__GET_REMAP__ERROR	}
		wxPalette palRemap = imgRemap.GetPalette();
		if( palRemap.IsOk() == false )
		{	RET_IMG_DATAS__GET_REMAP__ERROR	}
		
		// Now do the remapping
		wxImage* remappedImg = new wxImage(*_src);
		unsigned char* pixs = remappedImg->GetData();
		
		int _w = src->GetWidth();
		int nb_pixels = remappedImg->GetWidth() * remappedImg->GetHeight();
		map<unsigned long int,unsigned long int>::iterator it;
		for( int i = 0; i < nb_pixels /3;i++)
		{
			int x = i % _w;
			int y = i / _w;
			int ind = src2->GetIndex(x,y);
			if( ind >= 0 )
				palRemap.GetRGB(ind, &pixs[i], &pixs[i+1],&pixs[i+2]);
		}
	}

	//-----------------------------------------------------------------
	// Invalid image
	if( remappedImg == NULL || remappedImg->IsOk() == false )
	{
		if( remappedImg != NULL )
			delete remappedImg;
		remappedImg = NULL;
	}
	
	//-----------------------------------------------------------------
	// Recolor Selection zones and SetMask
	else
	{
		if( Get_Selection_ID(img_state_ID) > 0 )
			RecolorSelectZone( remappedImg, img_state_ID );

		if( mask_type != 0 )
			remappedImg->SetMaskColour( mask_r,mask_g,mask_b );
	}

	//-----------------------------------------------------------------
	// Finally
	remaps_src[img_state_ID] = remappedImg;
	return remappedImg;
}

//-------------------------------------------------------
void
Img_Collection::RecomputeImageState(
			  Img_Collection::Img_state* imgState
			, size_t img_state_ID
			, float zoom_factor
			)
{
	if( imgState->img != NULL )
	{
		delete imgState->img;
		imgState->img = NULL;
	}
	if( imgState->bmp != NULL )
	{
		delete imgState->bmp;
		imgState->bmp = NULL;
	}
	
	size_t num_remap = Get_Remap_Num(img_state_ID);
	
	//--------------------------------------------------------
	// REVERSED CASE
	if( Is_Reversed(img_state_ID) == true  )
	{
		size_t t_img_state_ID = img_state_ID & ~IMGSTT_MASK_REVERSE;
		imgState->img = this->Get_Image( t_img_state_ID, zoom_factor );
		if( imgState->img == NULL )
		{
			imgState->img = NULL;
			return;
		}
		imgState->img = new wxImage( imgState->img->Mirror() );
	}

	//--------------------------------------------------------
	// Non remapped case
	else if( num_remap == 0 )
	{
		if( Get_Selection_ID(img_state_ID) > 0 )
		{
			wxImage* alt_src = src_selected;
			if( Is_Miniature( img_state_ID ) == true )
				alt_src = src_selected_mini;
			
			if( alt_src == NULL )
				return;
			
			imgState->img = new wxImage(
				alt_src->Scale( ceil(src->GetWidth() * zoom_factor)
						,ceil(src->GetHeight() * zoom_factor)));
		}
		else
			imgState->img = new wxImage(
				src->Scale( ceil(src->GetWidth() * zoom_factor)
					    , ceil(src->GetHeight() * zoom_factor)));
	}
	
	// A Remapped image
	else // num_remap > 0
	{
		// See if there is some remaps...
		wxImage* remapped_src = Get_Remap( img_state_ID );
		if( remapped_src == NULL )
		{
			imgState->img = NULL;
			return;
		}
		
		imgState->img = new wxImage(
			remapped_src->Scale( ceil(src->GetWidth() * zoom_factor)
						, ceil(src->GetHeight() * zoom_factor)));
	}
	
	
	//-------------------------------------------------------------
	// FINALLY
	if( imgState->img != NULL && imgState->img->IsOk() == false )
	{
		delete imgState->img;
		imgState->img = NULL;
	}
	
	else if( imgState->img != NULL )
	{
		if( Get_Selection_ID(img_state_ID) > 0 )
			RecolorSelectZone( imgState->img, img_state_ID);

		if( mask_type != 0 )
			imgState->img->SetMaskColour( mask_r,mask_g,mask_b );
	}
	imgState->zoom_factor = zoom_factor;
}


//-------------------------------------------------------
#define ANN__IS_VALID_PIX( PIX ) \
		(							\
				pixels[PIX]   != mask_r		\
			||	pixels[PIX+1] != mask_g		\
			||	pixels[PIX+2] != mask_b		\
		)


wxImage*	    
Img_Collection::BuilSelectedImage(wxImage* src
				,wxColour& sel_colour,size_t _border_size )
{
	if( src == NULL)
		return NULL;
	if( src->IsOk() == false )
	{
		delete src;
		src = NULL;
		return NULL;
	}
	
	wxImage* res = new wxImage(*src);
	if( _border_size == 0 )
		return res;
	int border_size = _border_size;
	
	unsigned char r,g,b;
	res->FindFirstUnusedColour(&r,&g,&b);
	sel_colour = wxColour(r,g,b);
	
	int w = res->GetWidth();
	int h = res->GetHeight();
	int nb_pixels = w * h;
	
	// wxWidget don't handle the GetData() properly (ie: erase src data)
	unsigned char* origin_pixels = res->GetData();
	unsigned char* pixels = (unsigned char*) malloc(nb_pixels*3);
	memcpy( pixels, origin_pixels, nb_pixels*3);
	
	set<int>::iterator it;
	set<int> set_recolor;

	// First : image BORDERS
		// Left Borders
		for(int decal= border_size-1; decal >=0; decal--)
		{
			int offset = decal*3;
			for( int i = 0; i < h; i++ )
			{
				int pix_0 = offset;
				if( ANN__IS_VALID_PIX( pix_0 ) )
				{
					int m = (i>decal?i-decal:0);
					int M = (i+decal>=h?h-1:i+decal);
					// Must recolor this one and propagate left
					for(int j = decal; j>=0;j--)
					{
						for( int k = m;k<=M;k++)
							set_recolor.insert(k*w+j);
					}
				}
				offset += w*3;
			}
		}
		// Right Borders
		for(int decal= border_size-1; decal >=0; decal--)
		{
			int offset = (w-1-decal)*3;
			for( int i = 0; i < h; i++ )
			{
				int pix_0 = offset;
				if( ANN__IS_VALID_PIX( pix_0 ) )
				{
					int m = (i>decal?i-decal:0);
					int M = (i+decal>=h?h-1:i+decal);
					// Must recolor this one and propagate left
					for(int j = w-decal-1; j<w;j++)
					{
						for( int k = m;k<=M;k++)
							set_recolor.insert(k*w+j);
					}
				}
				offset += w*3;
			}
		}
		// Up Borders
		for(int decal= border_size-1; decal >=0; decal--)
		{
			for( int i = 0; i < w; i++ )
			{
				int pix_0 = (decal*w + i)*3;
				if( ANN__IS_VALID_PIX( pix_0 ) )
				{
					int m = (i>decal?i-decal:0);
					int M = (i+decal>=w?w-1:i+decal);
					// Must recolor this one and propagate left
					for(int j = decal; j>=0;j--)
					{
						for( int k = m;k<=M;k++)
							set_recolor.insert(decal*w+k);
					}
				}
			}
		}
		// Down Borders
		for(int decal= border_size-1; decal >=0; decal--)
		{
			for( int i = 0; i < w; i++ )
			{
				int pix_0 = ((h-1-decal)*w + i)*3;
				if( ANN__IS_VALID_PIX( pix_0 ) )
				{
					int m = (i>decal?i-decal:0);
					int M = (i+decal>=w?w-1:i+decal);
					// Must recolor this one and propagate left
					for(int j = decal; j>=0;j--)
					{
						for( int k = m;k<=M;k++)
							set_recolor.insert((h-1-j)*w+k);
					}
				}
			}
		}
	
	
	// Pass First  : HORIZONTAL
	int num_pix = border_size+border_size*w;
	int i_max = h-border_size;
	int j_max = w-border_size;
	for( int i = border_size; i < i_max; i++ )
	{
		int state = 0;
		for( int j = border_size; j < j_max; j++ )
		{
			int pix = num_pix*3;
			bool b_valid = ANN__IS_VALID_PIX( pix );
			
			// Come from a empty region and found a non empty pixel
			if( b_valid == true && state == 0 ) 
			{
				// Propagate to left
				for(int l = i-(border_size-1); l<i+border_size;l++)
				{
					for( int c = j-border_size;c<j;c++)
					{
						int t_pix = (l*w+c)*3;
						if( ANN__IS_VALID_PIX( t_pix ) == false )
							set_recolor.insert(l*w+c);
					}
				}
				state = 1;
			}
			// Come from a filled region and found an empty pixel
			else if( b_valid == false && state == 1 ) 
			{
				// Propagate to right
				for(int l = i-(border_size-1); l<i+border_size;l++)
				{
					for( int c = j+border_size-1;c>=j;c--)
					{
						int t_pix = (l*w+c)*3;
						if( ANN__IS_VALID_PIX( t_pix ) == false )
							set_recolor.insert(l*w+c);
					}
				}
				state = 0;
			}
			num_pix++;
		}
		
		// Special case : a region to fill in the borders zone
		int pix = num_pix*3;
		bool b_valid = ANN__IS_VALID_PIX( pix );
		if( state == 1 && b_valid == false )
		{
			// Propagate to right
			for(int l = i-(border_size-1); l<i+border_size;l++)
			{
				for( int c = j_max+border_size-1;c>=j_max;c--)
				{
					int t_pix = (l*w+c)*3;
					if( ANN__IS_VALID_PIX( t_pix ) == false )
						set_recolor.insert(l*w+c);
				}
			}
		}
			
		num_pix+=border_size*2;
	}
	
	// Pass Second : VERTICAL
	for( int j = border_size; j < j_max; j++ )
	{
		int num_pix = j+border_size*w;
		int state = 0;
		for( int i = border_size; i < i_max; i++ )
		{
			int pix = num_pix*3;
			bool b_valid = ANN__IS_VALID_PIX( pix );
			
			// Come from a empty region and found a non empty pixel
			if( b_valid == true && state == 0 ) 
			{
				// Propagate to up
				bool b_cont = true;
				for(int l = i-(border_size-1); l<i;l++)
				{
					int t_pix = (l*w+j)*3;
					if( ANN__IS_VALID_PIX( t_pix ) == false )
						set_recolor.insert(l*w+j);
					else
					{
						b_cont = false;
						break;
					}
				}
				
				if( b_cont == true )
				{
					for(int c = j-(border_size-1); c<j+border_size;c++)
					{
						int t_num_pix = ((i-border_size)*w+c);
						int t_pix = t_num_pix*3;
						if( ANN__IS_VALID_PIX( t_pix ) == false )
							set_recolor.insert(t_num_pix);
					}
				}
				state = 1;
			}
			// Come from a filled region and found an empty pixel
			else if( b_valid == false && state == 1 ) 
			{
				// Propagate to down
				bool b_cont = true;
				if( border_size > 1 )
				{
					for(int l = i+(border_size-2); l>=i;l--)
					{
						int t_pix = (l*w+j)*3;
						if( ANN__IS_VALID_PIX( t_pix ) == false )
							set_recolor.insert(l*w+j);
						else
						{
							b_cont = false;
							break;
						}
					}
				}
				
				if( b_cont == true )
				{
					for(int c = j-(border_size-1); c<j+border_size;c++)
					{
						int t_num_pix = ((i+border_size-1)*w+c);
						int t_pix = t_num_pix*3;
						if( ANN__IS_VALID_PIX( t_pix ) == false )
							set_recolor.insert(t_num_pix);
					}
				}
				state = 0;
			}
			num_pix+=w;
		}

		// Special case : a region to fill in the borders zone
		int pix = num_pix*3;
		bool b_valid = ANN__IS_VALID_PIX( pix );
		if( b_valid == false && state == 1 ) 
		{
			// Propagate to down
			bool b_cont = true;
			if( border_size > 1 )
			{
				for(int l = i_max+(border_size-2); l>=i_max;l--)
				{
					int t_pix = (l*w+j)*3;
					if( ANN__IS_VALID_PIX( t_pix ) == false )
						set_recolor.insert(l*w+j);
					else
					{
						b_cont = false;
						break;
					}
				}
			}
			
			if( b_cont == true )
			{
				for(int c = j-(border_size-1); c<j+border_size;c++)
				{
					int t_num_pix = ((i_max+border_size-1)*w+c);
					int t_pix = t_num_pix*3;
					if( ANN__IS_VALID_PIX( t_pix ) == false )
						set_recolor.insert(t_num_pix);
				}
			}
			state = 0;
		}
		num_pix+=w;
	}


	//------------------------------------------------------------
	// Apply recoloring
	while( set_recolor.empty() == false )
	{
		it = set_recolor.begin();
		int i = (*it);
		set_recolor.erase(it);
		pixels[i*3]   = r;
		pixels[i*3+1] = g;
		pixels[i*3+2] = b;
	}

	res->SetData( pixels );
	if( mask_type != 0 )
		res->SetMaskColour( mask_r,mask_g,mask_b );
	
	return res;
}


//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
wxColour ObImgs_Manager::def_mask_colour( 255,0,255);

ObImgs_Manager::ObImgs_Manager()
{
	Reset();
}

//-------------------------------------------------------
void
ObImgs_Manager::Reset()
{
	map<string,Img_Collection*>::iterator it;
	while( theImgsMap.empty() == false )
	{
		it = theImgsMap.begin();
		delete it->second;
		theImgsMap.erase( it );
	}
	
	// Add Some mandatory images
	wxImage* ent_shadow = new wxImage( GetRessourceFile_String(wxT("ent_shadow.png")) );
	AbsorbeImage( wxT("ent_shadow"), NULL, ent_shadow, MASK_IN_IMAGE );
//	wxImage* ent_AT = new wxImage( GetRessourceFile_String("ent_AT.png") );
//	AbsorbeImage( "ent_AT", NULL, ent_AT, false );
	AbsorbeImage( wxT("noImg"), NULL, new wxImage(*noImg), MASK_NONE );

	for( int i = 1; i <= 4; i++ )
	{
		wxString imgName = wxT("pSpawn_")+IntToStr(i)+wxT(".png");
		wxImage* sImg = new wxImage( GetRessourceFile_String(imgName) );
		AbsorbeImage( imgName, NULL, sImg, MASK_IN_IMAGE );
	}
}


//-------------------------------------------------------
void
ObImgs_Manager::Invalidate_ImagesStates()
{
	map<string,Img_Collection*>::iterator it(theImgsMap.begin())
							, it_end(theImgsMap.end());
	for(;it!=it_end;it++)
	{
		if( it->second == NULL )
			continue;
		it->second->Reset_States();
	}
}

//-------------------------------------------------------
ObImgs_Manager::~ObImgs_Manager()
{
	Reset();
	ReleaseImage( wxT("ent_shadow") );
//	ReleaseImage( "ent_AT" );
	ReleaseImage( wxT("noImg") );
	for( int i = 1; i <= 4; i++ )
		ReleaseImage( wxT("pSpawn_")+IntToStr(i)+wxT(".png") );
}

//-------------------------------------------------------
bool
ObImgs_Manager::AbsorbeImage( const wxString& _key, ob_object* obj
				, wxImage* theImage, int _mask_type )
{
	if( theImage == NULL || theImage->IsOk() == false )
	{
		if( theImage != NULL )
			delete theImage;
		return false;
	}
	
	string key = string((char*)_key.c_str());
	map<string,Img_Collection*>::iterator it(theImgsMap.find(key));

	// Collection exists in DB
	if( it != theImgsMap.end() )
	{
		if( it->second != NULL )
			delete it->second;
		theImgsMap.erase( it );
	}

	Img_Collection* t = new Img_Collection(obj, theImage, _key, _mask_type );
	theImgsMap[key] = t;
	return true;
}


//-------------------------------------------------------
void
ObImgs_Manager::ReleaseImage( const wxString& _key )
{
	string key((char*)_key.c_str());
	map<string,Img_Collection*>::iterator it(theImgsMap.find(key));

	// Collection exists in DB
	if( it != theImgsMap.end() )
	{
		delete it->second;
		theImgsMap.erase( it );
	}

	return;
}

//-------------------------------------------------------
Img_Collection*
ObImgs_Manager::GetCollection(ob_object* obj, const wxString& imgPath, bool b_quiet )
{
	string key((char*)imgPath.c_str());
	map<string,Img_Collection*>::iterator it(theImgsMap.find(key));
	
	// Collection exists in DB
	if( it != theImgsMap.end() )
		return it->second;
	
	// Have to fill one
	wxImage* t_img = NULL;
	if( wxFileExists( imgPath ) == true )
		t_img = new wxImage( imgPath );

	if( t_img == NULL || t_img->IsOk() == false )
	{
		if( b_quiet == false )
		{
			cerr<<"-----------------------------------------------------"<<endl;
			cerr<<"ERROR : ObImgs_Manager::GetCollection : Invalid Path :"<<imgPath<<endl;
		}
		if(t_img != NULL)
			delete t_img;
		theImgsMap[key] = NULL;
		return NULL;
	}
	
	Img_Collection* t = new Img_Collection(obj, t_img, imgPath );
	theImgsMap[key] = t;
	return t;
}

//-------------------------------------------------------
wxImage* 
ObImgs_Manager::GetImage(ob_object* obj, const wxString& imgPath, float zoom_factor, int remap, bool b_reversed, int sel_state, bool b_miniature , bool b_quiet )
{
	ANN_Init_ImgStates_Masks();
	
	size_t img_state_ID = remap;
	if( b_reversed == true )
		img_state_ID |= IMGSTT_MASK_REVERSE;
	if( b_miniature == true )
		img_state_ID |= IMGSTT_MASK_MINIATURE;

	if( sel_state > 0 )
		img_state_ID |= (IMGSTT_MASK_SEL_START>>(sel_state-1));
	
	Img_Collection* ic = GetCollection( obj, imgPath, b_quiet );
	if( ic == NULL )
		return NULL;
	
	wxImage* res = ic->Get_Image( img_state_ID, zoom_factor );
	return res;
}

//-------------------------------------------------------
wxBitmap* 
ObImgs_Manager::GetBitmap(wxImage*& pimg, ob_object* obj, const wxString& imgPath, float zoom_factor, int remap, bool b_reversed, int sel_state, bool b_miniature , bool b_quiet )
{
	ANN_Init_ImgStates_Masks();
	pimg = NULL;
	
	size_t img_state_ID = remap;
	if( b_reversed == true )
		img_state_ID |= IMGSTT_MASK_REVERSE;
	if( b_miniature == true )
		img_state_ID |= IMGSTT_MASK_MINIATURE;

	if( sel_state > 0 )
		img_state_ID |= (IMGSTT_MASK_SEL_START>>(sel_state-1));
	
	Img_Collection* ic = GetCollection( obj, imgPath, b_quiet );
	if( ic == NULL )
		return NULL;
	
	wxBitmap* res = ic->Get_Bitmap(pimg, img_state_ID, zoom_factor );
	if( pimg == NULL )
		return NULL;
	
	return res;
}

//-------------------------------------------------------
wxImage* 
ObImgs_Manager::GetImageSrc( ob_object* obj, const wxString& imgPath, bool b_quiet )
{
	Img_Collection* ic = GetCollection( obj, imgPath, b_quiet );
	if( ic == NULL )
		return NULL;
	
	return ic->src;
}

//-------------------------------------------------------
bool     
ObImgs_Manager::IsImageExists( const wxString& key )
{
	return ( theImgsMap.find(string((char*)key.c_str())) != theImgsMap.end() );
}







//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
/**
	 A Function to template AT/JOIN/WAIT/etc... images constructions
*/
#define OB_CURS_MAX_H_OVERFLOW 20
#define OB_CURS_TXT_SPACE_UP 2
#define OB_CURS_FONT_SIZE 13

wxImage* 
Get_Image_StageCursor(	const wxSize& view_size ,
				float  zoom_factor,
				int    select_state,
				int	 vert_truncate_up,
				int	 vert_truncate_down,
				int	 horiz_width_up,
				int	 horiz_width_down,
				const  wxColour& color,
				const  wxString& label
				)
{
	wxString img_key = wxT("Cursor_") + label;
	wxSize clientSize(view_size.x*zoom_factor, view_size.y*zoom_factor );
	
	// Try the potentially already builded one
	wxImage* res = obImgs_Mgr->GetImage(
				  NULL
				, img_key
				, 1
				, 0
				, false
				, select_state
				, false
				, true
				);
	if( res != NULL )
	{
		// Check if the size correspond
		int h = res->GetHeight();
		if(   h
			>= clientSize.y - OB_CURS_MAX_H_OVERFLOW
		   && h
		      <= clientSize.y+ OB_CURS_MAX_H_OVERFLOW 
		  )
			return res;
		
		// Not correspond
		obImgs_Mgr->ReleaseImage( img_key );
		res = NULL;
	}
	
	//--------------------------------
	// Must rebuild it
	//--------------------------------
	
	// Prepare the bitmap and the DC to draw the src img
	int btmp_w = max( horiz_width_down, horiz_width_up );
	wxBitmap btmp( btmp_w, clientSize.y-1 );
	wxMemoryDC theDC( btmp );
	
	wxColour m_color = ObImgs_Manager::def_mask_colour;
	theDC.SetBackground( m_color );
	theDC.Clear();
	theDC.SetPen( wxPen( color, D_PEN_CURS_WIDTH ) ); 
	theDC.SetFont( wxFont( OB_CURS_FONT_SIZE, wxFONTFAMILY_DEFAULT
					,wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD
			 ));
	
	// Intersection points
	wxSize sX_up(   btmp_w / 2, vert_truncate_up );
	wxSize sX_down( btmp_w / 2, clientSize.y - vert_truncate_down );
	wxSize txt_size = theDC.GetTextExtent( label );
	
	// Draw the Rectangle above the label
	if( label.Len() > 0 )
	{
		theDC.SetBrush( *wxBLACK_BRUSH );
		theDC.SetTextForeground( color );
		theDC.DrawRectangle( 
				  sX_down.x - horiz_width_down/2
				, sX_down.y
				, sX_down.x + horiz_width_down/2
				, OB_CURS_TXT_SPACE_UP * 2 + txt_size.y
				);
	}
	
	// Draw the Vertical Bar
	theDC.DrawLine( sX_up.x, sX_up.y, sX_down.x, sX_down.y );
	
	// Draw the Up Horyzontal Bar
	if( horiz_width_up > 0 )
		theDC.DrawLine(     sX_up.x - horiz_width_up/2, sX_up.y
					, sX_up.x + horiz_width_up/2, sX_up.y );

	// Draw the Down Horyzontal Bar
	if( horiz_width_down > 0 )
		theDC.DrawLine(     sX_down.x - horiz_width_down/2, sX_down.y
					, sX_down.x + horiz_width_down/2, sX_down.y );

	// Draw the label
	if( label.Len() > 0 )
	{
		theDC.SetTextForeground( color );
		theDC.DrawText( label
				, sX_down.x - txt_size.x/ 2
				, sX_down.y + OB_CURS_TXT_SPACE_UP
				);
	}
	
	// Remove the Bitmap from the DC
	theDC.SelectObject( wxNullBitmap );
	
	// Convert the Bitmap to an image
	res = new wxImage( btmp.ConvertToImage() );
	if( res->IsOk() == false )
	{
		delete res;
		res = new wxImage( *noImg );
		res->Rescale(btmp_w, clientSize.y );
	}
	else
		res->SetMaskColour( m_color.Red(), m_color.Green(), m_color.Blue() );
	
//	res->Resize( wxSize(btmp_w, clientSize.y), wxPoint(0,0)  );
	obImgs_Mgr->AbsorbeImage( img_key, NULL, res, MASK_IN_IMAGE );

	// And return the img
	return obImgs_Mgr->GetImage(
				  NULL
				, img_key
				, 1
				, 0
				, false
				, select_state
				);
}

/**
	 A Function to template line images constructions
*/
wxImage* 
Get_Image_Rect(		wxString& img_key,
				int ID_Rect,
				const wxSize& view_size ,
				float  zoom_factor,
				int    select_state,
				int	 ob_width,
				int	 ob_height,
				const wxColour& color
				)
{
	img_key = wxT("Rect_") + IntToStr( ID_Rect );
	int spaces = (4/MAP_ZF);
	wxSize btmp_size( ob_width*zoom_factor + spaces*2, ob_height*zoom_factor + spaces*2 );

	// Try the potentially already builded one
	wxImage* res = obImgs_Mgr->GetImage(
				  NULL
				, img_key
				, 1
				, 0
				, false
				, select_state
				, false
				, true
				);
	
	if( res != NULL )
	{
		// Check if the size correspond
		if(   res->GetWidth()  == btmp_size.x
		   && res->GetHeight() == btmp_size.y
		  )
			return res;
		
		// Not correspond
		obImgs_Mgr->ReleaseImage( img_key );
		res = NULL;
	}

	// Prepare the bitmap and the DC to draw the rect
	wxBitmap btmp( btmp_size.x, btmp_size.y );
	wxMemoryDC theDC( btmp );

	wxColour m_color = ObImgs_Manager::def_mask_colour;
	theDC.SetBackground( m_color );
	theDC.SetPen( wxPen( color, D_PEN_LINE_WIDTH ) ); 
	theDC.SetBrush( wxBrush( color ) ); 
	theDC.Clear();

	// Draw the rect in it
	theDC.DrawRectangle(spaces,spaces,ob_width*zoom_factor,ob_height*zoom_factor);
	
	// Remove the Bitmap from the DC
	theDC.SelectObject( wxNullBitmap );
	
	// Convert the Bitmap to an image
	res = new wxImage( btmp.ConvertToImage() );
	res->SetMaskColour( m_color.Red(), m_color.Green(), m_color.Blue() );
	
	// Put it in the DB
	obImgs_Mgr->AbsorbeImage( img_key, NULL, res, MASK_IN_IMAGE );

	// And return the img
	return res;
}
