#ifndef OB_FILES_H_
#define OB_FILES_H_

#include <list>
#include "common__ob_object.h"

extern wxFileName dataDirPath;

/**
 * A line token => token + prefix
 */
class MyToken
{
public:
	MyToken();
	MyToken( const wxString& _prefix, const wxString& _data );
	MyToken(const MyToken&);
	MyToken& operator=(const MyToken&);
	~MyToken();
	wxString Prefix()const;
	wxString ToStr()const;
	wxString *prefix;
	wxString *data;
};

/**
 * A parsed line => tokens + prefixs tokens + comments
 */
class MyLine
{
public:
	MyLine(const wxString& raw_myLine);
	MyLine();
	~MyLine();

	MyToken* operator[](const size_t);
	MyToken* GetToken(const size_t);
	wxString* GetDatas()const;
	wxString ToStr()const;
	bool Set( const wxString& raw_myLine );
	void Reset();
	size_t GetTokensCount()const;
	wxString Comment()const;
	void AppendToken( const wxString& _prefix, const wxString& _data );

	bool IsEmpty()const;
	bool IsComment()const;
	wxString Tag()const;
	bool changed;

private:
	void Tokenize( const wxString& raw_myLine, MyToken*& arr_tokens, size_t& nb_tokens, size_t& arr_tokens_size, wxString*& suffix );
	wxString *comment;
	MyToken *arr_tokens;
	size_t arr_tokens_size;
	size_t nb_tokens;
};

/**
 * A parsed line => tokens + prefixs tokens + comments
 */
class wxTextFile;


//*************************************************
//*************************************************

class obFile
{
public:
	obFile();
	obFile( const wxString& str_filename, bool b_load = true);
	obFile( const wxFileName fullpath, bool b_load = true );
	obFile( const wxTextFile& file, bool b_load = true);
	virtual ~obFile();

	virtual void Reset();
	virtual void Reload();
	virtual void SetChanged();

	MyLine* lines;
	int nb_lines;
	wxFileName filename;
	wxTextFile* textFile;
	int changed;

	ob_object*  GetProperty(const wxString& _name);
	ob_object** GetSubObjectS(const wxString& _name, size_t& count );


	bool LoadLines();
	bool OpenFile();
	bool Write();

	wxString ToStr();
	void AllocPrefixsLines();

	int alloced_lines;
	ob_object_container* obj_container;

protected:
	void ZeroInit();
	virtual void BuildObjects();
	virtual void ParseFile();
	virtual void Validation(MyLine* line, const int num_line, ob_object*& last_obj );
	virtual void Cleaning();
	virtual ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line ) = 0;
	virtual ob_object_container* NewContainer();
};


//*************************************************
//*************************************************

class obFileEntity : public obFile
{
public:
	obFileEntity( const wxString& str_filename);
	obFileEntity( const wxFileName fullpath);
	obFileEntity( const wxTextFile& file);
	virtual ~obFileEntity();
	void Reset();
	virtual ob_object_container* NewContainer();

	void AddProperty( ob_object* new_prop );
	ob_anim* GetFirstAnim();
	ob_anim** GetAnims(size_t& nb_anims );
	ob_anim* GetAnim( const wxString& _animID );

	wxString Name();
	wxString Get_AvatarImg_Path();
	wxImage* Load_AvatarImg();

	void PurgeImages();

	wxSize avatar_img_path_offset;
protected:
	wxString avatar_img_path;
	wxImage* avatarImg;
	virtual void Validation(MyLine* line, const int num_line, ob_object*& last_obj );
	virtual void Cleaning();
	virtual ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line );
};


//*************************************************
//*************************************************

class obFileModels : public obFile
{
public:
	obFileModels( const wxString& str_filename);
	obFileModels( const wxFileName fullpath);
	obFileModels( const wxTextFile& file);
	virtual ~obFileModels();

	ob_models** GetModels(int& count);
	ob_models* GetModel( const wxString& model_name );

protected:
	virtual void ParseFile();
	virtual ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line );
};


//*************************************************
//*************************************************

class obFileLevels : public obFile
{
public:
	obFileLevels( const wxString& str_filename);
	obFileLevels( const wxFileName fullpath);
	obFileLevels( const wxTextFile& file);
	virtual ~obFileLevels();


protected:
	virtual ob_object_container* NewContainer();
	virtual ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line );
};


//*************************************************
//*************************************************

class obFileStage : public obFile
{
public:
	static obFileEntity* GetEntity( const wxString& ent_name );
	static std::list<obFileEntity* >* GetEntity_ALL(wxWindow* busy_parent = NULL);
	static void ClearEntitiesCache();

public:
	obFileStage( const wxString& str_filename);
	obFileStage( const wxFileName fullpath);
	obFileStage( const wxTextFile& file);
	~obFileStage();

	size_t GetSetsCount();
	ob_StagesSet** GetSets();


protected:
	virtual void Cleaning();
	virtual ob_object_container* NewContainer();
	virtual ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line );
};


//*************************************************
//*************************************************

class obFileVideoTxt : public obFile
{
public:
	obFileVideoTxt( const wxString& str_filename);
	obFileVideoTxt( const wxFileName fullpath);
	obFileVideoTxt( const wxTextFile& file);
	virtual ~obFileVideoTxt();

protected:
	virtual void ParseFile();
	virtual ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line );
};



//*************************************************
//*************************************************
#ifndef _TEST_
extern obFileModels *fileModels;
extern obFileLevels *fileLevels;
#endif

#endif /*OB_FILES_H_*/
