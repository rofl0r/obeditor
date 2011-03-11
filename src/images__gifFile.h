/*
 * gifFile.h
 *
 *  Created on: 12 nov. 2008
 *      Author: pat
 */

#ifndef GIFFILE_H_
#define GIFFILE_H_

#include <wx/image.h>
#include <gif_lib.h>
#include "images__imgFile.h"


class wxIndexedGIF : public imgFile
{
public:
	wxIndexedGIF();
	wxIndexedGIF( const wxString& str_GIFFile );
	virtual ~wxIndexedGIF();

	virtual bool LoadFrom(const  wxString& str_GIFFile );

	virtual bool IsOk();
	virtual bool SaveAs( const wxString& str_GIFFile );

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
	virtual int WriteGIF( const wxString& str_fn );
	virtual int ReadGIF( const wxString& str_fn );

    GifWord SWidth, SHeight,
      SColorResolution,
      SBackGroundColor;
    struct SavedImage *SavedImage;
	unsigned char *RasterBits;
};


#endif /* GIFFILE_H_ */
