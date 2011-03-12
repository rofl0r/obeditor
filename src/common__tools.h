#ifndef OB_TOOLS_H_
#define OB_TOOLS_H_

#include <wx/colour.h>
#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/log.h>
#include <wx/hashmap.h>
#include <wx/image.h>
#include <wx/event.h>
#include <wx/valtext.h>

#include "common__globals.h"
#include "images__pngFile.h"
#include "images__gifFile.h"
#include "common__object_file.h"
#include "common__validators.h"

#ifdef MTRACE
#endif


#define t_size_of(t) (int)(sizeof(t)/sizeof(wxString))


/**\internal*****************
 * Miscellanious facilities
 * **************************
 */
bool Frame_RestorePrevCoord( wxWindow* it, const wxString& name );
void Frame_SaveCoord( wxWindow* it, const wxString& name );
void Frame_DestroySavedCoord( const wxString& name );

// Shortcuts Saves/Write config for the EditFrame objects
wxString	ConfigEdit_Read( const wxString& subpath, const wxString& def_val = wxString() );
void 		ConfigEdit_Write( const wxString& subpath, const wxString& val );

void ClearMenu( wxMenu* menu );
void EnableItemsMenu( wxMenu* menu, bool b_enable );


void OpenInExternalEditor( wxWindow* parent, const wxString& filePath );

/**\internal*****************
 * Ob_objects facilities
 * **************************
 */
void Sort_ob_Object_ByName( ob_object**& objs, size_t count );
void Sort_ob_Object_ByTag( ob_object**& objs, size_t count, size_t num_tag = 0 );
void Sort_ob_Object_ByTag( ob_anim**& objs, size_t count, size_t num_tag = 0 );

wxString* Reverse_Null_StrArray( wxString* tokens, size_t tokens_size );

wxArrayString	Tokenize( const wxString& str );
wxString 		UnTokenize( ob_object* _obj );

// ... are <const char*>
void 
	wxArraystring__Add( wxArrayString& arr_str, size_t nb_str, ... );
wxArrayString 
	wxArrayString_Build( size_t nb_str, ... );


/**\internal*****************
 * Conversions facilities
 * **************************
 */
wxString IntToStr( int i );
wxString IntToStr( size_t i );
wxString FloatToStr( const float f);
inline wxString BoolToStr(const bool b ){ return (b ? wxT("1") : wxT("0")); }

int StrToInt( const wxString& str, bool& ok );
int StrToInt( const wxString& str );
unsigned int StrToUInt( const wxString& str, bool& ok );
float StrToFloat(const wxString& str, bool& ok );
float StrToFloat(const wxString& str );

bool IntToBool(const int i );
bool StrToBool( const wxString& str );



/**\internal*****************
 * Strings facilities
 * **************************
 */

bool StrIsObBool( const wxString& str );
bool StrIsInt( const wxString& str );
bool StrIsUInt( const wxString& str );
bool StrIsFloat( const wxString& str );

inline wxArrayString
	Concat_ArrayString( const wxArrayString & arr1, const wxArrayString & arr2 );
wxString 
	ArrStrConcat( const wxArrayString& arrStr, const wxString& separator = wxString() );
wxArrayString 
	StrSplit( const wxString& str, const wxString& delim, bool b_remove_emtpy = true );

wxArrayString 
	ArrayString_PrependAll( const wxArrayString& arrStr, const wxString& _prefix );
wxString	  
	ArrayString_Join( const wxArrayString& arrStr, const wxString& _jointure );

void 	TabStringToArrayString( wxString* tStr,  size_t nb, wxArrayString& arrStr );

bool 	IsInArray(const wxString& _t, wxString* arr, size_t arr_tay );

//  Compare String with taking care of numbers in it
//  Return -1 if s0 < s1 , 0 if s0 == s1, 1 else
int 	P_StrCmp( const wxString& s0, const wxString& s1 );
bool	IsEmpty( const wxString& s );

/**\internal*****************
 * Files facilities
 * **************************
 */

wxFileName 	MyDirAppend( const wxFileName dataDirPath, const wxString& subDir );
wxFileName 	MyPathAppend( const wxFileName dataDirPath, const wxString& subPath );
wxString 	ObPath_To_FullPath( const wxString& _subPath );
wxFileName 	GetObFile( const wxString& _subPath );
wxString 	Convert_To_Ob_Path( const wxString& _path );

wxFileName	GetRessourceFile( const wxString& _strFileName );
wxString	GetRessourceFile_String( const wxString& _strFileName );

wxString 	TryRepairObPath( const wxFileName& _filename );

bool SimpleBackUpFile( const wxString& filePath );
bool myBackUp( const wxString& filePath );
bool myMove(const wxString& _src,const wxString& _dest );

bool IsFileEmpty(const wxString& _path );

class Repair_Set
{
public:
	Repair_Set();
	~Repair_Set();
	void Add_Repairable( const wxString& _prop_name, size_t _num_token );
	size_t Try_Repair(ob_object* o );
	struct d_s_i
	{
		wxString* prop_name;
		size_t num_token;
	};
	
protected:
	std::list<struct d_s_i> props;
	size_t RECURS_Repair(ob_object* o);
};



/**\internal*****************
 * Images facilities
 * **************************
 */
// The NoImage image
wxColour MeltColor(wxColour _c0, wxColour _c1 );


// Tool class to save some code reusing when an object hold an image
class Image_Holder
{
public:
	Image_Holder();
	virtual ~Image_Holder();

	virtual wxFileName GetFileName() = 0;

	wxImage* GetImage();
	imgFile* Get_GIF_or_PNG();

	void     RemoveImages();
	int 	 Get_ColorIndex( const int x, const int y );

protected:
	wxImage* img;
	imgFile* png_or_gif;

};

wxImage* 
Build_Remap(  wxImage* src   // The target image in wxImage format
		, imgFile* src2  // The target image in imgFile format
		, imgFile* remap_src    // The reamp src  image
		, imgFile* remap_dest	// The reamp dest image
		);
wxString wxColour_ToStr( const wxColour& c );
wxColour wxColour_FromStr( const wxString& s );


void Image_Rescale_To_Fit( wxImage& img, size_t w, size_t h );

/**\internal*****************
 * LOGGINGS facilities
 * **************************
 */
// LOG LEVELS
enum {MYLOG_ERROR = 0, MYLOG_WARNING = 1, MYLOG_INFO = 2, MYLOG_DEBUG = 3 };

// wxWindow log
extern wxLogWindow* guiLog;

//const int max_log_level = wxLOG_Debug;

void MyLog( int level, const wxString& source, const wxString& message );
void ObjectsLog( int level, int num_line, const wxString& message );
void EmptyLogKeeper();

class myLog : public wxLog
{
	public:
		virtual void DoLog(wxLogLevel level, const wxChar *msg, time_t timestamp);
};
extern myLog *logger;



/**\internal******************************
 * Validations facilities
 * 		Note : implemented in parsing.cpp
 * ***************************************
 */
#define _V int*

class ob_validator
{
public:
	ob_validator();
	~ob_validator();
	ob_validator( _V* _arr_glob_constraint );
	void Validate( const ob_object* it, wxString* tokens, const size_t tokens_size );

	_V* arr_glob_constraint;
};


WX_DECLARE_HASH_MAP( wxString, ob_validator*, wxStringHash, wxStringEqual, hash_constraints );
extern hash_constraints entity_constraints;
extern hash_constraints models_constraints;
extern hash_constraints levels_constraints;


#endif /*OB_TOOLS_H_*/
