/*
 * pngFile.h
 *
 *  Created on: 11 nov. 2008
 *      Author: pat
 */

#ifndef PNGFILE_H_
#define PNGFILE_H_

#include <wx/image.h>
#include "png.h"

#include "images__imgFile.h"


class wxIndexedPNG : public imgFile
{
public:
	wxIndexedPNG();
	wxIndexedPNG( const wxString& str_pngFile );
	wxIndexedPNG( wxImage& _image );
	virtual ~wxIndexedPNG();

	virtual bool LoadFrom( const wxString& str_pngFile );
	virtual bool LoadFrom( wxImage& _image );

	virtual bool IsOk();
	virtual bool SaveAs( const wxString& str_pngFile );

	virtual int GetIndex( const int x, const int y);
	virtual MyPalette* GetPalette();
	virtual bool SetPalette( wxPalette _pal );
	virtual bool Remap( unsigned char *remap, int sz_remap );

	virtual unsigned char * GetDatas( int& datas_size );
	virtual void SetDatas( unsigned char *datas, int datas_size );
	virtual bool SetPixel( int x, int y, int ind );

protected:
	virtual void Init();
	virtual void Reset();
	virtual int WritePng( const wxString& str_fn );
	virtual int ReadPng( const wxString& str_fn );

	int bit_depth, color_type, interlace_type, compression_type, filter_method;

	png_colorp palette;
	int num_palette;

	double gamma;
	int srgb_intent;
	png_color_8 sig_bit;

	png_byte *trans;
	int num_trans;
	png_color_16* trans_values;
	png_color_16* background;

	png_bytep *row_pointers;
};

#endif /* PNGFILE_H_ */
