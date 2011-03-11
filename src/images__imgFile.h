/*
 * imgFile.h
 *
 *  Created on: 13 nov. 2008
 *      Author: pat
 */

#ifndef IMGFILE_H_
#define IMGFILE_H_

enum
{	pIMG_NONE, pIMG_GIF, pIMG_PNG };

class MyPalette;

class imgFile
{
public:
	static imgFile* LoadImage( const wxString& img_path );
	
public:
	imgFile(){type = pIMG_NONE;};
	virtual ~imgFile(){};

	virtual bool LoadFrom( const wxString& str_GIFFile ) = 0;

	virtual bool IsOk() = 0;
	virtual bool SaveAs( const wxString& str_GIFFile ) = 0;

	virtual int GetIndex( const int x, const int y) = 0;
	virtual MyPalette* GetPalette() = 0;
	virtual bool SetPalette( wxPalette _pal ) = 0;
	virtual bool Remap( unsigned char *remap, int sz_remap ) = 0;

	virtual unsigned char * GetDatas( int& datas_size ) = 0;
	virtual void SetDatas( unsigned char *datas, int datas_size ) = 0;
	virtual bool SetPixel( int x, int y, int ind ) = 0;

	virtual void Init() = 0;
	virtual void Reset() = 0;

	int height, width;
	wxString str_fn;
	int type;
};

#endif /* IMGFILE_H_ */
