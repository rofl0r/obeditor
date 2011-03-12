#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/gbsizer.h>
#include <wx/arrstr.h>

#include "common__mod.h"
#include "common__ob_controls.h"
#include "common__validators.h"
#include "myControls.h"

#include "ob_editor.h"
#include "levels__globals.h"
#include "levels__classes.h"
#include "levels__sidesWindows.h"
#include "common__object_stage.h"
#include "levels__Panel_StageView.h"
#include "levels__CtrlStageView.h"
#include "levels__Ob_GObjects.h"

using namespace std;

//-------------------------------------------------------

extern LevelEditorFrame *leFrame;


//****************************************************
void 
LevelEditorFrame::VIEW__Reset()
{
	TYPE__curr_selected = STAGETYPE_NONE;
	OBJECT__CtrlList_Clear();
	aSet_Highlighted.clear();
	aSet_Selected.clear();
	stageView->Reset();
}

//****************************************************
void 
LevelEditorFrame::VIEW__Update( bool b_quiet )
{
	stageView->UpdateView();
}

//****************************************************
//****************************************************
//-------------------------------------------------------
void 
LevelEditorFrame::SuperType__Select( int _sst )
{
	if( curr_stageFile == NULL )
		return;
	
	// Check if something change...
	if( object_Type_list->GetCount() > 0 )
	{
		wxString _t_str = object_Type_list->GetString( 0 );
		int _t_id = Stage__Type_ToInt( _t_str.Right(_t_str.Len()-2) );
		int prev_st = Stage__Type__Get_SuperType( _t_id );
		
		// Nothing change
		if( prev_st == _sst )
			return;
	}
	
	// Clear stuffs
	objects_List->SetSelection( wxNOT_FOUND );
	OBJECT__CtrlList_Clear();
	object_Type_list->SetSelection( wxNOT_FOUND );
	object_Type_list->Clear();
	TYPE__curr_selected = STAGETYPE_NONE;
	
	size_t arr_type_size;
	const int* arr_type = Stage__SuperType_Get_Type_Array( _sst, arr_type_size );
	if( arr_type != NULL )
	{
		for( size_t i = 0; i < arr_type_size; i++ )
		{
			wxString s = Stage__Type_ToStr(arr_type[i]);
			if( aSet_Hide_TYPE.find(arr_type[i]) == aSet_Hide_TYPE.end() )
				s = wxT("  ") + s;
			else
				s = wxT("# ") + s;
			
			object_Type_list->Append( s, (void*) arr_type[i] );
		}
	}
	
	sizer_bottom->Layout();
	OBJECT__Visibilities_Update();
}

//-------------------------------------------------------
int  
LevelEditorFrame::TYPE__Get_Current_Selected()
{
	return TYPE__curr_selected;
}

//-------------------------------------------------------
#define LEVELEDITORFRAME__TYPE__SELECT__FINALY	\
		OBJECT__Visibilities_Update();	\
		sizer_bottom->Layout();			\
		

void 
LevelEditorFrame::TYPE__Select( int _stype, bool b_force )
{
	if( curr_stageFile == NULL )
		return;
	
	// Check if something change...
	int curr_sel = TYPE__Get_Current_Selected();
	if( _stype == curr_sel && b_force == false )
		return;
	
	// Update current selected type
	TYPE__curr_selected = _stype;
	
	// Clear stuffs
	OBJECT__CtrlList_Clear();

	// None Type  ==>>  Return
	if( _stype == STAGETYPE_NONE || curr_stageFile == NULL )
	{
		LEVELEDITORFRAME__TYPE__SELECT__FINALY
		return;
	}
	
	ob_stage* _the_stage = (ob_stage*) curr_stageFile->obj_container;
	if( _the_stage == NULL )
	{
		LEVELEDITORFRAME__TYPE__SELECT__FINALY
		return;
	}
	
	bool b_singleton;
	switch( Stage__Type__Get_SuperType(_stype ) )
	{
		// Fill the list with coords of ALL objects
		case SST_GEOMETRY:
		case SST_CONTROL:
		case SST_VISUAL_FX:
		case SST_OTHER:
			b_singleton = true;
			break;
			
		// Fill the list with UNIQUE names of associated entities
		case SST_STUFFS:
		case SST_DECORATION:
		case SST_LIVINGS:
			b_singleton = false;
			break;
			
		case SST_NONE:
		default:
		{
			LEVELEDITORFRAME__TYPE__SELECT__FINALY
			return;
		}
	}
	
	list<ob_stage_object*>*  l_objs = 
			_the_stage->Get_SubObj_With_StageType(_stype);
	
	
	list<ob_stage_object*>::iterator it( l_objs->begin())
					, it_end(l_objs->end());

	int curr_ind = 0;
	while( l_objs->empty() == false )
	{
		ob_stage_object* t = l_objs->front();
		l_objs->pop_front();
		
		if( b_singleton == true )
		{
			list<ob_stage_object*>* t_l = new list<ob_stage_object*>;
			t_l->push_back( t );
			objects_List->Append( t->CoordsToStr(), (void*) t_l );
			curr_ind++;
		}
		
		else // b_singleton == false
		{
			wxArrayString arr_str = objects_List->GetStrings();
			wxString t_screenname = t->Get_ScreenList_Name();
			bool b_inserted = false;
			for( size_t i = 0; i <arr_str.Count(); i++ )
			{
				if( arr_str[i] == t_screenname )
				{
					list<ob_stage_object*>* p_l = 
						(list<ob_stage_object*>*) objects_List->GetClientData( i );
					p_l->push_back( t );
					b_inserted = true;
				}
			}
			
			if( b_inserted == false )
			{
				list<ob_stage_object*>* t_l = new list<ob_stage_object*>;
				t_l->push_back( t );
				wxString _name = t->Get_ScreenList_Name();
				
				// Sort the List
				for( size_t j = 0; j < objects_List->GetCount();j++ )
				{
					if( objects_List->GetString( j).CmpNoCase(_name )> 0 )
					{
						objects_List->Insert(_name,j, (void*) t_l );
						b_inserted = true;
						break;
					}
				}
				
				if( b_inserted == false )
					objects_List->Append(_name, (void*) t_l );
			}
		}
	}
	
	delete l_objs;
	LEVELEDITORFRAME__TYPE__SELECT__FINALY
	return;
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__CtrlList_Clear()
{
	// Clear stuffs
	objects_List->SetSelection( wxNOT_FOUND );
	for( size_t i = 0; i < objects_List->GetCount();i++)
	{
		list<ob_stage_object*>* p_l = 
			(list<ob_stage_object*>*) objects_List->GetClientData( i );
		if( p_l != NULL )
		{
			delete p_l;
			objects_List->SetClientData(i,NULL);
		}
	}
	objects_List->Clear();

	OBJECT__Visibilities_Update();
	sizer_bottom->Layout();
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__CtrlList_Selection_Change()
{
	if( curr_stageFile == NULL )
		return;
	
	int ind_sel = objects_List->GetSelection();
	if( ind_sel == wxNOT_FOUND )
	{
		TYPE__Select(TYPE__curr_selected);
		return;
	}

	OBJECT__Visibilities_Update();
	return;
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__Visibilities_Update()
{
	if( curr_stageFile == NULL || b_closing == true )
		return;
	ob_stage* _the_stage = (ob_stage*) curr_stageFile->obj_container;
	if( _the_stage == NULL )
		return;
	
	int ind_sel;
	
	//---------------------------------------------
	// Object selection Exist
	ind_sel = objects_List->GetSelection();
	if( ind_sel != wxNOT_FOUND )
	{
		// Higlighted Set == current objects set
		aSet_Highlighted.clear();
		
		list<ob_stage_object*>* t_l = (list<ob_stage_object*>*)
				objects_List->GetClientData( ind_sel );
		if( t_l != NULL )
		{
			list<ob_stage_object*>::iterator it(t_l->begin())
								,it_end(t_l->end());
			for(; it != it_end;it++)
				aSet_Highlighted.insert( (*it) );
		}
		VIEW__Update();
		return;
	}
		
	//---------------------------------------------
	// Type selection Exist
	ind_sel = object_Type_list->GetSelection();
	if( ind_sel != wxNOT_FOUND )
	{
		// Higlighted Set  == objects of this type
		aSet_Highlighted.clear();
		
		int _type = (size_t) object_Type_list->GetClientData(ind_sel);
		if( _type != STAGETYPE_NONE )
		{
			list<ob_stage_object*>*  l_objs = 
					_the_stage->Get_SubObj_With_StageType(_type);
			
			while( l_objs->empty() == false )
			{
				ob_stage_object* t = l_objs->front();
				l_objs->pop_front();
				aSet_Highlighted.insert( t );
			}
			delete l_objs;
		}
		VIEW__Update();
		return;
	}

	//---------------------------------------------
	// SuperType selection Exist
	ind_sel = object_SuperType_list->GetSelection();
	if( ind_sel != wxNOT_FOUND )
	{
		// Higlighted Set  == objects of this SuperType
		aSet_Highlighted.clear();
		
		int _stype = (size_t) object_SuperType_list->GetClientData(ind_sel);
		if( _stype != SST_NONE )
		{
			size_t arr_type_size;
			const int* arr_type = 
				Stage__SuperType_Get_Type_Array( _stype, arr_type_size );
			if( arr_type != NULL )
			{
				for( size_t i = 0; i < arr_type_size; i++ )
				{
					list<ob_stage_object*>*  l_objs = 
					    _the_stage->Get_SubObj_With_StageType(arr_type[i]);
					
					while( l_objs->empty() == false )
					{
						ob_stage_object* t = l_objs->front();
						l_objs->pop_front();
						aSet_Highlighted.insert( t );
					}
					delete l_objs;
				}
			}
		}
		VIEW__Update();
		return;
	}
	
	// nothing to highlight
	aSet_Highlighted.clear();
	VIEW__Update();
	return;
}

//-------------------------------------------------------
void 
LevelEditorFrame::TYPE__Reselect()
{
	if(TYPE__curr_selected == STAGETYPE_NONE )
		return;
	
	int ind = objects_List->GetSelection();
	if( ind == wxNOT_FOUND )
	{
		TYPE__Select( TYPE__curr_selected, true );
		return;
	}
	
	wxString prev_selstr = objects_List->GetStringSelection();
	
	// Force reselection
	TYPE__Select( TYPE__curr_selected, true );
	
	// Try to reselect previous object
	if( objects_List->SetStringSelection( prev_selstr ) )
		OBJECT__Visibilities_Update();
}


//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__Visibilities_Clear()
{
	// First Unselect every thing
	OBJECT__CtrlList_Clear();
	TYPE__Select( STAGETYPE_NONE );
	SuperType__Select( SST_NONE );

	// Second Reset the Show States
	OBJECT__ShowStates__Clear();
	sizer_bottom->Layout();
	VIEW__Update();
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__Visibilities_SaveState()
{
	wxString s;
	set<int>::iterator it(aSet_Hide_SuperType.begin())
			  ,it_end(aSet_Hide_SuperType.end());
	for( ;it!=it_end;it++)
		s += IntToStr(*it) + wxT(",");
	s.Truncate(s.Len()-1);
	ConfigEdit_Write(wxT("OBJECT__Visibilities_State_SuperType"), s );
	
	s = wxString();
	it 	 = aSet_Hide_TYPE.begin();
	it_end = aSet_Hide_TYPE.end();
	for( ;it!=it_end;it++)
		s += IntToStr(*it) + wxT(",");
	s.Truncate(s.Len()-1);
	ConfigEdit_Write(wxT("OBJECT__Visibilities_State_TYPE"), s );
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__Visibilities_LoadState()
{
	wxString s = ConfigEdit_Read(wxT("OBJECT__Visibilities_State_SuperType"));
	if( s == wxString() )
		return;
	wxArrayString arr_s = StrSplit(s, wxT(",") );
	for( size_t i = 0;i < arr_s.Count();i++)
		OBJECT__ShowStates__Add_SuperType( StrToInt( arr_s[i] ));
	
	s = ConfigEdit_Read(wxT("OBJECT__Visibilities_State_TYPE"));
	arr_s = StrSplit(s, wxT(",") );
	for( size_t i = 0;i < arr_s.Count();i++)
		OBJECT__ShowStates__Add_TYPE( StrToInt( arr_s[i] ));
	
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ShowStates__Clear()
{
	set<int>::iterator it;
	while( aSet_Hide_SuperType.empty() == false )
	{
		it = aSet_Hide_SuperType.begin();
		OBJECT__ShowStates__Remove_SuperType( *it );
	}
	aSet_Hide_SuperType.clear();
	
	while( aSet_Hide_TYPE.empty() == false )
	{
		it = aSet_Hide_TYPE.begin();
		OBJECT__ShowStates__Remove_TYPE( *it );
	}
	aSet_Hide_TYPE.clear();
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ShowStates__Toggle_SuperType( int _st )
{
	for( size_t i = 0; i < object_SuperType_list->GetCount();i++)
	{
		int t_t = (size_t) object_SuperType_list->GetClientData(i);
		if(t_t == _st )
		{
			wxString s = object_SuperType_list->GetString( i );
			if( (int) s[0] == ' ' )
				OBJECT__ShowStates__Add_SuperType( _st );
			else
				OBJECT__ShowStates__Remove_SuperType( _st );
			break;
		}
	}
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ShowStates__Add_SuperType(int _st )
{
	size_t i;
	for(  i = 0; i < object_SuperType_list->GetCount();i++)
	{
		int t_st = (size_t) object_SuperType_list->GetClientData(i);
		if( t_st == _st )
		{
			object_SuperType_list->SetString( i, wxT("# ") + Stage__SuperType_ToStr(t_st) );
			break;
		}
	}
	
	if( i != object_SuperType_list->GetCount() )
	{
		aSet_Hide_SuperType.insert( _st );
		VIEW__Update();
	}
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ShowStates__Remove_SuperType( int _st )
{
	if( aSet_Hide_SuperType.find( _st ) == aSet_Hide_SuperType.end() )
		return;
	
	size_t i;
	for(  i = 0; i < object_SuperType_list->GetCount();i++)
	{
		int t_st = (size_t) object_SuperType_list->GetClientData(i);
		if( t_st == _st )
		{
			object_SuperType_list->SetString( i, wxT("  ") + Stage__SuperType_ToStr(t_st) );
			break;
		}
	}

	if( i != object_SuperType_list->GetCount() )
	{
		aSet_Hide_SuperType.erase( _st );
		VIEW__Update();
	}
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ShowStates__Toggle_TYPE( int _t )
{
	for( size_t i = 0; i < object_Type_list->GetCount();i++)
	{
		int t_t = (size_t) object_Type_list->GetClientData(i);
		if(t_t == _t )
		{
			wxString s = object_Type_list->GetString( i );
			if( (int) s[0] == ' ' )
				OBJECT__ShowStates__Add_TYPE( _t );
			else
				OBJECT__ShowStates__Remove_TYPE( _t );
			break;
		}
	}
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ShowStates__Add_TYPE( int _t )
{
	for( size_t i = 0; i < object_Type_list->GetCount();i++)
	{
		int t_t = (size_t) object_Type_list->GetClientData(i);
		if(t_t == _t )
		{
			object_Type_list->SetString( i, wxT("# ") + Stage__Type_ToStr(t_t) );
			break;
		}
	}
	
	aSet_Hide_TYPE.insert( _t );
	VIEW__Update();
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ShowStates__Remove_TYPE( int _t )
{
	for( size_t i = 0; i < object_Type_list->GetCount();i++)
	{
		int t_t = (size_t) object_Type_list->GetClientData(i);
		if(t_t == _t )
		{
			object_Type_list->SetString( i, wxT("  ") + Stage__Type_ToStr(t_t) );
			break;
		}
	}
	
	aSet_Hide_TYPE.erase( _t );
	VIEW__Update();
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ListSelected__Set( ob_stage_object* obj )
{
	if( 	   OBJECT__ListSelected__IsIn(obj) == false
		|| aSet_Selected.size() != 1 )
	{
		aSet_Selected.clear();
		aSet_Selected.insert( obj );
		OBJECT__ListSelected__UpdateNoteBook();
		stageView->Refresh();
	}
}


//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ListSelected__Add( ob_stage_object* obj )
{
	if( aSet_Selected.find( obj ) == aSet_Selected.end() )
	{
		aSet_Selected.insert( obj );
		OBJECT__ListSelected__UpdateNoteBook();
		stageView->Refresh();
	}
}


//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ListSelected__Rm( ob_stage_object* obj )
{
	set<ob_stage_object*>::iterator it( aSet_Selected.find( obj ) );
	if( it != aSet_Selected.end() )
	{
		aSet_Selected.erase( it );
		OBJECT__ListSelected__UpdateNoteBook();
		stageView->Refresh();
	}
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ListSelected__Clear()
{
	if( aSet_Selected.empty() == false )
	{
		aSet_Selected.clear();
		NOTEBOOK__Set_NoSelection();
		stageView->OBJECTS__Refresh();
		
		Register_ControlsState_inHistory();
	}
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ListSelected__UpdateNoteBook()
{
	Register_ControlsState_inHistory();
	
	// Multi selected object
	if( aSet_Selected.size() > 1 )
		NOTEBOOK__Set_MultiSelection();
	
	else if( aSet_Selected.size() == 0 )
		NOTEBOOK__Set_NoSelection();
	
	// Single selected object
	else
	{
		NOTEBOOK__Set_With( *aSet_Selected.begin() );
	}
}

//-------------------------------------------------------
bool 
LevelEditorFrame::OBJECT__ListSelected__IsIn( ob_stage_object* obj )
{
	return (aSet_Selected.find( obj ) != aSet_Selected.end() );
}


//-------------------------------------------------------
int  
LevelEditorFrame::OBJECT__Get_SelectionState( ob_stage_object* obj )
{
	if( aSet_Selected.find( obj ) != aSet_Selected.end() )
		return SEL_ON;
	if( aSet_Highlighted.find(obj) != aSet_Highlighted.end() )
		return SEL_HIGHLIGHTED;
	
	return SEL_OFF;
}

//-------------------------------------------------------
void
LevelEditorFrame::OBJECT__ListSelected__Delete()
{
	if( aSet_Selected.size() == 0 || curr_stageFile == NULL )
		return;
	
	// Make the user sure
	wxString mess = wxT("Are you sure you want to\n");
	if( aSet_Selected.size() == 1 )
		mess += wxT("this object ??");
	else
		mess += wxT("these objects ??");
	
	int _reponse = wxMessageBox( mess
	, wxT("ATTENTION !")
			, wxICON_EXCLAMATION | wxYES_NO );

	if( _reponse != wxYES )
		return;
	
	theHistoryManager.GroupStart( wxT("Delete Object(s)"));
	
	NOTEBOOK__Set_NoSelection();
	curr_stageFile->changed = true;
	while( aSet_Selected.empty() == false )
	{
		ob_stage_object* o = *aSet_Selected.begin();
		aSet_Selected.erase( o );
		stageView->Delete_GuiObject_of( o ); 
		if( curr_associated_obj == o )
			NOTEBOOK__Set_NoSelection();
		o->Rm();
	}
	
	theHistoryManager.GroupEnd();
	
	TYPE__Reselect();
	stageView->Refresh();
}

//-------------------------------------------------------
void
LevelEditorFrame::OBJECT__ListSelected__Duplicate()
{
	if( aSet_Selected.empty() == true  || curr_stageFile == NULL )
		return;

	theHistoryManager.GroupStart( wxT("Duplicate Object(s)"));
	
	list<ob_stage_object*> t_news;
	while( aSet_Selected.empty() == false )
	{
		theHistoryManager.Set_State( false );
		ob_stage_object* src = *aSet_Selected.begin();
		aSet_Selected.erase( src );
		ob_stage_object* t = (ob_stage_object*)src->Clone();
		if( t->Is_Ob_Stage_Object() == false )
		{
			t->Rm();
			continue;
		}
		theHistoryManager.Set_State( true );
		t->Coords_Decal( wxSize( 40,0 ) );
		((ob_stage*)curr_stageFile->obj_container)
			->Add_SubObj( t, true );
		t_news.push_back( t );
	}

	while( t_news.empty() == false )
	{
		ob_stage_object* t = t_news.front();
		t_news.pop_front();
		aSet_Selected.insert( t );
	}

	theHistoryManager.GroupEnd();

	OBJECT__ListSelected__UpdateNoteBook();
	TYPE__Reselect();
	stageView->Refresh();
}

//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__ListSelected__Props(Ob_GAtom* obAtom)
{
	if(aSet_Selected.size() != 1  || curr_stageFile == NULL )
		return;
//		wxMessageBox( "Object \n" + obAtom->referer->obj->ToStr(), "DEBUG" );
	// Get the atom associate
	if( obAtom == NULL )
	{
		ob_object* so = *aSet_Selected.begin();
		list<Ob_GAtom*>::iterator it(stageView->stageView->l_atoms.begin())
					, it_end(stageView->stageView->l_atoms.end());
		for(;it!=it_end;it++ )
		{
			Ob_GAtom* o = *it;
			if( o != NULL && o->referer != NULL 
				&& o->referer->obj == so
				&& o->id_atom != GATOM_SHADOW
				&& o->id_atom != GATOM_AT
				)
			{
				obAtom = o;
				break;
			}
		}
	}
	
	if( obAtom == NULL )
		return;

	WndImgShow t(this,  obAtom->referer->obj->GetToken(0)
				, obAtom->lastImg
				, obAtom->ToStr() );
	t.ShowModal();
}


//*******************************************************
//*******************************************************
//-------------------------------------------------------
static
wxWindow*
ANN_newCombo(   wxWindow* parent, int max_w, const wxString& name,int more_style=0, bool b_not_RO = false )
{
	int style = wxBORDER_SUNKEN|wxCB_DROPDOWN|more_style;
	if( b_not_RO == false )
		style |= wxCB_READONLY;
	
	wxWindow* res =  new wxComboBox( 
					parent, wxID_ANY, wxString()
					, wxDefaultPosition, wxSize( max_w, -1 )
					, 0, NULL
					, style
					, wxDefaultValidator
					, name );
	return res;
}


//-------------------------------------------------------
static inline
wxString
ANN__GetItemHolded_Tag( ob_object* o )
{
	ob_object* so = o->GetSubObject( wxT("item") );
	if( so != NULL )
		return wxT("item");
	for( int i = 2; i <= 4; i++ )
	{
		wxString tag = IntToStr(i) + wxT("pitem");
		so = o->GetSubObject( tag );
		if( so != NULL )
			return tag;
	}
	return wxString();
}

//-------------------------------------------------------
static inline
wxString
ANN__GetItemHolded_Name( ob_object* o )
{
	wxString tag = ANN__GetItemHolded_Tag( o );
	if( tag == wxString() )
		return wxString();
	else
		return o->GetSubObject( tag )->GetToken( 0 );
}


//-------------------------------------------------------
static inline
size_t
ANN__GetNbRemap(obFileEntity* e )
{
	if( e == NULL )
		return 0;

	size_t res;
	ob_object** dummy = e->obj_container->GetSubObjectS( wxT("remap"), res );
	if( res > 0 )
	{
		delete[] dummy;
		return res;
	}
	
	dummy = e->obj_container->GetSubObjectS( wxT("alternatepal"), res );
	if( dummy != NULL )
		delete[] dummy;
	
	return res;
}

//-------------------------------------------------------
static
void
ANN__Refill_Map_ComboBox( wxComboBox* co,obFileEntity* ent )
{
	size_t nb_remaps = ANN__GetNbRemap( ent );
	if( co->GetCount() == nb_remaps + 1 )
		return;
		
	wxArrayString _displayed;
	wxArrayString _values;
	
	_displayed.Add( wxT("None") );
	_values.Add( wxT("0") );
	
	co->Clear();
	if( nb_remaps > 0 )
	{
		for( size_t i = 0; i < nb_remaps; i++ )
		{
			wxString v = IntToStr(i+1);
			_displayed.Add( v );
			_values.Add( v );
		}
	}

	ob_property::SetEnums( co, _displayed, _values );
	co->SetSelection(0);
}

//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Init()
{
	wxFont bigFont( 18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, true );
	wxNotebookPage* pg;
	wxSizer*	    sizer;
	wxSizer 	    *sizer_left,*sizer_center,*sizer_right,*sizer_sub;
	wxStaticText*   txtStat;
	wxWindow*       theCtrl;
	wxWindow*       combo;
	
	//-------------------------------------------------------------
	// No Selection Page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_TEXT] = pg;
	map_pages_names[PG_TEXT] = new wxString(wxT("OFF"));
	
	sizer = new wxBoxSizer( wxVERTICAL );
	pg->SetSizer(sizer);
	sizer->AddStretchSpacer();
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		sizer_sub->AddStretchSpacer();
		txtStat = new wxStaticText( pg, wxID_ANY, wxT("No Selection")
						, wxDefaultPosition, wxDefaultSize
						, wxALIGN_CENTRE|wxBORDER_RAISED
						, wxT("pg_text"));
		txtStat->SetFont( bigFont );
		sizer_sub->Add( txtStat );
		sizer_sub->AddStretchSpacer();
		sizer->Add(sizer_sub,0,wxEXPAND);
	sizer->AddStretchSpacer();
	
	map_pages_elts[PG_TEXT].push_back(wxT("pg_text"));
	
	//-------------------------------------------------------------
	//-------------------------------------------------------------
	// More Props page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_MORE] = pg;
	map_pages_names[PG_MORE] = new wxString(wxT("More"));
	
	sizer = new wxBoxSizer( wxVERTICAL );
	pg->SetSizer(sizer);
	more_props = new GridOb_ObjectProperties( pg );
	sizer->Add(more_props,1,wxEXPAND);
	
	//-------------------------------------------------------------
	// COORDS page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_ENT_COORDS] = pg;
	map_pages_names[PG_ENT_COORDS] = new wxString(wxT("Coords"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND );
		//--------------------------------------------------
		
		//--------------------------------------------------
		// AT
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"), 100, wxT("At"), 60 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_COORDS].push_back(wxT("TCSP:at=0"));

		//--------------------------------------------------
		// Coords
		sizer_sub = ob_props[wxT("SP:coords=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:coords=0"), 50, wxT("Coords"), 60 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		wxSizer* sizer_t = ob_props[wxT("SP:coords=1")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:coords=1"), 50 );
		delete sizer_t;
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_sub->Add(theCtrl );
		
		sizer_t = ob_props[wxT("SP:coords=2")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:coords=2"), 50 );
		delete sizer_t;
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_sub->Add(theCtrl );
		
		map_pages_elts[PG_ENT_COORDS].push_back(wxT("TCSP:coords=0"));
		map_pages_elts[PG_ENT_COORDS].push_back(wxT("TCSP:coords=1"));
		map_pages_elts[PG_ENT_COORDS].push_back(wxT("TCSP:coords=2"));

		//--------------------------------------------------
		// Flip
		sizer_sub = ob_props[wxT("SP:flip=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("CHSP:flip=0"),100, wxT("Flip"),80 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_CHECKBOX_CLICKED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		delete sizer_sub;
		sizer_left->Add(theCtrl, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_COORDS].push_back(wxT("CHSP:flip=0"));
		
		//--------------------------------------------------
		sizer_right = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_right, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------
		
		//--------------------------------------------------
		// entity name
		//--------------------------------------------------
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		sizer_right->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
		
		txtStat = new wxStaticText( pg, wxID_ANY, wxT("Entity")
						,wxDefaultPosition,wxSize(80,-1) );
		sizer_sub->Add(txtStat, 0, wxALIGN_CENTER );
		theCtrl = new wxTextCtrl( pg, wxID_ANY, wxString(),
						  wxDefaultPosition,wxSize(150,-1),
						  wxTE_READONLY,
						  wxDefaultValidator,
							wxT("entity_name")
						  );
		theCtrl->SetBackgroundColour( wxColour(230,230,230));
		sizer_sub->Add( theCtrl );
		
		map_pages_elts[PG_ENT_COORDS].push_back(wxT("entity_name"));
		
		//--------------------------------------------------
		// Alias
		sizer_sub = ob_props[wxT("SP:alias=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:alias=0"),150, wxT("Alias"),80 );
		sizer_right->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_COORDS].push_back(wxT("TCSP:alias=0"));
		
		//--------------------------------------------------
		// Remaps
		sizer_sub = ob_props[wxT("SP:map=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("COSP:map=0"),100, wxT("Remap"),80 );
		sizer_right->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_COMBOBOX_SELECTED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
						
			map_pages_elts[PG_ENT_COORDS].push_back(wxT("COSP:map=0"));
		

	//-------------------------------------------------------------
	// ENTITY infos page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_ENT_INFOS] = pg;
	map_pages_names[PG_ENT_INFOS] = new wxString(wxT("Params"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------
		
		//--------------------------------------------------
		// Health
		//--------------------------------------------------
		txtStat = new wxStaticText( pg, wxID_ANY, wxT("Health") );
		sizer_left->Add(txtStat, 0 );
		
		sizer_sub = ob_props[wxT("SP:health=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:health=0"), 50, wxT("1P") );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_INFOS].push_back(wxT("TCSP:health=0"));

		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:2phealth=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:2phealth=0"), 50, wxT("2P") );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_INFOS].push_back(wxT("TCSP:2phealth=0"));

		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:3phealth=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:3phealth=0"), 50, wxT("3P") );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_INFOS].push_back(wxT("TCSP:3phealth=0"));

		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:4phealth=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:4phealth=0"), 50, wxT("4P") );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_INFOS].push_back(wxT("TCSP:4phealth=0"));


		//--------------------------------------------------
		sizer_center = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_center, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// Nb Player min
		//--------------------------------------------------
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		sizer_center->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
		
		txtStat = new wxStaticText( pg, wxID_ANY, wxT("Nb Player Min") );
		sizer_sub->Add(txtStat, 0, wxALIGN_CENTER );
		
		combo = ANN_newCombo( pg, 50 , wxT("pspawn") );
		((wxComboBox*)combo)->Append(wxT("1"));
		((wxComboBox*)combo)->Append(wxT("2"));
		((wxComboBox*)combo)->Append(wxT("3"));
		((wxComboBox*)combo)->Append(wxT("4"));
		combo->Connect( 
			  wxEVT_COMMAND_COMBOBOX_SELECTED
			, wxCommandEventHandler(LevelEditorFrame::NOTEBOOK__Evt_pspawn)
			, NULL, leFrame );
		sizer_sub->Add(combo);
		map_pages_elts[PG_ENT_INFOS].push_back(wxT("pspawn"));

		//--------------------------------------------------
		// Agression
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:aggression=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:aggression=0"),60, wxT("Aggression"),100 );
		sizer_center->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_INFOS].push_back(wxT("TCSP:aggression=0"));

		//--------------------------------------------------
		// score
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:score=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:score=0"),60, wxT("Score"),100 );
		sizer_center->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
						
		map_pages_elts[PG_ENT_INFOS].push_back(wxT("TCSP:score=0"));

		//--------------------------------------------------
		// SIZER RIGHT
		sizer_right = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_right, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// MP
		sizer_sub = ob_props[wxT("SP:mp=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:mp=0"),50, wxT("MP") );
		sizer_right ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_ENT_INFOS].push_back(wxT("TCSP:mp=0"));


	//-------------------------------------------------------------
	// ITEM HOLDED page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_ENT_ITEMHOLD] = pg;
	map_pages_names[PG_ENT_ITEMHOLD] = new wxString(wxT("Item Hold"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------
		
		//--------------------------------------------------
		// name
		//--------------------------------------------------
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
		
		txtStat = new wxStaticText( pg, wxID_ANY, wxT("Name")
						,wxDefaultPosition,wxSize(100,-1) );
		sizer_sub->Add(txtStat, 0, wxALIGN_CENTER );
		
		combo = new ComboBoxAutoComplete( pg );
		combo->SetMinSize( wxSize(100,-1));
		combo->SetName(wxT("item_name"));
		combo->Connect( 
			wxEVT_COMBOBOXAUTOCOMPLETE_CHANGE
		    , wxCommandEventHandler(
				  LevelEditorFrame::NOTEBOOK__Evt_ItemName_Change
						)
		    , NULL, leFrame );
		combo->Connect( 
			wxEVT_COMBOBOXAUTOCOMPLETE_VALID
		    , wxCommandEventHandler(
				LevelEditorFrame::NOTEBOOK__Evt_ItemName_Valid
						)
		    , NULL, leFrame );
		
		// Fill the combo list
		for( size_t i = 0; i < arr_entities__size; i++ )
		{
			if( arr_entities[i] == NULL )
				continue;
			((ComboBoxAutoComplete*)combo)->Add( arr_entities[i]->Name() );
		}
		
		sizer_sub->Add(combo);
		map_pages_elts[PG_ENT_ITEMHOLD].push_back(wxT("item_name"));
		
		//--------------------------------------------------
		// itemalias
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:itemalias=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:itemalias=0"),100, wxT("itemalias"), 100 );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_ENT_ITEMHOLD].push_back(wxT("TCSP:itemalias=0"));
		
		//--------------------------------------------------
		// itemmap
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:itemmap=0")]->BuildControls(
		NULL, pg, theCtrl, wxT("COSP:itemmap=0"),80, wxT("itemmap"), 100 );
		sizer_left->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_ENT_ITEMHOLD].push_back(wxT("COSP:itemmap=0"));
		
		//--------------------------------------------------
		// SIZER RIGHT
		sizer_right = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_right, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------
		
		//--------------------------------------------------
		// Nb players min
		//--------------------------------------------------
		sizer_sub = new wxBoxSizer( wxHORIZONTAL );
		sizer_right->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );
		
		txtStat = new wxStaticText( pg, wxID_ANY, wxT("Nb Player Min ") );
		sizer_sub->Add(txtStat, 0, wxALIGN_CENTER );
		
		combo = ANN_newCombo( pg, 50 , wxT("pitem") );
		((wxComboBox*)combo)->Append(wxT("1"));
		((wxComboBox*)combo)->Append(wxT("2"));
		((wxComboBox*)combo)->Append(wxT("3"));
		((wxComboBox*)combo)->Append(wxT("4"));
		combo->Connect(
		      wxEVT_COMMAND_COMBOBOX_SELECTED
		    , wxCommandEventHandler(LevelEditorFrame::NOTEBOOK__Evt_Item_pitem)
		    , NULL, leFrame );
		sizer_sub->Add(combo);
		map_pages_elts[PG_ENT_ITEMHOLD].push_back(wxT("pitem"));

		//--------------------------------------------------
		// itemhealth
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:itemhealth=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:itemhealth=0"),60, wxT("itemhealth"), 100 );
		sizer_right ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_ENT_ITEMHOLD].push_back(wxT("TCSP:itemhealth=0"));
		
/*		
		//--------------------------------------------------
		// itemalpha
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:itemalpha=0")]->BuildControls(
					NULL, pg, theCtrl, "COitemalpha",50, "itemalpha", 100 );
		sizer_right ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_ENT_ITEMHOLD].push_back("COitemalpha");*/



	//-------------------------------------------------------------
	// GROUP page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_GROUP] = pg;
	map_pages_names[PG_GROUP] = new wxString(wxT("Group"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// AT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"),100, wxT("at"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_GROUP].push_back(wxT("TCSP:at=0"));

		//--------------------------------------------------
		// Min
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:group=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:group=0"),100, wxT("Min"), 100 );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_LIGHT].push_back(wxT("TCSP:group=0"));

		//--------------------------------------------------
		// Max
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:group=1")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:group=1"),100, wxT("Max"), 100 );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_LIGHT].push_back(wxT("TCSP:group=1"));

	//-------------------------------------------------------------
	// WAIT page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_WAIT] = pg;
	map_pages_names[PG_WAIT] = new wxString(wxT("Wait"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// AT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"),100, wxT("at"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WAIT].push_back(wxT("TCSP:at=0"));

	//-------------------------------------------------------------
	// JOIN page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_CANJOIN] = pg;
	map_pages_names[PG_CANJOIN] = new wxString(wxT("CanJoin"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// AT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"),100, wxT("at"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_CANJOIN].push_back(wxT("TCSP:at=0"));

	//-------------------------------------------------------------
	// NOJOIN page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_NOJOIN] = pg;
	map_pages_names[PG_NOJOIN] = new wxString(wxT("NoJoin"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// AT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"),100, wxT("at"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_NOJOIN].push_back(wxT("TCSP:at=0"));

	//-------------------------------------------------------------
	// BLOCKADE page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_BLOCKADE] = pg;
	map_pages_names[PG_BLOCKADE] = new wxString(wxT("Blockade"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// AT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"),100, wxT("at"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_BLOCKADE].push_back(wxT("TCSP:at=0"));

		//--------------------------------------------------
		// Pos
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:blockade=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:blockade=0"),100, wxT("pos"), 100 );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_BLOCKADE].push_back(wxT("TCSP:blockade=0"));

	//-------------------------------------------------------------
	// LIGHT page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_LIGHT] = pg;
	map_pages_names[PG_LIGHT] = new wxString(wxT("Light"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// AT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"),100, wxT("at"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_LIGHT].push_back(wxT("TCSP:at=0"));

		//--------------------------------------------------
		// X
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:light=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:light=0"),100, wxT("X"), 100 );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_LIGHT].push_back(wxT("TCSP:light=0"));

		//--------------------------------------------------
		// Z
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:light=1")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:light=1"),100, wxT("Z"), 100 );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_LIGHT].push_back(wxT("TCSP:light=1"));

	//-------------------------------------------------------------
	// SCROLLZ page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_SCROLLZ] = pg;
	map_pages_names[PG_SCROLLZ] = new wxString(wxT("Scrollz"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// AT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:at=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:at=0"),100, wxT("at"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_AT_Change
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_SCROLLZ].push_back(wxT("TCSP:at=0"));

		//--------------------------------------------------
		// X
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:scrollz=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:scrollz=0"),100, wxT("Min"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_SCROLLZ].push_back(wxT("TCSP:scrollz=0"));

		//--------------------------------------------------
		// Z
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP:scrollz=1")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP:scrollz=1"),100, wxT("Max"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_SCROLLZ].push_back(wxT("TCSP:scrollz=1"));


		
	//-------------------------------------------------------------
	// PLAYERS SPAWNS page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_PL_SPAWNS] = pg;
	map_pages_names[PG_PL_SPAWNS] = new wxString(wxT("Player Spawn"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------

		//--------------------------------------------------
		// X
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_spawnP:=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_spawnP:=0"),100, wxT("X"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_PL_SPAWNS].push_back(wxT("TCSP_spawnP:=0"));

		//--------------------------------------------------
		// Z
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_spawnP:=1")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_spawnP:=1"),100, wxT("Z"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_PL_SPAWNS].push_back(wxT("TCSP_spawnP:=1"));

		//--------------------------------------------------
		// Alt
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_spawnP:=2")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_spawnP:=2"),100, wxT("Alt"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_PL_SPAWNS].push_back(wxT("TCSP_spawnP:=2"));

	//-------------------------------------------------------------
	// WALLS/HOLES page
	//-------------------------------------------------------------
	pg = new wxPanel( onglets, wxID_ANY );
	pg->Hide();
	map_pages[PG_WALLS] = pg;
	map_pages_names[PG_WALLS] = new wxString(wxT("Wall/Hole"));
	
	sizer = new wxBoxSizer( wxHORIZONTAL );
	pg->SetSizer( sizer );

		//--------------------------------------------------
		sizer_left = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_left, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------
		//--------------------------------------------------
		// SIZER RIGHT
		sizer_right = new wxBoxSizer( wxVERTICAL );
		sizer->Add(sizer_right, 0,wxEXPAND|wxLEFT|wxRIGHT,10 );
		//--------------------------------------------------
		

		//--------------------------------------------------
		// X
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=0")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=0"),100, wxT("X"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=0"));

		//--------------------------------------------------
		// Z
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=1")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=1"),100, wxT("Z"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=1"));

		//--------------------------------------------------
		// UL
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=2")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=2"),100, wxT("UpperLeft"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_right ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=2"));

		//--------------------------------------------------
		// LL
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=3")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=3"),100, wxT("LowerLeft"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_right ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=3"));

		//--------------------------------------------------
		// UR
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=4")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=4"),100, wxT("UpperRight"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_right ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=4"));

		//--------------------------------------------------
		// LR
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=5")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=5"),100, wxT("LowerRight"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_right ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=5"));

		//--------------------------------------------------
		// Depth
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=6")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=6"),100, wxT("Depth"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=6"));

		//--------------------------------------------------
		// ALT
		//--------------------------------------------------
		sizer_sub = ob_props[wxT("SP_wall:=7")]->BuildControls( 
		NULL, pg, theCtrl, wxT("TCSP_wall:=7"),100, wxT("Alt"), 100 );
		theCtrl->Connect( 
			  wxEVT_COMMAND_TEXT_UPDATED
			, wxCommandEventHandler(
					LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange
						)
			, NULL, leFrame );
		sizer_left ->Add(sizer_sub, 0,wxEXPAND|wxALL,3 );

		map_pages_elts[PG_WALLS].push_back(wxT("TCSP_wall:=7"));


		
	//**************************************************
	// SET ASSOCIATIONS
	//**************************************************
	wxWindow* pg_ent_more = map_pages[PG_MORE];
	map_idPg_page[wxT("PG_MORE")] = pg_ent_more;
	wxWindow* pg_ent_infos = map_pages[PG_ENT_INFOS];
	map_idPg_page[wxT("PG_ENT_INFOS")] = pg_ent_infos;
	wxWindow* pg_ent_coords = map_pages[PG_ENT_COORDS];
	map_idPg_page[wxT("PG_ENT_COORDS")] = pg_ent_coords;
	wxWindow* pg_ent_itemhold = map_pages[PG_ENT_ITEMHOLD];
	map_idPg_page[wxT("PG_ENT_ITEMHOLD")] = pg_ent_itemhold;
	wxWindow* pg_wait = map_pages[PG_WAIT];
	map_idPg_page[wxT("PG_WAIT")] = pg_wait;
	wxWindow* pg_blockade = map_pages[PG_BLOCKADE];
	map_idPg_page[wxT("PG_BLOCKADE")] = pg_blockade;
	wxWindow* pg_scrollz = map_pages[PG_SCROLLZ];
	map_idPg_page[wxT("PG_SCROLLZ")] = pg_scrollz;
	wxWindow* pg_group = map_pages[PG_GROUP];
	map_idPg_page[wxT("PG_GROUP")] = pg_group;
	wxWindow* pg_light = map_pages[PG_LIGHT];
	map_idPg_page[wxT("PG_LIGHT")] = pg_light;
	wxWindow* pg_canjoin = map_pages[PG_CANJOIN];
	map_idPg_page[wxT("PG_CANJOIN")] = pg_canjoin;
	wxWindow* pg_nojoin = map_pages[PG_NOJOIN];
	map_idPg_page[wxT("PG_NOJOIN")] = pg_nojoin;

	//--------------------------------------------------
	map_type_l_pgs[SOBJ_NONE].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_NONE].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_NONE].push_back(wxT("TCSP:alias=0"));
	
	//--------------------------------------------------
	map_type_l_pgs[SOBJ_STEAMER].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_STEAMER].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_STEAMER].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_STEAMER].push_back(wxT("TCSP:alias=0"));
	map_type_l_disable[SOBJ_STEAMER].push_back(wxT("PG_ENT_INFOS"));
	
	map_type_l_enable[SOBJ_STEAMER].push_back(wxT("pspawn"));
	map_type_l_enable[SOBJ_STEAMER].push_back(wxT("TCSP:score=0"));

	//--------------------------------------------------
	map_type_l_pgs[SOBJ_PANEL].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_PANEL].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_PANEL].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_PANEL].push_back(wxT("TCSP:alias=0"));
	map_type_l_disable[SOBJ_PANEL].push_back(wxT("PG_ENT_INFOS"));
	
	map_type_l_enable[SOBJ_PANEL].push_back(wxT("pspawn"));

	//--------------------------------------------------
	map_type_l_pgs[SOBJ_TEXT].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_TEXT].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_TEXT].push_back(PG_ENT_ITEMHOLD);
	map_type_l_pgs[SOBJ_TEXT].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_TEXT].push_back(wxT("PG_ENT_INFOS"));
	map_type_l_disable[SOBJ_TEXT].push_back(wxT("TCSP:alias=0"));
	
	map_type_l_enable[SOBJ_TEXT].push_back(wxT("pspawn"));
	map_type_l_enable[SOBJ_PANEL].push_back(wxT("TCSP:score=0"));
	
	//--------------------------------------------------
	map_type_l_pgs[SOBJ_TRAP].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_TRAP].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_TRAP].push_back(PG_ENT_ITEMHOLD);
	map_type_l_pgs[SOBJ_TRAP].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_TRAP].push_back(wxT("PG_ENT_INFOS"));
	
	map_type_l_enable[SOBJ_TRAP].push_back(wxT("pspawn"));
	map_type_l_enable[SOBJ_PANEL].push_back(wxT("TCSP:score=0"));
	
	//--------------------------------------------------
	map_type_l_pgs[SOBJ_SHOT].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_SHOT].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_SHOT].push_back(PG_ENT_ITEMHOLD);
	map_type_l_pgs[SOBJ_SHOT].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_SHOT].push_back(wxT("PG_ENT_INFOS"));
	
	map_type_l_enable[SOBJ_SHOT].push_back(wxT("pspawn"));
	map_type_l_enable[SOBJ_PANEL].push_back(wxT("TCSP:score=0"));
	
	//--------------------------------------------------
	map_type_l_pgs[SOBJ_OBSTACLE].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_OBSTACLE].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_OBSTACLE].push_back(PG_ENT_ITEMHOLD);
	map_type_l_pgs[SOBJ_OBSTACLE].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_OBSTACLE].push_back(wxT("TCSP:aggression=0"));
	map_type_l_disable[SOBJ_OBSTACLE].push_back(wxT("TCSP:mp=0"));
	
	//--------------------------------------------------
	map_type_l_pgs[SOBJ_NPC].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_NPC].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_NPC].push_back(PG_ENT_ITEMHOLD);
	map_type_l_pgs[SOBJ_NPC].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_NPC].push_back(wxT("TCSP:mp=0"));
	
	//--------------------------------------------------
	map_type_l_pgs[SOBJ_ENEMY].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_ENEMY].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_ENEMY].push_back(PG_ENT_ITEMHOLD);
	map_type_l_pgs[SOBJ_ENEMY].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_ENEMY].push_back(wxT("TCSP:mp=0"));
	
	//--------------------------------------------------
	map_type_l_pgs[SOBJ_ITEM].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_ITEM].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_ITEM].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_ITEM].push_back(wxT("PG_ENT_INFOS"));
	
	map_type_l_enable[SOBJ_ITEM].push_back(wxT("pspawn"));
	map_type_l_enable[SOBJ_ITEM].push_back(wxT("TCSP:mp=0"));

	//--------------------------------------------------
	map_type_l_pgs[SOBJ_ENDLEVEL].push_back(PG_ENT_COORDS);
	map_type_l_pgs[SOBJ_ENDLEVEL].push_back(PG_ENT_INFOS);
	map_type_l_pgs[SOBJ_ENDLEVEL].push_back(PG_MORE);
	
	map_type_l_disable[SOBJ_ENDLEVEL].push_back(wxT("PG_ENT_INFOS"));
	
	map_type_l_enable[SOBJ_ENDLEVEL].push_back(wxT("pspawn"));

	//--------------------------------------------------
	map_type_l_pgs[SOBJ_WAIT].push_back(PG_WAIT);
	map_type_l_pgs[SOBJ_NOJOIN].push_back(PG_NOJOIN);
	map_type_l_pgs[SOBJ_CANJOIN].push_back(PG_CANJOIN);
	map_type_l_pgs[SOBJ_LIGHT].push_back(PG_LIGHT);
	map_type_l_pgs[SOBJ_SCROLLZ].push_back(PG_SCROLLZ);
	map_type_l_pgs[SOBJ_BLOCKADE].push_back(PG_BLOCKADE);
	map_type_l_pgs[SOBJ_GROUP].push_back(PG_GROUP);

//	map_type_l_pgs[SOBJ_SETPALETTE].push_back(PG_SETPALETTE);

	//-------------------------------------------------------------
	map_type_l_pgs[SOBJ_PLAYER_SPAWN].push_back(PG_PL_SPAWNS);

	//--------------------------------------------------
	map_type_l_pgs[SOBJ_WALL].push_back(PG_WALLS);

	//--------------------------------------------------
	map_type_l_pgs[SOBJ_HOLE].push_back(PG_WALLS);
// 	map_type_l_disable[SOBJ_HOLE].push_back("TCSP_wall:=6");
	map_type_l_disable[SOBJ_HOLE].push_back(wxT("TCSP_wall:=7"));
	
	//-------------------------------------------------------------
	// FINALLY
	NOTEBOOK__Set_NoSelection();
}

//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__UnInit()
{
	// ~wxWindow will take care of destroying pages childs
	map_pages.clear();
	
	map<int,wxString*>::iterator it;
	while( map_pages_names.empty() == false )
	{
		it = map_pages_names.begin();
		if( it->second != NULL )
			delete it->second;
		map_pages_names.erase(it);
	}
}


//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__ClearPages()
{
	// Must diassociate object and control
	for( size_t i = 0 ; i < onglets->GetPageCount(); i ++ )
	{
		wxWindow* pg = onglets->GetPage( i );
		wxWindowList& w_list = pg->GetChildren();
		wxWindowList::iterator it(w_list.begin())
					,it_end(w_list.end());
		for( ;it != it_end; it++)
		{
			if( (*it)->GetName() != wxString() )
				(*it)->SetClientData( NULL );
		}
	}
	
	while( onglets->GetPageCount() > 0 )
	{
		onglets->GetPage(0)->Hide();
		onglets->RemovePage(0);
	}
}

//-------------------------------------------------------
void 
LevelEditorFrame::NOTEBOOK__Save_Current_TypePage()
{
	// Save the current selected page for the previous object type
	if( curr_associated_obj != NULL )
/*
		if( 	map_type_last_active_pg.find(curr_associated_obj->object_type)
			!= 
			map_type_last_active_pg.end()
		  )
*/
			map_type_last_active_pg[curr_associated_obj->object_type] = 
						onglets->GetSelection();
}
					
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//*****************************************************************
//-------------------------------------------------------
inline static
void
ANN__ParseCtrlName( ob_object* o, const wxString& _w_name, wxString& ctrlType,wxString& ob_prop_name
,wxString& sobj_name,int& num_token)
{
	ctrlType  = _w_name.Left(2);
	ob_prop_name = _w_name.Right(_w_name.Len()-2);

	sobj_name = ob_prop_name;
	num_token = 0;
	
	int ind = sobj_name.find('=');
	if( ind != wxNOT_FOUND )
	{
		num_token = StrToInt( sobj_name.Right(sobj_name.Len()-ind-1));
		sobj_name = sobj_name.Left(ind);
	}
		
	ind = sobj_name.find(':');
	if( ind != wxNOT_FOUND )
	{
		if( sobj_name.Len()-ind-1 > 0 )
			sobj_name = sobj_name.Right(sobj_name.Len()-ind-1);
		else
			sobj_name = o->name;
	}
	
	
}


//-------------------------------------------------------
static void
ANN__Enable_ItemsCtrls( wxWindow* pg, bool b_enable )
{
	wxWindowList& w_list = pg->GetChildren();
	wxWindowList::iterator it(w_list.begin())
				,it_end(w_list.end());
	for( ;it != it_end; it++)
	{
		wxString w_name = (*it)->GetName();
		if( 	   w_name != wxString() 
			&& w_name != wxT("item_name")
			&& w_name != wxT("staticText") 
			)
			(*it)->Enable( b_enable );
	}
}

//-------------------------------------------------------
static void
ANN__Clear_ItemsCtrls( wxWindow* pg )
{
	wxWindowList& w_list = pg->GetChildren();
	wxWindowList::iterator it(w_list.begin())
				,it_end(w_list.end());
	for( ;it != it_end; it++)
	{
		if( (*it)->GetName() != wxString() )
		{
			wxString typ_Ctrl = (*it)->GetName().Left(2);
			if( typ_Ctrl == wxT("TC") )
				((wxTextCtrl*)(*it))->SetValue(wxString());
			else if( typ_Ctrl == wxT("CO") )
				((wxComboBox*)(*it))->SetSelection(0);
			else if( typ_Ctrl == wxT("CH") )
				((wxCheckBox*)(*it))->SetValue(false);
		}
	}
}


//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__SetPage_StaticVal( const wxString& wxW_name, const wxString& val)
{
	wxStaticText* t = (wxStaticText*) onglets->FindWindowByName( wxW_name, onglets );
	if( t == NULL )
		return;
	t->SetLabel( val );
	onglets->Refresh();
}

//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Set_NoSelection(bool b_init)
{
	if( (curr_associated_obj == NULL || curr_stageFile == NULL) && b_init == false )
		return;

	if(    onglets->GetPageCount() > 0
		&& onglets->GetPage(0) == map_pages[PG_TEXT] )
	{
		NOTEBOOK__SetPage_StaticVal( wxT("pg_text"), wxT("No Selection") );
		return;
	}

	if( b_init == false )
		NOTEBOOK__Save_Current_TypePage();
	
	// Check if the more page have changed some text
	if( b_init == false )
	{
		if( more_props->changed == true )
			curr_stageFile->changed = true;
		else if( curr_associated_obj->edited == true )
		{
			curr_stageFile->changed = true;
			curr_associated_obj->SetEdited( false );
		}
	}

	// Reset the props control
	more_props->Clear();
	more_props->changed = false;
	
	
	curr_page = PG_TEXT;
	NOTEBOOK__ClearPages();
	map_pages[curr_page]->Show();
	onglets->AddPage(  map_pages[curr_page]
				,*map_pages_names[curr_page]
				,true);
	NOTEBOOK__SetPage_StaticVal( wxT("pg_text"), wxT("No Selection") );
	curr_associated_obj = NULL;
}


//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Set_MultiSelection()
{
	if( curr_associated_obj == NULL || curr_stageFile == NULL )
		return;

	if( 	   onglets->GetPageCount() > 0
		&& onglets->GetPage(0) == map_pages[PG_TEXT] )
	{
		NOTEBOOK__SetPage_StaticVal( wxT("pg_text"), wxT("Multi Selection") );
		return;
	}
	
	NOTEBOOK__Save_Current_TypePage();
	curr_page = PG_TEXT;
	NOTEBOOK__ClearPages();
	map_pages[curr_page]->Show();
	onglets->AddPage(  map_pages[curr_page]
				,*map_pages_names[curr_page]
				,true);
				NOTEBOOK__SetPage_StaticVal( wxT("pg_text"), wxT("Multi Selection") );
	curr_associated_obj = NULL;
}

//-------------------------------------------------------
void 
LevelEditorFrame::NOTEBOOK__Set_With( ob_stage_object* obj )
{
	if( obj == curr_associated_obj )
		return;
	
	NOTEBOOK__Save_Current_TypePage();
	
	if( map_type_l_pgs.find( obj->object_type ) == map_type_l_pgs.end() )
	{
		wxMessageBox( wxT("Not Implemented for stage object type : ") + IntToStr(obj->object_type));
		return;
	}

	if( map_type_l_pgs[obj->object_type].size() == 0 )
	{
		wxMessageBox( wxT("No Page defined for stage object type : ") + IntToStr(obj->object_type));
		return;
	}

	m_flags |= LF_SETTINGS_TAGS;

	//----------------------------------------------------------
	// Have to rebuild the pages
	{
		NOTEBOOK__ClearPages();
		list<int> l_wnds =  map_type_l_pgs[obj->object_type];
		list<int>::iterator    it(l_wnds.begin())
					,it_end(l_wnds.end());
						
		// Add the page for this object
		for(;it!=it_end;it++)
		{
			if( map_pages[*it] != NULL )
			{
				map_pages[*it]->Show();
				onglets->AddPage( map_pages[*it], *map_pages_names[*it] );
			}
		}
	}


	//----------------------------------------------------------
	// Disable stuffs for this object
	if( true )
	{
		//----------------------------------------------------
		// First Reenable all
		list<int> l_wnds =  map_type_l_pgs[obj->object_type];
		list<int>::iterator    it(l_wnds.begin())
					,it_end(l_wnds.end());
		for(;it!=it_end;it++)
		{
			wxWindow* pg = map_pages[*it];
			if( pg == NULL )
				continue;
			
			wxWindowList& l_wnd = pg->GetChildren();
			wxWindowList::iterator it_w(l_wnd.begin())
						, it_w_end(l_wnd.end());
			for( ;it_w != it_w_end; it_w++ )
			{
				wxString w_name = (*it_w)->GetName();
				if( w_name == wxString() )
					continue;
				(*it_w)->Enable();
			}
		}
		
		//----------------------------------------------------
		// Disablers 
		if( 		map_type_l_disable.find(obj->object_type) 
			!=
				map_type_l_disable.end()
				)
		{
			list<wxString> l_disabled = map_type_l_disable[obj->object_type];
			list<wxString>::iterator it, it_end;
			
			it_end = l_disabled.end();

			for( it = l_disabled.begin(); it != it_end;it++)
			{
				wxString w_name = (*it);
				// Must deselect an entire page
				if( map_idPg_page.find(w_name) != map_idPg_page.end() )
				{
					wxWindow* pg = map_idPg_page[w_name];
					if( pg == NULL )
						continue;
					
					wxWindowList& l_wnd = pg->GetChildren();
					wxWindowList::iterator it_w(l_wnd.begin())
								, it_w_end(l_wnd.end());
					for( ;it_w != it_w_end; it_w++ )
					{
						if( (*it_w)->GetName() != wxString() )
							(*it_w)->Disable();
					}
					continue;
				}
				
				// Deselect only an element
				wxWindow* _wnd = onglets->FindWindowByName( w_name, onglets );
				if( _wnd == NULL )
				{
					wxMessageBox( wxT("BUG\nNOTEBOOK__Set_With() : Disabler \n\nNo Ctrl with name : ") + w_name );
					continue;
				}
				
				_wnd->Disable();
			}
		}

		//----------------------------------------------------
		// Enablers last
		if( 		map_type_l_enable.find(obj->object_type) 
			!=
				map_type_l_enable.end()
				)
		{
			list<wxString> l_enabled = map_type_l_enable[obj->object_type];
			list<wxString>::iterator it(l_enabled.begin())
							, it_end(l_enabled.end());
			
			for(; it != it_end;it++)
			{
				// Deselect only an element
				wxWindow* _wnd = onglets->FindWindowByName( *it, onglets );
				if( _wnd == NULL )
				{
					wxMessageBox( wxT("BUG\nNOTEBOOK__Set_With() : Enabler \n\nNo Ctrl with name : ") + *it );
					continue;
				}
				
				_wnd->Enable();
			}
		}
	}

	//----------------------------------------------------------
	// Change the associated object 
	curr_associated_obj = obj;
	
	//----------------------------------------------------------
	// Fill the Ctrl Fields
	list<int> l_wnds =  map_type_l_pgs[obj->object_type];
	list<int>::iterator    it(l_wnds.begin())
				,it_end(l_wnds.end());

	// list of stuff which have to been filter out from more_props
	wxArrayString to_filter_out;
	to_filter_out.Add(wxT("spawn"));
	
	// For each page
	for(;it!=it_end;it++)
	{
		wxWindow* pg = map_pages[*it];
		if( pg == NULL )
			continue;
		
		wxWindowList& l_wnd = pg->GetChildren();
		wxWindowList::iterator it_w(l_wnd.begin())
					, it_w_end(l_wnd.end());
		for( ;it_w != it_w_end; it_w++ )
		{
			wxString w_name = (*it_w)->GetName();
			if( w_name == wxString() || w_name == wxT("staticText") )
				continue;
			
			// First Some fucking specials case...
			if( w_name == wxT("entity_name" ))
			{
				to_filter_out.Add( wxT("spawn") );
				wxString ent_name = curr_associated_obj->GetToken(0);
				((wxTextCtrl*)(*it_w))->SetValue( ent_name );
				continue;
			}
			
			else if( w_name == wxT("pspawn") )
			{
				to_filter_out.Add( wxT("2pspawn" ));
				to_filter_out.Add( wxT("3pspawn" ));
				to_filter_out.Add( wxT("4pspawn" ));
				
				if( obj->GetSubObject( wxT("2pspawn")) != NULL 
					&& obj->GetSubObject(wxT("2pspawn"))->GetToken(0) != wxT("0") )
					((wxComboBox*)*it_w)->SetSelection( 1 );
				
				else if( obj->GetSubObject( wxT("3pspawn")) != NULL 
					&& obj->GetSubObject( wxT("3pspawn"))->GetToken(0) != wxT("0") )
					((wxComboBox*)*it_w)->SetSelection( 2 );
				else if( obj->GetSubObject( wxT("4pspawn")) != NULL 
					&& obj->GetSubObject( wxT("4pspawn"))->GetToken(0) != wxT("0") )
					((wxComboBox*)*it_w)->SetSelection( 3 );
				else
					((wxComboBox*)*it_w)->SetSelection( 0 );
				continue;
			}

			else if( w_name == wxT("item_name") )
			{
				to_filter_out.Add( wxT("item") );
				wxString item_name =
						ANN__GetItemHolded_Name(curr_associated_obj);
				if( item_name == wxString() )
					((ComboBoxAutoComplete*)*it_w)->Set_Value( wxString() );
				else
					((ComboBoxAutoComplete*)*it_w)->Set_Value( item_name);
				
				// If items page appears  ==>> Enable/Disable some Ctrls
				ANN__Enable_ItemsCtrls( map_pages[PG_ENT_ITEMHOLD] , item_name != wxString() );
			}
			
			else if( w_name == wxT("pitem") )
			{
				to_filter_out.Add( wxT("2pitem") );
				to_filter_out.Add( wxT("3pitem") );
				to_filter_out.Add( wxT("4pitem") );
				
				if( obj->GetSubObject( wxT("2pitem")) != NULL )
					((wxComboBox*)*it_w)->SetSelection( 1 );
				else if( obj->GetSubObject( wxT("3pitem")) != NULL )
					((wxComboBox*)*it_w)->SetSelection( 2 );
				else if( obj->GetSubObject( wxT("4pitem")) != NULL )
					((wxComboBox*)*it_w)->SetSelection( 3 );
				else
					((wxComboBox*)*it_w)->SetSelection( 0 );
				continue;
			}
			
			wxString ctrlType, ob_prop_name, sobj_name;
			int num_token;
			ANN__ParseCtrlName(curr_associated_obj,w_name,ctrlType, ob_prop_name,sobj_name,num_token);
			
			to_filter_out.Add( sobj_name );
			
			// have to refill the remap combobox
			if( w_name == wxT("COSP:map=0") )
			{
				wxComboBox* cb = (wxComboBox*)*it_w;
				
				ANN__Refill_Map_ComboBox(
						  cb
						, ((ob_spawn*) obj)->entity_ref
						);
			}
			
			// have also to refill the item remap combobox
			else if( w_name == wxT("COSP:itemmap=0") )
			{
				wxComboBox* cb = (wxComboBox*)*it_w;

				// Get the associated entity
				wxString item_name =
						ANN__GetItemHolded_Name(curr_associated_obj);
				ANN__Refill_Map_ComboBox( 
						  cb
						, Entity__Get( item_name ) 
						);
			}

			//-----------------------------------------------
			// associate the object to the control
			(*it_w)->SetClientData( (void*) curr_associated_obj);

			//-----------------------------------------------
			// Get the value for the control
			wxString val = wxString();
			bool b_def_val_spe = false;
			
			ob_property* t_prop = NULL;
			if( ob_props.find(ob_prop_name) != ob_props.end())
			{
				t_prop = ob_props[ob_prop_name];
				val = t_prop->Get_Curr_Value(curr_associated_obj);
			}

			
			// If there is a ob_props for this prop 
			//	=> Get the default value
			if( val == wxString() && t_prop != NULL )
			{
				// Check if there is a special default val
				val = t_prop->Get_Default_Val( curr_associated_obj );
				b_def_val_spe = t_prop->Have_SpecialDefault_Val(curr_associated_obj);
			}
				
			//-----------------------------------------------
			// Assign values to controls
			if( ctrlType == wxT("TC") )
			{
				((wxTextCtrl*)*it_w)->ChangeValue(val);
				if( b_def_val_spe == true && (*it_w)->IsEnabled() )
					(*it_w)->SetBackgroundColour( ob_property::default_values_color );
				else if( (*it_w)->IsEnabled() )
					(*it_w)->SetBackgroundColour( *wxWHITE );
			}
			else if( ctrlType == wxT("CO") )
			{
				int cb_ind = StrToInt(val);
/*				if( sobj_name.Upper() == "MAP" 
					|| sobj_name == "ITEMMAP" )
					cb_ind++;*/
				((wxComboBox*)*it_w)->SetSelection( cb_ind );
			}
			else if( ctrlType == wxT("CH") )
				((wxCheckBox*)*it_w)->SetValue( StrToBool(val) );
		}
	}
	
	more_props->SetObjectHandled( obj );
	more_props->SetFilter_Properties( to_filter_out );

	m_flags &= ~LF_SETTINGS_TAGS;
	onglets->Refresh();

	//----------------------------------------------------------
	// Select the best Page
	if( map_type_last_active_pg.find(obj->object_type) != map_type_last_active_pg.end() )
		onglets->SetSelection( map_type_last_active_pg[obj->object_type]);
	else
		onglets->SetSelection(0);


}


//-------------------------------------------------------
void 
LevelEditorFrame::NOTEBOOK__Update_Vals()
{
	// if no selection or multi selection ==>> nothing to do
	if( 	   onglets->GetPageCount() == 0
		|| onglets->GetPage(0) == map_pages[PG_TEXT] )
		return;
	
	// Some real page are here, lets each control make the update
	size_t nb_pgs = onglets->GetPageCount();
	for( size_t i = 0; i < nb_pgs; i++)
	{
		wxNotebookPage* pg = onglets->GetPage(i);
		
		wxWindowList& w_list = pg->GetChildren();
		wxWindowList::iterator it(w_list.begin())
					,it_end(w_list.end());
		for( ;it != it_end; it++)
		{
			if( (*it)->IsEnabled() == false )
				continue;
			
			if( (*it)->GetName() != wxString() )
			{
				wxString typ_Ctrl = (*it)->GetName().Left(2);
				if( 	   typ_Ctrl != wxT("TC")
					&& typ_Ctrl != wxT("CO")
					&& typ_Ctrl != wxT("CH")
				   )
					continue;
					
				wxString ctrlType, ob_prop_name, sobj_name;
				int num_token;
				ANN__ParseCtrlName(curr_associated_obj,(*it)->GetName(),ctrlType, ob_prop_name,sobj_name,num_token);

				if( ob_props.find(ob_prop_name) == ob_props.end())
					continue;
				ob_property* t_prop = ob_props[ob_prop_name];
				t_prop->Update_CtrlVal( (*it) );
			}
		}
	}
}


//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//*********************************************************************
//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Evt_ObjVisualChange( wxCommandEvent& evt )
{
	if( m_flags & LF_SETTINGS_TAGS || curr_associated_obj == NULL )
		return;
	
	evt.Skip();
	
	wxWindow* theCtrl = (wxWindow*) evt.GetEventObject();
	ob_object* obj = (ob_object*) theCtrl->GetClientData();
	if( obj == NULL )
	{
		wxMessageBox( wxT("BUG !!!\nNOTEBOOK__Evt_ObjVisualChange()\n\nobj != curr_associated_obj\n" ));
		return;
	}
	
	VIEW__Update();
}

//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Evt_AT_Change( wxCommandEvent& evt )
{
	if( m_flags & LF_SETTINGS_TAGS || curr_associated_obj == NULL )
		return;
	evt.Skip();
	
	wxWindow* theCtrl = (wxWindow*) evt.GetEventObject();
	ob_object* obj = (ob_object*) theCtrl->GetClientData();
	if( obj == NULL )
	{
		wxMessageBox( wxT("BUG !!!\nNOTEBOOK__Evt_AT_Change()\n\nobj != curr_associated_obj\n" ));
		return;
	}
	
	// Must change the object to his right place
	if( ob_stage::Is_Stage_At_Type( obj ) == true )
		((ob_stage_at*)obj)->Reorder_At();
	
	VIEW__Update();
	
}


//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Evt_ItemName_Change( wxCommandEvent& evt )
{
	//------------------------------------------------------------
	// Some preventives checks
	if( m_flags & LF_SETTINGS_TAGS || curr_associated_obj == NULL )
		return;

	ComboBoxAutoComplete* theCtrl = (ComboBoxAutoComplete*) evt.GetEventObject();
	if( theCtrl->IsShownOnScreen() == false )
		return;

	//------------------------------------------------------------
	// Mark the object as modified
	curr_stageFile->changed = true;

	//------------------------------------------------------------
	// Affect the change in the object
	wxString item_name = theCtrl->Get_Value();
	wxString item_tag = ANN__GetItemHolded_Tag( curr_associated_obj );
	ob_object* obj_item = curr_associated_obj->GetSubObject( item_tag );
	if( obj_item == NULL )
	{
		obj_item = new ob_object( wxT("item" ));
		curr_associated_obj->Add_SubObj( obj_item );
	}
	obj_item->SetToken( 0, item_name );
	
	ANN__Enable_ItemsCtrls( map_pages[PG_ENT_ITEMHOLD] , item_name != wxString() );
	
	//------------------------------------------------------------
	// Must refresh map combobox
	obFileEntity* ent =  Entity__Get( item_name );
	
	wxComboBox* cbItemMap = 
	(wxComboBox*) wxWindow::FindWindowByName( wxT("COSP:itemmap=0"), onglets );
	if( cbItemMap == NULL )
	{
		wxMessageBox( wxT("BUG !!!\nNOTEBOOK__Evt_ObjVisualChange()\n\ncbItemMap == NULL\n") );
		return;
	}
	
	// No valid entity for this item_name
	if( ent == NULL )
	{
		if( cbItemMap->GetCount() != 1 )
		{
			cbItemMap->Clear();
			wxArrayString _displayed;
			wxArrayString _values;
			_displayed.Add( wxT("None" ));
			_values.Add( wxT("0") );
			ob_property::SetEnums(cbItemMap,_displayed,_values);
			cbItemMap->SetSelection( 0 );
		}
		return;
	}
	
	// Got a valid entity  ==>> Refill the itemmap combobox
	ANN__Refill_Map_ComboBox( cbItemMap, ent );
}

//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Evt_ItemName_Valid( wxCommandEvent& evt )
{
	//------------------------------------------------------------
	// Some preventives checks
	if( m_flags & LF_SETTINGS_TAGS || curr_associated_obj == NULL )
		return;

	ComboBoxAutoComplete* theCtrl = (ComboBoxAutoComplete*) evt.GetEventObject();
	if( theCtrl->IsShownOnScreen() == false )
		return;
	
	wxString item_name = ANN__GetItemHolded_Name( curr_associated_obj );

	// Nothing to do if there's a valid name 
	if( item_name != wxString() )
		return;
	
	//------------------------------------------------------------
	// Clear all Controls
	ANN__Clear_ItemsCtrls( map_pages[PG_ENT_ITEMHOLD] );
	
	//------------------------------------------------------------
	// Also delete all subobj related in the current object
	ob_object* subobj;

	list<wxString>::iterator it(map_pages_elts[PG_ENT_ITEMHOLD].begin())
				, it_end (map_pages_elts[PG_ENT_ITEMHOLD].end());
	
	for(;it != it_end; it++)
	{
		// SPECIAL CASE
		if( *it == wxT("item_name") )
		{
			wxString item_tag = ANN__GetItemHolded_Tag( curr_associated_obj );
			subobj = curr_associated_obj->GetSubObject( item_tag );
		}
		
		// SPECIAL CASE
		else if( *it == wxT("pitem") )
		{
			wxWindow* _w = wxWindow::FindWindowByName( *it, onglets );
			((wxComboBox*)_w)->SetSelection( 0 );
			continue;
		}
		
		// GENERAL CASE
		else
		{
			wxWindow* _w = wxWindow::FindWindowByName( *it, onglets );
			if( _w == NULL )
			{
				wxMessageBox( wxT("BUG !!!\nNOTEBOOK__Evt_ItemName_Valid()\n\n_w == NULL\n") );
				continue;
			}
			
			wxString w_name = _w->GetName();
			wxString ctrlType, ob_prop_name, sobj_name;
			int num_token;
			ANN__ParseCtrlName(curr_associated_obj,w_name,ctrlType, ob_prop_name,sobj_name,num_token);
			
			subobj = curr_associated_obj->GetSubObject( ob_prop_name );
			
			if( ctrlType == wxT("TC"))
				((wxTextCtrl*)_w)->ChangeValue(wxString());
			else if( ctrlType == wxT("CO"))
				((wxComboBox*)_w)->SetSelection(0);
			else if( ctrlType == wxT("CH"))
				((wxCheckBox*)_w)->SetValue(false);
		}
		
		if( subobj != NULL )
			subobj->Rm();
	}
}

//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Evt_Item_pitem( wxCommandEvent& evt )
{
	//------------------------------------------------------------
	// Some preventives checks
	if( m_flags & LF_SETTINGS_TAGS || curr_associated_obj == NULL )
		return;
	
	// Get the comboBox associated and its name
	wxComboBox* co = (wxComboBox*) evt.GetEventObject();
	wxString w_name = co->GetName();
	if( w_name == wxString() )
		return;
	if( co->IsShownOnScreen() == false )
		return;

	//------------------------------------------------------------
	// Mark the object as modified
	curr_stageFile->changed = true;
	
	//------------------------------------------------------------
	// Get the item name...
	wxString item_name = ANN__GetItemHolded_Name(curr_associated_obj);
	
	//------------------------------------------------------------
	// Reflect change
	int ind = co->GetSelection();
	
	// Try with simple "item" tag
	wxString curr_item_tag = ANN__GetItemHolded_Tag(curr_associated_obj);
	if( curr_item_tag == wxString() )
	{
		wxMessageBox( wxT("BUG!!\nNOTEBOOK__Evt_pitem()\n\ncurr_item_tag == wxEmptyString!\n" ));
		return;
	}
	ob_object* subobj = curr_associated_obj->GetSubObject(curr_item_tag);
	if( curr_item_tag == wxString() )
	{
		wxMessageBox( wxT("BUG!!\nNOTEBOOK__Evt_pitem()\nsubobj == NULL !\n" ));
		return;
	}
	
	// The wanted declaration name for the item
	wxString subobjName = IntToStr( ind + 1 ) + wxT("pitem");
	if( ind == 0 )
		subobjName = wxT("item");

	// Have to rename it
	subobj->SetName( subobjName );
}

//-------------------------------------------------------
void
LevelEditorFrame::NOTEBOOK__Evt_pspawn( wxCommandEvent& evt )
{
	//------------------------------------------------------------
	// Some preventives checks
	if( m_flags & LF_SETTINGS_TAGS || curr_associated_obj == NULL )
		return;
	
	wxComboBox* co = (wxComboBox*) evt.GetEventObject();
	if(co->GetName() == wxString() )
		return;
	if( co->IsShownOnScreen() == false )
		return;

	//------------------------------------------------------------
	// Mark the object as modified
	curr_stageFile->changed = true;

	//------------------------------------------------------------
	// Reflect the change in the current object
	int ind = co->GetSelection();
	for( int i = 2; i <= 4; i++) 
	{
		wxString tok_name = IntToStr(i) + co->GetName();;
		ob_object* o = curr_associated_obj->GetSubObject(tok_name);
		
		if( i-1 == ind )
		{
			if( o == NULL )
			{
				curr_associated_obj->Add_SubObj( 
				new ob_object( tok_name, wxT("1") ) );
			}
		}
		else	if( o != NULL )
			o->Rm();
	}
}


//*******************************************************
//*******************************************************
//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__Edit_Settings()
{
}


//-------------------------------------------------------
void 
LevelEditorFrame::OBJECT__Edit_ExtendedSettings()
{
}


//****************************************************
//-------------------------------------------------------
void
LevelEditorFrame::ENTITIES__Load()
{
	theHistoryManager.Set_State( false );
	
	if( fileModels == NULL )
		return;
	
	list<obFileEntity* >* l_ents = obFileStage::GetEntity_ALL(this);
	
	if( l_ents->size() == 0 )
	{
		delete l_ents;
		arr_entities = NULL;
		arr_entities__size = 0;
		theHistoryManager.Set_State( true );
		return;
	}
	
	arr_entities__size = l_ents->size();
	arr_entities = new obFileEntity*[arr_entities__size];
	
	size_t ind = 0;
	while(l_ents->empty() == false )
	{
		arr_entities[ind++] = l_ents->front();
		l_ents->pop_front();
	}
	
	delete l_ents;
	theHistoryManager.Set_State( true );
	return;
}

//-------------------------------------------------------
void
LevelEditorFrame::ENTITIES__UnLoad()
{
/*	for( size_t i = 0; i < arr_entities__size; i++ )
		delete arr_entities[i];*/
	arr_entities__size = 0;
	if( arr_entities != NULL )
		delete[] arr_entities;
	arr_entities = NULL;
}

//-------------------------------------------------------
void 
LevelEditorFrame::ENTITIES__FILTER__Apply()
{
	if( arr_entities == NULL )
		return;
	
	// Emtpy the current list
	entities_list->Clear();
	
	// The text Filter
	wxString txt_filter = entities_filter->GetValue();
	
	// The type filter
	int _type = STAGETYPE_NONE;
	wxString str_type = cbbox_entities_type->GetStringSelection();
	if( str_type != wxT("ALL") )
		_type = Stage__Type_ToInt( str_type );
	
	for( size_t i = 0; i < arr_entities__size; i++ )
	{
		wxString ent_name = arr_entities[i]->Name();
		// Check if it pass the text Filter
		if( txt_filter != wxString() )
			if( ent_name.Upper().Find(txt_filter.Upper()) == wxNOT_FOUND )
				continue;
		
		// Check if it pass the Type Filter
		
		if( _type != STAGETYPE_NONE )
		{
			int ent_stype = ob_spawn::Get_Entity_StageType(ent_name);
			if( ent_stype != _type )
				continue;
		}
		
		entities_list->Append( ent_name, (void*) arr_entities[i] );
	}
}


//------------------------------------------------------------------
void
LevelEditorFrame::OBJECT__Add_Entity(obFileEntity* ent )
{
	if( curr_stageFile == NULL || b_closing == true )
		return;
	
	theHistoryManager.GroupStart( wxT("Add an Entity") );
	
	ob_object* o = ob_stage::Guess_and_ConstructNewObject( wxT("spawn"), ent->Name() );
	if( o == NULL )
	{
		wxMessageBox( wxT("BUG !!\nLevelEditorFrame::OBJECT__Add_Entity()\no == NULL") );
		theHistoryManager.GroupEnd();
		return;
	}
	if( ob_stage::Is_Stage_Spawn_Type( o ) == false )
	{
		wxMessageBox( wxT("BUG !!\nLevelEditorFrame::OBJECT__Add_Entity()\nIs_Stage_Spawn_Type( o ) == false ") );
		o->Rm();
		theHistoryManager.GroupEnd();
		return;
	}
	
	ob_spawn* os = (ob_spawn*) o;
	((ob_stage*)curr_stageFile->obj_container)->Add_SubObj( os );
	wxSize view_coords( stageView->stageView->coords );
	wxSize view_size(   stageView->stageView->Get_ObSize() );
	wxSize view_center(	view_coords.x+view_size.x/2
				    , view_coords.y+view_size.y/2
				    );
	os->Init_and_Center( view_center );
	os->Reorder_At();
	// Replace AT at the end of the object
	OBJECT__ListSelected__Set( os );
	NOTEBOOK__Set_With( os );

	theHistoryManager.GroupEnd();
}


//------------------------------------------------------------------
void
LevelEditorFrame::OBJECT__Add_Default_of_Type( int _TYPE )
{
	if( curr_stageFile == NULL || b_closing == true )
		return;

	ob_stage* stg = (ob_stage*) curr_stageFile->obj_container;
	if( stg == NULL )
		return;
	
	theHistoryManager.GroupStart( wxT("Add an object" ));
	
	ob_stage_object* o = NULL;
	wxArrayString as;
	wxString _name;
	switch( _TYPE )
	{
		case SOBJ_SETPALETTE:
			_name = wxT("SetPalette");
			o = new ob_stage_at( _name, as );
			break;
		case SOBJ_WALL:
			wxArraystring__Add( as,8, wxT("0"),wxT("0"),wxT("0"),wxT("0"),wxT("0"),wxT("0"),wxT("0"),wxT("0"));
			_name = wxT("Wall");
			o = new ob_wall( _name, as );
			break;
		case SOBJ_HOLE:
			wxArraystring__Add( as,7, wxT("0"),wxT("0"),wxT("0"),wxT("0"),wxT("0"),wxT("0"),wxT("0"));
			_name = wxT("Hole");
			o = new ob_hole( _name, as );
			break;
		case SOBJ_WAIT:
			_name = wxT("Wait");
			o = new ob_stage_at( _name, as );
			break;
		case SOBJ_NOJOIN:
			_name = wxT("NoJoin");
			o = new ob_stage_at( _name, as );
			break;
		case SOBJ_CANJOIN:
			_name = wxT("CanJoin");
			o = new ob_stage_at( _name, as );
			break;
		case SOBJ_SCROLLZ:
			wxArraystring__Add( as,2,wxT("30"),wxT("70"));
			_name = wxT("ScrollZ");
			o = new ob_stage_at( _name, as );
			break;
		case SOBJ_BLOCKADE:
			_name = wxT("Blockade");
			o = new ob_stage_at( _name, as );
			break;
		case SOBJ_GROUP:
			wxArraystring__Add( as,1,wxT("0"));
			_name = wxT("Group");
			o = new ob_stage_at( _name, as );
			break;
		case SOBJ_LIGHT:
			wxArraystring__Add( as,1,wxT("0"));
			_name = wxT("Light");
			o = new ob_stage_at( _name, as );
			break;

		// Special !!
		case SOBJ_PLAYER_SPAWN:
		{
			// Have to ask the player num
			int ind =
			wxGetSingleChoiceIndex( wxT("Spawn point for which player ?")
			,wxT("Question !")
			, wxArrayString_Build( 4, wxT("1"),wxT("2"),wxT("3"),wxT("4") )
							,this
							);
			if( ind < 0 || ind >= 4 )
			{
				theHistoryManager.GroupEnd();
				return;
			}
			_name = wxT("spawn") + IntToStr( ind + 1 );
			if( 	stg->GetSubObject( _name )
				!=
				NULL
			  )
			{
				wxMessageBox( wxT("There is already a spawn point for this player !")
				, wxT("Impossible"), wxOK | wxICON_INFORMATION, leFrame );
				return;
			}
			wxArraystring__Add( as, 2, wxT("0"),wxT("0") );
			o = new ob_player_spawn( _name, as );
			break;
		}
	}
	
	if( o == NULL )
	{
		theHistoryManager.GroupEnd();
		return;
	}
	
	stg->Add_SubObj( o );

	// Center the object
	wxSize view_coords( stageView->stageView->coords );
	wxSize view_size(   stageView->stageView->Get_ObSize() );
	wxSize view_center(	view_coords.x+view_size.x/2
				    , view_coords.y+view_size.y/2 );
	o->Init_and_Center( view_center );
	if( ob_stage::Is_Stage_At_Type( o ) == true )
		((ob_stage_at*)o)->Reorder_At();

	// Select the object
	OBJECT__ListSelected__Set( o );
	NOTEBOOK__Set_With( o );

	theHistoryManager.GroupEnd();
}

