/*
 * globals.h
 *
 *  Created on: 7 nov. 2008
 *      Author: pat
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <wx/colour.h>

//*******************
// ENUMERATIONS
//*******************

// some usual windows Flag
enum
{
	M_CLOSING,
};

// _propTypes
enum
{
	PROPTYPE_UNDEF,
	PROPTYPE_STRING,
	PROPTYPE_STRING_NO_WS,
	PROPTYPE_OBFILE,
	PROPTYPE_NUMBER,
	PROPTYPE_BOOL,
	PROPTYPE_BOOL_EXSISTENCE,
	PROPTYPE_ENUMS,
	PROPTYPE_RANGE,
	PROPTYPE_SPACER,
	PROPTYPE_NEGATIVENUMBER,
	PROPTYPE_RELATIVENUMBER,
	PROPTYPE_FLOAT,
	PROPTYPE_FLOAT_POSITIVE,
};



//*******************
//Globals variables
//*******************

// The object for saving configurations
class wxConfigBase;
extern wxConfigBase* config;

// The "NoImage" image
class wxImage;
extern wxImage *noImg;

// Ressource Directory
#include <wx/filename.h>
extern wxFileName dirRessources;

extern wxColour ob_pink;



//****************************************************
//***********NEW EVENTS DECL***********************

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_ADD_NEW_GRIDLINE, 7777)
    DECLARE_EVENT_TYPE(wxEVT_GRIDLINE_REPOS_CURSOR, 7778)
    DECLARE_EVENT_TYPE(wxEVT_OBFILECHOOSER_CHANGE, 7779)
    DECLARE_EVENT_TYPE(wxEVT_OBPROP_CHANGE, 7780)
END_DECLARE_EVENT_TYPES()



#endif /* GLOBALS_H_ */
