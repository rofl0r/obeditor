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
			    wxT("To less parameters for ") + it->name   );

	// If number of tokens > number max
	bound_size = *(arr_glob_constraint[1]);
	if( (int) tokens_size > bound_size )
		ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
			    wxT("To much parameters for ") + it->name );

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
				    wxT("Constraint ") + IntToStr( i - 1 ) + wxT(" for <") + it->name + wxT("> is empty !!") );
			i++;
			continue;
		}

		// Check type constraint
		int type_cstrts = (int) arr_constraint[0];

		// Path constraint
		if( type_cstrts == 5 )
		{
			if( tokens[i-2] == wxString() )
				ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
					    wxT("For <") + it->name + wxT("> Parameter ") + IntToStr(i-1) + wxT(" must be path but it's empty") );
			else
			{
				wxFileName temp = GetObFile( tokens[i-2] );
				if( ! temp.FileExists() )
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							    wxT("For <") + it->name + wxT("> , Parameter ") + IntToStr(i-1) + wxT(" must be valid path, but can't find ")
							    + temp.GetFullPath() + wxT(" in filesystem ") );
			}
		}

		if( type_cstrts == 1 && ! StrIsObBool( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							    wxT("For <") + it->name +
							    wxT("> , Parameter ") + IntToStr(i-1) + wxT(" must be a boolean (0,1)"));

		else if( type_cstrts == 2 && ! StrIsInt( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							    wxT("For <") + it->name +
							    wxT("> , Parameter ") + IntToStr(i-1) + wxT(" must be an integer"));

		else if( type_cstrts == 3 && ! StrIsUInt( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							    wxT("For <") + it->name +
							    wxT("> , Parameter ") + IntToStr(i-1) + wxT(" must be a positive integer" ));

		else if( type_cstrts == 4 && ! StrIsFloat( tokens[i-2] ))
						ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
							    wxT("For <") + it->name
							    + wxT("> , Parameter ") + IntToStr(i-1) + wxT(" must be a real number") );

		// Check Set + number constraint
		else if( type_cstrts == 8 )
		{
			int j = 0;
			bool found = false;
			while( (_V) arr_constraint[j+1] != NULL )
			{
				wxString t = *((wxString*) arr_constraint[j+1]);
				t = t.Upper();
				if( t.Right(1) == wxT("#") )
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
					setStr += *(wxString*)arr_constraint[j+1] + wxT(" | ");
					j++;
				}
				ObjectsLog( MYLOG_WARNING, it->num_line + it->nb_line,
					    wxT("For <") + it->name +
					    wxT("> Parameter ") + IntToStr(i-1) + wxT(" is <") +  tokens[i-2] + wxT("> AND must be in ") + setStr  );
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
		wxString t = wxString::FromAscii(va_arg( vals, char* ));
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

wxString _t__off_def[] = {wxT("all"), wxT("shock"), wxT("burn"), wxT("steal"), wxT("blast"), wxT("freeze"), wxT("normal"), wxT("normal1"), wxT("normal2"), wxT("normal3"), wxT("normal4"), wxT("normal5"), wxT("normal6"), wxT("normal7"), wxT("normal8"), wxT("normal9"), wxT("normal10")};

wxString _t__entity_type[] = {wxT("player"), wxT("enemy"), wxT("npc"), wxT("item"), wxT("none"), wxT("steamer"), wxT("obstacle"), wxT("text"), wxT("trap"), wxT("endlevel"), wxT("pshot"), wxT("panel") };

wxString _t__entity_subtype[] = {wxT("arrow"), wxT("noskip"), wxT("weapon"), wxT("biker"), wxT("notgrab"), wxT("touch"), wxT("flydie"), wxT("both"), wxT("project"), wxT("chase"), wxT("follow") };

wxString _t__entity_aimove[] = {wxT("Chase"), wxT("Chasex"), wxT("Chasez"), wxT("Avoid"), wxT("Avoidx"), wxT("Avoidz"), wxT("Wander"), wxT("Ignoreholes")};

wxString _t__entity_animation[] = {
	wxT("IDLE"), 
	wxT("ATTACK#"), 
	wxT("FALL#"), 
	wxT("WALK"), 
	wxT("BACKWALK"), 
	wxT("TURN"), 
	wxT("UP"), 
	wxT("DOWN"), 
	wxT("DUCK"), 
	wxT("LAND"), 
	wxT("RUNATTACK"), 
	wxT("RUNJUMPATTACK"), 
	wxT("BLOCK"), 
	wxT("DODGE"), 
	wxT("GET"), 
	wxT("CHARGEATTACK"), 
	wxT("ATTACKBOTH"), 
	wxT("UPPER"), 
	wxT("JUMP"), 
	wxT("JUMPDELAY"), 
	wxT("JUMPLAND"), 
	wxT("FORWARDJUMP"), 
	wxT("RUNJUMP"), 
	wxT("JUMPATTACK"), 
	wxT("JUMPFORWARD"), 
	wxT("JUMPATTACK2"), 
	wxT("JUMPATTACK3"), 
	wxT("JUMPSPECIAL"), 
	wxT("JUMPSPECIAL1"), 
	wxT("JUMPSPECIAL2"), 
	wxT("JUMPSPECIAL3"), 
	wxT("JUMPCANT"), 
	wxT("ATTACKUP"), 
	wxT("ATTACKDOWN"), 
	wxT("ATTACKFORWARD"), 
	wxT("ATTACKBACKWARD"), 
	wxT("FOLLOW#"), 
	wxT("FREESPECIAL#"), 
	wxT("SPECIAL"), 
	wxT("SPECIAL2"), 
	wxT("CHARGE"), 
	wxT("CANT"), 
	wxT("GRAB"), 
	wxT("GRABATTACK"), 
	wxT("GRABATTACK2"), 
	wxT("GRABFORWARD"), 
	wxT("GRABFORWARD2"), 
	wxT("GRABUP"), 
	wxT("GRABUP2"), 
	wxT("GRABDOWN"), 
	wxT("GRABDOWN2"), 
	wxT("GRABWALK"), 
	wxT("GRABBACKWALK"), 
	wxT("GRABWALKUP"), 
	wxT("GRABWALKDOWN"), 
	wxT("GRABTURN"), 
	wxT("THROW"), 
	wxT("GRABBED"), 
	wxT("GRABBEDWALK"), 
	wxT("GRABBEDBACKWALK"), 
	wxT("GRABBEDWALKUP"), 
	wxT("GRABBEDWALKDOWN"), 
	wxT("GRABBEDTURN"), 
	wxT("PAIN#"), 
	wxT("SPAIN"), 
	wxT("BPAIN"),  
	wxT("RISE"), 
	wxT("RISEATTACK"), 
	wxT("SHOCK"), 
	wxT("BURN"), 
	wxT("DEATH#"), 
	wxT("BDIE"), 
	wxT("SDIE"), 
	wxT("HITWALL"), 
	wxT("SLIDE"), 
	wxT("RUNSLIDE"), 
	wxT("RUN"), 
	wxT("SPAWN"), 
	wxT("WAITING"), 
	wxT("SELECT")
};

wxString _t__smartBomb[] = {wxT("0"), wxT("knockdown1"), wxT("1"), wxT("knockdown2"), wxT("2"), wxT("knockdown3"), wxT("3"), wxT("knockdown4"), wxT("4"), wxT("blast"), wxT("5"), wxT("burn"), wxT("6"), wxT("freeze"), wxT("7"), wxT("shock"), wxT("8"), wxT("steal") };

wxString _t__targetType[] = {wxT("enemy"), wxT("player"), wxT("npc"), wxT("obstacle"), wxT("shot"), wxT("ground") };

wxString _t__directions[] = { wxT("U"), wxT("D"), wxT("F"), wxT("B") };
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
	__ViD[wxT("EMPTY")]		= _Vcsrt(0,0, 0 );
	__ViD[wxT("noCstt")]		= _Vcsrt(0,50, 0 );

	__ViD[wxT("boolR")]		= _Vcsrt(1,1, 1, __bool_ );
	__ViD[wxT("bool")] 		= _Vcsrt(0,1, 1, __bool_ );
	__ViD[wxT("intR")] 		= _Vcsrt(1,1, 1, __int_ );
	__ViD[wxT("int")]		= _Vcsrt(0,1, 1, __int_ );
	__ViD[wxT("UintR")]		= _Vcsrt(1,1, 1, __Uint_ );
	__ViD[wxT("Uint")]		= _Vcsrt(0,1, 1, __Uint_ );
	__ViD[wxT("float")]		= _Vcsrt(0,1, 1, __float_ );
	__ViD[wxT("floatR")]		= _Vcsrt(1,1, 1, __float_ );
	__ViD[wxT("path")]		= _Vcsrt(0,1, 1, __path_ );
	__ViD[wxT("pathR")]		= _Vcsrt(1,1, 1, __path_ );
	__ViD[wxT("strR")]		= _Vcsrt(1,1, 1, __str_ );

	__ViD[wxT("2_int")]		= _Vcsrt(1,2, 2, __int_, __int_ );
	__ViD[wxT("2_Uint")]		= _Vcsrt(1,2, 2, __Uint_, __Uint_ );
	__ViD[wxT("2_path")]		= _Vcsrt(2,2, 2, __path_, __path_ );
	__ViD[wxT("2_bool")]		= _Vcsrt(1,2, 2, __bool_, __bool_ );
	__ViD[wxT("str_pathR")]	= _Vcsrt(2,2, 2, __str_, __path_ );
	__ViD[wxT("Uint_bool")]	= _Vcsrt(1,2, 2, __Uint_, __bool_ );

	__ViD[wxT("3_Uint")]		= _Vcsrt(1,3, 3, __Uint_, __Uint_, __Uint_ );
	__ViD[wxT("3_int")]		= _Vcsrt(1,3, 3, __int_, __int_, __int_ );
	__ViD[wxT("path_x_yR")]	= _Vcsrt(3,3, 3, __path_, __int_, __int_ );

	__ViD[wxT("4_bool")]		= _Vcsrt(1,4, 4, __bool_, __bool_, __bool_,__bool_ );
	__ViD[wxT("4_int")]		= _Vcsrt(1,4, 4, __int_, __int_, __int_, __int_ );
	__ViD[wxT("4_Uint")]		= _Vcsrt(1,4, 4, __Uint_, __Uint_, __Uint_, __Uint_ );

	__ViD[wxT("5_int")]		= _Vcsrt(1,5, 5, __int_, __int_, __int_, __int_, __int_ );
	__ViD[wxT("5_Uint")]		= _Vcsrt(1,5, 5, __Uint_, __Uint_, __Uint_, __Uint_, __Uint_ );

	__ViD[wxT("pathR_andMore")]	= _Vcsrt(1,10, 1, __path_ );

	/****************************
	// Specific validators
	****************************/

	// entity offense defense
	_V* set__off_def = _VSet2( t_size_of( _t__off_def ), _t__off_def );
	__ViD[wxT("off_def")] = _Vcsrt(2,2, 2, set__off_def, __int_ );

	// entity type
	_V* set__ent_type = _VSet2( t_size_of( _t__entity_type ), _t__entity_type );
	__ViD[wxT("entity_type")] = _Vcsrt(1,1, 1, set__ent_type  );

	// entity subtype
	_V* set__ent_subtype = _VSet2( t_size_of( _t__entity_subtype ), _t__entity_subtype );
	__ViD[wxT("entity_subtype")] = _Vcsrt(1,1, 1, set__ent_subtype );

	// entity aimove
	_V* set__ent_aimove = _VSet2( t_size_of( _t__entity_aimove ), _t__entity_aimove );
	__ViD[wxT("entity_aimove")] = _Vcsrt(1,1, 1, set__ent_aimove );

	// entity running
	 __ViD[wxT("running")] = _Vcsrt(1,5, 5, __Uint_, __Uint_, __Uint_, __bool_, __bool_ );

 	// entity animation
	_V* set__ent_animation = _VSet2( t_size_of( _t__entity_animation ), _t__entity_animation );
	__ViD[wxT("entity_animation")] = _Vcsrt(1,1, 1, set__ent_animation );

 	// entity attack
	 __ViD[wxT("entity_attack_ON")] = _Vcsrt(6,10, 10,
		__int_, __int_, __int_, __int_, __Uint_, __Uint_,
		__bool_, __bool_, __Uint_, __int_ );

	 __ViD[wxT("entity_attack")] = _Vcsrt(1,10, 10,
		__int_, __int_, __int_, __int_, __Uint_, __Uint_,
		__bool_, __bool_, __Uint_, __int_ );

	 __ViD[wxT("entity_attack_blast")] = _Vcsrt(1,9, 9,
		__int_, __int_, __int_, __int_, __Uint_,
		__bool_, __bool_, __Uint_, __int_ );


 	// entity spawn
	 __ViD[wxT("entity_spawn")] = _Vcsrt(1,5, 5, __Uint_, __int_, __int_, __int_, __bool_ );

	// smartBomb
	_V* set__smartBomb = _VSet2( t_size_of( _t__smartBomb ), _t__smartBomb );
	__ViD[wxT("entity_smartBomb")] = _Vcsrt(2,4, 4, __int_, set__smartBomb, __bool_, __Uint_ );

	// entity_filter_type
	_V* set__hostile = _VSet2( t_size_of( _t__targetType ), _t__targetType );
	__ViD[wxT("entity_filter_type")] = _Vcsrt(1,5, 5, set__hostile, set__hostile, set__hostile, set__hostile, set__hostile );

	// Atchain
	 __ViD[wxT("at_chain")] = _Vcsrt(1,15, 15,
	 		__Uint_, __Uint_, __Uint_, __Uint_, __Uint_,
	 		__Uint_, __Uint_, __Uint_, __Uint_, __Uint_,
	 		__Uint_, __Uint_, __Uint_, __Uint_, __Uint_);

	// com
	_V* set__ent_directions = _VSet2( t_size_of( _t__directions ), _t__directions );
	 __ViD[wxT("entity_com")] = _Vcsrt(3,17, 17,
		_VSet( 1, "freespecial#" ),
		_VSet( 7, "A", "A2", "A3", "A4", "J", "S", "K" )
		 , set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions,
		 set__ent_directions, set__ent_directions, set__ent_directions );

 __ViD[wxT("ent_lifebarstatus")] = _Vcsrt( 2, 10, 10,
	__Uint_, __Uint_, __bool_, __bool_, __bool_, __int_, __int_, __int_, __int_, __int_ );

 __ViD[wxT("platform")] = _Vcsrt( 8, 8, 8,
 __int_, __int_, __int_, __int_, __int_, __int_, __int_, __int_ );

 __ViD[wxT("drawmethod")] = _Vcsrt( 1, 10, 10,
	__int_, __int_, __bool_, __bool_, __int_, __int_ ,
	__int_, __Uint_, __Uint_, __bool_);


// Models.txt constraints
	models_constraints[wxT("name")] = __ViD[wxT("strR")];
	models_constraints[wxT("ajspecial")] = __ViD[wxT("boolR")];
	models_constraints[wxT("autoland")] = __ViD[wxT("UintR")];
	models_constraints[wxT("colourselect")] = __ViD[wxT("boolR")];
	models_constraints[wxT("nocost")] = __ViD[wxT("boolR")];
	models_constraints[wxT("nolost")] = __ViD[wxT("boolR")];
	models_constraints[wxT("blockratio")] = __ViD[wxT("boolR")];
	models_constraints[wxT("mpblock")] = __ViD[wxT("boolR")];
	models_constraints[wxT("nochipdeath")] = __ViD[wxT("boolR")];
	models_constraints[wxT("nochipdeath")] = __ViD[wxT("boolR")];
	models_constraints[wxT("versusdamage")] = __ViD[wxT("boolR")];
	models_constraints[wxT("spdirection")] = __ViD[wxT("4_bool")];
	models_constraints[wxT("maxattacks")] = __ViD[wxT("UintR")];
	models_constraints[wxT("maxattacktypes")] = __ViD[wxT("UintR")];
	models_constraints[wxT("maxfollows")] = __ViD[wxT("UintR")];
	models_constraints[wxT("maxfreespecials")] = __ViD[wxT("UintR")];
	models_constraints[wxT("lifescore")] = __ViD[wxT("intR")];
	models_constraints[wxT("credscore")] = __ViD[wxT("intR")];
	models_constraints[wxT("nocheats")] = __ViD[wxT("boolR")];
	models_constraints[wxT("load")] = __ViD[wxT("str_pathR")];
	models_constraints[wxT("know")] = __ViD[wxT("str_pathR")];

	// entity files constraints
	entity_constraints[wxT("name")] = __ViD[wxT("strR")];
	entity_constraints[wxT("type")] = __ViD[wxT("entity_type")];
	entity_constraints[wxT("subtype")] = __ViD[wxT("entity_subtype")];
	entity_constraints[wxT("health")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("mp")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("credit")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("alpha")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("speed")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("speedf")] = __ViD[wxT("floatR")];
	entity_constraints[wxT("running")] = __ViD[wxT("running")];
	entity_constraints[wxT("nomove")] = __ViD[wxT("2_bool")];
	entity_constraints[wxT("jumpspeed")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("jumpspeedf")] = __ViD[wxT("floatR")];
	entity_constraints[wxT("jumpheight")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("grabdistance")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("throwdamage")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("throw")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("throwframewait")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("height")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("secret")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("shadow")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("fmap")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("load")] = __ViD[wxT("strR")];
	entity_constraints[wxT("project")] = __ViD[wxT("strR")];
	entity_constraints[wxT("shootnum")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("counter")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("reload")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("typeshot")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("animal")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("playshot")] = __ViD[wxT("strR")];
	entity_constraints[wxT("playshotno")] = __ViD[wxT("strR")];
	entity_constraints[wxT("knife")] = __ViD[wxT("strR")];
	entity_constraints[wxT("star")] = __ViD[wxT("strR")];
	entity_constraints[wxT("bomb")] = __ViD[wxT("strR")];
	entity_constraints[wxT("pbomb")] = __ViD[wxT("strR")];
	entity_constraints[wxT("hitenemy")] = __ViD[wxT("2_bool")];
	entity_constraints[wxT("rider")] = __ViD[wxT("strR")];
	entity_constraints[wxT("flash")] = __ViD[wxT("strR")];
	entity_constraints[wxT("bflash")] = __ViD[wxT("strR")];
	entity_constraints[wxT("dust")] = __ViD[wxT("strR")];
	entity_constraints[wxT("nolife")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("noquake")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("nopain")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("nodrop")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("nodieblink")] = __ViD[wxT("Uint_bool")];
	entity_constraints[wxT("makeinv")] = __ViD[wxT("Uint_bool")];
	entity_constraints[wxT("blockodds")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("thold")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("blockpain")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("falldie")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("aironly")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("setlayer")] = __ViD[wxT("intR")];
	entity_constraints[wxT("grabback")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("grabfinish")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("icon")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("iconpain")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("icondie")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("iconget")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("iconw")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("iconmphigh")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("iconmphalf")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("iconmplow")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("diesound")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("parrow")] = __ViD[wxT("path_x_yR")];
	entity_constraints[wxT("parrow2")] = __ViD[wxT("path_x_yR")];
	entity_constraints[wxT("score")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("smartbomb")] = __ViD[wxT("entity_smartBomb")];
	entity_constraints[wxT("toflip")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("cantgrab")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("paingrab")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("noatflash")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("remove")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("escapehits")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("com")] = __ViD[wxT("entity_com")];
	arr_Constraints_To_Reverse.Add( wxT("com") );
	entity_constraints[wxT("remap")] = __ViD[wxT("2_path")];
	entity_constraints[wxT("atchain")] = __ViD[wxT("at_chain")];
	entity_constraints[wxT("chargerate")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("mprate]")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("risetime")] = __ViD[wxT("intR")];
	entity_constraints[wxT("turndelay")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("facing")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("weaploss")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("branch")] = __ViD[wxT("strR")];
	entity_constraints[wxT("hostile")] = __ViD[wxT("entity_filter_type")];
	entity_constraints[wxT("candamage")] = __ViD[wxT("entity_filter_type")];
	entity_constraints[wxT("projectilehit")] = __ViD[wxT("entity_filter_type")];
	entity_constraints[wxT("aggression")] = __ViD[wxT("intR")];
	entity_constraints[wxT("antigrab")] = __ViD[wxT("intR")];
	entity_constraints[wxT("grabforce")] = __ViD[wxT("intR")];
	entity_constraints[wxT("offense")] = __ViD[wxT("off_def")];
	entity_constraints[wxT("defense")] = __ViD[wxT("off_def")];
	entity_constraints[wxT("hmap")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("bounce")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("grabwalk")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("grabturn")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("load")] = __ViD[wxT("str_pathR")];
	entity_constraints[wxT("lifespan")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("nopassiveblock")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("knockdowncount")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("antigravity")] = __ViD[wxT("intR")];
	entity_constraints[wxT("aimove")] = __ViD[wxT("entity_aimove")];
	entity_constraints[wxT("gfxshadow")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("holdblock")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("jumpmove")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("riseinv")] = __ViD[wxT("Uint_bool")];
	entity_constraints[wxT("lifebarstatus")] = __ViD[wxT("ent_lifebarstatus")];
	entity_constraints[wxT("lifeposition")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("nameposition")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("iconposition")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("no_adjust_base")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("subject_to_wall")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("subject_to_hole")] = __ViD[wxT("boolR")] ;
	entity_constraints[wxT("subject_to_obstacle")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("subject_to_platform")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("subject_to_gravity")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("subject_to_screen")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("falldie")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("death")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("resetable")] = __ViD[wxT("int")];
	entity_constraints[wxT("sleepwait")] = __ViD[wxT("int")];
	entity_constraints[wxT("always_go_ahead")] = __ViD[wxT("bool")];
	entity_constraints[wxT("always_walk")] = __ViD[wxT("bool")];
	entity_constraints[wxT("nodropfly")] = __ViD[wxT("bool")];
	entity_constraints[wxT("palette")] = __ViD[wxT("pathR_andMore")];
	entity_constraints[wxT("alternatepal")] = __ViD[wxT("pathR")];

	//Animations properties
	entity_constraints[wxT("animation")] = __ViD[wxT("entity_animation")];
	entity_constraints[wxT("anim")] = __ViD[wxT("entity_animation")];

	entity_constraints[wxT("loop")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("fastattack")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("hitfx")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("hitflash")] = __ViD[wxT("strR")];
	entity_constraints[wxT("custknife")] = __ViD[wxT("strR")];
	entity_constraints[wxT("custstar")] = __ViD[wxT("strR")];
	entity_constraints[wxT("custbomb")] = __ViD[wxT("strR")];
	entity_constraints[wxT("custpbomb")] = __ViD[wxT("strR")];
	entity_constraints[wxT("delay")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("offset")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("bbox")] = __ViD[wxT("5_int")];
	entity_constraints[wxT("frame")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("range")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("rangez")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("rangea")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("attack")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack1")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack2")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack3")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack4")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack5")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack6")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack7")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack8")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("attack9")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("blast")] = __ViD[wxT("entity_attack_blast")];
	entity_constraints[wxT("shock")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("burn")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("freeze")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("steal")] = __ViD[wxT("entity_attack")];
	entity_constraints[wxT("quakeframe")] = __ViD[wxT("3_Uint")];
	entity_constraints[wxT("move")] = __ViD[wxT("intR")];
	entity_constraints[wxT("movea")] = __ViD[wxT("intR")];
	entity_constraints[wxT("movez")] = __ViD[wxT("intR")];
	entity_constraints[wxT("seta")] = __ViD[wxT("intR")];
	entity_constraints[wxT("platform")] = __ViD[wxT("platform")];
	entity_constraints[wxT("dive")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("sound")] = __ViD[wxT("path")];
	entity_constraints[wxT("pshotframe")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("throwframe")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("tossframe")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("pbombframe")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("jumpframe")] = __ViD[wxT("4_int")];
	entity_constraints[wxT("custpshot")] = __ViD[wxT("strR")];
	entity_constraints[wxT("mpcost")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("custfireb")] = __ViD[wxT("strR")];
	entity_constraints[wxT("shootframe")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("flipframe")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("followanim")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("followcond")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("counterframe")] = __ViD[wxT("3_Uint")];
	entity_constraints[wxT("spawnframe")] = __ViD[wxT("entity_spawn")];
	entity_constraints[wxT("summonframe")] = __ViD[wxT("entity_spawn")];
	entity_constraints[wxT("unsummonframe")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("subentity")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("custentity")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("weaponframe")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("attackone")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("grabin")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("forcedirection")] = __ViD[wxT("intR")];
	entity_constraints[wxT("damageonlanding")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("dropv")] = __ViD[wxT("3_int")];
	entity_constraints[wxT("dropframe")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("landframe")] = __ViD[wxT("UintR")];
	entity_constraints[wxT("counterattack")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("fshadow")] = __ViD[wxT("intR")];
	entity_constraints[wxT("shadowcoords")] = __ViD[wxT("2_int")];
	entity_constraints[wxT("itembox")] = __ViD[wxT("4_int")];
	entity_constraints[wxT("stun")] = __ViD[wxT("intR")];
	entity_constraints[wxT("seal")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("forcemap")] = __ViD[wxT("2_Uint")];
	entity_constraints[wxT("drain")] = __ViD[wxT("4_Uint")];
	entity_constraints[wxT("noreflect")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("drawmethod")] = __ViD[wxT("drawmethod")];
	entity_constraints[wxT("nodrawmethod")] = __ViD[wxT("EMPTY")];
	entity_constraints[wxT("bouncefactor")] = __ViD[wxT("intR")];

	entity_constraints[wxT("mponly")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("hponly")] = __ViD[wxT("boolR")];
	entity_constraints[wxT("energycost")] = __ViD[wxT("intR")];
	entity_constraints[wxT("throwframewait")] = __ViD[wxT("UintR")];

	entity_constraints[wxT("animationscript")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("takedamagescript")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("script")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("ondeathscript")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("didhitscript")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("didblockscript")] = __ViD[wxT("pathR")];
	entity_constraints[wxT("@cmd")] = __ViD[wxT("noCstt")];
	entity_constraints[wxT("@script")] = __ViD[wxT("noCstt")];
	entity_constraints[wxT("@end_script")] = __ViD[wxT("noCstt")];
	entity_constraints[wxT("fastattack")]  = __ViD[wxT("boolR")];
}
