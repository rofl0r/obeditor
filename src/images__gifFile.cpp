/*
 * gifFile.cpp
 *
 *  Created on: 12 nov. 2008
 *      Author: pat
 */
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gif_lib.h>
#include <wx/image.h>
#include <wx/palette.h>

#include "images__gifFile.h"
#include "images__pngFile.h"
#include "images__MyPalette.h"

using namespace std;

//---------------------------------------------------------------
imgFile* 
imgFile::LoadImage( const wxString& img_path )
{
	wxString ext = img_path.Right(3).Upper();
	imgFile* res = NULL;
	if( ext == wxT("GIF") )
		res = new wxIndexedGIF( img_path );
	else if( ext == wxT("PNG") )
		res = new wxIndexedPNG( img_path );
	
	if( res == NULL )
		return res;
	if( res->IsOk() == false )
	{
		delete res;
		return NULL;
	}
	return res;
}
	


//*************************************************

wxIndexedGIF::wxIndexedGIF()
{
	Init();
}


//*************************************************

void wxIndexedGIF::Init()
{
	height = width = 0;
    SWidth = SHeight = SColorResolution = SBackGroundColor = 0;
	RasterBits = NULL;
	SavedImage = NULL;
	str_fn =wxString();
	type = pIMG_GIF;
}


//*************************************************

wxIndexedGIF::~wxIndexedGIF()
{
	Reset();
}


//*************************************************

void wxIndexedGIF::Reset()
{
	height = width = 0;
	if( RasterBits != NULL )
		delete RasterBits;
	RasterBits = NULL ;

	if( SavedImage != NULL )
	{
		delete[] SavedImage->ImageDesc.ColorMap->Colors;
		delete SavedImage->ImageDesc.ColorMap;
		delete SavedImage;
		SavedImage = NULL;
	}
    SWidth = SHeight = SColorResolution = SBackGroundColor = 0;
	str_fn = wxString();
}


//*************************************************

bool wxIndexedGIF::IsOk()
{
	return ! ( height == 0 || width == 0  || SavedImage == NULL );
}


//*************************************************

wxIndexedGIF::wxIndexedGIF( const wxString& str_GIFFile )
{
	Init();
	LoadFrom( str_GIFFile );
}


//*************************************************

bool wxIndexedGIF::LoadFrom( const wxString& str_GIFFile )
{
	Reset();
	return ReadGIF( str_GIFFile ) == 0;
}


//*************************************************

int wxIndexedGIF::GetIndex( const int x, const int y)
{
	if( !IsOk() || x >= width || y >= height )
		return -1;

	return RasterBits[y*width+x];
}


//*************************************************

MyPalette* wxIndexedGIF::GetPalette()
{
	MyPalette* res = NULL;
	if( !IsOk() )
		return res;

	// Setting the palette
	if( SavedImage != NULL )
	{
		int nb_colour = SavedImage->ImageDesc.ColorMap->ColorCount;
		unsigned char r[nb_colour], g[nb_colour], b[nb_colour];
		for( int i = 0; i <  nb_colour; i++ )
		{
			r[i] = SavedImage->ImageDesc.ColorMap->Colors[i].Red;
			g[i] = SavedImage->ImageDesc.ColorMap->Colors[i].Green;
			b[i] = SavedImage->ImageDesc.ColorMap->Colors[i].Blue;
		}
		res = new MyPalette();
		res->Create( nb_colour, r, g, b );
		return res;
	}

	return res;
}

//*************************************************

bool wxIndexedGIF::SetPalette( wxPalette _pal )
{
	int ColorCount = _pal.GetColoursCount();
	if( !IsOk() ||  !_pal.IsOk() || ColorCount  <= 0 || ColorCount > 256 )
		return false;

	// Fill the palette of the gif
	unsigned char r,g,b;
	GifColorType *ColorMap = new GifColorType[ColorCount];
	for( int i = 0; i < _pal.GetColoursCount(); i ++)
	{
		_pal.GetRGB( i, &r,&g,&b );
		ColorMap[i].Red   = r;
		ColorMap[i].Green = g;
		ColorMap[i].Blue  = b;
	}

	// Fill remaining palette with black
	for( int i = _pal.GetColoursCount(); i < ColorCount; i ++)
	{
		ColorMap[i].Red   = 0;
		ColorMap[i].Green = 0;
		ColorMap[i].Blue  = 0;
	}

	// Replace old palette object in gif struct
	delete[] SavedImage->ImageDesc.ColorMap->Colors;
	SavedImage->ImageDesc.ColorMap->Colors = ColorMap;
	SavedImage->ImageDesc.ColorMap->ColorCount = _pal.GetColoursCount();

	return true;
}


//*************************************************

bool wxIndexedGIF::Remap( unsigned char *remap, int sz_remap )
{
	if( ! IsOk() )
		return false;

	for( int i = 0; i < height * width; i++)
	{
		// Out of bound, remap too small
		if( (int) RasterBits[i] >= sz_remap )
			return false;

		RasterBits[i] = remap[ RasterBits[i] ];
	}
	return true;
}


//*************************************************

unsigned char * wxIndexedGIF::GetDatas( int& datas_size )
{
	if( ! IsOk() )
	{
		datas_size = 0;
		return NULL;
	}

	datas_size = height * width;
	unsigned char *res = new unsigned char[ datas_size ];
	for( int i =0; i < datas_size; i++ )
		res[i] = RasterBits[i];

	return res;
}


//*************************************************

void wxIndexedGIF::SetDatas( unsigned char *datas, int datas_size )
{
	if( ! IsOk() || datas == NULL || datas_size == 0 )
		return;

	delete[] RasterBits;
	RasterBits = datas;
	SavedImage->RasterBits = RasterBits;
}


//*************************************************

bool wxIndexedGIF::SetPixel( int x, int y, int ind )
{
	if( ! IsOk() || x > width || y > height )
		return false;

	RasterBits[x + y*width] = ind;
	return true;
}


//*************************************************

bool wxIndexedGIF::SaveAs( const wxString& str_GIFFile )
{
	return WriteGIF( str_GIFFile ) == 0;
}


//***************************************************

struct ColorMapObject* Duplicate_ColorMap( struct ColorMapObject* cm )
{
	ColorMapObject *res = new ColorMapObject;
	res->ColorCount = cm->ColorCount;
	res->BitsPerPixel = cm->BitsPerPixel;
	if( res->ColorCount == 0 )
		res->Colors = NULL;
	else
	{
		res->Colors = new GifColorType[res->ColorCount];
		for( int i = 0; i < res->ColorCount; i++ )
			res->Colors[i] = cm->Colors[i];
	}
	return res;
}


//***************************************************
struct SavedImage* Duplicate_SavedImage( struct SavedImage* si )
{

	SavedImage* res = (SavedImage*) malloc( sizeof(SavedImage));
	res->ExtensionBlockCount = 0;
	res->ExtensionBlocks = NULL;
	res->ImageDesc.Left = si->ImageDesc.Left;
	res->ImageDesc.Top = si->ImageDesc.Top;
	res->ImageDesc.Width = si->ImageDesc.Width;
	res->ImageDesc.Height = si->ImageDesc.Height;
	res->ImageDesc.Interlace = si->ImageDesc.Interlace;
	if( si->ImageDesc.ColorMap == NULL )
		res->ImageDesc.ColorMap = NULL;
	else
	{
		ColorMapObject *SColorMap = (ColorMapObject*) 
							malloc( sizeof(ColorMapObject));
		SColorMap->ColorCount = si->ImageDesc.ColorMap->ColorCount;
		SColorMap->BitsPerPixel = si->ImageDesc.ColorMap->BitsPerPixel;
		if( SColorMap->ColorCount == 0 )
			SColorMap->Colors = NULL;
		else
		{
			SColorMap->Colors = (GifColorType*)
				malloc( sizeof(GifColorType) * SColorMap->ColorCount );
			for( int i = 0; i < SColorMap->ColorCount; i++ )
				SColorMap->Colors[i] = si->ImageDesc.ColorMap->Colors[i];
		}
		res->ImageDesc.ColorMap = Duplicate_ColorMap( si->ImageDesc.ColorMap );
	}

	if( si->RasterBits == NULL || res->ImageDesc.Width == 0 || res->ImageDesc.Height == 0 )
		res->RasterBits = NULL;
	else
	{
		int nb_pix = res->ImageDesc.Width * res->ImageDesc.Height;
		res->RasterBits = (unsigned char*) malloc(nb_pix);
		for (int i = 0; i < nb_pix; i++ )
			res->RasterBits[i] = si->RasterBits[i];
	}

	return res;
}

//*************************************************

int wxIndexedGIF::ReadGIF( const wxString& _str_fn)
{
	str_fn = _str_fn;


	// Try to open source file
	GifFileType *GifFileIn = DGifOpenFileName( (char*)_str_fn.c_str() );

	// Load the gif
	bool b_err = true;
	if( GifFileIn != NULL )
	{
		int res = DGifSlurp(GifFileIn);
		int gif_err = GifLastError();
		if( res == GIF_OK||gif_err == 0 /*|| gif_err == E_GIF_ERR_DATA_TOO_BIG */)
			b_err = (GifFileIn->ImageCount <= 0 );
	}
	
	if( b_err )
	{
		if( GifFileIn != NULL )
			DGifCloseFile(GifFileIn);
		GifFileIn = NULL;
		return 1;
	}
	Reset();

	height = GifFileIn->Image.Height;
	width  = GifFileIn->Image.Width;

    SWidth = GifFileIn->SWidth;
    SHeight = GifFileIn->SHeight;
    SColorResolution = GifFileIn->SColorResolution;
    SBackGroundColor = GifFileIn->SBackGroundColor;

    SavedImage = Duplicate_SavedImage( &GifFileIn->SavedImages[0] );
    if( SavedImage->ImageDesc.ColorMap == NULL )
    	SavedImage->ImageDesc.ColorMap = Duplicate_ColorMap( GifFileIn->SColorMap );

    if( SavedImage->ImageDesc.ColorMap == NULL )
    {
    	Reset();
    	return 1;
    }

    RasterBits = SavedImage->RasterBits;

	DGifCloseFile(GifFileIn);
	GifFileIn = NULL;

	return 0;
}


//*************************************************

int wxIndexedGIF::WriteGIF( const wxString& str_dest )
{
	// Verificatin on opened gif
	if( ! IsOk() )
		return 1;

	// Don't permit source == destinatino
	if( str_fn == str_dest )
		return 1;

	// Try to open dest file
	GifFileType *GifFileOut = EGifOpenFileName((char*) str_dest.c_str(), false );
	if( GifFileOut == NULL )
		return 1;

	// Copy data from the source file
     GifFileOut->SWidth = SWidth;
     GifFileOut->SHeight = SHeight;
     GifFileOut->SColorResolution = SColorResolution;
     GifFileOut->SBackGroundColor = SBackGroundColor;
     GifFileOut->SColorMap = NULL;

     struct SavedImage *si = Duplicate_SavedImage( SavedImage );
     MakeSavedImage(GifFileOut, si );

     int res = 0;
     if (EGifSpew(GifFileOut) == GIF_ERROR)
		res = 1;

	EGifCloseFile( GifFileOut );
	return res;
}
