/*
 * pngFile.cpp
 *
 *  Created on: 11 nov. 2008
 *      Author: pat
 */

//============================================================================
// Name        : libpng_test.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
/*
*/

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <stdlib.h>
#include <iostream>
#include <wx/image.h>
#include <wx/palette.h>

#include "png.h"
#include "images__pngFile.h"
#include "images__MyPalette.h"

using namespace std;


//*************************************************

wxIndexedPNG::wxIndexedPNG()
{
	Init();
}


//*************************************************

void wxIndexedPNG::Init()
{
	type = pIMG_PNG;

	width = height = 0;
	bit_depth = 8;
	color_type= 3;
	interlace_type = 0;
	compression_type = 0;
	filter_method = 0;

	palette = NULL;
	num_palette = 0;

	gamma = 0;
	srgb_intent = 0;

	trans = NULL;
	num_trans = 0;
	trans_values = NULL;
	background = NULL;

	row_pointers = NULL;
}


//*************************************************

wxIndexedPNG::~wxIndexedPNG()
{
	Reset();

}


//*************************************************

void wxIndexedPNG::Reset()
{
	if( palette != NULL )
		delete[] palette;

	if( trans != NULL )
		delete[] trans;

	if( trans_values != NULL )
		delete trans_values;

	if( row_pointers != NULL )
	{
		for( int i = 0; i < height; i++)
		{
			if( row_pointers[i] != NULL )
			{
				delete[] row_pointers[i];
			}
		}
		delete[] row_pointers;
	}
	row_pointers = NULL;
}


//*************************************************

bool wxIndexedPNG::IsOk()
{
	return !( palette == NULL || color_type != 3 || height == 0 || width == 0 || row_pointers == NULL );
}


//*************************************************

wxIndexedPNG::wxIndexedPNG( const wxString& str_pngFile )
{
	Init();
	LoadFrom( str_pngFile );
}


//*************************************************

wxIndexedPNG::wxIndexedPNG( wxImage& _image )
{
	Init();
	LoadFrom( _image );
}


//*************************************************

bool wxIndexedPNG::LoadFrom( const wxString& str_pngFile )
{
	Reset();
	return ReadPng( str_pngFile ) == 0;
}


//*************************************************

bool wxIndexedPNG::LoadFrom( wxImage& _img )
{
	Reset();
	if( ! _img.IsOk() )
		return false;

	// Some arg vérifications
	wxPalette _pal = _img.GetPalette();

	if( _img.GetHeight() <= 0 || _img.GetWidth() <= 0 )
		return false;

	// copy the palette
	if( ! SetPalette( _pal ) )
		return false;

	// Get the dimensions
	height = _img.GetHeight();
	width  = _img.GetWidth();

	// Copy the image
	row_pointers = new png_bytep[height];
	for( int i = 0; i < height; i++)
	{
		row_pointers[i] = new png_byte[width];
		for(int j=0; j< width; j++ )
		{
			// Getting the RGB of the pixel
			unsigned char r = _img.GetRed( i,j);
			unsigned char g = _img.GetGreen( i,j);
			unsigned char b = _img.GetBlue( i,j);

			// Getting the index of the current pixel
			int _ind = _pal.GetPixel( r,g,b );
			if( _ind == wxNOT_FOUND )
				_ind = 0;

			// Save the index
			row_pointers[i][j] = (png_byte) _ind;
		}
	}

	return true;
}

//*************************************************

int wxIndexedPNG::GetIndex( const int x, const int y)
{
	if( !IsOk() || x >= width || y >= height )
		return -1;

	return row_pointers[y][x];
}


//*************************************************

MyPalette* wxIndexedPNG::GetPalette()
{
	MyPalette *res = NULL;
	if( !IsOk() )
		return res;


	unsigned char r[num_palette], g[num_palette], b[num_palette];
	for( int i = 0; i < num_palette; i ++)
	{
		r[i] = palette[i].red;
		g[i] = palette[i].green;
		b[i] = palette[i].blue;
	}

	res = new MyPalette();
	res->Create( num_palette, r, g, b );
	return res;
}


//*************************************************

bool wxIndexedPNG::SetPalette( wxPalette _pal )
{
	if( !IsOk() || !_pal.IsOk() ||  _pal.GetColoursCount() <= 0 || _pal.GetColoursCount() > 256 )
		return false;

	if( palette != NULL )
		delete[] palette;

	num_palette = _pal.GetColoursCount();
	palette = new png_color[num_palette];

	unsigned char r,g,b;
	for( int i = 0; i < num_palette; i ++)
	{
		_pal.GetRGB( i, &r,&g,&b );
		palette[i].red   = r;
		palette[i].green = g;
		palette[i].blue  = b;
	}
	return true;
}


//*************************************************

bool wxIndexedPNG::Remap( unsigned char *remap, int sz_remap )
{
	if( ! IsOk() )
		return false;

	for( int i = 0; i < height; i++)
	{
		for(int j=0; j< width; j++ )
		{
			// Out of bound, remap too small
			if( row_pointers[i][j] >= sz_remap )
				return false;

			row_pointers[i][j] = remap[ row_pointers[i][j] ];
		}
	}
	return true;
}

//*************************************************

unsigned char * wxIndexedPNG::GetDatas( int& datas_size )
{
	if( ! IsOk() )
	{
		datas_size = 0;
		return NULL;
	}

	datas_size = height * width * sizeof(png_byte);
	png_byte *res = new png_byte[ datas_size ];

	for( int i = 0; i < height; i++)
	{
		for(int j=0; j< width; j++ )
		{
			res[i*width + j] = row_pointers[i][j];
		}
	}

	return res;
}


//*************************************************

void wxIndexedPNG::SetDatas( unsigned char *_datas, int datas_size )
{
	if( ! IsOk() || _datas == NULL || datas_size == 0 )
		return;

	datas_size = datas_size / sizeof(png_byte);
	png_byte *datas = (png_byte*) _datas;

	for( int i = 0; i < height; i++)
	{
		for(int j=0; j< width; j++ )
		{
			row_pointers[i][j] = datas[i*width + j];
		}
	}
}


//*************************************************

bool wxIndexedPNG::SetPixel( int x, int y, int ind )
{
	if( ! IsOk() || x > width || y > height )
		return false;

	row_pointers[y][x] = ind;
	return true;
}


//*************************************************

bool wxIndexedPNG::SaveAs( const wxString& str_pngFile )
{
	return WritePng( str_pngFile ) == 0;
}


//*************************************************

int wxIndexedPNG::ReadPng( const wxString& str_fn)
{
	FILE *fp = fopen((char*)str_fn.c_str(), "rb");
	if (!fp)
		return 1;

	png_structp png_ptr = png_create_read_struct
	   (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
		NULL, NULL);
	if (!png_ptr)
	{
		fclose(fp);
		return 1;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr,
		   (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return 1;
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr,
		  (png_infopp)NULL);
		fclose(fp);
		return 1;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr,
		   &end_info);
		fclose(fp);
		Reset();
		return 1;
	}

	png_init_io(png_ptr, fp);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY , NULL);

	png_uint_32 t_width, t_height;
	png_get_IHDR(png_ptr, info_ptr, &t_width, &t_height,
		   &bit_depth, &color_type, &interlace_type,
		   &compression_type, &filter_method);
	width = t_width;
	height = t_height;

	// Handle only paletted png
	if( color_type != PNG_COLOR_TYPE_PALETTE || width == 0 || height == 0)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr,
		   &end_info);
		fclose(fp);
		Reset();
		return 1;
	}

	png_byte *t_trans = NULL;
	png_color_16 *t_background = NULL;

	png_get_gAMA(png_ptr, info_ptr, &gamma);
	png_get_sRGB(png_ptr, info_ptr, &srgb_intent);
	png_get_tRNS(png_ptr, info_ptr, &t_trans, &num_trans, &trans_values);
	png_get_bKGD(png_ptr, info_ptr, &t_background);

	// Copy background and transvalues
	if( t_background != NULL )
		background = new png_color_16(*t_background);

	if( num_trans > 0 )
	{
		trans = new png_byte[num_trans];
		for( int i =0; i< num_trans; i++)
			trans[i] = t_trans[i];
	}


	png_colorp t_palette = NULL;
	png_get_PLTE(png_ptr, info_ptr, &t_palette,
						 &num_palette);

	// Copy the palette
	if( num_palette > 0 )
	{
		palette = new png_color[num_palette];
		for( int i =0; i < num_palette; i++ )
			palette[i] = t_palette[i];
	}


	png_read_update_info(png_ptr, info_ptr);

	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_bytep *temp_row_pointers = png_get_rows(png_ptr, info_ptr);
	row_pointers = new png_bytep[height];
	for(int i=0;i<height;i++)
		row_pointers[i] = new png_byte[rowbytes];

	int color_found = 0;
	png_byte prev_byte = 0;
	for( int i = 0; i < height; i++ )
	{
		for( size_t j = 0; j < rowbytes; j++ )
		{
			row_pointers[i][j] = temp_row_pointers[i][j];
			if( row_pointers[i][j] != prev_byte && color_found < 10)
			{
				prev_byte = row_pointers[i][j];
				color_found ++;
				cout << "coulor found at ("<< i <<","<< j<<") : " << (int)prev_byte << endl;
			}
		}
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	fclose( fp );
	return 0;
}


//*************************************************

int wxIndexedPNG::WritePng( const wxString& str_fn )
{ // Writing a copy from scratch
	FILE *fp = fopen((char*)str_fn.c_str(), "wb");
	if (!fp)
	   return 1;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose( fp );
		return 1;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
	   png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	   fclose( fp );
	   return 1;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr );
		fclose(fp);
		Reset();
		return 1;
	}


	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, width, height,
		   bit_depth, color_type, interlace_type,
		   compression_type, filter_method);

	if( palette != NULL )
		png_set_PLTE(png_ptr, info_ptr, palette, num_palette);

	png_set_gAMA(png_ptr, info_ptr, gamma);
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);

	png_set_tRNS(png_ptr, info_ptr, trans, num_trans, trans_values);

	if( background != NULL )
		png_set_bKGD(png_ptr, info_ptr, background);

	if( row_pointers != NULL )
		png_set_rows( png_ptr, info_ptr, row_pointers);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose( fp );
	return 0;
}
