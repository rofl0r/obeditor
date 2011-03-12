#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/file.h>

#include "common__tools.h"
#include "common__mod.h"
#include "ob_editor.h"
#include "entity__globals.h"
#include "images__MyPalette.h"

using namespace std;
extern wxString curr_entity_filename;


/**\internal*****************
 * SOME GLOBALS
 * **************************
 */
wxConfigBase *config;
wxFileName dirRessources;
wxImage *noImg;




/**\internal*****************
 * Miscellanious facilities
 * **************************
 */

bool Frame_RestorePrevCoord( wxWindow* it, const wxString& _name )
{
  	int last_x, last_y, last_width, last_height;
  	bool b_ok = true;
  	b_ok = config->Read(wxT("/")+ _name + wxT("/last_x"), &last_x, 20) && b_ok;
  	b_ok = config->Read(wxT("/")+ _name + wxT("/last_y"), &last_y, 20) && b_ok;
  	b_ok = config->Read(wxT("/")+ _name + wxT("/last_width"), &last_width, 400) && b_ok;
  	b_ok = config->Read(wxT("/")+ _name + wxT("/last_height"), &last_height, 200) && b_ok;

  	it->SetSize(last_width,last_height);
  	it->Move( last_x,last_y );
  	return b_ok;
}


void Frame_SaveCoord( wxWindow* it, const wxString& name )
{
  	int _width, _height;
  	it->GetSize( &_width, &_height );
  	wxPoint _position = it->GetScreenPosition();

  	bool b_ok = true;
  	b_ok = config->Write(wxT("/")+ name + wxT("/last_x"), _position.x ) && b_ok;
  	b_ok = config->Write(wxT("/")+ name + wxT("/last_y"), _position.y ) && b_ok;
  	b_ok = config->Write(wxT("/")+ name + wxT("/last_width"), _width ) && b_ok;
  	b_ok = config->Write(wxT("/")+ name + wxT("/last_height"), _height ) && b_ok;

  	if( ! b_ok )
	    wxMessageBox(wxT("Cannot save Coords for window ") + name,
	                 wxT("ProPlem"), wxOK | wxICON_INFORMATION, it );
}


void Frame_DestroySavedCoord( const wxString& _name )
{
  	config->DeleteEntry(wxT("/")+ _name + wxT("/last_x"));
  	config->DeleteEntry(wxT("/")+ _name + wxT("/last_y"));
  	config->DeleteEntry(wxT("/")+ _name + wxT("/last_width"));
  	config->DeleteEntry(wxT("/")+ _name + wxT("/last_height"));

  	return;
}


//****************************************************

void ClearMenu( wxMenu* menu )
{
	if( menu == NULL || menu->GetMenuItemCount() <= 0 )
		return;

	wxMenuItem *t = menu->FindItemByPosition( 0 );
	while( 1 )
	{
		menu->Delete( t );
		if( menu->GetMenuItemCount() <= 0 )
			break;
		else
			t = menu->FindItemByPosition( 0 );
	}
}

void OpenInExternalEditor( wxWindow* parent, const wxString& filePath )
{
	// Get the editor from the config
	wxString editor_path;
	if( ! config->Read( wxT("/startFrame/editor_program"), &editor_path ) )
	{
		wxMessageBox(wxT("Did you correctly set the editor in the start window ??"),
						 wxT("ProPlem"), wxOK | wxICON_EXCLAMATION, parent );
		return;
	}

	if( ! wxFileName(editor_path).FileExists() )
	{
		wxMessageBox(wxT("Unable to find ")+ editor_path +wxT(" ??"),
						 wxT("ProPlem"), wxOK | wxICON_EXCLAMATION, parent );
		return;
	}

	if( ! wxFileName(filePath).FileExists() )
	{
		wxMessageBox(wxT("Unable to open the file \n\n")+ filePath,
						 wxT("ProPlem"), wxOK | wxICON_EXCLAMATION, parent );
		return;
	}

	// Launch the editor
	wxExecute( wxT("\"") + editor_path + wxT("\" \"") + filePath + wxT("\"") );	
}



//****************************************************

void EnableItemsMenu( wxMenu* menu, bool b_enable )
{
	if( menu == NULL || menu->GetMenuItemCount() <= 0 )
		return;

	wxMenuItemList itemsList = menu->GetMenuItems();
	wxMenuItemList::iterator iter;
	for (iter = itemsList.begin(); iter != itemsList.end(); ++iter)
	{
		wxMenuItem *current = *iter;
		current->Enable( b_enable );
	}
}




//********************************
/**\internal*****************
 * Images facilities
 * **************************
 */

wxColour MeltColor(wxColour _c0, wxColour _c1 )
{
	float _fact = 1;
	if( _c1.Red() != 0 )
		_fact = 1 + (float) _c0.Red() / (float) _c1.Red();
	int _r = ( _c1.Red() - _c0.Red() ) + _c0.Red() * _fact;
	if( _r >= 256 ) _r = 255;

	_fact = 1;
	if( _c1.Green() != 0 )
		_fact = 1 + (float) _c0.Green() / (float) _c1.Green();
	int _g = ( _c1.Green() - _c0.Green() ) + _c0.Green() * _fact;
	if( _g >= 256 ) _g = 255;

	_fact = 1;
	if( _c1.Blue() != 0 )
		_fact = 1 + (float) _c0.Blue() / (float) _c1.Blue();
	int _b = ( _c1.Blue() - _c0.Blue() ) + _c0.Blue() * _fact;
	if( _b >= 256 ) _b = 255;

	return wxColour( _r, _g, _b );
}


Image_Holder::Image_Holder()
{
	img = NULL;
	png_or_gif = NULL;
}


Image_Holder::~Image_Holder()
{
	RemoveImages();
}


int Image_Holder::Get_ColorIndex( const int x, const int y )
{
	if( Get_GIF_or_PNG() == NULL )
		return -1;

	return png_or_gif->GetIndex( x, y );
}


void Image_Holder::RemoveImages()
{
	if( img != NULL )
		delete img;
	img = NULL;

	if( png_or_gif != NULL )
		delete png_or_gif;
	png_or_gif = NULL;
}


wxImage* Image_Holder::GetImage()
{
	if( img != NULL )
		return img;

	wxString _path = GetFileName().GetFullPath();
	if( ! wxFileName(_path).FileExists() )
		return NULL;

	if( IsFileEmpty( _path))
		return NULL;

	if( IsFileEmpty( _path))
		return NULL;

	img = new wxImage( _path );
	if( ! img->IsOk() )
	{
		delete img;
		img = NULL;
	}

	return img;
}


imgFile* Image_Holder::Get_GIF_or_PNG()
{
	if( img == NULL && GetImage() == NULL )
		return NULL;

	wxFileName fn = GetFileName();
	wxString str_path = fn.GetFullPath();
	if( str_path ==wxString() )
		return NULL;

	if( ! fn.FileExists() )
		return NULL;

	// Check if the file is Zero Size
	if( IsFileEmpty( str_path))
		return NULL;

	if( fn.GetExt().Upper() ==wxT("PNG"))
		png_or_gif = new wxIndexedPNG( str_path );

	else if( fn.GetExt().Upper() ==wxT("GIF"))
		png_or_gif = new wxIndexedGIF( str_path );

	else
		return NULL;

	if( ! png_or_gif->IsOk() )
	{
		delete png_or_gif;
		png_or_gif = NULL;
		return NULL;
	}

	return png_or_gif;
}

//---------------------------------------------------------
wxString 
wxColour_ToStr( const wxColour& c )
{
	return 	        IntToStr(c.Red())
			+wxT(",")+ IntToStr(c.Green())
			+wxT(",")+ IntToStr(c.Blue())
			;
}

//---------------------------------------------------------
wxColour
wxColour_FromStr( const wxString& s )
{
	wxArrayString as = StrSplit( s,wxT(","));
	return wxColour(    StrToInt(as[0])
				, StrToInt(as[1])
				, StrToInt(as[2])
				);
}

void 
Image_Rescale_To_Fit( wxImage& img, size_t w, size_t h )
{
	if( img.IsOk() == false )
		return;
	
	size_t img_w = img.GetWidth();
	size_t img_h = img.GetHeight();
	float zoom_x = 10000;
	float zoom_y = 10000;

	if( img_w >=  w )
		zoom_y = (float)w/(float)img_w;
	if( img_h >=  h )
		zoom_x = (float)h/(float)img_h;
	
	if( zoom_y != 10000 || zoom_x != 10000 )
	{
		float z = min(zoom_x,zoom_y);
		img.Rescale( img_w * z, img_h * z );
		return;
	}

	zoom_y = (float)w/(float)img_w;
	zoom_x = (float)h/(float)img_h;
	
	float z = min(zoom_x,zoom_y);
	img.Rescale( img_w * z, img_h * z );
	return;
}

/**\internal*****************
 * Ob_objects facilities
 * **************************
 */
void Sort_ob_Object_ByName( ob_object**& objs, size_t count )
{
	for( size_t i =0; i < count ; i++ )
	{
		size_t curr_ind = i;
		for( size_t j = i+1; j < count; j++ )
			if( objs[j]->name.Upper() < objs[curr_ind]->name.Upper() )
				curr_ind = j;

		ob_object *temp = objs[i];
		objs[i] = objs[curr_ind];
		objs[curr_ind] = temp;
	}
}


void Sort_ob_Object_ByTag( ob_object**& objs, size_t count, size_t num_tag )
{
	for( size_t i =0; i < count ; i++ )
	{
		size_t curr_ind = i;
		for( size_t j = i+1; j < count; j++ )
			if( objs[j]->GetToken(num_tag).Upper() < objs[curr_ind]->GetToken(num_tag).Upper() )
				curr_ind = j;

		ob_object *temp = objs[i];
		objs[i] = objs[curr_ind];
		objs[curr_ind] = temp;
	}
}

void Sort_ob_Object_ByTag( ob_anim**& objs, size_t count, size_t num_tag )
{
	ob_object** temp = (ob_object**) objs;
	Sort_ob_Object_ByTag( temp, count, num_tag );
}


wxString* Reverse_Null_StrArray( wxString* tokens, size_t tokens_size )
{
	wxString* res = new wxString[tokens_size];
	int i = 0;
	int j = tokens_size -1;
	while( j >=0 )
	{
		res[j] = tokens[i];
		i++;
		j--;
	}
	return res;
}


wxString UnTokenize( ob_object* _obj )
{
	if( _obj == NULL)
		return wxString();

	wxString res;
	int j = 0;
	while(j>=0)
	{
		wxString _t = _obj->GetToken(j);
		if( _t == wxString() )
			j = -1;
		else
		{
			res += _t + wxT(" ");
			j++;
		}
	}
	res.Trim();
	return res;
}


wxArrayString ArrayString_PrependAll( const wxArrayString& arrStr, const wxString& _prefix )
{
	wxArrayString res;
	for( size_t i = 0; i < arrStr.Count(); i++)
	{
		res.Add( _prefix + arrStr[i] );
	}
	return res;
}


wxString ArrayString_Join( const wxArrayString& arrStr, const wxString& _jointure )
{
	if( arrStr.Count() <= 0 )
		return wxString();

	wxString res = arrStr[0];
	for( size_t i = 1; i < arrStr.Count(); i++)
	{
		res += _jointure + arrStr[i];
	}
	return res;
}

void TabStringToArrayString( wxString* tStr, size_t nb,  wxArrayString& arrStr )
{
	arrStr.Clear();
	for( size_t i=0; i < nb; i++)
		arrStr.Add( tStr[i] );
}


bool IsInArray( const wxString& _t, wxString* arr, size_t arr_tay )
{
	for( size_t i = 0; i < arr_tay; i++ )
	{
		if( _t.Upper() == arr[i].Upper() )
			return true;
	}
	return false;
}


/**\internal*****************
 * Conversions facilities
 * **************************
 */
wxString IntToStr( int i )
{
	return wxString::Format(wxT("%d"), (int)i);
}

wxString IntToStr( size_t i )
{
	return wxString::Format(wxT("%d"), static_cast<int>(i));
}

int StrToInt( const wxString& str, bool& ok )
{
	long res = 0;
	ok = str.ToLong(&res);
	return (int)res;
}

int StrToInt( const wxString& str )
{
	long res = 0;
	bool ok;
	ok = str.ToLong(&res);
	return (int)res;
}

unsigned int StrToUInt( const wxString& str, bool& ok )
{
	long int res = 0;
	ok = str.ToLong(&res);
	if( res < 0 )
	{
		ok = false;
		return (unsigned int) -res;
	}
	return (unsigned int) res;
}

float StrToFloat(const wxString& str, bool& ok )
{
	double res = 0;
	wxString t(str);
	t.Replace( wxT("."), wxT(",") );
	ok = t.ToDouble(&res);
	return (float)res;
}

float StrToFloat(const wxString& str )
{
	bool dummy;
	return StrToFloat( str, dummy );
}

wxString FloatToStr( const float f)
{
	wxString t;
	t.Printf( wxT("%f"), f);
	t.Replace( wxT(","), wxT(".") );
	if( t.Find( wxT('.') ) )
	{
		while( t[t.Len()-1] == wxT('0') )
			t.Truncate(t.Len() -1 );
		if( t[t.Len()-1] == '.' )
			t.Truncate(t.Len() -1 );
	}
	return t;
}

bool IntToBool(const int i )
{	return i != 0; }

bool StrToBool( const wxString& str )
{
	bool ok = false;
	int res = StrToInt( str, ok );
	if( !ok )
		return false;
	return IntToBool( res );
}




/**\internal*****************
 * Strings facilities
 * **************************
 */

bool StrIsObBool( const wxString& str )
{
	if( str.Len() != 1 )
		return false;
	if( str[0] != wxT('0') && str[0] != wxT('1') )
		return false;
	return true;
}

bool StrIsInt( const wxString& str )
{
	bool res;
	StrToInt(str, res );
	return res;
}

bool StrIsUInt( const wxString& str )
{
	bool res;
	StrToUInt(str, res );
	return res;
}

bool StrIsFloat( const wxString& str )
{
	bool res;
	StrToFloat(str, res );
	return res;
}

wxString ArrStrConcat( const wxArrayString& arrStr, const wxString& separator  )
{
	if( arrStr.Count() == 0 )
		return wxString();

	wxString res;
	for( size_t i=0; i < arrStr.Count()-1; i++)
		res += arrStr[i] + separator;
	res += arrStr[arrStr.Count()-1];
	return res;
}

wxArrayString StrSplit( const wxString& str, const wxString& delim, bool b_remove_empty )
{
	wxArrayString res;
	if( str.Len() == 0 )
	{
		res.Add(wxString());
		return res;
	}
	if( delim.Len() == 0 )
	{
		res.Add( str );
		return res;
	}

	wxString temp = str;
	while( true )
	{
		int ind = temp.Find( delim );
		if( ind == wxNOT_FOUND )
		{
			res.Add( temp );
			break;
		}
		else
		{
			if( ind != 0 || ! b_remove_empty)
				res.Add( temp.Left(ind) );
			temp = temp.Right( temp.Len() -delim.Len() - ind );
		}
	}
	return res;
}


inline wxArrayString  Concat_ArrayString( const wxArrayString & arr1, const wxArrayString & arr2 )
{
	wxArrayString res = arr1;
	for( size_t i = 0; i < arr2.Count(); i++ )
		res.Add( arr2[i] );
	return res;
}

inline wxArrayString __Tokenize(  wxArrayString& arr_str, const wxString& delim )
{
	wxArrayString res;
	for( size_t i = 0; i < arr_str.Count(); i++ )
		res = Concat_ArrayString( res, StrSplit( arr_str[i], delim, true ) );

	// removing empty elts
	wxArrayString res2;
	for( size_t i = 0; i < res.Count(); i++ )
		if( res[i] != wxString() )
			res2.Add( res[i]);
	return res2;
}

wxArrayString Tokenize( const wxString& str )
{
	wxArrayString res;
	if( str.Len() == 0 )
		return res;

	res.Add( str );
	res =  __Tokenize(  res, wxT(" ") );
	res =  __Tokenize(  res, wxT("\t") );
	return res;
}


//--------------------------------------------------------------------
void
wxArraystring__Add( wxArrayString& arr_str, size_t nb_str, ... )
{
	va_list ap;
	va_start (ap, nb_str);
	for( size_t i= 0; i < nb_str; i++ )
		arr_str.Add( wxString::FromAscii(va_arg(ap, const char* )));
	va_end (ap);
}

//--------------------------------------------------------------------
wxArrayString 
wxArrayString_Build( size_t nb_str, ... )
{
	wxArrayString res;
	va_list ap;
	va_start (ap, nb_str);
	for( size_t i= 0; i < nb_str; i++ )
		res.Add( wxString::FromAscii(va_arg(ap, const char* )));
	va_end (ap);
	
	return res;
}

;

/**\internal*****************
 * Files facilities
 * **************************
 */
wxFileName MyDirAppend( const wxFileName dataDirPath, const wxString& subDir )
{
	wxFileName res = dataDirPath;
	wxArrayString arr_dirs = StrSplit( subDir, wxT('/') );
	for( size_t i = 1; i < arr_dirs.Count(); i++)
		 res.AppendDir( arr_dirs[i] );
	return res;
}

wxFileName MyPathAppend( const wxFileName _dataDirPath, const wxString& subPath )
{
	wxFileName res = _dataDirPath;
	wxArrayString arr_dirs = StrSplit( subPath, wxT('/') );
	if( arr_dirs.Count() == 1 )
	{
		return wxFileName(res.GetPath(), arr_dirs[0] );
	}

	wxString filename = arr_dirs[arr_dirs.Count() -1];

	for( size_t i = 0; i < arr_dirs.Count() -1; i++)
		 res.AppendDir( arr_dirs[i] );
	return wxFileName(res.GetPath(), filename );
}

wxString 
ObPath_To_FullPath( const wxString& _subPath )
{
	return GetObFile( _subPath ).GetFullPath();
}

wxFileName GetObFile( const wxString& _subPath )
{
	return MyPathAppend( dataDirPath, _subPath);
}

wxString Convert_To_Ob_Path( const wxString& s_path )
{
	// first remove the base dir
	wxString __t = dataDirPath.GetPath();

	wxString _path(s_path);
	//if( _path.Replace( __t , _T(""), false ) == 0 ) // FIXME: figure what _T did !
	if( _path.Replace( __t , wxString(), false ) == 0 )		
		// No replacement made => not a valid dataDir relative path
		return wxString();

	wxFileName temp( _path );
	wxArrayString t_arr = temp.GetDirs();
	if( t_arr.Count() == 0 )
	{
        if( _path[0] == '/' )
            return _path.Mid( 1, _path.Len() -1 );
        else
            return _path;
	}

	wxString res = ArrStrConcat( t_arr, wxT("/") );
	if( res[0] == wxT('/') )
		res = res.Mid( 1, res.Len() -1 );
	if( res.Len() > 0 )
	    res += wxT('/') + temp.GetFullName();

	return res;
}


wxFileName	GetRessourceFile( const wxString& _strFileName )
{
	wxFileName temp = dirRessources;
	temp.SetFullName( _strFileName );
	return temp;
}

wxString	GetRessourceFile_String( const wxString& _strFileName )
{
	return GetRessourceFile( _strFileName ).GetFullPath();
}

void RemoveLastAtom( wxFileName& _fn )
{
	if( _fn.HasName() )
		_fn = _fn.GetPath();
	else
		_fn.RemoveLastDir();
}

bool DirExists( wxFileName _fn )
{
	wxString ttt = _fn.GetFullPath();
	if( ! _fn.IsDir() )
		return false ;

	if( _fn.DirExists() )
		return true;

	return false;
}

bool __RECURS_FindFirsValidPath( wxString& path, wxArrayString& sub_dirs, int& arr_ind )
{
	if( sub_dirs.Count() == 0)
		return true;

	wxString new_dir = path + wxFileName::GetPathSeparator() + sub_dirs[arr_ind];

	wxFileName ttt(new_dir + wxFileName::GetPathSeparator());

	if( ttt.DirExists() )
	{
		path = new_dir;
		arr_ind++;
		return __RECURS_FindFirsValidPath( path, sub_dirs, arr_ind );
	}

	return true;
}

int __apply_string_path( wxString& curr_dir, long& camino)
{
	long t = camino;
	size_t camino_len = 0;
	while( t > 0 )
	{
		t = t >> 1;
		camino_len++;
	}

	if( camino_len > curr_dir.Len() )
		return false;

	wxString res;
	bool all_ones = true;
	for( size_t i = 0; i  < curr_dir.Len(); i ++)
	{
		long t = (camino >> i );
		if( t & 1 )
			res += toupper( curr_dir[i] );
		else
		{
			res += tolower( curr_dir[i] );
			all_ones = false;
		}
	}
	camino++;
	curr_dir = res;
	return all_ones;
}

wxString TryRepairObPath( const wxFileName& _fn )
{
	// easy repair
	if( _fn.FileExists() )
		return _fn.GetFullPath();

	wxString _sep = wxFileName::GetPathSeparator();

	wxString _fp = _fn.GetFullPath();
#ifdef OSLINUX
	_fp.Replace( wxT("\\"), wxT("/") );
#endif
	// Try to find the first existing dir
	wxArrayString sub_dirs = StrSplit( _fp, wxFileName::GetPathSeparator() );
	wxString _curr_path;
	int ind_sub_dirs = 0;
	bool res = __RECURS_FindFirsValidPath(_curr_path , sub_dirs, ind_sub_dirs );

	// Nothing in the path exists !!
	if( ! res )
		return wxString();

	// Try to rebuild the path dir after dir
	for( size_t i = ind_sub_dirs; i < sub_dirs.Count(); i++)
	{
		// try every combination of upper and lowercase string with the current dir
		wxString curr_subdir = sub_dirs[i];

		// If the curr_subdir name is too big => abandon repair
		if( curr_subdir.Len() > 32 )
			 return wxString();

		// Build the starting path of construction
		long camino = 0;

		int cont = 0;
		wxString curr_Try;
		while( cont == 0 )
		{
			wxString _temp_ = curr_subdir;
			cont = __apply_string_path( _temp_, camino);
			curr_Try = _curr_path + _sep + _temp_;

			// Dir existence check
			if( i < sub_dirs.Count() -1 )
			{
				_temp_ = curr_Try + _sep;
				wxFileName t(_temp_);
				if( t.DirExists() )
					cont = 2;
			}
			// Final Existence check
			else
				if( wxFileName(curr_Try).FileExists() || wxDirExists(curr_Try + _sep) )
					cont = 2;
		}

		// Not found
		if( cont != 2 )
			return wxString();

		// Found !!
		_curr_path = curr_Try;
	}

	// Repaired !!!
	return _curr_path;
}


bool SimpleBackUpFile( const wxString& filePath )
{
	wxString backup_path = filePath +wxT(".bak");
	return wxCopyFile( filePath, backup_path, true );
}

WX_DECLARE_HASH_MAP( wxString, bool, wxStringHash, wxStringEqual, Hash_File_backedup );
Hash_File_backedup hash_file_backup;

bool myBackUp( const wxString& filePath )
{
	// If already backedup
	if( hash_file_backup[filePath] )
		return true;

	if( SimpleBackUpFile( filePath ) )
	{
		hash_file_backup[filePath] = true;
		return true;
	}
	return false;
}

bool myMove( const wxString& _src, const wxString& _dest )
{
	wxString tempfile = wxFileName::CreateTempFileName( _src );
	if( ! wxRenameFile( _src, tempfile ) )
		return false;
	if( ! wxRenameFile( tempfile, _dest ) )
		return false;
	return true;
}

bool IsFileEmpty( const wxString& _path )
{
	wxFile _t(_path );
	if( ! _t.IsOpened() )
		return true;

	int len = _t.Length();
	_t.Close();
	return ( len == 0);
}

/**\internal*****************
 * Helper Class for repairing paths
 * **************************/
Repair_Set::Repair_Set()
{
}

Repair_Set::~Repair_Set()
{
	while( props.empty() == false )
	{
		struct d_s_i t = props.front();
		delete t.prop_name;
		props.pop_front();
	}
}

void
Repair_Set::Add_Repairable( const wxString& _prop_name, size_t _num_token )
{
	struct d_s_i add;
	add.prop_name = new wxString(_prop_name.Upper());
	add.num_token = _num_token;
	props.push_back( add );
}

// Return count of repaired
size_t
Repair_Set::Try_Repair(ob_object* o )
{
	return RECURS_Repair(o);
}


size_t
Repair_Set::RECURS_Repair(ob_object* o)
{
	size_t res = 0;
	list<struct d_s_i>::iterator it(props.begin()),
					it_end (props.end());
	for(;it!=it_end;it++)
	{
		if( o->name.Upper() == *((*it).prop_name) )
		{
			if( o->nb_tokens > (*it).num_token )
			{
				wxString st_file = GetObFile( o->GetToken( (*it).num_token )).GetFullPath();
				
				if( wxFileExists(st_file) == false && wxDirExists(st_file) == false )
				{
					wxString repaired_path =TryRepairObPath( st_file );
					if( repaired_path != wxString() )
					{
						o->SetToken( (*it).num_token, Convert_To_Ob_Path( repaired_path ));
						res++;
					}
				}
			}
		}
	}
	
	// Recurs in subobjects
	ob_object* t = o->first_subobj;
	while( t != NULL )
	{
		res += RECURS_Repair( t );
		t = t->next;
	}
	
	return res;
}


/**\internal*****************
 * LOGGINGS facilities
 * **************************
 */
myLog *logger;
void __logstderr( wxLogLevel level, const wxChar *msg, time_t timestamp);
void __loggui( wxLogLevel level, const wxChar *msg, time_t timestamp);

#define MAXKEEPEDLOGS  100
int logKeeper_level[MAXKEEPEDLOGS];
wxString logKeeper_sources[MAXKEEPEDLOGS];
wxString logKeeper_message[MAXKEEPEDLOGS];
int next_ind__keepedlog = 0;

void MyLog( int level, const wxString& source, const wxString& message )
{
	if( level == MYLOG_DEBUG )
	{
		#ifdef MYDEBUG
		cout<<"DEBUG : "<<source<<" : "<<message<<endl;
		#endif
		return;
	}
	
	if( startFrame->frame_launched == wxT("EDIT_ENTITIES") )
	{
		// Keep message when the panel errors is not OK
		if( panel_Errors == NULL )
		{
			if( next_ind__keepedlog == MAXKEEPEDLOGS )
				return;
			logKeeper_level[next_ind__keepedlog] = level;
			logKeeper_sources[next_ind__keepedlog] = source;
			logKeeper_message[next_ind__keepedlog] = message;
			next_ind__keepedlog++;
			return;
		}

		// Have keeped message ?
		if( next_ind__keepedlog > 0 )
		{
			for( int i=0; i < next_ind__keepedlog; i ++)
				panel_Errors->Log( logKeeper_level[i], logKeeper_sources[i], logKeeper_message[i] );
			next_ind__keepedlog = 0;
		}
		panel_Errors->Log( level, source, message );
		return;
	}


	// START FRAME LOG
	else
	{
		wxString log_mess = source + wxT(" : ") + message;
		time_t timestamp;
		switch(level)
		{

			// Debug level
			case wxLOG_Debug:
				wxLogInfo( log_mess );
				break;

			// Littles log levels
			case wxLOG_Warning:
			case wxLOG_Message:
				__loggui( level, log_mess, timestamp);
				break;

			default:
				wxLogInfo( log_mess );
				break;
		}
	}
}

void ObjectsLog( int level, int num_line, const wxString& message )
{
	wxString source = curr_entity_filename;
	if( source == wxString() )
		source = wxT("models.txt");

	if( num_line >= 0 )
		source += wxT(" : line ") + IntToStr( num_line );

	MyLog( level, source, message );
}

void EmptyLogKeeper()
{
	next_ind__keepedlog = 0;
}

wxLogStderr* debugLog = NULL;
wxLogWindow* guiLog = NULL;

void myLog::DoLog(wxLogLevel level, const wxChar *msg, time_t timestamp)
{

	switch(level)
	{

		// Debug level
		case wxLOG_Debug:
			__logstderr( level, msg, timestamp);
			break;

		// Littles log levels
		case wxLOG_Warning:
		case wxLOG_Message:
			__loggui( level, msg, timestamp);
			break;

		default:
			__logstderr( level, msg, timestamp);
			break;
	}
};

void __DispatchMsgLog(   wxLogLevel level, const wxChar *msg, time_t timestamp)
{
	switch( level )
	{
		case wxLOG_FatalError:
			wxLogFatalError( msg );
			break;

		case wxLOG_Warning:
			wxLogWarning( msg );
			break;

		case wxLOG_Error:
			wxLogError( msg );
			break;

		case wxLOG_Message:
			wxLogMessage( msg );
			break;

		case wxLOG_Info:
		case wxLOG_Status:
			wxLogStatus( msg );
			break;

		default:
			wxLogMessage( msg );
			break;
	}
}
void __logstderr( wxLogLevel level, const wxChar *msg, time_t timestamp)
{
	if( debugLog == NULL )
		debugLog = new wxLogStderr();
	wxLog::SetActiveTarget( debugLog );
	__DispatchMsgLog( level, msg, timestamp);
	wxLog::SetActiveTarget(logger);
}

void __loggui( wxLogLevel level, const wxChar *msg, time_t timestamp)
{
	if( guiLog == NULL )
		guiLog = new wxLogWindow(startFrame, wxT("Infos"), false, false);
	guiLog->Show();
	wxLog::SetActiveTarget( guiLog );
	__DispatchMsgLog( level, msg, timestamp);
	wxLog::SetActiveTarget(logger);
}


//----------------------------------------------------------------------
wxImage* 
Build_Remap(  wxImage* src   // The target image in wxImage format
		, imgFile* src2  // The target image in imgFile format
		, imgFile* remap_src    // The reamp src  image
		, imgFile* remap_dest	// The reamp dest image
		)
{
	
	if( 	   remap_src->height != remap_dest->height
		|| remap_src->width  != remap_dest->width
		)
		return NULL;
	
	// Loading Palettes
	MyPalette* src_palette = remap_src->GetPalette();
	if( src_palette == NULL || src_palette->IsOk() == false )
	{
		if( src_palette != NULL )
			delete src_palette;
		return NULL;
	}
	int max_ind = src_palette->GetColoursCount();
	if( max_ind <= 0 )
	{
		delete src_palette;
		return NULL;
	}
	MyPalette* t_palette = remap_dest->GetPalette();
	if( t_palette == NULL || t_palette->IsOk() == false )
	{
		if( t_palette != NULL )
			delete t_palette;
		remap_dest->SetPalette( wxPalette(*src_palette) );
	}
	else
	{
		if( t_palette->GetColoursCount() > max_ind )
		{
			delete src_palette;
			max_ind = t_palette->GetColoursCount();
			src_palette = t_palette;
			remap_src->SetPalette( wxPalette(*src_palette) );
		}
		else
			delete t_palette;
	}
	t_palette = src2->GetPalette();
	if( t_palette == NULL || t_palette->IsOk() == false )
	{
		if( t_palette != NULL )
			delete t_palette;
		src2->SetPalette( wxPalette(*src_palette) );
	}
	else
	{
		if( t_palette->GetColoursCount() > max_ind )
		{
			delete src_palette;
			max_ind = t_palette->GetColoursCount();
			src_palette = t_palette;
			remap_src->SetPalette(  wxPalette(*src_palette) );
			remap_dest->SetPalette( wxPalette(*src_palette) );
		}
		else
			delete t_palette;
	}
	
	
	// Init remap array
	int* arr_remaps = new int[max_ind];
	for( int i = 0; i < max_ind; i++ )
		arr_remaps[i] = i;
	
	// Some variables
	int nb_pixels = remap_src->height * remap_src->width;
	int w = remap_src->width;
//	int h = remap_src->height;
	
	// Get the list of remapped colors
	for( int i = 0; i < nb_pixels; i++)
	{
		int s_ind = remap_src->GetIndex( i%w, i/w );
		int d_ind = remap_dest->GetIndex( i%w, i/w );
		if( s_ind >= max_ind || d_ind >= max_ind )
		{
			delete src_palette;
			delete[] arr_remaps;
			return NULL;
		}
		if( s_ind < 0 )
			continue;
		arr_remaps[s_ind] = d_ind;
	}
	

	// Now do the remapping
	wxImage* res = new wxImage(*src);
	res->SetPalette( wxPalette(*src_palette) );
	nb_pixels = res->GetWidth() * res->GetHeight();
	unsigned char* pixs = res->GetData();
	w = src2->width;

	for( int i = 0; i < nb_pixels;i++)
	{
		int ind = src2->GetIndex(i%w,i/w);
		if( (int) ind >= max_ind  )
		{
			delete src_palette;
			delete res;
			delete[] arr_remaps;
			return NULL;
		}
		
		if( arr_remaps[ind] == ind )
			continue;
		
		src_palette->GetRGB(arr_remaps[ind],&pixs[3*i], &pixs[3*i+1], &pixs[3*i+2]);
	}
	
	delete src_palette;
	delete[] arr_remaps;
	return res;
}

inline static
bool ANN_IsNumber( int i ) { return (i >= '0' && i <= '9' ); };

int
P_StrCmp( const wxString& _s0, const wxString& _s1 )
{
	wxString s0 = _s0.Upper();
	wxString s1 = _s1.Upper();
	
	size_t max_ind = min( s0.Len(), s1.Len() );
	
	for( size_t i = 0; i < max_ind; i++ )
	{
		if( 	   ( isalpha(s0[i]) || ANN_IsNumber( s0[i] ) )
			&& ( isalpha(s1[i]) || ANN_IsNumber( s1[i] ) )
		  )
		{
			if( ANN_IsNumber( s0[i] ) && ANN_IsNumber( s1[i] ) )
			{
				wxString s0_number;
				for( size_t j = i; j < s0.Len(); j++ )
				{
					if( ANN_IsNumber( s0[j] ) == true )
						s0_number += s0[j];
					else
						break;
				}
				int s0_number_val = StrToInt( s0_number );
				
				wxString s1_number;
				for( size_t j = i; j < s1.Len(); j++ )
				{
					if( ANN_IsNumber( s1[j] ) == true )
						s1_number += s1[j];
					else
						break;
				}
				int s1_number_val = StrToInt( s1_number );
				
				if( s0_number_val < s1_number_val  )
					return -1;
				else if( s0_number_val > s1_number_val )
					return 1;
				else if( s0_number.Len() < s1_number.Len() )
					return -1;
				else if( s0_number.Len() > s1_number.Len() )
					return 1;
				
				// Exact Same number here => continue
				i += s0_number.Len() -1;
			}
			
			else
			{
				// Number are lower than letters
				if( ANN_IsNumber( s0[i] ) == true )
					return -1;
				else if( ANN_IsNumber( s1[i] ) == true )
					return 1;
				
				// Only letters here
				if( s0[i] < s1[i] )
					return -1;
				else if( s0[i] > s1[i] )
					return 1;
				
				// Exact same letter => continue
			}
		}
		
		else
		{
			// Alphas are greaters than non-alphas
			if( isalpha( s0[i] ) == true )
				return 1;
			else if( isalpha( s1[i] ) == true )
				return -1;
			
			// Two Non alphas
			if( s0[i] < s1[i] )
				return -1;
			else if( s0[i] > s1[i] )
				return 1;
			
			// Exact same non-alpha => continue
		}
	}
	
	// Same prefix => the longest is the greater
	int diff = s0.Len() < s1.Len();
	if( diff < 0 )
		return 1;
	else if( diff > 0 )
		return -1;
	return 0;
}

//----------------------------------------------------------------------
bool
IsEmpty( const wxString& s )
{
	bool res = true;
	for( size_t i = 0 ; i < s.Len(); i++ )
	{
		if( 	   s[i] != ' ' 
			&& s[i] != '\t' 
		  )
		{
			res = false;
			break;
		}
	}
	
	return res;
}

