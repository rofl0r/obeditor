/*
 * le_globals.h
 *
 *  Created on: 27 févr. 2009
 *      Author: pat
 */

#ifndef LE_GLOBALS_H_
#define LE_GLOBALS_H_

class wxWindow;
bool FileLevels_Reload( wxWindow* _parent );

class ob_StagesSet;
extern ob_StagesSet** levels_sets;
extern size_t nb_set;
extern ob_StagesSet* curr_set;

class ob_StageDeclaration;
extern ob_StageDeclaration** stage_list;
extern size_t nb_stage;
extern ob_StageDeclaration* curr_stage_declaration;
extern obFileStage* curr_stageFile;
extern int ob_screen_w;
extern int ob_screen_h;

//-----------------------------------------------------------------
// The possibles selection states of an object
enum
{
	SEL_OFF = 0,
	SEL_ON  = 1,
	SEL_HIGHLIGHTED  = 2,
	SEL_MORE  = 3,
	SEL_MAX   = 3,
};

//-----------------------------------------------------------------
// The entries for property pages
enum
{
	PG_TEXT = 0,
	PG_ENT_COORDS = 1,
	PG_ENT_INFOS  = 2,
	PG_ENT_ITEMHOLD = 3,
	PG_MORE = 4,
	PG_WAIT = 5,
	PG_BLOCKADE = 6,
	PG_SCROLLZ = 7,
	PG_GROUP = 8,
	PG_LIGHT = 9,
	PG_CANJOIN = 10,
	PG_NOJOIN = 11,
	PG_PL_SPAWNS = 12,
	PG_WALLS = 13,
};

// Map Zoom Factor
extern float MAP_ZF;
extern size_t SCENE_MAP_HEIGHT;

#endif /* LE_GLOBALS_H_ */
