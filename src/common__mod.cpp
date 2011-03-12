#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "ob_editor.h"
#include "common__mod.h"

int MAX_MODS_LIST = 50;
ModsList *theModsList;
Mod* curr_mod;

Mod::Mod( const wxString& _name )
{
	name = _name;
	wxString t;
	config->Read(wxT("/Mod/") + name +wxT("/dirData"), &t );
	dirData = t;
	
	ZeroInit();
	Load_Video_txt();
}

//-----------------------------------------------
Mod::Mod( const wxString& _name, const wxString& _dataDir )
{
	name = _name;
	dirData = _dataDir;
	ZeroInit();
}

//-----------------------------------------------
void
Mod::ZeroInit()
{
	models_txt = wxT("models.txt");
	levels_txt = wxT("levels.txt");
	bgs_dir = wxString();
	scenes_dir = wxString();
	colourdepth = 8;
	video_res.x = 320;
	video_res.y = 240;
}

//-----------------------------------------------
// Try Load video.txt
void
Mod::Load_Video_txt()
{
	wxFileName fn( GetObFile( wxT("data/video.txt") ) );
	if( fn.FileExists() == false )
		return;
	obFileVideoTxt video_txt( fn );
	if( video_txt.obj_container == NULL && video_txt.nb_lines == 0 )
		return;
	
	ob_object* subobj;
	
	subobj = video_txt.GetProperty( wxT("models") );
	if( subobj != NULL )
		models_txt = subobj->GetToken( 0 );
	
	subobj = video_txt.GetProperty( wxT("levels") );
	if( subobj != NULL )
		levels_txt = subobj->GetToken( 0 );
	
	subobj = video_txt.GetProperty( wxT("backgrounds") );
	if( subobj != NULL )
		bgs_dir = subobj->GetToken( 0 );
	
	subobj = video_txt.GetProperty( wxT("scenes" ));
	if( subobj != NULL )
		scenes_dir = subobj->GetToken( 0 );
	
	subobj = video_txt.GetProperty( wxT("colourdepth" ));
	if( subobj != NULL )
	{
		wxString t = subobj->GetToken( 0 );
		if( t.Upper() == wxT("8BIT" ))
			colourdepth = 8;
		else if( t.Upper() == wxT("16BIT" ))
			colourdepth = 16;
		else if( t.Upper() == wxT("32BIT") )
			colourdepth = 32;
	}
	
	
	subobj = video_txt.GetProperty( wxT("video") );
	if( subobj != NULL )
	{
		int video = StrToInt( subobj->GetToken(0));
		switch( video )
		{
			case 1:
				video_res = wxSize(480,272);
				break;
			case 2:
				video_res = wxSize(640,480);
				break;
			case 3:
				video_res = wxSize(720,480);
				break;
			case 4:
				video_res = wxSize(800,480);
				break;
			case 5:
				video_res = wxSize(800,600);
				break;
			case 6:
				video_res = wxSize(960,540);
				break;
			default:
				video_res = wxSize(320,240);
				break;
		}
	}
}

//-----------------------------------------------
bool Mod::SetDir( const wxString& _dataDir )
{
	dirData = _dataDir;
	Load_Video_txt();
	return true;
}

//-----------------------------------------------
bool Mod::SetName( const wxString& _name )
{
	name = _name;
	return true;
}


/**
 * Manager of Mods loaded.
 */
ModsList::ModsList()
{
	arr_mods = new Mod*[MAX_MODS_LIST];
	arr_mods_size = 0;
//	config->DeleteAll();
	Reload();
}

int ModsList::Reload()
{
	for( int i = 0; i < arr_mods_size; i++ )
	{
		delete arr_mods[i];
		arr_mods[i] = NULL;
	}

	arr_mods_size = 0;
	int i = 0;
	while( i>= 0 )
	{
		wxString cfg_path = wxT("/Mod") + IntToStr(i);
		if( ! config->Exists( cfg_path ) )
			i= -1;
		else
		{
			wxString name;
			config->Read( cfg_path, &name );
			if( ! ModExist( name))
			{
				Mod * temp = new Mod( name );
				arr_mods[arr_mods_size] = temp;
				arr_mods_size++;
			}
			i++;
		}
	}
	return arr_mods_size;
}

wxArrayString ModsList::GetProjectsNames()const
{
	wxArrayString res;
	for( int i =0; i < arr_mods_size; i++)
		res.Add( arr_mods[i]->name );
	return res;
}

/**
 * Test if a mod with the same name already exists in the list.
 */
bool ModsList::ModExist( const wxString& modName) const
{
	if( GetIndex( modName) >= 0 )
		return true;
	return false;
}


/**
 * Add a mod to the list.
 */
bool ModsList::Append( const wxString& modName, const wxString& dataDir)
{
	if( arr_mods_size >= MAX_MODS_LIST )
		return false;

	Mod * temp = new Mod( modName, dataDir );
	if( temp )
	{
		arr_mods[arr_mods_size] = temp;
		arr_mods_size++;
		return true;
	}
	return false;
}


/**
 * Remove a mod from the list.
 */
bool ModsList::Delete( const wxString& modName)
{
	int ind = GetIndex( modName);
	if( ind < 0 )
		return false;

	Mod * temp = arr_mods[ind];
	delete temp;
	for( int i = ind; i< arr_mods_size - 1; i ++)
	{
		config->RenameEntry(  wxT("Mod") + IntToStr(i +1), wxT("Mod") + IntToStr(i) );
		arr_mods[i] = arr_mods[i+1];
	}
	config->DeleteEntry( wxT("/Mod") + IntToStr(arr_mods_size -1) );
	arr_mods[arr_mods_size -1] = NULL;
	arr_mods_size--;
	return true;
}

/**
 * Get the index of the mod in the list.
 *
 * Return -1 if it doesn't exist
 */
int ModsList::GetIndex( const wxString& modName)const
{
	for( int i =0; i < arr_mods_size; i++)
	{
		if( arr_mods[i]->name == modName )
			return i;
	}
	return -1;
}

wxFileName ModsList::GetModPath( const wxString& modName)
{
	int i = GetIndex( modName);
	if( i< 0 )
		return wxFileName();

	Mod* _mod = Get( i );
	if( _mod == NULL )
		return wxFileName();

	return _mod->dirData;
}

Mod* ModsList::Get(int i)
{
	if( i<0 || i>= arr_mods_size )
		return (Mod*)NULL;

	return arr_mods[i];
}

wxFileName 
ModsList::GetSelectedProject()
{
	wxString curr_selection = startFrame->lstBx_projects->GetStringSelection();
	return GetModPath( curr_selection );
}

wxString ModsList::GetSelectedProjectName()
{
	return startFrame->lstBx_projects->GetStringSelection();
}

Mod* 
ModsList::GetCurrentMod()
{
	int i = startFrame->lstBx_projects->GetSelection();
	return Get(i);
}
