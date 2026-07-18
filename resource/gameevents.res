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

"gameevents"
{
	//////////////////////////////////////////////////////////////////////
	// Server events
	//////////////////////////////////////////////////////////////////////

	"server_spawn"		// Server is starting up
	{
		"eventid"	"1"
		"hostname"	"string"	// public host name
		"address"	"string"	// server address (IP:port)
		"port"		"short"		// server port
		"game"		"string"	// game dir
		"mapname"	"string"	// map name
		"startdate"	"string"	// start date (yy.mm.dd)
		"starttime"	"string"	// start time (hh.mm.ss)
		"maxplayers"	"byte"		// max players
		"os"		"string"	// WIN32 or LINUX
		"dedicated"	"byte"		// dedicated server?
	}

	"server_shutdown"	// Server is shutting down
	{
		"eventid"	"2"
		"reason"	"string"	// reason for shutdown
	}

	"server_cvar"		// a cvar has changed
	{
		"eventid"	"3"
		"cvarname"	"string"	// cvar name
		"cvarvalue"	"string"	// new cvar value
	}

	//////////////////////////////////////////////////////////////////////
	// Player events
	//////////////////////////////////////////////////////////////////////

	"player_connect"	// a player connected to the server
	{
		"eventid"	"10"
		"name"		"string"	// player name
		"index"		"byte"		// player slot (entity index-1)
		"userid"	"short"		// user ID
		"networkid"	"string"	// player network (i.e steam) id
		"address"	"string"	// IP address
	}

	"player_disconnect"	// player disconnected from the server
	{
		"eventid"	"11"
		"userid"	"short"		// user ID
		"reason"	"string"	// reason for disconnect
		"name"		"string"	// player name
		"networkid"	"string"	// player network (i.e steam) id
	}

	"player_activate"	// player is fully active in game
	{
		"eventid"	"12"
		"userid"	"short"		// user ID
	}

	"player_spawn"		// player spawned in game
	{
		"eventid"	"13"
		"userid"	"short"		// user ID
	}

	"player_death"
	{
		"eventid"	"14"
		"userid"	"short"		// user ID who died
		"attacker"	"short"		// user ID who killed
		"weapon"	"string"	// weapon name
	}

	"player_info"		// player info updated
	{
		"eventid"	"15"
		"name"		"string"	// player name
		"index"		"byte"		// player slot (entity index-1)
		"userid"	"short"		// user ID
		"networkid"	"string"	// player network (i.e steam) id
		"bot"		"bool"		// is player a bot
	}

	"player_team"		// player changed teams
	{
		"eventid"	"16"
		"userid"	"short"		// user ID
		"team"		"byte"		// new team index
		"oldteam"	"byte"		// old team index
		"disconnect"	"bool"		// leaving because of disconnect
	}

	"player_changename"	// player changed name
	{
		"eventid"	"17"
		"userid"	"short"		// user ID
		"oldname"	"string"	// old name
		"newname"	"string"	// new name
	}

	"player_hurt"		// player was hurt
	{
		"eventid"	"18"
		"userid"	"short"		// user ID
		"attacker"	"short"		// user ID of attacker
		"health"	"byte"		// remaining health
	}

	"player_chat"		// player sent a chat message
	{
		"eventid"	"19"
		"teamonly"	"bool"		// team only message
		"userid"	"short"		// user ID
		"text"		"string"	// chat text
	}

	//////////////////////////////////////////////////////////////////////
	// Game events
	//////////////////////////////////////////////////////////////////////

	"game_newmap"		// a new map is loading
	{
		"eventid"	"30"
		"mapname"	"string"	// map name
	}

	"game_start"		// game started
	{
		"eventid"	"31"
		"roundslimit"	"long"		// rounds limit
		"timelimit"	"long"		// time limit
		"fraglimit"	"long"		// frag limit
		"objective"	"string"	// round objective
	}

	"game_end"		// game ended
	{
		"eventid"	"32"
		"winner"	"byte"		// winner team/player
	}

	"round_start"		// round started
	{
		"eventid"	"33"
		"timelimit"	"long"		// round time limit
		"fraglimit"	"long"		// frag limit
		"objective"	"string"	// round objective
	}

	"round_end"		// round ended
	{
		"eventid"	"34"
		"winner"	"byte"		// winner team
		"reason"	"byte"		// reason round ended
		"message"	"string"	// end message
	}
}
