#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "common__tools.h"

/******************************************************11
********	OB files struct datas
******************************************************/
// arr_glob_constraint MEANING : for an entire line of an ob_file
//		[0] =>  nb param min
//		[1] =>  nb param max
//		[2] =>  arr_constraint on param 1
//		[3] =>  arr_constraint on param 2
//		etc...
//		[last] => NULL
//
// arr_constraint MEANING
//		[0] => Type :
//						1 => Bool
//						2 => Int
//						3 => Positive Int
//						4 => Float
//						5 => path
//						6 => must be a string
//						8 => must be in the set, where set members that finish by # match a trailing number
//		[1+] Element of the set, if [0]==8
//		[last] => NULL

 // Some constraints needs to be reversed cause they are tailed typed, so put them in this array
 wxArrayString arr_Constraints_To_Reverse;

ob_validator::~ob_validator()
{}

ob_validator::ob_validator()
{
	arr_glob_constraint = NULL;
}

ob_validator::ob_validator( _V* _arr_glob_constraint )
{
	arr_glob_constraint = _arr_glob_constraint;
}

void ob_validator::Validate( const ob_object* it, wxString* _tokens, const size_t tokens_size )
{
	if( arr_glob_constraint == NULL)
		return;

	// Have To reverse some rules sometimes
	wxString* tokens = _tokens;
	if( arr_Constraints_To_Reverse.Index( it->name ) != wxNOT_FOUND )
		tokens = Reverse_Null_StrArray( tokens, tokens_size );


	// If number of tokens < number min
	int bound_size = *(arr_glob_constraint[0]);
	if( (int) tokens_size <  bound_size )
		ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
					"To less parameters for " + it->name   );

	// If number of tokens > number max
	bound_size = *(arr_glob_constraint[1]);
	if( (int) tokens_size > bound_size )
		ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
				"To much parameters for " + it->name );

	// Check each parameter of the tag
	int i=2;
	while(  (int*) arr_glob_constraint[i] != NULL )
	{
		// Constraint on this parameter
		_V arr_constraint = arr_glob_constraint[i];

		// No more tokens => Everything fine :)
		if( (int) tokens_size < i -1 )
			return;

		// Constraints for this parameter are empty !!
		if( (_V) arr_constraint[0] == NULL )
		{
			ObjectsLog( MYLOG_ERROR, it->num_line + it->nb_line,
				"Constraint " + IntToStr( i - 1 ) + " for <" + it->name + "> is empty !!" );
			i++;
			continue;
		}

		// Check type constraint
		int type_cstrts = (int) arr_constraint[0];

		// Path constraint
		if( type_cstrts == 5 )
		{
			if( tokens[i-2] == "" )
				ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							"For <" + it->name + "> Parameter " + IntToStr(i-1) + " must be path but it's empty" );
			else
			{
				wxFileName temp = GetObFile( tokens[i-2] );
				if( ! temp.FileExists() )
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							"For <" + it->name + "> , Parameter " + IntToStr(i-1) + " must be valid path, but can't find "
							+ temp.GetFullPath() + " in filesystem " );
			}
		}

		if( type_cstrts == 1 && ! StrIsObBool( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							"For <" + it->name +
								"> , Parameter " + IntToStr(i-1) + " must be a boolean (0,1)");

		else if( type_cstrts == 2 && ! StrIsInt( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							"For <" + it->name +
								"> , Parameter " + IntToStr(i-1) + " must be an integer");

		else if( type_cstrts == 3 && ! StrIsUInt( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							"For <" + it->name +
								"> , Parameter " + IntToStr(i-1) + " must be a positive integer" );

		else if( type_cstrts == 4 && ! StrIsFloat( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							"For <" + it->name
								+ "> , Parameter " + IntToStr(i-1) + " must be a real number" );

		// Check Set + number constraint
		else if( type_cstrts == 8 )
		{
			int j = 0;
			bool found = false;
			while( (_V) arr_constraint[j+1] != NULL )
			{
				wxString t = *((wxString*) arr_constraint[j+1]);
				t = t.Upper();
				if( t.Right(1) == "#" )
				{
					t = t.Left( t.Len() -1);
					if( tokens[i-2].Len() < t.Len() )
					{
						j++;
						continue;
					}

					if(
						tokens[i-2].Left(t.Len()).Upper() == t	// prefix of token Match
						and
						(
							 StrIsInt( tokens[i-2].Right( tokens[i-2].Len() - t.Len()) ) // suffix is a number
							||
							(tokens[i-2].Len() - t.Len() ) == 0 // no suffix
						)
					   )
					{
						found = true;
						break;
					}
				}
				else if( tokens[i-2].Upper() == t )
					{
						found = true;
						break;
					}
				j++;
			}
			if( ! found )
			{
				wxString setStr;
				j = 0;
				while( (_V) arr_constraint[j+1] != NULL )
				{
					setStr += *(wxString*)arr_constraint[j+1] + " | ";
					j++;
				}
				ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
					"For <" + it->name +
						"> Parameter " + IntToStr(i-1) + " is <" +  tokens[i-2] + "> AND must be in " + setStr  );
			}
		}
		i++;
	}
}

/**
 * Creation of known constraints
 */


_V* _VSet( size_t setSize, ...)
{
	va_list vals;
	va_start(vals, setSize);

	_V* res = new _V[setSize+2];
	res[0] = (int*) 8;
	size_t i = 0;
	while ( i < setSize )
	{
		wxString t = va_arg( vals, char* );
		res[i+1] = (int*) new wxString(t);
		i++;
	}
	res[i+1] = NULL;
	va_end(vals);
	return res;
}

_V* _VSet2( size_t setSize, wxString* tab)
{
	_V* res = new _V[setSize+2];
	res[0] = (int*) 8;
	size_t i = 0;
	while ( i < setSize )
	{
		res[i+1] = (int*) new wxString(tab[i]);
		i++;
	}
	res[i+1] = NULL;
	return res;
}

ob_validator* _Vcsrt( int min_args, int max_args, int nb_csrt, ... )
{
	va_list vals;
	va_start(vals, nb_csrt);

	_V* res = new _V[3+nb_csrt];
	res[0] = new int(min_args);
	res[1] = new int(max_args);
	int i = 0;
	while ( i < nb_csrt )
	{
		res[i+2] = (_V) va_arg( vals, _V* );
		i++;
	}
	res[i+2] = NULL;
	va_end(vals);
	return new ob_validator( res );
}




/** \internal
 * The DATAs for the constraints
*/
hash_constraints __ViD;
hash_constraints entity_constraints;
hash_constraints models_constraints;
hash_constraints levels_constraints;

wxString _t__off_def[] = {"all",	"shock", 	"burn", 	"steal", "blast", "freeze",
	 	"normal", "normal1", 	"normal2", 	"normal3", "normal4", "normal5", "normal6", "normal7"
	 	, "normal8", "normal9", "normal10"};

wxString _t__entity_type[] = {"player", "enemy", "npc", "item", "none", "steamer",
	 				"obstacle","text","trap", "endlevel", "pshot", "panel" };

wxString _t__entity_subtype[] = {"arrow", "noskip", "weapon", "biker", "notgrab",
	 				"touch", "flydie", "both", "project", "chase", "follow" };

wxString _t__entity_aimove[] = {"Chase", "Chasex", "Chasez", "Avoid", "Avoidx",
	 				"Avoidz", "Wander", "Ignoreholes"};

wxString _t__entity_animation[] = {"IDLE", "ATTACK#", "FALL#", "WALK", "BACKWALK", "TURN", "UP", "DOWN", "DUCK",
			 "LAND","RUNATTACK", "RUNJUMPATTACK", "BLOCK", "DODGE", "GET", "CHARGEATTACK", "ATTACKBOTH", "UPPER",
 			"JUMP", "JUMPDELAY", "JUMPLAND", "FORWARDJUMP", "RUNJUMP", "JUMPATTACK", "JUMPFORWARD",
 			"JUMPATTACK2", "JUMPATTACK3", "JUMPSPECIAL", "JUMPSPECIAL1", "JUMPSPECIAL2", "JUMPSPECIAL3",
 			"JUMPCANT", "ATTACKUP", "ATTACKDOWN", "ATTACKFORWARD", "ATTACKBACKWARD", "FOLLOW#",
 			"FREESPECIAL#", "SPECIAL", "SPECIAL2", "CHARGE", "CANT", "GRAB", "GRABATTACK", "GRABATTACK2",
 			"GRABFORWARD", "GRABFORWARD2", "GRABUP", "GRABUP2", "GRABDOWN", "GRABDOWN2", "GRABWALK",
 			"GRABBACKWALK", "GRABWALKUP", "GRABWALKDOWN", "GRABTURN", "THROW", "GRABBED", "GRABBEDWALK",
 			"GRABBEDBACKWALK", "GRABBEDWALKUP", "GRABBEDWALKDOWN", "GRABBEDTURN", "PAIN#", "SPAIN",
 			"BPAIN",  "RISE", "RISEATTACK", "SHOCK", "BURN", "DEATH#", "BDIE", "SDIE",
 			"HITWALL", "SLIDE", "RUNSLIDE", "RUN", "SPAWN"
			,"WAITING", "SELECT"
			};

wxString _t__smartBomb[] = {"0", "knockdown1", "1", "knockdown2", "2", "knockdown3", "3", "knockdown4", "4", "blast", "5", "burn", "6", "freeze", "7", "shock", "8", "steal" };

wxString _t__targetType[] = { "enemy", "player", "npc", "obstacle", "shot", "ground" };

wxString _t__directions[] = { "U", "D", "F", "B" };
/**
 * Implement the constraints to each tokens in the *.txt
 * files of OB
*/

 _V __bool_;
 _V __int_;
 _V __Uint_;
 _V __float_;
 _V __path_;
 _V __str_;

void constraints_init()
{
	__bool_ = new int(1);
	__int_  = new int(2);
	__Uint_ = new int(3);
	__float_= new int(4);
	__path_ = new int(5);
	__str_  = new int(6);

	/****************************
	// General validators
	****************************/
	__ViD["EMPTY"]		= _Vcsrt(0,0, 0 );
	__ViD["noCstt"]		= _Vcsrt(0,50, 0 );

	__ViD["boolR"]		= _Vcsrt(1,1, 1, __bool_ );
	__ViD["bool"] 		= _Vcsrt(0,1, 1, __bool_ );
	__ViD["intR"] 		= _Vcsrt(1,1, 1, __int_ );
	__ViD["int"]		= _Vcsrt(0,1, 1, __int_ );
	__ViD["UintR"]		= _Vcsrt(1,1, 1, __Uint_ );
	__ViD["Uint"]		= _Vcsrt(0,1, 1, __Uint_ );
	__ViD["float"]		= _Vcsrt(0,1, 1, __float_ );
	__ViD["floatR"]		= _Vcsrt(1,1, 1, __float_ );
	__ViD["path"]		= _Vcsrt(0,1, 1, __path_ );
	__ViD["pathR"]		= _Vcsrt(1,1, 1, __path_ );
	__ViD["strR"]		= _Vcsrt(1,1, 1, __str_ );

	__ViD["2_int"]		= _Vcsrt(1,2, 2, __int_, __int_ );
	__ViD["2_Uint"]		= _Vcsrt(1,2, 2, __Uint_, __Uint_ );
	__ViD["2_path"]		= _Vcsrt(2,2, 2, __path_, __path_ );
	__ViD["2_bool"]		= _Vcsrt(1,2, 2, __bool_, __bool_ );
	__ViD["str_pathR"]	= _Vcsrt(2,2, 2, __str_, __path_ );
	__ViD["Uint_bool"]	= _Vcsrt(1,2, 2, __Uint_, __bool_ );

	__ViD["3_Uint"]		= _Vcsrt(1,3, 3, __Uint_, __Uint_, __Uint_ );
	__ViD["3_int"]		= _Vcsrt(1,3, 3, __int_, __int_, __int_ );
	__ViD["path_x_yR"]	= _Vcsrt(3,3, 3, __path_, __int_, __int_ );

	__ViD["4_bool"]		= _Vcsrt(1,4, 4, __bool_, __bool_, __bool_,__bool_ );
	__ViD["4_int"]		= _Vcsrt(1,4, 4, __int_, __int_, __int_, __int_ );
	__ViD["4_Uint"]		= _Vcsrt(1,4, 4, __Uint_, __Uint_, __Uint_, __Uint_ );

	__ViD["5_int"]		= _Vcsrt(1,5, 5, __int_, __int_, __int_, __int_, __int_ );
	__ViD["5_Uint"]		= _Vcsrt(1,5, 5, __Uint_, __Uint_, __Uint_, __Uint_, __Uint_ );

	__ViD["pathR_andMore"]	= _Vcsrt(1,10, 1, __path_ );

	/****************************
	// Specific validators
	****************************/

	// entity offense defense
	_V* set__off_def = _VSet2( t_size_of( _t__off_def ), _t__off_def );
	__ViD["off_def"] = _Vcsrt(2,2, 2, set__off_def, __int_ );

	// entity type
	_V* set__ent_type = _VSet2( t_size_of( _t__entity_type ), _t__entity_type );
	__ViD["entity_type"] = _Vcsrt(1,1, 1, set__ent_type  );

	// entity subtype
	_V* set__ent_subtype = _VSet2( t_size_of( _t__entity_subtype ), _t__entity_subtype );
	__ViD["entity_subtype"] = _Vcsrt(1,1, 1, set__ent_subtype );

	// entity aimove
	_V* set__ent_aimove = _VSet2( t_size_of( _t__entity_aimove ), _t__entity_aimove );
	__ViD["entity_aimove"] = _Vcsrt(1,1, 1, set__ent_aimove );

	// entity running
	 __ViD["running"] = _Vcsrt(1,5, 5, __Uint_, __Uint_, __Uint_, __bool_, __bool_ );

 	// entity animation
	_V* set__ent_animation = _VSet2( t_size_of( _t__entity_animation ), _t__entity_animation );
	__ViD["entity_animation"] = _Vcsrt(1,1, 1, set__ent_animation );

 	// entity attack
	 __ViD["entity_attack_ON"] = _Vcsrt(6,10, 10,
		__int_, __int_, __int_, __int_, __Uint_, __Uint_,
		__bool_, __bool_, __Uint_, __int_ );

	 __ViD["entity_attack"] = _Vcsrt(1,10, 10,
		__int_, __int_, __int_, __int_, __Uint_, __Uint_,
		__bool_, __bool_, __Uint_, __int_ );

	 __ViD["entity_attack_blast"] = _Vcsrt(1,9, 9,
		__int_, __int_, __int_, __int_, __Uint_,
		__bool_, __bool_, __Uint_, __int_ );


 	// entity spawn
	 __ViD["entity_spawn"] = _Vcsrt(1,5, 5, __Uint_, __int_, __int_, __int_, __bool_ );

	// smartBomb
	_V* set__smartBomb = _VSet2( t_size_of( _t__smartBomb ), _t__smartBomb );
	__ViD["entity_smartBomb"] = _Vcsrt(2,4, 4, __int_, set__smartBomb, __bool_, __Uint_ );

	// entity_filter_type
	_V* set__hostile = _VSet2( t_size_of( _t__targetType ), _t__targetType );
	__ViD["entity_filter_type"] = _Vcsrt(1,5, 5, set__hostile, set__hostile, set__hostile, set__hostile, set__hostile );

	// Atchain
	 __ViD["at_chain"] = _Vcsrt(1,15, 15,
	 		__Uint_, __Uint_, __Uint_, __Uint_, __Uint_,
	 		__Uint_, __Uint_, __Uint_, __Uint_, __Uint_,
	 		__Uint_, __Uint_, __Uint_, __Uint_, __Uint_);

	// com
	_V* set__ent_directions = _VSet2( t_size_of( _t__directions ), _t__directions );
	 __ViD["entity_com"] = _Vcsrt(3,17, 17,
		_VSet( 1, "freespecial#" ),
		_VSet( 7, "A", "A2", "A3", "A4", "J", "S", "K" )
		 , set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions );

 __ViD["ent_lifebarstatus"] = _Vcsrt( 2, 10, 10,
	__Uint_, __Uint_, __bool_, __bool_, __bool_, __int_, __int_, __int_, __int_, __int_ );

 __ViD["platform"] = _Vcsrt( 8, 8, 8,
 __int_, __int_, __int_, __int_, __int_, __int_, __int_, __int_ );

 __ViD["drawmethod"] = _Vcsrt( 1, 10, 10,
	__int_, __int_, __bool_, __bool_, __int_, __int_ ,
	__int_, __Uint_, __Uint_, __bool_);


// Models.txt constraints
	models_constraints["name"] = __ViD["strR"];
	models_constraints["ajspecial"] = __ViD["boolR"];
	models_constraints["autoland"] = __ViD["UintR"];
	models_constraints["colourselect"] = __ViD["boolR"];
	models_constraints["nocost"] = __ViD["boolR"];
	models_constraints["nolost"] = __ViD["boolR"];
	models_constraints["blockratio"] = __ViD["boolR"];
	models_constraints["mpblock"] = __ViD["boolR"];
	models_constraints["nochipdeath"] = __ViD["boolR"];
	models_constraints["nochipdeath"] = __ViD["boolR"];
	models_constraints["versusdamage"] = __ViD["boolR"];
	models_constraints["spdirection"] = __ViD["4_bool"];
	models_constraints["maxattacks"] = __ViD["UintR"];
	models_constraints["maxattacktypes"] = __ViD["UintR"];
	models_constraints["maxfollows"] = __ViD["UintR"];
	models_constraints["maxfreespecials"] = __ViD["UintR"];
	models_constraints["lifescore"] = __ViD["intR"];
	models_constraints["credscore"] = __ViD["intR"];
	models_constraints["nocheats"] = __ViD["boolR"];
	models_constraints["load"] = __ViD["str_pathR"];
	models_constraints["know"] = __ViD["str_pathR"];

	// entity files constraints
	entity_constraints["name"] = __ViD["strR"];
	entity_constraints["type"] = __ViD["entity_type"];
	entity_constraints["subtype"] = __ViD["entity_subtype"];
	entity_constraints["health"] = __ViD["UintR"];
	entity_constraints["mp"] = __ViD["UintR"];
	entity_constraints["credit"] = __ViD["UintR"];
	entity_constraints["alpha"] = __ViD["UintR"];
	entity_constraints["speed"] = __ViD["UintR"];
	entity_constraints["speedf"] = __ViD["floatR"];
	entity_constraints["running"] = __ViD["running"];
	entity_constraints["nomove"] = __ViD["2_bool"];
	entity_constraints["jumpspeed"] = __ViD["UintR"];
	entity_constraints["jumpspeedf"] = __ViD["floatR"];
	entity_constraints["jumpheight"] = __ViD["UintR"];
	entity_constraints["grabdistance"] = __ViD["UintR"];
	entity_constraints["throwdamage"] = __ViD["UintR"];
	entity_constraints["throw"] = __ViD["2_Uint"];
	entity_constraints["throwframewait"] = __ViD["UintR"];
	entity_constraints["height"] = __ViD["UintR"];
	entity_constraints["secret"] = __ViD["boolR"];
	entity_constraints["shadow"] = __ViD["UintR"];
	entity_constraints["fmap"] = __ViD["UintR"];
	entity_constraints["load"] = __ViD["strR"];
	entity_constraints["project"] = __ViD["strR"];
	entity_constraints["shootnum"] = __ViD["UintR"];
	entity_constraints["counter"] = __ViD["UintR"];
	entity_constraints["reload"] = __ViD["UintR"];
	entity_constraints["typeshot"] = __ViD["boolR"];
	entity_constraints["animal"] = __ViD["boolR"];
	entity_constraints["playshot"] = __ViD["strR"];
	entity_constraints["playshotno"] = __ViD["strR"];
	entity_constraints["knife"] = __ViD["strR"];
	entity_constraints["star"] = __ViD["strR"];
	entity_constraints["bomb"] = __ViD["strR"];
	entity_constraints["pbomb"] = __ViD["strR"];
	entity_constraints["hitenemy"] = __ViD["2_bool"];
	entity_constraints["rider"] = __ViD["strR"];
	entity_constraints["flash"] = __ViD["strR"];
	entity_constraints["bflash"] = __ViD["strR"];
	entity_constraints["dust"] = __ViD["strR"];
	entity_constraints["nolife"] = __ViD["boolR"];
	entity_constraints["noquake"] = __ViD["boolR"];
	entity_constraints["nopain"] = __ViD["boolR"];
	entity_constraints["nodrop"] = __ViD["boolR"];
	entity_constraints["nodieblink"] = __ViD["Uint_bool"];
	entity_constraints["makeinv"] = __ViD["Uint_bool"];
	entity_constraints["blockodds"] = __ViD["UintR"];
	entity_constraints["thold"] = __ViD["UintR"];
	entity_constraints["blockpain"] = __ViD["boolR"];
	entity_constraints["falldie"] = __ViD["UintR"];
	entity_constraints["aironly"] = __ViD["boolR"];
	entity_constraints["setlayer"] = __ViD["intR"];
	entity_constraints["grabback"] = __ViD["boolR"];
	entity_constraints["grabfinish"] = __ViD["boolR"];
	entity_constraints["icon"] = __ViD["pathR"];
	entity_constraints["iconpain"] = __ViD["pathR"];
	entity_constraints["icondie"] = __ViD["pathR"];
	entity_constraints["iconget"] = __ViD["pathR"];
	entity_constraints["iconw"] = __ViD["pathR"];
	entity_constraints["iconmphigh"] = __ViD["pathR"];
	entity_constraints["iconmphalf"] = __ViD["pathR"];
	entity_constraints["iconmplow"] = __ViD["pathR"];
	entity_constraints["diesound"] = __ViD["pathR"];
	entity_constraints["parrow"] = __ViD["path_x_yR"];
	entity_constraints["parrow2"] = __ViD["path_x_yR"];
	entity_constraints["score"] = __ViD["2_int"];
	entity_constraints["smartbomb"] = __ViD["entity_smartBomb"];
	entity_constraints["toflip"] = __ViD["boolR"];
	entity_constraints["cantgrab"] = __ViD["boolR"];
	entity_constraints["paingrab"] = __ViD["boolR"];
	entity_constraints["noatflash"] = __ViD["boolR"];
	entity_constraints["remove"] = __ViD["boolR"];
	entity_constraints["escapehits"] = __ViD["UintR"];
	entity_constraints["com"] = __ViD["entity_com"];
	arr_Constraints_To_Reverse.Add( "com" );
	entity_constraints["remap"] = __ViD["2_path"];
	entity_constraints["atchain"] = __ViD["at_chain"];
	entity_constraints["chargerate"] = __ViD["UintR"];
	entity_constraints["mprate]"] = __ViD["UintR"];
	entity_constraints["risetime"] = __ViD["intR"];
	entity_constraints["turndelay"] = __ViD["UintR"];
	entity_constraints["facing"] = __ViD["UintR"];
	entity_constraints["weaploss"] = __ViD["UintR"];
	entity_constraints["branch"] = __ViD["strR"];
	entity_constraints["hostile"] = __ViD["entity_filter_type"];
	entity_constraints["candamage"] = __ViD["entity_filter_type"];
	entity_constraints["projectilehit"] = __ViD["entity_filter_type"];
	entity_constraints["aggression"] = __ViD["intR"];
	entity_constraints["antigrab"] = __ViD["intR"];
	entity_constraints["grabforce"] = __ViD["intR"];
	entity_constraints["offense"] = __ViD["off_def"];
	entity_constraints["defense"] = __ViD["off_def"];
	entity_constraints["hmap"] = __ViD["2_Uint"];
	entity_constraints["bounce"] = __ViD["boolR"];
	entity_constraints["grabwalk"] = __ViD["boolR"];
	entity_constraints["grabturn"] = __ViD["boolR"];
	entity_constraints["load"] = __ViD["str_pathR"];
	entity_constraints["lifespan"] = __ViD["UintR"];
	entity_constraints["nopassiveblock"] = __ViD["boolR"];
	entity_constraints["knockdowncount"] = __ViD["UintR"];
	entity_constraints["antigravity"] = __ViD["intR"];
	entity_constraints["aimove"] = __ViD["entity_aimove"];
	entity_constraints["gfxshadow"] = __ViD["boolR"];
	entity_constraints["holdblock"] = __ViD["boolR"];
	entity_constraints["jumpmove"] = __ViD["2_Uint"];
	entity_constraints["riseinv"] = __ViD["Uint_bool"];
	entity_constraints["lifebarstatus"] = __ViD["ent_lifebarstatus"];
	entity_constraints["lifeposition"] = __ViD["2_int"];
	entity_constraints["nameposition"] = __ViD["2_int"];
	entity_constraints["iconposition"] = __ViD["2_int"];
	entity_constraints["no_adjust_base"] = __ViD["boolR"];
	entity_constraints["subject_to_wall"] = __ViD["boolR"];
	entity_constraints["subject_to_hole"] = __ViD["boolR"] ;
	entity_constraints["subject_to_obstacle"] = __ViD["boolR"];
	entity_constraints["subject_to_platform"] = __ViD["boolR"];
	entity_constraints["subject_to_gravity"] = __ViD["boolR"];
	entity_constraints["subject_to_screen"] = __ViD["boolR"];
	entity_constraints["falldie"] = __ViD["UintR"];
	entity_constraints["death"] = __ViD["UintR"];
	entity_constraints["resetable"] = __ViD["int"];
	entity_constraints["sleepwait"] = __ViD["int"];
	entity_constraints["always_go_ahead"] = __ViD["bool"];
	entity_constraints["always_walk"] = __ViD["bool"];
	entity_constraints["nodropfly"] = __ViD["bool"];
	entity_constraints["palette"] = __ViD["pathR_andMore"];
	entity_constraints["alternatepal"] = __ViD["pathR"];

	//Animations properties
	entity_constraints["animation"] = __ViD["entity_animation"];
	entity_constraints["anim"] = __ViD["entity_animation"];

	entity_constraints["loop"] = __ViD["boolR"];
	entity_constraints["fastattack"] = __ViD["boolR"];
	entity_constraints["hitfx"] = __ViD["pathR"];
	entity_constraints["hitflash"] = __ViD["strR"];
	entity_constraints["custknife"] = __ViD["strR"];
	entity_constraints["custstar"] = __ViD["strR"];
	entity_constraints["custbomb"] = __ViD["strR"];
	entity_constraints["custpbomb"] = __ViD["strR"];
	entity_constraints["delay"] = __ViD["UintR"];
	entity_constraints["offset"] = __ViD["2_int"];
	entity_constraints["bbox"] = __ViD["5_int"];
	entity_constraints["frame"] = __ViD["pathR"];
	entity_constraints["range"] = __ViD["2_int"];
	entity_constraints["rangez"] = __ViD["2_int"];
	entity_constraints["rangea"] = __ViD["2_int"];
	entity_constraints["attack"] = __ViD["entity_attack"];
	entity_constraints["attack1"] = __ViD["entity_attack"];
	entity_constraints["attack2"] = __ViD["entity_attack"];
	entity_constraints["attack3"] = __ViD["entity_attack"];
	entity_constraints["attack4"] = __ViD["entity_attack"];
	entity_constraints["attack5"] = __ViD["entity_attack"];
	entity_constraints["attack6"] = __ViD["entity_attack"];
	entity_constraints["attack7"] = __ViD["entity_attack"];
	entity_constraints["attack8"] = __ViD["entity_attack"];
	entity_constraints["attack9"] = __ViD["entity_attack"];
	entity_constraints["blast"] = __ViD["entity_attack_blast"];
	entity_constraints["shock"] = __ViD["entity_attack"];
	entity_constraints["burn"] = __ViD["entity_attack"];
	entity_constraints["freeze"] = __ViD["entity_attack"];
	entity_constraints["steal"] = __ViD["entity_attack"];
	entity_constraints["quakeframe"] = __ViD["3_Uint"];
	entity_constraints["move"] = __ViD["intR"];
	entity_constraints["movea"] = __ViD["intR"];
	entity_constraints["movez"] = __ViD["intR"];
	entity_constraints["seta"] = __ViD["intR"];
	entity_constraints["platform"] = __ViD["platform"];
	entity_constraints["dive"] = __ViD["2_int"];
	entity_constraints["sound"] = __ViD["path"];
	entity_constraints["pshotframe"] = __ViD["2_Uint"];
	entity_constraints["throwframe"] = __ViD["2_Uint"];
	entity_constraints["tossframe"] = __ViD["2_Uint"];
	entity_constraints["pbombframe"] = __ViD["2_Uint"];
	entity_constraints["jumpframe"] = __ViD["4_int"];
	entity_constraints["custpshot"] = __ViD["strR"];
	entity_constraints["mpcost"] = __ViD["UintR"];
	entity_constraints["custfireb"] = __ViD["strR"];
	entity_constraints["shootframe"] = __ViD["2_Uint"];
	entity_constraints["flipframe"] = __ViD["UintR"];
	entity_constraints["followanim"] = __ViD["UintR"];
	entity_constraints["followcond"] = __ViD["UintR"];
	entity_constraints["counterframe"] = __ViD["3_Uint"];
	entity_constraints["spawnframe"] = __ViD["entity_spawn"];
	entity_constraints["summonframe"] = __ViD["entity_spawn"];
	entity_constraints["unsummonframe"] = __ViD["UintR"];
	entity_constraints["subentity"] = __ViD["UintR"];
	entity_constraints["custentity"] = __ViD["UintR"];
	entity_constraints["weaponframe"] = __ViD["2_Uint"];
	entity_constraints["attackone"] = __ViD["boolR"];
	entity_constraints["grabin"] = __ViD["2_Uint"];
	entity_constraints["forcedirection"] = __ViD["intR"];
	entity_constraints["damageonlanding"] = __ViD["2_Uint"];
	entity_constraints["dropv"] = __ViD["3_int"];
	entity_constraints["dropframe"] = __ViD["UintR"];
	entity_constraints["landframe"] = __ViD["UintR"];
	entity_constraints["counterattack"] = __ViD["boolR"];
	entity_constraints["fshadow"] = __ViD["intR"];
	entity_constraints["shadowcoords"] = __ViD["2_int"];
	entity_constraints["itembox"] = __ViD["4_int"];
	entity_constraints["stun"] = __ViD["intR"];
	entity_constraints["seal"] = __ViD["2_Uint"];
	entity_constraints["forcemap"] = __ViD["2_Uint"];
	entity_constraints["drain"] = __ViD["4_Uint"];
	entity_constraints["noreflect"] = __ViD["boolR"];
	entity_constraints["drawmethod"] = __ViD["drawmethod"];
	entity_constraints["nodrawmethod"] = __ViD["EMPTY"];
	entity_constraints["bouncefactor"] = __ViD["intR"];

	entity_constraints["mponly"] = __ViD["boolR"];
	entity_constraints["hponly"] = __ViD["boolR"];
	entity_constraints["energycost"] = __ViD["intR"];
	entity_constraints["throwframewait"] = __ViD["UintR"];

	entity_constraints["animationscript"] = __ViD["pathR"];
	entity_constraints["takedamagescript"] = __ViD["pathR"];
	entity_constraints["script"] = __ViD["pathR"];
	entity_constraints["ondeathscript"] = __ViD["pathR"];
	entity_constraints["didhitscript"] = __ViD["pathR"];
	entity_constraints["didblockscript"] = __ViD["pathR"];
	entity_constraints["@cmd"] = __ViD["noCstt"];
	entity_constraints["@script"] = __ViD["noCstt"];
	entity_constraints["@end_script"] = __ViD["noCstt"];
	entity_constraints["fastattack"]  = __ViD["boolR"];
}
