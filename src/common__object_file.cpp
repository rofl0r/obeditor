#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/textfile.h>
#include <wx/progdlg.h>
#include "common__tools.h"
#include "common__object_stage.h"

using namespace std;

//----------------------------------------------------------------------------
wxFileName dataDirPath;
obFileModels *fileModels;
obFileLevels *fileLevels;

//****************************************************************************
//****************************************************************************

MyToken::MyToken()
{
	prefix =NULL;
	data=NULL;
}

MyToken::MyToken( const wxString& _prefix, const wxString& _data)
{
	prefix = NULL;
	if( _prefix != wxString() )
		prefix = new wxString( _prefix );
	data = NULL;
	if( _data != wxString() )
		data = new wxString( _data );
}

MyToken::MyToken(const MyToken& src)
{
	prefix = NULL;
	if( src.prefix != NULL )
		prefix = new wxString( *(src.prefix));

	data = NULL;
	if( src.data != NULL )
		data = new wxString( *(src.data));
}


MyToken& MyToken::operator=(const MyToken& src)
{
	if( &src == this )
		return *this;

	if( prefix != NULL )
	{
		delete prefix;
		prefix = NULL;
	}
	if( src.prefix != NULL )
		this->prefix = new wxString( *(src.prefix));

	if( data != NULL )
	{
		delete data;
		data = NULL;
	}
	if( src.data != NULL )
		this->data = new wxString( *(src.data));
	return *this;
}

MyToken::~MyToken()
{
	if( prefix != NULL )
		delete prefix;
	if( data != NULL )
		delete data;
}

wxString MyToken::Prefix()const
{
	if( prefix == NULL )
		return wxString();
	return *prefix;
}


wxString MyToken::ToStr()const
{
	wxString *res = new wxString();
	if( prefix != NULL )
		*res += *(prefix );
	if( data != NULL )
		*res += *(data);
	return *res;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

MyLine::MyLine(const wxString& raw_MyLine)
{
	comment = NULL;
	arr_tokens = NULL;
	nb_tokens = 0;
	arr_tokens_size = 0;
	Tokenize( raw_MyLine, arr_tokens, nb_tokens, arr_tokens_size, comment );
}
MyLine::MyLine()
{
	comment = NULL;
	arr_tokens = NULL;
	nb_tokens = 0;
	arr_tokens_size = 0;
}

MyLine::~MyLine()
{
	Reset();
}

void MyLine::Reset()
{
	if( arr_tokens != NULL )
	{
		delete[] arr_tokens;
		arr_tokens = NULL;
	}
	if( comment != NULL )
	{
		delete comment;
		comment = NULL;
	}
}


MyToken* MyLine::operator[](const size_t i)
{
	if( i>= nb_tokens || i < 0)
		return NULL;
	return 	&(arr_tokens[i]);
}

MyToken* MyLine::GetToken(const size_t i)
{
	if( i>= nb_tokens || i < 0)
		return NULL;
	return 	&(arr_tokens[i]);
}

wxString* MyLine::GetDatas()const
{
	wxString *res = new wxString[nb_tokens];
	for( size_t i =0; i< nb_tokens; i++)
	{
		if( arr_tokens[i].data ==NULL )
			res[i] = wxString();
		else
			res[i] = *(arr_tokens[i].data);
	}
	return res;
}

wxString MyLine::ToStr()const
{
	wxString *res = new wxString;
	for( size_t i =0; i< nb_tokens; i++ )
			*res += arr_tokens[i].ToStr();

	if( comment != NULL )
		*res += *comment;
	return *res;
}

bool MyLine::Set( const wxString& raw_MyLine )
{
	Reset();
	Tokenize( raw_MyLine, arr_tokens, nb_tokens, arr_tokens_size, comment );
	return true;
}



size_t MyLine::GetTokensCount()const
{
	return nb_tokens;
}

wxString MyLine::Comment()const
{
	if( comment == NULL )
		return wxString();
	return *comment;
}

bool MyLine::IsEmpty()const
{
	if( nb_tokens == 0 && comment == NULL)
		return true;
	return false;
}

bool MyLine::IsComment()const
{
	if( nb_tokens == 0 && comment != NULL)
		return true;
	return false;
}

wxString MyLine::Tag()const
{
	if( nb_tokens > 0 )
		return *arr_tokens[0].data;
	return wxString();
}


/**
 * \internal
 */

size_t pad_tokens = 8;
void expand_arr_tokens( MyToken*& arr_tokens, size_t& old_size)
 {
 	MyToken* res = new MyToken[old_size+pad_tokens];
 	for( size_t i=0; i< old_size; i++)
 		res[i] = arr_tokens[i];
 	old_size += pad_tokens;
 	MyToken* temp = arr_tokens;
 	arr_tokens = res;
 	delete[] temp;
 }

void MyLine::AppendToken( const wxString& _prefix, const wxString& _data )
{
	MyToken temp( _prefix, _data);
	if( nb_tokens == arr_tokens_size)
		expand_arr_tokens( arr_tokens, arr_tokens_size);
	arr_tokens[nb_tokens] = temp;
	nb_tokens++;
}

/**
 * Tool for MyLine class that split the MyLine in token and comment
 */
void MyLine::Tokenize( const wxString& raw_MyLine, MyToken*& arr_tokens, size_t& nb_tokens, size_t& arr_tokens_size, wxString*& comment )
 {
	Reset();
	if( raw_MyLine.Len() == 0 )
	{
		return;
	}

 	arr_tokens_size = pad_tokens;
 	arr_tokens = new MyToken[arr_tokens_size];
 	nb_tokens = 0;

 	wxString prefix;
 	wxString data;
 	int start=0;
 	int bascule = 0;
 	for( size_t i=0; i< raw_MyLine.Len(); i++ )
 	{
 		bool white = ( raw_MyLine[i] == wxT(' ') || raw_MyLine[i] == wxT('\t'));
		if( ! white && bascule==0 && raw_MyLine[i] != '#')
		{
			prefix = raw_MyLine.Mid(start, i-start);
			start = i;
			bascule = 1;
		}

		else if(bascule == 1 && white)
		{
			data = raw_MyLine.Mid(start, i-start);
			AppendToken( prefix, data );
			prefix = wxString();
			start = i;
			bascule = 0;
		}
		else if( raw_MyLine[i] == '#' )
		{
			if( bascule == 1)
			{
				data = raw_MyLine.Mid(start, i-start);
				AppendToken( prefix, data );
				prefix=wxString();
			}
			else
				prefix = raw_MyLine.Mid(start, i-start);

			comment = new wxString( prefix + raw_MyLine.Right( raw_MyLine.Len()-i ) );
			start=-1;
			break;
		}
 	}

	size_t i = raw_MyLine.Len() -1;
	data = raw_MyLine.Mid(start, i -start +1);

	// No comments found
	if( start != -1 && data.Len() > 0)
	{
		// Treatment of the last element
		bool white = ( raw_MyLine[raw_MyLine.Len() -1] == wxT(' ') || raw_MyLine[i] == wxT('\t'));
		if( ! white )
		{
			AppendToken( prefix, data );
			comment = NULL;
		}
		else
		// Trailing white spaces -> to trash
		{
			//comment = new wxString(data);
		}
	}


 	// empty the array in case
 	if( nb_tokens == 0 )
 	{
 		delete[] arr_tokens;
 		arr_tokens = NULL;
 		arr_tokens_size=0;
		nb_tokens=0;
 	}
}

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

size_t pad_nb_lines = 10;
obFile::obFile()
{
	ZeroInit();
}

obFile::obFile( const wxString& str_fullpath, bool b_init )
{
	ZeroInit();
	filename = str_fullpath;
	if( b_init )
		Reload();
}

obFile::obFile( const wxFileName _filename, bool b_init )
{
	ZeroInit();
	filename = _filename;
	if( b_init )
		Reload();
}

obFile::obFile( const wxTextFile& file, bool b_init )
{
	ZeroInit();
	filename = file.GetName();
	if( b_init )
		Reload();
}

obFile::~obFile()
{
	Reset();
}

/**
 * Reset everything, considering nothing has been allocated
 */
void obFile::ZeroInit()
{
	nb_lines = 0;
	alloced_lines = 0;
	lines = NULL;
	filename.Assign(wxString());
	textFile = NULL;
	obj_container = NULL;
	changed = 0;
}

/**
 * Reset everything, considering every pointer valid
 */
void obFile::Reset()
{
	nb_lines = 0;
	alloced_lines = 0;
	if( lines != NULL )
		delete[] lines;
	lines = NULL;
	filename.Assign(wxString());
	if( textFile != NULL )
	{
		textFile->Close();
		delete textFile;
	}
	textFile = NULL;
	if( obj_container != NULL)
		obj_container->Rm();
	obj_container = NULL;
	changed = 0;
}

//--------------------------------------------------------------
void 
obFile::SetChanged()
{
	if( this != NULL && changed == 0 )
		changed = 1;
}


//--------------------------------------------------------------
void 
obFile::Reload()
{
	wxFileName t_filename = filename;

	Reset();

	filename = t_filename;
	obj_container = NewContainer();

	OpenFile();
	LoadLines();
	if( textFile == NULL)
		ZeroInit();
	else
		textFile->GuessType();

	BuildObjects();
}

//--------------------------------------------------------------
ob_object_container* 
obFile::NewContainer()
{
	return new ob_object_container();
}


/**
 * * Open the TextFile from fileName
 */
bool obFile::OpenFile()
{
	if( !filename.FileExists() )
	{
		ZeroInit();
		return false;
	}
	textFile = new wxTextFile( filename.GetFullPath() );
	if( ! textFile->Open() )
	{
		delete textFile;
		textFile = NULL;
		return false;
	}
	return true;
}

bool obFile::Write()
{
	if( obj_container == NULL )
	{
		MyLog( MYLOG_ERROR, filename.GetFullName(), wxT("This file has not been loaded !!") );
		return false;
	}

	myBackUp( filename.GetFullPath() );

	textFile->Clear();

	// Open a temp file
	wxString tmp_name = wxFileName::CreateTempFileName( wxT("ob_edit_"));
	wxTextFile tt( tmp_name );
	tt.Create();

	wxArrayString __lines = StrSplit( obj_container->ToStr(), wxT("\n"), false );
	for( size_t i = 0; i < __lines.Count(); i ++ )
	{
		textFile->AddLine( __lines[i] );
		tt.AddLine( __lines[i] );
	}

	if( ! textFile->Write() )
	{
		if( tt.Write() )
		{
			tt.Close();
			return wxCopyFile( tmp_name, filename.GetFullName(), true );
		}
		MyLog( MYLOG_ERROR, filename.GetFullName(), wxT("Unable to write the file !!") );
		return false;
	}

	changed = false;
	obj_container->SetEdited( false );
	return true;
}


wxString obFile::ToStr()
{
	return obj_container->ToStr();
}

/**
 * * Load the lines from the Opened TextFile
 */
bool obFile::LoadLines()
{
	if( textFile == NULL )
		return false;

	if( ! textFile->IsOpened() )
		return false;

	// Reset and alloc array of lines
	if( lines != NULL )
	{
		delete lines;
	}
	alloced_lines = textFile->GetLineCount() + 1;
	lines = new MyLine[alloced_lines];

	wxString currLine;
	size_t i=0;
	for ( currLine = textFile->GetFirstLine(); !textFile->Eof(); currLine = textFile->GetNextLine() )
	{
	    lines[i].Set( currLine );
	    nb_lines++;
	    i++;
	}
	//last line in textFile
    	lines[i].Set( currLine );

	return true;
}


//------------------------------------------------------------
ob_object* 
obFile::GetProperty(const wxString& _name )
{
	if( obj_container == NULL )
		return NULL;
	return obj_container->GetSubObject(_name);
}


//************************************************************

ob_object** obFile::GetSubObjectS(const wxString& _name, size_t& count )
{
	count = 0;
	if( obj_container == NULL )
		return NULL;
	return obj_container->GetSubObjectS( _name, count );
}


//************************************************************

void obFile::AllocPrefixsLines()
{
	if( obj_container == NULL )
		return;
	obj_container->EatPrefixLines();
}


//************************************************************

void obFile::BuildObjects()
{
	ParseFile();
	Cleaning();
}


//************************************************************

void obFile::ParseFile()
{
	if( nb_lines <= 0 )
		return;

	ob_object *last_obj = Guess_and_ConstructNewObject( &lines[0],0 );

	if ( last_obj != NULL )
		obj_container->Add_SubObj( last_obj );

	for( int i=1; i < nb_lines; i++ )
	{
		// Error or else? with previous object
		if( last_obj == NULL )
		{
			last_obj = Guess_and_ConstructNewObject( &lines[i],i);
			if ( last_obj != NULL )
				obj_container->Add_SubObj( last_obj );
		}

		// there if the previous object eat the line
		else if( last_obj->Eat( &lines[i],i) )
		{
			// No validation cause the eater is reponsible of it
			continue;
		}
		// basic situation
		else
		{
			last_obj = Guess_and_ConstructNewObject( &lines[i],i);
			if ( last_obj != NULL )
				obj_container->Add_SubObj( last_obj );
		}

		Validation( &lines[i], i, last_obj );
	}
}


//************************************************************

void obFile::Validation(MyLine* line, const int num_line, ob_object*& last_obj )
{
}


//************************************************************
// Alloc comments and empty lines to the corrects objects

void obFile::Cleaning()
{
	if( obj_container == NULL )
		return;

	size_t nb_obj;
	ob_object** _objs = obj_container->GetAllSubObjS( nb_obj );

	// Unlink tailings meaningless lines
	for( size_t i=0; i < nb_obj; i++)
	{
		_objs[i]->ThrowTailings_LineOrComments();
	}

	// Allocate prefixs lines
	AllocPrefixsLines();

	if( _objs != NULL )
		delete[] _objs;

	// Remove Empty Trailing lines
	while( 1 )
	{
		ob_object* temp = obj_container->last_subobj;
		if( temp->Is_EmptyOrComment() == true )
		{
			temp->Rm();
		}
		else 
			break;
	}
		
	return;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************


obFileEntity::obFileEntity( const wxString& str_filename)
:obFile( str_filename, false ), avatarImg(NULL)
{
	Reload();
}


//************************************************************

obFileEntity::obFileEntity( const wxFileName fullpath)
:obFile( fullpath, false ), avatarImg(NULL)
{
	Reload();
}


//************************************************************

obFileEntity::obFileEntity( const wxTextFile& file)
:obFile( file, false ), avatarImg(NULL)
{
	Reload();
}

//------------------------------------------------------------
ob_object_container* 
obFileEntity::NewContainer()
{
	return new ob_entity_container();
}



//************************************************************

obFileEntity::~obFileEntity()
{
	if( avatarImg != NULL)
		delete avatarImg;
}


//************************************************************

void obFileEntity::obFileEntity::Reset()
{
	obFile::Reset();
	if( avatarImg != NULL)
		delete avatarImg;
	avatarImg = NULL;
	avatar_img_path = wxString();
	avatar_img_path_offset = wxSize(0,0);
}


//----------------------------------------------------------
void 
obFileEntity::AddProperty( ob_object* new_prop )
{
	ob_object* temp = GetFirstAnim();
	if( temp == NULL )
	{
		obj_container->Add_SubObj( new_prop );
		return;
	}

	temp->InsertObject_Before( new_prop );
}


//************************************************************

ob_anim* obFileEntity::GetFirstAnim()
{
	ob_object* temp = obj_container->first_subobj;
	while( temp != NULL )
	{
		if( temp->type == OB_TYPE_ANIM )
			return (ob_anim*) temp;
		temp = temp->next;
	}

	// No anim found
	return NULL;
}


//************************************************************
ob_anim** 
obFileEntity::GetAnims(size_t& nb_anims )
{
	return (ob_anim**)  GetSubObjectS( wxT("anim"), nb_anims );
}


//************************************************************

wxString obFileEntity::Name()
{
	ob_object *_name = GetProperty(wxT("name"));
	if( _name == NULL )
		return wxString();

	return _name->GetToken(0);
}


//************************************************************

ob_object* obFileEntity::Guess_and_ConstructNewObject(MyLine* line, const int num_line )
{
	return ob_entity::Guess_and_ConstructNewObject( line, num_line);
}


//************************************************************

void obFileEntity::Validation(MyLine* line, const int i, ob_object*& last_obj )
{
	// Construction errors
	if( last_obj == NULL )
		MyLog( MYLOG_ERROR, filename.GetFullName() + wxT(" line ") +  IntToStr(i+ 1),
		       last_obj->name + wxT(" : obFileEntity::BuildObject : ob_entity build is NULL") );

	// FrameTag outside of a tag (ie: have to be eat and had not)
	else if( ob_frame::IsFrameTag( last_obj->name ))
	{
		MyLog( MYLOG_WARNING, filename.GetFullName() + wxT(" line ") +  IntToStr(i+ 1),
		       last_obj->name + wxT(" : Frame related Tag outside of an anim zone !!") );
		//Create a dummy anim  object in case of an error
		ob_anim *t2 = new ob_anim();
		t2->dummy = true;
		t2->SetName( wxT("ANIM_DUMMY") );
		t2->num_line = i;
		t2->Eat( &lines[i],i );

		last_obj->Rm();
		last_obj = t2;
		obj_container->Add_SubObj( last_obj );
	}

	// AnimTag outside of a tag (ie: have to be eat and had not)
	else if( ob_anim::IsAnimTag( last_obj->name ) &&
		last_obj->name.Upper() != wxT("ANIM") && last_obj->name.Upper() !=wxT("ANIMATION"))
	{
		MyLog( MYLOG_WARNING, filename.GetFullName() + wxT(" line ") +  IntToStr(i+ 1),
		       last_obj->name + wxT(" : Anim related Tag outside of an anim zone !!"));

		//Create a dummy anim object in case of an error
		ob_anim *temp = new ob_anim();
		temp->SetName( wxT("ANIM_DUMMY") );
		temp->Add_SubObj(last_obj);
		temp->num_line = i;
		temp->dummy = true;
		last_obj = temp;
		obj_container->Add_SubObj( last_obj );
	}
	/*
	// FrameTag outside of a tag (ie: have to be eat and had not)
	else if( ob_frame::IsFrameTag( objs[objs_size]->name ))
		wxLogInfo( "At line " + IntToStr( i+1) + " with " + objs[objs_size]->name +
					", an frame related Tag outside of an frame zone !!" );
	*/
}


//************************************************************

void obFileEntity::Cleaning()
{
	// Algorithme to repair breaking constructs
//	ob_anim* prev_anim =NULL;

	if( obj_container == NULL )
		return;
	
	ob_object *temp = obj_container->first_subobj;
	while( temp != NULL )
	{
		// Try to repair breakings anim
		if( temp->dummy )
		{
			// merge this and all object before with previous anim
			while( temp->prev != NULL )
			{
				ob_object *_prev = temp->prev;
				if( _prev->type == OB_TYPE_ANIM && !_prev->dummy )
				{
					_prev->Absorb( temp );
					temp->dummy = false;
					temp = _prev;
					break;
				}
				temp->Absorb( _prev, false );
			}
		}
		temp = temp->next;
	}


	// Alloc comments and empty lines to the corrects objects
	size_t nb_anims;
	ob_anim** anims = GetAnims(nb_anims);

		// Unlink tailings meaningless lines of anims
		for( size_t i=0; i < nb_anims; i++)
		{
			// throw tailing meaningless lines of dummy frames
			ob_object *t = anims[i]->last_subobj;
			if( t != NULL && t->type == OB_TYPE_FRAME && t->dummy )
				t->ThrowTailings_LineOrComments();

			// thrown tailing meaningless lines
			anims[i]->ThrowTailings_LineOrComments();
		}

		// Allocate prefixs lines
		AllocPrefixsLines();


	//---------
	// Advanced checkings
	//---------

	// Anims must be uniques
	for( size_t i=0; i < nb_anims; i++)
	{
		for( size_t j=i+1; j < nb_anims; j++)
		{
			if( anims[j]->animID().Upper() == anims[i]->animID().Upper() )
			{
				MyLog( MYLOG_WARNING, filename.GetFullName() + wxT(" line ") +  anims[j]->GetLineRef_S(),
				       wxT("Redefinition of anim ") + anims[j]->animID() +
				       wxT(" previously defined at line ") + anims[i]->GetLineRef_S());
			    break;
			}
		}
	}

	//Anim must contains frames
	for( size_t i=0; i < nb_anims; i++)
	{
		if( anims[i]->GetSubObject(wxT("frame")) == NULL )
			MyLog( MYLOG_WARNING, filename.GetFullName() + wxT(" line ") +  anims[i]->GetLineRef_S(),
			       anims[i]->name + wxT(" : Anim contain no frames !!"));
	}

	// Inevitable anims
	// FIXME

	// Inevitable properties
	// FIXME

	if( anims != NULL )
		delete[] anims;

	// Remove Empty Trailing lines
	while( 1 )
	{
		ob_object* temp = obj_container->last_subobj;
		if( temp->Is_EmptyOrComment() == true )
		{
			temp->Rm();
		}
		else 
			break;
	}
		
	return;
}


//************************************************************
wxString
obFileEntity::Get_AvatarImg_Path()
{
	if( avatar_img_path != wxString() )
	{
		if( avatar_img_path == wxT("UNFOUND") )
			return wxString();
		return avatar_img_path;
	}
	
	avatar_img_path = wxT("UNFOUND");

	//Try to get the anim IDLE
	size_t nb_anims;
	ob_anim** arr_anims = 
		(ob_anim**) obj_container->GetSubObjectS_ofType( OB_TYPE_ANIM, nb_anims );
	if( arr_anims == NULL )
		return wxString();
	
	list<ob_anim*> to_try;
	for( size_t i = 0; i < nb_anims; i++ )
	{
		// IDLE Always at the start of the preference list
		if( arr_anims[i]->GetToken(0).Upper() == wxT("IDLE") )
			to_try.push_front( arr_anims[i] );
		
		// Must place WALK after IDLE in preference list
		else if( arr_anims[i]->GetToken(0).Upper() == wxT("WALK") )
		{
			if( 	   to_try.empty() == true 
				|| to_try.front()->GetToken(0).Upper() != wxT("IDLE")
				)
				to_try.push_front( arr_anims[i] );
			else
			{
				ob_anim* t = to_try.front();
				to_try.pop_front();
				to_try.push_front( arr_anims[i] );
				to_try.push_front( t );
			}
		}
		else
			to_try.push_back( arr_anims[i] );
	}
	if( arr_anims != NULL )
		delete[] arr_anims;

	// Try to find a valid image from all these anims
	ob_frame* theFrame = NULL;
	while( to_try.empty() == false)
	{
		ob_anim* a = to_try.front();
		to_try.pop_front();
		
		// Get all frames for this anim
		size_t nb_frames;
		ob_frame** arr_frames = a->GetFrames( nb_frames );
		
		for( size_t i = 0; i < nb_frames; i++)
		{
			wxImage* img = arr_frames[i]->GetImage();
			if( img != NULL )
			{
				theFrame = arr_frames[i];
				break;
			}
		}
		if( arr_frames != NULL )
			delete[] arr_frames;
		if( theFrame != NULL )
			break;
	}
	
	if( theFrame == NULL )
		return wxString();
	
	avatar_img_path = theFrame->GifPath().GetFullPath();
	ob_object* _off = theFrame->GetSubObject( wxT("offset") );
	if( _off != NULL )
		avatar_img_path_offset = wxSize( StrToInt(_off->GetToken(0))
							, StrToInt(_off->GetToken(1)) );
	return avatar_img_path;
}

//------------------------------------------------------------------
wxImage* 
obFileEntity::Load_AvatarImg()
{
	if( avatarImg != NULL )
		return avatarImg;
	if( Get_AvatarImg_Path() == wxString())
		return NULL;
	
	avatarImg = new wxImage( Get_AvatarImg_Path() );
	if( ! avatarImg->IsOk())
	{
		delete avatarImg;
		avatarImg = NULL;
		return NULL;
	}

	return avatarImg;
}


//************************************************************

void obFileEntity::PurgeImages()
{
	if( obj_container == NULL )
		return;

	size_t nb_anims;
	ob_anim** _anims = (ob_anim**)
			obj_container->GetSubObjectS_ofType( OB_TYPE_ANIM, nb_anims );

	if( _anims == NULL )
		return;

	for( size_t i = 0; i< nb_anims; i++ )
	{
		size_t nb_frame;
		ob_frame** _frames = _anims[i]->GetFrames(nb_frame );
		if( _frames == NULL )
			continue;

		for( size_t j = 0; j < nb_frame; j++ )
			_frames[j]->RemoveImage();

		if( _frames != NULL )
			delete[] _frames;
	}
	if( _anims != NULL )
		delete[] _anims;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//	OBFILEMODELS
//****************************************************************************

obFileModels::obFileModels( const wxString& str_filename)
:obFile( str_filename, false )
{
	Reload();
}


obFileModels::obFileModels( const wxFileName fullpath)
:obFile( fullpath, false )
{
	Reload();
}


obFileModels::obFileModels( const wxTextFile& file)
:obFile( file, false )
{
	Reload();
}


obFileModels::~obFileModels()
{
}

ob_models** obFileModels::GetModels(int& count)
{
	size_t nb_load, nb_know;
	ob_models** arr_loads = (ob_models**) GetSubObjectS(wxT("Load"), nb_load );
	ob_models** arr_knows = (ob_models**) GetSubObjectS(wxT("Know"), nb_know );
	
	count = nb_load + nb_know;
	if( count == 0 )
		return NULL;
	
	ob_models** res = new ob_models*[count];
	
	for( size_t i =0 ; i < nb_load; i++)
		res[i] = arr_loads[i];
	if( arr_loads != NULL )
		delete[] arr_loads;

	for( size_t i =0 ; i < nb_know; i++)
		res[ nb_load + i ] = arr_knows[i];
	if( arr_knows != NULL )
		delete[] arr_knows;

	return res;
}

ob_models* obFileModels::GetModel( const wxString& model_name )
{
	int count;
	ob_models** arr = obFileModels::GetModels(count);
	if( count <= 0 )
		return NULL;

	for( int i=0; i < count; i++)
	{
		if( arr[i]->GetToken(0).Upper() == model_name.Upper() )
		{
			ob_models* res = arr[i];
			delete[] arr;
			return res;
		}
	}

	if( arr != NULL )
		delete[] arr;
	return NULL;
}


//************************************************************

void obFileModels::ParseFile()
{
	if( nb_lines <= 0 )
		return;

	for( int i=0; i < nb_lines; i++ )
		obj_container->Add_SubObj( new ob_models( &lines[i],i) );

}

//************************************************************

ob_object* obFileModels::Guess_and_ConstructNewObject(MyLine* line, const int num_line )
{
	return new ob_object( line, num_line);
}



//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//	obFileLevels
//****************************************************************************


obFileLevels::obFileLevels( const wxString& str_filename )
:obFile( str_filename, false )
{
	Reload();
}


//****************************************************************************

obFileLevels::obFileLevels( const wxFileName fullpath)
:obFile( fullpath, false )
{
	Reload();
}


//****************************************************************************

obFileLevels::obFileLevels( const wxTextFile& file)
:obFile( file, false )
{
	Reload();
}


//****************************************************************************

obFileLevels::~obFileLevels()
{
}


//****************************************************************************

ob_object* obFileLevels::Guess_and_ConstructNewObject(MyLine* line, const int num_line )
{
	return ob_levels::Guess_and_ConstructNewObject( line, num_line);
}


//----------------------------------------------------------------------------
ob_object_container* 
obFileLevels::NewContainer()
{
	return new ob_levels();
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//	obFileStage
//****************************************************************************


//****************************************************************************

obFileStage::obFileStage( const wxString& str_filename )
:obFile( str_filename, false )
{
	Reload();
}


//****************************************************************************

obFileStage::obFileStage( const wxFileName fullpath)
:obFile( fullpath, false )
{
	Reload();
}


//****************************************************************************

obFileStage::obFileStage( const wxTextFile& file)
:obFile( file, false )
{
	Reload();
}


//****************************************************************************

obFileStage::~obFileStage()
{
}


//****************************************************************************

ob_object* obFileStage::Guess_and_ConstructNewObject(MyLine* line, const int num_line )
{
	return ob_stage::Guess_and_ConstructNewObject( line, num_line);
}


//----------------------------------------------------------------------------
ob_object_container* 
obFileStage::NewContainer()
{
	return new ob_stage();
}


//****************************************************************************
// Hash that maintain the relation :  EntityName <-> Object Loaded

WX_DECLARE_HASH_MAP( wxString, obFileEntity*, wxStringHash, wxStringEqual, hash_entName_entObjFile_bis );
hash_entName_entObjFile_bis hash_name_to_entity;


obFileEntity* 
obFileStage::GetEntity( const wxString& ent_name )
{
	obFileEntity* _ent_file = hash_name_to_entity[ent_name];
	if( _ent_file != NULL )
		return _ent_file;

	// Check if the entity exist in models.txt
	ob_object* _ent = fileModels->GetModel( ent_name );
	if( _ent == NULL )
			return NULL;

	// Build Entity path
	wxString _subpath = _ent->GetToken(1);
	wxString entity_filename = GetObFile( _subpath).GetFullName();
	wxFileName _ent_path = GetObFile( _subpath);

	if( ! _ent_path.FileExists() )
	{
		wxLogInfo( wxT("Entity File doesn't exist <") + entity_filename + wxT(">") );
		return NULL;
	}

	else
	{
		_ent_file = new obFileEntity( _ent_path );
		if( _ent_file->nb_lines <= 0 )
		{
			delete _ent_file;
			_ent_file = NULL;
			wxLogInfo( wxT("File is empty or something <") + entity_filename +wxT( ">") );
			return NULL;
		}
	}

	 hash_name_to_entity[ent_name] = _ent_file;
	 return _ent_file;
}

//---------------------------------------------------------------------
list<obFileEntity* >*
obFileStage::GetEntity_ALL(wxWindow* busy_parent )
{
	list<obFileEntity* >* res = new list<obFileEntity* >;
	
	if( fileModels == NULL )
		return res;
	
	int l_models__count;
	ob_models** l_models = fileModels->GetModels(l_models__count);
	
	if( l_models__count <= 0 )
		return res;
	
	wxProgressDialog *wProg = NULL;
	if(busy_parent!= NULL)
		wProg = new wxProgressDialog(wxT("Just a moment..."), wxT("Loading Entities..."), l_models__count, busy_parent);
	for( int i = 0; i < l_models__count; i++ )
	{
		if( l_models[i] == NULL )
			continue;
		if( l_models[i]->GetToken(0) == wxString() )
			continue;
		
		obFileEntity* _ent = obFileStage::GetEntity( l_models[i]->GetToken(0) );
		if( _ent == NULL )
			continue;
		
		if( wProg != NULL )
			wProg->Update(i+1, wxT("Loading Entities...\n")
			+IntToStr(i+1)+ wxT(" over ") + IntToStr(l_models__count)
				);
		res->push_back( _ent );
	}
	if( wProg != NULL )
		wProg->Destroy();
	
	if( l_models != NULL )
		delete[] l_models;
	return res;
}

//****************************************************************************

void obFileStage::ClearEntitiesCache()
{
	size_t count = hash_name_to_entity.size();
	hash_entName_entObjFile_bis::iterator i = hash_name_to_entity.begin();
	for( size_t j = 0; j< count; j++)
	{
		obFileEntity *temp = i->second;
		delete temp;
		i++;
	}
	hash_name_to_entity.clear();
}


//************************************************************

void obFileStage::Cleaning()
{
	obFile::Cleaning();

	if( obj_container == NULL )
		return;
	
	// Algorithme to repair breaking constructs

	ob_object *temp = obj_container->first_subobj;
	while( temp != NULL )
	{
		// Try to repair breakings objects
		ob_spawn_healthy dummy;
		if( dummy.Is_AcceptedTag( temp->name ) )
		{
			// merge this and all object before with previous spawn object
			ob_object* t = temp->prev;
			ob_object* n = temp->prev;
			while( t != NULL )
			{
				if( t->type >= OB_TYPE_STAGE_AT && t->type <= OB_TYPE_SPAWN_ITEM )
				{
					t->Add_SubObj( temp );
					temp = n;
					break;
				}
				t = t->prev;
			}
		}
		temp = temp->next;
	}

	// Remove Empty Trailing lines
	while( 1 )
	{
		ob_object* temp = obj_container->last_subobj;
		if( temp->Is_EmptyOrComment() == true )
		{
			temp->Rm();
		}
		else 
			break;
	}
		

	return;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//	obFileVideoTxt
//****************************************************************************


obFileVideoTxt::obFileVideoTxt( const wxString& str_filename )
:obFile( str_filename, false )
{
	Reload();
}


//****************************************************************************

obFileVideoTxt::obFileVideoTxt( const wxFileName fullpath)
:obFile( fullpath, false )
{
	Reload();
}


//****************************************************************************

obFileVideoTxt::obFileVideoTxt( const wxTextFile& file)
:obFile( file, false )
{
	Reload();
}


//****************************************************************************

obFileVideoTxt::~obFileVideoTxt()
{
}


//****************************************************************************
ob_object* 
obFileVideoTxt::Guess_and_ConstructNewObject(MyLine* line, const int num_line )
{
	return new ob_object( line, num_line);
}

//----------------------------------------------------------------------------
void 
obFileVideoTxt::ParseFile()
{
	if( nb_lines <= 0 )
		return;

	ob_object *last_obj = Guess_and_ConstructNewObject( &lines[0],0 );

	if ( last_obj != NULL )
		obj_container->Add_SubObj( last_obj );

	for( int i=1; i < nb_lines; i++ )
	{
		// Error or else? with previous object
		if( last_obj == NULL )
		{
			last_obj = Guess_and_ConstructNewObject( &lines[i],i);
			if ( last_obj != NULL )
				obj_container->Add_SubObj( last_obj );
		}

		// there if the previous object eat the line
		else if( last_obj->Eat( &lines[i],i) )
		{
			// No validation cause the eater is reponsible of it
			continue;
		}
		// basic situation
		else
		{
			last_obj = Guess_and_ConstructNewObject( &lines[i],i);
			if ( last_obj != NULL )
				obj_container->Add_SubObj( last_obj );
		}

//		Validation( &lines[i], i, last_obj );
	}
}
