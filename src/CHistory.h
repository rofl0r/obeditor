#ifndef CHISTORY_H
#define CHISTORY_H

#include <list>
#include <set>

class ob_object;
class History_Group;

//*********************************************************
class History_Elt
{
public:
	History_Elt();
	// For OBJ_INSERT , OBJ_RM
	History_Elt( int _type, ob_object* _o, ob_object* _o_related, bool b_append );
	// For TOKEN_CHANGE, TOKEN_ADD, TOKEN_RM
	History_Elt(  int _type, ob_object* _o
			, const wxString& _token_val = wxString(), int _token_pos = 0 );
	void Zero_Init();
	virtual ~History_Elt();

	// Return true if do or undo process must STOP
	bool Do(  std::list<History_Group*>::iterator it_grp);
	bool UnDo(std::list<History_Group*>::iterator it_grp);
	
public:
	wxString ToStr();
	void MayDeleteO();
	
	wxLongLong theDate;
	enum
	{
		H_DUMMY		=1,
		H_NAME_CHANGE	=2,
		H_TOKEN_CHANGE	=3,	// A token Value have change
		H_TOKEN_ADD		=4,	// A token have been appended
		H_TOKEN_RM		=5,	// A token have been de-appended
		H_OBJ_INSERT	=6,
		H_OBJ_RM		=7,
	};
	int type;
	bool is_past;
	// Very possible associate object
	ob_object* o;
	// Less often associate object  (ie:for insert,rm)
	ob_object* o_related;
	// possible related token position or tokens_size truncation
	int token_pos;
	// And his eventual previous value
	wxString token_val;
	
	// Usable by History users typically for storing datas about control states
	void* datas;
};


//*********************************************************
class History_Group
{
public:
	History_Group();
	virtual ~History_Group();
	
	// return true if the elt can be added
	bool IsElt__CanBeAdded( History_Elt* _elt );
	void Elt__Add( History_Elt* _elt );

	// Return true if do or undo process must CONTINUE
	bool Do(  std::list<History_Group*>::iterator it_grp);
	bool UnDo(std::list<History_Group*>::iterator it_grp);
	
	wxString GetName();
	
public:
	std::list<History_Elt*> l_elts;
	
	// True if more elts can be added
	bool b_group_opened;
	wxString name;
};

//*********************************************************
typedef int History_CallBAck(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
					);
typedef void History_CallBAck_Simple(bool b_state);
typedef void History_CallBAck_Delete(void*&  _elt_data);
typedef void History_CallBAck_Basic();

					
class History_Manager
{
public:
	History_Manager();
	void Reset();
	virtual ~History_Manager();
	
	bool Elt__Absorb( History_Elt* _elt );
	void Set_State(bool b_enable);
	void Force_Disable();
	void GroupStart( const wxString& grp_name );
	void GroupEnd();
	void Clear_History();

	void Set_CallBacks( History_CallBAck* onUndo
				, History_CallBAck* onDo
				, History_CallBAck* onEltAdd
				, History_CallBAck_Simple* onNoMoreUndo
				, History_CallBAck_Simple* onNoMoreDo
				// For clean deletion of <datas>
				, History_CallBAck_Delete* onDeleteHistory_data
				, History_CallBAck_Basic* onAfterEltAdded = NULL
				);

	// Return true if something have been done/undone
	bool Do();
	bool UnDo();

 	History_Elt* Get_Last_HistoryElt();
 	void Group__DeleteLast();
	
	bool IsFirstGroup( std::list<History_Group*>::iterator it );
	bool IsLastGroup(  std::list<History_Group*>::iterator it );
	bool IsHistory_Empty();
	bool IsPresent();
	bool IsEnabled()
		{return disable_count==0;};
	bool IsUndoing(){return (m_flag&M_IN_DO_OR_UNDO) != 0;};
	
public:
	void Group__Append( const wxString& grp_name = wxString() );
	
	void Clear_Future();

	// Objects wich are no longer referenced in the "present objects"
	std::set<ob_object*> l_orphans;
	 
	enum
	{
		M_IN_DO_OR_UNDO 	= 1,
	};
	int m_flag;
	size_t disable_count;
	size_t group_pack_count;
	std::list<History_Group*> l_history_groups;
	std::list<History_Group*>::iterator it_now;
	
	History_CallBAck* onUndo;
	History_CallBAck* onDo;
	History_CallBAck* onEltAdd;
	History_CallBAck_Simple* onNoMoreUndo;
	History_CallBAck_Simple* onNoMoreDo;
	History_CallBAck_Delete* onDeleteHistory_data;
	History_CallBAck_Basic*  onAfterEltAdded;

	// Usable by History users typically for storing datas about control states
	void* initial_datas;
};


//*********************************************************
extern History_Manager theHistoryManager;
//*********************************************************


int 
HCB_May_Merge_Hand_Edited_Object(
			  std::list<History_Group*>::iterator it_group
			, History_Elt*  _elt
					);


// find the most group-related-recent control states
void*
Get_Prev_Ctrl_States(	 std::list<History_Group*>::iterator it_grp
				,bool b_grp_end_elt_valid );


#endif //CHISTORY_H