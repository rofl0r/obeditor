#ifndef OB_OBJECT_H_
#define OB_OBJECT_H_

#include <list>

// for frames, the val a cloned prop have if there is no clone...
#define NO_CLONED_VAL -666

class MyLine;
class obFile;
/**************************
	GENERAL
	NOTE : 
	All protectd have been passed to public cause of this sister-son
	inheritance cheat
***************************/

/**
 * Init the constraint list for OB files format
 */

void constraints_init();

enum
{	OB_TYPE_STD 			=0,
	OB_TYPE_ENTITY			=1,
	OB_TYPE_MODELS			=2, 
	OB_TYPE_ANIM			=3, 
	OB_TYPE_FRAME			=4,
	OB_TYPE_FRAME_ATTACK		=5,
	OB_TYPE_COMMENT			=6, 
	OB_TYPE_EMPTY_LINE		=7, 
	OB_TYPE_FILE_CONTAINER		=8, 
	OB_TYPE_SCRIPT 			=9,
	OB_TYPE_LEVELS 			=10, 
	OB_TYPE_STAGES_SET		=11, 
	OB_TYPE_STAGE_DECLARATION	=12,
	OB_TYPE_STAGE			=13,
	OB_TYPE_BG_LAYER 			=14, 
	OB_TYPE_FRONT_PANEL		=15, 
	OB_TYPE_STAGE_PANEL		=16,
	OB_STAGE_OBJECT			=17, 
	OB_TYPE_WALL			=18, 
	OB_TYPE_HOLE			=19, 
	OB_TYPE_PLAYER_SPAWN		=20,
	OB_TYPE_STAGE_AT			=21, 
	OB_TYPE_SPAWN			=22,
	OB_TYPE_SPAWN_HEALTHY		=23, 
	OB_TYPE_SPAWN_ITEM 		=24,
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
class ob_object
{
public:
	/**
	 * construct-destruct
	**/
	void ZeroInit();
	ob_object( MyLine* line = NULL, const int num_line = -1);
	ob_object( const wxString& _name, const wxString& _thetoken = wxString() );
	ob_object( const wxString& _name, wxArrayString& _tokens);
	ob_object* Clone();
	void Rm();
	virtual ~ob_object();

	
	/**
	 *	DATA Read Functions
	 */
	wxString GetToken(const size_t i );
	size_t GetNbTokens();
	// Return String representation used by saving process
	virtual wxString ToStr()const;
	wxString GetAllTokenToStr();

	/**
	 *	DATA Write Functions
	 */
	// Must be implemented by class like attacks, which wanna control 
	// the settings of name or properties
	virtual void SetName( const wxString& name );
	virtual bool SetToken( const int i, const wxString& _data, wxString* default_missings = NULL, size_t nb_default_missings = 0);
	virtual void SetNbTokens( size_t _nb_tokens );
	
	// Set name and tokens in one shot
	bool SetAllTokenFromStr( const wxString& _tokens );
	// Set some default name and tokens to the object
	virtual void SetToDefault();
	
	/**
	 *	SUBOBJECTS : Read/Writes Functions
	 */
	ob_object*  GetSubObject( const wxString& name_subobj );
	wxString    GetSubObject_Token( const wxString& name_subobj
					, size_t num_tok = 0 , const wxString& def_val = wxString() );
	ob_object** GetSubObjectS( const wxString& name, size_t& count );
	ob_object** GetSubObjectS_ofType( const int _type , size_t& nb_subobj);
	ob_object** GetAllSubObjS( size_t& res_size );

	
	// Must be use for addition after the object is build from file
	// IE : when load from file checks must be done by Validate()
	//    : After that new mods must be done by calling this function
	virtual bool Add_SubObj( ob_object* temp ); 

	ob_object* SetProperty( const wxString& _name, wxString* _tokens, size_t _arr_tokens );
	ob_object* SetProperty( const wxString& _name, const wxString& _the_tokens );
	ob_object* SetProperty( const wxString& _name, wxArrayString& _tokens );

	/**
	 *	Miscelanious Functions
	 */
		  int  GetType()const;
		  bool Is_Ob_Stage_Object();
	virtual bool IsEqual( ob_object* o );
	
	// For object like frames wich are "last closed" (ie:closed by the object tag)
	virtual int GetLineRef()const;

	// Toggle the edited flag
	void SetEdited( bool _b_edited );

	
//********************************************************************
//********************************************************************
//*****             PROTECTEDs, but not :( cause sister instance can't be called...
//*****		  So, must not be used by outside objects
//***** 		  Must be carrefully used in sub classes
//********************************************************************
public:
	/**
	 * datas functions
	 */
	virtual void pClone(ob_object*& res );
	int  SetData( wxString* arr_data = NULL, const size_t arr_data_size = 0, bool b_update = true );
	int  SetData( MyLine* line, const int num_line, bool b_update = true );
	int  SetData( ob_object* _src );
	
	/**
	 * neighboors management
	 */
	bool 		InsertObject_Before( ob_object* o ); // insert o before this
	ob_object*	RemoveObject_Before(bool b_tell_histManager = true);
	ob_object*	GetObject_Before();

	bool		InsertObject_After( ob_object* );	// insert o after this
	ob_object*	RemoveObject_After(bool b_tell_histManager = true);
	ob_object*	GetObject_After();

	// detach from neighboors and
	// return true if the hist manager have absorbed the elt
	bool Detach(bool b_tell_histManager = true);

	// For Object like frames or anim : used by the repair errors process
	virtual bool Absorb( ob_object* tomerge, bool after = true );
	
public:
	bool Append_SubObj( ob_object* temp );
	bool Prepend_SubObj( ob_object* temp );
	bool Insert_SubObj_After( ob_object* temp );
	bool Insert_SubObj_Before( ob_object* temp );


	/**
	 * Parsing related vars and functions
	 */
	bool ungry;		// May eat more line
	int num_line;	// Corresponding line of the file or -1
	int nb_line;	// Nb line holded by object and subobject
	MyLine* referer;	// trivial

	// Called to an object to provid him the capability to eat the next line readed
	// RETURN true if the line was eated
	virtual bool Eat(  MyLine* line, const int num_line );

	// During Log errors process
	virtual void Validate();
	// For Flushing
	wxString GetLineRef_S()const;
	
	bool Is_EmptyLine()const;
	// Only overrided by StageDeclaration object
	virtual bool Is_EmptyOrComment()const;

	void AssociateToLine( MyLine* line, size_t num_line );
	void AddNbLine(int i);
	
	//--------------------------------------
	// comments lines relative
	bool	   SetComment( const wxString& _comment );
	wxString GetComment()const;
	// Only overrided by Anim object
	virtual void ThrowTailings_LineOrComments();

	//--------------------------------------
	// prefix lines relative
	void EatPrefixLines();
	void Prefix_Append( ob_object* line_prefix );
	void Prefix_Prepend( ob_object* line_prefix );
	void PrependCommentLine( const wxString& _comment, bool b_endOfPrefixComments = true);
	void Absorb_PrefixLines_Of( ob_object * _o, bool b_prepend = false );
	wxString GetPrefixFor(const size_t i)const;
	
	ob_object *first_prefix;
	ob_object *last_prefix;
	

public:
	/**
	 * DATAS
	 */
	// flags
	bool edited;
	bool dummy;

	int type;
	wxString name;
	size_t nb_tokens;
	wxString mycomment;

	// tokens relative
	wxString *arr_token;
	size_t arr_token_size;
	void arr_tokens_expand( size_t newSize );

	// Neighboors objects
	ob_object* prev;
	ob_object* next;

	// subobjects relative
	ob_object* first_subobj;
	ob_object* last_subobj;
	ob_object* parent;

public:
	/**
	 * Helpers Functions
	 */
	void SetType( int _id );
	// group relative management
	ob_object*	GetObject_Group_First();
	ob_object*	GetObject_Group_Last();

	static int id_unspecified;		// Used to set unique names to some objects.
	wxString _nameID( const wxString& _name )const; // Get an unspecified name
};

/**
 * CONTAINER object (the object which contain all objects of a file
 */
class ob_object_container: public ob_object
{
	friend class ob_object;
public:
	ob_object_container();
	virtual void pClone(ob_object*& res );
	~ob_object_container();

	virtual wxString ToStr()const;
};

//---------------------------------------------------------
class ob_entity_container: public ob_object_container
{
	friend class ob_object;
public:
	virtual void pClone(ob_object*& res );
	virtual bool Add_SubObj( ob_object* temp );
};


/**
 * MODELS object
 */

class ob_models: public ob_object
{
public:
	ob_models( MyLine* line = NULL, const int num_line = -1 );
	ob_models( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	~ob_models();
	virtual void Validate();

};


/**
 * Entity object
 */

class ob_entity: public ob_object
{
public:
	static ob_entity* Guess_and_ConstructNewObject(MyLine* line, const int num_line );
	ob_entity( MyLine* line = NULL, const int num_line = -1, bool b_validate =true );
	ob_entity( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	~ob_entity();
	virtual void Validate();
};

/**
 * Animation object
 */
class ob_frame;

class ob_anim : public ob_entity
{
public:
	static bool IsAnimTag( const wxString& data );
	ob_anim( MyLine* line = NULL, const int num_line = -1);
	virtual void pClone(ob_object*& res );
	~ob_anim();
	virtual void ThrowTailings_LineOrComments();

	virtual bool Absorb( ob_object* tomerge, bool after = true );
	
//	virtual void Validate();
	virtual bool Eat( MyLine* line, const int num_line );
	wxString animID();

	ob_frame*  GetFrame( const int num );
	ob_frame** GetFrames(size_t& nb_frames );
	
	virtual bool Add_SubObj( ob_object* temp ); 

public:
	size_t nb_frame;
	bool frame_open;
};


/**
 * Frame  object
 */
class ob_frame_attack;

class ob_frame : public ob_entity
{
public:
	static bool IsFrameTag( const wxString& data );

	ob_frame( MyLine* line = NULL, const int num_line = -1);
	virtual void pClone(ob_object*& res );
	~ob_frame();

	virtual bool Absorb( ob_object* tomerge, bool after = true );
	virtual bool Eat( MyLine* line, const int num_line);
	virtual bool Add_SubObj( ob_object* temp ); 

	wxFileName GifPath()const;
	wxImage* GetImage();
	void     RemoveImage();
	int Get_ColorIndex( const int x, const int y );
	
	virtual wxString ToStr()const;
	virtual int GetLineRef()const;

	int GetAttacks_Count();
	std::list<ob_frame_attack*> GetAttacks();
	wxString GetFirstAvalaibleAttack_name();

	ob_frame* Get_PreviousFrame();
	ob_frame* Get_NextFrame();

public:
	//---------------------
	//Cloned Datas
	//---------------------
	void UpdateClonedDatas(ob_frame* prev);
// 	void Set_Cloned_To_Static();
	
	// Delay
	bool b_delay_cloned;
	int  delay;
	// Offset
	bool b_offset_cloned;
	#define OFF_TOKS_COUNT 2
	int  offset[OFF_TOKS_COUNT];
	// Move
	bool b_move_cloned;
	int  move;
	// MoveZ
	bool b_moveZ_cloned;
	int  moveZ;
	// MoveA
	bool b_moveA_cloned;
	int  moveA;
	// BBox
	bool   b_bBox_cloned;
	#define BBOX_TOKS_COUNT 5
	int bBox[BBOX_TOKS_COUNT];
	// AttBox
	bool   b_attBox_cloned;
	#define ATT_TOKS_COUNT 10
	wxString attName;
	int attBox[ATT_TOKS_COUNT];
	// DrawMethod
	#define DM_TOKS_COUNT 10
	bool b_drawMethod_cloned;
	int drawMethod[DM_TOKS_COUNT];
	// noDrawMethod
	bool b_noDrawMethod_cloned;
	bool noDrawMethod;
	// platform
	bool b_platform_cloned;
	#define PLAT_TOKS_COUNT 8
	int platform[PLAT_TOKS_COUNT];
	
	// 0 => not flipped, 1 => flipped, 2 => flipped and start of the flip
	int flipState;
	int jumpState;
	int dropState;
	int landState;
	bool end_jump;

public:
	wxImage* img;
	imgFile* png_or_gif;
	bool Mergeable( ob_object *tomerge, bool b_after );

};


//------------------------------------------------------------------------
/**
 * Frame Attack object
 */

class ob_frame_attack : public ob_entity
{
public:
	static bool IsAttack_Declare_Tag( const wxString& _prop_name );
	static bool IsAttack_Tag( const wxString& _prop_name );
	static bool IsAttack_SubTag( const wxString& _prop_name );
	
public:	
	ob_frame_attack( MyLine* line = NULL, const int num_line = -1);
	ob_frame_attack( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	virtual ~ob_frame_attack();

	virtual bool IsEqual( ob_object* o );
	virtual bool Eat( MyLine* line, const int num_line);
	virtual void SetName( const wxString& name );
};


//------------------------------------------------------------------------
/**
 * Script object
 */
class ob_script : public ob_entity
{
public:
	ob_script( MyLine* line = NULL, const int num_line = -1);
	virtual void pClone(ob_object*& res );
	~ob_script();

	virtual bool Eat( MyLine* line, const int num_line);
};


/*\internal*********************************************
********	OBJECT RELATED TO LEVELS.TXT FILE
******************************************************/

class ob_StageDeclaration;
class ob_StagesSet;

/**
 * The object which hold the datas about a game (mod)
 */

class ob_levels: public ob_object_container
{
public:
	static ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line );

public:
	ob_levels();
	virtual void pClone(ob_object*& res );
	~ob_levels();

	ob_StagesSet**	 GetSets( size_t& _nb_set);
	bool 			 Set_Insert( ob_StagesSet* t, int ind =-1 ); // -1 Means append

	bool Set_Tag(const wxString& _tag, const wxString& _token );
	bool Set_Tag(const wxString& _tag, wxArrayString _tokens );
};


/**
 * The object which hold a Set configurations and stages declarations
 */

class ob_StagesSet: public ob_object
{
public:
	ob_StagesSet( MyLine* line = NULL, const int num_line = -1 );
	ob_StagesSet( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	~ob_StagesSet();

	virtual bool Eat(  MyLine* line, const int num_line );
	virtual void Validate();
	virtual bool Add_SubObj( ob_object* temp );

	bool 	 SetSetName(const wxString& _sn);
	wxString GetSetName();

	ob_StageDeclaration**	 GetStages( size_t& _nb_stages);
	bool Stage_Insert( ob_StageDeclaration* t, int ind =-1 ); // -1 Means append

	bool Set_Tag(const wxString& _tag, const wxString& _token );
	bool Set_Tag(const wxString& _tag, wxArrayString _tokens );

	virtual void SetToDefault();
};


/**
 * The object which hold a stage declaration in levels.txt
 */

class obFileStage;

class ob_StageDeclaration: public ob_object
{
public:
	ob_StageDeclaration( MyLine* line = NULL, const int num_line = -1 );
	ob_StageDeclaration( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	~ob_StageDeclaration();

	virtual bool Is_EmptyOrComment()const;
	virtual bool Add_SubObj( ob_object* temp );

	bool Eat(  MyLine* line, const int num_line );
	virtual void Validate();
	virtual wxString ToStr()const;

	void 	 SetStageName(const wxString& _sn);
	wxString GetStageName();	// RETURN : "UNNAMED" if none found

	bool 	 SetFileName(const wxString& _fn);
	wxString GetFileName();

	bool Set_BranchTag( const wxString& _bn);
	bool Set_Tag(const wxString& _tag, const wxString& _token );
	bool Set_Tag(const wxString& _tag, wxArrayString _tokens );

	wxSize GetDefScrollzBounds();
	
	bool LoadStage(bool b_reload = false);
	void UnLoadStage();

	bool b_completed;
	obFileStage* stage;
};




#endif /*OB_OBJECT_H*/
