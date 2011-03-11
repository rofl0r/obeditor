#ifndef LEVELS__CTRL_SCENEVIEW
#define LEVELS__CTRL_SCENEVIEW

#include <map>	
#include "common__object_stage.h"

//*************************************************************************
class Ctrl_SceneView;
//*************************************************************************

class Panel_SceneView : public wxPanel
{
public:
	Panel_SceneView( 
			  wxWindow* _parent
			, ob_stage* _stage
			, ob_StageDeclaration* _stage_declaration
			, ob_StagesSet* _stage_set
			);
			
	~Panel_SceneView();
	
	void Update_View();
	
	void Set_Panels( ob_stage_panel**& p_panels, size_t& p_nb_panels );
	void Set_Background( ob_BG_Layer**& p_bg_layers, size_t& p_nb_layers );
	void Set_Front_Panels(ob_front_panel**& p_front_panels, size_t& p_nb_front_panels );

protected:
	Ctrl_SceneView* ctrlSceneView;

protected:
	void Evt_ZoomM( wxCommandEvent& evt );
	void Evt_ZoomP( wxCommandEvent& evt );
	void Evt_MoveUp(   wxCommandEvent& evt );
	void Evt_MoveDown( wxCommandEvent& evt );
	void Evt_MoveLeft(   wxCommandEvent& evt );
	void Evt_MoveRight( wxCommandEvent& evt );
	
//	DECLARE_EVENT_TABLE()
};


//*************************************************************************
//*************************************************************************

class Ctrl_SceneView : public wxControl
{
public:
	Ctrl_SceneView( Panel_SceneView* _parent );
	~Ctrl_SceneView();

	void Zoom_More();
	void Zoom_Less();
	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	
	void Update_View();
	
public:
	ob_stage* stage;
	ob_StageDeclaration* stage_declaration;
	ob_StagesSet* stage_set;

	// Regular Panels
	size_t* p_nb_panels;
	ob_stage_panel*** p_panels;
	
	// Background layers
	size_t* p_nb_layers;
	ob_BG_Layer*** p_bg_layers;
	
	// Front panels
	size_t* p_nb_front_panels;
	ob_front_panel*** p_front_panels;
	
protected:
	float zoom_factor;
	int   x_decal, y_decal;
	int	level_start_x, last_panel_w;
	std::map<std::string,wxImage*> h_imgs;
	std::map<std::string,unsigned long int> h_masques;
	
protected:
	void 		RezoomUpdate(float old_zoom_factor);
	bool		Rescale_All_Images();
	bool 		ResizeImg_with_ZoomFactor( wxImage*& theImg );
	bool 		Check_ImgExists( const wxString& imgPath );
	wxImage* 	Get_Img( const wxString& imgPath );

	bool Update_Panels_Images();
	bool Update_Bgs_Images();
	bool Update_FrontPanels_Images();

	void pDrawImg( wxDC& theDC, const std::string& imgPath, wxImage* theImg, int x, int y, bool b_trans );
	
protected:
	void Evt_Paint( wxPaintEvent& evt );

	DECLARE_EVENT_TABLE()
};




#endif //LEVELS__CTRL_SCENEVIEW