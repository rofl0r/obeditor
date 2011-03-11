/*
 * common_object_stage.h
 *
 *  Created on: 25 avr. 2009
 *      Author: pat
 */

#ifndef COMMON_OBJECT_STAGE_H_
#define COMMON_OBJECT_STAGE_H_

#include "common__ob_object.h"
#include <list>

/*\internal*********************************************
********	OBJECT RELATED TO Stages FILES
******************************************************/
class ob_stage_object;
class ob_stage_panel;
//////////////////////////////////////////

/**
 * The object which hold the datas about a stage of a game
 */
// Possibles stage direction
enum
{
	STAGE_UNDEF_DIRECTION,
	STAGE_RIGHT,
	STAGE_LEFT,
	STAGE_UP,
	STAGE_DOWN,
	STAGE_IN,
	STAGE_OUT,
};

class ob_stage: public ob_object_container
{
public:
	static ob_object* Guess_and_ConstructNewObject(MyLine* line, const int num_line );
	static ob_object* Guess_and_ConstructNewObject(const wxString& _name, const wxString& token0 );

	static bool IsHeaderTag( const wxString& name );
	static bool Is_Stage_At_Type( ob_object* _obj );
	static bool Is_Stage_Spawn_Type( ob_object* _obj );
	
public:
	ob_stage();
	virtual void pClone(ob_object*& res );
	virtual ~ob_stage();
	
	int Direction_ComputeAndCache();
	int direction;
	
	wxSize GetDefScrollzBounds(ob_object* st_declar);
	ob_stage_panel** Get_Panels( size_t& nb_panels );
	wxString Get_Panels_Order();
	
	std::list<ob_stage_object*>* 
		Get_SubObj_With_StageType( const int stage_object_type );

	virtual bool Add_SubObj( ob_object* temp );
	virtual bool Add_SubObj( ob_object* temp, bool b_sort_at );

	bool Set_Tag(const wxString& _tag, const wxString& _token );
	bool Set_Tag(const wxString& _tag, wxArrayString _tokens );

protected:
	ob_object* Get_LastHeaderObj();
	ob_object* Get_First_AT_Obj();
	
public:
	bool Add_SubObj_AT( ob_object* _obj, bool b_after = true );
	bool Insert_SubObj_AT( ob_object* _obj );
	bool Add_SubObj_WallHole( ob_object* _obj, bool b_after = true );
	bool Add_SubObj_Panel( ob_object* _obj, bool b_after = true );
	bool Add_SubObj_FrontLayer( ob_object* _obj, bool b_after = true );
	bool Add_SubObj_BGs( ob_object* _obj, bool b_after = true );
	bool Add_SubObj_Header( ob_object* _obj, bool b_after = true );

};

/**
 * A background layer of the stage
 */

class ob_BG_Layer: public ob_object, Image_Holder
{
public:
	ob_BG_Layer( MyLine* line = NULL, const int num_line = -1 );
	ob_BG_Layer( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	virtual ~ob_BG_Layer();
	virtual void Validate();
	bool Update_With( ob_BG_Layer* oLay );
	
	virtual void SetToDefault();
		  void FillMissings();
		  int  Get_RepeatingVal(int direction );
		  
	wxString GetName();
	virtual wxFileName GetFileName();

};


/**
 * A front panel of the stage
 */

class ob_front_panel: public ob_object, Image_Holder
{
public:
	ob_front_panel( MyLine* line = NULL, const int num_line = -1 );
	ob_front_panel( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	virtual ~ob_front_panel();
	virtual void Validate();
	bool Update_With( ob_front_panel* oPan );

	wxString GetName();
	virtual wxFileName GetFileName();
};


/**
 * A panel of the stage
 */

class ob_stage_panel: public ob_object, public Image_Holder
{
public:
	ob_stage_panel( MyLine* line = NULL, const int num_line = -1 );
	ob_stage_panel(const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	virtual ~ob_stage_panel();
	virtual void Validate();

	wxString GetName();
	void SetToDefault();

	virtual wxFileName GetFileName();
};



/**---------------------------------------------------------------------
 *---------------------------------------------------------------
 *---------------------------------------------------------------
 *---------------------------------------------------------------
 *---------------------------------------------------------------
 * An object holded by a stage object
 */
enum
{
	STAGETYPE_NONE		= 1000,
	SOBJ_PLAYER_SPAWN		= 1002,	// Other
	SOBJ_WALL			= 1004,	// Geometry
	SOBJ_HOLE			= 1006,	// Geometry
};

class ob_stage_object : public ob_object
{
public:
	static int Get_StageObjectType( const wxString& );

public:
	ob_stage_object( MyLine* line = NULL, const int num_line = -1 );
	ob_stage_object( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );

	virtual ~ob_stage_object();
	virtual void Validate();

	virtual wxString ToStr();
	virtual wxString Get_ScreenList_Name(){return name;};

	int object_type;
	
	virtual bool Get_Coords(int& x, int& y);
	wxString CoordsToStr(); // used for object which use coords for screen name

	virtual void Coords_Decal( const wxSize& _decal );
	virtual void Init_and_Center( wxSize& _decal );
};


/**---------------------------------------------------------------------
 * A wall
 */

class ob_wall : public ob_stage_object
{
public:
	ob_wall( MyLine* line = NULL, const int num_line = -1 );
	ob_wall(const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );

	virtual ~ob_wall();
	virtual void Validate();

	virtual bool Get_Coords(int& x, int& y);

	virtual void Coords_Decal( const wxSize& _decal );
	virtual void Init_and_Center( wxSize& _decal );
};


/**---------------------------------------------------------------------
 * An hole
 */

class ob_hole : public ob_stage_object
{
public:
	ob_hole( MyLine* line = NULL, const int num_line = -1 );
	ob_hole( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );

	virtual ~ob_hole();
	virtual void Validate();

	virtual bool Get_Coords(int& x, int& y);

	virtual void Coords_Decal( const wxSize& _decal );
	virtual void Init_and_Center( wxSize& _decal );
};


/**---------------------------------------------------------------------
 * An player spawn location which contain X Y Z and no AT tag
 */

class ob_player_spawn : public ob_stage_object
{
public:
	static int Get_StageObjectType( const wxString&);

public:
	ob_player_spawn( MyLine* line = NULL, const int num_line = -1 );
	ob_player_spawn( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );

	virtual ~ob_player_spawn();
	virtual void Validate();

	virtual bool Get_Coords(int& x, int& y);

	virtual void Coords_Decal( const wxSize& _decal );
	virtual void Init_and_Center( wxSize& _decal );
};


/**----------------------------------------------------------------------
 * An object that hold a <AT> tag
 */

enum
{
	SOBJ_WAIT		= 1026,	// Control
	SOBJ_NOJOIN		= 1028,	// Control
	SOBJ_CANJOIN	= 1030,	// Control
	SOBJ_SHADOWCOLOR	= 1018,	// Visual Effect
	SOBJ_SHADOWALPHA	= 1020,	// Visual Effect
	SOBJ_LIGHT 	 	= 1014,	// Visual Effect
	SOBJ_SCROLLZ 	= 1010,	// Geometry
	SOBJ_BLOCKADE	= 1024,	// Control
	SOBJ_SETPALETTE	= 1022,	// Other
	SOBJ_GROUP 	 	= 1012,	// Control
};

class ob_stage_at : public ob_stage_object
{
public:
	static int  Get_StageObjectType( const wxString& );

public:
	ob_stage_at( MyLine* line = NULL, const int num_line = -1 );
	ob_stage_at( const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );

	virtual ~ob_stage_at();

	virtual bool Eat(  MyLine* line, const int num_line );
	virtual void Validate();

	virtual bool Get_Coords(int& x, int& y);

	int   GetAt();
	void  SetAt(int _here);
	virtual void Coords_Decal( const wxSize& _decal );
	virtual void Init_and_Center( wxSize& _decal );
	
	// Replace this object at his right position in the AT list of the stage
	void Reorder_At();
	
	virtual bool Add_SubObj( ob_object* temp ); 
};



/**----------------------------------------------------------------------
 * Class for spawned objects
 * This objects must have <coords> and <at>, alos have an <entity> object associated
 */

enum
{
	SOBJ_NONE    	= 1100,	// Decoration
	SOBJ_STEAMER 	= 1102,	// Decoration
	SOBJ_PANEL   	= 1104,	// Decoration
	SOBJ_TEXT 		= 1106,	// Other
	SOBJ_TRAP 		= 1108,	// Stuffs
	SOBJ_SHOT 		= 1110,	// Stuffs 
};


class ob_spawn : public ob_stage_at
{
public:
	static int Get_StageObjectType( const wxString& _name, const wxString& entity_name );
	static int Get_Entity_StageType( const wxString& ent_name );

public:
	ob_spawn( MyLine* line = NULL, const int num_line = -1 );
	ob_spawn(const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	virtual ~ob_spawn();

	virtual bool Eat(  MyLine* line, const int num_line );
	virtual bool Is_AcceptedTag(const wxString& _name );
	virtual void Validate();

	obFileEntity* entity_ref;

	virtual wxString 	Get_ScreenList_Name();
	virtual bool	Get_Coords(int& x, int& y);

	virtual void Coords_Decal( const wxSize& _decal );
	virtual void Init_and_Center( wxSize& _decal );
};


/**-----------------------------------------------------------------
 * Object with Health
 */

enum
{
	SOBJ_OBSTACLE	= 1200,	// Stuffs
	SOBJ_NPC		= 1202,	// Livings
	SOBJ_ENEMY		= 1204,	// Livings
};

class ob_spawn_healthy : public ob_spawn
{
public:
	static int Get_StageObjectType( const wxString& _name, const wxString& entity_name );
	static bool IsItemHoldedTag( const wxString& _tag );
	
public:
	ob_spawn_healthy( MyLine* line = NULL, const int num_line = -1 );
	ob_spawn_healthy(const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	virtual ~ob_spawn_healthy();

	virtual void Validate();
	virtual bool Is_AcceptedTag(const wxString& _name );
};


/**-----------------------------------------------------------------
 * An item spawned (not dropped)
 */

enum
{
	SOBJ_ITEM 		= 1300,	// item
	SOBJ_ENDLEVEL	= 1302,	// end level item
};

class ob_spawn_item: public ob_spawn
{
public:
	static int Get_StageObjectType( const wxString& _name, const wxString& entity_name );

public:
	ob_spawn_item( MyLine* line = NULL, const int num_line = -1 );
	ob_spawn_item(const wxString& _name, wxArrayString& _tokens);
	virtual void pClone(ob_object*& res );
	virtual ~ob_spawn_item();

	virtual bool Is_AcceptedTag(const wxString& _name );
	virtual void Validate();
};


/**-----------------------------------------------------------------
 * Stage Super Types enum for sorting types
 */
enum
{
	SST_NONE		= 0,
	SST_OTHER		= 1,
	SST_GEOMETRY	= 2,
	SST_CONTROL		= 3,
	SST_VISUAL_FX	= 4,
	SST_STUFFS 		= 5,
	SST_DECORATION	= 6,
	SST_LIVINGS 	= 7,
	
	SST_MAX 		= 7,	// Used for fillings Lists
};

int		Stage__SuperType_ToInt( const wxString& _sst );
wxString	Stage__SuperType_ToStr( const int _sst );
const int*	Stage__SuperType_Get_Type_Array( const int _sst, size_t& arr_size );

int		Stage__Type_ToInt( const wxString& _sst );
wxString	Stage__Type_ToStr( const int _sst );
int		Stage__Type__Get_SuperType( const int _sobj_type );
bool		Stage__Type_Is_in_SuperType( const int _sobj_type, const int _sst );

#endif /* COMMON_OBJECT_STAGE_H_ */
