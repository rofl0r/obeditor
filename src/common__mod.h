#ifndef OB_MODS_H_
#define OB_MODS_H_

#include <wx/filename.h>

class Mod
{
public:
	Mod( const wxString& name );
	Mod( const wxString& _name, const wxString& _dataDir );
	bool SetDir( const wxString& _dataDir );
	bool SetName( const wxString& _name );
	wxFileName dirData;
	wxString name;
	
	wxString models_txt;
	wxString levels_txt;
	wxString bgs_dir;
	wxString scenes_dir;
	int colourdepth;
	wxSize video_res;

protected:
	void ZeroInit();
	void Load_Video_txt();
};
extern Mod* curr_mod;

class ModsList
{
public:
	ModsList();
	bool ModExist( const wxString& )const;
	bool Append( const wxString& modName, const wxString& dataDir);
	bool Delete( const wxString& modName);
	int GetIndex( const wxString& modName)const;
	wxFileName GetModPath( const wxString& modName);
	wxArrayString GetProjectsNames()const;
	wxFileName GetSelectedProject();
	wxString   GetSelectedProjectName();
	int Reload();
	Mod* Get(int i);
	Mod* GetCurrentMod();

private:
	Mod** arr_mods;
	int	arr_mods_size;
};

extern ModsList *theModsList;

#endif /*OB_MODS_H_*/
