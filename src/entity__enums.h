/*
 * edit_entity_enums.h
 *
 *  Created on: 7 nov. 2008
 *      Author: pat
 */

#ifndef EDIT_ENTITY_ENUMS_H_
#define EDIT_ENTITY_ENUMS_H_

#include "common__tools.h"

//****************************************************
// Global variables for the edit entity frame objects
//****************************************************

// Max numbers of last entities history in the corresponding menu
#define MAX_ENTITIES_HISTORY 20

// Models Know/Load from the models.txt
extern int models_count;
extern ob_models** models;
extern wxString* models_name;	// The list of the names of the models

// The entity currently loaded and selected to edition
extern obFileEntity* entity;

// The filename of the curently loaded/loading entity
extern wxString curr_entity_filename;

// List of properties for the current entity
extern ob_object** entProps;
extern int entProps_count;


// Currently loaded anims
extern ob_anim** arr_anims;
extern int arr_anims_count;

// Currently selected anim (Note: -1 => no anim selected )
extern ob_anim* curr_anim;
extern wxString last_anim_selected;

// array and array size of Frames loaded
extern int curr_frames_count;
extern ob_frame** curr_frames;
extern int ind_active_frame;

// current active frame
extern ob_frame* frameActive;

// the original bg colour
extern wxColour default_BG_colour;

enum
{
	GM_ANIM_DELETION 		= 1,
	FRAME_DELETION		= 2,
	GF_RELOAD			= 4,
	GF_ACTIVATE			= 8,
	GM_REFRESH_SELECTION	=16,
	GM_FRAME_RELOAD		=32,
	GM_FRAME_INVALID		=64,
};
extern int ent_g_flag;

// Hash that maintain the relation :  EntityName <-> Object Loaded
WX_DECLARE_HASH_MAP( wxString, obFileEntity*, wxStringHash, wxStringEqual, hash_entName_entObjFile );
extern hash_entName_entObjFile hash_ent_obj;

/* UNUSED ??
extern ob_object** entNewProps;
extern size_t entNewProps_count;
#define MAX_NEW_PROPS 100
*/

//****************************************************
// New Events macros
//****************************************************

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_ANIM_LIST_CHANGE, 7777)
    DECLARE_EVENT_TYPE(wxEVT_ANIM_SELECTED_CHANGE, 7777)
    DECLARE_EVENT_TYPE(wxEVT_FRAME_SELECTED_CHANGE, 7777)
    DECLARE_EVENT_TYPE(wxEVT_FRAME_LIST_CHANGE, 7777)
END_DECLARE_EVENT_TYPES()

// define an event table macro for this event type
#ifndef EVT_ANIM_SELECTED_CHANGE
#define EVT_ANIM_SELECTED_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    	wxEVT_ANIM_SELECTED_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif

// define an event table macro for this event type
#ifndef EVT_ANIM_LIST_CHANGE
#define EVT_ANIM_LIST_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    	wxEVT_ANIM_LIST_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif

// define an event table macro for this event type
#ifndef EVT_FRAME_SELECTED_CHANGE
#define EVT_FRAME_SELECTED_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    	wxEVT_FRAME_SELECTED_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif

// define an event table macro for this event type
#ifndef EVT_FRAME_LIST_CHANGE
#define EVT_FRAME_LIST_CHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
    	wxEVT_FRAME_LIST_CHANGE, wxID_ANY, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),
#endif


//****************************************************
// enumeration of IDs for the edit entity frame
//****************************************************

enum
{
	ID_NEW_ANIM = 100,
	ID_NEW_FRAME = 101,
	ID_ANIM_PROPS = 102,
	ID_EXT_FRAM_PROPS = 103,
	BBOX_X = 104,
	BBOX_Y = 105,
	BBOX_H = 106,
	BBOX_W = 107,
	ATTACK_CHOICE = 108,
	DMGS_VAL = 109,
	PWD_VAL = 110,
	B_BLOK = 111,
	B_FLASH = 112,
	CTRL_PAUSE_VAL = 113,
	ZRANGE = 114,
	ATTBOX_X = 115,
	ATTBOX_Y = 116,
	ATTBOX_H = 117,
	ATTBOX_W = 118,
	MOVE_X = 119,
	MOVE_A = 120,
	MOVE_Z = 121,
	SND_FILE = 122,
	IMG_FILE = 123,
	OFFSET_X = 124,
	OFFSET_Y = 125,
	ID_DELAY = 126,
	ID_PLAY_STOP = 127,
	ID_ZOOM_M = 128,
	ID_ZOOM_P = 129,
	ID_TO_END = 130,
	ID_TO_START = 131,
	ID_LOOP_BTN = 10000,
	ID_PICKER_FRAME_PATH = 10001,
	ID_PICKER_SOUND_PATH = 10002,
	ID_To_BBOX = 10003,
	ID_To_ATTBOX = 10004,
	ID_To_OFFSET = 10005,
	ID_To_CLEAR = 10006,
	ID_DEL_BBOX = 10007,
	ID_DEL_ATTBOX = 10008,
	ID_SCALEX = 10009,
	ID_SCALEY = 10010,
	ID_FLIPX = 10011,
	ID_FLIPY = 10012,
	ID_SHIFTX = 10013,
	ID_ALPHA = 10014,
	ID_REMAP = 10015,
	ID_FILLCOLOR = 10016,
	ID_ROTATE = 10017,
	ID_FLIPROTATE = 10018,
	ID_FLIPFRAME = 10019,
	ID_NODRAWMETHOD = 10020,
	ID_To_XA = 10021,

   	CMD_RENAME = 10022,
   	CMD_NEW = 10023,
   	CMD_DEL = 10024,
   	CMD_COPY = 10025,
   	TIMER_ANIM = 10026,

   	REMAP_CHOICE = 10027,
   	IMG_BASE = 10028,
   	ID_PICKER_BASE_IMG = 10029,
   	ID_PICKER_DEST_IMG = 10030,
   	IMG_DEST = 10031,
   	ID_CREATE_BASEIMG_REMAP = 10032,
   	ID_GUESS_COLORS = 10033,
   	ID_SET_SEL_COLS = 10034,
   	ID_UNSET_SEL_COLS = 10035,
   	ID_HUE_CTRL = 10036,
   	ID_SAT_CTRL = 10037,
   	ID_LUM_CTRL = 10038,
   	ID_APPLY_COLORS = 10039,
   	ID_CANCEL_COLORS = 10040,

   	ID_NEW_REMAP = 10041,
   	ID_RENAME_REMAP = 10042,
   	ID_DELETE_REMAP = 10043,
   	B_8BIT = 10044,

   	ID_SAVE_REMAP = 10045,

   	ID_RED_CTRL = 10046,
   	ID_GREEN_CTRL = 10047,
   	ID_BLUE_CTRL = 10048,

   	ID_BBOX_COLOR = 10049,
   	ID_ATTACK_COLOR = 10050,

   	ID_PLAT_OFF_X = 10051,
   	ID_PLAT_OFF_Y = 10052,
   	ID_PLAT_UPL = 10053,
   	ID_PLAT_DL = 10054,
   	ID_PLAT_UPR = 10055,
   	ID_PLAT_LR = 10056,
   	ID_PLAT_DEPTH = 10057,
   	ID_PLAT_ALT = 10058,

   	ID_NO_PLATFORM = 10059,

   	ID_JUMPFRAME = 10060,
   	ID_DROPFRAME = 10061,
   	ID_LANDFRAME = 10062,
   	ID_JUMPH = 10063,
   	ID_JUMPX = 10064,
   	ID_JUMPZ = 10065,

   	ID_SOUND_MUTE = 10066,
   	ID_HELP_REMAPS = 10067,

   	BBOX_Z = 10068,
   	ID_ENTITIES_RESCALE_BOXES = 10069,
	ID_ATT_NUM	=	10070,
};


#endif /* EDIT_ENTITY_ENUMS_H_ */
