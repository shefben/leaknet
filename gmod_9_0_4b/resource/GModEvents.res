//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//=============================================================================

// No spaces in event names, max length 32
// All strings are case sensitive
// total game event byte length must be < 1024
//
// valid data key types are:
//   none   : value is not networked
//   string : a zero terminated string
//   bool   : unsigned int, 1 bit
//   byte   : unsigned int, 8 bit
//   short  : signed int, 16 bit
//   long   : signed int, 32 bit
//   float  : float, 32 bit

"gmodevents"
{
	// So we can show notifications on NPC deaths and stuff.
	"gmod_death"
	{
		"killer"	"string"	// Name of killer	
		"victim"	"string"	// Name of victim
		"weapon"	"string"	// Weapon used
		"killicon"	"string" 	// Name of texture to use (actual name - like "death_crossbow_bolt")
		"killerteam"	"byte"
		"victimteam"	"byte"
	}
	
	// Override - added an extra field for the kill icon
	"player_death"				
	{
		"userid"	"short"   	// user ID who died				
		"attacker"	"short"	 	// user ID who killed
		"weapon"	"string" 	// weapon name killer used 
		"killicon"	"string"	// Icon texture to use
	}
	
}
